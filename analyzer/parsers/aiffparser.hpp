#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// For AIFF detection
class AIFFParser: public Parser {
    uint64_t aiff;
    int aiffm,aiffs;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    AIFFParser();
    ~AIFFParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
