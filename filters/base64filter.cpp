#include "base64filter.hpp"

const char table1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline char valueb(char c){
       const char *p = strchr(table1, c);
       if(p) {
          return p-table1;
       } else {
          return 0;
       }
}
bool is_base64(unsigned char c) {
    return (isalnum(c) || (c=='+') || (c=='/')|| (c==10) || (c==13));
}
base64Filter::base64Filter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void base64Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
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

uint64_t base64Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
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
   // if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
       // }
    /*else if (mode==FCOMPARE){
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }*/
        
  fsize=outlen;

    return fsize;
}


base64Filter::~base64Filter() {
}

