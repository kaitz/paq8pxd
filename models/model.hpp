#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/aerr.hpp"
#include "../prt/mixers.hpp"

class Model {
public:
    std::vector<mparm> mxp; // model mixer parameters
    virtual int p(Mixers& m, int val1=0, int val2=0) {
    };
    virtual int inputs()=0;
    virtual void setword(U8 *w,int len=0){} ;
    virtual ~Model(){};
};
