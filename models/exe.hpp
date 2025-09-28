#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
//#include "../prt/blockdata.hpp"
//#include "../prt/buffers.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirectcontext.hpp"
//#include "../prt/mft.hpp"
//#include "../prt/helper.hpp"
#include "../prt/contextmap2.hpp"
#include "../prt/indirect.hpp"

#define CacheSize (1<<5)

#if (CacheSize&(CacheSize-1)) || (CacheSize<8)
  #error Cache size must be a power of 2 bigger than 4
#endif
//////////////////////////// exeModel /////////////////////////

// Model x86 code.  The contexts are sparse containing only those
// bits relevant to parsing (2 prefixes, opcode, and mod and r/m fields
// of modR/M byte).

// formats
enum InstructionFormat {
  // encoding mode
  fNM = 0x0,      // no ModRM
  fAM = 0x1,      // no ModRM, "address mode" (jumps or direct addresses)
  fMR = 0x2,      // ModRM present
  fMEXTRA = 0x3,  // ModRM present, includes extra bits for opcode
  fMODE = 0x3,    // bitmask for mode

  // no ModRM: size of immediate operand
  fNI = 0x0,      // no immediate
  fBI = 0x4,      // byte immediate
  fWI = 0x8,      // word immediate
  fDI = 0xc,      // dword immediate
  fTYPE = 0xc,    // type mask

  // address mode: type of address operand
  fAD = 0x0,      // absolute address
  fDA = 0x4,      // dword absolute jump target
  fBR = 0x8,      // byte relative jump target
  fDR = 0xc,      // dword relative jump target

  // others
  fERR = 0xf     // denotes invalid opcodes
};

// 1 byte opcodes
const static U8 Table1[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 0
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 2
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 3

  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 4
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 5
  fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fDI,fMR|fDI,fNM|fBI,fMR|fBI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 6
  fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR, // 7

  fMR|fBI,fMR|fDI,fMR|fBI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 8
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fAM|fDA,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 9
  fAM|fAD,fAM|fAD,fAM|fAD,fAM|fAD,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // a
  fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI, // b

  fMR|fBI,fMR|fBI,fNM|fWI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fBI,fMR|fDI,fNM|fBI,fNM|fNI,fNM|fWI,fNM|fNI,fNM|fNI,fNM|fBI,fERR   ,fNM|fNI, // c
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fBI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // d
  fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fAM|fDR,fAM|fDR,fAM|fAD,fAM|fBR,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // e
  fNM|fNI,fERR   ,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fMEXTRA,fMEXTRA,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fMEXTRA,fMEXTRA, // f
};

// 2 byte opcodes
const static U8 Table2[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fNM|fNI,fERR   ,fNM|fNI,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 0
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 2
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fERR   ,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 3

  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 4
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 5
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 6
  fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI, // 7

  fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR, // 8
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 9
  fNM|fNI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fBI,fMR|fNI,fERR   ,fMR|fNI, // a
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // b

  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // c
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // d
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // e
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   , // f
};

// 3 byte opcodes $0F38XX
const static U8 Table3_38[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   , // 0
  fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fERR   , // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   , // 2
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 3
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 4
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 5
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 6
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 7
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 8
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 9
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // a
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // b
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // c
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // d
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // e
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // f
};

// 3 byte opcodes $0F3AXX
const static U8 Table3_3A[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI, // 0
  fERR   ,fERR   ,fERR   ,fERR   ,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 1
  fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 2
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 3
  fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 4
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 5
  fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 6
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 7
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 8
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 9
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // a
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // b
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // c
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // d
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // e
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // f
};

// escape opcodes using ModRM byte to get more variants
const static U8 TableX[32] = {
  // 0       1       2       3       4       5       6       7
  fMR|fBI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // escapes for 0xf6
  fMR|fDI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // escapes for 0xf7
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // escapes for 0xfe
  fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fERR   ,fMR|fNI,fERR   , // escapes for 0xff
};

