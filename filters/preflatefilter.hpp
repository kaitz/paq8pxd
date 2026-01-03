#pragma once
#include "filter.hpp"

class preflateFilter : public Filter {
public:
    preflateFilter(std::string n, Filetype f = DEFAULT);
    ~preflateFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
