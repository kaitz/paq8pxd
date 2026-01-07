#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// sgi/rgb image parser
class sgiParser: public Parser {
    uint64_t rgbi;
    int rgbx,rgby;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    sgiParser();
    ~sgiParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
