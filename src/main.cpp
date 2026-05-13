#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "BscChannel.h"
#include "ConvolutionalCode.h"
#include "ViterbiDecoder.h"
#include "SymbolCodec.h"


struct CodeExperiment {
    std::string name;
    std::size_t input_bits_per_step;
    std::size_t memory;
    std::vector<std::uint64_t> generators;
};

std::vector<int> generateRandomBits(std::size_t bit_count, std::mt19937& gen) {
    std::uniform_int_distribution<int> dist(0, 1);
    std::vector<int> bits;
    bits.reserve(bit_count);

    for (std::size_t i = 0U; i < bit_count; ++i) {
        bits.push_back(dist(gen));
    }

    return bits;
}

std::size_t countBitErrors(const std::vector<int>& expected, const std::vector<int>& actual) {
    if (expected.size() != actual.size()) {
        throw std::runtime_error("Bit vectors must have the same size");
    }

    return std::inner_product(
        expected.begin(),
        expected.end(),
        actual.begin(),
        std::size_t{0},
        std::plus<>{},
        [](int lhs, int rhs) -> std::size_t {
            return (lhs != rhs) ? std::size_t{1} : std::size_t{0};
        }
    );
}

double runMonteCarloOnce(const CodeExperiment& experiment,
                         double channel_error_probability,
                         std::size_t information_bits_count,
                         std::size_t monte_carlo_trials,
                         std::mt19937& bit_gen) {
    ConvolutionalCode code(
        experiment.input_bits_per_step,
        experiment.memory,
        experiment.generators
    );

    ViterbiDecoder decoder(code);

    std::size_t total_errors = 0U;
    std::size_t total_bits = 0U;

    for (std::size_t trial = 0U; trial < monte_carlo_trials; ++trial) {
        const std::vector<int> original_bits = generateRandomBits(information_bits_count, bit_gen);
        const std::vector<int> encoded_bits = code.encode(original_bits);

        BscChannel channel(channel_error_probability);
        const std::vector<int> received_bits = channel.transmit(encoded_bits);

        const std::vector<int> decoded_bits = decoder.decode(received_bits, original_bits.size());

        total_errors += countBitErrors(original_bits, decoded_bits);
        total_bits += original_bits.size();
    }

    return static_cast<double>(total_errors) / static_cast<double>(total_bits);
}

int main() {
    const std::size_t information_bits_count = 1500U;
    const std::size_t monte_carlo_trials = 200U;

    const double p_start = 0.0;
    const double p_end = 0.30;
    const double p_step = 0.01;

    std::ofstream csv("ber_results.csv");
    if (!csv.is_open()) {
        std::cerr << "Cannot open ber_results.csv\n";
        return 1;
    }

    csv << "code,p,trials,information_bits,ber\n";
    csv << std::fixed << std::setprecision(6);

    std::mt19937 bit_gen(std::random_device{}());

    std::vector<CodeExperiment> experiments;
    experiments.push_back(CodeExperiment{
        "1/2",
        1U,
        2U,
        {
            0b111U,
            0b101U
        }
    });

    experiments.push_back(CodeExperiment{
        "1/5",
        1U,
        4U,
        {
            0b11111U,
            0b11011U,
            0b10101U,
            0b10011U,
            0b01111U
        }
    });

    experiments.push_back(CodeExperiment{
        "2/3",
        2U,
        2U,
        {
            0b001011U,
            0b001100U,
            0b001001U
        }
    });

    for (const CodeExperiment& experiment : experiments) {
        for (double p = p_start; p <= p_end + 1e-12; p += p_step) {
            const double ber = runMonteCarloOnce(
                experiment,
                p,
                information_bits_count,
                monte_carlo_trials,
                bit_gen
            );

            csv << experiment.name << ","
                << p << ","
                << monte_carlo_trials << ","
                << information_bits_count << ","
                << ber << "\n";

            std::cout << "code=" << experiment.name
                      << " p=" << p
                      << " ber=" << ber << "\n";
        }
    }

    std::cout << "Saved to ber_results.csv\n";
    return 0;
}