#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class TGAParser: public Parser {
    uint64_t tga;
    uint64_t tgax;
    int tgay,tgaz,tgat,tgaid,tgamap;//,total,line,detd,b;
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    TGAParser();
    ~TGAParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
