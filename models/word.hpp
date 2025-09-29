#pragma once
#include "../prt/types.hpp"
//#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
//#include "../prt/stationarymap.hpp"
//#include "../prt/indirect.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/contextmap.hpp"
//#include "../prt/sscm.hpp"
//#include "../prt/ols.hpp"
#include "../prt/wrt/wrton.hpp"
//#include "../prt/stemmer/stemmer.hpp"
#include "wordinfo.hpp"
//////////////////////////// wordModel /////////////////////////

// Model English text (words and columns/end of line)
class wordModel1: public Model {
   public:
   BlockData& x;
   Buf& buf;  
private:
   int N;
   ContextMap cm;
   ContextMap cm1;
    
  U8 pdf_text_parser_state,math_state,pre_state; // 0,1,2,3
  Info info_normal;
  Info info_pdf;
  Info math;
  Info pre;
  Info xhtml;
  U32 hq;
public:
  wordModel1( BlockData& bd,U32 val=16);
   int inputs() {return N*cm.inputs()+7;}
   int nets() {return 0;}
  int netcount() {return 0;}


   int p(Mixer& m,int val1=0,int val2=0) ;
  
 virtual ~wordModel1(){ }
};
