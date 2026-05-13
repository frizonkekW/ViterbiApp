#include "ViterbiDecoder.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

void ViterbiDecoder::precompute() {
    const std::size_t num_states = code_.stateCount();
    const std::size_t input_symbol_count = std::size_t(1) << code_.inputBitsPerStep();

    next_state_lut_.assign(num_states, std::vector<std::size_t>(input_symbol_count, 0U));
    output_bits_lut_.assign(
        num_states,
        std::vector<std::vector<int>>(input_symbol_count, std::vector<int>(code_.outputBitsPerStep(), 0))
    );

    for (std::size_t state = 0U; state < num_states; ++state) {
        for (std::size_t input_symbol = 0U; input_symbol < input_symbol_count; ++input_symbol) {
            const TrellisTransition step = code_.transition(state, input_symbol);
            next_state_lut_[state][input_symbol] = step.next_state;
            output_bits_lut_[state][input_symbol] = step.output_bits;
        }
    }
}

ViterbiDecoder::ViterbiDecoder(ConvolutionalCode code)
    : code_(std::move(code)),
        next_state_lut_(),
        output_bits_lut_() {
    precompute();
}

std::vector<int>  ViterbiDecoder::decode(const std::vector<int>& received, std::size_t original_bits_count) const {
    if (received.size() % code_.outputBitsPerStep() != 0U) {
        throw std::runtime_error("Invalid received size");
    }

    if (std::any_of(received.begin(), received.end(), [](int bit) {
            return bit != 0 && bit != 1;
        })) {
        throw std::invalid_argument("Received sequence must contain only 0/1 bits");
    }

    const std::size_t num_symbols = received.size() / code_.outputBitsPerStep();
    const std::size_t num_states = code_.stateCount();
    const std::size_t input_symbol_count = std::size_t(1) << code_.inputBitsPerStep();
    const std::size_t inf = std::numeric_limits<std::size_t>::max() / 4U;

    std::vector<std::size_t> path_metrics(num_states, inf);
    std::vector<std::size_t> next_metrics(num_states, inf);
    std::vector<TraceStep> traceback(num_symbols * num_states);

    path_metrics[0] = 0U;

    for (std::size_t t = 0U; t < num_symbols; ++t) {
        std::fill(next_metrics.begin(), next_metrics.end(), inf);

        for (std::size_t state = 0U; state < num_states; ++state) {
            const std::size_t current_metric = path_metrics[state];

            if (current_metric == inf) {
                continue;
            }

            for (std::size_t input_symbol = 0U; input_symbol < input_symbol_count; ++input_symbol) {
                const std::size_t next_state = next_state_lut_[state][input_symbol];
                const std::vector<int>& expected_bits = output_bits_lut_[state][input_symbol];

                std::size_t candidate_metric = current_metric;

                for (std::size_t j = 0U; j < code_.outputBitsPerStep(); ++j) {
                    const int rx_bit = received[t * code_.outputBitsPerStep() + j];
                    candidate_metric += (rx_bit != expected_bits[j]) ? 1U : 0U;
                }

                if (candidate_metric < next_metrics[next_state]) {
                    next_metrics[next_state] = candidate_metric;
                    traceback[t * num_states + next_state] = TraceStep{
                        state,
                        input_symbol,
                        true
                    };
                }
            }
        }

        path_metrics.swap(next_metrics);
    }

    std::vector<std::uint64_t> decoded_symbols(num_symbols, 0U);
    std::size_t current_state = 0U;

    for (std::size_t t = num_symbols; t-- > 0U;) {
        const TraceStep step = traceback[t * num_states + current_state];

        if (!step.valid) {
            throw std::runtime_error("Traceback failed");
        }

        decoded_symbols[t] = step.input_symbol;
        current_state = step.prev_state;
    }

    std::vector<int> decoded;
    decoded.reserve(num_symbols * code_.inputBitsPerStep());

    for (std::uint64_t symbol : decoded_symbols) {
        const std::vector<int> bits = code_.unpackInputSymbol(symbol);
        decoded.insert(decoded.end(), bits.begin(), bits.end());
    }

    const std::size_t tail_bits = code_.memory() * code_.inputBitsPerStep();

    if (decoded.size() >= tail_bits) {
        decoded.resize(decoded.size() - tail_bits);
    } else {
        decoded.clear();
    }

    if (original_bits_count > decoded.size()) {
        throw std::runtime_error("original_bits_count is too large");
    }

    decoded.resize(original_bits_count);
    return decoded;
}