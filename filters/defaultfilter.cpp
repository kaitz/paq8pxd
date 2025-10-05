#include "defaultfilter.hpp"

DefaultFilter::DefaultFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void DefaultFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  for (uint64_t offset=0; offset<size; offset+=BLOCK) {
    int rsize=min(int(size)-int(offset), BLOCK);
    int bytesRead= in->blockread(&blk[0], rsize);
    out->blockwrite(&blk[0], bytesRead);
  }
} 

uint64_t DefaultFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    for (int j=0; j<size; j+=BLOCK) {
        int size1=min(int(size)-j, BLOCK);
        int bytesRead=in->blockread(&blk[0], size1);
        out->blockwrite(&blk[0], bytesRead);
    }
   fsize=size;
   return size;
}


DefaultFilter::~DefaultFilter() {
}

