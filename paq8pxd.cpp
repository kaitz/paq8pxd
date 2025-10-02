    /* paq8pxd file compressor/archiver.  Release by Kaido Orav

    Copyright (C) 2008-2025 Matt Mahoney, Serge Osnach, Alexander Ratushnyak,
    Bill Pettis, Przemyslaw Skibinski, Matthew Fite, wowtiger, Andrew Paterson,
    Jan Ondrus, Andreas Morphis, Pavel L. Holoborodko, Kaido Orav, Simon Berger,
    Neill Corlett

    LICENSE

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details at
    Visit <http://www.gnu.org/copyleft/gpl.html>.

*/
 
#define PROGNAME "paq8pxd112"  // Please change this if you change the program.

//#define MT            //uncomment for multithreading, compression only. Handled by CMake and gcc when -DMT is passed.
#ifndef DISABLE_SM
//#define SM              // For faster statemap
#endif
//#define VERBOSE         // Show extra info

#ifdef WINDOWS                       
#ifdef MT
//#define PTHREAD       //uncomment to force pthread to igore windows native threads !This seems broke!
#endif
#endif

#ifdef UNIX
#ifdef MT   
#define PTHREAD 1
#endif
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>
#include "zlib/zlib.h"

//#include <inttypes.h> // PRIi64 or 
#include <cinttypes> // PRIi64
//#define NDEBUG  // remove for debugging (turns on Array bound checks)
#include <assert.h>

#ifdef UNIX
// Not tested!
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <cstdio>
#include <ctype.h>
#include <sys/cdefs.h>
#include <dirent.h>
#include <errno.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#ifndef DEFAULT_OPTION
#define DEFAULT_OPTION 8
#endif

 
// min, max functions
/*#if  !defined(WINDOWS) || !defined (min)
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
#endif*/

#if defined(WINDOWS) || defined(_MSC_VER)
    #define atoll(S) _atoi64(S)
#endif

#ifdef _MSC_VER  
#define fseeko(a,b,c) _fseeki64(a,b,c)
#define ftello(a) _ftelli64(a)
#else
#ifndef UNIX
#ifndef fseeko
#define fseeko(a,b,c) fseeko64(a,b,c)
#endif
#ifndef ftello
#define ftello(a) ftello64(a)
#endif
#endif
#endif




#include "prt/types.hpp"

#include "prt/array.hpp"
#include "prt/helper.hpp"
#include "prt/file.hpp"
#include "prt/hash.hpp"
#include "prt/rnd.hpp"
#include "prt/buffers.hpp"
#include "prt/log.hpp"
#include "prt/enums.hpp"
#include "prt/blockdata.hpp"
#include "prt/logistic.hpp"
#include "prt/tables.hpp"

#include "prt/EAPM.hpp"
#include "prt/ESSE.hpp"

#include "predictors/predictors.hpp"
#include "predictors/predictor.hpp"
#include "predictors/predictordec.hpp"
#include "predictors/predictorjpeg.hpp"
#include "predictors/predictorexe.hpp"
#include "predictors/predictorimg4.hpp"
#include "predictors/predictorimg8.hpp"
#include "predictors/predictorimg24.hpp"
#include "predictors/predictortext.hpp"
#include "predictors/predictorimg1.hpp"
#include "predictors/predictoraudio.hpp"

#include "prt/coder.hpp"
#include "prt/job.hpp"

#include "stream/streams.hpp"

#include "filters/img24filter.hpp" 
#include "filters/img32filter.hpp" 
#include "filters/imgmrb.hpp" 
Img24Filter img24("image 24bit");
Img32Filter img32("image 32bit");
ImgMRBFilter imgmrb("image mrb");

Streams streams;
/////////////////////// Global context /////////////////////////
U8 level=DEFAULT_OPTION;  // Compression level 0 to 15
bool slow=false; //-x
bool witmode=false; //-w
bool staticd=false;  // -e
bool doExtract=false;  // -d option
bool doList=false;  // -l option
int verbose=2;

char *externaDict;
int minfq=19;
Segment segment; //for file segments type size info(if not -1)

int dt[1024];  // i -> 16K/(i+i+3)

int n0n1[256]; // for contectmap
#define PNGFlag (1<<31)
#define GrayFlag (1<<30)


/*
#include <psapi.h>
size_t getPeakMemory(){
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakPagefileUsage; // recuested peak memory /PeakWorkingSetSize used memory
#elif defined(UNIX) 
    return (size_t)0L; //not tested
#else
    return (size_t)0L;
#endif
}*/



/////////////////////////// Filters /////////////////////////////////
//
// Before compression, data is encoded in blocks with the following format:
//
//   <type> <size> <encoded-data>
//
// Type is 1 byte (type Filetype): DEFAULT=0, JPEG, EXE, ...
// Size is 4 bytes in big-endian format.
// Encoded-data decodes to <size> bytes.  The encoded size might be
// different.  Encoded data is designed to be more compressible.
//
//   void encode(File* in, File* out, int n);
//
// Reads n bytes of in (open in "rb" mode) and encodes one or
// more blocks to temporary file out (open in "wb+" mode).
// The file pointer of in is advanced n bytes.  The file pointer of
// out is positioned after the last byte written.
//
//   en.setFile(File* out);
//   int decode(Encoder& en);
//
// Decodes and returns one byte.  Input is from en.decompress(), which
// reads from out if in COMPRESS mode.  During compression, n calls
// to decode() must exactly match n bytes of in, or else it is compressed
// as type 0 without encoding.
//
//   Filetype detect(File* in, int n, Filetype type);
//
// Reads n bytes of in, and detects when the type changes to
// something else.  If it does, then the file pointer is repositioned
// to the start of the change and the new type is returned.  If the type
// does not change, then it repositions the file pointer n bytes ahead
// and returns the old type.
//
// For each type X there are the following 2 functions:
//
//   void encode_X(File* in, File* out, int n, ...);
//
// encodes n bytes from in to out.
//
//   int decode_X(Encoder& en);
//
// decodes one byte from en and returns it.  decode() and decode_X()
// maintain state information using static variables.

#define bswap(x) \
+   ((((x) & 0xff000000) >> 24) | \
+    (((x) & 0x00ff0000) >>  8) | \
+    (((x) & 0x0000ff00) <<  8) | \
+    (((x) & 0x000000ff) << 24))

#define IMG_DET(type,start_pos,header_len,width,height) return dett=(type),\
deth=int(header_len),detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),HDR
#define IMG_DETP(type,start_pos,header_len,width,height) return dett=(type),\
deth=int(header_len),detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),TEXT
 
#define DBS_DET(type,start_pos,header_len,datalen,reclen) return dett=(type),\
deth=int(header_len),detd=int(datalen),info=int(reclen),\
 in->setpos(start+(start_pos)),HDR

#define IMG_DETX(type,start_pos,header_len,width,height) return dett=(type),\
deth=-1,detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),DEFAULT

#define AUD_DET(type,start_pos,header_len,data_len,wmode) return dett=(type),\
deth=int(header_len),detd=(data_len),info=(wmode),\
 in->setpos(start+(start_pos)),HDR

//Return only base64 data. No HDR.
#define B64_DET(type,start_pos,header_len,base64len) return dett=(type),\
deth=(-1),detd=int(base64len),\
 in->setpos(start+start_pos),DEFAULT
#define UUU_DET(type,start_pos,header_len,base64len,is96) return dett=(type),\
deth=(-1),detd=int(base64len),info=(is96),\
 in->setpos(start+start_pos),DEFAULT
 
#define B85_DET(type,start_pos,header_len,base85len) return dett=(type),\
deth=(-1),detd=int(base85len),\
 in->setpos(start+start_pos),DEFAULT

#define SZ_DET(type,start_pos,header_len,base64len,unsize) return dett=(type),\
deth=(-1),detd=int(base64len),info=(unsize),\
 in->setpos(start+start_pos),DEFAULT

#define MRBRLE_DET(type,start_pos,header_len,data_len,width,height) return dett=(type),\
deth=(header_len),detd=(data_len),info=(((width+3)/4)*4),info2=(height),\
 in->setpos(start+(start_pos)),HDR

#define TIFFJPEG_DET(start_pos,header_len,data_len) return dett=(JPEG),\
deth=(header_len),detd=(data_len),info=(-1),info2=(-1),\
 in->setpos(start+(start_pos)),HDR

#define NES_DET(type,start_pos,header_len,base64len) return dett=(type),\
deth=(-1),detd=int(base64len),\
 in->setpos(start+start_pos),DEFAULT

inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c=='+') || (c=='/')|| (c==10) || (c==13));
}

inline bool is_base85(unsigned char c) {
    return (isalnum(c) || (c==13) || (c==10) || (c=='y') || (c=='z') || (c>='!' && c<='u'));
}
// Function ecc_compute(), edc_compute() and eccedc_init() taken from 
// ** UNECM - Decoder for ECM (Error Code Modeler) format.
// ** Version 1.0
// ** Copyright (C) 2002 Neill Corlett

/* LUTs used for computing ECC/EDC */
static U8 ecc_f_lut[256];
static U8 ecc_b_lut[256];
static U32 edc_lut[256];
static int luts_init=0;

void eccedc_init(void) {
  if (luts_init) return;
  U32 i, j, edc;
  for(i = 0; i < 256; i++) {
    j = (i << 1) ^ (i & 0x80 ? 0x11D : 0);
    ecc_f_lut[i] = j;
    ecc_b_lut[i ^ j] = i;
    edc = i;
    for(j = 0; j < 8; j++) edc = (edc >> 1) ^ (edc & 1 ? 0xD8018001 : 0);
    edc_lut[i] = edc;
  }
  luts_init=1;
}

void ecc_compute(U8 *src, U32 major_count, U32 minor_count, U32 major_mult, U32 minor_inc, U8 *dest) {
  U32 size = major_count * minor_count;
  U32 major, minor;
  for(major = 0; major < major_count; major++) {
    U32 index = (major >> 1) * major_mult + (major & 1);
    U8 ecc_a = 0;
    U8 ecc_b = 0;
    for(minor = 0; minor < minor_count; minor++) {
      U8 temp = src[index];
      index += minor_inc;
      if(index >= size) index -= size;
      ecc_a ^= temp;
      ecc_b ^= temp;
      ecc_a = ecc_f_lut[ecc_a];
    }
    ecc_a = ecc_b_lut[ecc_f_lut[ecc_a] ^ ecc_b];
    dest[major              ] = ecc_a;
    dest[major + major_count] = ecc_a ^ ecc_b;
  }
}

U32 edc_compute(const U8  *src, int size) {
  U32 edc = 0;
  while(size--) edc = (edc >> 8) ^ edc_lut[(edc ^ (*src++)) & 0xFF];
  return edc;
}

int expand_cd_sector(U8 *data, int a, int test) {
  U8 d2[2352];
  eccedc_init();
  d2[0]=d2[11]=0;
  for (int i=1; i<11; i++) d2[i]=255;
  int mode=(data[15]!=1?2:1);
  int form=(data[15]==3?2:1);
  if (a==-1) for (int i=12; i<15; i++) d2[i]=data[i]; else {
    int c1=(a&15)+((a>>4)&15)*10;
    int c2=((a>>8)&15)+((a>>12)&15)*10;
    int c3=((a>>16)&15)+((a>>20)&15)*10;
    c1=(c1+1)%75;
    if (c1==0) {
      c2=(c2+1)%60;
      if (c2==0) c3++;
    }
    d2[12]=(c3%10)+16*(c3/10);
    d2[13]=(c2%10)+16*(c2/10);
    d2[14]=(c1%10)+16*(c1/10);
  }
  d2[15]=mode;
  if (mode==2) for (int i=16; i<24; i++) d2[i]=data[i-4*(i>=20)];
  if (form==1) {
    if (mode==2) {
      d2[1]=d2[12],d2[2]=d2[13],d2[3]=d2[14];
      d2[12]=d2[13]=d2[14]=d2[15]=0;
    } else {
      for(int i=2068; i<2076; i++) d2[i]=0;
    }
    for (int i=16+8*(mode==2); i<2064+8*(mode==2); i++) d2[i]=data[i];
    U32 edc=edc_compute(d2+16*(mode==2), 2064-8*(mode==2));
    for (int i=0; i<4; i++) d2[2064+8*(mode==2)+i]=(edc>>(8*i))&0xff;
    ecc_compute(d2+12, 86, 24,  2, 86, d2+2076);
    ecc_compute(d2+12, 52, 43, 86, 88, d2+2248);
    if (mode==2) {
      d2[12]=d2[1],d2[13]=d2[2],d2[14]=d2[3],d2[15]=2;
      d2[1]=d2[2]=d2[3]=255;
    }
  }
  for (int i=0; i<2352; i++) if (d2[i]!=data[i] && test) form=2;
  if (form==2) {
    for (int i=24; i<2348; i++) d2[i]=data[i];
    U32 edc=edc_compute(d2+16, 2332);
    for (int i=0; i<4; i++) d2[2348+i]=(edc>>(8*i))&0xff;
  }
  for (int i=0; i<2352; i++) if (d2[i]!=data[i] && test) return 0; else data[i]=d2[i];
  return mode+form-1;
}

//LZSS compressor/decompressor class
//http://my.execpc.com/~geezer/code/lzss.c
class LZSS {
    private:
    const U32 N;                // size of ring buffer
    const U32 F;                // upper limit for g_match_len.
                                // 16 for compatibility with Microsoft COMPRESS.EXE and EXPAND.EXE
    const U32 THRESHOLD;        // encode string into position and length if match_length is greater than this
    U32  NIL;                   // index for root of binary search trees
    Array<U8> LZringbuffer;     // ring buffer of size N, with extra F-1 bytes
                                // to facilitate string comparison
    U32 matchpos;               // position and length of longest match; set by insert_node()
    U32 matchlen;
    Array<U32> LZ_lchild;       // left & right children & parent -- these constitute binary search tree
    Array<U32> LZ_rchild;
    Array<U32> LZ_parent;
    File*g_infile, *g_outfile; //input and output file to be compressed
    U32 filesizez;

    // Inserts string of length F, LZringbuffer[r..r+F-1], into one of the
    // trees (LZringbuffer[r]'th tree) and returns the longest-match position
    // and length via the global variables matchpos and matchlen.
    // If matchlen = F, then removes the old node in favour of the new
    // one, because the old one will be deleted sooner.
    // Note r plays double role, as tree node and position in buffer.
void insert_node(int r){
    U8 *key;
    U32 i, p;
    int cmp;
    cmp = 1;
    key = &LZringbuffer[r];
    p=N+1+key[0];
    LZ_rchild[r]=LZ_lchild[r]=NIL;
    matchlen = 0;
    while(1){
        if(cmp>= 0){
            if(LZ_rchild[p]!=NIL) p=LZ_rchild[p];
            else{
                LZ_rchild[p]=r;
                LZ_parent[r]=p;
                return;
            }
        }
        else{
            if(LZ_lchild[p]!=NIL) p=LZ_lchild[p];
            else{
                LZ_lchild[p]=r;
                LZ_parent[r]=p;
                return;
            }
        }
        for(i=1;i<F;i++){
            cmp=key[i]-LZringbuffer[p+i];
            if(cmp != 0) break;
        }
        if(i>matchlen){
            matchpos=p;
            matchlen=i;
            if(matchlen>=F) break;
        }
    }
    LZ_parent[r]=LZ_parent[p];
    LZ_lchild[r]=LZ_lchild[p];
    LZ_rchild[r]=LZ_rchild[p];
    LZ_parent[LZ_lchild[p]]=r;
    LZ_parent[LZ_rchild[p]]=r;
    if(LZ_rchild[LZ_parent[p]]==p) LZ_rchild[LZ_parent[p]]=r;
    else LZ_lchild[LZ_parent[p]]=r;
    LZ_parent[p]=NIL;                   // remove p
}

//deletes node p from tree
void delete_node(unsigned p){
    U32 q;
    if(LZ_parent[p]==NIL) return;       // not in tree
    if(LZ_rchild[p]==NIL) q=LZ_lchild[p];
    else if(LZ_lchild[p]==NIL) q=LZ_rchild[p];
    else{
        q=LZ_lchild[p];
        if(LZ_rchild[q]!=NIL){
            do q=LZ_rchild[q];
            while(LZ_rchild[q]!=NIL);
            LZ_rchild[LZ_parent[q]]=LZ_lchild[q];
            LZ_parent[LZ_lchild[q]]=LZ_parent[q];
            LZ_lchild[q]=LZ_lchild[p];
            LZ_parent[LZ_lchild[p]]=q;
        }
        LZ_rchild[q]=LZ_rchild[p];
        LZ_parent[LZ_rchild[p]]=q;
    }
    LZ_parent[q] = LZ_parent[p];
    if(LZ_rchild[LZ_parent[p]]==p) LZ_rchild[LZ_parent[p]]=q;
    else LZ_lchild[LZ_parent[p]] = q;
    LZ_parent[p]=NIL;
}
public:
    U32 usize;
    LZSS(File*in, File* out,U32 fsize,U32 qn);

//may fail when compressed size is larger the input (uncompressible data)
U32 compress(){
    U32 i, len, r, s, last_match_length, code_buf_ptr;
    U8 code_buf[17], mask;
    U32 ocount;
    int c;
    // code_buf[1..16] saves eight units of code, and code_buf[0] works as
    // eight flags, "1" representing that the unit is an unencoded letter (1 byte),
    // "0" a position-and-length pair (2 bytes). Thus, eight units require at most
    // 16 bytes of code.
    ocount=0;
    code_buf[0]=0;
    code_buf_ptr=mask=1;
    s=0;
    r=N-F;
    // Clear the buffer with any character that will appear often.
    memset(&LZringbuffer[0]+s,' ',r-s);
    // Read F bytes into the last F bytes of the buffer
    for(len=0;len<F;len++){
        c=g_infile->getc();
        if(c==EOF)break;
        LZringbuffer[r+len]=c;
    }
    if(len==0) return 0; //text of size zero
    // Insert the F strings, each of which begins with one or more 'space'
    // characters. Note the order in which these strings are inserted.
    // This way, degenerate trees will be less likely to occur.
    for(i=1; i<=F;i++) insert_node(r-i);
    // Finally, insert the whole string just read. The global variables
    // matchlen and matchpos are set.
    insert_node(r);
    do{
        // matchlen may be spuriously long near the end of text.
        if(matchlen>len) matchlen=len;
        if(matchlen<=THRESHOLD){            // Not long enough match. Send one byte.
            matchlen=1;
            code_buf[0]|=mask;              // 'send one byte' flag 
            code_buf[code_buf_ptr]=LZringbuffer[r];  // Send uncoded.
            code_buf_ptr++;
        }
        else{                               // Send position and length pair. Note matchlen > THRESHOLD.
            code_buf[code_buf_ptr]=(U8)matchpos;
            code_buf_ptr++;
            code_buf[code_buf_ptr]=(U8)(((matchpos>>4)&0xF0)|(matchlen-(THRESHOLD+1)));
            code_buf_ptr++;
        }
        mask<<=1;                           // Shift mask left one bit.
        if(mask==0){                        // Send at most 8 units of code together
            for(i=0;i<code_buf_ptr;i++){
                g_outfile->putc(code_buf[i]),ocount++;
                if(ocount>=filesizez) return ocount;
            }
            code_buf[0]=0;
            code_buf_ptr=mask=1;
        }
        last_match_length=matchlen;
        for(i=0;i<last_match_length;i++){
            c=g_infile->getc();
            if(c==EOF) break;
            delete_node(s);                 // Delete old strings and read new bytes
            LZringbuffer[s] = c;
            // If the position is near the end of buffer, extend the buffer
            // to make string comparison easier.
            // Since this is a ring buffer, increment the position modulo N.
            // Register the string in LZringbuffer[r..r+F-1] 
            if(s<F-1) LZringbuffer[s+N]=c;
            s=(s+1)&(N-1);
            r=(r+1)&(N-1);
            insert_node(r);
        }
        while(i++<last_match_length){       // After the end of text,
            delete_node(s);                 // no need to read, but
            s=(s+1)&(N-1);
            r=(r+1)&(N-1);
            len--;
            if(len) insert_node(r);         // buffer may not be empty.
        }
    } while(len>0);                         //until length of string to be processed is zero
    if(code_buf_ptr>1){                     // Send remaining code.
        for(i=0;i<code_buf_ptr;i++){
            g_outfile->putc(code_buf[i]),ocount++;
            if(ocount>=filesizez) return ocount;
        }
    }
    return ocount;    //return compressed size
}

U32 decompress(){
    U32 r, flags;
    int i,c, j, k;
    U32 icount,incount;
    icount=incount=0;
    memset(&LZringbuffer[0],' ',N-F);
    r = N - F;
    for(flags=0;;flags>>=1){
    // Get a byte. For each bit of this byte:
    // 1=copy one byte literally, from input to output
    // 0=get two more bytes describing length and position of previously-seen
    // data, and copy that data from the ring buffer to output
        if((flags&0x100)==0){
            c=g_infile->getc(),incount++;
            if(c==EOF||icount>=filesizez) break;
            flags=c|0xFF00;
        }
        if(flags & 1){
            c=g_infile->getc(),incount++;
            if(c==EOF||icount>=filesizez) break;
            g_outfile->putc(c),icount++;
            LZringbuffer[r]=c;
            r=(r+1)&(N-1);
        }
        // 0=get two more bytes describing length and position of previously-
        // seen data, and copy that data from the ring buffer to output
        else{
            i=g_infile->getc(),incount++;
            if(i==EOF||icount>=filesizez) break;
            j=g_infile->getc(),incount++;
            if(j==EOF ||icount>=filesizez) break;
            i|=((j&0xF0)<< 4);
            j=(j&0x0F)+THRESHOLD;
            for(k=0;k<=j;k++){
                c=LZringbuffer[(i+k)&(N-1)];
                g_outfile->putc(c),icount++;
                LZringbuffer[r]=c;
                r=(r+1)&(N-1);
            }
        }
    }
    usize=icount;       //decompressed size
    return incount-1;   //return compressed size
}
};
LZSS::LZSS(File*in, File* out,U32 fsize,U32 qn=0): N(4096),F(16+qn),THRESHOLD(2),NIL(N),
LZringbuffer(N+F-1), LZ_lchild(N+1), LZ_rchild(N+257), LZ_parent(N+1),filesizez(fsize),usize(0){
    g_infile=in, g_outfile=out;
    // initialize trees
    // For i = 0 to N - 1, LZ_rchild[i] and LZ_lchild[i] will be the right and
    // left children of node i. These nodes need not be initialized.
    // Also, LZ_parent[i] is the parent of node i. These are initialized to
    // NIL (= N), which stands for 'not used.'
    // For i = 0 to 255, LZ_rchild[N + i + 1] is the root of the tree
    // for strings that begin with character i. These are initialized
    // to NIL.  Note there are 256 trees.
    for(U32 i=N+1;i<=N+256;i++)
        LZ_rchild[i]=NIL;
    for(U32 i=0;i<N;i++)
        LZ_parent[i]=NIL;
}

//read compressed word,dword
U32 GetCDWord(File*f){
    U16 w = f->getc();
    w=w | (f->getc()<<8);
    if(w&1){
        U16 w1 = f->getc();
        w1=w1 | (f->getc()<<8);
        return ((w1<<16)|w)>>1;
    }
    return w>>1;
}
U8 GetCWord(File*f){
    U8 b=f->getc();
    if(b&1) return ((f->getc()<<8)|b)>>1;
    return b>>1;
}

int parse_zlib_header(int header) {
    switch (header) {
        case 0x2815 : return 0;  case 0x2853 : return 1;  case 0x2891 : return 2;  case 0x28cf : return 3;
        case 0x3811 : return 4;  case 0x384f : return 5;  case 0x388d : return 6;  case 0x38cb : return 7;
        case 0x480d : return 8;  case 0x484b : return 9;  case 0x4889 : return 10; case 0x48c7 : return 11;
        case 0x5809 : return 12; case 0x5847 : return 13; case 0x5885 : return 14; case 0x58c3 : return 15;
        case 0x6805 : return 16; case 0x6843 : return 17; case 0x6881 : return 18; case 0x68de : return 19;
        case 0x7801 : return 20; case 0x785e : return 21; case 0x789c : return 22; case 0x78da : return 23;
    }
    return -1;
}
int zlib_inflateInit(z_streamp strm, int zh) {
    if (zh==-1) return inflateInit2(strm, -MAX_WBITS); else return inflateInit(strm);
}


bool IsGrayscalePalette(File* in, int n = 256, int isRGBA = 0){
  U64 offset = in->curpos();
  int stride = 3+isRGBA, res = (n>0)<<8, order=1;
  for (int i = 0; (i < n*stride) && (res>>8); i++) {
    int b = in->getc();
    if (b==EOF){
      res = 0;
      break;
    }
    if (!i) {
      res = 0x100|b;
      order = 1-2*(b>int(ilog2(n)/4));
      continue;
    }

    //"j" is the index of the current byte in this color entry
    int j = i%stride;
    if (!j){
      // load first component of this entry
      int k = (b-(res&0xFF))*order;
      res = res&((k>=0 && k<=8)<<8);
      res|=(res)?b:0;
    }
    else if (j==3)
      res&=((!b || (b==0xFF))*0x1FF); // alpha/attribute component must be zero or 0xFF
    else
      res&=((b==(res&0xFF))*0x1FF);
  }
   in->setpos( offset);
  return (res>>8)>0;
}

#define base64max 0x8000000 //128M limit
#define base85max 0x8000000 //128M limit

struct TARheader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char major[8];
    char minor[8];
    char pad[167];
};
int getoct(const char *p, int n){
    int i = 0;
    while (*p<'0' || *p>'7') ++p, --n;
    while (*p>='0' && *p<='7' && n>0) {
        i*=8;
        i+=*p-'0';
        ++p,--n;
    }
    return i;
}
int tarchecksum(char *p){
    int u=0;
    for (int n = 0; n < 512; ++n) {
        if (n<148 || n>155) u+=((U8 *)p)[n];
        else u += 0x20;
    }
    return (u==getoct(p+148,8));
}
bool tarend(const char *p){
    for (int n=511; n>=0; --n) if (p[n] != '\0') return false;
    return true;
}
struct dBASE {
  U8 Version;
  U32 nRecords;
  U16 RecordLength, HeaderLength;
  int Start, End;
};

struct dTIFF {
  U32 size;
  U32 offset;
  U8 compression;
  U32 width;
  U32 height;
  U8 bits;
  U8 bits1;
};

#define MIN_TEXT_SIZE 0x400 //1KB
#define MAX_TEXT_MISSES 3 //number of misses in last 32 bytes before resetting
struct TextInfo {
  U64 start;
  U32 lastSpace;
  U32 lastNL;
  U64 lastNLpos;
  U32 wordLength;
  U32 misses;
  U32 missCount;
  U32 zeroRun;
  U32 spaceRun;
  U32 countNL;
  U32 totalNL;
  U32 countLetters;
  U32 countNumbers;
  U32 countUTF8;
  bool isLetter, isUTF8, needsEolTransform, seenNL, isNumbertext;
};
struct bmpInfo {
U64 bmp;
  int bpp,
  x,
  y,
  of,
  size,
  hdrless; 
};  
struct gifInfo {
U64 gif,
    a,
  i,
  w,
  c,
  b,
  plt,
  gray  ; 
};  

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12
static const U8 utf8_state_table[] = {
  // byte -> character class
  // character_class = utf8_state_table[byte]
  1,1,1,1,1,0,1,1,1,0,0,1,1,0,1,1,  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 00..1f  
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
 10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8, // e0..ff
  // validating automaton
  // new_state = utf8_state_table[256*old_state + character_class]
   0,12,24,36,60,96,84,12,12,12,48,72, // state  0-11
  12,12,12,12,12,12,12,12,12,12,12,12, // state 12-23
  12, 0,12,12,12,12,12, 0,12, 0,12,12, // state 24-35
  12,24,12,12,12,12,12,24,12,24,12,12, // state 36-47
  12,12,12,12,12,12,12,24,12,12,12,12, // state 48-59
  12,24,12,12,12,12,12,12,12,24,12,12, // state 60-71
  12,12,12,12,12,12,12,36,12,36,12,12, // state 72-83
  12,36,12,12,12,12,12,36,12,36,12,12, // state 84-95
  12,36,12,12,12,12,12,12,12,12,12,12  // state 96-108
};

#define TEXT_MIN_SIZE 1024*64   // size of minimum allowed text block (in bytes)
#define TEXT_MAX_MISSES 6    // threshold: max allowed number of invalid UTF8 sequences seen recently before reporting "fail"
#define TEXT_ADAPT_RATE 256  // smaller (like 32) = illegal sequences are allowed to come more often, larger (like 1024) = more rigorous detection

struct TextParserStateInfo {
  Array<U64> _start;
  Array<U64> _end;      // position of last char with a valid UTF8 state: marks the end of the detected TEXT block
  Array<U32> _EOLType;  // 0: none or CR-only;   1: CRLF-only (applicable to EOL transform);   2: mixed or LF-only
  Array<U32> _number;  // 0: none or CR-only;   1: CRLF-only (applicable to EOL transform);   2: mixed or LF-only
  U32 invalidCount;     // adaptive count of invalid UTF8 sequences seen recently
  U32 UTF8State;        // state of utf8 parser; 0: valid;  12: invalid;  any other value: yet uncertain (more bytes must be read)
  U32 countUTF8;
  TextParserStateInfo(): _start(0), _end(0), _EOLType(0),_number(0) {}
  void reset(U64 startpos) {
    _start.resize(1);
    _end.resize(1);
    _start[0]=startpos;
    _end[0]=startpos-1;
    _EOLType.resize(1);
    _number.resize(1);
    _EOLType[0]=0;
    _number[0]=0;
    invalidCount=0;
    UTF8State=0;
    countUTF8=0;
  }
  U64 start(){return _start[_start.size()-1];}
  U64 end(){return _end[_end.size()-1];}
  void setend(U64 end){_end[_end.size()-1]=end;}
  U32 EOLType(){return _EOLType[_EOLType.size()-1];}
  void setEOLType(U32 EOLType){_EOLType[_EOLType.size()-1]=EOLType;}
  U32 number(){return _number[_number.size()-1];}
  void set_number(U32 number){_number[_number.size()-1]=number;}
  U64 validlength(){return end() - start() + 1;}
  void next(U64 startpos) {
    _start.push_back(startpos);
    _end.push_back(startpos-1);
    _EOLType.push_back(0);
    _number.push_back(0);
    invalidCount=0;
    UTF8State=0;
    countUTF8=0;
  }
  void removefirst() {
    if(_start.size()==1)
      reset(0);
    else {
      for(int i=0;i<(int)_start.size()-1;i++){
        _start[i]=_start[i+1];
        _end[i]=_end[i+1];
        _EOLType[i]=_EOLType[i+1];
        _number[i]=_number[i+1];
      }
      _start.pop_back();
      _end.pop_back();
      _EOLType.pop_back();
      _number.pop_back();
    }
  }
} textparser;
void printStatus1(U64 n, U64 size) {
fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}

#include "bzip2/bzlib.h"
#define BZ2BLOCK 100*1024*100

U64 bzip2compress(File* im, File* out,int level, U64 size) {
  bz_stream stream;
  Array<char> bzin(BZ2BLOCK);
  Array<char> bzout(BZ2BLOCK);
  stream.bzalloc=NULL;
  stream.bzfree=NULL;
  stream.opaque=NULL;
  stream.next_in=NULL;
  stream.avail_in=0U;
  stream.avail_out=0U;
  U64 p=0,usize=size;
  int part,ret,status;
  ret=BZ2_bzCompressInit(&stream, level&255, 0, 0,level/256);
  if (ret!=BZ_OK) return ret;  
  do {
    stream.avail_in =im->blockread((U8*) &bzin[0], min(BZ2BLOCK,usize));
    status = usize<BZ2BLOCK?BZ_FINISH:BZ_RUN;
    usize=usize-stream.avail_in;
    stream.next_in=(char*) &bzin[0] ;
    do {
      stream.avail_out=BZ2BLOCK;
      stream.next_out=(char*)&bzout[0] ;
      ret=BZ2_bzCompress(&stream, status);
      part=BZ2BLOCK-stream.avail_out;
      if (part>0) p+=part,out->blockwrite((U8*) &bzout[0],part);
    } while (stream.avail_in != 0);

  }  while (status!=BZ_FINISH);
  (void)BZ2_bzCompressEnd(&stream);
  return p;
}
U64 bzip2decompress(File* in, File* out, int compression_level, U64& csize, bool save=true) {
  bz_stream stream;
  Array<char> bzin(BZ2BLOCK);
  Array<char> bzout(BZ2BLOCK);
  stream.bzalloc=NULL;
  stream.bzfree=NULL;
  stream.opaque=NULL;
  stream.avail_in=0;
  stream.next_in=NULL;
  U64 dsize=0;
  int inbytes,part,ret;
  int blockz=csize?csize:BZ2BLOCK;
  csize = 0;
  ret = BZ2_bzDecompressInit(&stream, 0, 0);
  if (ret!=BZ_OK) return ret;
  do {
    stream.avail_in=in->blockread((U8*) &bzin[0], min(BZ2BLOCK,blockz));
    inbytes=stream.avail_in;
    if (stream.avail_in==0) break;
    stream.next_in=(char*)&bzin[0];
    do {
      stream.avail_out=BZ2BLOCK;
      stream.next_out=(char*)&bzout[0];
      ret=BZ2_bzDecompress(&stream);
      if ((ret!=BZ_OK) && (ret!=BZ_STREAM_END)) {
        (void)BZ2_bzDecompressEnd(&stream);
        return ret;
      }
      csize+=(inbytes-stream.avail_in);
      inbytes=stream.avail_in;
      part=BZ2BLOCK-stream.avail_out;
      if (save==true)out->blockwrite((U8*) &bzout[0], part);
      dsize+=part;

    } while (stream.avail_out == 0);
  } while (ret != BZ_STREAM_END);
  (void)BZ2_bzDecompressEnd(&stream);
  if (ret == BZ_STREAM_END) return dsize; else return 0;
}

