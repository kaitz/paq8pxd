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
 
#define PROGNAME "paq8pxd129"  // Please change this if you change the program.

//#define MT            //uncomment for multithreading, compression only. Handled by CMake and gcc when -DMT is passed.
#ifndef DISABLE_SM
//#define SM              // For faster statemap
#endif
//#define VERBOSE         // Show extra info

#ifdef WINDOWS                       
#ifdef MT
//#define PTHREAD       //uncomment to force pthread to igore windows native threads !This seems broken!
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

#include "filters/textfilter.hpp"
#include "analyzer/codec.hpp"


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

 

#define AUD_DET(type,start_pos,header_len,data_len,wmode) return dett=(type),\
deth=int(header_len),detd=(data_len),info=(wmode),\
 in->setpos(start+(start_pos)),HDR

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

/*
void printStatus1(U64 n, U64 size) {
fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}


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
// Print progress: n is the number of bytes compressed or decompressed
void printStatus(U64 n, U64 size,int tid=-1) {
if (level>0 && tid>=0)  fprintf(stderr,"%2d %6.2f%%\b\b\b\b\b\b\b\b\b\b",tid, float(100)*n/(size+1)), fflush(stdout);
else if (level>0)  fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}

//////////////////// Compress, Decompress ////////////////////////////

//for block statistics, levels 0-5
U64 typenamess[datatypecount][5]={0}; //total type size for levels 0-5
U32 typenamesc[datatypecount][5]={0}; //total type count for levels 0-5
int itcount=0;               //level count

#if defined(WINDOWS)      
      HANDLE  hConsole;
#endif
void SetConColor(int color) {
#if defined(WINDOWS)      
      SetConsoleTextAttribute(hConsole, color);
#endif     
}

void compressStream(int sid, U64 size, File* in, File* out) {
    Encoder* enc;
    Predictors* pred;
    U64 segmentsize;
    U64 segmentlen;
    int segmentpos;
    int segmentinfo;
    Filetype segmenttype;
    U64 scompsize=0;
    segmentsize=size;
    U64 total=size;
    segmentpos=0;
    segmentinfo=0;
    segmentlen=0;
    if (level>0) {
        // Select predictor for a stream (sid)
        switch(sid) {
        default:
        case 0: { pred=new Predictor(); break;}
        case 1: { pred=new PredictorJPEG(); break;}
        case 2: { pred=new PredictorIMG1(); break;}
        case 3: { pred=new PredictorIMG4(); break;}
        case 4: { pred=new PredictorIMG8(); break;}
        case 5: { pred=new PredictorIMG24(); break;}
        case 6: { pred=new PredictorAUDIO2(); break;}
        case 7: { pred=new PredictorEXE(); break;}
        case 8: 
        case 9: 
        case 10: { pred=new PredictorTXTWRT(); break;}
        case 11: { pred=new PredictorDEC(); break;}
        case 12: { pred=new Predictor(); break;}
        }
    }
    enc=new Encoder (COMPRESS, out,*pred); 
    if ((sid>=0 && sid<=7) || sid==10 || sid==11|| sid==12) {
        while (segmentsize>0) {
            while (segmentlen==0) {
                segmenttype=(Filetype)segment(segmentpos++);
                for (int ii=0; ii<8; ii++) segmentlen<<=8,segmentlen+=segment(segmentpos++);
                for (int ii=0; ii<4; ii++) segmentinfo=(segmentinfo<<8)+segment(segmentpos++);
                if (!(streams.isStreamType(segmenttype,sid) )) segmentlen=0;
                if (level>0) {
                    enc->predictor.x.filetype=segmenttype;
                    enc->predictor.x.blpos=0;
                    enc->predictor.x.finfo=segmentinfo;
                }
            }
            for (U64 k=0; k<segmentlen; ++k) {
                //#ifndef MT
                if (!(segmentsize&0x1fff)) printStatus(total-segmentsize, total,sid);
                //#endif
                enc->compress(in->getc());
                segmentsize--;
            }
            /* #ifndef NDEBUG 
                            printf("Stream(%d) block from %0lu to %0lu bytes\n",i,segmentlen, out->curpos()-scompsize);
                            scompsize= out->curpos();
                            #endif */
            segmentlen=0;
        }
        enc->flush();
    }
    if (sid==8 || sid==9) {
        bool dictFail=false;
        FileTmp tm;
        TextFilter textf("text");
        textf.encode(in,&tm,segmentsize,sid==8);
        segmentlen=tm.curpos();
        streams.streams[sid]->streamsize=segmentlen;
        // -e0 option ignores larger wrt size
        if (segmentlen>=segmentsize && minfq!=0) {
            dictFail=true; //wrt size larger
            if (verbose>0) printf(" WRT larger: %d bytes. Ignoring\n",segmentlen-segmentsize); 
        } else {
            if (verbose>0) printf(" Total %0" PRIi64 " wrt: %0" PRIi64 "\n",segmentsize,segmentlen); 
        }
        tm.setpos(0);
        in->setpos(0);
        if (level>0) {
            enc->predictor.x.filetype=DICTTXT;
            enc->predictor.x.blpos=0;
            enc->predictor.x.finfo=-1;
        }
        if (dictFail==true) {
            streams.streams[sid]->streamsize=segmentsize+1;
            segmentlen=segmentsize;
            enc->compress(0xAA); //flag
        }else {
            streams.streams[sid]->streamsize=segmentlen+1;
            enc->compress(0); //flag
        }
        for (U64 k=0; k<segmentlen; ++k) {
            if (!(k&0x1fff)) printStatus(k, segmentlen,sid);
            #ifndef NDEBUG 
            if (!(k&0x3ffff) && k) {
                if (verbose>0) printf("Stream(%d) block pos %0lu compressed to %0lu bytes\n",sid,k, out->curpos()-scompsize);
                scompsize= out->curpos();
            }
            #endif
            if (dictFail==false) enc->compress(tm.getc());
            else                 enc->compress(in->getc());
        }
        tm.close();
        enc->flush();
        //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,segmentlen+0.0,ftello(out)-scompsize+0.0);
        segmentlen=segmentsize=0;   
    }
    
    if (level>0) delete pred;
    delete enc;
    printf("Stream(");
    SetConColor(sid+2);
    printf("%d",sid);
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
                        if (k==0 ){
                        if ( b==0xAA) doWRT=false; // flag set?
                    }
                        else tm.putc(b);
                        datasegmentsize--;
                    }
                    if (doWRT==true) {
                        tm.setpos(0);
                        TextFilter textf("text");
                        textf.decode(&tm,&streams.streams[i]->file,datasegmentlen,0);
                    } else {
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
            printf(PROGNAME " archiver (C) 2025, Matt Mahoney et al.\n"
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
        clock_t start_time=clock();  // in ticks
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
                   streams.streams[i]->streamsize=archive->get64();
                   uint64_t ofg=streams.streams[i]->streamsize;
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
            Codec codec(FCOMPRESS, &streams, &segment);
            for (int i=0; i<files; ++i) {
                printf("\n%d/%d  Filename: %s (%0" PRIi64 " bytes)\n", i+1, files, fname[i], fsize[i]); 
                codec.EncodeFile(fname[i], fsize[i]);// DetectStreams(fname[i], fsize[i]);
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
                if (streams.streams[i]->streamsize>0) archive->put64(streams.streams[i]->streamsize);
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
            Codec codec(FDECOMPRESS, &streams, &segment);
            for (int i=0; i<files; ++i) {
                std::string out(dir.c_str());
                out+=fname[i];
                printf("Reading file %s\n",out.c_str());
                codec.DecodeFile(out.c_str(), fsize[i]);// DecodeStreams(out.c_str(), fsize[i]);
            } 
            int d=segment(segment.pos++);
            if (d!=0xff) printf("Segmend end marker not found\n");
            for (int i=0; i<streams.Count(); ++i) {
                streams.streams[i]->file.close();
            }
        }
        archive->close();
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