const static U8 InvalidX64Ops[19] = {0x06, 0x07, 0x16, 0x17, 0x1E, 0x1F, 0x27, 0x2F, 0x37, 0x3F, 0x60, 0x61, 0x62, 0x82, 0x9A, 0xD4, 0xD5, 0xD6, 0xEA,};
const static U8 X64Prefixes[8] = {0x26, 0x2E, 0x36, 0x3E, 0x9B, 0xF0, 0xF2, 0xF3,};
enum InstructionCategory {
  OP_INVALID              =  0,
  OP_PREFIX_SEGREG        =  1,
  OP_PREFIX               =  2,
  OP_PREFIX_X87FPU        =  3,
  OP_GEN_DATAMOV          =  4,
  OP_GEN_STACK            =  5,
  OP_GEN_CONVERSION       =  6,
  OP_GEN_ARITH_DECIMAL    =  7,
  OP_GEN_ARITH_BINARY     =  8,
  OP_GEN_LOGICAL          =  9,
  OP_GEN_SHF_ROT          = 10,
  OP_GEN_BIT              = 11,
  OP_GEN_BRANCH           = 12,
  OP_GEN_BRANCH_COND      = 13,
  OP_GEN_BREAK            = 14,
  OP_GEN_STRING           = 15,
  OP_GEN_INOUT            = 16,
  OP_GEN_FLAG_CONTROL     = 17,
  OP_GEN_SEGREG           = 18,
  OP_GEN_CONTROL          = 19,
  OP_SYSTEM               = 20,
  OP_X87_DATAMOV          = 21,
  OP_X87_ARITH            = 22,
  OP_X87_COMPARISON       = 23,
  OP_X87_TRANSCENDENTAL   = 24,
  OP_X87_LOAD_CONSTANT    = 25,
  OP_X87_CONTROL          = 26,
  OP_X87_CONVERSION       = 27,
  OP_STATE_MANAGEMENT     = 28,
  OP_MMX                  = 29,
  OP_SSE                  = 30,
  OP_SSE_DATAMOV          = 31
};

const static U8 TypeOp1[256] = {
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //03
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //07
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //0B
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_STACK         , OP_PREFIX            , //0F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //13
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //17
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //1B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //1F
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //23
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //27
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //2B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //2F
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //33
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //37
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //3B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //3F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //43
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //47
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //4B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //4F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //53
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //57
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //5B
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //5F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_BREAK         , OP_GEN_CONVERSION    , //63
  OP_PREFIX_SEGREG     , OP_PREFIX_SEGREG     , OP_PREFIX            , OP_PREFIX            , //67
  OP_GEN_STACK         , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_ARITH_BINARY  , //6B
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //6F
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //73
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //77
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //7B
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //7F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //83
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //87
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //8B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_STACK         , //8F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //93
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //97
  OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , OP_GEN_BRANCH        , OP_PREFIX_X87FPU     , //9B
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //A3
  OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , //A7
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_STRING        , OP_GEN_STRING        , //AB
  OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , //AF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //B3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //B7
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //BB
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //BF
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //C3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //C7
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //CB
  OP_GEN_BREAK         , OP_GEN_BREAK         , OP_GEN_BREAK         , OP_GEN_BREAK         , //CF
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , //D3
  OP_GEN_ARITH_DECIMAL , OP_GEN_ARITH_DECIMAL , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //D7
  OP_X87_ARITH         , OP_X87_DATAMOV       , OP_X87_ARITH         , OP_X87_DATAMOV       , //DB
  OP_X87_ARITH         , OP_X87_DATAMOV       , OP_X87_ARITH         , OP_X87_DATAMOV       , //DF
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //E3
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //E7
  OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //EB
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //EF
  OP_PREFIX            , OP_GEN_BREAK         , OP_PREFIX            , OP_PREFIX            , //F3
  OP_SYSTEM            , OP_GEN_FLAG_CONTROL  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //F7
  OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , //FB
  OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_ARITH_BINARY  , OP_GEN_BRANCH         //FF
};

