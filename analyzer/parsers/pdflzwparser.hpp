#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// In pdf stream detect only /LZWDecode
// no /ASCII85Decode /LZWDecode combo, etc
class PDFLzwParser: public Parser {
    uint64_t pLzwp;
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3, buf4;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    PDFLzwParser();
    ~PDFLzwParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
