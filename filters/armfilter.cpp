#include "armfilter.hpp"

armFilter::armFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void armFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
    const int BLOCK=0x10000;
    U8 blk[BLOCK];
    int count=0;
    for (int j=0; j<len; j+=BLOCK) {
        int size=min(int(len)-j, BLOCK);
        int bytesRead=in->blockread(&blk[0], size  );
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

uint64_t armFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=0x10000;  // block size
    U8 blk[BLOCK];
    U8 c;
    int b=0;
    U32 count=0;
    for (int j=0; j<size; j+=BLOCK) {
        int size1=min(int(size)-j, BLOCK);
        int bytesRead=in->blockread(&blk[0],   size1  );
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
        out->blockwrite(&blk[0],   bytesRead  );
    }
    if(count<16) fsize=diffFound=1;; //fail if replaced below threshold
    fsize=size;
    return fsize;
}

armFilter::~armFilter() {
}

