#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class WAVParser: public Parser {
    uint64_t wavi,wavlist;
    int wavsize,wavch,wavbps,wavm,wavsr,wavt,wavtype,wavlen;
    uint64_t info,info2;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    WAVParser();
    ~WAVParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
