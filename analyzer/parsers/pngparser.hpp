#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// Only signle IDAT chunk images are supported
class PNGParser: public Parser {
    uint64_t png, lastchunk, nextchunk;               // For PNG detection
    int pngw, pngh, pngbps, pngtype,pnggray; 
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    PNGParser();
    ~PNGParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
