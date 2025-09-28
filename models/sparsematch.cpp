#include "sparsematch.hpp"

extern U8 level;

inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}

void SparseMatchModel::Update() {
    // update sparse hashes
    for (U32 i=0; i<NumHashes; i++) {
      hashes[i] = (i+1)*PHI;
      for (U32 j=0, k=sparse[i].offset+1; j<sparse[i].minLen; j++, k+=sparse[i].stride)
        hashes[i] = combine(hashes[i], (buffer(k)&sparse[i].bitMask)<<i);
      hashes[i]&=mask;
    }
    // extend current match, if available
    if (length) {
      index++;
      if (length<MaxLen)
        length++;
    }
    // or find a new match
    else {     
      for (int i=list.GetFirst(); i>=0; i=list.GetNext()) {
        index = Table[hashes[i]];
        if (index>0) {
          U32 offset = sparse[i].offset+1;
          while (length<sparse[i].minLen && ((buffer(offset)^buffer[index-offset])&sparse[i].bitMask)==0) {
            length++;
            offset+=sparse[i].stride;
          }
          if (length>=sparse[i].minLen) {
            length-=(sparse[i].minLen-1);
            index+=sparse[i].deletions;
            hashIndex = i;
            list.MoveToFront(i);
            break;
          }
        }
        length = index = 0;
      }
    }
    // update position information in hashtable
    for (U32 i=0; i<NumHashes; i++)
      Table[hashes[i]] = buffer.pos;
    
    expectedByte = buffer[index];
    if (valid)
      iCtx8+=x.y, iCtx16+=buffer(1);
    valid = length>1; // only predict after at least one byte following the match
    if (valid) {
      Maps[0].set(hash(expectedByte, x.c0, buffer(1), buffer(2), ilog2(length+1)*NumHashes+hashIndex));
      Maps[1].set((expectedByte<<8)|buffer(1));
      iCtx8=(buffer(1)<<8)|expectedByte, iCtx16=(buffer(1)<<8)|expectedByte;
      Maps[2].set(iCtx8());
      Maps[3].set(iCtx16());
    }
  }

SparseMatchModel::SparseMatchModel(BlockData& bd, U32 val1) :
    x(bd),buffer(bd.buf),
    Table(level>9?0x10000000:CMlimit(MEM()/2)),//?
    Maps{ {22, 1}, {17, 4}, {8, 1}, {19,1} },
    iCtx8{19,1},
    iCtx16{16,8},
    list(NumHashes),
    hashes{ 0 },
    hashIndex(0),
    length(0),
    mask(level>9?(0x10000000-1):CMlimit(MEM()/2)-1),
    expectedByte(0),
    valid(false)
  {
    //assert(ispowerof2(Size));
  }
  
  int SparseMatchModel::p(Mixer& m,int val1,int val2) {
    const U8 B = x.c0<<(8-x.bpos);
    if (x.bpos==0)
      Update();
    else if (valid) {
      U8 B = x.c0<<(8-x.bpos);
      Maps[0].set(hash(expectedByte, x.c0, buffer(1), buffer(2), ilog2(length+1)*NumHashes+hashIndex));
      if (x.bpos==4)
        Maps[1].set(0x10000|((expectedByte^U8(x.c0<<4))<<8)|buffer(1));
      iCtx8+=x.y, iCtx8=(x.bpos<<16)|(buffer(1)<<8)|(expectedByte^B);
      Maps[2].set(iCtx8());
      Maps[3].set((x.bpos<<16)|(iCtx16()^U32(B|(B<<8))));
    }

    // check if next bit matches the prediction, accounting for the required bitmask
    if (length>0 && (((expectedByte^B)&sparse[hashIndex].bitMask)>>(8-x.bpos))!=0)
      length = 0;

    if (valid) {
      if (length>1 && ((sparse[hashIndex].bitMask>>(7-x.bpos))&1)>0) {
        const int expectedBit = (expectedByte>>(7-x.bpos))&1;
        const int sign = 2*expectedBit-1;
        m.add(sign*(min(length-1, 64)<<4)); // +/- 16..1024
        m.add(sign*(1<<min(length-2, 3))*min(length-1, 8)<<4); // +/- 16..1024
        m.add(sign*512);
      }
      else {
        m.add(0); m.add(0); m.add(0);
      }

      for (int i=0;i<4;i++)
        Maps[i].mix(m);

    }
    else
      for (int i=0; i<11; i++, m.add(0));
    m.set((hashIndex<<6)|(x.bpos<<3)|min(7, length), NumHashes*64); //256
    m.set((hashIndex<<11)|(min(7, ilog2(length+1))<<8)|(x.c0^(expectedByte>>(8-x.bpos))), NumHashes*2048); //8192
    return length;
  }

