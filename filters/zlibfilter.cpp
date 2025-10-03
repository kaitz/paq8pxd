#include "zlibfilter.hpp"

MTFList  MTF(81);

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
zlibFilter::zlibFilter(std::string n) {  
    name=n;
} 

void zlibFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
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
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) {diffFound=1;return;}
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
  if (minCount==LIMIT) {diffFound=1;return;}
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
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) {diffFound=1;return;}
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
  diffFound= main_ret==Z_STREAM_END?0:1;
} 

uint64_t zlibFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
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
      /*if (mode==FDECOMPRESS) */out->blockwrite(&zout[0],   have  );
      //else if (mode==FCOMPARE) for (int j=0; j<have; j++) if (zout[j]!=out->getc() && !diffFound) diffFound=recpos+j+1;
      recpos+=have;
      
    } while (rec_strm.avail_out==0);
  }
  while (diffIndex<=diffCount) {
    /*if (mode==FDECOMPRESS)*/ out->putc(diffByte[diffIndex]);
    //else if (mode==FCOMPARE) if (diffByte[diffIndex]!=out->getc() && !diffFound) diffFound=recpos+1;
    diffIndex++;
    recpos++;
  }  
  deflateEnd(&rec_strm);
  fsize=recpos==len ? len : 0;
    return fsize;
}


zlibFilter::~zlibFilter() {
}

