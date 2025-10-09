#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class TextParser: public Parser {
    uint64_t info;
    uint32_t i;
    Filetype type;
    uint64_t jstart,jend,inSize,inpos,txtStart,binLen,spaces,txtLen,txtMinLen;
public:    
    TextParser();
    ~TextParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
