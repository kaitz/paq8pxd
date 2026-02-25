    /* paq8pxd file compressor/archiver.  Release by Kaido Orav

    Copyright (C) 2008-2026 Matt Mahoney, Serge Osnach, Alexander Ratushnyak,
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
 
#define PROGNAME "paq8pxd144"  // Please change this if you change the program.

//#define MT            //uncomment for multithreading, compression only. Handled by CMake and gcc when -DMT is passed.
#ifndef DISABLE_SM
//#define SM              // For faster statemap
#endif
//#define VERBOSE         // Show extra info
/*
This needs to be define globaly by compiler
//#define PTHREAD       //uncomment to force pthread to igore windows native threads
*/

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>

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



#if defined(WINDOWS) || defined(_MSC_VER)
    #define atoll(S) _atoi64(S)
#endif
#define _FILE_OFFSET_BITS 64
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

#include "prt/cli.hpp"
#include "prt/program.hpp"
#include "prt/settings.hpp"

Settings settings;
CLI cli(PROGNAME);
Program prog(cli,settings,PROGNAME);

int dt[1024];  // i -> 16K/(i+i+3)
int n0n1[256]; // for contectmap

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

/*#define AUD_DET(type,start_pos,header_len,data_len,wmode) return dett=(type),\
deth=int(header_len),detd=(data_len),info=(wmode),\
 in->setpos(start+(start_pos)),HDR
*/
/*bool IsGrayscalePalette(File* in, int n = 256, int isRGBA = 0){
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
}*/

/*

Filetype detect(File* in, U64 n, Filetype type, int &info, int &info2, int it=0) {
  U32 buf4=0,buf3=0, buf2=0, buf1=0, buf0=0;  // last 8 bytes
  static U64 start=0;
  static U64 prv_start=0;
  prv_start = start;    // for DEC Alpha detection
  start= in->curpos();
  
  // For ARM detection
  Array<U64> absposARM(256),  // CALL/JMP abs. addr. low byte -> last offset
    relposARM(256);    // CALL/JMP relative addr. low byte -> last offset
  int ARMcount=0;  // number of consecutive CALL/JMPs
  U64 ARMpos=0;    // offset of first CALL or JMP instruction
  U64 ARMlast=0;   // offset of most recent CALL or JMP

 
  U64 s3mi=0;
  int s3mno=0,s3mni=0;  // For S3M detection
  

  U32 op=0;//DEC A

  int textbin=0,txtpdf=0; //if 1/3 is text

  static int deth=0,detd=0;  // detected header/data size in bytes
  static Filetype dett;      // detected block type
  if (deth >1) return  in->setpos(start+deth),deth=0,dett;
  else if (deth ==-1) return  in->setpos(start),deth=0,dett;
  else if (detd) return  in->setpos( start+detd),detd=0,DEFAULT;
 

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
   

        
    // ARM
    op=(buf0)>>26; 
    //test if bl opcode and if last 3 opcodes are valid 
    // BL(4) and (ADD(1) or MOV(4)) as previous, 64 bit
    // ARMv8-A_Architecture_Reference_Manual_(Issue_A.a).pdf
    if (op==0x25 && //DECcount==0 &&//||(buf3)>>26==0x25 
    (((buf1)>>26==0x25 ||(buf2)>>26==0x25) ||
    (( ((buf1)>>24)&0x7F==0x11 || ((buf1)>>23)&0x7F==0x25  || ((buf1)>>23)&0x7F==0xa5 || ((buf1)>>23)&0x7F==0x64 || ((buf1)>>24)&0x7F==0x2A) )
    )&&     (buf1)>>31==1&& (buf2)>>31==1&& (buf3)>>31==1&& (buf4)>>31==1){ 
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
    
    
    
  }
  return type;


}
*/
void CalcTables() {
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
}

