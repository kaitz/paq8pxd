#include "img24filter.hpp"

Img24Filter::Img24Filter(std::string n) {  
    name=n;
}

// simple color transform (b, g, r) -> (g, g-r, g-b)
void Img24Filter::encode(File *in, File *out, uint64_t size, int info) {
  int r,g,b, total=0;
  bool isPossibleRGB565 = true;
  for (uint64_t i=0; i<size/info; i++) {
    for (int j=0; j<info/3; j++) {
      b=in->getc(), g=in->getc(), r=in->getc();
      if (isPossibleRGB565) {
        int pTotal=total;
        total=min(total+1, 0xFFFF)*((b&7)==((b&8)-((b>>3)&1)) && (g&3)==((g&4)-((g>>2)&1)) && (r&7)==((r&8)-((r>>3)&1)));
        if (total>RGB565_MIN_RUN || pTotal>=RGB565_MIN_RUN) {
          b^=(b&8)-((b>>3)&1);
          g^=(g&4)-((g>>2)&1);
          r^=(r&8)-((r>>3)&1);
        }
        isPossibleRGB565=total>0;
      }
      out->putc(g);
      out->putc(g-r);
      out->putc(g-b);
    }
    for (int j=0; j<info%3; j++) out->putc(in->getc());
  }
} 

uint64_t Img24Filter::decode(File *in, File *out, uint64_t size, int info) {
  int r,g,b,p, total=0;
  bool isPossibleRGB565 = true;
  for (uint64_t i=0; i<size/info; i++) {
    p=i*info;
    for (int j=0; j<info/3; j++) {
      g=in->getc(), r=in->getc(), b=in->getc();
      r=g-r, b=g-b;
      if (isPossibleRGB565){
        if (total>=RGB565_MIN_RUN) {
          b^=(b&8)-((b>>3)&1);
          g^=(g&4)-((g>>2)&1);
          r^=(r&8)-((r>>3)&1);
        }
        total=min(total+1, 0xFFFF)*((b&7)==((b&8)-((b>>3)&1)) && (g&3)==((g&4)-((g>>2)&1)) && (r&7)==((r&8)-((r>>3)&1)));
        isPossibleRGB565=total>0;
      }
      out->putc(b);
      out->putc(g);
      out->putc(r);
    }
    for (int j=0; j<info%3; j++) {
        out->putc(in->getc());
    }
  }
  return size;
}

Img24Filter::~Img24Filter() {
}

