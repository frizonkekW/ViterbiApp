#pragma once

#include "SymbolCodec.h"

#include <cstddef>
#include <cstdint>
#include <vector>

struct TrellisTransition {
    std::size_t next_state = 0U;
    std::vector<int> output_bits;
};

class ConvolutionalCode {
private:
    std::size_t input_bits_per_step_;
    std::size_t output_bits_per_step_;
    std::size_t memory_;
    std::size_t state_bits_;
    std::size_t register_bits_;
    std::size_t num_states_;
    std::uint64_t state_mask_;
    std::vector<std::uint64_t> generators_;
    SymbolCodec codec_;

    void validateParameters(std::size_t input_bits_per_step,
                            std::size_t memory,
                            const std::vector<std::uint64_t>& generators) const;

    int parity(std::uint64_t value) const;

    std::size_t stateBitIndex(std::size_t stream, std::size_t delay) const;

    std::size_t registerBitIndex(std::size_t stream, std::size_t delay) const;

    int getInputBit(std::uint64_t input_symbol, std::size_t stream) const ;

    int getStateBit(std::uint64_t state, std::size_t stream, std::size_t delay) const ;

public:
    ConvolutionalCode(std::size_t input_bits_per_step,
                      std::size_t memory,
                      std::vector<std::uint64_t> generators);

    std::size_t inputBitsPerStep() const;

    std::size_t outputBitsPerStep() const;

    std::size_t memory() const;

    std::size_t stateCount() const;

    std::uint64_t packInputSymbol(const std::vector<int>& data, std::size_t offset) const;

    std::vector<int> unpackInputSymbol(std::uint64_t symbol) const;

    TrellisTransition transition(std::size_t state, std::uint64_t input_symbol) const;

    std::vector<int> encode(const std::vector<int>& data) const;
};
