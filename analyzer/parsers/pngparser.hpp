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
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    uint64_t idat_end;
    int idats;
public:    
    PNGParser();
    ~PNGParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
