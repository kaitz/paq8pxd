#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// detect rle encoded, uncompressed mrb files inside windows .hlp files
// signature: 0x506C (SHG,lP) or 0x706C (MRB,lp)
class mrbParser: public Parser {
    uint64_t mrb=0,mrbsize=0,mrbcsize=0,mrbPictureType=0,mrbPackingMethod=0,mrbTell=0,mrbTell1=0,mrbw=0,mrbh=0;
    uint32_t mrbmulti=0;
    uint32_t poffset=0;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    uint32_t GetCDWord(unsigned char *data );
    uint16_t GetCWord(unsigned char *data );
    uint8_t GetC(unsigned char *data );
public:    
    mrbParser();
    ~mrbParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
