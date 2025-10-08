#pragma once
#include "../parser.hpp"
#include "../prt/enums.hpp"
#include <cstdint>
#include <vector>


class BMPParser: public Parser {
    int bmp, bpp, x, y;
    int of, size, hdrless; 
    uint64_t info;
    uint32_t buf0,buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart,jend,inSize,inpos;
public:    
    BMPParser();
    ~BMPParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
};
