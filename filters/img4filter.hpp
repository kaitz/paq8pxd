#pragma once
#include "filter.hpp"
#include <vector>
#include <algorithm>

class Img4Filter: public Filter {

public:
    Img4Filter(std::string n, Settings &s, Filetype f=DEFAULT);
    ~Img4Filter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
