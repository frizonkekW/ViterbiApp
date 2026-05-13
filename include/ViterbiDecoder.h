#pragma once

#include "ConvolutionalCode.h"
#include <cstddef>
#include <vector>

struct TraceStep {
    std::size_t prev_state = 0U;
    std::size_t input_symbol = 0U;
    bool valid = false;
};


class ViterbiDecoder {
private:
    ConvolutionalCode code_;
    std::vector<std::vector<std::size_t>> next_state_lut_;
    std::vector<std::vector<std::vector<int>>> output_bits_lut_;

    void precompute();

public:
    explicit ViterbiDecoder(ConvolutionalCode code);

    std::vector<int> decode(const std::vector<int>& received, std::size_t original_bits_count) const;
};