const static U8 TypeOp2[256] = {
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //03
  OP_INVALID           , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //07
  OP_SYSTEM            , OP_SYSTEM            , OP_INVALID           , OP_GEN_CONTROL       , //0B
  OP_INVALID           , OP_GEN_CONTROL       , OP_INVALID           , OP_INVALID           , //0F
  OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , //13
  OP_SSE               , OP_SSE               , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , //17
  OP_SSE               , OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , //1B
  OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , //1F
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //23
  OP_SYSTEM            , OP_INVALID           , OP_SYSTEM            , OP_INVALID           , //27
  OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE               , OP_SSE               , //2B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //2F
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //33
  OP_SYSTEM            , OP_SYSTEM            , OP_INVALID           , OP_INVALID           , //37
  OP_PREFIX            , OP_INVALID           , OP_PREFIX            , OP_INVALID           , //3B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //43
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //47
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //4B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //4F
  OP_SSE_DATAMOV       , OP_SSE               , OP_SSE               , OP_SSE               , //53
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //57
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //5B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //5F
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //63
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //67
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //6B
  OP_INVALID           , OP_INVALID           , OP_MMX               , OP_MMX               , //6F
  OP_SSE               , OP_MMX               , OP_MMX               , OP_MMX               , //73
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_MMX               , OP_MMX               , //7F
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //83
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //87
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //8B
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //8F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //93
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //97
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_CONTROL       , OP_GEN_BIT           , //A3
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_INVALID           , OP_INVALID           , //A7
  OP_GEN_STACK         , OP_GEN_STACK         , OP_SYSTEM            , OP_GEN_BIT           , //AB
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_STATE_MANAGEMENT  , OP_GEN_ARITH_BINARY  , //AF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_BIT           , //B3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , //B7
  OP_INVALID           , OP_GEN_CONTROL       , OP_GEN_BIT           , OP_GEN_BIT           , //BB
  OP_GEN_BIT           , OP_GEN_BIT           , OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , //BF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_SSE               , OP_SSE               , //C3
  OP_SSE               , OP_SSE               , OP_SSE               , OP_GEN_DATAMOV       , //C7
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //CB
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //CF
  OP_INVALID           , OP_MMX               , OP_MMX               , OP_MMX               , //D3
  OP_SSE               , OP_MMX               , OP_INVALID           , OP_SSE               , //D7
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //DB
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //DF
  OP_SSE               , OP_MMX               , OP_SSE               , OP_MMX               , //E3
  OP_SSE               , OP_MMX               , OP_INVALID           , OP_SSE               , //E7
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //EB
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //EF
  OP_INVALID           , OP_MMX               , OP_MMX               , OP_MMX               , //F3
  OP_SSE               , OP_MMX               , OP_SSE               , OP_SSE               , //F7
  OP_MMX               , OP_MMX               , OP_MMX               , OP_SSE               , //FB
  OP_MMX               , OP_MMX               , OP_MMX               , OP_INVALID            //FF
};

const static U8 TypeOp3_38[256] = {
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //03
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //07
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //0F
  OP_SSE               , OP_INVALID           , OP_INVALID           , OP_INVALID           , //13
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_SSE               , //17
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //1F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //23
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_INVALID           , //27
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //2B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //33
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_SSE               , //37
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //3B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //3F
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_INVALID           , //43
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //47
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //53
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //57
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //63
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //67
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //73
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //83
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //87
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //93
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //97
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_INVALID           , OP_INVALID           , //F3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //FB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID            //FF
};

const static U8 TypeOp3_3A[256] = {
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //03
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //07
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //13
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //17
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //23
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //27
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //33
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //37
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //43
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //47
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //53
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //57
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //63
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //67
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //73
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //83
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //87
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //93
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //97
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //FB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID            //FF
};

const static U8 TypeOpX[32] = {
  // escapes for F6
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_ARITH_BINARY  ,
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  ,
  // escapes for F7
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_ARITH_BINARY  ,
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  ,
  // escapes for FE
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_INVALID           , OP_INVALID           ,
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           ,
  // escapes for FF
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_BRANCH        , OP_GEN_BRANCH        ,
  OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_STACK         , OP_INVALID           
};
enum Prefixes {
  ES_OVERRIDE = 0x26,
  CS_OVERRIDE = 0x2E,
  SS_OVERRIDE = 0x36,
  DS_OVERRIDE = 0x3E,
  FS_OVERRIDE = 0x64,
  GS_OVERRIDE = 0x65,
  AD_OVERRIDE = 0x67,
  WAIT_FPU    = 0x9B,
  LOCK        = 0xF0,
  REP_N_STR   = 0xF2,
  REP_STR     = 0xF3
};

