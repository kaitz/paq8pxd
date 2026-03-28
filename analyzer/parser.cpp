#include "parser.hpp"

Parser::Parser():state(NONE),file_handle(nullptr),priority(0),name("default"),pinfo("") {
}

Parser::~Parser() {
}

std::string Parser::itos(int64_t x, int n) {
  assert(x>=0);
  assert(n>=0);
  std::string r;
  for (; x || n>0; x/=10, --n) r=std::string(1, '0'+x%10)+r;
  return r;
}

bool Parser::IsGrayscalePalette(uint8_t* palb, int n, int isRGBA) {
  int stride = 3+isRGBA, res = (n>0)<<8, order=1;
  int l=0;
  for (int i = 0; (i < n*stride) && (res>>8); i++) {
    int b = palb[l++];
    if (l>n*3) {
      res = 0;
      break;
    }
    if (!i) {
      res = 0x100|b;
      order = 1-2*(b>int(ilog2(n)/4));
      continue;
    }

    //"j" is the index of the current byte in this color entry
    int j = i%stride;
    if (!j){
      // load first component of this entry
      int k = (b-(res&0xFF))*order;
      res = res&((k>=0 && k<=8)<<8);
      res|=(res)?b:0;
    }
    else if (j==3)
      res&=((!b || (b==0xFF))*0x1FF); // alpha/attribute component must be zero or 0xFF
    else
      res&=((b==(res&0xFF))*0x1FF);
  }
  return (res>>8)>0;
}
