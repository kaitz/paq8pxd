#pragma once
#include "../prt/types.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/indirect.hpp"
#include "../prt/statemap.hpp"
#include "../prt/stationarymap.hpp"
#include "../prt/APM.hpp"

//////////////////////////// jpegModel /////////////////////////

// Model JPEG. Return 1 if a JPEG file is detected or else 0.
// Only the baseline and 8 bit extended Huffman coded DCT modes are
// supported.  The model partially decodes the JPEG image to provide
// context for the Huffman coded symbols.

// Print a JPEG segment at buf[p...] for debugging
/*
void dump(const char* msg, int p) {
  printf("%s:", msg);
  int len=buf[p+2]*256+buf[p+3];
  for (int i=0; i<len+2; ++i)
    printf(" %02X", buf[p+i]);
  printf("\n");
}
*/
#define finish(success){ \
  int length = buf.pos - images[idx].offset; \
  if (success && idx && buf.pos-lastPos==1) \
    /*printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bEmbedded JPEG at offset %d, size: %d bytes, level %d\nCompressing... ", images[idx].offset-buf.pos+x.blpos, length, idx), fflush(stdout); */\
  memset(&images[idx], 0, sizeof(JPEGImage)); \
  mcusize=0,dqt_state=-1; \
  idx-=(idx>0); \
  images[idx].app-=length; \
  if (images[idx].app < 0) \
    images[idx].app = 0; \
}
// Detect invalid JPEG data.  The proper response is to silently
// fall back to a non-JPEG model.
#define jassert(x) if (!(x)) { \
  /*printf("JPEG error at %d, line %d: %s\n", buf.pos, __LINE__, #x);*/ \
  if (idx>0) \
    finish(false) \
  else \
    images[idx].jpeg=0; \
  return images[idx].next_jpeg;}
