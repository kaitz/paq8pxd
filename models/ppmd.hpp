#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "model.hpp"
#include "../prt/mod_ppmd.hpp"

class ppmdModel1: public Model {
    BlockData& x;
    Buf& buf;
    ppmd_Model ppmd_12_256_1;
    ppmd_Model ppmd_6_64_2;
public:
    ppmdModel1(BlockData& bd, U32 val=0);
    int inputs() {return 2;}
    int p(Mixers& m, int val1=0, int val2=0);
    virtual ~ppmdModel1(){ }
};

