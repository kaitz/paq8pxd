#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/statemap.hpp"
#include "../prt/bh.hpp"
#include "../prt/largestationarymap.hpp"
#include "../prt/sscm.hpp"

//////////////////////////// im4bitModel /////////////////////////////////

// Model for 4-bit image data
class im4bitModel1: public Model {
    BlockData& x;
    Buf& buf;
    BH<16> t;
    const int S; // number of contexts
    Array<U8*> cp;
    Array<U32> cxt;
    StateMap *sm;
    StateMap map;
    U8 WWW, WW, W, NWW, NW, N, NE, NEE, NNWW, NNW, NN,NNN, NNE, NNEE;
    int col, line, run, runN, prevColor, px;
    LargeStationaryMap mapL[20];
    SmallStationaryContextMap sMap[3+2+1]{ {8,1},{8,1},{8,1},{8,1},{8,1},{8,1} };
    int nPrd;
    U8 prd[6]{0};

public:
    int mxcxt[7];
    im4bitModel1(BlockData& bd, U32 val=0);
    int inputs() {return S*3+S*3+2+nPrd*1;}
    int p(Mixers& m,int w=0,int val2=0);
    ~im4bitModel1() {
        delete [] sm;
    }

};
