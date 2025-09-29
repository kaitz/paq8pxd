#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "../prt/hash.hpp"
#include "model.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/statemap.hpp"
#include "../prt/stationarymap.hpp"
#include "../prt/sscm.hpp"


//////////////////////////// matchModel ///////////////////////////
class matchModel1: public Model {
private:
    BlockData& x;
    Buf& buffer;
    const U64 Size;
  enum Parameters : U32{
      MaxLen = 0xFFFF, // longest allowed match
    MaxExtend = 0,   // longest match expansion allowed // warning: larger value -> slowdown
    MinLen = 5,      // minimum required match length
    StepSize = 2,    // additional minimum length increase per higher order hash
    DeltaLen = 5,    // minimum length to switch to delta mode
    NumCtxs = 3,     // number of contexts used
    NumHashes = 3    // number of hashes used

  };
  Array<U32> Table;
  StateMap StateMaps[NumCtxs];
  SmallStationaryContextMap SCM[3];
  StationaryMap Maps[3];
  IndirectContext<U8> iCtx;
  U32 hashes[NumHashes];
  U32 ctx[NumCtxs];
  U32 length;    // rebased length of match (length=1 represents the smallest accepted match length), or 0 if no match
  U32 index;     // points to next byte of match in buffer, 0 when there is no match
  U32 mask;
  U8 expectedByte; // prediction is based on this byte (buffer[index]), valid only when length>0
  bool delta;
  void Update();
public:
  bool canBypass;
  virtual ~matchModel1(){ }
  int inputs() {return  2+NumCtxs+3*2+3*2+2;}
  int nets() {return 8;}
  int netcount() {return 1;}
  matchModel1(BlockData& bd, U32 val1=0);
  int p(Mixer& m,int val1=0,int val2=0) ;
  };

