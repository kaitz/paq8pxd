#pragma once
#include "types.hpp"
#include "file.hpp"
#include "../predictors/predictor.hpp"

static const int SCALElog = 15;
static const int SCALE    = 1<<SCALElog;

struct Rangecoder {
  typedef unsigned char  byte;
  typedef unsigned int   uint;
  typedef unsigned long long qword;

  uint f_DEC; // 0=encode, 1=decode;
  File* f;
  byte get() { return byte(f->getc()); }
  void put( byte c ) { f->putc(c); }
  void StartEncode( File *F ) { f=F; f_DEC=0; rc_Init(); }
  void FinishEncode( void ) { rc_Quit(); }
  void StartDecode( File *F ) { f=F; f_DEC=1; rc_Init(); }

  static const uint NUM   = 32;
  static const uint sTOP  = 0x80000000U;
  static const uint Thres = 0x80000000U;

  union {
    struct {
      union {
        uint  low;  
        uint  code; 
      };
      uint  Carry;
    };
    qword lowc;
  };
  uint  FFNum;
  uint  Cache;
  uint  range;

  int   bits;

  void inpbit( void ) {
    if( (bits<<=1)>=0 ) bits = 0xFF000000 + get();
    code = code*2 + (((signed char)bits)<0);
  }

  void outbit( uint c ) {
    (bits<<=1)+=(c&1);
    if( bits>=0 ) {
      put( bits );
      bits = 0xFF000000;
    }
  }

  void rc_Init( void ) {
    low     = 0x00000000;
    Carry   = 0;    
    FFNum   = 0;
    range   = 1<<(NUM-1);
    Cache   = 0xC0000000;

    bits    = 0xFF000000;
    if( f_DEC ) {
      bits  = 0x00000000;
      for(int _=0; _<NUM-1; _++) inpbit();
    }
  }

  void rc_Quit( void ) {
    if( f_DEC==0 ) {
      uint i, n = NUM;

      // Carry=0: cache   .. FF x FFNum .. low
      // Carry=1: cache+1 .. 00 x FFNum .. low
#if 0
      qword llow=low;
      qword high=llow+range;
      qword mask=1;

      for( i=0; i<NUM; i++ ) {
        if( (llow|mask)<high ) llow|=mask,n--;
        (mask<<=1)+=1;
      }

      low = llow;
#endif
      if( (int(Cache)>=0) && ((n!=0) || (Cache^Carry!=1)) ) outbit( Cache^Carry );
      if( (n==0) && (Carry==0) ) FFNum=0; // they're also 1s
      for( i=0; i<FFNum; i++ ) outbit( Carry^1 );
      for( i=0; i<n; i++ ) outbit( low>>31 ), low<<=1;
      while( byte(bits>>24)!=0xFF ) outbit(1);
    }
  }

  uint muldivR( uint a, uint b ) { return (qword(a)*b)/range; }

  uint mulRdiv( uint a, uint c ) { return (qword(a)*range)/c; }

  uint rc_GetFreq( uint totFreq ) {
    return muldivR( code, totFreq );
  }

  uint rc_BProcess( uint freq, uint bit ) { 
    uint x[] = { 0, freq, SCALE };
    if( f_DEC ) {
      uint val = rc_GetFreq( SCALE );
      bit = (val>=freq);
    }
    rc_Process( x[bit], x[1+bit]-x[bit], SCALE );
    return bit;
  }

  void rc_Process( uint cumFreq, uint freq, uint totFreq ) {

    uint tmp  = range-mulRdiv( totFreq-cumFreq, totFreq );
    uint rnew = range-mulRdiv( totFreq-(cumFreq+freq), totFreq );

    if( f_DEC ) code-=tmp; else lowc+=tmp;
    range = rnew - tmp;

    rc_Renorm();
  }

  void rc_Renorm( void ) {
    while( range<sTOP ) ShiftStuff();
  }

  void ShiftStuff( void ) {
    range<<=1;
    if( f_DEC ) {
      inpbit();
    } else {
      ShiftLow();
    }
  }

  void ShiftLow( void ) {
    if( (int(low)>=0) || Carry ) {
      if( int(Cache)>=0 ) outbit( Cache^Carry );
      for(; FFNum!=0; FFNum-- ) outbit(Carry^1);
      Cache = 2*(Cache&0xFF000000) + (int(low)<0);
      Carry = 0;
    } else FFNum++;
    low<<=1;
  }

};



//////////////////////////// Encoder ////////////////////////////

// An Encoder does arithmetic encoding.  Methods:
// Encoder(COMPRESS, f) creates encoder for compression to archive f, which
//   must be open past any header for writing in binary mode.
// Encoder(DECOMPRESS, f) creates encoder for decompression from archive f,
//   which must be open past any header for reading in binary mode.
// code(i) in COMPRESS mode compresses bit i (0 or 1) to file f.
// code() in DECOMPRESS mode returns the next decompressed bit from file f.
//   Global y is set to the last bit coded or decoded by code().
// compress(c) in COMPRESS mode compresses one byte.
// decompress() in DECOMPRESS mode decompresses and returns one byte.
// flush() should be called exactly once after compression is done and
//   before closing f.  It does nothing in DECOMPRESS mode.
// size() returns current length of archive
// setFile(f) sets alternate source to FILE* f for decompress() in COMPRESS
//   mode (for testing transforms).
// If level (global) is 0, then data is stored without arithmetic coding.
//#include "sh_v2f.inc"
typedef enum {COMPRESS, DECOMPRESS} Mode;
class Encoder {
private:
  const Mode mode;       // Compress or decompress?
  File* archive;         // Compressed data file
  U32 x1, x2;            // Range, initially [0, 1), scaled by 2^32
  U32 x;                 // Decompress mode: last 4 input bytes of archive
  File*alt;             // decompress() source in COMPRESS mode
  Rangecoder rc; 
  // Compress bit y or return decompressed bit
  void code(int i=0);
  int decode();
 
public:
  Predictors& predictor;
  Encoder(Mode m, File* f,Predictors& predict);
  Mode getMode() const {return mode;}
  U64 size() const {return  archive->curpos();}  // length of archive so far
  void flush();  // call this when compression is finished
  void setFile(File* f) {alt=f;}

  // Compress one byte
  void compress(int c) ;

  // Decompress and return one byte
  int decompress() ;
  ~Encoder(){
  
   }
};


 
