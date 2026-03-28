#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class BMP2Parser: public Parser {
    int bpp, x, y;
    int64_t bmp;
    uint64_t  of, size;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    int TCOLORS;
    std::vector<ColorRGBA> bmcolor;
public:    
uint8_t pal[256];
    BMP2Parser();
    ~BMP2Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
