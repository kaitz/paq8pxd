#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/CM128.hpp"

//////////////////////////// sparseModel ///////////////////////

// Model order 1-2 contexts with gaps.
class sparseModely: public Model {
    BlockData& x;
    Buf& buf;
    const int N;
    ContextMap3 cm;
    U32 ctx;
public:
    int mxcxt[1];
    sparseModely(BlockData& bd, U32 val=0);
    int inputs() {return N*cm.inputs();}
    int p(Mixers& m, int seenbefore, int howmany);
    virtual ~sparseModely(){ }
};
