#pragma once
#include "filter.hpp"
#include "../prt/buffers.hpp"

class ImPngFilter: public Filter {
public:
    ImPngFilter(std::string n, Settings &s, Filetype f=DEFAULT);
    ~ImPngFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
    uint8_t paeth(uint8_t const W, uint8_t const N, uint8_t const NW);
};