Filetype detect(File* in, U64 n, Filetype type, int &info, int &info2, int it=0) {
  U32 buf4=0,buf3=0, buf2=0, buf1=0, buf0=0;  // last 8 bytes
  static U64 start=0;
  static U64 prv_start=0;
  prv_start = start;    // for DEC Alpha detection
  start= in->curpos();
  // For EXE detection
  Array<U64> abspos(256),  // CALL/JMP abs. addr. low byte -> last offset
    relpos(256);    // CALL/JMP relative addr. low byte -> last offset
  int e8e9count=0;  // number of consecutive CALL/JMPs
  U64 e8e9pos=0;    // offset of first CALL or JMP instruction
  U64 e8e9last=0;   // offset of most recent CALL or JMP
  // For ARM detection
  Array<U64> absposARM(256),  // CALL/JMP abs. addr. low byte -> last offset
    relposARM(256);    // CALL/JMP relative addr. low byte -> last offset
  int ARMcount=0;  // number of consecutive CALL/JMPs
  U64 ARMpos=0;    // offset of first CALL or JMP instruction
  U64 ARMlast=0;   // offset of most recent CALL or JMP

  U64 soi=0, sof=0, sos=0, app=0,eoi=0;  // For JPEG detection - position where found
  U64 wavi=0,wavlist=0;
  int wavsize=0,wavch=0,wavbps=0,wavm=0,wavsr=0,wavt=0,wavtype=0,wavlen=0;  // For WAVE detection
  U64 aiff=0;
  int aiffm=0,aiffs=0;  // For AIFF detection
  U64 s3mi=0;
  int s3mno=0,s3mni=0;  // For S3M detection
  bmpInfo bmp = {};    // For BMP detection
  U64 rgbi=0;
  int rgbx=0,rgby=0;  // For RGB detection
  U64 tga=0;
  U64 tgax=0;
  int tgay=0,tgaz=0,tgat=0,tgaid=0,tgamap=0;  // For TGA detection
  U64 pgm=0;
  int pgmcomment=0,pgmw=0,pgmh=0,pgm_ptr=0,pgmc=0,pgmn=0,pamatr=0,pamd=0;  // For PBM, PGM, PPM, PAM detection
  char pgm_buf[32];
  U64 cdi=0;
  U64 mdfa=0;
  int cda=0,cdm=0,cdif=0;   // For CD sectors detection
  U32 cdf=0;
  TextInfo text = {}; // For TEXT
  
  // For DEC Alpha detection
  struct {
    Array<uint64_t> absPos{ 256 };
    Array<uint64_t> relPos{ 256 };
    uint32_t opcode = 0u, idx = 0u, count[4] = { 0 }, branches[4] = { 0 };
    uint64_t offset = 0u, last = 0u;
  } DEC_ALPHA;
  

 ///
   U64 uuds=0,uuds1=0,uudp=0,uudslen=0,uudh=0;//,b64i=0;
  U64 uudstart=0,uudend=0,uudline=0,uudnl=0,uudlcount=0,uuc=0;
  //base64
  U64 b64s=0,b64s1=0,b64p=0,b64slen=0,b64h=0;//,b64i=0;
  U64 base64start=0,base64end=0,b64line=0,b64nl=0,b64lcount=0;
  //base85
  U64 b85s=0,b85s1=0,b85p=0,b85slen=0,b85h=0;
  U64 base85start=0,base85end=0,b85line=0;
  //U64 gif=0,gifa=0,gifi=0,gifw=0,gifc=0,gifb=0,gifplt=0,gifgray=0; // For GIF detection
  gifInfo gif = {};
  U64 png=0, lastchunk=0, nextchunk=0;               // For PNG detection
  int pngw=0, pngh=0, pngbps=0, pngtype=0,pnggray=0; 
  //MSZip
  U64 MSZip=0, MSZ=0, MSZipz=0;
  int yu=0;
  int zlen=0;
  U64 fSZDD=0; //
  LZSS* lz77;
  U8 zbuf[256+32], zin[1<<16], zout[1<<16]; // For ZLIB stream detection
  int zbufpos=0, histogram[256]={};
  U64 zzippos=-1;
  bool brute = true;
  int pdfim=0,pdfimw=0,pdfimh=0,pdfimb=0,pdfgray=0;
  U64 pdfimp=0;
  U64 mrb=0,mrbsize=0,mrbcsize=0,mrbPictureType=0,mrbPackingMethod=0,mrbTell=0,mrbTell1=0,mrbw=0,mrbh=0; // For MRB detection
  U32 mrbmulti=0;
  //
  U64 tar=0,tarn=0,tarl=0,utar=0;
  TARheader tarh;
  U32 op=0;//DEC A
  U64 nesh=0,nesp=0,nesc=0;
  int textbin=0,txtpdf=0; //if 1/3 is text
  dBASE dbase;
  U64 dbasei=0;
  memset(&dbase, 0, sizeof(dBASE));
  // pdf image
  U64 pdfi1=0,pdfiw=0,pdfih=0,pdfic=0;
  char pdfi_buf[32];
  int pdfi_ptr=0,pdfin=0;
  U64 pLzwp=0;
  int pLzw=0;
  //BZip2
  U64 BZip2=0;
  bz_stream stream;
  char bzin[512],bzout[512];
  static int bzlevel=0;
  bool isBSDIFF=false;
   // For image detection
  static Array<U32> tfidf(0);
  static int tiffImages=-1;
  static Array<dTIFF> tiffFiles(10);
  static U64 tiffImageStart=0;
  static U64 tiffImageEnd=0;
  bool tiffMM=false;

  static int deth=0,detd=0;  // detected header/data size in bytes
  static Filetype dett;      // detected block type
  if (deth >1) return  in->setpos(start+deth),deth=0,dett;
  else if (deth ==-1) return  in->setpos(start),deth=0,dett;
  else if (detd) return  in->setpos( start+detd),detd=0,DEFAULT;
 
  textparser.reset(0);
  for (U64 i=0; i<n; ++i) {
    int c=in->getc();
    if (c==EOF) return (Filetype)(-1);
    buf4=buf4<<8|buf3>>24;
    buf3=buf3<<8|buf2>>24;
    buf2=buf2<<8|buf1>>24;
    buf1=buf1<<8|buf0>>24;
    buf0=buf0<<8|c;
    
    if (!(i&0x1fffff)) printStatus1(i, n); // after every 2mb
    bool isStandard=  ((c<128 && c>=32) || c==10 || c==13|| c==0x12 || c==12 || c==9 || c==4 );
    U8 lasc=buf0>>8;
    bool lastisc=((lasc<128 && lasc>=32) || lasc==10 || lasc==13|| lasc==0x12 || lasc==12 || lasc==9 || lasc==4 );
    lasc=buf0>>16;
    bool lastlastisc= ((lasc<128 && lasc>=32) || lasc==10 || lasc==13 || lasc==0x12|| lasc==12 || lasc==9 || lasc==4 );
    bool isExtended= (    (lastisc ||lastlastisc )&&
    ((c>=0xd0 && c<=0xdf) ||(c>=0xc0 && c<=0xcf)||(c>=0xe0 && c<=0xef)||(c>=0xf0 && c<=0xff)));  //ISO latin
    if (isStandard || isExtended) textbin++,info=textbin;
    
    if(tiffImages>=0){
        brute=false;
        textbin=0;
    for (int o=0;o<tiffImages; o++) { 
       if(  in->curpos()== tiffFiles[o].offset+1 ) {
           if (tiffFiles[o].compression==6 || tiffFiles[o].compression==7  ) {
               tiffImageEnd++;
                 if (type!=JPEG)return  in->setpos( start+i),JPEG;
                 else  return  in->setpos(start+tiffFiles[o].size),DEFAULT;
               }else if ( tiffFiles[o].compression==2) {
               tiffImageEnd++;
                 if (type!=DEFAULT)return  in->setpos( start+i),DEFAULT;
                 else  return  in->setpos(start+tiffFiles[o].size),DEFAULT;
           } else if (tiffFiles[o].compression==1 ||tiffFiles[o].compression==255) {
               tiffImageEnd++;
                 if (tiffFiles[o].bits==1  &&type!=IMAGE8 && tiffFiles[o].bits1!=14 ) return info=tiffFiles[o].width, in->setpos(start+i),IMAGE8;
                 if (tiffFiles[o].bits==1  &&type!=IMGUNK && tiffFiles[o].bits1==14  ) return info=0, in->setpos(start+i),IMGUNK;
                 else if (tiffFiles[o].bits==3 &&type!=IMAGE24 ) return info=tiffFiles[o].width, in->setpos( start+i),IMAGE24;
                 //else if (tiffFiles[o].bits==4 &&type!=IMAGE32 ) return info=tiffFiles[o].width, in->setpos( start+i),IMAGE32;
                 else if (tiffFiles[o].bits==1 && type==IMAGE8 ) return info=tiffFiles[o].width, in->setpos(start+tiffFiles[o].size),DEFAULT;
                 else if (tiffFiles[o].bits1==14 && type==IMGUNK ) return info=tiffFiles[o].width, in->setpos( start+tiffFiles[o].size),DEFAULT;
                 else if (tiffFiles[o].bits==3 &&type==IMAGE24 ) return info=tiffFiles[o].width, in->setpos(start+tiffFiles[o].size),DEFAULT;
                 //else if (tiffFiles[o].bits==4 &&type==IMAGE32 ) return info=tiffFiles[o].width, in->setpos(start+tiffFiles[o].size),DEFAULT;
            } 
       }
       if(tiffImageEnd>>1==tiffImages) tiffImages=-1,tiffImageEnd=0;
       }
    }  
    if (i==7 && buf1==0x42534449 && buf0==0x46463430/*-35*/) isBSDIFF=true;
    // BZhx = 0x425A68xx header, xx = level '1'-'9'
    if (isBSDIFF==false && (buf0&0xffffff00)==0x425A6800 && type!=BZIP2 && tarl==0){
        bzlevel=c-'0';
        if ((bzlevel>=1) && (bzlevel<=9)) {
            BZip2=i;
            U64 savepos=0;
            stream.bzalloc=NULL;
            stream.bzfree=NULL;
            stream.opaque=NULL;
            stream.avail_in=0;
            stream.next_in=NULL;
            int ret=BZ2_bzDecompressInit(&stream, 0, 0);
            if (ret==BZ_OK){
                savepos=in->curpos();
                in->setpos(savepos-4);
                stream.avail_in = in->blockread((U8*) &bzin, 512);
                stream.next_in = (char*)&bzin;
                stream.avail_out=512;
                stream.next_out = (char*)&bzout;
                ret = BZ2_bzDecompress(&stream);
                if ((ret==BZ_OK) || (ret==BZ_STREAM_END)) {
                    in->setpos(savepos);
                   (void)BZ2_bzDecompressEnd(&stream);
                   return in->setpos(start+BZip2-3),BZIP2;
                }
            }
            in->setpos(savepos);
            BZip2=bzlevel=0;
        }
    }
    if (type==BZIP2){
        U64 csize=0;
        FileTmp outf;
        U64 savepos=in->curpos();
        info=bzlevel;
        in->setpos(savepos-1);
        U64 dsize=bzip2decompress(in,&outf, bzlevel, csize, false); // decompress only (false)
        if (dsize>0){
            in->setpos(savepos);
            outf.close();
            return in->setpos(start+csize),DEFAULT;
        }
        in->setpos(savepos);        
        type=DEFAULT;
        BZip2=bzlevel=0;
    }
    
    // detect PNG images
    if (!png && buf3==0x89504E47 && buf2==0x0D0A1A0A && buf1==0x0000000D && buf0==0x49484452) png=i, pngtype=-1, lastchunk=buf3;//%PNG
    if (png){
      const int p=i-png;
      if (p==12){
        pngw = buf2;
        pngh = buf1;
        pngbps = buf0>>24;
        pngtype = (U8)(buf0>>16);
        pnggray = 0;
        png*=((buf0&0xFFFF)==0 && pngw && pngh && pngbps==8 && (!pngtype || pngtype==2 || pngtype==3 || pngtype==4 || pngtype==6));
      }
      else if (p>12 && pngtype<0)
        png = 0;
      else if (p==17){
        png*=((buf1&0xFF)==0);
        nextchunk =(png)?i+8:0;
      }
      else if (p>17 && i==nextchunk){
        nextchunk+=buf1+4+8;//CRC, Chunk length+Id
        lastchunk = buf0;
        png*=(lastchunk!=0x49454E44);//IEND
        if (lastchunk==0x504C5445){//PLTE
          png*=(buf1%3==0);
          pnggray = (png && IsGrayscalePalette(in, buf1/3));
        }
      }
    }
    // tar    
    // ustar header detection
    if (((buf0)==0x61722020 || (buf0&0xffffff00)==0x61720000) && (buf1&0xffffff)==0x757374 && tar==0&&tarl==0) tar=i,tarn=0,tarl=1,utar=263;
    // brute force detection on recursion level 0, at the block start only
    if(tarl==0 && it==0 && i==512 && start==0){
        U64 tarsave= in->curpos();
        in->setpos( tarsave-513);
        int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
            if (tarchecksum((char*)&tarh)){
                tar=i,tarn=512,tarl=2,utar=0;
                tar=in->curpos();
               int a=getoct(tarh.size,12);
               int b=a-(a/512)*512;
               if (b) tarn=tarn+512*2+(a/512)*512;
               else if (a==0) tarn=tarn+512;
               else tarn=tarn+512+(a/512)*512;
               tarn=tarn+int(i-tar+utar);
            } else  in->setpos(tarsave);
    }
    if (tarl) {
        const int p=int(i-tar+utar);        
        if (p==512 && tarn==0 && tarl==1) {
            U64 savedpos= in->curpos();
             in->setpos( savedpos-513);
             int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
            if (!tarchecksum((char*)&tarh)) tar=0,tarn=0,tarl=0;
            else{
                tarl=2;
                tar=in->curpos();
               int a=getoct(tarh.size,12);
               int b=a-(a/512)*512;
               if (b) tarn=tarn+512*2+(a/512)*512;
               else if (a==0) tarn=tarn+512;
               else tarn=tarn+512+(a/512)*512;
               tarn=tarn+p;
            }
        }
        if (tarn && tarl==2 && tarn==(start+i-tar+512)) {
            U64 savedpos= in->curpos();
             in->setpos(savedpos-512);
            int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
            if (!tarchecksum((char*)&tarh))  tarn=tar-512-start,tar=0,tarl=0;
            if (tarend((char*)&tarh)==true) {
                if (type==TAR) return  in->setpos(start+i),DEFAULT;
                return  in->setpos(start+tarn),TAR;
            } else{
                int a=getoct(tarh.size,12);
                int b=a-(a/512)*512;
                if (b) tarn=tarn+512*2+(a/512)*512;
                else if (a==0) tarn=tarn+512;
                else tarn=tarn+512+(a/512)*512;
            }
        }
        continue;
    }

    if ((buf0)==0x0080434b && MSZip==0  && !cdi  && type!=MDF) {
       MSZ=i;
       MSZip=i-4,MSZipz=(buf1&0xffff);
       MSZipz=((MSZipz&0xff)<<8) +(MSZipz >>8);
       zlen=MSZipz;
       yu=1;
    }
    if ( MSZip) {
        const int p=int(i-MSZip-12);        
        if (p==zlen) {
            MSZip=i-4;
            zlen=(buf1&0xffff);
            zlen=((zlen&0xff)<<8) +(zlen >>8);
            if( buf0==0x0080434b ) {    //32768 CK
                MSZipz+=zlen;           //12?
                yu++;
            }else if( (buf0&0xffff)==0x434b && zlen<32768) {                      //if final part <32768 CK
                yu++;
                MSZipz+=zlen+yu*8; //4 2 2
                if (type==MSZIP ) return  in->setpos(start+MSZipz),DEFAULT;
                return  in->setpos(start+MSZ-3),MSZIP;
            }else  {   
                MSZip=MSZipz=zlen=0;
            }
       }
    }
    
    // ZLIB stream detection
    histogram[c]++;
    if (i>=256)
      histogram[zbuf[zbufpos]]--;
    zbuf[zbufpos] = c;
    if (zbufpos<32)
      zbuf[zbufpos+256] = c;
    zbufpos=(zbufpos+1)&0xFF;
    if(!cdi && !mdfa && type!=MDF && b85s==0)  {
      int zh=parse_zlib_header(((int)zbuf[(zbufpos-32)&0xFF])*256+(int)zbuf[(zbufpos-32+1)&0xFF]);
    bool valid = (i>=31 && zh!=-1);
    if (!valid && brute && i>=255){
      U8 BTYPE = (zbuf[zbufpos]&7)>>1;
      if ((valid=(BTYPE==1 || BTYPE==2))){
        int maximum=0, used=0, offset=zbufpos;
        for (int i=0;i<4;i++,offset+=64){
          for (int j=0;j<64;j++){
            int freq = histogram[zbuf[(offset+j)&0xFF]];
            used+=(freq>0);
            maximum+=(freq>maximum);
          }
          if (maximum>=((12+i)<<i) || used*(6-i)<(i+1)*64){
            valid = false;
            break;
          }
        }
      }
    }
    if (valid || zzippos==i) {
      int streamLength=0, ret=0, brute=(zh==-1 && zzippos!=i);
      // Quick check possible stream by decompressing first 32 bytes
      z_stream strm;
      strm.zalloc=Z_NULL; strm.zfree=Z_NULL; strm.opaque=Z_NULL;
      strm.next_in=Z_NULL; strm.avail_in=0;
      if (zlib_inflateInit(&strm,zh)==Z_OK) {
        strm.next_in=&zbuf[(zbufpos-(brute?0:32))&0xFF]; strm.avail_in=32;
        strm.next_out=zout; strm.avail_out=1<<16;
        ret=inflate(&strm, Z_FINISH);
        ret=(inflateEnd(&strm)==Z_OK && (ret==Z_STREAM_END || ret==Z_BUF_ERROR) && strm.total_in>=16);
      }
      if (ret) {
        // Verify valid stream and determine stream length
        U64 savedpos= in->curpos();
        strm.zalloc=Z_NULL; strm.zfree=Z_NULL; strm.opaque=Z_NULL;
        strm.next_in=Z_NULL; strm.avail_in=0; strm.total_in=strm.total_out=0;
        if (zlib_inflateInit(&strm,zh)==Z_OK) {
          for (U64 j=i-(brute?255:31); j<n; j+=1<<16) {
            unsigned int blsize=min(n-j,1<<16);
             in->setpos( start+j);
            if (in->blockread(zin,   blsize  )!=blsize) break;
            strm.next_in=zin; strm.avail_in=blsize;
            do {
              strm.next_out=zout; strm.avail_out=1<<16;
              ret=inflate(&strm, Z_FINISH);
            } while (strm.avail_out==0 && ret==Z_BUF_ERROR);
            if (ret==Z_STREAM_END) streamLength=strm.total_in;
            if (ret!=Z_BUF_ERROR) break;
          }
          if (inflateEnd(&strm)!=Z_OK) streamLength=0;
        }
         in->setpos( savedpos);
      }
      if (streamLength>(brute<<7)) {
        info=0;
        if (pdfimw>0 && pdfimw<0x1000000 && pdfimh>0) {
          if (pdfimb==8 && (int)strm.total_out==pdfimw*pdfimh) info=((pdfgray?IMAGE8GRAY:IMAGE8)<<24)|pdfimw;
          if (pdfimb==8 && (int)strm.total_out==pdfimw*pdfimh*3) info=(IMAGE24<<24)|pdfimw*3;
          if (pdfimb==8 && (int)strm.total_out==pdfimw*pdfimh*4) info=(IMAGE32<<24)|pdfimw*4;
          if (pdfimb==4 && (int)strm.total_out==((pdfimw+1)/2)*pdfimh) info=(IMAGE4<<24)|((pdfimw+1)/2);
          if (pdfimb==1 && (int)strm.total_out==((pdfimw+7)/8)*pdfimh) info=(IMAGE1<<24)|((pdfimw+7)/8);
          pdfgray=0;
        }
        else if (png && pngw<0x1000000 && lastchunk==0x49444154){//IDAT
          if (pngbps==8 && pngtype==2 && (int)strm.total_out==(pngw*3+1)*pngh) info=(PNG24<<24)|(pngw*3), png=0;
          else if (pngbps==8 && pngtype==6 && (int)strm.total_out==(pngw*4+1)*pngh) info=(PNG32<<24)|(pngw*4), png=0;
          else if (pngbps==8 && (!pngtype || pngtype==3) && (int)strm.total_out==(pngw+1)*pngh) info=(((!pngtype || pnggray)?PNG8GRAY:PNG8)<<24)|(pngw), png=0;
        }
       return in->setpos( start+i-(brute?255:31)),detd=streamLength,ZLIB;
      }
    }
    if (zh==-1 && zbuf[(zbufpos-32)&0xFF]=='P' && zbuf[(zbufpos-32+1)&0xFF]=='K' && zbuf[(zbufpos-32+2)&0xFF]=='\x3'
      && zbuf[(zbufpos-32+3)&0xFF]=='\x4' && zbuf[(zbufpos-32+8)&0xFF]=='\x8' && zbuf[(zbufpos-32+9)&0xFF]=='\0') {
        int nlen=(int)zbuf[(zbufpos-32+26)&0xFF]+((int)zbuf[(zbufpos-32+27)&0xFF])*256
                +(int)zbuf[(zbufpos-32+28)&0xFF]+((int)zbuf[(zbufpos-32+29)&0xFF])*256;
        if (nlen<256 && i+30+nlen<n) zzippos=i+30+nlen;
    }
    if (i-pdfimp>1024) pdfim=pdfimw=pdfimh=pdfimb=pdfgray=0;
    if (pdfim>1 && !(isspace(c) || isdigit(c))) pdfim=1;
    if (pdfim==2 && isdigit(c)) pdfimw=pdfimw*10+(c-'0');
    if (pdfim==3 && isdigit(c)) pdfimh=pdfimh*10+(c-'0');
    if (pdfim==4 && isdigit(c)) pdfimb=pdfimb*10+(c-'0');
    if ((buf0&0xffff)==0x3c3c) pdfimp=i,pdfim=1; // <<
    if (pdfim && (buf1&0xffff)==0x2f57 && buf0==0x69647468) pdfim=2,pdfimw=0; // /Width
    if (pdfim && (buf1&0xffffff)==0x2f4865 && buf0==0x69676874) pdfim=3,pdfimh=0; // /Height
    if (pdfim && buf3==0x42697473 && buf2==0x50657243 && buf1==0x6f6d706f
       && buf0==0x6e656e74 && zbuf[(zbufpos-32+15)&0xFF]=='/') pdfim=4,pdfimb=0; // /BitsPerComponent
    if (pdfim && (buf2&0xFFFFFF)==0x2F4465 && buf1==0x76696365 && buf0==0x47726179) pdfgray=1; // /DeviceGray
}
    // NES rom 
    //The format of the header is as follows:
    //0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
    //4: Size of PRG ROM in 16 KB units
    //5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
    //6: Flags 6
    //7: Flags 7
    //8: Size of PRG RAM in 8 KB units (Value 0 infers 8 KB for compatibility; see PRG RAM circuit)
    //9: Flags 9
    //10: Flags 10 (unofficial)
    //11-15: Zero filled
    if (buf0==0x4E45531A && type!=MDF &&  !cdi) nesh=i,nesp=0;
    if (nesh) {
      const int p=int(i-nesh);
      if (p==1) nesp=buf0&0xff; //count of pages*0x3FFF
      else if (p==2) nesc=buf0&0xff; //count of CHR*0x1FFF
      else if (p==6 && ((buf0&0xfe)!=0) )nesh=0; // flags 9
      else if (p==11 && (buf0!=0) )nesh=0;
      else if (p==12) {
        if (nesp>0 && nesp<129) NES_DET(NESROM,nesh-3,0,nesp*0x3FFF+nesc*0x1FFF+15);
        nesh=0;
      }
    }
    
    // dBASE VERSIONS
    //  '02' > FoxBase
    //  '03' > dBase III without memo file
    //  '04' > dBase IV without memo file
    //  '05' > dBase V without memo file
    //  '07' > Visual Objects 1.x
    //  '30' > Visual FoxPro
    //  '31' > Visual FoxPro with AutoIncrement field
    //  '43' > dBASE IV SQL table files, no memo
    //  '63' > dBASE IV SQL system files, no memo
    //  '7b' > dBase IV with memo file
    //  '83' > dBase III with memo file
    //  '87' > Visual Objects 1.x with memo file
    //  '8b' > dBase IV with memo file
    ///  '8e' > dBase IV with SQL table
    //  'cb' > dBASE IV SQL table files, with memo
    //  'f5' > FoxPro with memo file - tested
    //  'fb' > FoxPro without memo file
    //
    if (dbasei==0 && ((c&7)==3 || (c&7)==4 || (c>>4)==3|| c==0xf5 || c==0x30) && tiffImages==-1) {
        dbasei=i+1,dbase.Version = ((c>>4)==3)?3:c&7;
        dbase.HeaderLength=dbase.Start=dbase.nRecords=dbase.RecordLength=0;
    }
    if (dbasei) {
      const int p=int(i-dbasei+1);
      if (p==2 && !(c>0 && c<13)) dbasei=0;      //month
      else if (p==3 && !(c>0 && c<32)) dbasei=0; //day
      else if (p==7 && !((dbase.nRecords = bswap(buf0)) > 0 && dbase.nRecords<0xFFFFF)) dbasei=0;
      else if (p==9 && !((dbase.HeaderLength = ((buf0>>8)&0xff)|(c<<8)) > 32 && ( ((dbase.HeaderLength-32-1)%32)==0 || (dbase.HeaderLength>255+8 && (((dbase.HeaderLength-=255+8)-32-1)%32)==0) )) ) dbasei=0;
      else if (p==11 && !(((dbase.RecordLength = ((buf0>>8)&0xff)|(c<<8))) > 8) ) dbasei=0;
      else if (p==15 && ((buf0&0xfffffefe)!=0 && ((buf0>>8)&0xfe)>1 && ((buf0)&0xfe)>1  )) dbasei=0;
      else if (p==16 && dbase.RecordLength >4000)dbasei=0;
      else if (p==17) {
          //Field Descriptor terminator 
          U64 savedpos = in->curpos();
          in->setpos(savedpos+dbase.HeaderLength-19);
          U8 marker=in->getc();
          if (marker!=0xd) dbasei=0,in->setpos(savedpos); 
          else{
            dbase.Start = 0;//dbase.HeaderLength;
            dbase.End =  dbase.Start + dbase.nRecords * dbase.RecordLength;
            U32 seekpos = dbase.End+in->curpos();
            in->setpos(seekpos);
            // get file end marker, fail if not present
            marker=in->getc();
            if (marker!=0x1a) dbasei=0, in->setpos(savedpos);
            else{
               in->setpos(savedpos);
               DBS_DET(DBASE,dbasei- 1,dbase.HeaderLength, dbase.nRecords * dbase.RecordLength+1,dbase.RecordLength); 
            }
          }
     }
     else if (p>dbase.HeaderLength && p>68) dbasei=0; // Fail if past Field Descriptor terminator
    }
    
    //detect LZSS compressed data in compress.exe generated archives
    if ((buf0==0x88F02733 && buf1==0x535A4444 && !cdi  && type!=MDF) ||(buf1==0x535A2088 && buf0==0xF02733D1)) fSZDD=i;
    if (fSZDD  && type!=MDF && buf0!=0 && (((i-fSZDD ==6) && (buf1&0xff00)==0x4100 && ((buf1&0xff)==0 ||(buf1&0xff)>'a')&&(buf1&0xff)<'z') || (buf1!=0x88F02733 && !cdi  && (i-fSZDD)==4))){
       int lz2=0;
        if (buf1!=0x88F02733 && (i-fSZDD)==4) lz2=2;  //+2 treshold
        U32 fsizez=bswap(buf0); //uncompressed file size
        if (fsizez<0x1ffffff){
            FileTmp outf;//=tmpfile2();          // try to decompress file
            lz77=new LZSS(in,&outf,fsizez,lz2);
            U64 savedpos= in->curpos();
            U32 u=lz77->decompress(); //compressed size
            int uf= lz77->usize; //uncompressed size
            delete lz77;
            U32 csize= in->curpos()-savedpos-(!in->eof()?1:0); //? overflow
            if (u!=csize || u>fsizez) fSZDD=0;          // reset if not same size or compressed size > uncompressed size
            else{
                 outf.setpos(0);  // try to compress decompressed file
                FileTmp out2;//=tmpfile2();
                lz77=new LZSS(&outf,&out2,u,lz2);
                U32 r=lz77->compress();
                delete lz77;
                //compare
                out2.setpos(0); 
                in->setpos(savedpos); 
                if (r!=(csize)) fSZDD=csize=0;    // reset if not same size
                for(int i=0;i<csize;i++){
                    U8 b=out2.getc();
                    if (b!=in->getc() ){
                        r=fSZDD=0; // just fail
                        break;
                    } 
                }
                out2.close();
                outf.close();
                if (fSZDD!=0) {
                     in->setpos( savedpos); //all good
                    //flag for +2 treshold, set bit 25
                    SZ_DET(SZDD,fSZDD+7-lz2,14-lz2,r,uf+(lz2?(1<<25):0)); 
                }
            }
            outf.close();
        }
        else fSZDD=0;
    } 
    
     // MDF (Alcohol 120%) CD (mode 1 and mode 2 form 1+2 - 2352 bytes+96 channel data)
    if ( !cdi && mdfa && type!=MDF)  return   in->setpos( start+mdfa-7), MDF;
    if (buf1==0x00ffffff && buf0==0xffffffff   && !mdfa  && type==MDF) mdfa=i;
    if (mdfa && i>mdfa) {
        const int p=(i-mdfa)%2448;
        if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) {
          mdfa=0;
          }
        if (!mdfa && type==MDF)  return  in->setpos( start+i-p-7), DEFAULT;
    }
    if (type==MDF) continue;
    
    // CD sectors detection (mode 1 and mode 2 form 1+2 - 2352 bytes)
    if (buf1==0x00ffffff && buf0==0xffffffff && !cdi && !mdfa) cdi=i,cda=-1,cdm=0;
    if (cdi && i>cdi) {
      const int p=(i-cdi)%2352;
      if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) cdi=0; // FIX it ?
      else if (p==16 && i+2336<n) {
        U8 data[2352];
        U64 savedpos= in->curpos();
         in->setpos( start+i-23);
        in->blockread(data,   2352  );
        int t=expand_cd_sector(data, cda, 1);
        if (t!=cdm) cdm=t*(i-cdi<2352);
        if (cdm && cda!=10 && (cdm==1 || buf0==buf1) && type!=CD) {
            //skip possible 96 byte channel data and test if another frame
             in->setpos(  in->curpos()+96);
            U32 mdf= (in->getc()<<24)+(in->getc()<<16)+(in->getc()<<8)+in->getc();
            U32 mdf1=(in->getc()<<24)+(in->getc()<<16)+(in->getc()<<8)+in->getc();
            if (mdf==0x00ffffff && mdf1==0xffffffff ) mdfa=cdi,cdi=cdm=0; //drop to mdf mode?
        }
         in->setpos( savedpos); // seek back if no mdf
        if (cdm && cda!=10 && (cdm==1 || buf0==buf1)) {
          if (type!=CD) return info=cdm, in->setpos( start+cdi-7), CD;
          cdif=cdm;
          cda=(data[12]<<16)+(data[13]<<8)+data[14];
          if (cdm!=1 && i-cdi>2352 && buf0!=cdf) cda=10;
          if (cdm!=1) cdf=buf0;
        } else cdi=0;
      }
      if (!cdi && type==CD) return info=cdif, in->setpos( start+i-p-7), DEFAULT;
    }
    if (type==CD) continue;
 
    // Detect JPEG by code SOI APPx (FF D8 FF Ex) followed by
    // SOF0 (FF C0 xx xx 08) and SOS (FF DA) within a reasonable distance.
    // Detect end by any code other than RST0-RST7 (FF D9-D7) or
    // a byte stuff (FF 00).
     if (!soi && i>=3 && ((
    ((buf0&0xffffff00)==0xffd8ff00 && ((buf0&0xfe)==0xC0 || (U8)buf0==0xC4 || ((U8)buf0>=0xDB && (U8)buf0<=0xFE)))
    ||(buf0&0xfffffff0)==0xffd8ffe0  ) )    
    ) soi=i, app=i+2, sos=sof=0;
    if (soi) {
      if (app==i && (buf0>>24)==0xff &&
         ((buf0>>16)&0xff)>0xc1 && ((buf0>>16)&0xff)<0xff) app=i+(buf0&0xffff)+2,brute=false;
      if (app<i && (buf1&0xff)==0xff && (buf0&0xfe0000ff)==0xc0000008) sof=i,brute=false;
      
      if (sof && sof>soi && i-sof<0x1000 && (buf0&0xffff)==0xffda) {
        sos=i;
        if (type!=JPEG) return  in->setpos(start+soi-3), JPEG;
      }
      if (i-soi>0x40000 && !sos) soi=0;
    }
    if (type==JPEG && soi && (buf0&0xffff)==0xffd9) eoi=i;
    if (type==JPEG &&  soi  && sos && eoi && (buf0&0xffff)==0xffd8) {
        return  in->setpos( start+i-1), DEFAULT;
    }
    if (type==JPEG && sos && i>sos && (buf0&0xff00)==0xff00
        && (buf0&0xff)!=0 && ((buf0&0xf8)!=0xd0 )) {
        return DEFAULT;
    }
    if (type==JPEG) continue;
