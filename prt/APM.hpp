#pragma once
#include "array.hpp"
#include "types.hpp"
#include "statemap.hpp"
#include "logistic.hpp"
// An APM maps a probability and a context to a new probability.  Methods:
//
// APM a(n) creates with n contexts using 96*n bytes memory.
// a.pp(y, pr, cx, limit) updates and returns a new probability (0..4095)
//     like with StateMap.  pr (0..4095) is considered part of the context.
//     The output is computed by interpolating pr into 24 ranges nonlinearly
//     with smaller ranges near the ends.  The initial output is pr.
//     y=(0..1) is the last bit.  cx=(0..n-1) is the other context.
//     limit=(0..1023) defaults to 255.

class APM: public StateMap {
    int steps;
public:
  APM(int n,const int s=24);
  int p(int pr, int cx,const  int y,int limit=255);
};

