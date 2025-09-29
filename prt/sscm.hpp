#pragma once
#include "types.hpp"
#include "array.hpp"
#include "mixer.hpp"
/*
Map for modelling contexts of (nearly-)stationary data.
The context is looked up directly. For each bit modelled, a 16bit prediction is stored.
The adaptation rate is controlled by the caller, see mix().

- BitsOfContext: How many bits to use for each context. Higher bits are discarded.
- InputBits: How many bits [1..8] of input are to be modelled for each context.
New contexts must be set at those intervals.

Uses (2^(BitsOfContext+1))*((2^InputBits)-1) bytes of memory.
*/

class SmallStationaryContextMap {
  Array<U16> Data;
  int Context, Mask, Stride, bCount, bTotal, B;
  U16 *cp;
public:
  SmallStationaryContextMap(int BitsOfContext, int InputBits = 8);
  void set(U32 ctx);
  void Reset();
  void mix(Mixer& m, const int rate = 7, const int Multiplier = 1, const int Divisor = 4);
};
