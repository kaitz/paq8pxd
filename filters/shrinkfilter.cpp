#include "shrinkfilter.hpp"
#include <vector>

shrinkFilter::shrinkFilter(std::string n, Filetype f) {
    name = n;
    Type = f;
}

shrinkFilter::~shrinkFilter() {}

void shrinkFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(info+1);
    in->blockread(&inbuf[0], size);
    size_t src_used=0, dst_used=0;
    int retval=0;
    retval=hwunshrink(&inbuf[0],size_t(size), &src_used, &dst[0], info+1, &dst_used);
    if (retval!=0) diffFound=1;
    out->blockwrite(&dst[0], info); 
    diffFound=0;
}

uint64_t shrinkFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    Array<uint8_t> inbuf(size);
    Array<uint8_t> dst(size);
    in->blockread(&inbuf[0], size);
    
    size_t data_dst_sz, comp_sz;
    hwshrink(&inbuf[0], size,  &dst[0], size-1, &comp_sz) ;
    out->blockwrite(&dst[0], comp_sz);

    fsize=comp_sz;
    return comp_sz;
}
