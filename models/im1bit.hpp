#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/statemap.hpp"
#include "../prt/bh.hpp"
#include "../prt/largestationarymap.hpp"
//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data
class im1bitModel1: public Model {
    static constexpr int C = (1<<2)+(1<<7) + (1<<4) + (1<<4) + (1<<6) + (1<<8) + (1<<8) + (1<<8) + (1<<8) + (1<<8) + (1<<9) + (1<<10) + (1<<12) + (1<<12) + 5 + 13 + 25 + 41; //11192
    BlockData& x;
    Buf& buf;
    U32 r0, r1, r2, r3, r4, r5, r6, r7, r8;  // last 4 rows, bit 8 is over current pixel
    Array<U8> t;         // model: cxt -> state
    const int N;         // number of contexts
    Array<int> cxt;      // contexts
    Array<uint32_t> counts;
    StateMap* sm;
    //const int nLSM=5;
    LargeStationaryMap mapL[5];
    //BH<4> t1;
    //U8* cp;
      

public:
    int mxcxt[8];
    im1bitModel1(BlockData& bd, U32 val=0);
    int inputs() {return N*3+2+4+5*3;}
    int p(Mixers& m, int w=0, int val2=0);
    void add(Mixers& m, uint32_t n0, uint32_t n1);
    virtual ~im1bitModel1(){ delete[] sm;}
};

