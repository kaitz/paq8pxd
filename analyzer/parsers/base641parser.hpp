#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// base64 encoded data detection
// detect base64 in html/xml container, single stream
class base64_1Parser: public Parser {
    uint64_t base64end,b64h,base64start;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    base64_1Parser();
    ~base64_1Parser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
