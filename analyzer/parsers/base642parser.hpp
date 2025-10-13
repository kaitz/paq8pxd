#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

#define base64max 0x8000000 //128M limit
// base64 encoded data detection
// detect base64 in eml, etc. multiline
class base64_2Parser: public Parser {
    uint64_t b64s,b64p,b64slen,b64h;
    uint64_t base64start,base64end,b64line,b64nl,b64lcount;
    uint64_t info;
    uint32_t buf0, buf1;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    bool is_base64(unsigned char c);
public:    
    base64_2Parser();
    ~base64_2Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
