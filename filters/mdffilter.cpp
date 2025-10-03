#include "mdffilter.hpp"

MDFFilter::MDFFilter(std::string n) {  
    name=n;
}

void MDFFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    U8 blk1[CHAN];
    int ql=size/(BLOCK+CHAN);
    out->dumpToDisk();
    out->putc(ql>>16); 
    out->putc(ql>>8);
    out->putc(ql);
    U64 beginin=in->curpos();
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

uint64_t MDFFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    int q=in->getc();   // count of channels
    q=(q<<8)+in->getc();
    q=(q<<8)+in->getc();
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    Array<U8,1> ptr(CHAN*q);
    out->dumpToDisk();
    in->blockread(&ptr[0], CHAN*q);
    for (int offset=0; offset<q; offset++) { 
        in->blockread(&blk[0], BLOCK);
        out->blockwrite(&blk[0], BLOCK);
        out->blockwrite(&ptr[offset*CHAN], CHAN);
    }
    fsize=size-3;
    return fsize;
}


MDFFilter::~MDFFilter() {
}

