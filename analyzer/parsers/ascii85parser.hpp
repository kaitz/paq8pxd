#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

#define base85max 0x8000000 //128M limit

class ascii85Parser: public Parser {
    uint64_t b85s,b85s1,b85p,b85slen,b85h;
    uint64_t base85start,base85end,b85line;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    bool is_base85(unsigned char c);
public:    
    ascii85Parser();
    ~ascii85Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