// Standard Huffman tables (cf. JPEG standard section K.3)
  // IMPORTANT: these are only valid for 8-bit data precision
  const static U8 bits_dc_luminance[16] = {
    0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
  };
  const static U8 values_dc_luminance[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
  };

  const static U8 bits_dc_chrominance[16] = {
    0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
  };
  const static U8 values_dc_chrominance[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
  };

  const static U8 bits_ac_luminance[16] = {
    0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d
  };
  const static U8 values_ac_luminance[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
  };

  const static U8 bits_ac_chrominance[16] = {
    0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77
  };
  const static U8 values_ac_chrominance[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
  };
  
    const static U8 zzu[64]={  // zigzag coef -> u,v
    0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
    3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
  const static U8 zzv[64]={
    0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
    4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};
struct HUF {U32 min, max; int val;}; // Huffman decode tables
  // huf[Tc][Th][m] is the minimum, maximum+1, and pointer to codes for
  // coefficient type Tc (0=DC, 1=AC), table Th (0-3), length m+1 (m=0-15)

struct JPEGImage{
  int offset, // offset of SOI marker
  jpeg, // 1 if JPEG is header detected, 2 if image data
  next_jpeg, // updated with jpeg on next byte boundary
  app, // Bytes remaining to skip in this marker
  sof, sos, data, // pointers to buf
  htsize; // number of pointers in ht
  //mcusize, // number of coefficients in an MCU
  //linesize; // width of image in MCU
  //int hufsel[2][10];  // DC/AC, mcupos/64 -> huf decode table
  int ht[8]; // pointers to Huffman table headers
  //HUF huf[128]; // Tc*64+Th*16+m -> min, max, val
  //U8 hbuf[2048]; // Tc*1024+Th*256+hufcode -> RS
  U8 qtab[256]; // table
  int qmap[10]; // block -> table number
};


 class BHMap {
  enum {M=3};  // search limit
  Array<U8> t; // elements
  const U32 n; // size-1
 // int replaced;
 int B;
 int ncontext;
 Array<U8*> cp;
 BlockData& x;
 StateMap **sm;
 int n0,n1,iy;
 inline U8* find(U32 i) {
    int chk=(i>>24^i>>12^i)&255;
    i&=n;
    int bi=i, b=1024;  // best replacement so far
    U8 *p;
    for (int j=0; j<M; ++j) {
      p=&t[(i^j)*B];
      if (p[0]==chk) return p;  // match
      else if (p[1]==0) return p[0]=chk, p;  // empty
      else if (p[1]<b) b=p[1], bi=i^j;  // best replacement so far
    }
    //++replaced;
    p=&t[bi*B];  // replacement element
    memset(p, 0, B);
    p[0]=chk;
    return p;
  }
public:
  BHMap(U32 i,U32 b,int N, BlockData& bd): t(i*b), n(i-1),B(b),ncontext(N),cp(n), x(bd),n0(0),n1(0),iy(0) {
    assert(B>=2 && i>0 && (i&(i-1))==0); // size a power of 2? N((n+3)&-4),
    sm = new StateMap*[ncontext];

    for (int i=0; i<ncontext; i++) {
      sm[i] = new StateMap(1<<8);
    }
  }

  void update(){
      if (cp[ncontext-1]) {
           const int y=x.y;
      for (int i=0; i<ncontext; ++i)
        *cp[i]=nex(*cp[i],y);
   }
  }
 int sn0() {return n0;}
 int sn1() {return n1;}
 int siy() {return iy;}
 inline int ps(int i,U32 a,const int y){
      assert(i<ncontext);
      cp[i]=find(a)+1;  // set
      const int s=*cp[i];
      iy = int(s <= 2);
      n0=-!nex(s,2);
      n1=-!nex(s,3);
      return sm[i]->p1(s,y);
  }
  inline int p(int i,U32 a,const int y){
      assert(i<ncontext);
      cp[i]+=a;                    // add
      const int s=*cp[i];
      iy = int(s <= 2);
      n0=-!nex(s,2);
      n1=-!nex(s,3);
      return sm[i]->p1(s,y);
  }
  

  ~BHMap() {
      for (int i=0; i<ncontext; i++) {
      delete sm[i];
    }
    delete[] sm;
      // Show memory usage for debugging

 /* int empty=0, once=0;
  for (int i=1; i<t.size(); i+=B) {
    if (t[i]==0) ++empty;
    else if (t[i]<2) ++once;
  }
  printf("BH<%d> %d empty, %d once, %d replaced of %d\n", B,
    empty, once, replaced, t.size()/B);
*/
}
};

class jpegModelx: public Model {
     int MaxEmbeddedLevel ;
   JPEGImage  images[3];
   int idx;
   int lastPos;
   // State of parser
   enum {SOF0=0xc0, SOF1, SOF2, SOF3, DHT, RST0=0xd0, SOI=0xd8, EOI, SOS, DQT,
    DNL, DRI, APP0=0xe0, COM=0xfe, FF};  // Second byte of 2 byte codes
   int jpeg;  // 1 if JPEG is header detected, 2 if image data
  //static int next_jpeg=0;  // updated with jpeg on next byte boundary
   int app;  // Bytes remaining to skip in APPx or COM field
   int sof, sos, data;  // pointers to buf
   Array<int> ht;  // pointers to Huffman table headers
   int htsize;  // number of pointers in ht

  // Huffman decode state
   U32 huffcode;  // Current Huffman code including extra bits
   int huffbits;  // Number of valid bits in huffcode
   int huffsize;  // Number of bits without extra bits
   int rs;  // Decoded huffcode without extra bits.  It represents
    // 2 packed 4-bit numbers, r=run of zeros, s=number of extra bits for
    // first nonzero code.  huffcode is complete when rs >= 0.
    // rs is -1 prior to decoding incomplete huffcode.
   int mcupos;  // position in MCU (0-639).  The low 6 bits mark
    // the coefficient in zigzag scan order (0=DC, 1-63=AC).  The high
    // bits mark the block within the MCU, used to select Huffman tables.

  // Decoding tables
   Array<HUF> huf;  // Tc*64+Th*16+m -> min, max, val
   int mcusize;  // number of coefficients in an MCU
   int linesize; // width of image in MCU
   int hufsel[2][10];  // DC/AC, mcupos/64 -> huf decode table
   Array<U8> hbuf;  // Tc*1024+Th*256+hufcode -> RS

  // Image state
   Array<int> color;  // block -> component (0-3)
   Array<int> pred;  // component -> last DC value
   int dc;  // DC value of the current block
   int width;  // Image width in MCU
   int row, column;  // in MCU (column 0 to width-1)
   Buf cbuf; // Rotating buffer of coefficients, coded as:
    // DC: level shifted absolute value, low 4 bits discarded, i.e.
    //   [-1023...1024] -> [0...255].
    // AC: as an RS code: a run of R (0-15) zeros followed by an S (0-15)
    //   bit number, or 00 for end of block (in zigzag order).
    //   However if R=0, then the format is ssss11xx where ssss is S,
    //   xx is the first 2 extra bits, and the last 2 bits are 1 (since
    //   this never occurs in a valid RS code).
   int cpos;  // position in cbuf
  //static U32 huff1=0, huff2=0, huff3=0, huff4=0;  // hashes of last codes
   int rs1;//, rs2, rs3, rs4;  // last 4 RS codes
   int ssum, ssum1, ssum2, ssum3;
    // sum of S in RS codes in block and sum of S in first component

   IntBuf cbuf2;
   Array<int> adv_pred,adv_pred1,adv_pred2,adv_pred3, run_pred, sumu, sumv ;
   Array<int> ls;  // block -> distance to previous block
   Array<int> lcp, zpos;
   Array<int> blockW, blockN,/* nBlocks,*/ SamplingFactors;
    //for parsing Quantization tables
   int dqt_state , dqt_end , qnum,qnum16,qnum16b;
   Array<U8> qtab; // table
   Array<int> qmap; // block -> table number

   // Context model
   const int N,M; // size of t, number of contexts
   Array<U64> cxt;  // context hashes
   Mixer m1;
   APM apm[9];
   BlockData& x;
   Buf& buf;
   IndirectMap MJPEGMap;
   int hbcount;
   int pr0;
   int prev_coef,prev_coef2, prev_coef_rs;
   int rstpos,rstlen; // reset position
   BHMap hmap; // context hash -> bit history   
    // As a cache optimization, the context does not include the last 1-2
    // bits of huffcode if the length (huffbits) is not a multiple of 3.
    // The 7 mapped values are for context+{"", 0, 00, 01, 1, 10, 11}.
    U32 skip;
    StateMap smx;
    U32 jmiss,zux,ccount,lma,ama;
    StationaryMap Map1[66] = { 
    {16,3},{16,3},{16,3},{16,3}, {16,3},{16,3},{16,3},   {15,3},{15,3},{15,3},{15,3},{15,3},
    {15,3},{15,3},{15,3},{15,3},{15,3} ,{15,3},{15,3},{15,3} ,{15,3} ,{15,3},{15,3} ,{15,3},{15,3},
    {15,3},{15,3},{15,3},{14,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},
     {15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},
     {15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3},{15,3}
    };

public:
  jpegModelx(BlockData& bd);
  int inputs() {return 2*N+24+M;}
  int nets() {return 9 + 1025 + 1024 + 512 + 4096 + 64 + 4096 + 1024;}
  int netcount() {return 8;}
  int p(Mixer& m,int val1=0,int val2=0);
  virtual ~jpegModelx(){
   }
 
};