//if (tiffImages>=0) continue;
    // Detect .wav file header
    if (buf0==0x52494646) wavi=i,wavm=0;
    if (wavi) {
            int p=i-wavi;
            if (p==4) wavsize=bswap(buf0);
            else if (p==8){
                wavtype=(buf0==0x57415645)?1:(buf0==0x7366626B)?2:0;
                if (!wavtype) wavi=0;
            }
            else if (wavtype){
                if (wavtype==1) {
                    if (p==16 && (buf1!=0x666d7420 || bswap(buf0)!=16)) wavi=0;
                    else if (p==20) wavt=bswap(buf0)&0xffff;
                    else if (p==22) wavch=bswap(buf0)&0xffff;
                    else if (p==24) wavsr=bswap(buf0) ;
                    else if (p==34) wavbps=bswap(buf0)&0xffff;
                    else if (p==40+wavm && buf1!=0x64617461) wavm+=bswap(buf0)+8,wavi=(wavm>0xfffff?0:wavi);
                    else if (p==40+wavm) {
                        int wavd=bswap(buf0);
                        info2=wavsr;
                        if ((wavch==1 || wavch==2) && (wavbps==8 || wavbps==16) && wavt==1 && wavd>0 && wavsize>=wavd+36
                             && wavd%((wavbps/8)*wavch)==0 && wavsr>=0) AUD_DET(AUDIO,wavi-3,44+wavm,wavd,wavch+wavbps/4-3);
                        wavi=0;
                    }
                }
                else{
                    if ((p==16 && buf1!=0x4C495354) || (p==20 && buf0!=0x494E464F))
                        wavi=0;
                    else if (p>20 && buf1==0x4C495354 && (wavi*=(buf0!=0))){
                        wavlen = bswap(buf0);
                        wavlist = i;
                    }
                    else if (wavlist){
                        p=i-wavlist;
                        if (p==8 && (buf1!=0x73647461 || buf0!=0x736D706C))
                            wavi=0;
                        else if (p==12){
                            int wavd = bswap(buf0);
                            info2=44100;
                            if (wavd && (wavd+12)==wavlen)
                                AUD_DET(AUDIO,wavi-3,(12+wavlist-(wavi-3)+1)&~1,wavd,1+16/4-3);
                            wavi=0;
                        }
                    }
                }
            }
    }

    // Detect .aiff file header
    if (buf0==0x464f524d) aiff=i,aiffs=0; // FORM
    if (aiff) {
      const int p=int(i-aiff);
      if (p==12 && (buf1!=0x41494646 || buf0!=0x434f4d4d)) aiff=0; // AIFF COMM
      else if (p==24) {
        const int bits=buf0&0xffff, chn=buf1>>16;
        if ((bits==8 || bits==16) && (chn==1 || chn==2)) aiffm=chn+bits/4+1; else aiff=0;
      } else if (p==42+aiffs && buf1!=0x53534e44) aiffs+=(buf0+8)+(buf0&1),aiff=(aiffs>0x400?0:aiff);
      else if (p==42+aiffs) AUD_DET(AUDIO,aiff-3,54+aiffs,buf0-8,aiffm);
    }

    // Detect .mod file header 
    if ((buf0==0x4d2e4b2e || buf0==0x3643484e || buf0==0x3843484e  // M.K. 6CHN 8CHN
       || buf0==0x464c5434 || buf0==0x464c5438) && (buf1&0xc0c0c0c0)==0 && i>=1083) {
      int64_t savedpos= in->curpos();
      const int chn=((buf0>>24)==0x36?6:(((buf0>>24)==0x38 || (buf0&0xff)==0x38)?8:4));
      int len=0; // total length of samples
      int numpat=1; // number of patterns
      for (int j=0; j<31; j++) {
         in->setpos(start+i-1083+42+j*30);
        const int i1=in->getc();
        const int i2=in->getc(); 
        len+=i1*512+i2*2;
      }
       in->setpos(start+i-131);
      for (int j=0; j<128; j++) {
        int x=in->getc();
        if (x+1>numpat) numpat=x+1;
      }
      if (numpat<65) AUD_DET(AUDIO,i-1083,1084+numpat*256*chn,len,4);
       in->setpos(savedpos);
    }
    
    // Detect .s3m file header 
    if (buf0==0x1a100000) s3mi=i,s3mno=s3mni=0;
    if (s3mi) {
      const int p=int(i-s3mi);
      if (p==4) s3mno=bswap(buf0)&0xffff,s3mni=(bswap(buf0)>>16);
      else if (p==16 && (((buf1>>16)&0xff)!=0x13 || buf0!=0x5343524d)) s3mi=0;
      else if (p==16) {
        int64_t savedpos= in->curpos();
        int b[31],sam_start=(1<<16),sam_end=0,ok=1;
        for (int j=0;j<s3mni;j++) {
           in->setpos( start+s3mi-31+0x60+s3mno+j*2);
          int i1=in->getc();
          i1+=in->getc()*256;
           in->setpos( start+s3mi-31+i1*16);
          i1=in->getc();
          if (i1==1) { // type: sample
            for (int k=0;k<31;k++) b[k]=in->getc();
            int len=b[15]+(b[16]<<8);
            int ofs=b[13]+(b[14]<<8);
            if (b[30]>1) ok=0;
            if (ofs*16<sam_start) sam_start=ofs*16;
            if (ofs*16+len>sam_end) sam_end=ofs*16+len;
          }
        }
        if (ok && sam_start<(1<<16)) AUD_DET(AUDIO,s3mi-31,sam_start,sam_end-sam_start,0);
        s3mi=0;
         in->setpos(savedpos);
      }
    }
   
    //detect rle encoded mrb files inside windows hlp files 506C
    if (!mrb && ((buf0&0xFFFF)==0x6c70 || (buf0&0xFFFF)==0x6C50) && !b64s1 && !b64s && !b85s1 && !b85s && type!=MDF &&  !cdi)
        mrb=i,mrbsize=0,mrbPictureType=mrbmulti=0; 
    if (mrb){
        U32 BitCount=0;
        const int p=int(i-mrb)-mrbmulti*4; //select only first image from multiple
        if (p==1 && c>1 && c<4&& mrbmulti==0)    mrbmulti=c-1;
        if (p==1 && c==0) mrb=0;
        if (p==7 ){  // 5=DDB   6=DIB   8=metafile
            if ((c==5 || c==6 )) mrbPictureType=c;
            else mrb=0;
         }
        if (p==8) {         // 0=uncomp 1=RunLen 2=LZ77 3=both
           if(c==1||c==2||c==3||c==0) mrbPackingMethod=c;
           else mrb=0;
        }
        if (p==10){
          if (mrbPictureType==6 && (mrbPackingMethod==1 || mrbPackingMethod==2)){
        //save ftell
        mrbTell= in->curpos()-2;
         in->setpos(mrbTell);
        U32 Xdpi=GetCDWord(in);
        U32 Ydpi=GetCDWord(in);
        U32 Planes=GetCWord(in);
         BitCount=GetCWord(in);
        mrbw=GetCDWord(in);
        mrbh=GetCDWord(in);
        U32 ColorsUsed=GetCDWord(in);
        U32 ColorsImportant=GetCDWord(in);
        mrbcsize=GetCDWord(in);
        U32 HotspotSize=GetCDWord(in);
        int CompressedOffset=(in->getc()<<24)|(in->getc()<<16)|(in->getc()<<8)|in->getc();
        int HotspotOffset=(in->getc()<<24)|(in->getc()<<16)|(in->getc()<<8)|in->getc();
        CompressedOffset=bswap(CompressedOffset);
        HotspotOffset=bswap(HotspotOffset);
        mrbsize=mrbcsize+ in->curpos()-mrbTell+10+(1<<BitCount)*4; // ignore HotspotSize
        int pixelBytes = (mrbw * mrbh * BitCount) >> 3;
        mrbTell=mrbTell+2;
            in->setpos(mrbTell);
        if (!(BitCount==8 || BitCount==4)|| mrbw<4 || mrbw>1024 || mrbPackingMethod==2|| mrbPackingMethod==3|| mrbPackingMethod==0) {
            if ((type==CMP ) &&   (mrbPackingMethod==2|| mrbPackingMethod==2) && mrbsize){
               return  in->setpos(start+mrbsize),DEFAULT;
            }      
            if( mrbPackingMethod==2 || mrbPackingMethod==2) MRBRLE_DET(CMP,mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
            mrbPictureType=mrb=mrbsize=mrbmulti=0;
            
        }else if (mrbPackingMethod <= 1 && pixelBytes < 360) {
            //printf("MRB: skipping\n");
            mrbPictureType=mrb=mrbsize=mrbmulti=0; // block is too small to be worth processing as a new block
        }
       } else mrbPictureType=mrb=mrbsize=0;
       }
       
       if ((type==MRBR || type==MRBR4 ) &&   (mrbPictureType==6 || mrbPictureType==8) && mrbsize){
        return  in->setpos(start+mrbsize),DEFAULT;
       }
       if ( (mrbPictureType==6 && BitCount==8) && mrbsize && mrbw>4 && mrbh>4){
        MRBRLE_DET(MRBR,mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
       }
       else if ( (mrbPictureType==6 && BitCount==4) && mrbsize && mrbw>4 && mrbh>4){
        MRBRLE_DET(MRBR4,mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
       }
    }
    // Detect .bmp image
    if ( !(bmp.bmp || bmp.hdrless) && (((buf0&0xffff)==16973) || (!(buf0&0xFFFFFF) && ((buf0>>24)==0x28))) ) //possible 'BM' or headerless DIB
      bmp = {},bmp.hdrless=!(U8)buf0,bmp.of=bmp.hdrless*54,bmp.bmp=i-bmp.hdrless*16;
    if (bmp.bmp || bmp.hdrless) {
      const int p=i-bmp.bmp;
      if (p==12) bmp.of=bswap(buf0);
      else if (p==16 && buf0!=0x28000000) bmp = {}; //BITMAPINFOHEADER (0x28)
      else if (p==20) bmp.x=bswap(buf0),bmp.bmp=((bmp.x==0||bmp.x>0x30000)?(bmp.hdrless=0):bmp.bmp); //width
      else if (p==24) bmp.y=abs((int)bswap(buf0)),bmp.bmp=((bmp.y==0||bmp.y>0x10000)?(bmp.hdrless=0):bmp.bmp); //height
      else if (p==27) bmp.bpp=c,bmp.bmp=((bmp.bpp!=1 && bmp.bpp!=4 && bmp.bpp!=8 && bmp.bpp!=24 && bmp.bpp!=32)?(bmp.hdrless=0):bmp.bmp);
      else if ((p==31) && buf0) bmp = {};
      else if (p==36) bmp.size=bswap(buf0);
      // check number of colors in palette (4 bytes), must be 0 (default) or <= 1<<bpp.
      // also check if image is too small, since it might not be worth it to use the image models
      else if (p==48){
        if ( (!buf0 || ((bswap(buf0)<=(U32)(1<<bmp.bpp)) && (bmp.bpp<=8))) && (((bmp.x*bmp.y*bmp.bpp)>>3)>64) ) {
          // possible icon/cursor?
          if (bmp.hdrless && (bmp.x*2==bmp.y) && bmp.bpp>1 &&
             (
              (bmp.size>0 && bmp.size==( (bmp.x*bmp.y*(bmp.bpp+1))>>4 )) ||
              ((!bmp.size || bmp.size<((bmp.x*bmp.y*bmp.bpp)>>3)) && (
               (bmp.x==8)   || (bmp.x==10) || (bmp.x==14) || (bmp.x==16) || (bmp.x==20) ||
               (bmp.x==22)  || (bmp.x==24) || (bmp.x==32) || (bmp.x==40) || (bmp.x==48) ||
               (bmp.x==60)  || (bmp.x==64) || (bmp.x==72) || (bmp.x==80) || (bmp.x==96) ||
               (bmp.x==128) || (bmp.x==256)
              ))
             )
          )
            bmp.y=bmp.x;

          // if DIB and not 24bpp, we must calculate the data offset based on BPP or num. of entries in color palette
          if (bmp.hdrless && (bmp.bpp<24))
            bmp.of+=((buf0)?bswap(buf0)*4:4<<bmp.bpp);
          bmp.of+=(bmp.bmp-1)*(bmp.bmp<1);

          if (bmp.hdrless && bmp.size && bmp.size<((bmp.x*bmp.y*bmp.bpp)>>3)) { }//Guard against erroneous DIB detections
          else if (bmp.bpp==1) IMG_DET(IMAGE1,max(0,bmp.bmp-1),bmp.of,(((bmp.x-1)>>5)+1)*4,bmp.y);
          else if (bmp.bpp==4) IMG_DET(IMAGE4,max(0,bmp.bmp-1),bmp.of,((bmp.x*4+31)>>5)*4,bmp.y);
          else if (bmp.bpp==8){
             in->setpos(start+bmp.bmp+53);
            IMG_DET( (IsGrayscalePalette(in, (buf0)?bswap(buf0):1<<bmp.bpp, 1))?IMAGE8GRAY:IMAGE8,max(0,bmp.bmp-1),bmp.of,(bmp.x+3)&-4,bmp.y);
          }
          else if (bmp.bpp==24) IMG_DET(IMAGE24,max(0,bmp.bmp-1),bmp.of,((bmp.x*3)+3)&-4,bmp.y);
          else if (bmp.bpp==32) IMG_DET(IMAGE32,max(0,bmp.bmp-1),bmp.of,bmp.x*4,bmp.y);
        }
        bmp = {};
      }
    }
    // Detect .pbm .pgm .ppm .pam image
    if ((buf0&0xfff0ff)==0x50300a && textparser.validlength()<TEXT_MIN_SIZE ) { 
      pgmn=(buf0&0xf00)>>8;
     if ((pgmn>=4 && pgmn<=6) || pgmn==7) pgm=i,pgm_ptr=pgmw=pgmh=pgmc=pgmcomment=pamatr=pamd=0;
    }
    if (pgm) {
      if (i-pgm==1 && c==0x23) pgmcomment=1; //pgm comment
      if (!pgmcomment && pgm_ptr) {
        int s=0;
        if (pgmn==7) {
           if ((buf1&0xffff)==0x5749 && buf0==0x44544820) pgm_ptr=0, pamatr=1; // WIDTH
           if ((buf1&0xffffff)==0x484549 && buf0==0x47485420) pgm_ptr=0, pamatr=2; // HEIGHT
           if ((buf1&0xffffff)==0x4d4158 && buf0==0x56414c20) pgm_ptr=0, pamatr=3; // MAXVAL
           if ((buf1&0xffff)==0x4445 && buf0==0x50544820) pgm_ptr=0, pamatr=4; // DEPTH
           if ((buf2&0xff)==0x54 && buf1==0x55504c54 && buf0==0x59504520) pgm_ptr=0, pamatr=5; // TUPLTYPE
           if ((buf1&0xffffff)==0x454e44 && buf0==0x4844520a) pgm_ptr=0, pamatr=6; // ENDHDR
           if (c==0x0a) {
             if (pamatr==0) pgm=0;
             else if (pamatr<5) s=pamatr;
             if (pamatr!=6) pamatr=0;
           }
        }
        else if ((c==0x20|| c==0x0a) && !pgmw) s=1;
        else if (c==0x0a && !pgmh) s=2;
        else if (c==0x0a && !pgmc && pgmn!=4) s=3;
        if (s) {
          if (pgm_ptr>=32) pgm_ptr=31;
          pgm_buf[pgm_ptr++]=0;
          int v=atoi(&pgm_buf[0]);
          if (v<0 || v>20000) v=0;
          if (s==1) pgmw=v; else if (s==2) pgmh=v; else if (s==3) pgmc=v; else if (s==4) pamd=v;
          if (v==0 || (s==3 && v>255)) pgm=0; else pgm_ptr=0;
        }
      }
      if (!pgmcomment) pgm_buf[pgm_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
      if (pgm_ptr>=32) pgm=pgm_ptr=0;
      if (i-pgm>255) pgm=pgm_ptr=0;
      if (pgmcomment && c==0x0a) pgmcomment=0;
      if (pgmw && pgmh && !pgmc && pgmn==4) IMG_DET(IMAGE1,pgm-2,i-pgm+3,(pgmw+7)/8,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==5 || (pgmn==7 && pamd==1 && pamatr==6))) IMG_DET(IMAGE8GRAY,pgm-2,i-pgm+3,pgmw,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==6 || (pgmn==7 && pamd==3 && pamatr==6))) IMG_DET(IMAGE24,pgm-2,i-pgm+3,pgmw*3,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==7 && pamd==4 && pamatr==6)) IMG_DET(IMAGE32,pgm-2,i-pgm+3,pgmw*4,pgmh);
    }
    
   // image in pdf
   //  'BI
   //   /W 86
   ///   /H 85
   //   /BPC 1 
    //  /IM true
   //   ID '
   /// 
    if ((buf0)==0x42490D0A  && pdfi1==0 ) { 
        pdfi1=i,pdfi_ptr=pdfiw=pdfih=pdfic=pdfi_ptr=0;
    }
    if (pdfi1) {
      if (pdfi_ptr) {
        int s=0;
        if ((buf0&0xffffff)==0x2F5720) pdfi_ptr=0, pdfin=1; // /W 
        if ((buf0&0xffffff)==0x2F4820 ) pdfi_ptr=0, pdfin=2; // /H
        if ((buf1&0xff)==0x2F && buf0==0x42504320) pdfi_ptr=0, pdfin=3; // /BPC
        if (buf1==0x2F494D20 && buf0==0x74727565) pdfi_ptr=0, pdfin=4; // /IM
        if ((buf0&0xffffff)==0x435320) pdfi_ptr=0, pdfin=-1; // CS
        if ((buf0&0xffffff)==0x494420) pdfi_ptr=0, pdfin=5; // ID
        if (c==0x0a) {
           if (pdfin==0) pdfi1=0;
           else if (pdfin>0 && pdfin<4) s=pdfin;
           if (pdfin==-1) pdfi_ptr=0;
           if (pdfin!=5) pdfin=0;
           
        }
        if (s) {
          if (pdfi_ptr>=16) pdfi_ptr=16;
          pdfi_buf[pdfi_ptr++]=0;
          int v=atoi(&pdfi_buf[0]);
          if (v<0 || v>1000) v=0;
          if (s==1) pdfiw=v; else if (s==2) pdfih=v; else if (s==3) pdfic=v; else if (s==4) { };
          if (v==0 || (s==3 && v>255)) pdfi1=0; else pdfi_ptr=0;
        }
      }
      pdfi_buf[pdfi_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
      if (pdfi_ptr>=16) pdfi1=pdfi_ptr=0;
      if (i-pdfi1>63) pdfi1=pdfi_ptr=0;
      if (pdfiw && pdfih && pdfic==1 && pdfin==5) IMG_DETP(IMAGE1,pdfi1-3,i-pdfi1+4,(pdfiw+7)/8,pdfih);
      if (pdfiw && pdfih && pdfic==8 && pdfin==5) IMG_DETP(IMAGE8,pdfi1-3,i-pdfi1+4,pdfiw,pdfih);
    }
    //detect lzw in pdf
    //headers: /LZWDecode >>stream 0x2F4C5A57 0x4465636F 0x64650D0A 0x3E3E0D0A 0x73747265 0x616D0D0A
    if (pLzw==0 && buf4==0x2F4C5A57 && buf3==0x4465636F && buf2==0x64650D0A && buf1==0x3E3E0D0A && buf0==0x73747265){
        pLzw=1,pLzwp=i-(5*4);
    }
    else if (pLzw==1 && buf0==0x616D0D0A){
             pLzw=2;
         }
    else if (pLzw==2 &&    buf1==0x0D0A656E &&    buf0==0x64737472){ //endstr
            pLzw=0;
            info2=0;
            B85_DET(CMP,(pLzwp+6*4+1),0,((i-2*4) -(pLzwp+6*4+1)+2));//type startpos 0 len
    }
   
    
    // Detect .rgb image
    if ((buf0&0xffff)==0x01da) rgbi=i,rgbx=rgby=0;
    if (rgbi) {
      const int p=int(i-rgbi);
      if (p==1 && c!=0) rgbi=0;
      else if (p==2 && c!=1) rgbi=0;
      else if (p==4 && (buf0&0xffff)!=1 && (buf0&0xffff)!=2 && (buf0&0xffff)!=3) rgbi=0;
      else if (p==6) rgbx=buf0&0xffff,rgbi=(rgbx==0?0:rgbi);
      else if (p==8) rgby=buf0&0xffff,rgbi=(rgby==0?0:rgbi);
      else if (p==10) {
        int z=buf0&0xffff;
        if (rgbx && rgby && (z==1 || z==3 || z==4)) IMG_DET(IMAGE8,rgbi-1,512,rgbx,rgby*z);
        rgbi=0;
      }
    }
      
    // Detect .tiff file header (2/8/24 bit color, not compressed).
   if ( (((buf1==0x49492a00 ||(buf1==0x4949524f && buf0==0x8000000  ) ) && n>i+(int)bswap(buf0) && tiffImages==-1)|| 
       ((buf1==0x4d4d002a  ) && n>i+(int)(buf0) && tiffImages==-1)) && !soi){
      if (buf1==0x4d4d002a) tiffMM=true;
       tiffImageStart=0,tiffImages=-1;
       U64 savedpos=in->curpos();
       int dirEntry=tiffMM==true?(int)buf0:(int)bswap(buf0);
       in->setpos(start+i+dirEntry-7);

      // read directory
      int dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
      int tifx=0,tify=0,tifz=0,tifzb=0,tifc=0,tifofs=0,tifofval=0,b[12],tifsiz=0;
      for (;;){
        if (dirsize>0 && dirsize<256) {            
        tiffImages++;
        for (int i=0; i<dirsize; i++) {
          for (int j=0; j<12; j++) b[j]=in->getc();
          if (b[11]==EOF) break;
          int tag=tiffMM==false? b[0]+(b[1]<<8):(int)bswap(b[0]+(b[1]<<8))>>16;;
          int tagfmt=tiffMM==false? b[2]+(b[3]<<8):(int)bswap(b[2]+(b[3]<<8))>>16;;
          int taglen=tiffMM==false?b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24):(int)bswap(b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24));;
          int tagval=tiffMM==false?b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24):(int)bswap(b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24));;
          //printf("Tag %d  val %d\n",tag, tagval);
          if (tagfmt==3||tagfmt==4) {
              tagval= (taglen==1 && tiffMM==true)?tagval>>16:tagval;
            if (tag==256) tifx=tagval,tiffFiles[tiffImages].width=tifx;
            else if (tag==257) tify=tagval,tiffFiles[tiffImages].height=tify;
            else if (tag==258) tifzb=(tagval==12||tagval==14||tagval==16)?14:taglen==1?tagval:8,tiffFiles[tiffImages].bits1=tifzb; // bits per component
            else if (tag==259) tifc=tagval, tiffFiles[tiffImages].compression=tifc ; // 1 = no compression, 6 jpeg
            else if (tag==273 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs&0xffff;
            else if (tag==513 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs; //jpeg
            else if (tag==514 && tagfmt==4) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz,tiffFiles[tiffImages].compression=6; //jpeg
            else if (tag==277) tifz=tagval,tiffFiles[tiffImages].bits=tifz; // components per pixel
            else if (tag==279) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz;
             else if (tag==50752 || tag==50649) tiffFiles[tiffImages].size=0; //to skip cr2 jpg
            else if (tag==330 ||  tag==34665) {
                int a=tfidf.size();
                if (a==0) tfidf.resize(a+taglen);
                U64 savedpos1= in->curpos();
                 in->setpos( start+i+tagval-5);
                if (a==0 && taglen==1) tfidf[a]=tagval;
                else if (taglen==1) tfidf[a+1]=tagval;
                else{
                
                for (int i2=0;i2<taglen; i2++) {
                     int g;
                     if (tiffMM==false) 
                   g=(in->getc()|(in->getc()<<8)|(in->getc()<<16)|(in->getc()<<24));
                    else
                    g=(in->getc()<<24|(in->getc()<<16)|(in->getc()<<8)|(in->getc()));
                    tfidf[i2]=g;
                }
                }
                 in->setpos(savedpos1);               
            }
          }
        }
         if(tiffFiles[tiffImages].size==0)tiffImages--;
        }
        int gg=in->getc()|(in->getc()<<8)|(in->getc()<<24)|(in->getc()<<16);
          gg=tiffMM==false?(int)gg:(int)bswap(gg);
        if (gg>0) {
         in->setpos( start+i+(gg)-7);
        dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
        }
        else break;
        
      }
       //
       if(int a=tfidf.size()>0){
            a++;
            for (int i2=0;i2<a; i2++) { 
               in->setpos( start+i+tfidf[i2]-7);
          // read directory
      int dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
      int tifx=0,tify=0,tifz=0,tifzb=0,tifc=0,tifofs=0,tifofval=0,b[12],tifsiz=0;
      if (dirsize>0 && dirsize<256) {
           tiffImages++;
        for (int i1=0; i1<dirsize; i1++) {
          for (int j=0; j<12; j++) b[j]=in->getc();
          if (b[11]==EOF) break;
          int tag=tiffMM==false? b[0]+(b[1]<<8):(int)bswap(b[0]+(b[1]<<8))>>16;;
          int tagfmt=tiffMM==false? b[2]+(b[3]<<8):(int)bswap(b[2]+(b[3]<<8))>>16;;
          int taglen=tiffMM==false?b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24):(int)bswap(b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24));;
          int tagval=tiffMM==false?b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24):(int)bswap(b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24));;
          // printf("Tag %d  val %d\n",tag, tagval);
          if (tagfmt==3||tagfmt==4) {
              tagval= (taglen==1 && tiffMM==true)?tagval>>16:tagval;
            if (tag==256) tifx=tagval,tiffFiles[tiffImages].width=tifx;
            else if (tag==257) tify=tagval,tiffFiles[tiffImages].height=tify;
            else if (tag==258) tifzb=(tagval==12||tagval==14||tagval==16)?14:taglen==1?tagval:8,tiffFiles[tiffImages].bits1=tifzb; // bits per component
            else if (tag==259) tifc=tagval, tiffFiles[tiffImages].compression=tifc ; // 1 = no compression, 6 jpeg
            else if (tag==273 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs;
            else if (tag==513 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs; //jpeg
            else if (tag==514 && tagfmt==4) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz,tiffFiles[tiffImages].compression=6; //jpeg
            else if (tag==277) tifz=tagval,tiffFiles[tiffImages].bits=tifz; // components per pixel
            else if (tag==279) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz;
            else if (tag==50752) tiffFiles[tiffImages].size=0;
           
          }
        }
        if(tiffFiles[tiffImages].size==0)tiffImages--;
      }
      }
        
      }
      tiffImageStart= start+i-7;
      tiffImages++;
      for (int o=0;o<tiffImages; o++) { 
      if (tiffFiles[o].height && tiffFiles[o].bits1==14)tiffFiles[o].width=tiffFiles[o].size/tiffFiles[o].height;
       tiffFiles[o].offset+=tiffImageStart;
       }
       //
      if (tifx>1 && tify && tifzb && (tifz==1 || tifz==3) && ((tifc==1) || (tifc==5 && tifsiz>0)) && (tifofs && tifofs+i<n)) {//tifc==5 LZW
        if (!tifofval) {
           in->setpos( start+i+tifofs-7);
          for (int j=0; j<4; j++) b[j]=in->getc();
          tifofs=b[0]+(b[1]<<8)+(b[2]<<16)+(b[3]<<24);
          tifofs=tiffMM==false?(int)tifofs:(int)bswap(tifofs);          
        }
        if (tifofs && tifofs<(1<<18) && tifofs+i<n && tifx>1) {
            if (tifc==1) {
          if (tifz==1 && tifzb==1) IMG_DET(IMAGE1,i-7,tifofs,((tifx-1)>>3)+1,tify);
          else if (tifz==1 && tifzb==8 && tifx<30000) IMG_DET(IMAGE8,i-7,tifofs,tifx,tify);
          else if (tifz==3 && tifzb==8 && tifx<30000) IMG_DET(IMAGE24,i-7,tifofs,tifx*3,tify);
        }
        else if (tifc==5 && tifsiz>0) {
            tifx=((tifx+8-tifzb)/(9-tifzb))*tifz;
            info=tifz*tifzb;
            info=(((info==1)?IMAGE1:((info==8)?IMAGE8:IMAGE24))<<24)|tifx;
            detd=tifsiz;
            in->setpos(start+i-7+tifofs);
            return dett=LZW;
          }
        }
      }
      in->setpos( savedpos);
    }
       
    // Detect .tga image (8-bit 256 colors or 24-bit uncompressed)
    if ((buf1&0xFFF7FF)==0x00010100 && (buf0&0xFFFFFFC7)==0x00000100 && (c==16 || c==24 || c==32)) tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF,tgaid=buf1>>24,tgamap=c/8;
    else if ((buf1&0xFFFFFF)==0x00000200 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=24,tgat=2;
    else if ((buf1&0xFFF7FF)==0x00000300 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF;
    if (tga) {
      if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
      else if (i-tga==10) {
          if ((buf0&0xFFF7)==32<<8)
          tgaz=32;
        if ((tgaz<<8)==(int)(buf0&0xFFD7) && tgax && tgay && U32(tgax*tgay)<0xFFFFFFF) {
          if (tgat==1){
            in->setpos(start+tga+11+tgaid);
            IMG_DET( (IsGrayscalePalette(in))?IMAGE8GRAY:IMAGE8,tga-7,18+tgaid+256*tgamap,tgax,tgay);
          }
          else if (tgat==2) IMG_DET((tgaz==24)?IMAGE24:IMAGE32,tga-7,18+tgaid,tgax*(tgaz>>3),tgay);
          else if (tgat==3) IMG_DET(IMAGE8GRAY,tga-7,18+tgaid,tgax,tgay);
          else if (tgat==9 || tgat==11) {
              const U64 savedpos=in->curpos();
            in->setpos(start+tga+11+tgaid);
            if (tgat==9) {
              info=(IsGrayscalePalette(in)?IMAGE8GRAY:IMAGE8)<<24;
              in->setpos(start+tga+11+tgaid+256*tgamap);
            }
            else
              info=IMAGE8GRAY<<24;
            info|=tgax;
            // now detect compressed image data size
            detd=0;
            int c=in->getc(), b=0, total=tgax*tgay, line=0;
            while (total>0 && c>=0 && (++detd, b=in->getc())>=0){
              if (c==0x80) { c=b; continue; }
              else if (c>0x7F) {
                total-=(c=(c&0x7F)+1); line+=c;
                c=in->getc();
                detd++;
              }
              else {
                in->setpos(in->curpos()+c); 
                detd+=++c; total-=c; line+=c;
                c=in->getc();
              }
              if (line>tgax) break;
              else if (line==tgax) line=0;
            }
            if (total==0) {
              in->setpos(start+tga+11+tgaid+256*tgamap);
              return dett=RLE;
            }
            else
              in->setpos(savedpos);
          }
        }
        tga=0;
      }
    }
    // Detect .gif
    if (type==DEFAULT && dett==GIF && i==0) {
      dett=DEFAULT;
      if (c==0x2c || c==0x21) gif.gif=2,gif.i=2;
      else gif.gray=0;
    }
    if (!gif.gif && (buf1&0xffff)==0x4749 && (buf0==0x46383961 || buf0==0x46383761)) gif.gif=1,gif.i=i+5;
    if (gif.gif) {
      if (gif.gif==1 && i==gif.i) gif.gif=2,gif.i = i+5+(gif.plt=(c&128)?(3*(2<<(c&7))):0),brute=false;
      if (gif.gif==2 && gif.plt && i==gif.i-gif.plt-3) gif.gray = IsGrayscalePalette(in, gif.plt/3), gif.plt = 0;
      if (gif.gif==2 && i==gif.i) {
        if ((buf0&0xff0000)==0x210000) gif.gif=5,gif.i=i;
        else if ((buf0&0xff0000)==0x2c0000) gif.gif=3,gif.i=i;
        else gif.gif=0;
      }
      if (gif.gif==3 && i==gif.i+6) gif.w=(bswap(buf0)&0xffff);
      if (gif.gif==3 && i==gif.i+7) gif.gif=4,gif.c=gif.b=0,gif.a=gif.i=i+2+(gif.plt=((c&128)?(3*(2<<(c&7))):0));
      if (gif.gif==4 && gif.plt) gif.gray = IsGrayscalePalette(in, gif.plt/3), gif.plt = 0;
      if (gif.gif==4 && i==gif.i) {
        if (c>0 && gif.b && gif.c!=gif.b) gif.w=0;
        if (c>0) gif.b=gif.c,gif.c=c,gif.i+=c+1;
        else if (!gif.w) gif.gif=2,gif.i=i+3;
        else return  in->setpos( start+gif.a-1),detd=i-gif.a+2,info=((gif.gray?IMAGE8GRAY:IMAGE8)<<24)|gif.w,dett=GIF;
      }
      if (gif.gif==5 && i==gif.i) {
        if (c>0) gif.i+=c+1; else gif.gif=2,gif.i=i+3;
      }
    }
    
    // Detect EXE if the low order byte (little-endian) XX is more
    // recently seen (and within 4K) if a relative to absolute address
    // conversion is done in the context CALL/JMP (E8/E9) XX xx xx 00/FF
    // 4 times in a row.  Detect end of EXE at the last
    // place this happens when it does not happen for 64KB.

    if (((buf1&0xfe)==0xe8 || (buf1&0xfff0)==0x0f80) && ((buf0+1)&0xfe)==0) {
      int r=buf0>>24;  // relative address low 8 bits
      int a=((buf0>>24)+i)&0xff;  // absolute address low 8 bits
      U64 rdist=(i-relpos[r]);
      U64 adist=(i-abspos[a]);
      if (adist<rdist && adist<0x800 && abspos[a]>5) {
        e8e9last=i;
        ++e8e9count;
        if (e8e9pos==0 || e8e9pos>abspos[a]) e8e9pos=abspos[a];
      }
      else e8e9count=0;
      if (type==DEFAULT && e8e9count>=4 && e8e9pos>5)
        return  in->setpos( start+e8e9pos-5), EXE;
      abspos[a]=i;
      relpos[r]=i;
    }
    if (i-e8e9last>0x4000) {
      if (type==EXE) return  in->setpos( start+e8e9last), DEFAULT;
      e8e9count=0,e8e9pos=0;
    }
    if (type==EXE) continue;

    // DEC Alpha
        // detect DEC Alpha
    DEC_ALPHA.idx = i & 3u;
    DEC_ALPHA.opcode = bswap(buf0);
    DEC_ALPHA.count[DEC_ALPHA.idx] = ((i >= 3u) && DECAlpha::IsValidInstruction(DEC_ALPHA.opcode)) ? DEC_ALPHA.count[DEC_ALPHA.idx] + 1u : DEC_ALPHA.count[DEC_ALPHA.idx] >> 3u;
    DEC_ALPHA.opcode >>= 21u;
    //test if bsr opcode and if last 4 opcodes are valid
    if (
      (DEC_ALPHA.opcode == (0x34u << 5u) + 26u) &&
      (DEC_ALPHA.count[DEC_ALPHA.idx] > 4u) &&
      ((e8e9count == 0) && !soi && !pgm && !rgbi && !bmp.bmp && !wavi && !tga)
    ) {
      std::uint32_t const absAddrLSB = DEC_ALPHA.opcode & 0xFFu; // absolute address low 8 bits
      std::uint32_t const relAddrLSB = ((DEC_ALPHA.opcode & 0x1FFFFFu) + static_cast<std::uint32_t>(i) / 4u) & 0xFFu; // relative address low 8 bits
      std::uint64_t const absPos = DEC_ALPHA.absPos[absAddrLSB];
      std::uint64_t const relPos = DEC_ALPHA.relPos[relAddrLSB];
      std::uint64_t const curPos = static_cast<std::uint64_t>(i);
      if ((absPos > relPos) && (curPos < absPos + 0x8000ull) && (absPos > 16u) && (curPos > absPos + 16ull) && (((curPos-absPos) & 3ull) == 0u)) {
        DEC_ALPHA.last = curPos;
        DEC_ALPHA.branches[DEC_ALPHA.idx]++;      
        if ((DEC_ALPHA.offset == 0u) || (DEC_ALPHA.offset > DEC_ALPHA.absPos[absAddrLSB])) {
          std::uint64_t const addr = curPos - (DEC_ALPHA.count[DEC_ALPHA.idx] - 1u) * 4ull;          
          DEC_ALPHA.offset = ((start > 0u) && (start == prv_start)) ? DEC_ALPHA.absPos[absAddrLSB] : std::min<std::uint64_t>(DEC_ALPHA.absPos[absAddrLSB], addr);
        }
      }
      else
        DEC_ALPHA.branches[DEC_ALPHA.idx] = 0u;
      DEC_ALPHA.absPos[absAddrLSB] = DEC_ALPHA.relPos[relAddrLSB] = curPos;
    }
     
    if ((type == DEFAULT) && (DEC_ALPHA.branches[DEC_ALPHA.idx] >= 16u))
      return in->setpos(start + DEC_ALPHA.offset - (start + DEC_ALPHA.offset) % 4), DECA;    
   
    if ((i + 1 == n) ||(static_cast<std::uint64_t>(i) > DEC_ALPHA.last + (type==DECA ? 0x8000ull : 0x4000ull)) && (DEC_ALPHA.count[DEC_ALPHA.offset & 3] == 0u)) {
      if (type == DECA)
        return in->setpos(start + DEC_ALPHA.last - (start + DEC_ALPHA.last) % 4), DEFAULT;
      DEC_ALPHA.last = 0u, DEC_ALPHA.offset = 0u;
       memset(&DEC_ALPHA.branches[0], 0u, sizeof(DEC_ALPHA.branches));
    }
    if (type==DECA) continue;
    
    // ARM
    op=(buf0)>>26; 
    //test if bl opcode and if last 3 opcodes are valid 
    // BL(4) and (ADD(1) or MOV(4)) as previous, 64 bit
    // ARMv8-A_Architecture_Reference_Manual_(Issue_A.a).pdf
    if (op==0x25 && //DECcount==0 &&//||(buf3)>>26==0x25 
    (((buf1)>>26==0x25 ||(buf2)>>26==0x25) ||
    (( ((buf1)>>24)&0x7F==0x11 || ((buf1)>>23)&0x7F==0x25  || ((buf1)>>23)&0x7F==0xa5 || ((buf1)>>23)&0x7F==0x64 || ((buf1)>>24)&0x7F==0x2A) )
    )&& e8e9count==0 && textparser.validlength()<TEXT_MIN_SIZE && !tar && !soi && !pgm && !rgbi && !bmp.bmp && !wavi && !tga && (buf1)>>31==1&& (buf2)>>31==1&& (buf3)>>31==1&& (buf4)>>31==1){ 
      int a=(buf0)&0xff;// absolute address low 8 bits
      int r=(buf0)&0x3FFFFFF;
      r+=(i)/4;  // relative address low 8 bits
      r=r&0xff;
      int rdist=int(i-relposARM[r]);
      int adist=int(i-absposARM[a]);
      if (adist<rdist && adist<0x3FFFFF && absposARM[a]>16 &&  adist>16 && adist%4==0) {
        ARMlast=i;
        ++ARMcount;
        if (ARMpos==0 || ARMpos>absposARM[a]) ARMpos=absposARM[a];
      }
      else ARMcount=0;
      if (type==DEFAULT && ARMcount>=18 && ARMpos>16 ) 
          return in->setpos(start+ARMpos-ARMpos%4), ARM;
      absposARM[a]=i;
      relposARM[r]=i;
    }
    if (i-ARMlast>0x4000) {
      if (type==ARM)
      return  in->setpos( start+ARMlast-ARMlast%4), DEFAULT;
      ARMcount=0,ARMpos=0,ARMlast=0;
      memset(&relposARM[0], 0, sizeof(relposARM));
      memset(&absposARM[0], 0, sizeof(absposARM));
    }
    
    // detect uuencoode in eml 
    // only 61 byte linesize and, ignore with trailin 1 byte lines.
    // 0A424547 494E2D2D 63757420
   if (uuds==0 && ((buf0==0x67696E20 && (buf1&0xffffff)==0x0A6265) ||
      ( buf2==0x0A424547&& buf1==0x494E2D2D&& buf0==0x63757420) )) uuds=1,uudp=i-8,uudh=0,uudslen=0,uudlcount=0; //'\n begin ' '\nBEGIN--cut '
    else if (uuds==1 && (buf0&0xffff)==0x0A4D ) {
        uuds=2,uudh=i,uudslen=uudh-uudp;
        uudstart=i;
        if (uudslen>40) uuds=0; //drop if header is larger 
        }
    else if (uuds==1 && (buf0&0xffff)==0x0A62 ) uuds=0; //reset for begin
    else if (uuds==2 && (buf0&0xff)==0x0A && uudline==0) {
         uudline=i-uudstart,uudnl=i;      //capture line lenght
         if (uudline!=61) uuds=uudline=0; //drop if not
    }
    else if (uuds==2 &&( (buf0&0xff)==0x0A || (buf0==0x454E442D && (buf1&0xff)==0x0A))  && uudline!=0){// lf or END-
         if ( (i-uudnl+1<uudline && (buf0&0xffff)!=0x0A0A) ||  ((buf0&0xffff)==0x0A0A) ) { // if smaller and not padding
            uudend=i-1;
            if ( (((uudend-uudstart)>128) && ((uudend-uudstart)<512*1024))  ){
             uuds=0;
             UUU_DET(UUENC,uudh,uudslen,uudend -uudstart,uuc);
            }
         }
         else if(buf0==0x454E442D){ // 'END-'
             uudend=i-5;
              UUU_DET(UUENC,uudh,uudslen,uudend -uudstart,uuc);
         }
         uudnl=i+2; //update 0x0D0A pos
         uudlcount++;
         }
    else if (uuds==2 && (c>=32 && c<=96)) {if (uuc==0 && c==96) uuc=1;} // some files use char 96, set for info;
    else if (uuds==2)   uuds=0;
    
    // base64 encoded data detection
    // detect base64 in html/xml container, single stream
    // ';base64,' or '![CDATA[' :image> 3a696d6167653e
    if (b64s1==0 &&   ((buf1==0x3b626173 && buf0==0x6536342c)||(buf1==0x215b4344 && buf0==0x4154415b) )) b64s1=1,b64h=i+1,base64start=i+1; //' base64' ||((buf1&0xffffff)==0x3a696d && buf0==0x6167653e)
    else if (b64s1==1 && (isalnum(c) || (c == '+') || (c == '/')||(c == '=')) ) {
        continue;
        }  
    else if (b64s1==1) {
         base64end=i,b64s1=0;
         if (base64end -base64start>128) B64_DET(BASE64,b64h, 8,base64end -base64start);
    }
   
   // detect base64 in eml, etc. multiline
   if (b64s==0 && buf0==0x73653634 && ((buf1&0xffffff)==0x206261 || (buf1&0xffffff)==0x204261)) b64s=1,b64p=i-6,b64h=0,b64slen=0,b64lcount=0; //' base64' ' Base64'
    else if (b64s==1 && buf0==0x0D0A0D0A ) {
        b64s=2,b64h=i+1,b64slen=b64h-b64p;
        base64start=i+1;
        if (b64slen>192) b64s=0; //drop if header is larger 
        }
    else if (b64s==2 && (buf0&0xffff)==0x0D0A && b64line==0) {
         b64line=i-base64start,b64nl=i+2;//capture line lenght
         if (b64line<=4 || b64line>255) b64s=0;
         //else continue;
    }
    else if (b64s==2 && (buf0&0xffff)==0x0D0A  && b64line!=0 && (buf0&0xffffff)!=0x3D0D0A && buf0!=0x3D3D0D0A ){
         if (i-b64nl+1<b64line && buf0!=0x0D0A0D0A) { // if smaller and not padding
            base64end=i-1;
            if (((base64end-base64start)>512) && ((base64end-base64start)<base64max)){
             b64s=0;
             B64_DET(BASE64,b64h,b64slen,base64end -base64start);
            }
         }
         else if (buf0==0x0D0A0D0A) { // if smaller and not padding
           base64end=i-1-2;
           if (((base64end-base64start)>512) && ((base64end-base64start)<base64max))
               B64_DET(BASE64,b64h,b64slen,base64end -base64start);
           b64s=0;
         }
         b64nl=i+2; //update 0x0D0A pos
         b64lcount++;
         //continue;
         }
    else if (b64s==2 && ((buf0&0xffffff)==0x3D0D0A ||buf0==0x3D3D0D0A)) { //if padding '=' or '=='
        base64end=i-1;
        b64s=0;
        if (((base64end-base64start)>512) && ((base64end-base64start)<base64max))
            B64_DET(BASE64,b64h,b64slen,base64end -base64start);
    }
    else if (b64s==2 && (is_base64(c) || c=='='))   ;//continue;
    else if (b64s==2)   b64s=0;
    
    //detect ascii85 encoded data
    //headers: stream\n stream\r\n oNimage\n utimage\n \nimage\n
    if (b85s==0 && ((buf0==0x65616D0A && (buf1&0xffffff)==0x737472)|| (buf0==0x616D0D0A && buf1==0x73747265)|| (buf0==0x6167650A && buf1==0x6F4E696D) || (buf0==0x6167650A && buf1==0x7574696D) || (buf0==0x6167650A && (buf1&0xffffff)==0x0A696D))){
        b85s=1,b85p=i-6,b85h=0,b85slen=0;//,b85lcount=0; // 
        b85s=2,b85h=i+1,b85slen=b85h-b85p;
        base85start=i;//+1;
        if (b85slen>128) b85s=0; //drop if header is larger 
        //txtStart=0;
        }
    else if (b85s==2){
        if  ((buf0&0xff)==0x0d && b85line==0) {
            b85line=i-base85start;//,b85nl=i+2;//capture line lenght
            if (b85line<=25 || b85line>255) b85s=0;
        }
        
        else if ( (buf0&0xff)==0x7E)  { //if padding '~' or '=='
            base85end=i-1;//2
            b85s=0;
            if (((base85end-base85start)>60) && ((base85end-base85start)<base85max))
            B85_DET(BASE85,b85h,b85slen,base85end -base85start);
        }
        else if ( (is_base85(c)))          ;
        else if  ((buf0&0xff)==0x0d && b85line!=0) {
            if (b85line!=i-base85start) b85s=0;
        }
        else     b85s=0;   
    }
    
    // Detect text, utf-8, eoltext and text0
    text.isLetter = tolower(c)!=toupper(c);
    //text.countLetters+=(text.isLetter)?1:0;
    text.countNumbers+=(c>='0' && c<='9') ?1:0;
    //text.isNumbertext=text.countLetters< text.countNumbers;
    text.isUTF8 = ((c!=0xC0 && c!=0xC1 && c<0xF5) && (
        (c<0x80) ||
        // possible 1st byte of UTF8 sequence
        ((buf0&0xC000)!=0xC000 && ((c&0xE0)==0xC0 || (c&0xF0)==0xE0 || (c&0xF8)==0xF0)) ||
        // possible 2nd byte of UTF8 sequence
        ((buf0&0xE0C0)==0xC080 && (buf0&0xFE00)!=0xC000) || (buf0&0xF0C0)==0xE080 || ((buf0&0xF8C0)==0xF080 && ((buf0>>8)&0xFF)<0xF5) ||
        // possible 3rd byte of UTF8 sequence
        (buf0&0xF0C0C0)==0xE08080 || ((buf0&0xF8C0C0)==0xF08080 && ((buf0>>16)&0xFF)<0xF5) ||
        // possible 4th byte of UTF8 sequence
        ((buf0&0xF8C0C0C0)==0xF0808080 && (buf0>>24)<0xF5)
    ));
     textparser.countUTF8+=((text.isUTF8 && !text.isLetter && (c>=0x80))?1:0);
    if (text.lastNLpos==0 && c==NEW_LINE ) text.lastNLpos=i;
    else if (text.lastNLpos>0 && c==NEW_LINE ) {
        int tNL=i-text.lastNLpos;
        if (tNL<90 && tNL>45) 
            text.countNL++;          //Count if in range   
        else 
            text.totalNL+=tNL>3?1:0; //Total new line count
        text.lastNLpos=i;
    }
    text.lastNL = (c==NEW_LINE || c==CARRIAGE_RETURN ||c==10|| c==5)?0:text.lastNL+1;
   /* if (c==SPACE || c==TAB ||c==0x12 ){
      text.lastSpace = 0;
      text.spaceRun++;
    }
    else{
      text.lastSpace++;
      text.spaceRun = 0;
    }
    text.wordLength = (text.isLetter)?text.wordLength+1:0;
    text.missCount-=text.misses>>31;
    text.misses<<=1;
    text.zeroRun=(!c && text.zeroRun<32)?text.zeroRun+1:0;
    //if (c==NEW_LINE || c==5){
      //if (!text.seenNL)
       // text.needsEolTransform = true;
    //  text.seenNL = true;
      //text.needsEolTransform&=(text.countNL>text.totalNL);//U8(buf0>>8)==CARRIAGE_RETURN;
    //}
   /* bool tspace=(c<SPACE && c!=TAB && (text.zeroRun<2 || text.zeroRun>8) && text.lastNL!=0);
    //bool tcr=((buf0&0xFF00)==(CARRIAGE_RETURN<<8) || (buf0&0xFF00)==(10<<8));
    bool tscr= (text.spaceRun>8 && text.lastNL>256 && !text.isUTF8); // utf8 line lenght can be more then 4 times longer
    bool tword=(!text.isLetter && !text.isUTF8 &&  ( text.lastNL>256 || text.lastSpace > max( text.lastNL, text.wordLength*8) || text.wordLength>32) );
    if (tspace || 
       // tcr||
        tscr||
        tword
     ) {
        text.misses|=1;
        text.missCount++;
        int length = i-text.start-1; 
        bool dtype=(png || pdfimw || cdi || soi || pgm || rgbi || tga || gif.gif || b64s||tar || bmp.bmp ||wavi ||b64s1 ||b85s1||b85s||DECcount||mrb||uuds );
        if (((length<MIN_TEXT_SIZE && text.missCount>MAX_TEXT_MISSES) || dtype)){
          text = {};
          text.start = i+1;
        }
        else if (text.missCount>MAX_TEXT_MISSES ) {
          text.needsEolTransform=(text.countNL>text.totalNL);
          if (text.isNumbertext)     info=1;
          in->setpos(start + text.start);
          detd = length;
          return (text.needsEolTransform)?EOLTEXT:(( text.isNumbertext)?TEXT0:(text.countUTF8>MIN_TEXT_SIZE?TXTUTF8:TEXT));
        }
    }
    //disable zlib brute if text lenght is over minimum.
    //if ( (i-text.start)>MIN_TEXT_SIZE) brute=false;
  }
    if (n-text.start>=MIN_TEXT_SIZE && ! (png || pdfimw || cdi || soi || pgm || rgbi || tga || gif.gif || b64s||tar || bmp.bmp ||wavi ||b64s1 ||b85s1||b85s||DECcount||mrb ||uuds) ||
       (s1==0 && (n-text.start)==n && type==DEFAULT) // ignore minimum text lenght
       ){
        text.needsEolTransform=(text.countNL>text.totalNL);
        in->setpos(start + text.start);
        detd = n-text.start;
        if ( text.isNumbertext)     info=1;
    return (text.needsEolTransform)?EOLTEXT:(( text.isNumbertext)?TEXT0:(text.countUTF8>MIN_TEXT_SIZE?TXTUTF8:TEXT));*/
    // Detect text
    // This is different from the above detection routines: it's a negative detection (it detects a failure)
    //text.countNumbers+=(c>='0' && c<='9') ?1:0;
    textparser.set_number(text.countNumbers);
    U32 t = utf8_state_table[c];
    textparser.UTF8State = utf8_state_table[256 + textparser.UTF8State + t];

    if(textparser.UTF8State == UTF8_ACCEPT) { // proper end of a valid utf8 sequence
      if (c==NEW_LINE || c==5) {
      //  if (((buf0>>8)&0xff) != CARRIAGE_RETURN)
      //    textparser.setEOLType(2); // mixed or LF-only
      //  else 
      //    if (textparser.EOLType()==0)textparser.setEOLType(1); // CRLF-only
      if (textparser.validlength()>TEXT_MIN_SIZE*64) brute=false; //4mb
      if(textparser.invalidCount)textparser.invalidCount=0;
      }
      if(textparser.invalidCount)textparser.invalidCount=(textparser.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE);
      
      if(textparser.invalidCount==0){
      textparser.setEOLType(text.countNL>text.totalNL);
        textparser.setend(i); // a possible end of block position
    }
    }
    else
    if(textparser.UTF8State == UTF8_REJECT) { // illegal state
      if(text.totalNL/(text.countNL+1)==0)textparser.invalidCount=0;
      textparser.invalidCount = textparser.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE + TEXT_ADAPT_RATE;
      textparser.UTF8State = UTF8_ACCEPT; // reset state
      if (textparser.validlength()<TEXT_MIN_SIZE) {
         // printf("%d",textparser.validlength());
        textparser.reset(i+1); // it's not text (or not long enough) - start over
        text.countNumbers=0;
      }
      else if (textparser.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE) {
        if (textparser.validlength()<TEXT_MIN_SIZE)
        {  textparser.reset(i+1); // it's not text (or not long enough) - start over
          text.countNumbers=0;}
        else // Commit text block validation
          {
          textparser.next(i+1);return type;}
      }
    }
  }
  return type;


}



