#include "ConvolutionalCode.h"

#include <algorithm>
#include <stdexcept>


void ConvolutionalCode::validateParameters(std::size_t input_bits_per_step,
                            std::size_t memory,
                            const std::vector<std::uint64_t>& generators) const {
    if (input_bits_per_step < 1U) {
        throw std::invalid_argument("input_bits_per_step must be at least 1");
    }

    if (memory < 1U) {
        throw std::invalid_argument("memory must be at least 1");
    }

    if (generators.empty()) {
        throw std::invalid_argument("generators must not be empty");
    }

    const std::size_t state_bits = input_bits_per_step * memory;
    const std::size_t register_bits = input_bits_per_step * (memory + 1U);

    if (register_bits > 63U) {
        throw std::invalid_argument("Too many bits for uint64_t-based implementation");
    }

    if (state_bits > 62U) {
        throw std::invalid_argument("State space is too large");
    }

    const std::uint64_t limit = (std::uint64_t(1) << register_bits);

    if (std::any_of(generators.begin(), generators.end(), [limit](std::uint64_t g) {
            return g == 0U || g >= limit;
        })) {
        throw std::invalid_argument("Each generator must be in range [1, 2^(k*(memory+1)) - 1]");
    }
}

int ConvolutionalCode::parity(std::uint64_t value) const {
    int count = 0;

    while (value != 0U) {
        value &= (value - 1U);
        ++count;
    }

    return count & 1;
}

std::size_t ConvolutionalCode::stateBitIndex(std::size_t stream, std::size_t delay) const {
    return stream * memory_ + (delay - 1U);
}

std::size_t ConvolutionalCode::registerBitIndex(std::size_t stream, std::size_t delay) const {
    return stream * (memory_ + 1U) + delay;
}

int ConvolutionalCode::getInputBit(std::uint64_t input_symbol, std::size_t stream) const {
    return static_cast<int>((input_symbol >> stream) & 1U);
}

int ConvolutionalCode::getStateBit(std::uint64_t state, std::size_t stream, std::size_t delay) const {
    return static_cast<int>((state >> stateBitIndex(stream, delay)) & 1U);
}

ConvolutionalCode::ConvolutionalCode(std::size_t input_bits_per_step,
                    std::size_t memory,
                    std::vector<std::uint64_t> generators)
    : input_bits_per_step_(0U),
        output_bits_per_step_(0U),
        memory_(0U),
        state_bits_(0U),
        register_bits_(0U),
        num_states_(0U),
        state_mask_(0U),
        generators_(),
        codec_(input_bits_per_step) {
    validateParameters(input_bits_per_step, memory, generators);

    input_bits_per_step_ = input_bits_per_step;
    output_bits_per_step_ = generators.size();
    memory_ = memory;
    state_bits_ = input_bits_per_step_ * memory_;
    register_bits_ = input_bits_per_step_ * (memory_ + 1U);
    num_states_ = std::size_t(1) << state_bits_;
    generators_ = std::move(generators);
    state_mask_ = (state_bits_ == 0U) ? 0U : ((std::uint64_t(1) << state_bits_) - 1U);
}

std::size_t ConvolutionalCode::inputBitsPerStep() const {
    return input_bits_per_step_;
}

std::size_t ConvolutionalCode::outputBitsPerStep() const {
    return output_bits_per_step_;
}

std::size_t ConvolutionalCode::memory() const {
    return memory_;
}

std::size_t ConvolutionalCode::stateCount() const {
    return num_states_;
}

std::uint64_t ConvolutionalCode::packInputSymbol(const std::vector<int>& data, std::size_t offset) const {
    return codec_.pack(data, offset);
}

std::vector<int> ConvolutionalCode::unpackInputSymbol(std::uint64_t symbol) const {
    return codec_.unpack(symbol);
}

TrellisTransition ConvolutionalCode::transition(std::size_t state, std::uint64_t input_symbol) const {
    std::uint64_t full_register = 0U;
    std::uint64_t next_state = 0U;
    std::vector<int> output_bits(output_bits_per_step_, 0);

    for (std::size_t stream = 0U; stream < input_bits_per_step_; ++stream) {
        const int input_bit = getInputBit(input_symbol, stream);

        if (input_bit == 1) {
            full_register |= (std::uint64_t(1) << registerBitIndex(stream, 0U));
            next_state |= (std::uint64_t(1) << stateBitIndex(stream, 1U));
        }

        for (std::size_t delay = 1U; delay <= memory_; ++delay) {
            const int state_bit = getStateBit(state, stream, delay);

            if (state_bit == 1) {
                full_register |= (std::uint64_t(1) << registerBitIndex(stream, delay));
            }

            if (delay < memory_ && state_bit == 1) {
                next_state |= (std::uint64_t(1) << stateBitIndex(stream, delay + 1U));
            }
        }
    }

    for (std::size_t i = 0U; i < output_bits_per_step_; ++i) {
        output_bits[i] = parity(full_register & generators_[i]);
    }

    return TrellisTransition{next_state & state_mask_, std::move(output_bits)};
}

std::vector<int> ConvolutionalCode::encode(const std::vector<int>& data) const {
    std::vector<int> terminated_data = data;
    terminated_data.insert(terminated_data.end(), memory_ * input_bits_per_step_, 0);

    std::vector<int> encoded;
    const std::size_t num_steps =
        (terminated_data.size() + input_bits_per_step_ - 1U) / input_bits_per_step_;

    encoded.reserve(num_steps * output_bits_per_step_);

    std::size_t state = 0U;

    for (std::size_t offset = 0U; offset < terminated_data.size(); offset += input_bits_per_step_) {
        const std::uint64_t input_symbol = codec_.pack(terminated_data, offset);
        const TrellisTransition step = transition(state, input_symbol);

        encoded.insert(encoded.end(), step.output_bits.begin(), step.output_bits.end());
        state = step.next_state;
    }

    return encoded;
}