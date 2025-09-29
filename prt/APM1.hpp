#pragma once
#include "array.hpp"
#include "types.hpp"
#include "blockdata.hpp"
#include "logistic.hpp"
//////////////////////////// APM1 //////////////////////////////

// APM1 maps a probability and a context into a new probability
// that bit y will next be 1.  After each guess it updates
// its state to improve future guesses.  Methods:
//
// APM1 a(N) creates with N contexts, uses 66*N bytes memory.
// a.p(pr, cx, rate=7) returned adjusted probability in context cx (0 to
//   N-1).  rate determines the learning rate (smaller = faster, default 7).
//   Probabilities are scaled 12 bits (0-4095).

class APM1 {
  int index;     // last p, context
  const int N;   // number of contexts
  Array<U16> t;  // [N][33]:  p, context -> p
  BlockData& x;
public:
  APM1(int n,BlockData& x);
  int p(int pr=2048, int cxt=0, int rate=7);
};

