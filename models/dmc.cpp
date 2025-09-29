#include "dmc.hpp"

extern U8 level;
//////////////////////////// dmcModel //////////////////////////

// Model using DMC (Dynamic Markov Compression).
//
// The bitwise context is represented by a state graph.
//
// See the original paper: http://webhome.cs.uvic.ca/~nigelh/Publications/DMC.pdf
// See the original DMC implementation: http://maveric0.uwaterloo.ca/ftp/dmc/
//
// Main differences:
// - Instead of floats we use fixed point arithmetic.
// - For probability estimation each state maintains both a 0,1 count ("c0" and "c1") 
//   and a bit history ("state"). The bit history is mapped to a probability adaptively using 
//   a StateMap. The two computed probabilities are emitted to the Mixer to be combined.
// - All counts are updated adaptively.
// - The "dmcModel" is used in "dmcForest". See below.



  dmcModel1::dmcModel1(BlockData& bd, U32 val):
  mem(CMlimit(( (level<11?(0x10000UL<<level):((0x10000UL<<11))))/9)),
  dmcmodel1a(mem,240,bd),
  dmcmodel1b(mem,240,bd),
  dmcmodel2a(mem,480,bd),
  dmcmodel2b(mem,480,bd),
  dmcmodel3a(mem,720,bd),
  dmcmodel3b(mem,720,bd),
  x(bd){}
  int dmcModel1::p(Mixer& m,int val1,int val2){

    switch(model1_state) {
      case 0:
        dmcmodel1a.mix(m,true);
        dmcmodel1b.mix(m,false);
        if(dmcmodel1a.isalmostfull()){dmcmodel1b.reset();model1_state++;}
        break;
      case 1:
        dmcmodel1a.mix(m, true);
        dmcmodel1b.mix(m, false);
        if(dmcmodel1a.isfull() && dmcmodel1b.isalmostfull()){dmcmodel1a.reset();model1_state++;}
        break;
      case 2:
        dmcmodel1b.mix(m,true);
        dmcmodel1a.mix(m,false);
        if(dmcmodel1b.isfull() && dmcmodel1a.isalmostfull()){dmcmodel1b.reset();model1_state--;}
        break;
    }
    
    switch(model2_state) {
    case 0:
      dmcmodel2a.mix(m,true);
      dmcmodel2b.mix(m,false);
      if(dmcmodel2a.isalmostfull()){dmcmodel2b.reset();model2_state++;}
      break;
    case 1:
      dmcmodel2a.mix(m,true);
      dmcmodel2b.mix(m,false);
      if(dmcmodel2a.isfull() && dmcmodel2b.isalmostfull()){dmcmodel2a.reset();model2_state++;}
      break;
    case 2:
      dmcmodel2b.mix(m,true);
      dmcmodel2a.mix(m,false);
      if(dmcmodel2b.isfull() && dmcmodel2a.isalmostfull()){dmcmodel2b.reset();model2_state--;}
      break;
    }

    switch(model3_state) {
    case 0:
      dmcmodel3a.mix(m,true);
      dmcmodel3b.mix(m,false);
      if(dmcmodel3a.isalmostfull()){dmcmodel3b.reset();model3_state++;}
      break;
    case 1:
      dmcmodel3a.mix(m,true);
      dmcmodel3b.mix(m,false);
      if(dmcmodel3a.isfull() && dmcmodel3b.isalmostfull()){dmcmodel3a.reset();model3_state++;}
      break;
    case 2:
      dmcmodel3b.mix(m,true);
      dmcmodel3a.mix(m,false);
      if(dmcmodel3b.isfull() && dmcmodel3a.isalmostfull()){dmcmodel3b.reset();model3_state--;}
      break;
    }
    return 0;
  }

