#include "reducefilter.hpp"
#include <vector>

reduceFilter::reduceFilter(std::string n, Filetype f) {
    name = n;
    Type = f;
}

reduceFilter::~reduceFilter() {}

void reduceFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    assert(size>0);
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(info+1);
    in->blockread(&inbuf[0], size);
    size_t src_used=0, dst_used=0;
    int retval=0;
    retval=hwexpand(&inbuf[0],size_t(size), info,  4, &dst_used,&dst[0]);
    if (retval!=0) diffFound=1;
    else diffFound=0;
    out->blockwrite(&dst[0], info); 
}

uint64_t reduceFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    assert(size>0);
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(size);
    in->blockread(&inbuf[0], size);
    
    size_t data_dst_sz=0, comp_sz=0;
    bool res=hwreduce(&inbuf[0], size, 4, &dst[0], size, &comp_sz);
    out->blockwrite(&dst[0], comp_sz);
    if (res==false) fsize=1;
    else fsize=comp_sz;
    return comp_sz;
}
