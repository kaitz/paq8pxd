#include "sscm.hpp"
/*
Map for modelling contexts of (nearly-)stationary data.
The context is looked up directly. For each bit modelled, a 16bit prediction is stored.
The adaptation rate is controlled by the caller, see mix().

- BitsOfContext: How many bits to use for each context. Higher bits are discarded.
- InputBits: How many bits [1..8] of input are to be modelled for each context.
New contexts must be set at those intervals.

Uses (2^(BitsOfContext+1))*((2^InputBits)-1) bytes of memory.
*/

SmallStationaryContextMap::SmallStationaryContextMap(int BitsOfContext, int InputBits ) : Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), Context(0), Mask((1<<BitsOfContext)-1), Stride((1<<InputBits)-1), bCount(0), bTotal(InputBits), B(0) {
    assert(BitsOfContext<=16);
    assert(InputBits>0 && InputBits<=8);
    Reset();
    cp=&Data[0];
  }
  void SmallStationaryContextMap::set(U32 ctx) {
    Context = (ctx&Mask)*Stride;
    bCount=B=0;
  }
  void SmallStationaryContextMap::Reset() {
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=0x7FFF;
  }
  void SmallStationaryContextMap::mix(Mixer& m, const int rate, const int Multiplier, const int Divisor) {
    *cp+=((m.x.y<<16)-(*cp)+(1<<(rate-1)))>>rate;
    B+=(m.x.y && B>0);
    cp = &Data[Context+B];
    int Prediction = (*cp)>>4;
    m.add((stretch(Prediction)*Multiplier)/Divisor);
    m.add(((Prediction-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }

