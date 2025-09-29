#include "segment.hpp"
// Buffer for file segment info 
// type size info(if not -1)

  Segment::Segment(int i): b(i),pos(0),hpos(0)/*,count(0)*/ {}
  void Segment::setsize(int i) {
    if (!i) return;
    assert(i>0);
    b.resize(i);
  }
  // put 8 bytes to segment buffer
  void Segment::put8(U64 num){
    if ((pos+8)>=b.size()) setsize(pos+8);
    b[pos++]=(num>>56)&0xff;
    b[pos++]=(num>>48)&0xff;
    b[pos++]=(num>>40)&0xff;
    b[pos++]=(num>>32)&0xff;
    b[pos++]=(num>>24)&0xff;
    b[pos++]=(num>>16)&0xff;
    b[pos++]=(num>>8)&0xff;
    b[pos++]=num&0xff;  
  }
  void Segment::put4(U32 num){
    if ((pos+4)>=b.size()) setsize(pos+4);
    b[pos++]=(num>>24)&0xff;
    b[pos++]=(num>>16)&0xff;
    b[pos++]=(num>>8)&0xff;
    b[pos++]=num&0xff;  
  }
  void Segment::put1(U8 num){
    if (pos>=b.size()) setsize(pos+1);
    b[pos++]=num;
  }
  void Segment::putdata(U8 a, U64 b, U32 c){
      put1(a);
      put8(b);
      put4(c);
  }
 

