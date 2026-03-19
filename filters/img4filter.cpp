#include "img4filter.hpp"

Img4Filter::Img4Filter(std::string n, Settings &s, Filetype f):Filter(s) {  
    name=n;
    Type=f;
}

void Img4Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    //hdr len
    out->putc(((16*4+16)>>8)&255);
    out->putc((16*4+16)&255);
    // pal
    const int TCOLORS=16;
    for (int j=0; j<TCOLORS; ++j) {
        uint32_t c=in->get32();
        out->put32(c);
        Color4 colori;
        colori.c=c;
        colori.i=j;
        bmcolor.push_back(colori);
    }
    // sort colors by Cartesian distance
    std::sort(bmcolor.begin(), bmcolor.end(), [](const Color4 &a, const Color4 &b){
        int a1=std::sqrt(((a.rgba[0]-255) * (a.rgba[0]-255)) + (a.rgba[1]*a.rgba[1]) + (a.rgba[2]*a.rgba[2]));
        int b1=std::sqrt(((b.rgba[0]-255) * (b.rgba[0]-255)) + (b.rgba[1]*b.rgba[1]) + (b.rgba[2]*b.rgba[2]));
        // plain a.c < b.c also works but is worse
        //return (a.c < b.c);
        return (a1 < b1);
    });

    uint8_t pal[16];
    // map to new order
    for (int i=0; i<TCOLORS; ++i) {
        for (int j=0; j<TCOLORS; ++j) {
            Color4 colori=bmcolor[j];
            if (colori.i==i) {
                pal[i]=j;
                out->putc(j);
                break;
            } 
        }
    }
    const int imgSize=size-64;
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b>>4]*16+pal[b&15]);
    }
} 

uint64_t Img4Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    //hdr
    int b=in->getc();
    b=b*256+in->getc();
    assert(b==(64+16));
    // pal
    const int TCOLORS=16;
    for (int j=0; j<TCOLORS; ++j) {
        uint32_t c=in->get32();
        out->put32(c);
    }
    // original index order
    uint8_t pal[16];
    for (int j=0; j<TCOLORS; ++j) {
        int i=in->getc();
        pal[i]=j;
    }
    const int imgSize=size-(64+16+2); // - hdr/pal
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b>>4]*16+pal[b&15]);
    }
    fsize=size-(16+2);
    return fsize;
}

Img4Filter::~Img4Filter() {
}