// Print progress: n is the number of bytes compressed or decompressed
void printStatus(U64 n, U64 size,int tid=-1) {
if (level>0 && tid>=0)  fprintf(stderr,"%2d %6.2f%%\b\b\b\b\b\b\b\b\b\b",tid, float(100)*n/(size+1)), fflush(stdout);
else if (level>0)  fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}

void encode_cd(File* in, File* out, int len, int info) {
  const int BLOCK=2352;
  U8 blk[BLOCK];
  out->putc((len%BLOCK)>>8);
  out->putc(len%BLOCK);
  for (int offset=0; offset<len; offset+=BLOCK) {
    if (offset+BLOCK > len) {
       in->blockread(&blk[0],   len-offset );
      out->blockwrite(&blk[0],  len-offset  );
    } else {
      in->blockread(&blk[0],   BLOCK  );
      if (info==3) blk[15]=3;
      if (offset==0) out->blockwrite(&blk[12],   4+4*(blk[15]!=1)  );
      out->blockwrite(&blk[16+8*(blk[15]!=1)],   2048+276*(info==3)  );
      if (offset+BLOCK*2 > len && blk[15]!=1) out->blockwrite(&blk[16],  4  );
    }
  }
}

int decode_cd(File*in, int size, File*out, FMode mode, U64 &diffFound) {
  const int BLOCK=2352;
  U8 blk[BLOCK];
  long i=0, i2=0;
  int a=-1, bsize=0, q=in->getc();
  q=(q<<8)+in->getc();
  size-=2;
  while (i<size) {
    if (size-i==q) {
      in->blockread(blk, q  );
      out->blockwrite(blk, q  );
      i+=q;
      i2+=q;
    } else if (i==0) {
      in->blockread(blk+12, 4  );
      if (blk[15]!=1) in->blockread(blk+16, 4  );
      bsize=2048+(blk[15]==3)*276;
      i+=4*(blk[15]!=1)+4;
    } else {
      a=(blk[12]<<16)+(blk[13]<<8)+blk[14];
    }
    in->blockread(blk+16+(blk[15]!=1)*8, bsize   );
    i+=bsize;
    if (bsize>2048) blk[15]=3;
    if (blk[15]!=1 && size-q-i==4) {
      in->blockread(blk+16, 4   );
      i+=4;
    }
    expand_cd_sector(blk, a, 0);
    if (mode==FDECOMPRESS) out->blockwrite(blk, BLOCK  );
    else if (mode==FCOMPARE) for (int j=0; j<BLOCK; ++j) if (blk[j]!=out->getc() && !diffFound) diffFound=i2+j+1;
    i2+=BLOCK;
  }
  return i2;
}

void encode_rle(File *in, File *out, U64 size, int info, int &hdrsize) {
  U8 b, c = in->getc();
  int i = 1, maxBlockSize = info&0xFFFFFF;
  out->put32(maxBlockSize);
  hdrsize=(4);
  while (i<(int)size) {
    b = in->getc(), i++;
    if (c==0x80) { c = b; continue; }
    else if (c>0x7F) {
      for (int j=0; j<=(c&0x7F); j++) out->putc(b);
      c = in->getc(), i++;
    }
    else {
      for (int j=0; j<=c; j++, i++) { out->putc(b), b = in->getc(); }
      c = b;
    }
  }
}

#define rleOutputRun { \
  while (run > 128) { \
    *outPtr++ = 0xFF, *outPtr++ = byte; \
    run-=128; \
  } \
  *outPtr++ = (U8)(0x80|(run-1)), *outPtr++ = byte; \
}

U64 decode_rle(File *in, U64 size, File *out, FMode mode, U64 &diffFound) {
  U8 inBuffer[0x10000]={0};
  U8 outBuffer[0x10200]={0};
  U64 pos = 0;
  int maxBlockSize = (int)in->get32();
  enum { BASE, LITERAL, RUN, LITERAL_RUN } state;
  do {
    U64 remaining = in->blockread(&inBuffer[0], maxBlockSize);
    U8 *inPtr = (U8*)inBuffer;
    U8 *outPtr= (U8*)outBuffer;
    U8 *lastLiteral = nullptr;
    state = BASE;
    while (remaining>0){
      U8 byte = *inPtr++, loop = 0;
      int run = 1;
      for (remaining--; remaining>0 && byte==*inPtr; remaining--, run++, inPtr++);
      do {
        loop = 0;
        switch (state) {
          case BASE: case RUN: {
            if (run>1) {
              state = RUN;
              rleOutputRun;
            }
            else {
              lastLiteral = outPtr;
              *outPtr++ = 0, *outPtr++ = byte;
              state = LITERAL;
            }
            break;
          }
          case LITERAL: {
            if (run>1) {
              state = LITERAL_RUN;
              rleOutputRun;
            }
            else {
              if (++(*lastLiteral)==127)
                state = BASE;
              *outPtr++ = byte;
            }
            break;
          }
          case LITERAL_RUN: {
            if (outPtr[-2]==0x81 && *lastLiteral<(125)) {
              state = (((*lastLiteral)+=2)==127)?BASE:LITERAL;
              outPtr[-2] = outPtr[-1];
            }
            else
              state = RUN;
            loop = 1;
          }
        }
      } while (loop);
    }

    U64 length = outPtr-(U8*)(&outBuffer[0]);
    if (mode==FDECOMPRESS)
      out->blockwrite(&outBuffer[0], length);
    else if (mode==FCOMPARE) {
      for (int j=0; j<(int)length; ++j) {
        if (outBuffer[j]!=out->getc() && !diffFound) {
          diffFound = pos+j+1;
          break; 
        }
      }
    }
    pos+=length;
  } while (!in->eof() && !diffFound);
  return pos;
}


struct LZWentry{
  int16_t prefix;
  int16_t suffix;
};

#define LZW_RESET_CODE 256
#define LZW_EOF_CODE   257

class LZWDictionary{
private:
  const static int32_t HashSize = 9221;
  LZWentry dictionary[4096];
  int16_t table[HashSize];
  uint8_t buffer[4096];
public:
  int32_t index;
  LZWDictionary(): index(0){ reset(); }
  void reset(){
    memset(&dictionary, 0xFF, sizeof(dictionary));
    memset(&table, 0xFF, sizeof(table));
    for (int32_t i=0; i<256; i++){
      table[-findEntry(-1, i)-1] = (int16_t)i;
      dictionary[i].suffix = i;
    }
    index = 258; //2 extra codes, one for resetting the dictionary and one for signaling EOF
  }
  int32_t findEntry(const int32_t prefix, const int32_t suffix){
    int32_t i = finalize32(hash(prefix, suffix), 13);
    int32_t offset = (i>0)?HashSize-i:1;
    while (true){
      if (table[i]<0) //free slot?
        return -i-1;
      else if (dictionary[table[i]].prefix==prefix && dictionary[table[i]].suffix==suffix) //is it the entry we want?
        return table[i];
      i-=offset;
      if (i<0)
        i+=HashSize;
    }
  }
  void addEntry(const int32_t prefix, const int32_t suffix, const int32_t offset = -1){
    if (prefix==-1 || prefix>=index || index>4095 || offset>=0)
      return;
    dictionary[index].prefix = prefix;
    dictionary[index].suffix = suffix;
    table[-offset-1] = index;
    index+=(index<4096);
  }
  int32_t dumpEntry(File *f, int32_t code){
    int32_t n = 4095;
    while (code>256 && n>=0){
      buffer[n] = uint8_t(dictionary[code].suffix);
      n--;
      code = dictionary[code].prefix;
    }
    buffer[n] = uint8_t(code);
    f->blockwrite(&buffer[n], 4096-n);
    return code;
  }
};

int encode_lzw(File *in, File *out, U64 size, int &hdrsize) {
  LZWDictionary dic;
  int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
  bool done = false;
  while (!done) {
    buffer = in->getc();
    if (buffer<0) { return 0; }
    for (int32_t j=0; j<8; j++ ) {
      code+=code+((buffer>>(7-j))&1), bitsUsed++;
      if (bitsUsed>=bitsPerCode) {
        if (code==LZW_EOF_CODE){ done=true; break; }
        else if (code==LZW_RESET_CODE){
          dic.reset();
          parent=-1; bitsPerCode=9;
        }
        else{
          if (code<dic.index){
            if (parent!=-1)
              dic.addEntry(parent, dic.dumpEntry(out, code));
            else
              out->putc(code);
          }
          else if (code==dic.index){
            int32_t a = dic.dumpEntry(out, parent);
            out->putc(a);
            dic.addEntry(parent,a);
          }
          else return 0;
          parent = code;
        }
        bitsUsed=0; code=0;
        if ((1<<bitsPerCode)==dic.index+1 && dic.index<4096)
          bitsPerCode++;
      }
    }
  }
  return 1;
}

inline void writeCode(File *f, const FMode mode, int32_t *buffer, U64 *pos, int32_t *bitsUsed, const int32_t bitsPerCode, const int32_t code, U64 *diffFound){
  *buffer<<=bitsPerCode; *buffer|=code;
  (*bitsUsed)+=bitsPerCode;
  while ((*bitsUsed)>7) {
    const uint8_t B = *buffer>>(*bitsUsed-=8);
    (*pos)++;
    if (mode==FDECOMPRESS) f->putc(B);
    else if (mode==FCOMPARE && B!=f->getc()) *diffFound=*pos;
  }
}

U64 decode_lzw(File *in, U64 size, File *out, FMode mode, U64 &diffFound) {
  LZWDictionary dic;
  U64 pos=0;
  int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
  writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE, &diffFound);
  while ((code=in->getc())>=0 && diffFound==0) {
    int32_t index = dic.findEntry(parent, code);
    if (index<0){ // entry not found
      writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, parent, &diffFound);
      if (dic.index>4092){
        writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE, &diffFound);
        dic.reset();
        bitsPerCode = 9;
      }
      else{
        dic.addEntry(parent, code, index);
        if (dic.index>=(1<<bitsPerCode))
          bitsPerCode++;
      }
      parent = code;
    }
    else
      parent = index;
  }
  if (parent>=0)
    writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, parent, &diffFound);
  writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_EOF_CODE, &diffFound);
  if (bitsUsed>0) { // flush buffer
    pos++;
    if (mode==FDECOMPRESS) out->putc(uint8_t(buffer));
    else if (mode==FCOMPARE && uint8_t(buffer)!=out->getc()) diffFound=pos;
  }
  return pos;
}
// EXE transform: <encoded-size> <begin> <block>...
// Encoded-size is 4 bytes, MSB first.
// begin is the offset of the start of the input file, 4 bytes, MSB first.
// Each block applies the e8e9 transform to strings falling entirely
// within the block starting from the end and working backwards.
// The 5 byte pattern is E8/E9 xx xx xx 00/FF (x86 CALL/JMP xxxxxxxx)
// where xxxxxxxx is a relative address LSB first.  The address is
// converted to an absolute address by adding the offset mod 2^25
// (in range +-2^24).

void encode_exe(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  out->put32((U32)begin);

  // Transform
  for (int offset=0; offset<len; offset+=BLOCK) {
    int size=min(int(len-offset), BLOCK);
    int bytesRead= in->blockread(&blk[0],   size );
    if (bytesRead!=size) quit("encode_exe read error");
    for (int i=bytesRead-1; i>=5; --i) {
      if ((blk[i-4]==0xe8 || blk[i-4]==0xe9 || (blk[i-5]==0x0f && (blk[i-4]&0xf0)==0x80))
         && (blk[i]==0||blk[i]==0xff)) {
        int a=(blk[i-3]|blk[i-2]<<8|blk[i-1]<<16|blk[i]<<24)+offset+begin+i+1;
        a<<=7;
        a>>=7;
        blk[i]=a>>24;
        blk[i-1]=a^176;
        blk[i-2]=(a>>8)^176;
        blk[i-3]=(a>>16)^176;
      }
    }
    out->blockwrite(&blk[0],   bytesRead  );
  }
}

U64 decode_exe(Encoder& en, int size, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
  const int BLOCK=0x10000;  // block size
  int begin, offset=6, a, showstatus=(s2!=0);
  U8 c[6];
  begin=en.decompress()<<24;
  begin|=en.decompress()<<16;
  begin|=en.decompress()<<8;
  begin|=en.decompress();
  size-=4;
  for (int i=4; i>=0; i--) c[i]=en.decompress();  // Fill queue

  while (offset<size+6) {
    memmove(c+1, c, 5);
    if (offset<=size) c[0]=en.decompress();
    // E8E9 transform: E8/E9 xx xx xx 00/FF -> subtract location from x
    if ((c[0]==0x00 || c[0]==0xFF) && (c[4]==0xE8 || c[4]==0xE9 || (c[5]==0x0F && (c[4]&0xF0)==0x80))
     && (((offset-1)^(offset-6))&-BLOCK)==0 && offset<=size) { // not crossing block boundary
      a=((c[1]^176)|(c[2]^176)<<8|(c[3]^176)<<16|c[0]<<24)-offset-begin;
      a<<=7;
      a>>=7;
      c[3]=a;
      c[2]=a>>8;
      c[1]=a>>16;
      c[0]=a>>24;
    }
    if (mode==FDECOMPRESS) out->putc(c[5]);
    else if (mode==FCOMPARE && c[5]!=out->getc() && !diffFound) diffFound=offset-6+1;
    if (showstatus && !(offset&0xfff)) printStatus(s1+offset-6, s2);
    offset++;
  }
  return size;
}

U64 encode_bzip2(File* in, File* out, U64 len,int level) {
    U64 compressed_stream_size=len;
    return bzip2decompress(in,out, level, compressed_stream_size);;
}

U64 decode_bzip2(File* in, U64 size, File*out, FMode mode, U64 &diffFound,int info) {
    int filelen=0;
    if (mode==FDECOMPRESS){
            filelen=bzip2compress(in, out, info, size);
        }
    else if (mode==FCOMPARE){
        FileTmp o;
        filelen=bzip2compress(in, &o, info, size);
        o.setpos(0);
        for(int i=0;i<filelen;i++){
            U8 b=o.getc();
            if (b!=out->getc() && !diffFound) diffFound= out->curpos();
        }
        o.close();
    }
    return filelen;
}
MTFList  MTF(81);

#define ZLIB_NUM_COMBINATIONS 81

int encode_zlib(File* in, File* out, int len) {
  const int BLOCK=1<<16, LIMIT=256;
  U8 zin[BLOCK*2],zout[BLOCK],zrec[BLOCK*2];//, diffByte[81*LIMIT];
  Array<U8>  diffByte(ZLIB_NUM_COMBINATIONS*LIMIT);
  //int diffPos[81*LIMIT];
  Array<int>  diffPos(ZLIB_NUM_COMBINATIONS*LIMIT);
  // Step 1 - parse offset type form zlib stream header
  U64 pos= in->curpos();
  unsigned int h1=in->getc(), h2=in->getc();
   in->setpos( pos);
  int zh=parse_zlib_header(h1*256+h2);
  int memlevel,clevel,window=zh==-1?0:MAX_WBITS+10+zh/4,ctype=zh%4;
  int minclevel=window==0?1:ctype==3?7:ctype==2?6:ctype==1?2:1;
  int maxclevel=window==0?9:ctype==3?9:ctype==2?6:ctype==1?5:1;
  int index=-1, nTrials=0;
  bool found=false;
  // Step 2 - check recompressiblitiy, determine parameters and save differences
  z_stream main_strm, rec_strm[ZLIB_NUM_COMBINATIONS];
  int diffCount[ZLIB_NUM_COMBINATIONS], recpos[ZLIB_NUM_COMBINATIONS], main_ret=Z_STREAM_END;
  main_strm.zalloc=Z_NULL; main_strm.zfree=Z_NULL; main_strm.opaque=Z_NULL;
  main_strm.next_in=Z_NULL; main_strm.avail_in=0;
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) return false;
  for (int i=0; i<ZLIB_NUM_COMBINATIONS; i++) {
      clevel=(i/9)+1;
    // Early skip if invalid parameter
    if (clevel<minclevel || clevel>maxclevel){
      diffCount[i]=LIMIT;
      continue;
    }
    memlevel=(i%9)+1;
    rec_strm[i].zalloc=Z_NULL; rec_strm[i].zfree=Z_NULL; rec_strm[i].opaque=Z_NULL;
    rec_strm[i].next_in=Z_NULL; rec_strm[i].avail_in=0;
    int ret=deflateInit2(&rec_strm[i], clevel, Z_DEFLATED, window-MAX_WBITS, memlevel, Z_DEFAULT_STRATEGY);
    diffCount[i]=(  ret==Z_OK)?0:LIMIT;
    recpos[i]=BLOCK*2;
    diffPos[i*LIMIT]=-1;
    diffByte[i*LIMIT]=0;
  }
  for (U64 i=0; i<len; i+=BLOCK) {
    U32 blsize=min(U32(len-i),BLOCK);
    nTrials=0;
    for (int j=0; j<ZLIB_NUM_COMBINATIONS; j++) {
      if (diffCount[j]==LIMIT) continue;
      nTrials++;
      if (recpos[j]>=BLOCK)
        recpos[j]-=BLOCK;
    }
    // early break if nothing left to test
    if (nTrials==0)
      break;
    memmove(&zrec[0], &zrec[BLOCK], BLOCK);
    memmove(&zin[0], &zin[BLOCK], BLOCK);
    in->blockread(&zin[BLOCK],   blsize  ); // Read block from input file
    
    // Decompress/inflate block
    main_strm.next_in=&zin[BLOCK]; main_strm.avail_in=blsize;
    do {
      main_strm.next_out=&zout[0]; main_strm.avail_out=BLOCK;
      main_ret=inflate(&main_strm, Z_FINISH);
      nTrials=0;
      // Recompress/deflate block with all possible parameters
      for (int j=MTF.GetFirst(); j>=0; j=MTF.GetNext()){
        if (diffCount[j]>=LIMIT) continue;
        nTrials++;
        rec_strm[j].next_in=&zout[0];  rec_strm[j].avail_in=BLOCK-main_strm.avail_out;
        rec_strm[j].next_out=&zrec[recpos[j]]; rec_strm[j].avail_out=BLOCK*2-recpos[j];
        int ret=deflate(&rec_strm[j], (int)main_strm.total_in == len ? Z_FINISH : Z_NO_FLUSH);
        if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END && ret!=Z_OK) { diffCount[j]=LIMIT; continue; }

        // Compare
        int end=2*BLOCK-(int)rec_strm[j].avail_out;
        int tail=max(main_ret==Z_STREAM_END ? len-(int)rec_strm[j].total_out : 0,0);
        for (int k=recpos[j]; k<end+tail; k++) {
          if ((k<end && i+k-BLOCK<len && zrec[k]!=zin[k]) || k>=end) {
            if (++diffCount[j]<LIMIT) {
              const int p=j*LIMIT+diffCount[j];
              diffPos[p]=i+k-BLOCK;
              assert(k < sizeof(zin)/sizeof(*zin));
              diffByte[p]=zin[k];
            }
          }
        }
        // Early break on perfect match
        if (main_ret==Z_STREAM_END && diffCount[j]==0){
          index=j;
          found=true;
          break;
        }
        recpos[j]=2*BLOCK-rec_strm[j].avail_out;
      }
     } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR && nTrials>0);
    if ((main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) || nTrials==0) break;
  }
  int minCount=(found)?0:LIMIT;
  for (int i=ZLIB_NUM_COMBINATIONS-1; i>=0; i--) {
     clevel=(i/9)+1;
    if (clevel>=minclevel && clevel<=maxclevel)
      deflateEnd(&rec_strm[i]);
    if (!found && diffCount[i]<minCount)
      minCount=diffCount[index=i];
  }
  inflateEnd(&main_strm);
  if (minCount==LIMIT) return false;
  MTF.MoveToFront(index);
  // Step 3 - write parameters, differences and precompressed (inflated) data
  out->putc(diffCount[index]);
  out->putc(window);
  out->putc(index);
  for (int i=0; i<=diffCount[index]; i++) {
    const int v=i==diffCount[index] ? len-diffPos[index*LIMIT+i]
                                    : diffPos[index*LIMIT+i+1]-diffPos[index*LIMIT+i]-1;
    out->put32(v);
  }
  for (int i=0; i<diffCount[index]; i++) out->putc(diffByte[index*LIMIT+i+1]);
  
   in->setpos( pos);
  main_strm.zalloc=Z_NULL; main_strm.zfree=Z_NULL; main_strm.opaque=Z_NULL;
  main_strm.next_in=Z_NULL; main_strm.avail_in=0;
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) return false;
  for (int i=0; i<len; i+=BLOCK) {
    unsigned int blsize=min(len-i,BLOCK);
    in->blockread(&zin[0],  blsize  );
    main_strm.next_in=&zin[0]; main_strm.avail_in=blsize;
    do {
      main_strm.next_out=&zout[0]; main_strm.avail_out=BLOCK;
      main_ret=inflate(&main_strm, Z_FINISH);
      out->blockwrite(&zout[0],   BLOCK-main_strm.avail_out  );
    } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR);
    if (main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) break;
  }
  inflateEnd(&main_strm);
  return main_ret==Z_STREAM_END;
}

