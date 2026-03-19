#pragma once
#include "filter.hpp"
#include <vector>
#include <algorithm>

struct Color8 {
    union {
	uint32_t  c;
	uint8_t   rgba[4];
	};
	uint8_t   i;
};

class Img8Filter: public Filter {
    std::vector<Color8> bmcolor;
public:
    Img8Filter(std::string n, Settings &s, Filetype f=DEFAULT);
    ~Img8Filter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};
