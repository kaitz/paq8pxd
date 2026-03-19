#include "img8filter.hpp"

Img8Filter::Img8Filter(std::string n, Settings &s, Filetype f):Filter(s) {  
    name=n;
    Type=f;
}

void Img8Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    //hdr len
    out->putc(((256*4+256)>>8)&255);
    out->putc((256*4+256)&255);
    // pal
    const int TCOLORS=256;
    for (int j=0; j<TCOLORS; ++j) {
        uint32_t c=in->get32();
        out->put32(c);
        Color8 colori;
        colori.c=c;
        colori.i=j;
        bmcolor.push_back(colori);
    }
    // sort colors by Cartesian distance
    std::sort(bmcolor.begin(), bmcolor.end(), [](const Color8 &a, const Color8 &b){
        int a1=std::sqrt(((a.rgba[0]-255) * (a.rgba[0]-255)) + (a.rgba[1]*a.rgba[1]) + (a.rgba[2]*a.rgba[2]));
        int b1=std::sqrt(((b.rgba[0]-255) * (b.rgba[0]-255)) + (b.rgba[1]*b.rgba[1]) + (b.rgba[2]*b.rgba[2]));
        // plain a.c < b.c also works but is worse
        //return (a.c < b.c);
        return (a1 < b1);
    });

    uint8_t pal[256];
    // map to new order
    for (int i=0; i<TCOLORS; ++i) {
        for (int j=0; j<TCOLORS; ++j) {
            Color8 colori=bmcolor[j];
            if (colori.i==i) {
                pal[i]=j;
                out->putc(j);
                break;
            } 
        }
    }
    const int imgSize=size-1024;
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b]);
    }
} 

uint64_t Img8Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    //hdr
    int b=in->getc();
    b=b*256+in->getc();
    assert(b==(1024+256));
    // pal
    const int TCOLORS=256;
    for (int j=0; j<TCOLORS; ++j) {
        uint32_t c=in->get32();
        out->put32(c);
    }
    // original index order
    uint8_t pal[256];
    for (int j=0; j<TCOLORS; ++j) {
        int i=in->getc();
        pal[i]=j;
    }
    const int imgSize=size-(1024+256+2); // - hdr/pal
    for (int j=0; j<imgSize; ++j) {
        int b=in->getc();
        out->putc(pal[b]);
    }
    fsize=size-(256+2);
    return fsize;
}

Img8Filter::~Img8Filter() {
}