int decode_zlib(File* in, int size, File*out, FMode mode, U64 &diffFound) {
  const int BLOCK=1<<16, LIMIT=256;
  U8 zin[BLOCK],zout[BLOCK];
  int diffCount=min(in->getc(),LIMIT-1);
  int window=in->getc()-MAX_WBITS;
  int index=in->getc();
  int memlevel=(index%9)+1;
  int clevel=(index/9)+1;  
  int len=0;
   
  Array<int>  diffPos(LIMIT);
  diffPos[0]=-1;
  for (int i=0; i<=diffCount; i++) {
    int v=in->get32();
    if (i==diffCount) len=v+diffPos[i]; else diffPos[i+1]=v+diffPos[i]+1;
  }
  Array<U8>  diffByte(LIMIT);
  diffByte[0]=0;
  for (int i=0; i<diffCount; i++) diffByte[i+1]=in->getc();
  size-=7+5*diffCount;
  
  z_stream rec_strm;
  int diffIndex=1,recpos=0;
  rec_strm.zalloc=Z_NULL; rec_strm.zfree=Z_NULL; rec_strm.opaque=Z_NULL;
  rec_strm.next_in=Z_NULL; rec_strm.avail_in=0;
  int ret=deflateInit2(&rec_strm, clevel, Z_DEFLATED, window, memlevel, Z_DEFAULT_STRATEGY);
  if (ret!=Z_OK) return 0;
  for (int i=0; i<size; i+=BLOCK) {
    int blsize=min(size-i,BLOCK);
    in->blockread(&zin[0],  blsize  );
    rec_strm.next_in=&zin[0];  rec_strm.avail_in=blsize;
    do {
      rec_strm.next_out=&zout[0]; rec_strm.avail_out=BLOCK;
      ret=deflate(&rec_strm, i+blsize==size ? Z_FINISH : Z_NO_FLUSH);
      if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END && ret!=Z_OK) break;
      const int have=min(BLOCK-rec_strm.avail_out,len-recpos);
      while (diffIndex<=diffCount && diffPos[diffIndex]>=recpos && diffPos[diffIndex]<recpos+have) {
        zout[diffPos[diffIndex]-recpos]=diffByte[diffIndex];
        diffIndex++;
      }
      if (mode==FDECOMPRESS) out->blockwrite(&zout[0],   have  );
      else if (mode==FCOMPARE) for (int j=0; j<have; j++) if (zout[j]!=out->getc() && !diffFound) diffFound=recpos+j+1;
      recpos+=have;
      
    } while (rec_strm.avail_out==0);
  }
  while (diffIndex<=diffCount) {
    if (mode==FDECOMPRESS) out->putc(diffByte[diffIndex]);
    else if (mode==FCOMPARE) if (diffByte[diffIndex]!=out->getc() && !diffFound) diffFound=recpos+1;
    diffIndex++;
    recpos++;
  }  
  deflateEnd(&rec_strm);
  return recpos==len ? len : 0;
}

 // Transform DEC Alpha code
void encode_dec(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  int count=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_dec read error");
        for (int i=0; i<bytesRead-3; i+=4) {
        unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
        if ((op>>21)==0x34*32+26) { // bsr r26,offset
        int offset=op&0x1fffff;
        offset+=(i)/4;
        op&=~0x1fffff;
        op|=offset&0x1fffff;
        
        count++;
      }
      DECAlpha::Shuffle(op);
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
    }
    out->blockwrite(&blk[0],  bytesRead  );
  }
}

U64 decode_dec(Encoder& en, int size1, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
    const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    U8 c;
    int b=0;
    FileTmp dtmp;
    FileTmp dtmp1;
    U32 count=0;
    //decompress file
    for (int i=0; i<size1; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     
    dtmp.setpos(0);
    for (int j=0; j<size1; j+=BLOCK) {
        int size=min(int(size1-j), BLOCK);
        int bytesRead=dtmp.blockread(&blk[0],   size  );
        if (bytesRead!=size) quit("encode_dec read error");
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
            DECAlpha::Unshuffle(op);
                if ((op>>21)==0x34*32+26  ) { // bsr r26,offset
                   int offset=op&0x1fffff;
                   offset-=(i)/4;
                   op&=~0x1fffff;
                   op|=offset&0x1fffff;
                   count++;
                }
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
        }
        dtmp1.blockwrite(&blk[0],   bytesRead  );
    }
    dtmp1.setpos(0);
    dtmp.close();
    for ( int i=0; i<size1; i++) {
        b=dtmp1.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    if(count<16) diffFound=1; //fail if replaced below threshold
    dtmp1.close();
    return size1; 
}

// Transform DEC Alpha code
void encode_arm(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  int count=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_arm read error");
        for (int i=0; i<bytesRead-3; i+=4) {
        unsigned op=blk[i+3]|(blk[i+2]<<8)|(blk[i+1]<<16)|(blk[i]<<24);
        if ((op>>26)==0x25) {
        int offset=op&0x3FFFFFF;
        offset+=(i)/4;
        op&=~0x3FFFFFF;
        op|=offset&0x3FFFFFF;
        count++;
      }
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
    }
    out->blockwrite(&blk[0],  bytesRead  );
  }
}

U64 decode_arm(Encoder& en, int size1, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
    const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    U8 c;
    int b=0;
    FileTmp dtmp;
    FileTmp dtmp1;
    U32 count=0;
    //decompress file
    for (int i=0; i<size1; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     
     dtmp.setpos(0);
    for (int j=0; j<size1; j+=BLOCK) {
        int size=min(int(size1-j), BLOCK);
        int bytesRead=dtmp.blockread(&blk[0],   size  );
        if (bytesRead!=size) quit("encode_arm read error");
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
                if ((op>>26)==0x25) { 
                   int offset=op&0x3FFFFFF;
                   offset-=(i)/4;
                   op&=~0x3FFFFFF;
                   op|=offset&0x3FFFFFF;
                   count++;
                }
        blk[i+3]=op;
        blk[i+2]=op>>8;
        blk[i+1]=op>>16;
        blk[i]=op>>24;
        }
        dtmp1.blockwrite(&blk[0],   bytesRead  );
    }
    dtmp1.setpos(0);
    dtmp.close();
    for ( int i=0; i<size1; i++) {
        b=dtmp1.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    if(count<16) diffFound=1; //fail if replaced below threshold
    dtmp1.close();
    return size1; 
}
//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com
#include "wrtpre.inc"

void encode_txt(File* in, File* out, U64 len,int wrtn) {
    assert(wrtn<2);
   XWRT_Encoder* wrt;
   wrt=new XWRT_Encoder();
   wrt->defaultSettings(wrtn);
   wrt->WRT_start_encoding(in,out,len,false);
   delete wrt;
}

//called only when encode_txt output was smaller then input
U64 decode_txt(Encoder& en, U64 size, File*out, FMode mode, U64 &diffFound) {
    XWRT_Decoder* wrt;
    wrt=new XWRT_Decoder();
    char c;
    int b=0;
    U64 bb=0L;
    FileTmp dtmp;
    //decompress file
    for (U64 i=0L; i<size; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     dtmp.setpos(0);
    wrt->defaultSettings(0);
    bb=wrt->WRT_start_decoding(&dtmp);
    for ( U64 i=0L; i<bb; i++) {
        b=wrt->WRT_decode();    
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    dtmp.close();
    delete wrt;
    return bb; 
}
//it's not standard so some files use 'space' some use '`'
#define UUENCODE(c,b) ((c) ? ((c) & 077) + ' ': (b) ? '`':((c) & 077) + ' ')
int decode_uud(File*in, int size, File*out, FMode mode, U64 &diffFound){
    //U8 inn[3];
    int i;//, len=0, blocksout = 0;
    int fle=0;
    int flag=0; 
    int outlen=0,n;
    int tlf=0;//,g=0;
    flag=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> p(45+4);
    Array<U8,1> ptr((outlen>>1)*4+10);
    tlf=(tlf&192);                     //ignored
    if (tlf==128)       tlf=10;        // LF: 10 
    else if (tlf==64)   tlf=13;        // LF: 13
    else                tlf=0;
    int c1, c2, c3, c4;
    while(fle<outlen){
        memset(&p[0], 0, 45);
        n=in->blockread(&p[0], 45);
        ptr[fle++]=UUENCODE(n,flag);
        for(i = 0; i < n; i += 3){
            c1 = p[0+i] >> 2;
            c2 = (p[0+i] << 4) & 060 | (p[1+i] >> 4) & 017;
            c3 = (p[1+i] << 2) & 074 | (p[2+i] >> 6) & 03;
            c4 = p[2+i] & 077;

            ptr[fle++]=(UUENCODE(c1,flag));
            ptr[fle++]=(UUENCODE(c2,flag));
            ptr[fle++]=(UUENCODE(c3,flag));
            ptr[fle++]=(UUENCODE(c4,flag));
       }
       ptr[fle++]=10; //lf
    }

    //Write out or compare
    if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
        }
    else if (mode==FCOMPARE){
       // out->setpos(0);
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}
    
#define UUDECODE(c) (((c) - ' ') & 077)
void encode_uud(File* in, File* out, int len,int info) {
  int in_len = 0;
  int i = 0;
  int j = 0;
  int b=0,n=0;
  int lfp=0;
  int tlf=0;
  char src[4];
  int uumem=(len>>1)*3+10;
  Array<U8,1> ptr(uumem);
  Array<U8,1> p(62);
  int olen=5,inbytes=0;
  int c1, c2, c3,lp=0;
  lfp=in->getc();
  inbytes++;
  b=lfp;

  while (inbytes<len){
    n=UUDECODE(b);
    memset(&p[0], 0, 61);
    in->blockread(&p[0], 61  );
    inbytes=inbytes+61;
    lp=0;
    for(; n > 0; lp += 4, n -= 3){
      c1 = UUDECODE(p[0+lp]) << 2 | UUDECODE(p[1+lp]) >> 4;
      c2 = UUDECODE(p[1+lp]) << 4 | UUDECODE(p[2+lp]) >> 2;
      c3 = UUDECODE(p[2+lp]) << 6 | UUDECODE(p[3+lp]);
      if(n >= 1)
        ptr[olen++]=c1;
      if(n >= 2)
        ptr[olen++]=c2;
      if(n >= 3)
        ptr[olen++]=c3;
   }
   b=in->getc(); //read lf
   inbytes++;
  }

  ptr[0]=info&255; //special flag for space or '`'
  ptr[1]=len&255;
  ptr[2]=len>>8&255;
  ptr[3]=len>>16&255;
  if (tlf!=0) {
    if (tlf==10) ptr[4]=128;
    else ptr[4]=64;
  }
  else
      ptr[4]=len>>24&63; //1100 0000
  out->blockwrite(&ptr[0],   olen  );
}

// WIT
const char UTF8bytes[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 
    3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

// utf8 len
int utf8len(char *s){
    return UTF8bytes[(U32)(U8)s[0]] + 1;
}

void wfgets(char *str, int count, File  *fp) {
    int c, i = 0;
    while (i<count-1 && ((c=fp->getc())!=EOF)) {
        str[i++]=c;
        if (c=='\n')
            break;
    }
    str[i]=0;
}
/*
  void wfputs(   char *str,File *fp) {
    int c, i = 0;
i = strlen(str);
fp->blockwrite((U8*)  &str[0],   i  );
   
}
void wfputs1(   char *str,FileTmp *fp) {
    int c, i = 0;
i = strlen(str);
fp->blockwrite((U8*)  &str[0],   i  ); // killer of pagefaults
   
}*/
void wfputs(char *str,File *fp) {
    for (int i = 0; *str; str++){
        fp->putc(*str);
    }
}
void wfputs1(char *str,FileTmp *fp) {
    for (int i = 0; *str; str++){
        fp->putc(*str);
    }
}
// UTF8 to WC (USC) and reverse
int utf8towc(char *dest, U32 ch) {
    int val=0;
    if (ch==1) {
        val=(char)dest[0] ;
        return val;
    }
    if (ch==2) {
        val=dest[1]&0x3F;
        val=val|(dest[0]&0x1F)<<6;
        return val;
    }
    if (ch==3) {
        val|=(dest[0]&0x1F)<<12;
        val|=(dest[1]&0x3F)<<6;
        val=val|(dest[2]&0x3F);
        return val;
    }
    if (ch==4) {
        val|=(dest[0]&0xF)<<18;
        val|=(dest[1]&0x3F)<<12;
        val|=(dest[2]&0x3F)<<6;
        val=val|(dest[3]&0x3F);
        return val;
    }
    return 0;
}
int wctoutf8(char *dest, U32 ch){
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

// get num string length terminated by ; or non number
int numlen(char *str) {
    int c, i = 0;
    for (i = 0; *str!=';'; str++){
        if (*str<'0' || *str>'9') return 0;
        i++;
    }   
    return i;
} 
// reduce
//re3
void hent1(char *in,char *out){
    int j, k;
    do {
    j=*in++; *out++=j;
    if (j=='&') {
        k=*in++;
        if (k=='&')  *(int*)out=0x3B706D61, out+=4;  else//';pma'
        if (k=='"')  *(int*)out=0x746F7571, out+=4, *out++=';';  else//'touq'
        if (k=='<')  *(int*)out=0x203B746C, out+=3;  else//' ;tl'
        if (k=='>')  *(int*)out=0x203B7467, out+=3;//' ;tg'
        else   *out++=k;
    }
  }
  while (j!=0);
}

void hent6(char *in,char *out){
    int j, k;
    do {
    j=*in++; *out++=j;

    if (j=='&') {
        k=*in++;
        if (k==5){// escape char
            k=utf8len(in);
            int nu=utf8towc(in,k);
            *out='&';
            sprintf(out+1, "#%d;",   nu);
            int a=numlen(out+1+1);
            out=out+2+a+1;
            in=in+k;
        }     
        else   *out++=k;
    }
  }
  while (j!=0);
}
//re4
void hent3(char *in,char *out) {
    int j, k;
    do {
    j=*in++; *out++=j;
    if (j==';' && *(int*)(in-5)==0x706D6126) { //'pma&'
      k = *in++;
      if (k==3); else
      if (k=='"')  *(int*)out=0x746F7571, out+=4, *out++=';';  else//'touq'
      if (k=='<')  *(int*)out=0x203B746C, out+=3;  else//' ;tl'
      if (k=='>')  *(int*)out=0x203B7467, out+=3;  else//' ;tg'
      if (k=='!')  *(int*)out=0x7073626E, out+=4, *out++=';';  else//'psbn'
      if (k=='*')  *(int*)out=0x7361646E, out+=4, *out++='h', *out++=';'; else//'sadn'
      if (k=='^')  *(int*)out=0x7361646D, out+=4, *out++='h', *out++=';'; else //'sadm'
      if (U8(k)==0xc2 && U8(*(in))==0xae ) {  *(int*)out=0x3B676572, out+=4,in++;}else //'reg'    
      if (U8(k)==0xc2 && U8(*(in))==0xb0 ) {  *(int*)out=0x3B676564, out+=4,in++;}else //'deg'  old=enwik8/9 only
      if (U8(k)==0xc2 && U8(*(in))==0xb2 ) {  *(int*)out=0x32707573, out+=4,*out++=';',in++;}else //'sup2'  // old    
      if (U8(k)==0xc2 && U8(*(in))==0xb3 ) {  *(int*)out=0x33707573, out+=4,*out++=';',in++;}else //'sup3'  // old   
      if (U8(k)==0xe2 && U8(*(in))==0x82&& U8(*(in+1))==0xac ) {  *(int*)out=0x6F727565, out+=4,*out++=';',in+=2;}else //'euro'
      if (U8(k)==0xc3 && U8(*(in))==0x97  ) {  *(int*)out=0x656D6974, out+=4,*out++='s',*out++=';',in++;}else //'times' old
      if (U8(k)==0xe2 && U8(*(in))==0x88&& U8(*(in+1))==0x88 ) {  *(int*)out=0x6E697369, out+=4,*out++=';',in+=2;}else //'isin'
      if (U8(k)==0xe2 && U8(*(in))==0x86&& U8(*(in+1))==0x92 ) {  *(int*)out=0x72726172, out+=4,*out++=';',in+=2;}else //'rarr'
      if (U8(k)==0xe2 && U8(*(in))==0x88&& U8(*(in+1))==0x92 ) {  *(int*)out=0x756E696D, out+=4,*out++='s',*out++=';',in+=2;}  //'minus' old
      else *out++=k;
    }
  }
  while (j!=0);
}
//pre3
void hent(char *in,char *out){
    int j, k;
    do {
    j=*in++; *out++=j;

    if (j=='&') {
      k = *(int*)in;
      if (k==0x3B706D61) { //';pma'
       *out++='&', in+=4; k=*(in);
      if ((k=='"') || (k=='<') || (k=='>') || (k=='!') || (k=='*') || (k=='^') ||
      (U8(k)==0xc2 && U8(*(in+1))==0xb2 ) || 
      (U8(k)==0xc2 && U8(*(in+1))==0xb3 ) || 
      (U8(k)==0xc2 && U8(*(in+1))==0xae )  ||
      (U8(k)==0xc2 && U8(*(in+1))==0xb0 )  ||
      (U8(k)==0xe2 && U8(*(in+1))==0x82&& U8(*(in+1+1))==0xac )  ||
      (U8(k)==0xc3 && U8(*(in+1))==0x97  ) ||
      (U8(k)==0xe2 && U8(*(in+1))==0x88&& U8(*(in+1+1))==0x92 )  ||
      (U8(k)==0xe2 && U8(*(in+1))==0x88&& U8(*(in+1+1))==0x88 )  ||
      (U8(k)==0xe2 && U8(*(in+1))==0x86&& U8(*(in+1+1))==0x92 )  
      
      )    *out++=3; }  else  // sup2, escape if char present
      if (k==0x746F7571 && *(in+4)==';')  *out++='"', in+=5;  else//'touq'
      {
        k = k*256 + ' ';
        if (k==0x3B746C20) *out++='<', in+=3;  else//';tl '
        if (k==0x3B746720) *out++='>', in+=3;//';tg '
      }
    }
  }
  while (j!=0);
}
//pre4
void hent2(char *in,char *out){
    int j, k;
    do {
    j=*in++; *out++=j;

    if (j=='&' && *(in-2)=='&') {
        k=*(int*)in;
        if (k==0x746F7571 && *(in+4)==';')  *out++='"', in+=5;  else//'touq'
        if (k==0x7073626E && *(in+4)==';')  *out++='!', in+=5;  else//'psbn'
        if (k==0x7361646E && *(in+4)=='h' && *(in+5)==';')  *out++='*', in+=6;  else//'sadn'
        if (k==0x7361646D && *(in+4)=='h' && *(in+5)==';')  *out++='^', in+=6;  else//'sadm'
        if (k==0x3B676572  ) { *out++=0xc2,*out++=0xae, in+=4;  }  else//'reg'
        if (k==0x3B676564  ) { *out++=0xc2,*out++=0xb0, in+=4;  }  else//'deg'
        if (k==0x6F727565  && *(in+4)==';' ) { *out++=0xe2,*out++=0x82,*out++=0xac, in+=5;  }  else//'euro'
        if (k==0x32707573  && *(in+4)==';' ) { *out++=0xc2,*out++=0xb2, in+=5;  }  else//'sup2'
        if (k==0x33707573  && *(in+4)==';' ) { *out++=0xc2,*out++=0xb3, in+=5;  }  else//'sup3'
        if (k==0x656D6974  && *(in+4)=='s'&& *(in+5)==';' ) { *out++=0xc3,*out++=0x97, in+=6;  }  else//'times'
        if (k==0x6E697369  && *(in+4)==';' ) { *out++=0xe2,*out++=0x88,*out++=0x88, in+=5;  }  else//'isin'
        if (k==0x72726172  && *(in+4)==';' ) { *out++=0xe2,*out++=0x86,*out++=0x92, in+=5;  }  else//'rarr'
        
        if (k==0x756E696D  && *(in+4)=='s'&& *(in+5)==';' ) { *out++=0xe2,*out++=0x88,*out++=0x92, in+=6;  }  else//'minus'
        {
        k = k*256 + ' ';
        if (k==0x3B746C20)  *out++='<', in+=3;  else//';tl '
        if (k==0x3B746720)  *out++='>', in+=3;//';tg '
        }
    }
  }
  while (j!=0);
}

//&#
void hent5( char *in,char *out){
    int j, k;
    do {
        j=*in++; *out++=j;
        if (j=='&' && *(in)=='#' && *(in-2)=='&' && *(in+1)>'0'&& *(in+1)<='9') { //  &&#xxx; to &@UTF8
            int n=numlen(in+1);
            int d=atoi(&in[1]);
            if (d>255 && n){//>2 && n<6
                in++;
                *(out-1)=5;// escape char
                int e=wctoutf8(out,d);
                out=out+e;
                in=in+n+1;
                // printf("Numlen: %d value: %d utflen: %d\n",n,d,e);
            }
        }
  }
  while (j!=0);
}

void hent9( char *in,char *out){
    int j, k;
     int t=0;
  
#define PROCESS(sym, src, dst, CONDITION) \
  {\
     char *t,  *p = src,  *q = dst,  *end = p + strlen(src);\
    while ( (t=strchr(p, sym)) != 0) {   \
        memcpy(q, p, t-p);  q+=t-p;      \
        int count = 0;                   \
        while(*t++ == sym)  ++count;     \
        if ((CONDITION) && (count==1 || count==2))  count = 3-count;\
        memset(q, sym, count);  q+=count;\
        p = t-1;\
    }\
    memcpy(q, p, end-p);  q[end-p]=0; \
  }
   // breaks wordmodel
   // PROCESS('[', in, out, 1)
  //  PROCESS(']', out, in, 1)
   /* PROCESS('&', in, out, 1)
    do {
           j=*in++; *out++=j;
        }
        while (j!=0); */


}
void skipline( char *in,char *out ){
    int j;
    do {
        j=*in++; *out++=j;
    }
    while (j!=0);
}
// &" -> "   for now
void removeamp( char *in,char *out ,int skip){
    int j;
    for (int i=0;i<skip;i++) {j=*in++; *out++=j;
    }
     do {
        j=*in++; 
        if (j=='"' /*|| j=='<' || j=='>'*/)  { /*assert(p[-1]=='&');*/  --out; }
        *out++=j;
      }
      while (j!=0); 
}
void restoreamp( char *in,char *out,int skip ){
    int j;
    for (int i=0;i<skip;i++) {j=*in++; *out++=j;
    }
     do {
        j=*in++; 
        if (j=='"' /*|| j=='<' || j=='>'*/)  { /*assert(p[-1]=='&');*/  *out++='&'; }
        *out++=j;
      }
      while (j!=0); 
}

void henttail( char *in,char *out,FileTmp *o){
    int c, i=0,  j=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0;
    unsigned char   *ps;
   
    ++lnu;
    j = strlen(in);
    for(i=0; i<j; ++i)  if (*(int*)&in[i]==0x7865743C)  b1 = lnu;//'xet<'
    // parse comment tag
    if (memcmp(&in[6],"<comment>",9)==0 && f==0) co=1;
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x6F632F3C && f==0 && co==1) {//'oc/<'
        co=0,lnu=0,b1=0; // </co mment
        do {
           j=*in++; *out++=j;
        }
        while (j!=0); 
        return;
    }
    if (f==0) {
        
      if (in[0]=='[' && in[1]=='[') {
        ps = (unsigned char*)in + (in[2]==':' ? 1 : 0);
        for (c=2; c<j; ++c)  if ( (in[c]<'a' /*&& in[c]!='-'*/) || in[c]>'z'){
        break;
        } 
         if (c<j && in[c]==':' && !(in[3]==':' || in[2]==':') && co==0) {
            if ((memcmp(&ps[2],"http:",5)==0) ||(memcmp(&ps[2],"https:",6)==0) ||
              (memcmp(&ps[2],"user:",5)==0) || (memcmp(&ps[2],"User:",5)==0) || 
              (memcmp(&ps[2],"media:",6)==0) ||
              (memcmp(&ps[2],"File:",5)==0) || (memcmp(&ps[2],"file:",5)==0) ||
              (memcmp(&ps[3], "mage:",5)==0) ||
              (memcmp(&ps[3], "ategory:",8)==0) ||
              (memcmp(&ps[3], "iktionary:",10)==0) ||
              (memcmp(&ps[3], "ikipedia:",9)==0) ||
              (memcmp(&ps[2],"Kategoria:",10)==0) ||
              (memcmp(&ps[7],     "gorie:",6)==0) ||
              (memcmp(&ps[2],"imagem:",7)==0) ||
              (memcmp(&ps[2],"wikt:",5)==0) ||
              (memcmp(&ps[2],"Categor",7)==0) ||
              (memcmp(&ps[2],"archivo:",8)==0) ||
              (memcmp(&ps[2],"imagen:",7)==0) ||
              (memcmp(&ps[2],"Archivo:",8)==0) ||
              (memcmp(&ps[2],"Wikiproyecto:",13)==0) ||
              (memcmp(&ps[2],"Utente:",7)==0) || (memcmp(&ps[2],"utente:",7)==0) ||
              (memcmp(&ps[2],":Immagine:",10)==0) ||
              (memcmp(&ps[2],"plik:",5)==0) ||
              (memcmp(&ps[2],"iarchive:",9)==0) ||
              (memcmp(&ps[2],"Datei:",6)==0) ||(memcmp(&ps[2],"datei:",6)==0) ||
              (memcmp(&ps[2],"commons:",8)==0) ||
              (memcmp(&ps[2],"wikisource:",11)==0) ||
              (memcmp(&ps[2],"doi:",4)==0) ||
              (memcmp(&ps[2],"fichier:",8)==0) ||
              (memcmp(&ps[2],"utilisateur:",12)==0) ||
              (memcmp(&ps[2],"hdl:",4)==0) ||
              (memcmp(&ps[2],"irc:",4)==0) ||
              (memcmp(&ps[2],"wikibooks:",10)==0) ||    
              (memcmp(&ps[2],"meta:",5)==0) || 
              (memcmp(&ps[2],"categoria:",10)==0) || 
              (memcmp(&ps[2],"immagine:",9)==0) || 
              (memcmp(&ps[3], "ikipedysta:",11)==0) || 
              (memcmp(&ps[2],"wikia:",6)==0) || 
              (memcmp(&ps[2],"incubator:",10)==0) || 
              (memcmp(&ps[2],"ficheiro:",9)==0) || (memcmp(&ps[2],"Ficheiro:",9)==0) ||
              (memcmp(&ps[2],"arquivo:",8)==0) ||
              (memcmp(&ps[2],"foundation:",11)==0) ||
              (memcmp(&ps[2],"template:",9)==0) ||
              (memcmp(&ps[2],"wikinews:",9)==0) ||
              (memcmp(&ps[2],"bild:",5)==0) ||
              (memcmp(&ps[2],"fr:Wikipédia:Aide]]",20)==0) ||
              (memcmp(&ps[2],"de:Boogie Down Produ",20)==0) ||
              (memcmp(&ps[2],"da:Wikipedia:Hvordan",20)==0) ||
              (memcmp(&ps[2],"sv:Indiska musikinstrument",26)==0) ||
              (memcmp(&ps[2],"es:Coronel Sanders",18)==0) ||
             ((memcmp(&ps[2],"fr:Wikip",8)==0)  &&  (memcmp(&ps[10+2],"dia:Aide",8)==0) ) ||
              (memcmp(&ps[2],"pt:Wikipedia:Artigos cu",23)==0) ||
              (lnu-b1<4) ){
                // skip if not lang at end
                do {
                 j=*in++; *out++=j;
                 }
                 while (j!=0); 
                 return;
            }
            f=1, lc=0;
            ++lc;
            for(i=0; i<j; i++)  if (*(int*)&in[i]==0x65742F3C && (*(in+4+i+2)=='>'))  f=0,lnu=0,b1=0;//'et/<'
            hent9(in,out);
            wfputs1(in,o);
            out[0]=0;
            return;
        }
      }
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      return;
    }
    else if (f==1){
     ++lc;
     for(i=0; i<j; i++)  if (*(int*)&in[i]==0x65742F3C)  f=0,lnu=0,b1=0;//'et/<'
     hent9(in,out);
     wfputs1(in,o);
     out[0]=0;
    }
}

void henttail1( char *in,char *out,FileTmp *o,char *p2,int size){
    int i, j, k;
    static int c=0 ,lnu=0, f=0;
    static  char *p4=p2;
    char su[8192*8];
    char *s=su;
    char ou[8192*8];
    ++lnu;
    j = strlen(in);
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x7865743C) c=1, f=lnu;//'xet<'
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x65742F3C && (*(in+4+i+2)=='>' )) c=0; //'et/<' </te xt>

    if ((memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</sha1>",7)==0)&& c==1 && (lnu-f>=4) /*&& *p4!=0&& (p4-p2)<size*/) {
        c=0;
        do {
          k=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k;i++ ){
            su[i]=*p4++;
          }
          su[k]=0;
          // parse line 
          hent9(s,ou);
          if (!(strstr(s, "</text>") || strstr(s, "</revision>")|| strstr(s, "</page>")|| strstr(s, "</sha1>"))) {
            /*  int skip=0;
        char *p = strstr(s, "<text ");//, *w;
        if (p)  {   p = strchr(p, '>'); assert(p);  
        if(p[-1]=='/' || p[2]==0)  ;  
        else  skip= (char*)p-(char*)s; }*/
        
          restoreamp(s,ou,0);
          skipline(ou,s);
          } 
          
          hent6( s,ou);
          hent1( ou,s);
          hent3( s,ou);
          
          wfputs1(ou,o);
        }
        while (memcmp(&p4[-8],"</text>",7) );
        wfputs1(in,o); // out current line
        out[0]=0;
    }else{
        do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
    }
}

// combine this
// ,
void henttag1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;

    j = strlen(in);
 
    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=0;
      
    if (title==1 && move==0 && text)   {
       move=1;
    }    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
             k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
        } else out[0]=0;
        wfputs1(in+k1,o);
        in[0]=0;
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}

int henttag1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int c=0 ,lnu=0, f=0,move=0,text=0;;
    static  char *p4=p2;
    char *p8=out;

    j = strlen(in);
  
    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=k=f=0,move=1;
    if (title==1 && move==0 && text)   {
       if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0/*||memcmp(&in[j-8],"</text>",7)==0*/){
            move=1;text=0;
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      return 0; 
        }
    }
  
    
    if (move==0 && title && text && j){
        if (f){
        // text line only
        for (int i = 0; i<k;i++ ){
            out[i]=*in++;
          }
          f=0;
      }
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=0;
            return 0;
        } 
        return 1;
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      
      return 0;
}

//file
void hentfiles1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=0;
      
    if (title && move==0 && text) {
       move=1;
    }
    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
             k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
            
        } else out[0]=0;
        wfputs1(in+k1,o);in[0]=0;

    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}
// :
int hentfiles1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);
  
    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=k=f=0,move=1;
    if (title  && move==0 && text)   {
       if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0||memcmp(&in[j-8],"</text>",7)==0){
            move=1;text=0;
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      return 0; 
        }
    }  
    
    if (move==0 && title && text && j){
        if (f){
        // text line only
        for (int i = 0; i<k;i++ ){
            out[i]=*in++;
          }
          f=0;
      
      }
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      
      return 0;
}

//numbers
void hentnumbers1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=0;
      
    if (title && move==0 && text) {
       move=1;
    }
    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
             k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
            
        } else out[0]=0;
        wfputs1(in+k1,o);in[0]=0;

    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}
