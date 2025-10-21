#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

class MSCFParser: public Parser {
    int64_t mscf,mscfs,mscfoff;
    uint32_t  files, folders;
    uint64_t  of, size;
    uint64_t info;
    uint32_t buf0, buf1, buf2;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    MSCFParser();
    ~MSCFParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
