#pragma once
#include "../prt/types.hpp"
#include "../prt/helper.hpp"
#include "../prt/array.hpp"
#include "../prt/mixer.hpp"
#include "model.hpp"
#include "../prt/contextmap.hpp"
/*
====== XML model ======
*/

#define CacheSize (1<<5)

#if (CacheSize&(CacheSize-1)) || (CacheSize<8)
  #error Cache size must be a power of 2 bigger than 4
#endif
struct XMLAttribute {
  U32 Name, Value, Length;
};

struct XMLContent {
  U32 Data, Length, Type;
};

struct XMLTag {
  U32 Name, Length;
  int Level;
  bool EndTag, Empty;
  XMLContent Content;
  struct XMLAttributes {
    XMLAttribute Items[4];
    U32 Index;
  } Attributes;    
};

struct XMLTagCache {
  XMLTag Tags[CacheSize];
  U32 Index;
};

enum ContentFlags {
  Text        = 0x001,
  Number      = 0x002,
  Date        = 0x004,
  Time        = 0x008,
  URL         = 0x010,
  Link        = 0x020,
  Coordinates = 0x040,
  Temperature = 0x080,
  ISBN        = 0x100
};

enum XMLState {
  None               = 0,
  ReadTagName        = 1,
  ReadTag            = 2,
  ReadAttributeName  = 3,
  ReadAttributeValue = 4,
  ReadContent        = 5,
  ReadCDATA          = 6,
  ReadComment        = 7
};


#define DetectContent(){ \
  if ((x.c4&0xF0F0F0F0)==0x30303030){ \
    int i = 0, j = 0; \
    while ((i<4) && ( (j=(x.c4>>(8*i))&0xFF)>=0x30 && j<=0x39 )) \
      i++; \
\
    if (i==4 && ( ((c8&0xFDF0F0FD)==0x2D30302D && buf(9)>=0x30 && buf(9)<=0x39) || ((c8&0xF0FDF0FD)==0x302D302D) )) \
      (*Content).Type |= Date; \
  } \
  else if (((c8&0xF0F0FDF0)==0x30302D30 || (c8&0xF0F0F0FD)==0x3030302D) && buf(9)>=0x30 && buf(9)<=0x39){ \
    int i = 2, j = 0; \
    while ((i<4) && ( (j=(c8>>(8*i))&0xFF)>=0x30 && j<=0x39 )) \
      i++; \
\
    if (i==4 && (x.c4&0xF0FDF0F0)==0x302D3030) \
      (*Content).Type |= Date; \
  } \
\
  if ((x.c4&0xF0FFF0F0)==0x303A3030 && buf(5)>=0x30 && buf(5)<=0x39 && ((buf(6)<0x30 || buf(6)>0x39) || ((c8&0xF0F0FF00)==0x30303A00 && (buf(9)<0x30 || buf(9)>0x39)))) \
    (*Content).Type |= Time; \
\
  if ((*Content).Length>=8 && (c8&0x80808080)==0 && (x.c4&0x80808080)==0) \
    (*Content).Type |= Text; \
\
  if ((c8&0xF0F0FF)==0x3030C2 && (x.c4&0xFFF0F0FF)==0xB0303027){ \
    int i = 2; \
    while ((i<7) && buf(i)>=0x30 && buf(i)<=0x39) \
      i+=(i&1)*2+1; \
\
    if (i==10) \
      (*Content).Type |= Coordinates; \
  } \
\
  if ((x.c4&0xFFFFFA)==0xC2B042 && B!=0x47 && (((x.c4>>24)>=0x30 && (x.c4>>24)<=0x39) || ((x.c4>>24)==0x20 && (buf(5)>=0x30 && buf(5)<=0x39)))) \
    (*Content).Type |= Temperature; \
\
  if (B>=0x30 && B<=0x39) \
    (*Content).Type |= Number; \
\
  if (x.c4==0x4953424E && buf(5)==0x20) \
    (*Content).Type |= ISBN; \
} 
class XMLModel1: public Model {
  BlockData& x;
  Buf& buf;
  XMLTagCache Cache;
  XMLState State, pState;
  U32 c8, WhiteSpaceRun, pWSRun, IndentTab, IndentStep, LineEnding,lastState;
  ContextMap cm;
  U32 StateBH[8],stateContext;
  XMLTag *pTag,*Tag;
    XMLAttribute *Attribute;
    XMLContent *Content ;
public:
  XMLModel1(BlockData& bd,U32 val=0);
  int inputs() {return 4*cm.inputs();}
  int nets() {return 0;}
  int netcount() {return 0;}
  void setContexts();
int p(Mixer& m,int val1=0,int val2=0);
virtual ~XMLModel1(){ }
};


