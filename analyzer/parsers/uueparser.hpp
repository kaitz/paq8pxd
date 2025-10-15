#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// detect uuencoode in eml 
// only 61 byte linesize and, ignore with trailin 1 byte lines.
// only '\n' line endings
class uueParser: public Parser {
    uint64_t uuds,uuds1,uudp,uudslen,uudh;
    uint64_t uudstart,uudend,uudline,uudnl,uudlcount,uuc;
    uint64_t info;
    uint32_t buf0, buf1, buf2;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    uueParser();
    ~uueParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
