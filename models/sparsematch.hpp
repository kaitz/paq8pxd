#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/blockdata.hpp"
#include "../prt/buffers.hpp"
#include "../prt/stationarymap.hpp"
#include "../prt/indirectcontext.hpp"
#include "../prt/mft.hpp"
#include "../prt/helper.hpp"


class SparseMatchModel: public Model {
private:
    BlockData& x;
    Buf& buffer;
  enum Parameters : U32 {
    MaxLen    = 0xFFFF, // longest allowed match
    MinLen    = 3,      // default minimum required match length
    NumHashes = 4,      // number of hashes used
  };
  struct sparseConfig {
    U32 offset;//    = 0;      // number of last input bytes to ignore when searching for a match
    U32 stride ;//    = 1;      // look for a match only every stride bytes after the offset
    U32 deletions;//  = 0;      // when a match is found, ignore these many initial post-match bytes, to model deletions
    U32 minLen ;//    = MinLen;
    U32 bitMask ;//   = 0xFF;   // match every byte according to this bit mask
  };
  const sparseConfig sparse[NumHashes] = { {0,1,0,5,0xDF},{1,1,0,4,0xFF}, {0,2,0,4,0xDF}, {0,1,0,5,0x0F}};
  Array<U32> Table;
  StationaryMap Maps[4];
  IndirectContext<U8> iCtx8;
  IndirectContext<U16> iCtx16;
  MTFList list;
  U32 hashes[NumHashes];
  U32 hashIndex;   // index of hash used to find current match
  U32 length;      // rebased length of match (length=1 represents the smallest accepted match length), or 0 if no match
  U32 index;       // points to next byte of match in buffer, 0 when there is no match
  U32 mask;
  U8 expectedByte; // prediction is based on this byte (buffer[index]), valid only when length>0
  bool valid;
  void Update();
public:
    SparseMatchModel(BlockData& bd, U32 val1=0);
  int inputs() {return  4*2+3;}
   int nets() {return NumHashes*64+NumHashes*2048;}
  int netcount() {return 2;}
  int p(Mixer& m,int val1=0,int val2=0);
};
