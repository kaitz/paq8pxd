#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <cstring>

class ARMParser: public Parser {
    uint64_t absposARM[256], relposARM[256];
    int ARMcount;
    uint64_t ARMpos;
    uint64_t ARMlast;
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3, buf4;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    void ReadARM();
    uint32_t OpC(uint32_t op);
public:    
    ARMParser();
    ~ARMParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
