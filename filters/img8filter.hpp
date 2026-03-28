#pragma once
#include "filter.hpp"

class Img8Filter: public Filter {

public:
    Img8Filter(std::string n, Settings &s, Filetype f=DEFAULT);
    ~Img8Filter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
