#include "defaultfilter.hpp"

DefaultFilter::DefaultFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void DefaultFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
  const int BLOCK=0x10000;
  U8 blk[BLOCK];
  uint64_t remaining=size;
  while (remaining) {
    size_t reads=min64(BLOCK, remaining);
    int ReadIn=in->blockread(&blk[0], reads);
    out->blockwrite(&blk[0], ReadIn);
    remaining-=ReadIn;
  }
} 

uint64_t DefaultFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=0x10000;  // block size
    U8 blk[BLOCK];
    diffFound=0;
    uint64_t remaining=size;
    while (remaining) {
        size_t reads=min64(BLOCK, remaining);
        int ReadIn=in->blockread(&blk[0], reads);
        out->blockwrite(&blk[0], ReadIn);
        remaining-=ReadIn;
    }
   fsize=size;
   return size;
}

DefaultFilter::~DefaultFilter() {
}

