#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <cstring>
#include <deque>

//detect LZSS compressed data in compress.exe generated archives
class SZDDParser: public Parser {
    uint64_t fSZDD; //
    int lz2;
    uint32_t fsizez,csize,usize;
    uint32_t rpos;
    std::deque<uint8_t> d;
    uint8_t *LZringbuffer;
    uint32_t N,F,THRESHOLD;
    uint32_t icount,incount;
    uint32_t r, flags;
            uint32_t i1,c1, j, k;
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    SZDDParser();
    ~SZDDParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
