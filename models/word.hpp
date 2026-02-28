#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixers.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/wrt/wrton.hpp"
#include "wordinfo.hpp"
#include "../prt/CM128.hpp"
//////////////////////////// wordModel /////////////////////////

// Model English text (words and columns/end of line)
class wordModel1: public Model {
public:
    BlockData& x;
    Buf& buf;  
private:
    int N;
    ContextMap3 cm;
    ContextMap3 cm1;
    ContextMap3 cm2;
    
    U8 pdf_text_parser_state,math_state,pre_state; // 0,1,2,3
    Info info_normal;
    Info info_pdf;
    Info math;
    Info pre;
    Info xhtml;
    U32 hq;
public:
    wordModel1(BlockData& bd, U32 val=16);
    int inputs() {return N*cm.inputs()+cm1.inputs()*7+cm2.inputs();}
    int p(Mixers& m, int val1=0, int val2=0);
    virtual ~wordModel1(){ }
};