enum Opcodes {
  // 1-byte opcodes of special interest (for one reason or another)
  OP_2BYTE  = 0x0f,     // start of 2-byte opcode
  OP_OSIZE  = 0x66,     // operand size prefix
  OP_CALLF  = 0x9a,
  OP_RETNI  = 0xc2,     // ret near+immediate
  OP_RETN   = 0xc3,
  OP_ENTER  = 0xc8,
  OP_INT3   = 0xcc,
  OP_INTO   = 0xce,
  OP_CALLN  = 0xe8,
  OP_JMPF   = 0xea,
  OP_ICEBP  = 0xf1
};

enum ExeState {
  Start             =  0,
  Pref_Op_Size      =  1,
  Pref_MultiByte_Op =  2,
  ParseFlags        =  3,
  ExtraFlags        =  4,
  ReadModRM         =  5,
  Read_OP3_38       =  6,
  Read_OP3_3A       =  7,
  ReadSIB           =  8,
  Read8             =  9,
  Read16            = 10,
  Read32            = 11,
  Read8_ModRM       = 12,
  Read16_f          = 13,
  Read32_ModRM      = 14,
  Error             = 15
};

struct OpCache {
  U32 Op[CacheSize];
  U32 Index;
};

struct Instruction {
  U32 Data;
  U8 Prefix, Code, ModRM, SIB, REX, Flags, BytesRead, Size, Category;
  bool MustCheckREX, Decoding, o16, imm8;
};

#define CodeShift            3
#define CodeMask             (0xFF<<CodeShift)
#define ClearCodeMask        ((-1)^CodeMask)
#define PrefixMask           ((1<<CodeShift)-1)
#define OperandSizeOverride  (0x01<<(8+CodeShift))
#define MultiByteOpcode      (0x02<<(8+CodeShift))
#define PrefixREX            (0x04<<(8+CodeShift))
#define Prefix38             (0x08<<(8+CodeShift))
#define Prefix3A             (0x10<<(8+CodeShift))
#define HasExtraFlags        (0x20<<(8+CodeShift))
#define HasModRM             (0x40<<(8+CodeShift))
#define ModRMShift           (7+8+CodeShift)
#define SIBScaleShift        (ModRMShift+8-6)
#define RegDWordDisplacement (0x01<<(8+SIBScaleShift))
#define AddressMode          (0x02<<(8+SIBScaleShift))
#define TypeShift            (2+8+SIBScaleShift)
#define CategoryShift        5
#define CategoryMask         ((1<<CategoryShift)-1)
#define ModRM_mod            0xC0
#define ModRM_reg            0x38
#define ModRM_rm             0x07
#define SIB_scale            0xC0
#define SIB_index            0x38
#define SIB_base             0x07
#define REX_w                0x08

#define MinRequired          8 // minimum required consecutive valid instructions to be considered as code


class exeModel1: public Model {
  BlockData& x;
  Buf& buf;
  const int N1, N2;
 ContextMap2 cm;
 IndirectMap iMap;
    OpCache Cache;
    ExeState pState , State ;
    Instruction Op;
    U32 TotalOps, OpMask, OpCategMask, Context, BrkPoint , BrkCtx ;
    bool Valid;
    U32 StateBH[256];
 bool IsInvalidX64Op(U8 Op);

 bool IsValidX64Prefix(U8 Prefix);

void ProcessMode(Instruction &Op, ExeState &State);

void ProcessFlags2(Instruction &Op, ExeState &State);

void ProcessFlags(Instruction &Op, ExeState &State);

void CheckFlags(Instruction &Op, ExeState &State);

void ReadFlags(Instruction &Op, ExeState &State);

void ProcessModRM(Instruction &Op, ExeState &State);

void ApplyCodeAndSetFlag(Instruction &Op, U32 Flag = 0);

 U32 OpN(OpCache &Cache, U32 n);

 U32 OpNCateg(U32 &Mask, U32 n);

 int pref(int i);

// Get context at buf(i) relevant to parsing 32-bit x86 code
U32 execxt(int i, int x=0);
public:
  exeModel1(BlockData& bd,U32 val=0);
  int inputs() {return 2+(N1+N2)*cm.inputs();}
  int nets() {return  1024+ 1024+ 1024+ 8192+ 8192+ 8192;}
  int netcount() {return 6;}
int p(Mixer& m,int val1=0,int val2=0);


virtual ~exeModel1(){ }
};
