#include "img8filter.hpp"

Img8Filter::Img8Filter(std::string n, Settings &s, Filetype f):Filter(s) {  
    name=n;
    Type=f;
}

void Img8Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    // Hdr len
    out->putc(((256*0+256)>>8)&255);
    out->putc((256*0+256)&255);
    // pal order
    for (int i=0; i<256; ++i) out->putc(pData[i]); // pal order
    // Reindex image
    for (uint64_t j=0; j<size; ++j) {
        int b=in->getc();
        out->putc(pData[b]);
    }
} 

uint64_t Img8Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    // Read hdr
    int b=in->getc();
    b=b*256+in->getc();
    assert(b==(256));
    // Read pal
    uint8_t pal[256];
    for (int j=0; j<256; ++j) {
        int i=in->getc();
        pal[i]=j;
    }
    // Restore original indexes
    const uint64_t imgSize=size-(256+2); // - hdr/pal
    for (uint64_t j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b]);
        //out->putc(b);
    }
    fsize=size-(256+2);
    return fsize;
}

Img8Filter::~Img8Filter() {
}

