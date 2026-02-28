#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "model.hpp"
#include "../prt/CM128.hpp"

class nestModel1: public Model {
    BlockData& x;
    Buf& buf;
    int ic, bc, pc,vc, qc, lvc, wc,ac, ec, uc, sense1, sense2, w;
    const int N;
    ContextMap3 cm;
public:
    int mxcxt[1];
    nestModel1(BlockData& bd, U32 val=0);
    int inputs() {return N*cm.inputs();}
    int p(Mixers& m, int val1=0, int val2=0);
    virtual ~nestModel1(){ }
};


