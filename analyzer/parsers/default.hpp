#pragma once
#include "../parser.hpp"
#include "../prt/enums.hpp"
#include <cstdint>
#include <vector>


class DefaultParser: public Parser {
    uint64_t info;
    uint32_t i;
    Filetype type;
    uint64_t jstart,jend,inpos;
public:    
    DefaultParser();
    ~DefaultParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
};
