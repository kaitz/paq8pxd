#pragma once
#include "segment.hpp"
#include "array.hpp"
#include <assert.h>
// Buffer for file segment info 
// type size info(if not -1)
class Segment {
  Array<U8> b;
public:
    U32 pos;  //size of buffer
    U64 hpos; //header pos points to segment info at archive end
    //int count; //count of segments
  Segment(int i=0);
  void setsize(int i);
    U8& operator[](U32 i) {
      if (i>=b.size()) setsize(i+1);
    return b[i];
  }
  U8 operator()(U32 i) const {
    assert(i>=0);
    assert(i<=b.size());
    return b[i];
  }
  U8 Get() {
    assert(pos>=0);
    assert(pos<=b.size());
    return b[pos++];
  }
  
  // put 8 bytes to segment buffer
  void put8(U64 num);
  void put4(U32 num);
  void put1(U8 num);
  void putdata(U8 a, U64 b, U32 c);
   int size() const {
    return b.size();
  }
};
