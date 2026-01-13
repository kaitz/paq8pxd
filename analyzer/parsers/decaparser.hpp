#pragma once
#include "../parser.hpp"
#include "../../prt/DECAlpha.hpp"
#include <cstdint>
#include <vector>

  // For DEC Alpha detection
  struct DEC_ALPHA {
    Array<uint64_t> absPos{ 256 };
    Array<uint64_t> relPos{ 256 };
    uint32_t opcode = 0u, idx = 0u, count[4] = { 0 }, branches[4] = { 0 };
    uint64_t offset = 0u, last = 0u;
  } ;

class DECaParser: public Parser {
    int bpp, x, y, hdrless;
    uint64_t bmp, of, size;
    DEC_ALPHA dec;
    uint64_t info;
    uint32_t buf0, buf1;
    uint64_t i;
    Filetype type;
    uint64_t jstart, jend, inSize, inpos;
public:    
    DECaParser();
    ~DECaParser();
    DetectState Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) final;
    dType getType() final;
    void Reset() final;
};
