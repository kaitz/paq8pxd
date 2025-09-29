#pragma once
////////////////////////////// Buf /////////////////////////////

// Buf(n) buf; creates an array of n bytes (must be a power of 2).
// buf[i] returns a reference to the i'th byte with wrap (no out of bounds).
// buf(i) returns i'th byte back from pos (i > 0)
// buf.size() returns n.

class Buf {
  Array<U8> b;
public:
int pos;  // Number of input bytes in buf (is wrapped)
int poswr; //wrapp
  Buf(U32 i=0): b(i),pos(0) {poswr=i-1;}
  void setsize(int i) {
    if (!i) return;
    assert(i>0 && (i&(i-1))==0);
    poswr=i-1;
    b.resize(i);
  }
  U8& operator[](int i) {
    return b[i&(b.size()-1)];
  }
  int operator()(int i) const {
    assert(i<b.size());
    //assert(i>0);
    return b[(pos-i)&(b.size()-1)];
  }
  int size() const {
    return b.size();
  }
};

// IntBuf(n) is a buffer of n int (must be a power of 2).
// intBuf[i] returns a reference to i'th element with wrap.

class IntBuf {
  Array<int> b;
public:
  IntBuf(U32 i=0): b(i) {}
  int& operator[](U32 i) {
    return b[i&(b.size()-1)];
  }
};

class RingBuffer {
  Array<U8> b;
  U32 offset;
public:
  RingBuffer(const int i=0): b(i), offset(0) {}
  void Fill(const U8 B) {
    memset(&b[0], B, b.size());
  }
  void Add(const U8 B){
    b[offset&(b.size()-1)] = B;
    offset++;
  }
  int operator()(const int i) const {
    return b[(offset-i)&(b.size()-1)];
  }
};

