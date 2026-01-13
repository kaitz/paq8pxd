#pragma once
#include "../../prt/array.hpp"
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include "../../filters/cdfilter.hpp"


// CD sectors detection (mode 1 and mode 2 form 1+2 - 2352 bytes)
class cdParser: public Parser {
    uint64_t cdi;
    int cda,cdm,cdif;   // For CD sectors detection
    uint32_t cdf;
    Array<uint8_t> cdata;
    uint32_t cdatai,cdscont;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    cdParser();
    ~cdParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
