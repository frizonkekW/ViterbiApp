#include "BscChannel.h"
#include <stdexcept>

BscChannel::BscChannel(double error_probability)
    : error_probability_(0.0),
      gen_(std::random_device{}()),
      flip_distribution_(0.0) {
    if (error_probability < 0.0 || error_probability > 1.0) {
        throw std::invalid_argument("error_probability must be in range [0, 1]");
    }
    error_probability_ = error_probability;
    flip_distribution_ = std::bernoulli_distribution(error_probability_);
}

std::vector<int> BscChannel::transmit(const std::vector<int>& input) {
    std::vector<int> output;
    output.reserve(input.size());

    for (int bit : input) {
        if (bit != 0 && bit != 1) {
            throw std::invalid_argument("BSC input must contain only 0/1 bits");
        }
        const bool flip = flip_distribution_(gen_);
        output.push_back(flip ? (bit ^ 1) : bit);
    }
    return output;
}