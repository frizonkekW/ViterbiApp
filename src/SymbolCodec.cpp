#include "SymbolCodec.h"

#include <stdexcept>

SymbolCodec::SymbolCodec(std::size_t bits_per_symbol)
    : bits_per_symbol_(bits_per_symbol) {
    if (bits_per_symbol_ < 1U) {
        throw std::invalid_argument("bits_per_symbol must be at least 1");
    }
    if (bits_per_symbol_ > 63U) {
        throw std::invalid_argument("bits_per_symbol is too large");
    }
}

std::uint64_t SymbolCodec::pack(const std::vector<int>& data, std::size_t offset) const {
    std::uint64_t symbol = 0U;

    for (std::size_t i = 0U; i < bits_per_symbol_; ++i) {
        const std::size_t index = offset + i;
        const int bit = (index < data.size()) ? data[index] : 0;

        if (bit != 0 && bit != 1) {
            throw std::invalid_argument("Bit vector must contain only 0/1");
        }

        if (bit == 1) {
            symbol |= (std::uint64_t(1) << i);
        }
    }

    return symbol;
}

std::vector<int> SymbolCodec::unpack(std::uint64_t symbol) const {
    std::vector<int> bits(bits_per_symbol_, 0);

    for (std::size_t i = 0U; i < bits_per_symbol_; ++i) {
        bits[i] = static_cast<int>((symbol >> i) & 1U);
    }

    return bits;
}

std::size_t SymbolCodec::width() const {
    return bits_per_symbol_;
}