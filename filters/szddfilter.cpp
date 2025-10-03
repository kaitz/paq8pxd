#include "szddfilter.hpp"

szddFilter::szddFilter(std::string n) {  
    name=n;
} 

void szddFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    LZSS* lz77;
    lz77=new LZSS(in,out,info&0x1ffffff,(info>>25)*2);
    lz77->decompress();
    delete lz77;
} 

uint64_t szddFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    LZSS* lz77;
    int r=0;
    //Write out or compare

            lz77=new LZSS(in,out,size,(info>>25)*2);
             r=lz77->compress();
            delete lz77;
        
  fsize=r;

    return fsize;
}


szddFilter::~szddFilter() {
}

