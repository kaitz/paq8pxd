#pragma once
#include "filter.hpp"

class PNGFilter : public Filter {
public:
    PNGFilter(std::string n, Filetype f);
    ~PNGFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
