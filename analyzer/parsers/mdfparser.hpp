#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include "../../filters/cdfilter.hpp"

// CD sectors detection (mode 1 and mode 2 form 1+2 - 2352 bytes)
class mdfParser: public Parser {
    uint64_t mdfa;
    uint64_t cdi;
    int cda,cdm,cdif;   // For CD sectors detection
    uint32_t cdf;
    uint8_t *cdata;
    uint32_t cdatai;
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    mdfParser();
    ~mdfParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
