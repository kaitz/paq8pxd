#include "uudfilter.hpp"

// needst test with proper input
uudFilter::uudFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void uudFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
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

uint64_t uudFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
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
   // if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
       // }
   /* else if (mode==FCOMPARE){
       // out->setpos(0);
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }*/
    //return outlen;
  fsize=outlen;

    return fsize;
}


uudFilter::~uudFilter() {
}

