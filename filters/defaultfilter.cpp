#include "defaultfilter.hpp"

DefaultFilter::DefaultFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void DefaultFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
  const int BLOCK=0x10000;
  U8 blk[BLOCK];
  for (uint64_t j=0; j<size; j+=BLOCK) {
    int rsize=min(int(size)-int(j), BLOCK);
    int bytesRead=in->blockread(&blk[0], rsize);
    out->blockwrite(&blk[0], bytesRead);
  }
} 

uint64_t DefaultFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=0x10000;  // block size
    U8 blk[BLOCK];
    diffFound=0;
    uint64_t remaining=size;

    while (remaining) {
        size_t reads=min(BLOCK, remaining);
        int ReadIn=in->blockread(&blk[0], reads);
        out->blockwrite(&blk[0], ReadIn);
        remaining-=ReadIn;
    }
   fsize=size;
   return size;
}


DefaultFilter::~DefaultFilter() {
}

