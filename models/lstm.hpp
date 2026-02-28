#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/APM.hpp"
#include "../prt/LSTM.hpp"

// LSTM
class lstmModel1: public Model {
    BlockData& x;
    Buf& buf;
    APM apm1,apm2,apm3;
    const int horizon;
    LSTM::ByteModel *lstm;
    IndirectContext1<std::uint16_t> iCtx;
public:
    int mxcxt[2];
    lstmModel1(BlockData& bd, U32 val=0);
    int inputs() {return 2+1+1+1;}
    int p(Mixers& m, int val1=0, int val2=0);
    virtual ~lstmModel1(){ delete lstm;}
};
