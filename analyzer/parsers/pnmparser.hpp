#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// portable anymap format (PNM)
// Detect .pbm .pgm .ppm .pam image
class PNMParser: public Parser {
    uint64_t pgm;
    int pgmcomment,pgmw,pgmh,pgm_ptr,pgmc,pgmn,pamatr,pamd;
    char pgm_buf[32];
    uint64_t info;
    uint32_t buf0, buf1, buf2;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    PNMParser();
    ~PNMParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
