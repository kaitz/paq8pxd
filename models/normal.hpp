#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "model.hpp"
#include "../prt/statemap.hpp"
#include "../prt/run.hpp"
#include "../prt/CM128.hpp"

static const int primes[17]={ 0, 257,251,241,239,233,229,227,223,211,199,197,193,191,181,179,173};   

class normalModel1: public Model {
    BlockData& x;
    Buf& buf;
    const int N;
    ContextMap3   cm;
    StateMap StateMaps[4];
    RunContextMap rcm9, rcm10;
    int inpt;
public:
    normalModel1(BlockData& bd, U32 val=0);
    int inputs() {return 10*cm.inputs() +3+2+2+1;}
    int p(Mixers& m, int val1=0, int val2=0);
    virtual ~normalModel1() {
    }
};
