#pragma once
#include "enums.hpp"
#include "types.hpp"
#include "array.hpp"
#include "segment.hpp"
#include "buffers.hpp"
//#include <mem.h>
U64 MEM();
// Contain all global data usable between models
class BlockData {
public: 
//wrt
  U64 wrtpos,wrtfile,wrtsize,wrtcount,wrtdata;
  bool wrtLoaded;
  Array<U8> wrtText;
  int wrtTextSize,wrtstatus,wrtbytesize;
    Segment segment; //for file segments type size info(if not -1)
    int y; // Last bit, 0 or 1, set by encoder
    int c0; // Last 0-7 bits of the partial byte with a leading 1 bit (1-255)
    U32 c4,c8; // Last 4,4 whole bytes, packed.  Last byte is bits 0-7.
    int bpos; // bits in c0 (0 to 7)
    Buf buf;  // Rotating input queue set by Predictor
    Buf bufn;  // Rotating input queue set by Predictor
    int blpos; // Relative position in block
    int rm1;
    Filetype filetype;
    U32 b2,b3,b4,w4;
    U32 w5,f4,tt;
    U32 col;
    U32 x4,s4;
    int finfo;
    U32 fails, failz, failcount,x5;
    U32 frstchar,spafdo,spaces,spacecount, words,wordcount,wordlen,wordlen1;
    U8 grp;
    struct {
    U8 state:3;
    U8 lastPunct:5;
    U8 wordLength:4;
    U8 boolmask:4;
    U8 firstLetter;
    U8 mask;
  } Text;
  struct {
    U32 length;
    U8 byte;
    bool bypass;
    U16 bypassprediction;
    U32 length3;
  } Match;
  struct {
    struct {
      U8 WW, W, NN, N, Wp1, Np1;
    } pixels;
    U8 plane;
    U8 ctx;
  } Image;
  U64 Misses;
  U32 count;
  int wwords;
  U32 tmask;
  U64 wrtc4;
  bool dictonline;
  bool inpdf;
  int wcol;
  int utf8l,wlen;
  bool wstat,wdecoded;
  U32 pwords,pbc;
  int bc4;
  // used by SSE stage
  struct {
        std::uint8_t state; 
        std::uint8_t bcount;
      } DEC;
  struct {
        std::uint16_t state;
      } JPEG;
      bool istex,ishtml;
    Array<U64>  cxt;
    struct {
        std::uint8_t state; // used by SSE stage
      } x86_64;
BlockData();
    ~BlockData(){ }
};
