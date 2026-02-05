#pragma once
#include "filter.hpp"
#include "shrink/shrink.h"

// This filter is based on https://www.hanshq.net/zip2.html
// Soruce: hwzip-2.4.zip (public domain)
// 
// This filter needs reconstruction info.

class shrinkFilter : public Filter {
public:
    shrinkFilter(std::string n, Filetype f = DEFAULT);
    ~shrinkFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
