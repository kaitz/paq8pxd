#pragma once
#include "filter.hpp"
#include "../prt/hash.hpp"
class gifFilter: public Filter {

public:
  gifFilter(std::string n, Filetype f=DEFAULT);
  ~gifFilter();
  void encode(File *in, File *out, uint64_t size, uint64_t info);
  uint64_t decode(File *in, File *out,  uint64_t size, uint64_t info);
  
};

#define LZW_TABLE_SIZE 9221

#define lzw_find(k) {\
  offset = ((k)*PHI)>>19; \
  int stride = (offset>0)?LZW_TABLE_SIZE-offset:1; \
  while (true){ \
    if ((index=table[offset])<0){ index=-offset-1; break; } \
    else if (dict[index]==int(k)){ break; } \
    offset-=stride; \
    if (offset<0) \
      offset+=LZW_TABLE_SIZE; \
  } \
}

#define lzw_reset { for (int i=0; i<LZW_TABLE_SIZE; table[i]=-1, i++); }


#define gif_write_block(count) { output[0]=(count);\
out->blockwrite(&output[0],  (count)+1  );\
outsize+=(count)+1; blocksize=0; }

#define gif_write_code(c) { buf+=(c)<<shift; shift+=bits;\
while (shift>=8) { output[++blocksize]=buf&255; buf>>=8;shift-=8;\
if (blocksize==bsize) gif_write_block(bsize); }}
