#pragma once
#include "../parser.hpp"
#include "../../zlib/zlib.h"
#include "../../prt/helper.hpp"
#include <cstdint>
#include <vector>

class zlibParser: public Parser {
    uint8_t zbuf[256+32], zin[1<<16], zout[1<<16]; // For ZLIB stream detection
    int zbufpos, histogram[256];
    //bool valid;
    bool brute;
    int pdfim,pdfimw,pdfimh,pdfimb,pdfgray;
    uint64_t pdfimp;
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    int parse_zlib_header(int header);
    int zlib_inflateInit(z_streamp strm, int zh);
    z_stream *strm;
    void SetPdfImageInfo();
public:    
    zlibParser(bool b=true);
    ~zlibParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
