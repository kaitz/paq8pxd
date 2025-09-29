#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
//#include "../prt/indirectcontext.hpp"
#include "../prt/statemap.hpp"

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


class dmcNode {
private:
struct DMCNode { // 12 bytes
private:
  // c0,c1: adaptive counts of zeroes and ones; 
  //   fixed point numbers with 4 integer and 8 fractional bits, i.e. scaling factor=256;
  //   thus the counts 0.0 .. 15.996 are represented by 0 .. 4095
  // state: bit history state - as in a contextmodel
  U32 state_c0_c1;  // 8 + 12 + 12 = 32 bits
public:
  U32 nx0,nx1;     //indexes of next DMC nodes in the state graph
  U8   get_state() const {return state_c0_c1>>24;}
  void set_state(U8 state) {state_c0_c1=(state_c0_c1 & 0x00FFFFFF)|(state<<24);}
  U32 get_c0() const {return (state_c0_c1>>12) & 0xFFF;}
  void set_c0(U32 c0) {assert(c0>=0 && c0<4096);state_c0_c1=(state_c0_c1 &0xFF000FFF) | (c0<<12);}
  U32 get_c1() const {return state_c0_c1 & 0xFFF;}
  void set_c1(U32 c1) {assert(c1>=0 && c1<4096);state_c0_c1=(state_c0_c1 &0xFFFFF000) | c1;}
};
  U32 top, curr;     // index of first unallocated node (i.e. number of allocated nodes); index of current node
  U32 threshold;     // cloning threshold parameter: fixed point number as c0,c1
  Array<DMCNode> t;  // state graph
  StateMap sm;
  BlockData& x;
  // Initialize the state graph to a bytewise order 1 model
  // See an explanation of the initial structure in:
  // http://wing.comp.nus.edu.sg/~junping/docs/njp-icita2005.pdf
  
  void resetstategraph() {
    assert(top==0 || top>65280);
    for (int i=0; i<255; ++i) { //255 nodes in each tree
      for (int j=0; j<256; ++j) { //256 trees
        int node_idx=j*255+i;
        if (i<127) { //internal tree nodes
          t[node_idx].nx0=node_idx+i+1; // left node 
          t[node_idx].nx1=node_idx+i+2; // right node
        }
        else { // 128 leaf nodes - they each references a root node of tree(i)
          t[node_idx].nx0=(i-127)*255; // left node -> root of tree 0,1,2,3,... 
          t[node_idx].nx1=(i+1)*255;   // right node -> root of tree 128,129,...
        }
        t[node_idx].set_c0(128); //0.5
        t[node_idx].set_c1(128); //0.5
        t[node_idx].set_state(0);
      }
    }
    top=65280;
    curr=0;
  }

  // helper function: adaptively increment a counter
  U32 increment_counter (const U32 x, const U32 increment) const { //"*x" is a fixed point number as c0,c1 ; "increment"  is 0 or 1
    return (((x<<4)-x)>>4)+(increment<<8); // x * (1-1/16) + increment*256
  }

  //update stategraph
  void processbit(int y) {

    U32 c0=t[curr].get_c0();
    U32 c1=t[curr].get_c1();
    const U32 n = y ==0 ? c0 : c1;

    // update counts, state
    t[curr].set_c0(increment_counter(c0,1-y));
    t[curr].set_c1(increment_counter(c1,y));

    t[curr].set_state(nex(t[curr].get_state(), y));

    // clone next state when threshold is reached
    const U32 next = y==0 ? t[curr].nx0 : t[curr].nx1;
    c0=t[next].get_c0();
    c1=t[next].get_c1();
    const U32 nn=c0+c1;
    if(n>=threshold && nn>=n+threshold && top<t.size()) {
      U32 c0_top=U64(c0)*n/nn;
      U32 c1_top=U64(c1)*n/nn;
      assert(c0>=c0_top);
      assert(c1>=c1_top);
      c0-=c0_top;
      c1-=c1_top;

      t[top].set_c0(c0_top);
      t[top].set_c1(c1_top);
      t[next].set_c0(c0);
      t[next].set_c1(c1);
      
      t[top].nx0=t[next].nx0;
      t[top].nx1=t[next].nx1;
      t[top].set_state(t[next].get_state());
      if(y==0) t[curr].nx0=top;
      else t[curr].nx1=top;
      ++top;
    }

    if(y==0) curr=t[curr].nx0;
    else     curr=t[curr].nx1;
  }

public: 
  dmcNode(U32 mem, U32 th,BlockData& bd) : top(0),threshold(th),t(mem+(255*256)),  sm(),x(bd) {resetstategraph();  }//min(mem+(255*256),((U64(1)<<31)/sizeof(DMCNode)))