void PrintHelp() {
    // Print help message quick 
    printf(PROGNAME " archiver (C) 2026, Matt Mahoney et al.\n"
            "Free under GPL, http://www.gnu.org/licenses/gpl.txt\n");
    if (settings.verbose==0 && settings.showhelp==false) {
        printf("\nUsage: paq8pxd -{COMMAND} [archive] input\n");
        printf("\nFor help: paq8pxd -h\n");
    }
    if (settings.verbose==3 && settings.showhelp) {
#ifdef __GNUC__
        printf("\nCompiled %s, compiler gcc version %d.%d.%d\n",__DATE__, __GNUC__, __GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#endif
#ifdef __clang_major__
        printf("\nCompiled %s, compiler clang version %d.%d\n",__DATE__, __clang_major__, __clang_minor__);
#endif
#ifdef _MSC_VER 
        printf("\nCompiled %s, compiler Visual Studio version %d\n",__DATE__, _MSC_VER);
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
    }
    printf("\n");
     if (settings.showhelp) {
#ifdef WINDOWS
        if (settings.verbose>1 && settings.showhelp) {
            printf(
            "To compress or extract, drop a file or folder on the "
            PROGNAME " icon.\n"
            "The output will be put in the same folder as the input.\n"
            "\n"
            "Or from a command window:\n"
            ); 
        }
#endif
        printf("To compress:\n"
            "  " PROGNAME " -s[level] file             (compresses to file." PROGNAME ")\n"
            "  " PROGNAME " -s[level] archive files... (creates archive." PROGNAME ")\n"
            "  " PROGNAME " file                       (level -s8 pause when done)\n");
        printf("\nCommands:\n"
            "  -{s|x}[level]     Compression mode: slow, extreme.\n"
            "                    Level selects memory usage, range 0-15.\n"   
            "                    Default level is 8.\n"
            "                    If level is omitted, default is 0 (store).\n"
            "  -d                Decompress archive. If target exist then compare.\n"
            "  -l                List files in archive.\n"
            "  -r{n}             Recursion depth. Range 0-9. Default 6.\n"
            "  -v{n}             Set verbose level to n. Range 0-3. Defaul 0.\n"
            "  -h                Show help. Use command -v for more info.\n"
#ifdef MT 
            "  -t{n}             Select number of threads when compressing.\n"
            "                    Valid range 1-4. Default 1.\n"
#endif
        );
        if (settings.verbose>0) {
            printf("  -e{name}          Use external dictionary: name.\n"
            "  -q{frq}           Set minimum frequency for dictionary transform.\n"
            "                    Default 19.\n"
            "  -w                Preprocces wikipedia xml dump with transform\n"
            "                    before dictionary transform.\n"
            "  -p{name}          Enable only parser: name\n"
            "                    List of allowed parsers:\n"
            "                       P_BMP, P_TXT, P_DECA, P_MRB, P_EXE, P_NES\n"
            "                       P_MZIP,P_JPG, P_WAV, P_PNM, P_PLZW, P_GIF\n"
            "                       P_DBS, P_AIFF, P_A85, P_B641, P_B642, P_MOD\n"
            "                       P_SGI, P_TGA, P_ICD, P_MDF, P_UUE, P_TIFF\n"
            "                       P_TAR, P_PNG, P_ZIP, P_GZIP, P_BZIP2, P_SZDD\n"
            "                       P_MSCF, P_ZLIB, P_ZLIBP, P_ISO9960, P_ISCAB, P_PBIT\n"
            );
        }
        if (settings.verbose>0) {
        printf("\nMemory usage (level):\n"
            "  -s0               store (no compression)\n"
            "                    (These values are ~ and may change between versions)\n"
            "  -s1...-s3         uses 393, 398, 409 MB\n"
            "  -s4...-s9         uses 1.2  1.3  1.5  1.9 2.7 4.9 GB\n"
            "  -s10...-s15       uses 7.0  9.0 11.1 27.0   x.x x.x GB\n"
          );
        }
        if (settings.verbose>0) {
        printf(
#if defined(WINDOWS) || defined (UNIX)
            "\nYou may also compress directories.\n"
#endif
            "To extract or compare:\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME "      (extract to dir1)\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME " dir2 (extract to dir2)\n"
            "  " PROGNAME " archive." PROGNAME "              (extract, pause when done)\n"
            "\n");
        }
    }
    exit(0);
}

int main(int argc, char** argv) {
    bool pause=argc<=2;  // Pause when done?
    try {
        // Parse command line
        bool ok=cli.Parse(argc, argv);
        if (ok==false) {
            PrintHelp();
        } else {
            CliCommand ccm;
            while (ccm=cli.GetCommand(),ccm.type!=CL_UNK) {
                if (ccm.type==CL_LIST) {
                    settings.doList=true;
                } else if (ccm.type==CL_DECOMPRESS) {
                    settings.doExtract=true;
                } else if (ccm.type==CL_EXTREME) {
                    settings.slow=true;
                    settings.level=ccm.val;
                } else if (ccm.type==CL_SLOW) {
                    settings.level=ccm.val;
                } else if (ccm.type==CL_FAST) {
                    settings.level=ccm.val;
                } else if (ccm.type==CL_STORE) {
                    settings.level=0;
                } else if (ccm.type==CL_THREADS) {
                    settings.topt=ccm.val;
                } else if (ccm.type==CL_WIKI) {
                    settings.witmode=true;
                } else if (ccm.type==CL_EDICT) {
                    settings.externaDict=ccm.valstr;
                    settings.staticd=true;
                } else if (ccm.type==CL_DICTF) {
                    settings.minfq=ccm.val;
                } else if (ccm.type==CL_RECUR) {
                    settings.rdepth=ccm.val;
                } else if (ccm.type==CL_VERBOSE) {
                    settings.verbose=ccm.val;
                } else if (ccm.type==CL_HELP) {
                    settings.showhelp=true;
                } else if (ccm.type==CL_PARSER) {
                    if (static_cast<ParserType>(settings.userPTsize)>P_LAST) settings.showhelp=true,PrintHelp();
                    for (size_t j=0; j<settings.userPTsize; j++) {
                        if (settings.userPT[j]==static_cast<ParserType>(ccm.val)) {
                            settings.showhelp=true;
                            PrintHelp();
                        }
                    }
                    settings.userPT[settings.userPTsize++]=static_cast<ParserType>(ccm.val);
                }
            }
            if (settings.userPTsize!=1) {
                settings.userPT[settings.userPTsize++]=P_LAST;
                assert(static_cast<ParserType>(settings.userPTsize)<P_LAST);
            }
        }
        if (settings.showhelp) PrintHelp();

        clock_t start_time=clock();  // in ticks

        CalcTables();
        // Execute user command
        if (settings.doList) {
            prog.List();
        } else if (settings.doExtract) {
            prog.Decompress();
        } else {
            prog.Compress();
        }

        printf("Time %1.2f sec.\n", double(clock()-start_time)/CLOCKS_PER_SEC);
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

