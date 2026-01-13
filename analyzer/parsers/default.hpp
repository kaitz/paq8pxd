#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class DefaultParser: public Parser {
    uint64_t info;
    uint64_t i;
    Filetype type;
    uint64_t jstart,jend,inpos;
public:    
    DefaultParser();
    ~DefaultParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
