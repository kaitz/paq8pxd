#include "img32filter.hpp"

Img32Filter::Img32Filter(std::string n) {  
    name=n;
}

// simple color transform (b, g, r) -> (g, g-r, g-b)
void Img32Filter::encode(File *in, File *out, uint64_t size, int info) {
  int r,g,b,a;
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

uint64_t Img32Filter::decode(File *in, File *out, uint64_t size, int info) {
  int r,g,b,a,p;
  bool rgb = (info&(1<<31))>0;
  if (rgb) info^=(1<<31);
  for (int i=0; i<size/info; i++) {
    p=i*info;
    for (int j=0; j<info/4; j++) {
      b=in->getc(), g=in->getc(), r=in->getc(), a=in->getc();
        out->putc(b-r); out->putc(b); out->putc(b-g); out->putc(a);
    }
    for (int j=0; j<info%4; j++) {
        out->putc(in->getc());
    }
  }
  return size;
}


Img32Filter::~Img32Filter() {
}

