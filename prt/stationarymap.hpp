#pragma once
#include "types.hpp"
#include "array.hpp"
#include "mixer.hpp"
/*
  Map for modelling contexts of (nearly-)stationary data.
  The context is looked up directly. For each bit modelled, a 32bit element stores
  a 22 bit prediction and a 10 bit adaptation rate offset.

  - BitsOfContext: How many bits to use for each context. Higher bits are discarded.
  - InputBits: How many bits [1..8] of input are to be modelled for each context.
    New contexts must be set at those intervals.
  - Rate: Initial adaptation rate offset [0..1023]. Lower offsets mean faster adaptation.
    Will be increased on every occurrence until the higher bound is reached.

    Uses (2^(BitsOfContext+2))*((2^InputBits)-1) bytes of memory.
*/

class StationaryMap {
  Array<U32> Data;
  int Context, Mask, Stride, bCount, bTotal, B;
  U32 *cp;
public:
  StationaryMap(int BitsOfContext, int InputBits = 8, int Rate = 0);
  void set(U32 ctx);
    void Reset( int Rate = 0 );
  int mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 1023);
  int mix1(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 1023);
};