// :
int hentnumbers1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);
  
    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=k=f=0,move=1;
    if (title  && move==0 && text)   {
       if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0||memcmp(&in[j-8],"</text>",7)==0){
            move=1;text=0;
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      return 0; 
        }
    }  
    
    if (move==0 && title && text && j){
        if (f){
        // text line only
        for (int i = 0; i<k;i++ ){
            out[i]=*in++;
          }
          f=0;
      
      }
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
      
      return 0;
}

//alb
void hentalb1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;

    if (title && move==0 && text ) {
       move=1;
    }
    
    if (move &&  j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
            
        } else {
        out[0]=0;
        wfputs1(in+k1,o);in[0]=0;
}
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}

int hentalb1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  move=0,text=0;
    static  char *p4=p2;
   // char *p8=out;
    j = strlen(in);
  
    if (title&& move==1 && text && j){
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=0;
            return 0;
        } 
        return 1;    
    }
    else{
    if (j>13&&  size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "Album infobox |"))) {
    text=move=1;
    do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
       return 1;
    }
    
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
    }  
      return 0;
}

void hentdis1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    //char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;

    if (title && move==0 && text ) {
       move=1;
    }
    
    if (move &&  j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
            
        } else {
        out[0]=0;
        wfputs1(in+k1,o);in[0]=0;
}
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}
int hentdis1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  move=0,text=0;
    static  char *p4=p2;
    //char *p8=out;
    j = strlen(in);
  
    if (title&& move==1 && text && j){
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=0;
            return 0;
        } 
        return 1;    
    }
    else{
    if (j>13&& size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "Disorder infobox |"))) {
    text=move=1;
    do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
       return 1;
    }
    
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
    }  
      return 0;
}

void hentclu1( char *in,char *out,FileTmp *o,int title){
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;

    if (title && move==0 && text ) {
       move=1;
    }
    
    if (move &&  j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0){
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
            
        } else {
        out[0]=0;
        wfputs1(in+k1,o);in[0]=0;
}
    }
    else
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
}
int hentclu1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);
  
    if (title&& move==1 && text && j){
       int k1=(int)(strchr(p4,10)+1-(char*)p4);
          for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
          }
          out[k+k1]=0;
       j = strlen(out);
       if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    }
    else{
    if (j>13&& size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "club infobox |"))) {
    text=f=1,move=1;
    do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
       return 1;
    }
    
      do {
        j=*in++; *out++=j;
      }
      while (j!=0); 
    }  
      return 0;
}
// above needs global vars to remove some functions (less code)
// 
//wit restore
int decode_txt_wit(File*in, U64 size, File*out, FMode mode, U64 &diffFound,int winfo){
    FileTmp out1;
    char s[8192*8];
    char o[8192*8];
    int i, j, f = 0, lastID = 0,tf=0;
    U64 tsize=size-winfo; // winfo <- tail lenght
    in->setpos(tsize); // tail data pos
    //header
    wfgets(s, 16, in);    
    int headerlenght=atoi(&s[0]);
    char *h1=(char*)calloc(headerlenght+1,1);
    char *h1p=h1;
    // tag
    wfgets(s, 16, in);    
    int taglenght=atoi(&s[0]);
    char *t1=(char*)calloc(taglenght+1,1);
    // files
    wfgets(s, 16, in);    
    int fileslenght=atoi(&s[0]);
    char *f1=(char*)calloc(fileslenght+1,1);
    // numbers
    wfgets(s, 16, in);    
    int numberslenght=atoi(&s[0]);
    char *n1=(char*)calloc(numberslenght+1,1);
    //lang
    wfgets(s, 16, in);    
    int langlenght=atoi(&s[0]);
    char *p1=(char*)calloc(langlenght+1,1);
    //alb
    wfgets(s, 16, in);    
    int alblenght=atoi(&s[0]);
    char *a1=(char*)calloc(alblenght+1,1);
    //diso
    wfgets(s, 16, in);    
    int disolenght=atoi(&s[0]);
    char *d1=(char*)calloc(disolenght+1,1);
    //clu
    wfgets(s, 16, in);    
    int clulenght=atoi(&s[0]);
    char *l1=(char*)calloc(clulenght+1,1);
    
    if(clulenght)     in->blockread((U8*)l1,U64(clulenght));  //read clu
    if(disolenght)    in->blockread((U8*)d1,U64(disolenght));  //read dis
    if(alblenght)     in->blockread((U8*)a1,U64(alblenght));  //read alb
    if(fileslenght)   in->blockread((U8*)f1,U64(fileslenght));  //read files
    if(numberslenght) in->blockread((U8*)n1,U64(numberslenght));  //read numbers
    if(taglenght)     in->blockread((U8*)t1,U64(taglenght));  //read sens    
    if(headerlenght)in->blockread((U8*)h1,U64(headerlenght));  //read header
    if(langlenght)  in->blockread((U8*)p1,U64(langlenght));  //read lang
    
    in->setpos(0);
    int header=0,title=0,files=0,number=0,alb=0,diso=0,clu=0;
    do {
        wfgets(s, 8192*8, in);    
        
        j = strlen(s);
        if (in->curpos() > tsize ) {
            j=j-(in->curpos()-tsize); // cut tail
            s[j]=0;
        }
        
        if (header==1){
            int n=0;
            int cont=0;
            do {
                *(int*)o = 0x20202020;
                j=4;
                //'ider' 'iver' 'tser'
                if (*(int*)&h1p[0]!=0x69646572 &&*(int*)&h1p[0]!=0x69766572&& *(int*)&h1p[0]!=0x74736572 && n!=0) *(int*)(o+j)= 0x20202020,j=j+2;
                if (cont==1) *(int*)(o+j)= 0x20202020,j=j+2;
                if (*(int*)&h1p[0]==0x6E6F632F) cont=0;//'noc/'
                int k=(int)(strchr(h1p,10)+1-(char*)h1p);

                // id  
                if ( n==0){
                    if ( (*(int*)&h1p[0]&0xffffff)==0x3E736E){//'>sn'
                        o[j]='<';
                        memcpy(o+j+1, h1p, k);
                        int e=(strchr(h1p, '>')-h1p)+2;
                        if (e!=k || cont==1) { //end tag
                            o[k+j++]='<';
                            o[k+j++]='/';
                            memcpy(o+j+k, h1p, k);
                            j=j+(strchr(h1p, '>')-h1p)+1;
                        }
                        o[k+j++]=10;
                        o[k+j]=0;
                        wfputs1(o,&out1);
                    } else{
                        
                        n++;
                        lastID = lastID+ atoi(&h1p[1]);
                        sprintf(o+j, "<id>%d</id>%c",   lastID, 10);
                        wfputs1(o,&out1);
                    }
                }
                else if (*(int*)&h1p[0]==0x656D6974 ){//'emit'
                    char *p = strchr(h1p, ':');
                    int d = atoi(&h1p[19-7]), hms = atoi(p+1), h = hms/3600;
                    o[0]=h1p[17-7],o[1]=h1p[18-7],o[2]=' ';
                    int y=atoi(&o[0]);
                    sprintf(o, "      <timestamp>%d-%02d-%02dT%02d:%02d:%02dZ</timestamp>%c",
                    y + 2001, d/31+1, d%31+1, h, hms/60 - h*60, hms%60, 10);
                    wfputs1(o,&out1);  
                }
                else if (n || cont==1){
                    o[j]='<';
                    memcpy(o+j+1, h1p, k);
                    int e=(strchr(h1p, '>')-h1p)+2;
                    if (e!=k || cont==1) { //end tag
                        o[k+j++]='<';
                        o[k+j++]='/';
                        memcpy(o+j+k, h1p, k);
                        j=j+(strchr(h1p, '>')-h1p)+1;
                        
                    }
                    o[k+j++]=10;
                    o[k+j]=0;
                    wfputs1(o,&out1);
                    if (*(int*)&h1p[0]==0x746E6F63) cont=1;//'tnoc'
                }
                h1p=h1p+k;
                if (memcmp(&h1p[0],"contributor dele",16)==0)break;
            }
            while (memcmp(&h1p[0],"/contributor>",13)    );
            *(int*)o = 0x20202020;
            j=4;
            *(int*)(o+j)= 0x20202020,j=j+2;
            int k=(int)(strchr(h1p,10)+1-(char*)h1p);
            o[j]='<';
            memcpy(o+j+1, h1p, k);
            o[k+j++]=10;
            o[k+j]=0;
            wfputs1(o,&out1);
            h1p=h1p+k;
            header=0;
        }

        {
            if (memcmp(&s[j-9],"</title>",8)==0 && *(int*)s==0x20202020) {header=1,title= files=number=0;
                for(i=0; i<j; i++) {

                    if (s[i]==',' )  title=1;
                    if (s[i]==':' )  files=2;
                    char *listsp = strstr(s, "List of ");
                    if (listsp)files=2;
                    if (s[i]=='>' && s[i+1]!=10 && s[i+1]>'0'&& s[i+1]<='9'/*&& atoi(&s[i+1])*/)  {
                        number = atoi(&s[i+1]);
                        //int nlen=numlen(&s[i+1]);
                        if (number<10 )number=0;//printf("%d, %d, %s",number,nlen,s);
                        else {number=1;break;}
                    } 
                }
                if (title && files==2) title=0;//,printf("%s",s); // active reset to files only
                if (files==1) files=0;
                if (number) title=files=0;
            }
        }
        if (strstr(s, "<text ") && strstr(s, "Disorder infobox |")) diso=1,number= title=files=0;
        if (strstr(s, "<text ") && strstr(s, "Album infobox |")) alb=1,number= title=files=0;
        if (strstr(s, "<text ") && strstr(s, "club infobox |")) clu=1,number= title=files=0;
        int m=1,n=1,q=1,w=1,z=1,x=1;
        while (m||n||q||w||z||x){        // loop over until not
            m=henttag1r(s,o,t1,taglenght,title);
            n=hentfiles1r(o,s,f1,fileslenght,files);
            q=hentnumbers1r(s,o,n1,numberslenght,number);
            w=hentalb1r(o,s,a1,alblenght,alb);
            z=hentdis1r(s,o,d1,disolenght,diso);
            x=hentclu1r(o,s,l1,clulenght,clu);
            skipline(s,o);
            hent9(o,s);
            henttail1(o,s,&out1,p1,langlenght);
            if (s[0]==0) {   break;       }
            
            int skip=0;
            char *p = strstr(s, "<text ");//, *w;
            if (p)  { tf=1, p = strchr(p, '>'); assert(p);  skip= (char*)p+1-(char*)s; 
                if(p[-1]=='/' /*|| p[2]==0*/) tf=0;  
                
            }
            
            if (strstr(s, "</text>") || strstr(s, "</revision>")|| strstr(s, "</page>")|| strstr(s, "</sha1>"))  tf=alb=diso=clu=0;
            if (tf) {
                restoreamp(s,o,skip);
                skipline(o,s);
            }
            hent6( s,o);
            hent1( o,s);
            hent3( s,o);
            wfputs1(o,&out1);
            // printf("%s",o );
        }
        
    }
    while (in->curpos() < tsize);
    if(taglenght)   free(t1);
    if(fileslenght)   free(f1);
    if(numberslenght)  free(n1);
    if(headerlenght)free(h1);
    if(langlenght)  free(p1);
    if(alblenght)  free(a1);    
    if(disolenght)  free(d1);
    if(clulenght)  free(l1);
    int b=0,c=0;
    U64 bb=0L;
    bb=out1.curpos();
    out1.setpos(0);
    for ( U64 i=0L; i<bb; i++) {
        b=out1.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            c=out->getc();
            if (b!=c && !diffFound) diffFound=i;
        }
    }
    out1.close();
    in->setend(); // or it fails
    return bb; 
}

int encode_txt_wit(File* in, File* out, U64 len) {
    char s[8192*8];
    char o[8192*8];
    FileTmp out1; // lang
    FileTmp out2; // tag
    FileTmp out3; // header
    FileTmp out4; // files
    FileTmp out5; // number
    FileTmp out6; // alb
    FileTmp out7; // dis
    FileTmp out8; // clu
    int i, j, f = 0, lastID = 0,tf=0;
    int ti=0,files=0,number=0,alb=0,diso=0,clu=0;
  do {
    wfgets(s, 8192*8, in);
    j = strlen(s);
    if (f==2) {
        if (*(int*)&s[4]==0x3E736E3C) { // ns '>sn<'
            char *p =strchr(s, '>');
            if (p) {
                p = strchr(p+1, '<');
                if (p)  p[0] = 10,  p[1] = 0;
            }
            if (s[0]!=' ' ||s[1]!=' ' ||s[2]!=' ' ||s[3]!=' ')
                return 0;// just fail
            wfputs1(s+5,&out3);
            continue;
        }
        // id
        int curID = atoi(&s[8]);
        if (*(int*)&s[4] != 0x3E64693C) return 0;// just fail it '>di<'
        //if (curID <= lastID)        return 0;
       sprintf(o,  ">%d%c", curID - lastID, 10);
        wfputs1(o,&out3);
        lastID = curID;
        f = 1;
        continue;
    }
    if (f) {
        if (*(int*)&s[6]==0x6D69743C) {//'mit<'
            int year   = atoi(&s[17]);
            int month  = atoi(&s[22]);
            int day    = atoi(&s[25]);
            int hour   = atoi(&s[28]);
            int minute = atoi(&s[31]);
            int second = atoi(&s[34]);
            sprintf(o, "timestamp>%02d%d:%d%c", 
                    year-2001, month*31+day-32, hour*3600+minute*60+second, 10);
            wfputs1(o,&out3);
            continue;
        }
         char *p =strchr(s, '>');
        if (p) {
            p = strchr(p+1, '<');
            if (p)  p[0] = 10,  p[1] = 0;
        }
        if (s[0]!=' ' ||s[1]!=' ' ||s[2]!=' ' ||s[3]!=' ')
          return 0;// just fail
        int s2=0; //lenght
        if (f==3) {
            if (*(int*)&s[6]==0x6F632F3C)  s2=7;//'oc/<'
            else                       s2=9;
        }
        else {
          if (*(int*)&s[4]==0x7665723C || *(int*)&s[4]==0x7365723C|| *(int*)&s[4]==0x6465723C) s2=5;//'ver<' 'ser<' 'der<'
          else                                             s2=7;
          if (*(int*)&s[6]==0x6E6F633C) {//'noc<'
           f=3;
           if (f==3 && *(int*)&s[6+4+4+4]==0x6C656420)  f=0; //'led 'special case "deleted"
          }
        }
        if (s2){
            wfputs1(s+s2,&out3);
        }
    }
    else  {hent(s,o);
        hent2(o,s);
        hent5(s,o);
        
        int skip=0;
        char *p = strstr(o, "<text ");
        if (p)  { tf=1, p = strchr(p, '>'); assert(p);  skip= (char*)p+1-(char*)o;
        if(p[-1]=='/' /*|| p[2]==0*/) tf=0;  
        if (strstr(o, "Disorder infobox |")) diso=1,ti=files=number=0;
        if (strstr(o, "Album infobox |")) alb=1,ti=files=number=0;
        if (strstr(o, "club infobox |")) clu=1,ti=files=number=0;
        }
        
        if (strstr(o, "</text>") || strstr(o, "</revision>")|| strstr(o, "</page>")|| strstr(o, "</sha1>"))  tf=0;
        if (tf) {
        removeamp(o,s,skip);
        skipline(s,o);
        }
        henttail(o,s,&out1);
        
        henttag1(s,o,&out2,ti);     if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hentfiles1(s,o,&out4,files);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hentnumbers1(s,o,&out5,number);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hentalb1(s,o,&out6,alb);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hentdis1(s,o,&out7,diso);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hentclu1(s,o,&out8,clu);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
        hent9(o,s);
        wfputs(o,out);
}
    for(i=0; i<j; i++)
      if (*(int*)&s[i]==0x69742F3C && *(int*)&s[i+4]==0x3E656C74&&*(int*)s==0x20202020) {f=2;//'it/<' '>elt'
      ti=files=number=0;
       alb=diso=clu=0;
      for(int i=0; i<j; i++){
      if (s[i]==',' )  ti=1;      
      if (s[i]==':' )  files=2;// article about files
      char *listsp = strstr(s, "List of ");
      if (listsp)files=2;
       if (s[i]=='>' && s[i+1]!=10 && s[i+1]>'0'&& s[i+1]<='9')  {
       number = atoi(&s[i+1]);
       if (number<10  )number=0;
       else {number=1;break;}
       } 
      }
      if (ti && files==2) ti=0;
      if (number) ti=files=0;
      if (alb ||diso||clu)ti=files=number=0;
  }
    for(i=0; i<j; i++)
      if (*(int*)&s[i]==0x6F632F3C && *(int*)&s[i+4]==0x6972746E) f=0;//'oc/<' 'irtn'
  }
  while (!in->eof());
  // output tail to main file and report tail size as info
  U64 msize=out->curpos();
  int tsize=out1.curpos();
  int tagsize=out2.curpos();
  int headersize=out3.curpos();
  int filesize=out4.curpos();
  int numbersize=out5.curpos();
  int albsize=out6.curpos();
  int disosize=out7.curpos();  
  int clusize=out8.curpos();
   /*FileDisk aaa,bbb,ccc,ddd,eee,fff,ggg;
   aaa.create("xxxxxxxtag");
   bbb.create("xxxxxxxlang");
   ccc.create("xxxxxxxheader");
   ddd.create("xxxxxxxfiles");
   eee.create("xxxxxxxalb");
   fff.create("xxxxxxxdiso");
   ggg.create("xxxxxxxclu");*/
   out1.setpos(0);
   out2.setpos(0);
   out3.setpos(0);
    out4.setpos(0);
    out5.setpos(0);
    out6.setpos(0);
    out7.setpos(0);
    out8.setpos(0);
   sprintf(o, "%d%c", headersize, 10);
   j=strlen(o);
   wfputs(o,out); //header
   sprintf(o, "%d%c", tagsize, 10);
   j=j+strlen(o);
   wfputs(o,out); //tag
   sprintf(o, "%d%c", filesize, 10);
   j=j+strlen(o);
   wfputs(o,out); //numbersize
   sprintf(o, "%d%c", numbersize, 10);
   j=j+strlen(o);
   wfputs(o,out); //filesize
   sprintf(o, "%d%c", tsize, 10);
   j=j+strlen(o);
   wfputs(o,out); //lang
   sprintf(o, "%d%c", albsize, 10);
   j=j+strlen(o);
   wfputs(o,out); //alb
   sprintf(o, "%d%c", disosize, 10);
   j=j+strlen(o);
   wfputs(o,out); //diso
   sprintf(o, "%d%c", clusize, 10);
   j=j+strlen(o);
   wfputs(o,out); //clu
   for(U64 i=0; i<clusize; i++) {
       int a=out8.getc();
       out->putc(a);
   //      ggg.putc(a);
   }
   for(U64 i=0; i<disosize; i++) {
       int a=out7.getc();
       out->putc(a);
   //      fff.putc(a);
   }
   for(U64 i=0; i<albsize; i++) {
       int a=out6.getc();
       out->putc(a);
   //      eee.putc(a);
   }
   for(U64 i=0; i<filesize; i++) {
       int a=out4.getc();
       out->putc(a);
   //     ddd.putc(a);
   }
   
   for(U64 i=0; i<numbersize; i++) {
       int a=out5.getc();
       out->putc(a);
   //     ddd.putc(a);
   }
   for(U64 i=0; i<tagsize; i++) {
       int a=out2.getc();
       out->putc(a);
   //     aaa.putc(a);
   }

   for(U64 i=0; i<headersize; i++) {
       int a=out3.getc();
       out->putc(a);
   //     ccc.putc(a);
   }
   
   for(U64 i=0; i<tsize; i++) {
       int a=out1.getc();
       out->putc(a);
    //    bbb.putc(a);
   }
   
  //aaa.close(); bbb.close();ccc.close();ddd.close();eee.close();fff.close();ggg.close();
   out8.close();
   out7.close();
   out6.close();
   out5.close();
   out4.close();
   out3.close();
   out2.close();
   out1.close();
  /*printf("Main size: %d kb\n",U32(msize/1024));
  printf("tags size: %d kb\n",U32(tagsize/1024));
  printf("file size: %d kb\n",U32(filesize/1024));
  printf("number size: %d kb\n",U32(numbersize/1024));
  printf("header size: %d kb\n",U32(headersize/1024));
  printf("Langs size: %d kb\n",U32(tsize/1024));
  printf("Alb size: %d kb\n",U32(albsize/1024));
  printf("Diso size: %d kb\n",U32(disosize/1024));
  printf("Clu size: %d kb\n",U32(clusize/1024));*/
  tsize=tsize+j+tagsize+headersize+filesize+numbersize+albsize+disosize+clusize;
  return tsize;
}
// end WIT

// decode/encode base64 
static const char  table1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
bool isbase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/')|| (c == 10) || (c == 13));
}

int decode_base64(File*in, int size, File*out, FMode mode, U64 &diffFound){
    U8 inn[3];
    int i, len=0, blocksout = 0;
    int fle=0;
    int linesize=0; 
    int outlen=0;
    int tlf=0,g=0;
    linesize=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> ptr((outlen>>2)*4+10);
    tlf=(tlf&192);
    if (tlf==128)       tlf=10;        // LF: 10
    else if (tlf==64)   tlf=13;        // LF: 13
    else                tlf=0;
 
    while(fle<outlen){
        len=0;
        for(i=0;i<3;i++){
            int c=in->getc();
            if(c!=EOF) {
                inn[i]=c;
                len++;
            }
            else {
                inn[i] = 0,g=1;
            }
        }
        if(len){
            U8 in0,in1,in2;
            in0=inn[0],in1=inn[1],in2=inn[2];
            ptr[fle++]=(table1[in0>>2]);
            ptr[fle++]=(table1[((in0&0x03)<<4)|((in1&0xf0)>>4)]);
            ptr[fle++]=((len>1?table1[((in1&0x0f)<<2)|((in2&0xc0)>>6)]:'='));
            ptr[fle++]=((len>2?table1[in2&0x3f]:'='));
            blocksout++;
        }
        if(blocksout>=(linesize/4) && linesize!=0){ //no lf if linesize==0
            if( blocksout &&  !in->eof() && fle<=outlen) { //no lf if eof
                if (tlf) ptr[fle++]=(tlf);
                else ptr[fle++]=13,ptr[fle++]=10;
            }
            blocksout = 0;
        }
        if (g) break; //if past eof, break
    }
    //Write out or compare
    if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
        }
    else if (mode==FCOMPARE){
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}
   
inline char valueb(char c){
       const char *p = strchr(table1, c);
       if(p) {
          return p-table1;
       } else {
          return 0;
       }
}

void encode_base64(File* in, File* out, int len) {
  int in_len = 0;
  int i = 0;
  int j = 0;
  int b=0;
  int lfp=0;
  int tlf=0;
  char src[4];
  int b64mem=(len>>2)*3+10;
  Array<U8,1> ptr(b64mem);
  int olen=5;

  while (b=in->getc(),in_len++ , ( b != '=') && is_base64(b) && in_len<=len) {
    if (b==13 || b==10) {
       if (lfp==0) lfp=in_len ,tlf=b;
       if (tlf!=b) tlf=0;
       continue;
    }
    src[i++] = b; 
    if (i ==4){
          for (j = 0; j <4; j++) src[j] = valueb(src[j]);
          src[0] = (src[0] << 2) + ((src[1] & 0x30) >> 4);
          src[1] = ((src[1] & 0xf) << 4) + ((src[2] & 0x3c) >> 2);
          src[2] = ((src[2] & 0x3) << 6) + src[3];
    
          ptr[olen++]=src[0];
          ptr[olen++]=src[1];
          ptr[olen++]=src[2];
      i = 0;
    }
  }

  if (i){
    for (j=i;j<4;j++)
      src[j] = 0;

    for (j=0;j<4;j++)
      src[j] = valueb(src[j]);

    src[0] = (src[0] << 2) + ((src[1] & 0x30) >> 4);
    src[1] = ((src[1] & 0xf) << 4) + ((src[2] & 0x3c) >> 2);
    src[2] = ((src[2] & 0x3) << 6) + src[3];

    for (j=0;(j<i-1);j++) {
        ptr[olen++]=src[j];
    }
  }
  ptr[0]=lfp&255; //nl lenght
  ptr[1]=len&255;
  ptr[2]=len>>8&255;
  ptr[3]=len>>16&255;
  if (tlf!=0) {
    if (tlf==10) ptr[4]=128;
    else ptr[4]=64;
  }
  else
      ptr[4]=len>>24&63; //1100 0000
  out->blockwrite(&ptr[0],   olen  );
}

//base85
int powers[5] = {85*85*85*85, 85*85*85, 85*85, 85, 1};

int decode_ascii85(File*in, int size, File*out, FMode mode, U64 &diffFound){
    int i;
    int fle=0;
    int nlsize=0; 
    int outlen=0;
    int tlf=0;
    nlsize=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> ptr((outlen>>2)*5+10);
    tlf=(tlf&192);
    if (tlf==128)      tlf=10;        // LF: 10
    else if (tlf==64)  tlf=13;        // LF: 13
    else               tlf=0;
    int c, count = 0, lenlf = 0;
    uint32_t tuple = 0;

    while(fle<outlen){ 
        c = in->getc();
        if (c != EOF) {
            tuple |= ((U32)c) << ((3 - count++) * 8);
            if (count < 4) continue;
        }
        else if (count == 0) break;
        int i, lim;
        char out[5];
        if (tuple == 0 && count == 4) { // for 0x00000000
            if (nlsize && lenlf >= nlsize) {
                if (tlf) ptr[fle++]=(tlf);
                else ptr[fle++]=13,ptr[fle++]=10;
                lenlf = 0;
            }
            ptr[fle++]='z';
        }
        /*    else if (tuple == 0x20202020 && count == 4 ) {
            if (nlsize && lenlf >= nlsize) {
                if (tlf) fptr[fle++]=(tlf);
                else fptr[fle++]=13,fptr[fle++]=10;
                lenlf = 0;
            }
            fptr[fle++]='y',lenlf++;
        }*/
        else {
            for (i = 0; i < 5; i++) {
                out[i] = tuple % 85 + '!';
                tuple /= 85;
            }
            lim = 4 - count;
            for (i = 4; i >= lim; i--) {
                if (nlsize && lenlf >= nlsize && ((outlen-fle)>=5)) {//    skip nl if only 5 bytes left
                    if (tlf) ptr[fle++]=(tlf);
                    else ptr[fle++]=13,ptr[fle++]=10;
                    lenlf = 0;}
                ptr[fle++]=out[i],lenlf++;
            }
        }
        if (c == EOF) break;
        tuple = 0;
        count = 0;
    }
    if (mode==FDECOMPRESS){
        out->blockwrite(&ptr[0],   outlen  );
    }
    else if (mode==FCOMPARE){
        for(i=0;i<outlen;i++){
            U8 b=ptr[i];
            if (b!=out->getc() && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}

void encode_ascii85(File* in, File* out, int len) {
    int lfp=0;
    int tlf=0;
    int b64mem=(len>>2)*5+100;
    Array<U8,1> ptr(b64mem);
    int olen=5;
    int c, count = 0;
    uint32_t tuple = 0;
    for (int f=0;f<len;f++) {
        c = in->getc();
        if (olen+10>b64mem) {count = 0; break;} //!!
        if (c==13 || c==10) {
            if (lfp==0) lfp=f ,tlf=c;
            if (tlf!=c) tlf=0;
            continue;
        }
        if (c == 'z' && count == 0) {
            if (olen+10>b64mem) {count = 0; break;} //!!
            for (int i = 1; i < 5; i++) ptr[olen++]=0;
            continue;
        }
        /*    if (c == 'y' && count == 0) {
            for (int i = 1; i < 5; i++) fptr[olen++]=0x20;
            continue;
        }*/
        if (c == EOF) {  
        if (olen+10>b64mem) {count = 0; break;} //!!      
            if (count > 0) {
                
                tuple += powers[count-1];
                for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            }
            break;
        }
        tuple += (c - '!') * powers[count++];
        if (count == 5) {
           if (olen>b64mem+10) {count = 0; break;} //!!
            for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            tuple = 0;
            count = 0;
        }
    }
    if (count > 0) {
        
        tuple += powers[count-1];
        for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
    }
    ptr[0]=lfp&255; //nl lenght
    ptr[1]=len&255;
    ptr[2]=len>>8&255;
    ptr[3]=len>>16&255;
    if (tlf!=0) {
        if (tlf==10) ptr[4]=128;
        else ptr[4]=64;
    }
    else
    ptr[4]=len>>24&63; //1100 0000
    out->blockwrite(&ptr[0],   olen  );
}

//SZDD
int decode_szdd(File*in, int size, int info, File*out, FMode mode, U64 &diffFound){
    LZSS* lz77;
    int r=0;
    //Write out or compare
    if (mode==FDECOMPRESS){
            lz77=new LZSS(in,out,size,(info>>25)*2);
             r=lz77->compress();
            delete lz77;
        }
    else if (mode==FCOMPARE){
        FileTmp out1;
        lz77=new LZSS(in,&out1,size,(info>>25)*2);
        r=lz77->compress();
        delete lz77;
        out1.setpos(0);
        for(int i=0;i<r;i++){
            U8 b=out1.getc();
            if (b!=out->getc() && !diffFound) diffFound= out->curpos();
        }
        out1.close();
    }
    return r;
}

void encode_szdd(File* in, File* out, int len) {
    LZSS* lz77;
    lz77=new LZSS(in,out,len&0x1ffffff,(len>>25)*2);
    lz77->decompress();
    delete lz77;
}

//mdf 
int decode_mdf(File*in, int size,  File*out, FMode mode, U64 &diffFound){
    int q=in->getc();   // count of channels
    q=(q<<8)+in->getc();
    q=(q<<8)+in->getc();
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    Array<U8,1> ptr(CHAN*q);
    //Write out or compare
    if (mode==FDECOMPRESS){
        in->blockread(&ptr[0], CHAN*q);
        for (int offset=0; offset<q; offset++) { 
            in->blockread(&blk[0], BLOCK);
            out->blockwrite(&blk[0], BLOCK);
            out->blockwrite(&ptr[offset*CHAN], CHAN);
        }
    }
    else if (mode==FCOMPARE){
        in->blockread(&ptr[0], CHAN*q);
        int offset=0;
        for( int i=3;i<size;){
           in->blockread(&blk[0], BLOCK);
            for(int j=0;j<BLOCK;j++,i++){
                U8 b=blk[j];
                if (b!=out->getc() && !diffFound) diffFound= out->curpos();
            } 
            for(int j=0;j<CHAN;j++,i++){
                U8 b=ptr[offset*CHAN+j];
                if (b!=out->getc() && !diffFound) diffFound= out->curpos();
            }
            offset++;
        }
    }
    return size;
}

void encode_mdf(File* in, File* out, int len) {
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    U8 blk1[CHAN];
    int ql=len/(BLOCK+CHAN);
    out->putc(ql>>16); 
    out->putc(ql>>8);
    out->putc(ql);
    U64 beginin= in->curpos();
    //channel out
    for (int offset=0; offset<ql; offset++) { 
        in->setpos(in->curpos()+  BLOCK);
        in->blockread(&blk1[0],   CHAN);
        out->blockwrite(&blk1[0], CHAN);
    }
    in->setpos( beginin);
    for (int offset=0; offset<ql; offset++) { 
        in->blockread(&blk[0],   BLOCK);
        in->setpos(in->curpos()+ CHAN) ;
        out->blockwrite(&blk[0], BLOCK);
  }
}

#define LZW_TABLE_SIZE 9221

#define lzw_find(k) {\
  offset = ((k)*PHI)>>19; \
  int stride = (offset>0)?LZW_TABLE_SIZE-offset:1; \
  while (true){ \
    if ((index=table[offset])<0){ index=-offset-1; break; } \
    else if (dict[index]==int(k)){ break; } \
    offset-=stride; \
    if (offset<0) \
      offset+=LZW_TABLE_SIZE; \
  } \
}

#define lzw_reset { for (int i=0; i<LZW_TABLE_SIZE; table[i]=-1, i++); }

int encode_gif(File* in, File* out, int len) {
  int codesize=in->getc(),hdrsize=6,clearpos=0,bsize=0,code,offset=0;
  U64 diffpos=0,beginin= in->curpos(),beginout= out->curpos();
  Array<U8,1> output(4096);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(bsize);
  out->putc(clearpos>>8);
  out->putc(clearpos&255);
  out->putc(codesize);
  Array<int> table(LZW_TABLE_SIZE);  
  for (int phase=0; phase<2; phase++) {
    in->setpos( beginin);
    int bits=codesize+1,shift=0,buf=0;
    int blocksize=0,maxcode=(1<<codesize)+1,last=-1;//,dict[4096];
    Array<int> dict(4096);
    lzw_reset;
    bool end=false;
    while ((blocksize=in->getc())>0 &&  in->curpos()-beginin<len && !end) {
      for (int i=0; i<blocksize; i++) {
        buf|=in->getc()<<shift;
        shift+=8;
        while (shift>=bits && !end) {
          int code=buf&((1<<bits)-1);
          buf>>=bits;
          shift-=bits;
          if (!bsize && code!=(1<<codesize)) {
            hdrsize+=4; out->put32(0);
          }
          if (!bsize) bsize=blocksize;
          if (code==(1<<codesize)) {
            if (maxcode>(1<<codesize)+1) {
              if (clearpos && clearpos!=69631-maxcode) return 0;
              clearpos=69631-maxcode;
            }
            bits=codesize+1, maxcode=(1<<codesize)+1, last=-1;
            lzw_reset;
          }
          else if (code==(1<<codesize)+1) end=true;
          else if (code>maxcode+1) return 0;
          else {
            int j=(code<=maxcode?code:last),size=1;
            while (j>=(1<<codesize)) {
              output[4096-(size++)]=dict[j]&255;
              j=dict[j]>>8;
            }
            output[4096-size]=j;
            if (phase==1) out->blockwrite(&output[4096-size],  size  ); else diffpos+=size;
            if (code==maxcode+1) { if (phase==1) out->putc(j); else diffpos++; }
            if (last!=-1) {
              if (++maxcode>=8191) return 0;
              if (maxcode<=4095)
              {
                int key=(last<<8)+j, index=-1;
                lzw_find(key);
                dict[maxcode]=key;
                table[(index<0)?-index-1:offset]=maxcode;
                if (phase==0 && index>0) {
                    hdrsize+=4;
                    j=diffpos-size-(code==maxcode);
                    out->put32(j);
                    diffpos=size+(code==maxcode);
                  }
                }
              //}
              if (maxcode>=((1<<bits)-1) && bits<12) bits++;
            }
            last=code;
          }
        }
      }
    }
  }
  diffpos= out->curpos();
  out->setpos(beginout);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(255-bsize);
  out->putc((clearpos>>8)&255);
  out->putc(clearpos&255);
  out->setpos(diffpos);
  return in->curpos()-beginin==len-1;
}

#define gif_write_block(count) { output[0]=(count);\
if (mode==FDECOMPRESS) out->blockwrite(&output[0],  (count)+1  );\
else if (mode==FCOMPARE) for (int j=0; j<(count)+1; j++) if (output[j]!=out->getc() && !diffFound) diffFound=outsize+j+1;\
outsize+=(count)+1; blocksize=0; }

#define gif_write_code(c) { buf+=(c)<<shift; shift+=bits;\
while (shift>=8) { output[++blocksize]=buf&255; buf>>=8;shift-=8;\
if (blocksize==bsize) gif_write_block(bsize); }}

