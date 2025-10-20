#include "filter.hpp"

Filter::Filter():Type(DEFAULT),Info(0),name(""),diffFound(0),fsize(0) {
}

Filter::~Filter(){}

FMode Filter::compare(File *in, File *out, uint64_t size) {
    diffFound=0;
    const int BLOCK=0x10000;
    U8 blkin[BLOCK];
    U8 blkout[BLOCK];
    uint64_t remaining=size;

    while (remaining) {
        size_t reads=min(BLOCK, remaining);
        int ReadIn=in->blockread(&blkin[0], reads);
        int ReadOut=out->blockread(&blkout[0], reads);
        if (memcmp(blkin, blkout, ReadIn)!=0 || ReadIn!=reads || ReadIn!=ReadOut) {
            diffFound=size-remaining+1;
            return FDISCARD;
        }
        remaining-=ReadIn;
    }
    return FEQUAL;
}

uint64_t Filter::CompareFiles(File *in, File *out, uint64_t size,uint64_t info,FMode m) {
    FileTmp tmpcmp;
    if (m==FCOMPARE) {
        diffFound=0;
        uint64_t len=decode(in,&tmpcmp,size,info);
        tmpcmp.setpos(0);
        FMode result=compare(&tmpcmp,out,len);
        if (result==FDISCARD) {
            tmpcmp.close();
            return diffFound;
        }
    } else if (m==FDECOMPRESS) {
        decode(in,out,size,info);
    }
    tmpcmp.close();
    return 0;
}
