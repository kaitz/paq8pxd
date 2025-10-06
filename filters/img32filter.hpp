#pragma once
#include "filter.hpp"

class Img32Filter: public Filter {
public:
    Img32Filter(std::string n, Filetype f=DEFAULT);
    ~Img32Filter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
};