int decode_gif(File* in, int size, File*out, FMode mode, U64 &diffFound) {
  int diffcount=in->getc(), curdiff=0;
    Array<int> diffpos(4096);//, diffpos[4096];
  diffcount=((diffcount<<8)+in->getc()-6)/4;
  int bsize=255-in->getc();
  int clearpos=in->getc(); clearpos=(clearpos<<8)+in->getc();
  clearpos=(69631-clearpos)&0xffff;
  int codesize=in->getc(),bits=codesize+1,shift=0,buf=0,blocksize=0;
  if (diffcount>4096 || clearpos<=(1<<codesize)+2) return 1;
  int maxcode=(1<<codesize)+1, input,code,offset=0;
    Array<int> dict(4096);
      Array<int> table(LZW_TABLE_SIZE);
  lzw_reset;
  for (int i=0; i<diffcount; i++) {
    diffpos[i]=in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    if (i>0) diffpos[i]+=diffpos[i-1];
  }
  Array<U8,1> output(256);
  size-=6+diffcount*4;
  int last=in->getc(),total=size+1,outsize=1;
  if (mode==FDECOMPRESS) out->putc(codesize);
  else if (mode==FCOMPARE) if (codesize!=out->getc() && !diffFound) diffFound=1;
  if (diffcount==0 || diffpos[0]!=0) gif_write_code(1<<codesize) else curdiff++;
  while (size!=0 && (input=in->getc())!=EOF) {
    size--;
    int key=(last<<8)+input, index=(code=-1);
    if (last<0) index=input; else lzw_find(key);
    code = index;
    if (curdiff<diffcount && total-(int)size>diffpos[curdiff]) curdiff++, code=-1;
    if (code<0) {
      gif_write_code(last);
      if (maxcode==clearpos) { gif_write_code(1<<codesize); bits=codesize+1, maxcode=(1<<codesize)+1; lzw_reset }
      else
      {
        ++maxcode;
        if (maxcode<=4095) { dict[maxcode]=key; table[(index<0)?-index-1:offset]=maxcode; }
        if (maxcode>=(1<<bits) && bits<12) bits++;
      }
      code=input;
    }
    last=code;
  }
  gif_write_code(last);
  gif_write_code((1<<codesize)+1);
  if (shift>0) {
    output[++blocksize]=buf&255;
    if (blocksize==bsize) gif_write_block(bsize);
  }
  if (blocksize>0) gif_write_block(blocksize);
  if (mode==FDECOMPRESS) out->putc(0);
  else if (mode==FCOMPARE) if (0!=out->getc() && !diffFound) diffFound=outsize+1;
  return outsize+1;
}

//EOL

enum EEOLType {UNDEFINED, CRLF, LF};

#define MAX_FREQ_ORDER1 255
#define ORDER1_STEP    4

class RangeCoder{
    U32 code, range, FFNum, Cache;
    U64 low;
    int mZero[MAX_FREQ_ORDER1];
    int mOne[MAX_FREQ_ORDER1];
    File*outeol; 
public:
    inline void ShiftLow(){                                             
        if ((low^0xFF000000)>0xFFFFFF){            
            outeol->putc( Cache + (low>>32));       
            int c = 0xFF+(low>>32);                       
            while( FFNum ) outeol->putc(c), FFNum--; 
            Cache = U32(low)>>24;                        
        } else FFNum++;                               
        low = U32(low)<<8;                           
    }
    
    void StartEncode(File*out ){
        low=FFNum=Cache=0;  
        range=0xffffffff; 
        outeol=out; 
    }
    
    void StartDecode(File*out){ 
        outeol=out; 
        code=0; 
        range=0xffffffff;
        for (int i=0; i<5; i++) code=(code<<8) | outeol->getc();
    }
    
    void FinishEncode(){ 
        for (int i=0; i<5; i++) ShiftLow();
    }
    
    void Encode(U32 cumFreq, U32 freq, U32 totFreq){
        low += cumFreq * (range/= totFreq);
        range*= freq;
        while( range<(1<<24) ) { ShiftLow(); range<<=8; }
    }
    
    inline U32 GetFreq (U32 totFreq) {
        return code / (range/= totFreq);
    }
    void Decode (U32 cumFreq, U32 freq, U32 totFreq){
        code -= cumFreq*range;
        range *= freq;
        while (range<(1<<24)) code=(code<<8)|outeol->getc(), range<<=8;
    }
    
    inline void UpdateOrder1(int prev,int c, int step){
        if (c==0) mZero[prev]+=step;
        else      mOne[prev]+=step;

        if (mZero[prev]+mOne[prev] >= 1<<15){
            mZero[prev]=(mZero[prev]+1)/2;
            mOne[prev]=(mOne[prev]+1)/2;
        }    
    }

    inline void EncodeOrder1(int prev, int c){
        if (c==0)  Encode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else       Encode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
    }

    inline int DecodeOrder1(int prev){
        int c=GetFreq(mZero[prev]+mOne[prev]);

        if (c<mZero[prev]) c=0;
        else c=1;

        if (c==0) Decode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else      Decode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
        return c;
    }

    U32 DecodeOrder(U32 prev){
        U32 result=DecodeOrder1(prev); 
        UpdateOrder1(prev,result,ORDER1_STEP); 
        return result;
    }
    void EncodeOrder(U32 prev, U32 result){
        EncodeOrder1(prev,result); 
        UpdateOrder1(prev,result,ORDER1_STEP); 
    }

    RangeCoder(){
        for (int i=0; i<MAX_FREQ_ORDER1; i++){
            mZero[i]=1;
            mOne[i]=1;
        }
    }
};


#define TOLOWER(c)    ((c>='A' && c<='Z')?(c+32):c)
//#define TOUPPER(c)    ((c>='a' && c<='z')?(c-32):c)
class EOLEncoderCoder{
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int lastEOL,lastChar;
public:
    EOLEncoderCoder (File*out ){
        coder.StartEncode(out);
    }
    inline int ContextEncode(int leftChar,int c,int rightChar,int distance){
        U32 prev,result;

        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z' )
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\''  /*&& leftChar!='>'*/) || rightChar<'a' || rightChar>'z' )
        return c;
        
        if (c==32)
        result=0;
        else
        result=1;

        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        coder.EncodeOrder(prev,result); 
        return 32;
    }
    void EncodeEOLformat(EEOLType EOLType){
        if(EOLType==CRLF)    coder.Encode(0,1,2);
        else     coder.Encode(1,1,2);
    }

    void EOLencode(File* file,File* fileout,int fileLen){
        int xc=0;
        int last_c,c,next_c;
        last_c=0;
        lastEOL=0;
        EOLType=UNDEFINED;
        lastEOL=0;
        c=file->getc(),fpos++;
        fpos=0;
        while ( fpos<fileLen)    {
            next_c=file->getc(),fpos++;
            if (c==32 || c==10 || (c==13 && next_c==10)){
                if (c==13){
                    if (EOLType==CRLF || EOLType==UNDEFINED){
                        c=next_c;
                        if (fpos<fileLen){
                           next_c=file->getc(),fpos++;
                        }
                        else{
                             next_c=0,fpos++;
                        }
                        lastEOL++;
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c){
                            EOLType=CRLF;
                            EncodeEOLformat(EOLType);
                        }
                        lastEOL=fpos;
                        if (last_c==10)  xc=5;//LF marker
                        else xc=last_c;
                    }
                    else
                    xc=c;
                }
                else{
                    if (c==10 && EOLType==CRLF){ 
                        xc=c;
                    }
                    else{
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c){
                            EOLType=LF;
                            EncodeEOLformat(EOLType);
                        }
                        xc=last_c;
                    }
                    if (c==10) lastEOL=fpos;
                }
            }
            else{
               xc=c;
            }
            last_c=c;
            c=xc;  
            fileout->putc(c );
            c=next_c;
        }
        coder.FinishEncode();
    }
};

class EOLDecoderCoder{
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int bufChar,lastEOL,lastChar;
public:
     
    inline int ContextDecode(int leftChar,int rightChar,int distance){
        U32 prev,result;

        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z'  )
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\''/* && leftChar!='>'*/) || rightChar<'a' || rightChar>'z' )
        return 32;

        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        result=coder.DecodeOrder(prev); 
        if (result==0)   return 32;
        else     return 10;
    }

    EEOLType DecodeEOLformat(){
        int c=coder.GetFreq(2);
        if (c<1){
            coder.Decode(0,1,2);        
            return CRLF;
        }
        else{     
            coder.Decode(1,1,2);
            return LF;
        }
    }

    void hook_putc(int c,File* out,int maxlen){
        if (bufChar<0 && c==' '){
            bufChar=c;
            return;
        }
        if (bufChar>=0){            
            bufChar=ContextDecode(TOLOWER(lastChar),TOLOWER(c),fpos-lastEOL);
            if (bufChar==10){
                if (EOLType==UNDEFINED)
                EOLType=DecodeEOLformat();
                if (EOLType==CRLF){
                    lastChar=13;
                    if (fpos==maxlen) return;
                    out->putc(lastChar),fpos++;
                }
                lastEOL=fpos;
            }
            if (fpos==maxlen) return;
            out->putc(bufChar),fpos++;
            if (c==' '){
                lastChar=bufChar;
                bufChar=c;
                return;
            }
            bufChar=-1;
        }
        if (c==10)
        lastEOL=fpos;
        lastChar=c;
        if (c==EOF) return;
        if (fpos==maxlen) return;
        out->putc(c),fpos++;
    }

    void EOLdecode(File* in,File* out,int size,File*outeol,File*wd,int len){
        int c=0;
        bufChar=-1;
        lastEOL=-1;
        EOLType=UNDEFINED;
        fpos=0;
        coder.StartDecode(outeol);
        
        for ( int i=0; i<size; i++) {
            c=wd->getc();
            if (c==5){
                hook_putc(13,out,len);
                hook_putc(10,out,len);
            }
            else {    
                hook_putc(c,out,len);
            }
        }
    }
};

//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com
int encode_txtd(File* in, File* out, int len,int wrtn) {
    U64 eolz=0;
    U64 wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
    
    EOLEncoderCoder* eolc;
    eolc=new EOLEncoderCoder(&wrtfi);
    eolc->EOLencode(in,&tmpout,len); 
    out->put32(len);
    eolz= wrtfi.curpos();
    out->put32(eolz);
    
    wrtz= tmpout.curpos();
    out->put32(wrtz);
    wrtfi.setpos(0);
    for (U64 offset=0; offset<eolz; offset++) { 
        out->putc(wrtfi.getc()); 
   }
    wrtz= tmpout.curpos();
    tmpout.setpos(0);
    for (U64 offset=0; offset<wrtz; offset++) { 
        out->putc(tmpout.getc()); 
    }
    delete eolc;
    wrtfi.close();
    tmpout.close();
   // if (eolz<35) printf("Eol count %d<35\n",eolz);
    return eolz<35;
}

int decode_txtd(File* in, int size, File*out, FMode mode, U64 &diffFound) {
    int b=0;
    U64 bb=0;
    U64 eolz=0,wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
    int len=in->get32();
    eolz=in->get32();
    wrtz=in->get32();
    
    for (U64 offset=0; offset<eolz; offset++) wrtfi.putc(in->getc()); 
    wrtfi.setpos(0);
    EOLDecoderCoder* eold;
    eold=new EOLDecoderCoder(); 
    eold->EOLdecode(in,&tmpout,wrtz,&wrtfi,in,len);

    bb= tmpout.curpos();
    tmpout.setpos(0);
    for ( U64 i=0; i<bb; i++) {
        b=tmpout.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    delete eold;
    tmpout.close();
    wrtfi.close();
    return bb; 
}

//////////////////// Compress, Decompress ////////////////////////////

//for block statistics, levels 0-5
U64 typenamess[datatypecount][5]={0}; //total type size for levels 0-5
U32 typenamesc[datatypecount][5]={0}; //total type count for levels 0-5
int itcount=0;               //level count

void direct_encode_blockstream(Filetype type, File*in, U64 len, int info=0) {
  //assert(s1<(s1+len));
  segment.putdata(type,len,info);
  int srid=streams.GetStreamID(type);
  Stream *sout=streams.streams[srid];
  for (U64 j=0; j<len; ++j) sout->file.putc(in->getc());
}

void DetectRecursive(File*in, U64 n, Encoder &en, char *blstr, int it);

void transform_encode_block(Filetype type, File*in, U64 len, Encoder &en, int info, int info2, char *blstr, int it, U64 begin) {
    if (streams.GetTypeInfo(type)&TR_TRANSFORM) {
        U64 diffFound=0;
        U32 winfo=0;
        FileTmp* tmp;
        tmp=new FileTmp;
        if (type==IMAGE24) img24.encode(in, tmp, len, uint64_t(info));
        else if (type==IMAGE32) img32.encode(in, tmp, len, uint64_t(info));
        else if (type==MRBR) imgmrb.encode(in, tmp, len, uint64_t(info)+(uint64_t(info2)<<32));
        else if (type==MRBR4) imgmrb.encode(in, tmp, len,     uint64_t(((info*4+15)/16)*2)+(uint64_t(info2)<<32));
        else if (type==RLE) encode_rle(in, tmp, len, info, info2);
        else if (type==LZW) encode_lzw(in, tmp, len, info2);
        else if (type==EXE) encode_exe(in, tmp, int(len), int(begin));
        else if (type==DECA) encode_dec(in, tmp, int(len), int(begin));
        else if (type==ARM) encode_arm(in, tmp, int(len), int(begin));
        else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0||type==ISOTEXT) ) {
            if ( type!=TXTUTF8 ){
            encode_txt(in, tmp, (len),1);
            U64 txt0Size= tmp->curpos();
            //reset to text mode
             in->setpos(begin);
            tmp->close();
            tmp=new FileTmp;
            encode_txt(in, tmp, (len),0);
            U64 txtSize= tmp->curpos();
            tmp->close();
            in->setpos( begin);
            tmp=new FileTmp;
            if (txt0Size<txtSize && (((txt0Size*100)/txtSize)<95)) {
                in->setpos( begin);
                encode_txt(in, tmp, (len),1);
                type=TEXT0,info=1;
            }else{
                encode_txt(in, tmp, (len),0);
                type=TEXT,info=0;
            }
            }
            else encode_txt(in, tmp, (len),info&1); 
        }
        else if (type==EOLTEXT ) diffFound=encode_txtd(in, tmp, int(len),info&1);
        else if (type==BASE64) encode_base64(in, tmp, int(len));
        else if (type==UUENC) encode_uud(in, tmp, int(len),info);
        else if (type==BASE85) encode_ascii85(in, tmp, int(len));
        else if (type==SZDD) encode_szdd(in, tmp, info);
        else if (type==ZLIB) diffFound=encode_zlib(in, tmp, int(len))?0:1;
        else if (type==BZIP2) encode_bzip2(in, tmp, len,info);
        else if (type==CD) encode_cd(in, tmp, int(len), info);
        else if (type==MDF) encode_mdf(in, tmp, int(len));
        else if (type==GIF) diffFound=encode_gif(in, tmp, int(len))?0:1;
        else if (type==WIT) winfo=encode_txt_wit(in, tmp, (len));
        if (type==EOLTEXT && diffFound) {
            // if EOL size is below 25 then drop EOL transform and try TEXT type
            diffFound=0, in->setpos(begin),type=TEXT,tmp->close(),tmp=new FileTmp(),encode_txt(in, tmp, int(len),info&1); 
        }
        const U64 tmpsize= tmp->curpos();
        int tfail=0;
        tmp->setpos(0);
        en.setFile(tmp);
        
        if (type==BZIP2 || type==ZLIB || type==GIF || type==MRBR|| type==MRBR4|| type==RLE|| type==LZW||type==BASE85 ||type==BASE64 || type==UUENC|| type==DECA|| type==ARM || (type==WIT||type==TEXT || type==TXTUTF8 ||type==TEXT0)||type==EOLTEXT ){
        int ts=0;
         in->setpos(begin);
        if (type==BASE64 ) decode_base64(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==UUENC ) decode_uud(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==BASE85 ) decode_ascii85(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==ZLIB && !diffFound) decode_zlib(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==BZIP2  )             decode_bzip2(tmp, tmpsize, in, FCOMPARE, diffFound,info=info+256*17);
        else if (type==GIF && !diffFound) decode_gif(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==MRBR || type==MRBR4) {
            diffFound=imgmrb.CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
        }
        else if (type==RLE)                 decode_rle(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==LZW)                 decode_lzw(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==DECA) decode_dec(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==ARM) decode_arm(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if ((type==TEXT || (type==TXTUTF8 &&witmode==false) ||type==TEXT0) ) decode_txt(en, (tmpsize), in, FCOMPARE, diffFound);
        else if ((type==WIT) ) decode_txt_wit(tmp, (tmpsize), in, FCOMPARE, diffFound,winfo);
        else if ((type==TXTUTF8 &&witmode==true) ) tmp->setend(); //skips 2* input size reading from a file
        else if (type==EOLTEXT ) ts=decode_txtd(tmp, int(tmpsize), in, FCOMPARE, diffFound)!=len?1:0;  
        if (type==EOLTEXT && (diffFound || ts)) {
            // if fail fall back to text
            diffFound=0,ts=0,info=-1, in->setpos(begin),type=TEXT,tmp->close(),tmp=new FileTmp(),encode_txt(in, tmp, int(len),0); 
        }  else if (type==BZIP2 && (diffFound) ) {
            // maxLen was changed from 20 to 17 in bzip2-1.0.3 so try 20
            diffFound=0,in->setpos(begin),tmp->setpos(0),decode_bzip2(tmp, tmpsize, in, FCOMPARE, diffFound,info=(info&255)+256*20);
        }            
        tfail=(diffFound || tmp->getc()!=EOF || ts ); 
        }
        // Test fails, compress without transform
        if (tfail) {
            if (verbose>2) printf(" Transform fails at %0lu, skipping...\n", diffFound-1);
             in->setpos(begin);
             Filetype type2;
             if (type==ZLIB || (type==BZIP2))  type2=CMP; else type2=DEFAULT;
              
            direct_encode_blockstream(type2, in, len);
            typenamess[type][it]-=len,  typenamesc[type][it]--;       // if type fails set
            typenamess[type2][it]+=len,  typenamesc[type2][it]++; // default info
        } else {
            tmp->setpos(0);
            if (type==EXE) {
               direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==DECA || type==ARM) {
                direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==IMAGE24 || type==IMAGE32) {
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if (type==MRBR || type==MRBR4) {
                segment.putdata(type,tmpsize,0);
                int hdrsize=( tmp->getc()<<8)+(tmp->getc());
                Filetype type2 =type==MRBR?IMAGE8:IMAGE4;
                hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==RLE) {
                segment.putdata(type,tmpsize,0);
                int hdrsize=( 4);
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==LZW) {
                segment.putdata(type,tmpsize,0);
                int hdrsize=( 0);
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info&0xffffff);
            }else if (type==GIF) {
                segment.putdata(type,tmpsize,0);
                int hdrsize=(tmp->getc()<<8)+tmp->getc();
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                typenamess[info>>24][it+1]+=tmpsize-hdrsize,  typenamesc[IMAGE8][it+1]++;
                direct_encode_blockstream((Filetype)(info>>24), tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==AUDIO) {
                segment.putdata(type,len,info2); //original lenght
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0)  ) {
                   if ( len>0xA00000){ //if WRT is smaller then original block 
                      if (tmpsize>(len-256) ) {
                         in->setpos( begin);
                         direct_encode_blockstream(NOWRT, in, len); }
                      else
                        direct_encode_blockstream(BIGTEXT, tmp, tmpsize);}
                   else if (tmpsize< (len*2-len/2)||len) {
                        // encode as text without wrt transoform, 
                        // this will be done when stream is compressed
                        in->setpos( begin);
                        direct_encode_blockstream(type, in, len);
                   }
                   else {
                        // wrt size was bigger, encode as NOWRT and put in bigtext stream.
                        in->setpos(begin);
                        direct_encode_blockstream(NOWRT, in, len);
                   }
            }else if (type==EOLTEXT) {
                segment.putdata(type,tmpsize,0);
                int hdrsize=tmp->get32();
                hdrsize=tmp->get32();
                hdrsize=hdrsize+12;
                tmp->setpos(0);
                typenamess[CMP][it+1]+=hdrsize,  typenamesc[CMP][it+1]++; 
                direct_encode_blockstream(CMP, tmp, hdrsize);
                typenamess[TEXT][it+1]+=tmpsize-hdrsize,  typenamesc[TEXT][it+1]++;
                transform_encode_block(TEXT,  tmp, tmpsize-hdrsize, en, -1,-1, blstr, it, hdrsize); 
            } else if (streams.GetTypeInfo(type)&TR_RECURSIVE) {
                int isinfo=0;
                if (type==SZDD ||  type==ZLIB  || type==BZIP2) isinfo=info;
                else if (type==WIT) isinfo=winfo;
                segment.putdata(type,tmpsize,isinfo);
                if (type==ZLIB) {// PDF or PNG image && info
                    Filetype type2 =(Filetype)(info>>24);
                    if (it==itcount)    itcount=it+1;
                    int hdrsize=7+5*tmp->getc();
                    tmp->setpos(0);
                    typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                    direct_encode_blockstream(HDR,  tmp, hdrsize);
                    if (info){
                        typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                        transform_encode_block(type2,  tmp, tmpsize-hdrsize, en, info&0xffffff,-1, blstr, it, hdrsize); }
                    else{
                         DetectRecursive( tmp, tmpsize-hdrsize, en, blstr,it+1);//it+1
                    }
                } else {     
                    DetectRecursive( tmp, tmpsize, en, blstr,it+1);//it+1
                    tmp->close();
                    return;
                }    
            }
        }
        tmp->close();  // deletes
    } else {
        
#define tarpad  //remove for filesize padding \0 and add to default stream as hdr        
        //do tar recursion, no transform
        if (type==TAR){
        int tarl=int(len),tarn=0,blnum=0,pad=0;;
        TARheader tarh;
        char b2[32];
        strcpy(b2, blstr);
        if (b2[0]) strcat(b2, "-");
        while (tarl>0){
            tarl=tarl-pad;
            U64 savedpos= in->curpos(); 
            in->setpos(savedpos+pad);
            in->blockread( (U8*)&tarh,  sizeof(tarh)  );
            in->setpos(savedpos);
            if (tarend((char*)&tarh)) {
                tarn=512+pad;
                if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu]",blstr,typenames[BINTEXT],tarn,savedpos,savedpos+tarn-1);
                typenamess[BINTEXT][it+1]+=tarn,  typenamesc[BINTEXT][it+1]++; 
                direct_encode_blockstream(BINTEXT, in, tarn);
               }
            else if (!tarchecksum((char*)&tarh))  
                quit("tar checksum error\n");
            else{
                int a=getoct(tarh.size,12);
                int b=a-(a/512)*512;
                if (b) tarn=512+(a/512)*512;
                else if (a==0) tarn=512;
                else tarn= a;
                sprintf(blstr,"%s%d",b2,blnum++);
                int tarover=512+pad;
                //if (a && a<=512) tarover=tarover+tarn,a=0,tarn+=512;
                if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu] %s\n",blstr,typenames[BINTEXT],tarover,savedpos,savedpos+tarover-1,tarh.name);
                typenamess[BINTEXT][it+1]+=tarover,  typenamesc[BINTEXT][it+1]++; 
                if (it==itcount)    itcount=it+1;
                direct_encode_blockstream(BINTEXT, in, tarover);
                pad=0;
                if (a!=0){
                    #ifdef tarpad
                        int filenamesize=strlen(tarh.name);
                        U64 ext=0;
                        if( filenamesize>4) for (int i=5;i>0;i--) {
                            U8 ch=tarh.name[filenamesize-i];
                            if (ch>='A' && ch<='Z') ch+='a'-'A';
                            ext=(ext<<8)+ch;
                        }
                        
                        if( filenamesize>3 && (
                        (ext&0xffff)==0x2E63 ||  // .c
                        (ext&0xffff)==0x2E68||   //.h
                        (ext&0xffffffff)==0x2E747874 ||   //.txt
                        (ext&0xffffffffff)==0x2E68746D6C ||  //.html
                        (ext&0xffffffff)==0x2E637070 ||   //.cpp
                        (ext&0xffffff)==0x2E706F // .po
                       // ((tarh.name[filenamesize-1]=='c' || tarh.name[filenamesize-1]=='h') && tarh.name[filenamesize-2]=='.') ||
                      //  (tarh.name[filenamesize-4]=='.' && tarh.name[filenamesize-3]=='t' && tarh.name[filenamesize-2]=='x' &&  tarh.name[filenamesize-1]=='t')
                        )){
                           if (verbose>2) printf(" %-16s | %-9s |%12.0" PRIi64 " [%0lu - %0lu] %s\n",blstr,typenames[TEXT],a,0,a,"direct");
                             direct_encode_blockstream(TEXT, in, a);
                        }else{
                        
 
                        DetectRecursive(in, a, en, blstr, 0);
                        }
                        pad=tarn-a; 
                        tarn=a+512;
                    #else
                        DetectRecursive(in, tarn, en, blstr, 0);
                        pad=0;
                        tarn+=512;
                    #endif
               }
             }
             tarl-=tarn;
             }
             if (verbose>2) printf("\n");
        }else {
            const int i1=(streams.GetTypeInfo(type)&TR_INFO)?info:-1;
            direct_encode_blockstream(type, in, len, i1);
        }
    }
    
}
#if defined(WINDOWS)      
      HANDLE  hConsole;
#endif
void SetConColor(int color) {
#if defined(WINDOWS)      
      SetConsoleTextAttribute(hConsole, color);
#endif     
}

void DetectRecursive(File*in, U64 n, Encoder &en, char *blstr, int it=0) {
  static const char* audiotypes[6]={"8b mono","8b stereo","16b mono","16b stereo","32b mono","32b stereo"};
  Filetype type=DEFAULT;
  int blnum=0, info,info2;  // image width or audio type
  U64 begin= in->curpos(), end0=begin+n;
  U64 textstart;
  U64 textend=0;
  U64 end=0;U64 len;
  Filetype nextType;
  //Filetype nextblock_type;
  Filetype nextblock_type_bak=DEFAULT; //initialized only to suppress a compiler warning, will be overwritten
  char b2[32];
  strcpy(b2, blstr);
  if (b2[0]) strcat(b2, "-");
  if (it==5) {
    direct_encode_blockstream(DEFAULT, in, n);
    return;
  }
  //s2+=n;
  // Transform and test in blocks
  while (n>0) {
    if (it==0 && witmode==true) {
      len=end=end0,info=0,type=WIT;
    }
    else if (it==1 && witmode==true){    
      len=end=end0,info=0,type=TXTUTF8;
    } else {   
    if(type==TEXT || type==EOLTEXT || type==TXTUTF8) { // it was a split block in the previous iteration: TEXT -> DEFAULT -> ...
      nextType=nextblock_type_bak;
      end=textend+1;
    }
    else {
      nextType=detect(in, n, type, info,info2,it);
      end=in->curpos();
      in->setpos(begin);
    }
   // Filetype nextType=detect(in, n, type, info,info2,it,s1);
   // U64 end= in->curpos();
   //  in->setpos( begin);
     // override (any) next block detection by a preceding text block
    textstart=begin+textparser._start[0];
    textend=begin+textparser._end[0];
    if(textend>end-1)textend=end-1;
    if(type==DEFAULT && textstart<textend) { // only DEFAULT blocks may be overridden
     U64 nsize=0;
      if(textstart==begin && textend == end-1) { // whole first block is text
        type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // DEFAULT -> TEXT
        U64 nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        //if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8;
      }
      else if (textend - textstart + 1 >= TEXT_MIN_SIZE) { // we have one (or more) large enough text portion that splits DEFAULT
        if (textstart != begin) { // text is not the first block 
          end=textstart; // first block is still DEFAULT
          nextblock_type_bak=nextType;
          nextType=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; //next block is text
          //if (textparser.number()>((end-begin)>>1))nextblock_type=TEXT0;
           nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))nextType=TEXT0; 
          textparser.removefirst();
        } else {
          type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // first block is text
          nextType=DEFAULT;     // next block is DEFAULT
          end=textend+1; 
          //if (textparser.number()>((end-begin)>>1))type=TEXT0;
           nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        }
      }
      if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8,info=0;
      // no text block is found, still DEFAULT
      
    }
    if (end>end0) {  // if some detection reports longer then actual size file is
      end=begin+1;
      type=nextType=DEFAULT;
    }
      len=U64(end-begin);
    if (begin>end) len=0;
    if (len>=2147483646) {  
      if (!(type==BZIP2||type==WIT ||type==TEXT || type==TXTUTF8 ||type==TEXT0 ||type==EOLTEXT))len=2147483646,type=DEFAULT; // force to int
    }
   }
    if (len>0) {
    if ((type==EOLTEXT) && (len<1024*64 || len>0x1FFFFFFF)) type=TEXT;
    if (it>itcount)    itcount=it;
    if((len>>1)<(info) && type==DEFAULT && info<len) type=BINTEXT;
    if(len==info && type==DEFAULT ) type=ISOTEXT;
    if(len<=TEXT_MIN_SIZE && type==TEXT0 ) type=TEXT;
    typenamess[type][it]+=len,  typenamesc[type][it]++; 
      //s2-=len;
      sprintf(blstr,"%s%d",b2,blnum++);
      // printf(" %-11s | %-9s |%10.0" PRIi64 " [%0lu - %0lu]",blstr,typenames[type],len,begin,end-1);
      if (verbose>2) printf(" %-16s |",blstr);
      int streamcolor=streams.GetStreamID(type)+1+1;
      if (streamcolor<1) streamcolor=7;
      SetConColor(streamcolor);
      if (verbose>2) printf(" %-9s ",typenames[type]);
      SetConColor(7);
      if (verbose>2) {
        printf("|%12.0f [%0.0f - %0.0f]",len+0.0,begin+0.0,(end-1)+0.0);
        if (type==AUDIO) printf(" (%s)", audiotypes[(info&31)%4+(info>>7)*2]);
        else if (type==IMAGE1 || type==IMAGE4 || type==IMAGE8 || type==IMAGE24 || type==MRBR|| type==MRBR4|| type==IMAGE8GRAY || type==IMAGE32 ||type==GIF) printf(" (width: %d)", info&0xFFFFFF);
        else if (type==CD) printf(" (m%d/f%d)", info==1?1:2, info!=3?1:2);
        else if (type==ZLIB && (info>>24) > 0) printf(" (%s)",typenames[info>>24]);
        printf("\n");
      }
      transform_encode_block(type, in, len, en, info,info2, blstr, it, begin);
      n-=len;
    }
    
    type=nextType;
    begin=end;
  }
}

// Compress a file. Split filesize bytes into blocks by type.
// For each block, output
// <type> <size> and call encode_X to convert to type X.
// Test transform and compress.
void DetectStreams(const char* filename, U64 filesize) {
  assert(filename && filename[0]);
  FileTmp tmp;
  Predictors *t;
  t=0;
  Encoder en(COMPRESS, &tmp,*t);
  assert(en.getMode()==COMPRESS);
  assert(filename && filename[0]);
  FileDisk in;
  in.open(filename,true);
  if (verbose>2) printf("Block segmentation:\n");
  char blstr[32]="";
  DetectRecursive(&in, filesize, en, blstr);
  in.close();
  tmp.close();
}

U64 decompressStreamRecursive(File*out, U64 size, Encoder& en, FMode mode, int it=0, U64 s1=0, U64 s2=0) {
    Filetype type;
    U64 len,i,diffFound;
    len=i=diffFound=0L;
    int info=-1;
    s2+=size;
    while (i<size) {
        type=(Filetype)segment(segment.pos++);
        for (int k=0; k<8; k++) len=len<<8,len+=segment(segment.pos++);
        for (int k=info=0; k<4; ++k) info=(info<<8)+segment(segment.pos++);
        int srid=streams.GetStreamID(type);
        if (srid>=0) en.setFile(&streams.streams[srid]->file);
        #ifdef VERBOSE  
         printf(" %d  %-9s |%0lu [%0lu]\n",it, typenames[type],len,i );
        #endif
        if (type==IMAGE24 && !(info&PNGFlag))   {
            FileTmp tmp;
            for (uint64_t j=0; j<len; j++) {
                tmp.putc(en.decompress());
            }
            tmp.setpos(0);
            diffFound=img24.CompareFiles(&tmp,out,len,uint64_t(info),mode);
            //tmp.close();
        }  
        else if (type==IMAGE32 && !(info&PNGFlag)) {
            FileTmp tmp;
            for (uint64_t j=0; j<len; j++) {
                tmp.putc(en.decompress());
            }
            tmp.setpos(0);
            diffFound=img32.CompareFiles(&tmp,out,len,uint64_t(info),mode);
        }
        else if (type==EXE)     len=decode_exe(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==DECA)    len=decode_dec(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==ARM)     len=decode_arm(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==BIGTEXT) len=decode_txt(en, (len), out, mode, diffFound);
        // LZW ?
        else if (type==BASE85 || type==BASE64 || type==UUENC || type==SZDD || type==ZLIB || type==BZIP2 || type==CD || type==MDF  || type==GIF || type==MRBR|| type==MRBR4 || type==RLE ||type==EOLTEXT||type==WIT) {
            FileTmp tmp;
            decompressStreamRecursive(&tmp, len, en, FDECOMPRESS, it+1, s1+i, s2-len);
            if (mode!=FDISCARD) {
                tmp.setpos(0);
                if (type==BASE64) len=decode_base64(&tmp, int(len), out, mode, diffFound);
                else if (type==UUENC)  len=decode_uud(&tmp, int(len), out, mode, diffFound);
                else if (type==BASE85) len=decode_ascii85(&tmp, int(len), out, mode, diffFound);
                else if (type==SZDD)   len=decode_szdd(&tmp,info,info ,out, mode, diffFound);
                else if (type==ZLIB)   len=decode_zlib(&tmp,int(len),out, mode, diffFound);
                else if (type==BZIP2)  len=decode_bzip2(&tmp,int(len),out, mode, diffFound,info);
                else if (type==CD)     len=decode_cd(&tmp, int(len), out, mode, diffFound);
                else if (type==MDF)    len=decode_mdf(&tmp, int(len), out, mode, diffFound);
                else if (type==GIF)    len=decode_gif(&tmp, int(len), out, mode, diffFound);
                else if (type==MRBR|| type==MRBR4)   {
                    diffFound=imgmrb.CompareFiles(&tmp,out,len,uint64_t(info),mode);
                    len=imgmrb.fsize; // get decoded size
                }
                else if (type==EOLTEXT)len=decode_txtd(&tmp, int(len), out, mode, diffFound);
                else if (type==RLE)    len=decode_rle(&tmp, len, out, mode, diffFound);
                else if ((type==WIT) ) len=decode_txt_wit(&tmp, (len), out, mode, diffFound,info);
            }
            tmp.close();
        }
        else {
            for (U64 j=i+s1; j<i+s1+len; ++j) {
                if (!(j&0x1ffff)) printStatus(j, s2);
                if (mode==FDECOMPRESS) out->putc(en.decompress());
                else if (mode==FCOMPARE) {
                    int a=out->getc();
                    int b=en.decompress();
                    if (a!=b && !diffFound) {
                        mode=FDISCARD;
                        diffFound=j+1;
                    }
                } else en.decompress();
            }
        }
        i+=len;
    }
    return diffFound;
}

