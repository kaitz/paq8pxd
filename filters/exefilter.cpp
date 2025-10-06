#include "exefilter.hpp"

ExeFilter::ExeFilter(std::string n, Filetype f) {
    name=n;
    Type=f;
}

void ExeFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    U32 begin=U32(info&0xffffffff);
    const int BLOCK=0x10000;
    U8 blk[BLOCK];
    out->put32((U32)begin);
    // Transform
    for (uint64_t offset=0; offset<size; offset+=BLOCK) {
        int rsize=min(int(size)-int(offset), BLOCK);
        int bytesRead= in->blockread(&blk[0], rsize);
        for (int i=bytesRead-1; i>=5; --i) {
            if ((blk[i-4]==0xe8 || blk[i-4]==0xe9 || (blk[i-5]==0x0f && (blk[i-4]&0xf0)==0x80))
                    && (blk[i]==0||blk[i]==0xff)) {
                int a=(blk[i-3]|blk[i-2]<<8|blk[i-1]<<16|blk[i]<<24)+U32(offset)+begin+i+1;
                a<<=7;
                a>>=7;
                blk[i]=a>>24;
                blk[i-1]=a^176;
                blk[i-2]=(a>>8)^176;
                blk[i-3]=(a>>16)^176;
            }
        }
        out->blockwrite(&blk[0], bytesRead);
    }
}

uint64_t ExeFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=0x10000;  // block size
    int begin, offset=6, a;
    U8 c[6];
    begin=in->getc()<<24;
    begin|=in->getc()<<16;
    begin|=in->getc()<<8;
    begin|=in->getc();
    size-=4;
    for (int i=4; i>=0; i--) c[i]=in->getc();  // Fill queue

    while (offset<size+6) {
        memmove(c+1, c, 5);
        if (offset<=size) c[0]=in->getc();
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
        out->putc(c[5]);
        offset++;
    }
    fsize=size;
    return size;
}

ExeFilter::~ExeFilter() {
}

