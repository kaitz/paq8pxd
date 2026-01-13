#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// Detect JPEG by code SOI APPx (FF D8 FF Ex) followed by
// SOF0 (FF C0 xx xx 08) and SOS (FF DA) within a reasonable distance.
// Detect end by any code other than RST0-RST7 (FF D9-D7) or
// a byte stuff (FF 00).

class JPEGParser: public Parser {
    uint64_t soi, sof, sos, app,eoi; 
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    JPEGParser();
    ~JPEGParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