  bool isfull() {return x.bpos==1 && top==t.size();}
  bool isalmostfull() {return x.bpos==1 && top>=t.size()*15 >>4;} // *15/16
  void reset() {resetstategraph();sm.Reset();}
  void mix(Mixer& m, bool activate) {
    processbit(m.x.y);
    if(activate) {
      const U32 n0=t[curr].get_c0()+1;
      const U32 n1=t[curr].get_c1()+1;
      const int pr1=(n1<<12)/(n0+n1);
      const int pr2=sm.p(t[curr].get_state(),m.x.y);
      m.add(stretch(pr1)>>2);
      m.add(stretch(pr2)>>2);
    }
  }
};

// This class solves two problems of the DMC model
// 1) The DMC model is a memory hungry algorighm. In theory it works best when it can clone
//    nodes forever. But memory is a limited resource. When the state graph is full you can't
//    clone nodes anymore. You can either i) reset the model (the state graph) and start over
//    or ii) you can keep updating the counts forever in the already fixed state graph. Both
//    choices are troublesome: i) resetting the model degrades the predictive power significantly
//    until the graph becomes large enough again and ii) a fixed structure can't adapt anymore.
//    To solve this issue:
//    Two models with the same parameters work in tandem. Always both models are updated but
//    only one model (the larger, mature one) is active (predicts) at any time. When one model
//    needs resetting the other one feeds the mixer with predictions until the first one
//    becomes mature (nearly full) again.
//    Disadvantages: with the same memory reuirements we have just half of the number of nodes
//    in each model. Also keeping two models updated at all times requires 2x as much
//    calculations as updating one model only.
//    Advantage: stable and better compression - even with reduced number of nodes.
// 2) The DMC model is sensitive to the cloning threshold parameter. Some files prefer
//    a smaller threshold other files prefer a larger threshold.
//    The difference in terms of compression is significant.
//    To solve this issue:
//    Three models with different thresholds are used and their predictions are emitted to 
//    the mixer. This way the model with the better threshold will be favored naturally.
//    Disadvantage: same as in 1) just the available number of nodes is 1/3 of the 
//    one-model case.
//    Advantage: same as in 1).

class dmcModel1: public Model {
private:
  U32 mem;
  dmcNode dmcmodel1a; // models a and b have the same parameters and work in tandem
  dmcNode dmcmodel1b;
  dmcNode dmcmodel2a; // models 1,2,3 have different threshold parameters
  dmcNode dmcmodel2b;
  dmcNode dmcmodel3a;
  dmcNode dmcmodel3b;
  int model1_state=0; // initial state, model (a) is active, both models are growing
  int model2_state=0; // model (a) is full and active, model (b) is reset and growing
  int model3_state=0; // model (b) is full and active, model (a) is reset and growing
  BlockData& x;
public:
  dmcModel1(BlockData& bd, U32 val=0);
  int inputs() {return 2;}
  int nets() {return 0;}
  int netcount() {return 0;}
  int p(Mixer& m,int val1=0,int val2=0);
  virtual ~dmcModel1(){ }
};

