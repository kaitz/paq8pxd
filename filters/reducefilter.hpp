#pragma once
#include "filter.hpp"
#include "shrink/reduce.h"

// This filter is based on https://www.hanshq.net/zip2.html
// Soruce: hwzip-2.4.zip (public domain)
// 
// This filter works for 0% of the files tested. Currently not used.
// Encode is working, decode produces non-identical data.

class reduceFilter : public Filter {
public:
    reduceFilter(std::string n, Filetype f = DEFAULT);
    ~reduceFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
