#pragma once
#include "../parser.hpp"
#include <cstdint>

class ZIPParser: public Parser {
    uint32_t buf0, buf1, buf2, buf3;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    uint64_t info;
public:
    ZIPParser();
    ~ZIPParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
