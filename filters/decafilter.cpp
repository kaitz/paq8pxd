#include "decafilter.hpp"

DecAFilter::DecAFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void DecAFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
  int len=size;
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  int count=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size1=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size1  );
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

uint64_t DecAFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
  const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    U8 c;
    int b=0;
    U32 count=0;
    for (int j=0; j<size; j+=BLOCK) {
        int size1=min(int(size)-j, BLOCK);
        int bytesRead=in->blockread(&blk[0], size1);
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
        out->blockwrite(&blk[0],   bytesRead  );
    }
    if(count<16) fsize=diffFound=1; //fail if replaced below threshold
    fsize=size;
    return fsize;
}

DecAFilter::~DecAFilter() {
}
