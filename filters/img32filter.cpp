#include "img32filter.hpp"
// simple color transform (b, g, r) -> (g, g-r, g-b)

Img32Filter::Img32Filter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void Img32Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    int r,g,b,a;
    int witdh=int(info&0xffffffff);
    for (int i=0; i<size/info; i++) {
        for (int j=0; j<info/4; j++) {
            b=in->getc(), g=in->getc(), r=in->getc(); a=in->getc();
            out->putc(g);
            out->putc(g-r);
            out->putc(g-b);
            out->putc(a);
        }
        for (int j=0; j<info%4; j++) out->putc(in->getc());
    }
} 

uint64_t Img32Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    int r,g,b,a,p;
    int witdh=int(info&0xffffffff);
    bool rgb = (info&(1<<31))>0;
    if (rgb) info^=(1<<31);
    for (int i=0; i<size/witdh; i++) {
        p=i*info;
        for (int j=0; j<witdh/4; j++) {
            b=in->getc(), g=in->getc(), r=in->getc(), a=in->getc();
            out->putc(b-r); out->putc(b); out->putc(b-g); out->putc(a);
        }
        for (int j=0; j<witdh%4; j++) {
            out->putc(in->getc());
        }
    }
    fsize=size;
    return size;
}

Img32Filter::~Img32Filter() {
}

