#include "cdfilter.hpp"

CDFilter::CDFilter(std::string n) {  
    name=n;
}

void CDFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
    const int BLOCK=2352;
    U8 blk[BLOCK];
    out->dumpToDisk();
    out->putc((len%BLOCK)>>8);
    out->putc(len%BLOCK);
    for (int offset=0; offset<len; offset+=BLOCK) {
        if (offset+BLOCK > len) {
            in->blockread(&blk[0], len-offset);
            out->blockwrite(&blk[0], len-offset);
        } else {
            in->blockread(&blk[0], BLOCK);
            if (info==3) blk[15]=3;
            if (offset==0) out->blockwrite(&blk[12], 4+4*(blk[15]!=1));
            out->blockwrite(&blk[16+8*(blk[15]!=1)], 2048+276*(info==3));
            if (offset+BLOCK*2 > len && blk[15]!=1) out->blockwrite(&blk[16], 4);
        }
    }
} 

uint64_t CDFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=2352;
    U8 blk[BLOCK];
    long i=0, i2=0;
    int a=-1, bsize=0, q=in->getc();
    q=(q<<8)+in->getc();
    size-=2;
    out->dumpToDisk();
    while (i<size) {
        if (size-i==q) {
            in->blockread(blk, q);
            out->blockwrite(blk, q);
            i+=q;
            i2+=q;
        } else if (i==0) {
            in->blockread(blk+12, 4);
            if (blk[15]!=1) in->blockread(blk+16, 4);
            bsize=2048+(blk[15]==3)*276;
            i+=4*(blk[15]!=1)+4;
        } else {
            a=(blk[12]<<16)+(blk[13]<<8)+blk[14];
        }
        in->blockread(blk+16+(blk[15]!=1)*8, bsize);
        i+=bsize;
        if (bsize>2048) blk[15]=3;
        if (blk[15]!=1 && size-q-i==4) {
            in->blockread(blk+16, 4);
            i+=4;
        }
        expand_cd_sector(blk, a, 0);
        out->blockwrite(blk, BLOCK);
        i2+=BLOCK;
    }
    fsize=i2;
    return i2;
}


CDFilter::~CDFilter() {
}

