#include "img4filter.hpp"

Img4Filter::Img4Filter(std::string n, Settings &s, Filetype f):Filter(s) {  
    name=n;
    Type=f;
}

void Img4Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    // Hdr len
    out->putc((16>>8)&255);
    out->putc(16&255);
    // pal order
    for (int i=0; i<16; ++i) out->putc(pData[i]);
    // Reindex image
    const int imgSize=size-0;
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pData[b>>4]*16+pData[b&15]);
        //out->putc(b);
    }
} 

uint64_t Img4Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read hdr
    int b=in->getc();
    b=b*256+in->getc();
    assert(b==16);
    // Read pal
    uint8_t pal[16];
    for (int j=0; j<16; ++j) {
        int i=in->getc();
        pal[i]=j;
    }
    // Restore original indexes
    const int imgSize=size-(16+2); // - hdr/pal
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b>>4]*16+pal[b&15]);
        //out->putc(b);
    }
    fsize=size-(16+2);
    return fsize;
}

Img4Filter::~Img4Filter() {
}

