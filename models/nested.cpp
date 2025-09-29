#include "nested.hpp"
extern U8 level;


  nestModel1::nestModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf), ic(0), bc(0),
   pc(0),vc(0), qc(0), lvc(0), wc(0),ac(0), ec(0), uc(0), sense1(0), sense2(0), w(0), N(12),
   cm(CMlimit(level>8?0x800000 :(MEM()/2) ), N,M_NEST)  {
  }

int nestModel1::p(Mixer& m,int val1,int val2){
    if (x.filetype==DBASE ||x.filetype==HDR ||x.filetype==DECA || x.filetype==ARM|| x.filetype==IMGUNK){
        if (val2==-1) return 1;
        for (int i=0; i<inputs(); i++)
        m.add(0);
        return 0;
    }
    if (x.bpos==0) {
    int c=x.c4&255, matched=1, vv;
    w*=((vc&7)>0 && (vc&7)<3);
    if (c&0x80) w = w*11*32 + c;
    const int lc = (c >= 'A' && c <= 'Z'?c+'a'-'A':c) ;
    if (lc == 'a' || lc == 'e' || lc == 'i' || lc == 'o' || lc == 'u'){ vv = 1; w = w*997*8 + (lc/4-22); } else
    if (lc >= 'a' && lc <= 'z' || c>128){ vv = 2; w = w*271*32 + lc-97; } else
    if (lc == ' ' || lc == '.' || lc == ',' || lc == '\n'|| lc == 5) vv = 3; else
    if (lc >= '0' && lc <= '9') vv = 4; else
    if (lc == 'y') vv = 5; else
    if (lc == '\'') vv = 6; else vv=(c&32)?7:0;
    vc = (vc << 3) | vv;
    if (vv != lvc) {
      wc = (wc << 3) | vv;
      lvc = vv;
    }
    switch(c) {
      case ' ': qc = 0; break;
      case '(': ic += 31; break;
      case ')': ic -= 31; break;
      case '[': ic += 11; break;
      case ']': ic -= 11; break;
      case '<': ic += 23; qc += 34; break;
      case '>': ic -= 23; qc /= 5; break;
      case ':': pc = 20; break;
      case '{': ic += 17; break;
      case '}': ic -= 17; break;
      case '|': pc += 223; break;
      case '"': pc += 0x40; break;
      case '\'': pc += 0x42; if (c!=(U8)(x.c4>>8)) sense2^=1; else ac+=(2*sense2-1); break;
      case 5: 
      case '\n': pc = qc = 0; break;
      case '.': pc = 0; break;
      case '!': pc = 0; break;
      case '?': pc = 0; break;
      case '#': pc += 0x08; break;
      case '%': pc += 0x76; break;
      case '$': pc += 0x45; break;
      case '*': pc += 0x35; break;
      case '-': pc += 0x3; break;
      case '@': pc += 0x72; break;
      case '&': qc += 0x12; break;
      case ';': qc /= 3; break;
      case '\\': pc += 0x29; break;
      case '/': pc += 0x11;
                if (buf.size() > 1 && buf(1) == '<') qc += 74;
                break;
      case '=': pc += 87; if (c!=(U8)(x.c4>>8)) sense1^=1; else ec+=(2*sense1-1); break;
      default: matched = 0;
    }
    if (x.c4==0x266C743B) uc=min(7,uc+1);
    else if (x.c4==0x2667743B) uc-=(uc>0);
    if (matched) bc = 0; else bc += 1;
    if (bc > 300) bc = ic = pc = qc = uc = 0;
if (val2==-1) return 1;
    cm.set(hash( (vv>0 && vv<3)?0:(lc|0x100), ic&0x3FF, ec&0x7, ac&0x7, uc ));
    cm.set(hash(ic, w, ilog2(bc+1)));
    
    cm.set(U32((3*vc+77*pc+373*ic+qc)&0xffff));
    cm.set(U32((31*vc+27*pc+281*qc)&0xffff));
    cm.set(U32((13*vc+271*ic+qc+bc)&0xffff));
    cm.set(U32((13*vc+ic)&0xffff));
    cm.set(U32((vc/3+pc)&0xffff));
    cm.set(U32((17*pc+7*ic)&0xffff));
    cm.set(U32((7*wc+qc)&0xffff));
    cm.set(U32((vc&0xffff)|((x.f4&0xf)<<16)));
    cm.set(U32(((3*pc)&0xffff)|((x.f4&0xf)<<16)));
    cm.set(U32((ic&0xffff)|((x.f4&0xf)<<16)));
  }
    
    cm.mix(m);
    m.set(vc&511,512);
  return 0;
}
