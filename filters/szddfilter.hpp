#pragma once
#include "filter.hpp"
#include "../prt/lzss.hpp"

class szddFilter: public Filter {
public:
    szddFilter(std::string n, Filetype f=DEFAULT);
    ~szddFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
  
};



