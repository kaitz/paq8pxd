#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include "../../bzip2/bzlib.h"
#include "../../prt/helper.hpp"


class bzip2Parser: public Parser {
    uint64_t BZip2,dsize;
    int csize,blockz,inbytes,part;
    bz_stream stream;
    int bzlevel;
    bool isBSDIFF;
    uint8_t *bzout;
    uint64_t info;
    uint32_t buf0, buf1, buf2, buf3;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    bzip2Parser();
    ~bzip2Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
