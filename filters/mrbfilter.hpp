#pragma once
#include "filter.hpp"
#include "../prt/types.hpp"

class ImgMRBFilter: public Filter {
private:
    int encodeRLE(U8 *dst, U8 *ptr, int src_end, int maxlen);
public:
    ImgMRBFilter(std::string n, Filetype f=DEFAULT);
    ~ImgMRBFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
