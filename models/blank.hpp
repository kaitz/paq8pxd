#pragma once
#include "../prt/types.hpp"
#include "../prt/mixers.hpp"

//
class blankModel1: public Model {
public:
    blankModel1(BlockData& bd,U32 val=0){ }
    int inputs() {return 0;}
    inline int p(Mixers& m,int val1=0,int val2=0){  
        return 0;
    }
    virtual ~blankModel1(){}
};
