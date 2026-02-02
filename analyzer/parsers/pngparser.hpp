#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

struct PNGfile {
    uint64_t start;
    uint64_t size;
    Filetype f;
    uint64_t info;
};

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
    uint8_t pal[768];
    bool isPal;
    uint32_t palcolors,pali;
    bool isiCCP;
    uint32_t iCCPsize, iCCPinfo;
    Array<PNGfile> pngF;
    uint32_t pfcount;
    bool rec;
public:    
    PNGParser();
    ~PNGParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
    bool IsGrayscalePalette(uint8_t * palb, int n = 256, int isRGBA = 0);
};
