#pragma once
#include "../parser.hpp"
#include <cstdint>
#include <vector>

// NES rom 
// The format of the header is as follows:
// 0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
// 4: Size of PRG ROM in 16 KB units
// 5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
// 6: Flags 6
// 7: Flags 7
// 8: Size of PRG RAM in 8 KB units (Value 0 infers 8 KB for compatibility; see PRG RAM circuit)
// 9: Flags 9
// 10: Flags 10 (unofficial)
// 11-15: Zero filled
class NesParser: public Parser {
    uint64_t nesh,nesp,nesc;
    uint64_t info;
    uint32_t buf0;
    uint32_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    NesParser();
    ~NesParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last);
    dType getType(int i);
    int TypeCount();
    void Reset();
    void SetEnd(uint64_t e);
};
