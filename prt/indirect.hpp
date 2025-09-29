#pragma once
#include "types.hpp"
#include "array.hpp"
#include "rnd.hpp"
#include "mixer.hpp"
#include "statemap.hpp"
#include "hash.hpp"

class IndirectMap {
  Array<U8> Data;
  StateMap Map;
  const int mask, maskbits, stride;
  int Context, bCount, bTotal, B;
  U8 *cp;
  Random rnd;
  int Limit;
  int scale;
public:
  IndirectMap(int BitsOfContext, int InputBits = 8,int s=256,int l=1023);
  void set_direct(const U32 ctx);
  void set(const U64 ctx);
  void mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 1023);
  void mix1(Mixer& m);
  void mix2(Mixer& m);
};
