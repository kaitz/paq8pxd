#include "filter.hpp"

Filter::Filter():Type(DEFAULT),Info(0),name(""),diffFound(0),fsize(0) {
}

Filter::~Filter(){}

FMode Filter::compare(File *in, File *out, uint64_t size) {
    diffFound=0;
    for (uint64_t j=0; j<size; ++j) {
        int a=in->getc();
        int b=out->getc();
        if (a!=b && !diffFound) {
            diffFound=j+1;
            return FDISCARD;
        }
    }
    return FEQUAL;
}

uint64_t Filter::CompareFiles(File *in, File *out, uint64_t size,uint64_t info,FMode m) {
    FileTmp tmpcmp;
    if (m==FCOMPARE){
        diffFound=0;
        uint64_t len=decode(in,&tmpcmp,size,info);
        tmpcmp.setpos(0);
        FMode result=compare(&tmpcmp,out,len);
        if (result==FDISCARD) return diffFound;
    } else if (m==FDECOMPRESS) {
        decode(in,out,size,info);
    }
    tmpcmp.close();
    return 0;
}
