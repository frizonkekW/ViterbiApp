#pragma once

#include <vector>
#include <random>

class BscChannel {
private:
    double error_probability_;
    std::mt19937 gen_;
    std::bernoulli_distribution flip_distribution_;

public:
    explicit BscChannel(double error_probability);
    std::vector<int> transmit(const std::vector<int>& input);
};