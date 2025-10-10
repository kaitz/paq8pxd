#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>
#include <mem.h>
// Detect EXE if the low order byte (little-endian) XX is more
// recently seen (and within 4K) if a relative to absolute address
// conversion is done in the context CALL/JMP (E8/E9) XX xx xx 00/FF
// 4 times in a row.  Detect end of EXE at the last
// place this happens when it does not happen for 64KB.
    
class EXEParser: public Parser {
    uint64_t abspos[256],  // CALL/JMP abs. addr. low byte -> last offset
    relpos[256];    // CALL/JMP relative addr. low byte -> last offset
    int e8e9count;  // number of consecutive CALL/JMPs
    uint64_t e8e9pos;    // offset of first CALL or JMP instruction
    uint64_t e8e9last;   // offset of most recent CALL or JMP
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
    void ReadEXE();
public:    
    EXEParser();
    ~EXEParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
