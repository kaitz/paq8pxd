#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class TGAParser: public Parser {
    uint64_t tga;
    uint64_t tgax;
    int tgay,tgaz,tgat,tgaid,tgamap;
    uint64_t info;
    uint32_t buf0, buf1;
    int32_t total,line,rlep;             // RLE
    uint8_t pal[768];                    // pal
    bool isPal;
    uint32_t palcolors,pali,tgagray;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    TGAParser();
    ~TGAParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    bool IsGrayscalePalette(uint8_t *palb, int n=256, int isRGBA=0);
    dType getType() final;
    void Reset() final;
};
