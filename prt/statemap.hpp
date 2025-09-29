#pragma once
#include "array.hpp"
#include "types.hpp"
#include "statetable.hpp"
#include <assert.h>
//////////////////////////// StateMap, APM //////////////////////////

// A StateMap maps a context to a probability.  Methods:
//
// Statemap sm(n) creates a StateMap with n contexts using 4*n bytes memory.
// sm.p(y, cx, limit) converts state cx (0..n-1) to a probability (0..4095).
//     that the next y=1, updating the previous prediction with y (0..1).
//     limit (1..1023, default 1023) is the maximum count for computing a
//     prediction.  Larger values are better for stationary sources.

class StateMap {
protected:
  const int N;  // Number of contexts
  int cxt;      // Context of last prediction
  Array<U32> t;       // cxt -> prediction in high 22 bits, count in low 10 bits
  void update(const int y, int limit);
  void update1(const int y, int limit);
  void update2(const int y, int limit);
public:
  StateMap(int n=256);
  ~StateMap();
  void Reset(int Rate=0);
  // update bit y (0..1), predict next bit in context cx
  int p(int cx,const int y,int limit=1023);
  int p1(int cx,const int y,int limit=1023);
  int p2(int cx,const int y,int limit=1023);
};


