#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class GIFParser: public Parser {
    uint64_t gif, gifa, gifi, gifw, gifc, gifb, plt, gray;
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    bool rec;
public:    
    GIFParser();
    ~GIFParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
