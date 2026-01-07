#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class MSZIPParser: public Parser {
    uint64_t MSZ,MSZip,MSZipz,zlen;
    uint64_t count;//  yu=1;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    MSZIPParser();
    ~MSZIPParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
