#pragma once
#include "types.hpp"
#include "array.hpp"
#include "mixer.hpp"
#include "statemap.hpp"
/*
Context map for large contexts (32bits).
Maps to a bit history state, a 3 MRU byte history, and 1 byte RunStats.

Bit and byte histories are stored in a hash table with 64 byte buckets.
The buckets are indexed by a context ending after 0, 2 or 5 bits of the
current byte. Thus, each byte modeled results in 3 main memory accesses
per context, with all other accesses to cache.

On a byte boundary (bit 0), only 3 of the 7 bit history states are used.
Of the remaining 4 bytes, 3 are then used to store the last bytes seen
in this context, 7 bits to store the length of consecutive occurrences of
the previously seen byte, and 1 bit to signal if more than 1 byte as been
seen in this context. The byte history is then combined with the bit history
states to provide additional states that are then mapped to predictions.
*/
/*
   0b 1 1 1 1 1 1 1 1 1   bit set activates mixer input
      | | | | | | | | |___ short run count
      | | | | | | | |_____ main run count
      | | | | | | |_______ main input 1
      | | | | | |_________ main input 2
      | | | | |___________ main input 3
      | | | |_____________ main input 4
      | | |_______________ main input 5
      | |_________________ map12
      |___________________ map6
  */
enum {       //  0b111111111
     CM_RUN1=  0b000000001,
     CM_RUN0=  0b000000010,
     CM_MAIN1= 0b000000100,
     CM_MAIN2= 0b000001000,
     CM_MAIN3= 0b000010000,
     CM_MAIN4= 0b000100000,
     CM_MAIN5= 0b001000000,
     CM_M12=   0b010000000,
     CM_M6=    0b100000000,
     CM_E1=   0b1000000000,
     CM_E2=  0b10000000000,
     CM_E3= 0b100000000000,
     CM_E4=0b1000000000000,
    CM_E5=0b10000000000000,
 CM_RUN2=0b100000000000000, // RUN2 or RUN0
  CM_MR=0b1000000000000000
};
class ContextMap2 {
  const U32 C; // max number of contexts
  class Bucket { // hash bucket, 64 bytes
    U16 Checksums[7]; // byte context checksums
    U8 MRU; // last 2 accesses (0-6) in low, high nibble
  public:
    U8 BitState[7][7]; // byte context, 3-bit context -> bit history state
                       // BitState[][0] = 1st bit, BitState[][1,2] = 2nd bit, BitState[][3..6] = 3rd bit
                       // BitState[][0] is also a replacement priority, 0 = empty
    inline U8* Find(U16 Checksum) { // Find or create hash element matching checksum.
                                    // If not found, insert or replace lowest priority (skipping 2 most recent).
      if (Checksums[MRU&15]==Checksum)
        return &BitState[MRU&15][0];
      int worst=0xFFFF, idx=0;
      for (int i=0; i<7; ++i) {
        if (Checksums[i]==Checksum)
          return MRU=MRU<<4|i, (U8*)&BitState[i][0];
        if (BitState[i][0]<worst && (MRU&15)!=i && MRU>>4!=i) {
          worst = BitState[i][0];
          idx=i;
        }
      }
      MRU = 0xF0|idx;
      Checksums[idx] = Checksum;
      return (U8*)memset(&BitState[idx][0], 0, 7);
    }
  };
  Array<Bucket, 64> Table; // bit histories for bits 0-1, 2-4, 5-7
                           // For 0-1, also contains run stats in BitState[][3] and byte history in BitState[][4..6]
  Array<U8*> BitState; // C pointers to current bit history states
  Array<U8*> BitState0; // First element of 7 element array containing BitState[i]
  Array<U8*> ByteHistory; // C pointers to run stats plus byte history, 4 bytes, [RunStats,1..3]
  Array<U32> Contexts; // C whole byte contexts (hashes)
  Array<bool> HasHistory; // True if context has a full valid byte history (i.e., seen at least 3 times)
  StateMap **Maps6b, **Maps8b, **Maps12b, **MapsRun;
  U32 index; // Next context to set by set()
  U32 bits;
  U8 lastByte, lastBit, bitPos;
  int model;  
  const int inputCount;
  const int param;
  inline void Update();
public:
  // Construct using Size bytes of memory for Count contexts
  ContextMap2(const U64 Size, const U32 Count,int mod,int m=0b110111110);
  ~ContextMap2();

   void set(U32 ctx);
   void  set() {
    set(U32(0));
}
  int mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4);
  int inputs() {return inputCount;}
};




