#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class SymbolCodec {
public:
    explicit SymbolCodec(std::size_t bits_per_symbol);

    std::uint64_t pack(const std::vector<int>& data, std::size_t offset) const;
    std::vector<int> unpack(std::uint64_t symbol) const;
    std::size_t width() const;

private:
    std::size_t bits_per_symbol_;
};