// Decompress a file from datastream
void DecodeStreams(const char* filename, U64 filesize) {
  FMode mode=FDECOMPRESS;
  assert(filename && filename[0]);
  FileTmp  tmp;
  Predictors *t; //dummy
  t=0;
  Encoder en(COMPRESS, &tmp,*t);
  // Test if output file exists.  If so, then compare.
  FileDisk f;
  bool success=f.open(filename,false);
  if (success) mode=FCOMPARE,printf("Comparing");
  else {
    // Create file
    f.create(filename);
    mode=FDECOMPRESS, printf("Extracting");
  }
  printf(" %s %0.0f -> \n", filename, filesize+0.0);

  // Decompress/Compare
  U64 r=decompressStreamRecursive(&f, filesize, en, mode);
  if (mode==FCOMPARE && !r && f.getc()!=EOF) printf("file is longer\n");
  else if (mode==FCOMPARE && r) printf("differ at %0lu\n",r-1);
  else if (mode==FCOMPARE) printf("identical\n");
  else printf("done   \n");
  f.close();
  tmp.close();
}

void compressStream(int streamid, U64 size, File* in, File* out) {
    int i; //stream
    i=streamid;
    Encoder* threadencode;
    Predictors* threadpredict;
    U64 datasegmentsize;
    U64 datasegmentlen;
    int datasegmentpos;
    int datasegmentinfo;
    Filetype datasegmenttype;
    U64 scompsize=0;
    datasegmentsize=size;
    U64 total=size;
    datasegmentpos=0;
    datasegmentinfo=0;
    datasegmentlen=0;
    // datastreams
    if (level>0) {
        switch(i) {
        default:
        case 0: { threadpredict=new Predictor(); break;}
        case 1: { threadpredict=new PredictorJPEG(); break;}
        case 2: { threadpredict=new PredictorIMG1(); break;}
        case 3: { threadpredict=new PredictorIMG4(); break;}
        case 4: { threadpredict=new PredictorIMG8(); break;}
        case 5: { threadpredict=new PredictorIMG24(); break;}
        case 6: { threadpredict=new PredictorAUDIO2(); break;}
        case 7: { threadpredict=new PredictorEXE(); break;}
        case 8: 
        case 9: 
        case 10: { threadpredict=new PredictorTXTWRT(); break;}
        case 11: { threadpredict=new PredictorDEC(); break;}
        case 12: { threadpredict=new Predictor(); break;}
        }
    }
    threadencode=new Encoder (COMPRESS, out,*threadpredict); 
    if ((i>=0 && i<=7) || i==10 || i==11|| i==12) {
        while (datasegmentsize>0) {
            while (datasegmentlen==0) {
                datasegmenttype=(Filetype)segment(datasegmentpos++);
                for (int ii=0; ii<8; ii++) datasegmentlen<<=8,datasegmentlen+=segment(datasegmentpos++);
                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                if (!(streams.isStreamType(datasegmenttype,i) ))datasegmentlen=0;
                if (level>0){
                    threadencode->predictor.x.filetype=datasegmenttype;
                    threadencode->predictor.x.blpos=0;
                    threadencode->predictor.x.finfo=datasegmentinfo;
                }
            }
            for (U64 k=0; k<datasegmentlen; ++k) {
                //#ifndef MT
                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                //#endif
                threadencode->compress(in->getc());
                datasegmentsize--;
            }
            /* #ifndef NDEBUG 
                            printf("Stream(%d) block from %0lu to %0lu bytes\n",i,datasegmentlen, out->curpos()-scompsize);
                            scompsize= out->curpos();
                            #endif */
            datasegmentlen=0;
        }
        threadencode->flush();
    }
    if (i==8 || i==9) {
        bool dictFail=false;
        FileTmp tm;
        XWRT_Encoder* wrt;
        wrt=new XWRT_Encoder();
        wrt->defaultSettings(i==8);
        wrt->WRT_start_encoding(in,&tm,datasegmentsize,false,true);
        delete wrt;
        datasegmentlen= tm.curpos();
        streams.streams[i]->streamsize=datasegmentlen;
        // -e0 option ignores larger wrt size
        if (datasegmentlen>=datasegmentsize && minfq!=0){
            dictFail=true; //wrt size larger
            if (verbose>0) printf(" WRT larger: %d bytes. Ignoring\n",datasegmentlen-datasegmentsize ); 
        }else{                            
            if (verbose>0)printf(" Total %0" PRIi64 " wrt: %0" PRIi64 "\n",datasegmentsize,datasegmentlen); 
        }
        tm.setpos(0);
        in->setpos(0);
        if (level>0) {
            threadencode->predictor.x.filetype=DICTTXT;
            threadencode->predictor.x.blpos=0;
            threadencode->predictor.x.finfo=-1;
        }
        if (dictFail==true) {
            streams.streams[i]->streamsize=datasegmentlen+1;
            threadencode->compress(0xAA); //flag
        }
        for (U64 k=0; k<datasegmentlen; ++k) {
            if (!(k&0x1fff)) printStatus(k, datasegmentlen,i);
            #ifndef NDEBUG 
            if (!(k&0x3ffff) && k) {
                if (verbose>0) printf("Stream(%d) block pos %0lu compressed to %0lu bytes\n",i,k, out->curpos()-scompsize);
                scompsize= out->curpos();
            }
            #endif
            if (dictFail==false) threadencode->compress(tm.getc());
            else                 threadencode->compress(in->getc());
        }
        tm.close();
        threadencode->flush();
        //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,datasegmentlen+0.0,ftello(out)-scompsize+0.0);
        datasegmentlen=datasegmentsize=0;   
    }
    
    
    if (level>0) delete threadpredict;
    delete threadencode;
    printf("Stream(");
    SetConsoleTextAttribute(hConsole, i+2);
    SetConColor(i+2);
    printf("%d",i);
    SetConColor(7);

    printf(") compressed from %0" PRIi64 " to ",size);
    SetConColor(9);
    printf("%0" PRIi64 "",out->curpos());
    SetConColor(7);
    printf(" bytes\n");
}

#ifdef MT
void decompress(const Job& job) {
}
#endif

void CompressStreams(File *archive,U16 &streambit) {
    for (int i=0; i<streams.Count(); ++i) {
        U64 datasegmentsize;
        datasegmentsize= streams.streams[i]->file.curpos();    //get segment data offset
        streams.streams[i]->streamsize=datasegmentsize;
        streams.streams[i]->file.setpos(0);
        streambit=(streambit+(datasegmentsize>0))<<1; //set stream bit if streamsize >0
        if (datasegmentsize>0){                       //if segment contains data
            if (verbose>0) {
                SetConColor(i+2);
                printf("%-12s", streams.streams[i+1]->name.c_str());
                SetConColor(7);  
                printf("stream(%d).  Total %0" PRIi64 "\n",i,datasegmentsize);
            }
#ifdef MT
            // add streams to job list
            Job job;
            job.out=&streams.streams[i]->out;
            job.in=&streams.streams[i]->file;
            job.streamid=i;
            job.command=0; //0 compress
            job.datasegmentsize=datasegmentsize;
            jobs.push_back(job);
#else
            compressStream(i,datasegmentsize,&streams.streams[i]->file,archive);
#endif
        }
    }

#ifdef MT
    // Loop until all jobs return OK or ERR: start a job whenever one
    // is eligible. If none is eligible then wait for one to finish and
    // try again. If none are eligible and none are running then it is
    // an error.
    int thread_count=0;  // number RUNNING, not to exceed topt
    U32 job_count=0;     // number of jobs with state OK or ERR

    // Aquire lock on jobs[i].state.
    // Threads can access only while waiting on a FINISHED signal.
#ifdef PTHREAD
    pthread_attr_t attr; // thread joinable attribute
    check(pthread_attr_init(&attr));
    check(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
    check(pthread_mutex_lock(&mutex));  // locked
#else
    mutex=CreateMutex(NULL, FALSE, NULL);  // not locked
#endif

    while(job_count<jobs.size()) {

        // If there is more than 1 thread then run the biggest jobs first
        // that satisfies the memory bound. If 1 then take the next ready job
        // that satisfies the bound. If no threads are running, then ignore
        // the memory bound.
        int bi=-1;  // find a job to start
        if (thread_count<topt) {
            for (U32 i=0; i<jobs.size(); ++i) {
                if (jobs[i].state==READY  && bi<0 ) {
                    bi=i;
                    if (topt==1) break;
                }
            }
        }

        // If found then run it
        if (bi>=0) {
            jobs[bi].state=RUNNING;
            ++thread_count;
#ifdef PTHREAD
            check(pthread_create(&jobs[bi].tid, &attr, thread, &jobs[bi]));
#else
            jobs[bi].tid=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread,
            &jobs[bi], 0, NULL);
#endif
        }

        // If no jobs can start then wait for one to finish
        else {
#ifdef PTHREAD
            check(pthread_cond_wait(&cv, &mutex));  // wait on cv

            // Join any finished threads. Usually that is the one
            // that signaled it, but there may be others.
            for (U32 i=0; i<jobs.size(); ++i) {
                if (jobs[i].state==FINISHED || jobs[i].state==FINISHED_ERR) {
                    void* status=0;
                    check(pthread_join(jobs[i].tid, &status));
                    if (jobs[i].state==FINISHED) jobs[i].state=OK;
                    if (jobs[i].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
                    ++job_count;
                    --thread_count;
                }
            }
#else
            // Make a list of running jobs and wait on one to finish
            HANDLE joblist[MAXIMUM_WAIT_OBJECTS];
            int jobptr[MAXIMUM_WAIT_OBJECTS];
            DWORD njobs=0;
            WaitForSingleObject(mutex, INFINITE);
            for (U32 i=0; i<jobs.size() && njobs<MAXIMUM_WAIT_OBJECTS; ++i) {
                if (jobs[i].state==RUNNING || jobs[i].state==FINISHED
                        || jobs[i].state==FINISHED_ERR) {
                    jobptr[njobs]=i;
                    joblist[njobs++]=jobs[i].tid;
                }
            }
            ReleaseMutex(mutex);
            DWORD id=WaitForMultipleObjects(njobs, joblist, FALSE, INFINITE);
            if (id>=WAIT_OBJECT_0 && id<WAIT_OBJECT_0+njobs) {
                id-=WAIT_OBJECT_0;
                id=jobptr[id];
                if (jobs[id].state==FINISHED) jobs[id].state=OK;
                if (jobs[id].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
                ++job_count;
                --thread_count;
            }
#endif
        }
    }
#ifdef PTHREAD
    check(pthread_mutex_unlock(&mutex));
#endif

    // Append temporary files to archive if OK.
    for (U32 i=0; i<jobs.size(); ++i) {
        if (jobs[i].state==OK) {
            streams.streams[jobs[i].streamid]->out.setpos( 0);
            //append streams to archive
            const int BLOCK=4096*16;
            U8 blk[BLOCK];
            bool readdone=false; 
            for (;;) { 
                if (readdone) break;
                int bytesread=streams.streams[jobs[i].streamid]->out.blockread(&blk[0], BLOCK);
                if (bytesread!=BLOCK) {
                    readdone=true;                   
                    archive->blockwrite(&blk[0], bytesread);
                } else      
                archive->blockwrite(&blk[0], BLOCK);
            }
            streams.streams[jobs[i].streamid]->out.close();
        }
    }

#endif
    for (int i=0; i<streams.Count(); ++i) {
        streams.streams[i]->file.close();
    }
}

void DecompressStreams(File *archive) {
    U64 datasegmentsize;
    U64 datasegmentlen;
    int datasegmentpos;
    int datasegmentinfo;
    Filetype datasegmenttype;
    Predictors* predictord;
    //predictord=new Predictor();
    predictord=0;
    Encoder *defaultencoder;
    defaultencoder=0;
    for (int i=0; i<streams.Count(); ++i) {
        datasegmentsize=(streams.streams[i]->streamsize); // get segment data offset
        if (datasegmentsize>0){              // if segment contains data
            streams.streams[i]->file.setpos( 0);
            U64 total=datasegmentsize;
            datasegmentpos=0;
            datasegmentinfo=0;
            datasegmentlen=0;
            if (predictord) delete predictord,predictord=0;
            if (defaultencoder) delete defaultencoder,defaultencoder=0;
            printf("Decompressing %-12s stream(%d).\n", streams.streams[i+1]->name.c_str(),i);
            if (level>0){
                switch(i) {
                case 0: {
                        predictord=new Predictor();     break;}
                case 1: {
                        predictord=new PredictorJPEG(); break;}
                case 2: {
                        predictord=new PredictorIMG1(); break;}
                case 3: {
                        predictord=new PredictorIMG4(); break;}
                case 4: {
                        predictord=new PredictorIMG8(); break;}
                case 5: {
                        predictord=new PredictorIMG24(); break;}
                case 6: {
                        predictord=new PredictorAUDIO2(); break;}
                case 7: {
                        predictord=new PredictorEXE();    break;}
                case 8: {
                        predictord=new PredictorTXTWRT(); break;}
                case 9:
                case 10: {
                        predictord=new PredictorTXTWRT(); break;}
                case 11: {
                        predictord=new PredictorDEC(); break;}
                case 12: {
                        predictord=new Predictor(); break;}
                }
            }
            defaultencoder=new Encoder (DECOMPRESS, archive,*predictord); 
            if ((i>=0 && i<=7)||i==10||i==11||i==12){
                while (datasegmentsize>0) {
                    while (datasegmentlen==0){
                        datasegmenttype=(Filetype)segment(datasegmentpos++);
                        for (int ii=0; ii<8; ii++) datasegmentlen=datasegmentlen<<8,datasegmentlen+=segment(datasegmentpos++);
                        for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                        if (!(streams.isStreamType(datasegmenttype,i) ))datasegmentlen=0;
                        if (level>0) {
                            defaultencoder->predictor.x.filetype=datasegmenttype;
                            defaultencoder->predictor.x.blpos=0;
                            defaultencoder->predictor.x.finfo=datasegmentinfo; }
                    }
                    for (U64 k=0; k<datasegmentlen; ++k) {
                        if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                        streams.streams[i]->file.putc(defaultencoder->decompress());
                        datasegmentsize--;
                    }
                    datasegmentlen=0;
                }
            }
            if (i==8 || i==9 ){
                while (datasegmentsize>0) {
                    FileTmp tm;
                    bool doWRT=true;
                    datasegmentlen=datasegmentsize;
                    if (level>0) {
                        defaultencoder->predictor.x.filetype=DICTTXT;
                        defaultencoder->predictor.x.blpos=0;
                        defaultencoder->predictor.x.finfo=-1; }
                    for (U64 k=0; k<datasegmentlen; ++k) {
                        if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                        U8 b=defaultencoder->decompress();
                        if (k==0 && b==0xAA) doWRT=false; // flag set?
                        else tm.putc(b);
                        datasegmentsize--;
                    }
                    if (doWRT==true) {
                        
                        XWRT_Decoder* wrt;
                        wrt=new XWRT_Decoder();
                        int b=0;
                        wrt->defaultSettings(0);
                        tm.setpos( 0);
                        U64 bb=wrt->WRT_start_decoding(&tm);
                        for ( U64 ii=0; ii<bb; ii++) {
                            b=wrt->WRT_decode();    
                            streams.streams[i]->file.putc(b);
                        }
                        tm.close();
                        delete wrt;
                    }else{
                        tm.setpos( 0);
                        
                        for ( U64 ii=1; ii<datasegmentlen; ii++) {
                            U8 b=tm.getc(); 
                            streams.streams[i]->file.putc(b);
                        }
                        tm.close();
                    }
                    datasegmentlen=datasegmentsize=0;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    bool pause=argc<=2;  // Pause when done?
    try {

        // Get option
        char* aopt;
        aopt=&argv[1][0];
        
#ifdef MT 
        if (argc>1 && aopt[0]=='-' && aopt[1]  && strlen(aopt)<=6) {
#else
        if (argc>1 && aopt[0]=='-' && aopt[1]  && strlen(aopt)<=4) {    
#endif
            if (aopt[1]=='d' && !aopt[2])
                doExtract=true;
            else if (aopt[1]=='l' && !aopt[2])
                doList=true;
            else if (aopt[2]>='0' && aopt[2]<='9' && strlen(aopt)==3 && (aopt[1]=='s' || aopt[1]=='x')){
                level=aopt[2]-'0';
                slow=aopt[1]=='x'?true:false;
            }
            else if (aopt[2]=='1' && aopt[3]>='0' && aopt[3]<='5' && strlen(aopt)==4 && (aopt[1]=='s' || aopt[1]=='x')){
                slow=aopt[1]=='x'?true:false;
                aopt[1]='-', aopt[0]=' ';
                level=((~atol(aopt))+1);
            }
#ifdef MT 
            else if (aopt[2]>='0' && aopt[2]<='9'&& (aopt[4]<='9' && aopt[4]>'0') && strlen(aopt)==5 && 
            ((aopt[1]=='s' || aopt[1]=='x'))){
                topt=aopt[4]-'0';
                level=aopt[2]-'0';slow=aopt[1]=='x'?true:false;}
            else if (aopt[2]=='1' && aopt[3]>='0' && aopt[3]<='5' && 
            (aopt[5]<='9' && aopt[5]>'0')&& strlen(aopt)==6 && (aopt[1]=='s' || aopt[1]=='x')){
                topt=aopt[5]-'0';
                slow=aopt[1]=='x'?true:false;
                aopt[4]=0;
                aopt[1]='-';
                aopt[0]=' ';
                level=((~atol(aopt))+1);
            }
#endif
            else
                quit("Valid options are -s0 through -s15, -d, -l\n");
            
            --argc;
            ++argv;
            if (argv[1][0]=='-' && argv[1][1]=='w')   {
                witmode=true; printf("WIT\n");
                --argc;
                ++argv;
            }
            if (argv[1][0]=='-' && argv[1][1]=='e')   {
                staticd=true;
                if (argv[1][2]==0) minfq=0;
                else minfq=atol(&argv[1][2]);
                char *extd=(strchr(&argv[1][2], ','));
                if (minfq<0) printf("BAD command line: minimum word frequency must be >=0\n"),quit("");
                if (minfq<1) printf("WARNING: minimum word frequency=0, using static words only.\n");
                if (extd==0) staticd=false,printf("WARNING: dictionary file not found.\n");
                else externaDict=extd+1;
                //witmode=true; printf("WIT\n");
                --argc;
                ++argv;
            }
            if (argv[1][0]=='-' && argv[1][1]=='v')   {
                verbose=atol(&argv[1][2]);
                if (verbose>3 || verbose<0) printf("BAD verbose level\n"),quit("");
                printf("Verbose: level %d.\n",verbose);
                --argc;
                ++argv;
            }
            pause=false;
        }
        if (slow==true) printf("Slow mode\n");
        // Print help message quick 
        if (argc<2) {
            printf(PROGNAME " archiver (C) 2021, Matt Mahoney et al.\n"
            "Free under GPL, http://www.gnu.org/licenses/gpl.txt\n");
#ifdef __GNUC__     
            printf("Compiled %s, compiler gcc version %d.%d.%d\n\n",__DATE__, __GNUC__, __GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#endif
#ifdef __clang_major__
            printf("Compiled %s, compiler clang version %d.%d\n\n",__DATE__, __clang_major__, __clang_minor__);
#endif
#ifdef            _MSC_VER 
            printf("Compiled %s, compiler Visual Studio version %d\n\n",__DATE__, _MSC_VER);
#endif
#ifdef MT
printf("Multithreading enabled with %s.\n",
#ifdef PTHREAD
"PTHREAD"
#else
"windows native threads"
#endif
);

#if defined(__AVX2__)
printf("Compiled with AVX2\n");
#elif defined(__SSE4_1__)   
printf("Compiled with SSE41\n");
#elif  defined(__SSSE3__)
printf("Compiled with SSSE3\n");
#elif defined(__SSE2__) 
printf("Compiled with SSE2\n");
#elif defined(__SSE__)
printf("Compiled with SSE\n");
#else
printf("No vector instrucionts\n");
#endif
#endif
printf("\n");
            printf(
#ifdef WINDOWS
            "To compress or extract, drop a file or folder on the "
            PROGNAME " icon.\n"
            "The output will be put in the same folder as the input.\n"
            "\n"
            "Or from a command window: "
#endif
            "To compress:\n"
            "  " PROGNAME " -slevel file               (compresses to file." PROGNAME ")\n"
            "  " PROGNAME " -slevel archive files...   (creates archive." PROGNAME ")\n"
            "  " PROGNAME " file                       (level -%d pause when done)\n"
            "level: -s0          store\n"
            "  -s1...-s3         (uses 393, 398, 409 MB)\n"
            "  -s4...-s9         (uses 1.2  1.3  1.5  1.9 2.7 4.9 GB)\n"
            "  -s10...-s15       (uses 7.0  9.0 11.1 27.0   x.x x.x GB)\n"
#ifdef MT 
            "  to use multithreading -level:threads (1-9, compression only)\n"
            "  " PROGNAME " -s4:2 file (use level 4 threads 2)\n\n"
#endif            
#if defined(WINDOWS) || defined (UNIX)
            "You may also compress directories.\n"
#endif
            "\n"
            "To extract or compare:\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME "      (extract to dir1)\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME " dir2 (extract to dir2)\n"
            "  " PROGNAME " archive." PROGNAME "              (extract, pause when done)\n"
            "\n"
            "To view contents: " PROGNAME " -l archive." PROGNAME "\n"
            "\n",
            DEFAULT_OPTION);
            quit("");
        }
#if defined(WINDOWS)      
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
        for (int i=0;i<256;i++) {
            int n0=-!nex(i,2);
            int n1=-!nex(i,3);
            int r=0;
            if ((n1-n0)==1 ) r=2;
            if ((n1-n0)==-1 ) r=1;
            n0n1[i]=r;
        }

  for (int i=0; i<1024; ++i)
    dt[i]=16384/(i+i+3);
#ifdef SM    
    dt[1023]=1;
    dt[0]=4095;
#endif
        File* archive=0;               // compressed file
        int files=0;                   // number of files to compress/decompress
        Array<const char*> fname(1);   // file names (resized to files)
        Array<U64> fsize(1);           // file lengths (resized to files)
        U16 streambit=0;               //bit is set if stream has size, 11-0
        // Compress or decompress?  Get archive name
        Mode mode=COMPRESS;
        std::string archiveName(argv[1]);
        {
            const int prognamesize=strlen(PROGNAME);
            const int arg1size=strlen(argv[1]);
            if (arg1size>prognamesize+1 && argv[1][arg1size-prognamesize-1]=='.'
                    && equals(PROGNAME, argv[1]+arg1size-prognamesize)) {
                mode=DECOMPRESS,doExtract=true;
            }
            else if (doExtract || doList)
            mode=DECOMPRESS;
            else {
                archiveName+=".";
                archiveName+=PROGNAME;
            }
        }

        // Compress: write archive header, get file names and sizes
        std::string header_string;
        std::string filenames;
        
        if (mode==COMPRESS) {
            segment.setsize(48); //inital segment buffer size (about 277 blocks)
            // Expand filenames to read later.  Write their base names and sizes
            // to archive.
            int i;
            for (i=1; i<argc; ++i) {
                std::string name(argv[i]);
                int len=name.size()-1;
                for (int j=0; j<=len; ++j)  // change \ to /
                if (name[j]=='\\') name[j]='/';
                while (len>0 && name[len-1]=='/')  // remove trailing /
                name[--len]=0;
                int base=len-1;
                while (base>=0 && name[base]!='/') --base;  // find last /
                ++base;
                if (base==0 && len>=2 && name[1]==':') base=2;  // chop "C:"
                int expanded=expand(header_string, filenames, name.c_str(), base);
                if (!expanded && (i>1||argc==2))
                printf("%s: not found, skipping...\n", name.c_str());
                files+=expanded;
            }

            // If there is at least one file to compress
            // then create the archive header.
            if (files<1) quit("Nothing to compress\n");
            archive=new FileDisk();
            archive->create(archiveName.c_str());
            archive->append(PROGNAME);
            archive->putc(0);
            archive->putc(level|            ((slow==true)?64:0)|            ((witmode==true)?128:0));
            segment.hpos= archive->curpos();
            
            for (int i=0; i<12+4+2; i++) archive->putc(0); //space for segment size in header +streams info
            
            printf("Creating archive %s with %d file(s)...\n",
            archiveName.c_str(), files);
        }

        // Decompress: open archive for reading and store file names and sizes
        if (mode==DECOMPRESS) {
            archive= new FileDisk();
            archive->open(archiveName.c_str(),true);
            // Check for proper format and get option
            std::string header;
            int len=strlen(PROGNAME)+1, c, i=0;
            header.resize(len+1);
            while (i<len && (c=archive->getc())!=EOF) {
                header[i]=c;
                i++;
            }
            header[i]=0;
            if (strncmp(header.c_str(), PROGNAME "\0", strlen(PROGNAME)+1))
            printf("%s: not a %s file\n", archiveName.c_str(), PROGNAME), quit("");
            level=archive->getc();
            if (level&64) slow=true;
            if (level&128) witmode=true;
            level=level&0xf;
            
            // Read segment data from archive end
            U64 currentpos,datapos=0L;
            for (int i=0; i<8; i++) datapos=datapos<<8,datapos+=archive->getc();
            segment.hpos=datapos;
            U32 segpos=archive->get32();  //read segment data size
            segment.pos=archive->get32(); //read segment data size
            streambit=archive->getc()<<8; //get bitinfo of streams present
            streambit+=archive->getc();
            if (segment.hpos==0 || segment.pos==0) quit("Segment data not found.");
            segment.setsize(segment.pos);
            currentpos= archive->curpos();
             archive->setpos( segment.hpos); 
            if (archive->blockread( &segment[0],   segment.pos  )<segment.pos) quit("Segment data corrupted.");
            // Decompress segment data 
            Encoder* segencode;
            Predictors* segpredict;
            FileTmp  tmp;
            tmp.blockwrite(&segment[0],   segment.pos  ); 
            tmp.setpos(0); 
            segpredict=new Predictor();
            segencode=new Encoder (DECOMPRESS, &tmp ,*segpredict); 
            segment.pos=0;
            for (U32 k=0; k<segpos; ++k) {
                 segment.put1( segencode->decompress());
            }
            delete segpredict;
            delete segencode;
            tmp.close();
            //read stream sizes if stream bit is set
            for (int i=0;i<streams.Count();i++){
                if ((streambit>>(streams.Count()-i))&1){
                   streams.streams[i]->streamsize=archive->getVLI();
                }
            }
            archive->setpos(currentpos); 
            segment.pos=0; //reset to offset 0
        }
        Encoder* en;
        Predictors* predictord;
        predictord=new Predictor();
        en=new Encoder(mode, archive,*predictord);
        
        // Compress header
        if (mode==COMPRESS) {
            int len=header_string.size();
            assert(en->getMode()==COMPRESS);
            U64 start=en->size();
            en->compress(0); // block type 0
            en->compress(len>>24); en->compress(len>>16); en->compress(len>>8); en->compress(len); // block length
            for (int i=0; i<len; i++) en->compress(header_string[i]);
            printf("File list compressed from %d to %0lu bytes.\n",len,en->size()-start);
        }

        // Deompress header
        if (mode==DECOMPRESS) {
            if (en->decompress()!=0) printf("%s: header corrupted\n", archiveName.c_str()), quit("");
            int len=0;
            len+=en->decompress()<<24;
            len+=en->decompress()<<16;
            len+=en->decompress()<<8;
            len+=en->decompress();
            header_string.resize(len);
            for (int i=0; i<len; i++) {
                header_string[i]=en->decompress();
                if (header_string[i]=='\n') files++;
            }
            if (doList) printf("File list of %s archive:\n%s", archiveName.c_str(), header_string.c_str());
        }
        
        // Fill fname[files], fsize[files] with input filenames and sizes
        fname.resize(files);
        fsize.resize(files);
        char *p=&header_string[0];
        char* q=&filenames[0];
        for (int i=0; i<files; ++i) {
            assert(p);
            fsize[i]=atoll(p);
            assert(fsize[i]>=0);
            while (*p!='\t') ++p; *(p++)='\0';
            fname[i]=mode==COMPRESS?q:p;
            while (*p!='\n') ++p; *(p++)='\0';
            if (mode==COMPRESS) { while (*q!='\n') ++q; *(q++)='\0'; }
        }

        // Compress or decompress files
        assert(fname.size()==files);
        assert(fsize.size()==files);
        U64 total_size=0;  // sum of file sizes
        for (int i=0; i<files; ++i) total_size+=fsize[i];
        if (mode==COMPRESS) {
            en->flush();
            delete en;
            delete predictord;
            for (int i=0; i<files; ++i) {
                printf("\n%d/%d  Filename: %s (%0" PRIi64 " bytes)\n", i+1, files, fname[i], fsize[i]); 
                DetectStreams(fname[i], fsize[i]);
            }
            segment.put1(0xff); //end marker
            //Display Level statistics
            if (verbose>1) {
                printf("\n Segment data size: %d bytes\n",segment.pos);
                for (int j=0; j<=itcount; ++j) {
                    printf("\n %-2s |%-9s |%-10s |%-10s\n","TN","Type name", "Count","Total size");
                    printf("-----------------------------------------\n");
                    U32 ttc=0; U64 tts=0;
                    for (int i=0; i<datatypecount; ++i) {
                        if (typenamess[i][j]) {
                            printf(" %2d |%-9s |%10d |%10.0" PRIi64 "\n",i,typenames[i], typenamesc[i][j],typenamess[i][j]);
                            ttc+=typenamesc[i][j],tts+=typenamess[i][j];
                        }
                    }
                    printf("-----------------------------------------\n");
                    printf("%-13s%1d |%10d |%10.0" PRIi64 "\n\n","Total level",j, ttc,tts);
                }
            }

            CompressStreams(archive,streambit);
            
            // Write out segment data
            U64 segmentpos;
            segmentpos= archive->curpos();  //get segment data offset
            archive->setpos( segment.hpos);
            archive->put64(segmentpos);     //write segment data offset
            //compress segment data
            Encoder* segencode;
            Predictors* segpredict;
            FileTmp tmp;                    // temporary encoded file
            segpredict=new Predictor();
            segencode=new Encoder (COMPRESS, &tmp ,*segpredict); 
            for (U64 k=0; k<segment.pos; ++k) {
                segencode->compress(segment[k]);
            }
            segencode->flush();
            delete segpredict;
            delete segencode;
            archive->put32(segment.pos);     // write segment data size
            if (verbose>0) printf(" Segment data compressed from %d",segment.pos);
            segment.pos=tmp.curpos();
            segment.setsize(segment.pos);
            if (verbose>0) printf(" to %d bytes\n ",segment.pos);
            tmp.setpos( 0); 
            if (tmp.blockread(&segment[0], segment.pos)<segment.pos) quit("Segment data corrupted.");
            tmp.close();
            archive->put32(segment.pos);      // write  compressed segment data size
            archive->putc(streambit>>8&0xff); // write stream bit info
            archive->putc(streambit&0xff); 
            archive->setpos(segmentpos); 
            archive->blockwrite(&segment[0], segment.pos); //write out segment data
            //write stream size if present
            for (int i=0;i<streams.Count();i++){
                if (streams.streams[i]->streamsize>0) archive->putVLI(streams.streams[i]->streamsize);
            }
            printf("Total %0" PRIi64 " bytes compressed to %0" PRIi64 " bytes.\n", total_size,  archive->curpos()); 
        }
        // Decompress files to dir2: paq8pxd -d dir1/archive.paq8pxd dir2
        // If there is no dir2, then extract to dir1
        // If there is no dir1, then extract to .
        else if (!doList) {
            assert(argc>=2);
            std::string dir(argc>2?argv[2]:argv[1]);
            if (argc==2) {  // chop "/archive.paq8pxd"
                int i;
                for (i=dir.size()-2; i>=0; --i) {
                    if (dir[i]=='/' || dir[i]=='\\') {
                        dir[i]=0;
                        break;
                    }
                    if (i==1 && dir[i]==':') {  // leave "C:"
                        dir[i+1]=0;
                        break;
                    }
                }
                if (i==-1) dir=".";  // "/" not found
            }
            dir=dir.c_str();
            if (dir[0] && (dir.size()!=3 || dir[1]!=':')) dir+="/";
            /////
            
            delete en;
            delete predictord;
            DecompressStreams(archive);
            // set datastream file pointers to beginning
            for (int i=0; i<streams.Count(); ++i)         
            streams.streams[i]->file.setpos( 0);
            /////
            segment.pos=0;
            for (int i=0; i<files; ++i) {
                std::string out(dir.c_str());
                out+=fname[i];
                DecodeStreams(out.c_str(), fsize[i]);
            } 
            int d=segment(segment.pos++);
            if (d!=0xff) printf("Segmend end marker not found\n");
            for (int i=0; i<streams.Count(); ++i) {
                streams.streams[i]->file.close();
            }
        }
        archive->close();
    }
    catch(const char* s) {
        if (s) printf("%s\n", s);
    }
    if (pause) {
        printf("\nClose this window or press ENTER to continue...\n");
        getchar();
    }
    return 0;
}




