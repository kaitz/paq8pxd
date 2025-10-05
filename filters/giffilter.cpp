#include "giffilter.hpp"

gifFilter::gifFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

void gifFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
     int codesize=in->getc(),hdrsize=6,clearpos=0,bsize=0,code,offset=0;
  U64 diffpos=0,beginin= in->curpos(),beginout= out->curpos();
  Array<U8,1> output(4096);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(bsize);
  out->putc(clearpos>>8);
  out->putc(clearpos&255);
  out->putc(codesize);
  Array<int> table(LZW_TABLE_SIZE);  
  for (int phase=0; phase<2; phase++) {
    in->setpos( beginin);
    int bits=codesize+1,shift=0,buf=0;
    int blocksize=0,maxcode=(1<<codesize)+1,last=-1;//,dict[4096];
    Array<int> dict(4096);
    lzw_reset;
    bool end=false;
    while ((blocksize=in->getc())>0 &&  in->curpos()-beginin<len && !end) {
      for (int i=0; i<blocksize; i++) {
        buf|=in->getc()<<shift;
        shift+=8;
        while (shift>=bits && !end) {
          int code=buf&((1<<bits)-1);
          buf>>=bits;
          shift-=bits;
          if (!bsize && code!=(1<<codesize)) {
            hdrsize+=4; out->put32(0);
          }
          if (!bsize) bsize=blocksize;
          if (code==(1<<codesize)) {
            if (maxcode>(1<<codesize)+1) {
              if (clearpos && clearpos!=69631-maxcode) {diffFound=1;return;}
              clearpos=69631-maxcode;
            }
            bits=codesize+1, maxcode=(1<<codesize)+1, last=-1;
            lzw_reset;
          }
          else if (code==(1<<codesize)+1) end=true;
          else if (code>maxcode+1) {diffFound=1;return;}
          else {
            int j=(code<=maxcode?code:last),size=1;
            while (j>=(1<<codesize)) {
              output[4096-(size++)]=dict[j]&255;
              j=dict[j]>>8;
            }
            output[4096-size]=j;
            if (phase==1) out->blockwrite(&output[4096-size],  size  ); else diffpos+=size;
            if (code==maxcode+1) { if (phase==1) out->putc(j); else diffpos++; }
            if (last!=-1) {
              if (++maxcode>=8191) {diffFound=1;return;}
              if (maxcode<=4095)
              {
                int key=(last<<8)+j, index=-1;
                lzw_find(key);
                dict[maxcode]=key;
                table[(index<0)?-index-1:offset]=maxcode;
                if (phase==0 && index>0) {
                    hdrsize+=4;
                    j=diffpos-size-(code==maxcode);
                    out->put32(j);
                    diffpos=size+(code==maxcode);
                  }
                }
              //}
              if (maxcode>=((1<<bits)-1) && bits<12) bits++;
            }
            last=code;
          }
        }
      }
    }
  }
  diffpos= out->curpos();
  out->setpos(beginout);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(255-bsize);
  out->putc((clearpos>>8)&255);
  out->putc(clearpos&255);
  out->setpos(diffpos);
  diffFound=(in->curpos()-beginin==len-1)?0:1;
} 

uint64_t gifFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
     int diffcount=in->getc(), curdiff=0;
    Array<int> diffpos(4096);//, diffpos[4096];
  diffcount=((diffcount<<8)+in->getc()-6)/4;
  int bsize=255-in->getc();
  int clearpos=in->getc(); clearpos=(clearpos<<8)+in->getc();
  clearpos=(69631-clearpos)&0xffff;
  int codesize=in->getc(),bits=codesize+1,shift=0,buf=0,blocksize=0;
  if (diffcount>4096 || clearpos<=(1<<codesize)+2) return 1;
  int maxcode=(1<<codesize)+1, input,code,offset=0;
    Array<int> dict(4096);
      Array<int> table(LZW_TABLE_SIZE);
  lzw_reset;
  for (int i=0; i<diffcount; i++) {
    diffpos[i]=in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    if (i>0) diffpos[i]+=diffpos[i-1];
  }
  Array<U8,1> output(256);
  size-=6+diffcount*4;
  int last=in->getc(),total=size+1,outsize=1;
  out->putc(codesize);
  if (diffcount==0 || diffpos[0]!=0) gif_write_code(1<<codesize) else curdiff++;
  while (size!=0 && (input=in->getc())!=EOF) {
    size--;
    int key=(last<<8)+input, index=(code=-1);
    if (last<0) index=input; else lzw_find(key);
    code = index;
    if (curdiff<diffcount && total-(int)size>diffpos[curdiff]) curdiff++, code=-1;
    if (code<0) {
      gif_write_code(last);
      if (maxcode==clearpos) { gif_write_code(1<<codesize); bits=codesize+1, maxcode=(1<<codesize)+1; lzw_reset }
      else
      {
        ++maxcode;
        if (maxcode<=4095) { dict[maxcode]=key; table[(index<0)?-index-1:offset]=maxcode; }
        if (maxcode>=(1<<bits) && bits<12) bits++;
      }
      code=input;
    }
    last=code;
  }
  gif_write_code(last);
  gif_write_code((1<<codesize)+1);
  if (shift>0) {
    output[++blocksize]=buf&255;
    if (blocksize==bsize) gif_write_block(bsize);
  }
  if (blocksize>0) gif_write_block(blocksize);
  out->putc(0);
  fsize=outsize+1;

    return fsize;
}


gifFilter::~gifFilter() {
}

