#include "shrinkfilter.hpp"
#include <vector>

shrinkFilter::shrinkFilter(std::string n, Settings &s, Filetype f):Filter(s) {
    name = n;
    Type = f;
}

shrinkFilter::~shrinkFilter() {}

void shrinkFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    assert(size>0);
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(info+1);
    in->blockread(&inbuf[0], size);
    size_t src_used=0, dst_used=0;
    int retval=0;
    retval=hwunshrink(&inbuf[0],size_t(size), &src_used, &dst[0], info+1, &dst_used);
    if (retval!=0) diffFound=1;
    else diffFound=0;
    out->blockwrite(&dst[0], info); 
}

uint64_t shrinkFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    assert(size>0);
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(size);
    in->blockread(&inbuf[0], size);
    
    size_t data_dst_sz=0, comp_sz=0;
    bool res=hwshrink(&inbuf[0], size, &dst[0], size, &comp_sz);
    out->blockwrite(&dst[0], comp_sz);
    if (res==false) fsize=1;
    else fsize=comp_sz;
    return comp_sz;
}
