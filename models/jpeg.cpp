#include "jpeg.hpp"
//////////////////////////// jpegModel /////////////////////////

// Model JPEG. Return 1 if a JPEG file is detected or else 0.
// Only the baseline and 8 bit extended Huffman coded DCT modes are
// supported.  The model partially decodes the JPEG image to provide
// context for the Huffman coded symbols.

extern U8 level;

extern bool slow;

  jpegModelx::jpegModelx(BlockData& bd):  MaxEmbeddedLevel(3),idx(-1),
   lastPos(0), jpeg(0),app(0),sof(0),sos(0),data(0),ht(8),htsize(0),huffcode(0),
  huffbits(0),huffsize(0),rs(-1), mcupos(0), huf(128), mcusize(0),linesize(0),
  hbuf(2048),color(10), pred(4), dc(0),width(0), row(0),column(0),cbuf(0x20000),
  cpos(0), rs1(0), ssum(0), ssum1(0), ssum2(0), ssum3(0),cbuf2(0x20000),adv_pred(4),adv_pred1(3),adv_pred2(3),adv_pred3(3), run_pred(6),
  sumu(8), sumv(8), ls(10),lcp(7), zpos(64), blockW(10), blockN(10),  SamplingFactors(4),dqt_state(-1),dqt_end(0),qnum(0),qnum16(0),qnum16b(0),pr0(0),
  qtab(256),qmap(10),N(41),M(66),cxt(N),m1(32+32+3+4+1+1+1+1+N*3+1+3*M+6,2050+3+1024+64+1024 +256+16+64,bd, 8,0,3,2),
   apm{{0x40000,20-4},{0x40000,20-4},{0x20000,20-4},
   {0x20000,20-4},{0x20000,20-4},{0x20000,20-4},{0x20000,20-4},{0x20000,27},{0x20000,20-4}},
   x(bd),buf(bd.buf),MJPEGMap( {21, 3, 128, 127}),
  hbcount(2),prev_coef(0),prev_coef2(0), prev_coef_rs(0), rstpos(0),rstlen(0),
  hmap(level>10?0x8000000:(CMlimit(MEM()*2)),9,N,bd),skip(0), smx(256*256),jmiss(0),zux(0),ccount(1),lma(0),ama(0) {
  }

  int jpegModelx::p(Mixer& m,int val1,int val2){

 if (idx < 0){
    memset(&images[0], 0, sizeof(images));
    idx = 0;
    lastPos = buf.pos;
  }
  if (!x.bpos && !x.blpos) images[idx].next_jpeg=0;
  
  // Be sure to quit on a byte boundary
  if (!x.bpos) images[idx].next_jpeg=images[idx].jpeg>1;
  if (x.bpos && !images[idx].jpeg) return images[idx].next_jpeg;
  if (!x.bpos && images[idx].app>0){
    --images[idx].app;
    if (idx<MaxEmbeddedLevel && buf(4)==FF && (buf(3)==SOI && buf(2)==FF && ((buf(1)& 0xFE)==0xC0 || buf(1)==0xC4 || (buf(1)>=0xDB && buf(1)<=0xFE)) ))
      memset(&images[++idx], 0, sizeof(JPEGImage));
  }
  if (images[idx].app>0) return images[idx].next_jpeg;
  if (!x.bpos) {


    // Parse.  Baseline DCT-Huffman JPEG syntax is:
    // SOI APPx... misc... SOF0 DHT... SOS data EOI
    // SOI (= FF D8) start of image.
    // APPx (= FF Ex) len ... where len is always a 2 byte big-endian length
    //   including the length itself but not the 2 byte preceding code.
    //   Application data is ignored.  There may be more than one APPx.
    // misc codes are DQT, DNL, DRI, COM (ignored).
    // SOF0 (= FF C0) len 08 height width Nf [C HV Tq]...
    //   where len, height, width (in pixels) are 2 bytes, Nf is the repeat
    //   count (1 byte) of [C HV Tq], where C is a component identifier
    //   (color, 0-3), HV is the horizontal and vertical dimensions
    //   of the MCU (high, low bits, packed), and Tq is the quantization
    //   table ID (not used).  An MCU (minimum compression unit) consists
    //   of 64*H*V DCT coefficients for each color.
    // DHT (= FF C4) len [TcTh L1...L16 V1,1..V1,L1 ... V16,1..V16,L16]...
    //   defines Huffman table Th (1-4) for Tc (0=DC (first coefficient)
    //   1=AC (next 63 coefficients)).  L1..L16 are the number of codes
    //   of length 1-16 (in ascending order) and Vx,y are the 8-bit values.
    //   A V code of RS means a run of R (0-15) zeros followed by S (0-15)
    //   additional bits to specify the next nonzero value, negative if
    //   the first additional bit is 0 (e.g. code x63 followed by the
    //   3 bits 1,0,1 specify 7 coefficients: 0, 0, 0, 0, 0, 0, 5.
    //   Code 00 means end of block (remainder of 63 AC coefficients is 0).
    // SOS (= FF DA) len Ns [Cs TdTa]... 0 3F 00
    //   Start of scan.  TdTa specifies DC/AC Huffman tables (0-3, packed
    //   into one byte) for component Cs matching C in SOF0, repeated
    //   Ns (1-4) times.
    // EOI (= FF D9) is end of image.
    // Huffman coded data is between SOI and EOI.  Codes may be embedded:
    // RST0-RST7 (= FF D0 to FF D7) mark the start of an independently
    //   compressed region.
    // DNL (= FF DC) 04 00 height
    //   might appear at the end of the scan (ignored).
    // FF 00 is interpreted as FF (to distinguish from RSTx, DNL, EOI).

    // Detect JPEG (SOI followed by a valid marker)
    if (!images[idx].jpeg && buf(4)==FF && buf(3)==SOI && buf(2)==FF && (( (buf(1) & 0xFE)==0xC0 || buf(1)==0xC4 || (buf(1)>=0xDB && buf(1)<=0xFE))||(buf(1)>>4==0xe || buf(1)==0xdb)) ){
      images[idx].jpeg=1;
      images[idx].offset = buf.pos-4;
      images[idx].sos=images[idx].sof=images[idx].htsize=images[idx].data=0, images[idx].app=(buf(1)>>4==0xE)*2;
      mcusize=huffcode=huffbits=huffsize=mcupos=cpos=0, rs=-1;
      memset(&huf[0], 0, sizeof(huf));
      memset(&pred[0], 0, pred.size()*sizeof(int));
      rstpos=rstlen=0,ccount=1;
    }

    // Detect end of JPEG when data contains a marker other than RSTx
    // or byte stuff (00), or if we jumped in position since the last byte seen
    if (images[idx].jpeg && images[idx].data && ((buf(2)==FF && buf(1) && (buf(1)&0xf8)!=RST0) || (buf.pos-lastPos>1)) ) {
      jassert((buf(1)==EOI) || (buf.pos-lastPos>1));
      finish(true);
      //images[idx].jpeg=images[idx].next_jpeg; // ??
    }
    lastPos = buf.pos;
    if (!images[idx].jpeg) return images[idx].next_jpeg;
     //if (!images[idx].jpeg) return 0; //??
    // Detect APPx or COM field
    bool mainfileds=(( ((buf(3)>=0xC1) && (buf(3)<=0xCF) && (buf(3)!=DHT)) || ((buf(3)>=0xDC) && (buf(3)<=0xFE)) ) && idx==0);
    if (!images[idx].data && !images[idx].app && buf(4)==FF &&  (buf(3)>>4==0xe || buf(3)==COM||mainfileds)) {//(((buf(3)>=0xC1) && (buf(3)<=0xCF) && (buf(3)!=DHT)) || ((buf(3)>=0xDC) && (buf(3)<=0xFE)))){
//&& (buf(3)>>4==0xe || buf(3)==COM))
      images[idx].app=buf(2)*256+buf(1)+2;
   if (idx)
        jassert( buf.pos + images[idx].app < images[idx].offset + images[idx-1].app );
    }
    // Save pointers to sof, ht, sos, data,
    if (buf(5)==FF && buf(4)==SOS) {
      int len=buf(3)*256+buf(2);
      if (len==6+2*buf(1) && buf(1) && buf(1)<=4)  // buf(1) is Ns
        images[idx].sos=buf.pos-5, images[idx].data=images[idx].sos+len+2, images[idx].jpeg=2;
    }
    if (buf(4)==FF && buf(3)==DHT && images[idx].htsize<8) images[idx].ht[images[idx].htsize++]=buf.pos-4;
    if (buf(4)==FF && (buf(3)& 0xFE)==SOF0) images[idx].sof=buf.pos-4;
    if (buf(4)==FF && buf(3)==0xc3) images[idx].jpeg=images[idx].next_jpeg=0; //sof3
    // Parse Quantizazion tables
    if (buf(4)==FF && buf(3)==DQT) {
      dqt_end=buf.pos+buf(2)*256+buf(1)-1, dqt_state=0,qnum16b=0,qnum16=0;
  }
    else if (dqt_state>=0) {
      if (buf.pos>=dqt_end)
        dqt_state = -1;
      else if (qnum16 ){
        if (dqt_state%65==0){
          qnum = buf(1);qnum16b=0;
          qnum16=qnum>>4;qnum=qnum&0xf;
           } else if ((qnum16b&1)==0) {
          jassert(buf(1)>0);
          jassert(qnum>=0 && qnum<4);
          images[idx].qtab[qnum*64+((dqt_state%65)-1)]=buf(1)-1;
        }
        if((qnum16b&1)==0)dqt_state++;
        qnum16b++;
      }
      else {
        if (dqt_state%65==0){
          qnum = buf(1);qnum16b=0;
          qnum16=qnum>>4;qnum=qnum&0xf;
           } else {
          jassert(buf(1)>0);
          jassert(qnum>=0 && qnum<4);
          images[idx].qtab[qnum*64+((dqt_state%65)-1)]=buf(1)-1;
        }
        dqt_state++;qnum16b++;
      }
    }

    // Restart
    if (buf(2)==FF && (buf(1)&0xf8)==RST0) {
      huffcode=huffbits=huffsize=mcupos=0, rs=-1;
      memset(&pred[0], 0, pred.size()*sizeof(int));
      rstlen=column+row*width-rstpos;
      rstpos=column+row*width;
    }
  }

  {
    // Build Huffman tables
    // huf[Tc][Th][m] = min, max+1 codes of length m, pointer to byte values
    if (buf.pos==images[idx].data && x.bpos==1) {
      //jassert(htsize>0);
      int i;
      for (i=0; i<images[idx].htsize; ++i) {
        int p=images[idx].ht[i]+4;  // pointer to current table after length field
        int end=p+buf[p-2]*256+buf[p-1]-2;  // end of Huffman table
        int count=0;  // sanity check
        while (p<end && end<buf.pos && end<p+2100 && ++count<10) {
          int tc=buf[p]>>4, th=buf[p]&15;
          if (tc>=2 || th>=4) break;
          jassert(tc>=0 && tc<2 && th>=0 && th<4);
          HUF* h=&huf[tc*64+th*16]; // [tc][th][0];
          int val=p+17;  // pointer to values
          int hval=tc*1024+th*256;  // pointer to RS values in hbuf
          int j;
          for (j=0; j<256; ++j) // copy RS codes
            hbuf[hval+j]=buf[val+j];
          int code=0;
          for (j=0; j<16; ++j) {
            h[j].min=code;
            h[j].max=code+=buf[p+j+1];
            h[j].val=hval;
            val+=buf[p+j+1];
            hval+=buf[p+j+1];
            code*=2;
          }
          p=val;
          jassert(hval>=0 && hval<2048);
        }
        jassert(p==end);
      }
      huffcode=huffbits=huffsize=0, rs=-1;
// load default tables
      if (!images[idx].htsize){
        for (int tc = 0; tc < 2; tc++) {
          for (int th = 0; th < 2; th++) {
            HUF* h = &huf[tc*64+th*16];
            int hval = tc*1024 + th*256;
            int code = 0, c = 0, xd = 0;

            for (int i = 0; i < 16; i++) {
              switch (tc*2+th) {
                case 0: xd = bits_dc_luminance[i]; break;
                case 1: xd = bits_dc_chrominance[i]; break;
                case 2: xd = bits_ac_luminance[i]; break;
                case 3: xd = bits_ac_chrominance[i];
              }

              h[i].min = code;
              h[i].max = (code+=xd);
              h[i].val = hval;
              hval+=xd;
              code+=code;
              c+=xd;
            }

            hval = tc*1024 + th*256;
            c--;

            while (c >= 0){
              switch (tc*2+th) {
                case 0: xd = values_dc_luminance[c]; break;
                case 1: xd = values_dc_chrominance[c]; break;
                case 2: xd = values_ac_luminance[c]; break;
                case 3: xd = values_ac_chrominance[c];
              }

              hbuf[hval+c] = xd;
              c--;
            }
          }
        }
        images[idx].htsize = 4;
      }
      // Build Huffman table selection table (indexed by mcupos).
      // Get image width.
      if (!images[idx].sof && images[idx].sos) return  images[idx].next_jpeg;//return 0;
      int ns=buf[images[idx].sos+4];
      int nf=buf[images[idx].sof+9];
      jassert(ns<=4 && nf<=4);
      mcusize=0;  // blocks per MCU
      int hmax=0;  // MCU horizontal dimension
      for (i=0; i<ns; ++i) {
        for (int j=0; j<nf; ++j) {
          if (buf[images[idx].sos+2*i+5]==buf[images[idx].sof+3*j+10]) { // Cs == C ?
            int hv=buf[images[idx].sof+3*j+11];  // packed dimensions H x V
            SamplingFactors[j] = hv;
            if (hv>>4>hmax) hmax=hv>>4;
            hv=(hv&15)*(hv>>4);  // number of blocks in component C
            ccount=max(hv,ccount);
            jassert(hv>=1 && hv+mcusize<=10);
            while (hv) {
              jassert(mcusize<10);
              hufsel[0][mcusize]=buf[images[idx].sos+2*i+6]>>4&15;
              hufsel[1][mcusize]=buf[images[idx].sos+2*i+6]&15;
              jassert (hufsel[0][mcusize]<4 && hufsel[1][mcusize]<4);
              color[mcusize]=i;
              int tq=buf[images[idx].sof+3*j+12];  // quantization table index (0..3)
              jassert(tq>=0 && tq<4);
              images[idx].qmap[mcusize]=tq; // quantizazion table mapping
              --hv;
              ++mcusize;
            }
          }
        }
      }

      jassert(hmax>=1 && hmax<=10);
      int j;
      for (j=0; j<mcusize; ++j) {
        ls[j]=0;
        for (int i=1; i<mcusize; ++i) if (color[(j+i)%mcusize]==color[j]) ls[j]=i;
        ls[j]=(mcusize-ls[j])<<6;
        //blockW[j] = ls[j];
      }
      for (j=0; j<64; ++j) zpos[zzu[j]+8*zzv[j]]=j;
      width=buf[images[idx].sof+7]*256+buf[images[idx].sof+8];  // in pixels
      width=(width-1)/(hmax*8)+1;  // in MCU
      jassert(width>0);
      mcusize*=64;  // coefficients per MCU
      row=column=0;
      
      // we can have more blocks than components then we have subsampling
      int x=0, y=0; 
      for (j = 0; j<(mcusize>>6); j++) {
        int i = color[j];
        int w = SamplingFactors[i]>>4, h = SamplingFactors[i]&0xf;
        blockW[j] = x==0?mcusize-64*(w-1):64;
        blockN[j] = y==0?mcusize*width-64*w*(h-1):w*64;
        x++;
        if (x>=w) { x=0; y++; }
        if (y>=h) { x=0; y=0; }
      }
    }
  }
  const int y=x.y;

  // Decode Huffman
  {
    if (mcusize && buf(1+(!x.bpos))!=FF) {  // skip stuffed byte
      jassert(huffbits<=32);
      huffcode+=huffcode+y;
      ++huffbits;
      if (rs<0) {
        jassert(huffbits>=1 && huffbits<=16);
        const int ac=(mcupos&63)>0;
        jassert(mcupos>=0 && (mcupos>>6)<10);
        jassert(ac==0 || ac==1);
        const int sel=hufsel[ac][mcupos>>6];
        jassert(sel>=0 && sel<4);
        const int i=huffbits-1;
        jassert(i>=0 && i<16);
        const HUF *h=&huf[ac*64+sel*16]; // [ac][sel];
        jassert(h[i].min<=h[i].max && h[i].val<2048 && huffbits>0);
        if (huffcode<h[i].max) {
          jassert(huffcode>=h[i].min);
          int k=h[i].val+huffcode-h[i].min;
          jassert(k>=0 && k<2048);
          rs=hbuf[k];
          huffsize=huffbits;
        }
      }
      if (rs>=0) {
        if (huffsize+(rs&15)==huffbits) { // done decoding
          rs1=rs;
          int xe=0;  // decoded extra bits
          if (mcupos&63) {  // AC
            if (rs==0) { // EOB
              mcupos=(mcupos+63)&-64;
              jassert(mcupos>=0 && mcupos<=mcusize && mcupos<=640);
              while (cpos&63) {
                cbuf2[cpos]=0;
                cbuf[cpos]=(!rs)?0:(63-(cpos&63))<<4; cpos++; rs++;
              }
            }
            else {  // rs = r zeros + s extra bits for the next nonzero value
                    // If first extra bit is 0 then value is negative.
              jassert((rs&15)<=10);
              const int r=rs>>4;
              const int s=rs&15;
              jassert(mcupos>>6==(mcupos+r)>>6);
              mcupos+=r+1;
              xe=huffcode&((1<<s)-1);
              if (s && !(xe>>(s-1))) xe-=(1<<s)-1;
              for (int i=r; i>=1; --i) {
                cbuf2[cpos]=0;
                cbuf[cpos++]=i<<4|s;
              }
              cbuf2[cpos]=xe;
              cbuf[cpos++]=(s<<4)|(huffcode<<2>>s&3)|12;
              ssum+=s;
            }
          }
          else {  // DC: rs = 0S, s<12
            jassert(rs<12);
            ++mcupos;
            xe=huffcode&((1<<rs)-1);
            if (rs && !(xe>>(rs-1))) xe-=(1<<rs)-1;
            jassert(mcupos>=0 && mcupos>>6<10);
            const int comp=color[mcupos>>6];
            jassert(comp>=0 && comp<4);
            dc=pred[comp]+=xe;
            while (cpos&63) cpos++;  // recover,  mobile phone images (thumbnail)
            //jassert((cpos&63)==0);
            cbuf2[cpos]=dc;
            cbuf[cpos++]=(dc+1023)>>3;
            if ((mcupos>>6)==0) {
              ssum1=0;
              ssum2=ssum3;
            } else {
              if (color[(mcupos>>6)-1]==color[0]) ssum1+=(ssum3=ssum);
              ssum2=ssum1;
            }
            ssum=rs;
          }
          jassert(mcupos>=0 && mcupos<=mcusize);
          if (mcupos>=mcusize) {
            mcupos=0;
            if (++column==width) column=0, ++row;
          }
          huffcode=huffsize=huffbits=0, rs=-1;

          // UPDATE_ADV_PRED !!!!
          {
            const int acomp=mcupos>>6, q=64*images[idx].qmap[acomp];
            const int zz=mcupos&63, cpos_dc=cpos-zz;
            const bool norst=rstpos!=column+row*width;
            if (zz==0) {
              for (int i=0; i<8; ++i) sumu[i]=sumv[i]=0;
              // position in the buffer of first (DC) coefficient of the block
              // of this same component that is to the west of this one, not
              // necessarily in this MCU
              int offset_DC_W = cpos_dc - blockW[acomp];
              // position in the buffer of first (DC) coefficient of the block
              // of this same component that is to the north of this one, not
              // necessarily in this MCU
              int offset_DC_N = cpos_dc - blockN[acomp];
              for (int i=0; i<64; ++i) {
                sumu[zzu[i]]+=(zzv[i]&1?-1:1)*(zzv[i]?16*(16+zzv[i]):185)*(images[idx].qtab[q+i]+1)*cbuf2[offset_DC_N+i];
                sumv[zzv[i]]+=(zzu[i]&1?-1:1)*(zzu[i]?16*(16+zzu[i]):185)*(images[idx].qtab[q+i]+1)*cbuf2[offset_DC_W+i];
              }
            }
            else {
              sumu[zzu[zz-1]]-=(zzv[zz-1]?16*(16+zzv[zz-1]):185)*(images[idx].qtab[q+zz-1]+1)*cbuf2[cpos-1];
              sumv[zzv[zz-1]]-=(zzu[zz-1]?16*(16+zzu[zz-1]):185)*(images[idx].qtab[q+zz-1]+1)*cbuf2[cpos-1];
            }

            for (int i=0; i<3; ++i)
            {
              run_pred[i]=run_pred[i+3]=0;
              for (int st=0; st<10 && zz+st<64; ++st) {
                const int zz2=zz+st;
                int p=sumu[zzu[zz2]]*i+sumv[zzv[zz2]]*(2-i);
                p/=(images[idx].qtab[q+zz2]+1)*185*(16+zzv[zz2])*(16+zzu[zz2])/128;
                if (zz2==0 && (norst || ls[acomp]==64)) p-=cbuf2[cpos_dc-ls[acomp]];
                p=(p<0?-1:+1)*ilog(abs(p)+1);
                if (st==1)  adv_pred1[i]=p;
                if (st==2)  adv_pred2[i]=p;
                if (st==3)  adv_pred3[i]=p;
                if (st==0) {
                  adv_pred[i]=p;
                }
                else if (abs(p)>abs(adv_pred[i])+2 && abs(adv_pred[i]) < 210) {
                  if (run_pred[i]==0) run_pred[i]=st*2+(p>0);
                  if (abs(p)>abs(adv_pred[i])+21 && run_pred[i+3]==0) run_pred[i+3]=st*2+(p>0);
                }
              }
            }
            xe=0;
            for (int i=0; i<8; ++i) xe+=(zzu[zz]<i)*sumu[i]+(zzv[zz]<i)*sumv[i];
            xe=(sumu[zzu[zz]]*(2+zzu[zz])+sumv[zzv[zz]]*(2+zzv[zz])-xe*2)*4/(zzu[zz]+zzv[zz]+16);
            xe/=(images[idx].qtab[q+zz]+1)*185;
            if (zz==0 && (norst || ls[acomp]==64)) xe-=cbuf2[cpos_dc-ls[acomp]];
            adv_pred[3]=(xe<0?-1:+1)*ilog(abs(xe)+1);

            for (int i=0; i<4; ++i) {
              const int a=(i&1?zzv[zz]:zzu[zz]), b=(i&2?2:1);
              if (a<b) xe=65535;
              else {
                const int zz2=zpos[zzu[zz]+8*zzv[zz]-(i&1?8:1)*b];
                xe=(images[idx].qtab[q+zz2]+1)*cbuf2[cpos_dc+zz2]/(images[idx].qtab[q+zz]+1);
                xe=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));
              }
              lcp[i]=xe;
            }
            if ((zzu[zz]*zzv[zz])!=0){
              const int zz2=zpos[zzu[zz]+8*zzv[zz]-9];
              xe=(images[idx].qtab[q+zz2]+1)*cbuf2[cpos_dc+zz2]/(images[idx].qtab[q+zz]+1);
              lcp[4]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));

              xe=(images[idx].qtab[q+zpos[8*zzv[zz]]]+1)*cbuf2[cpos_dc+zpos[8*zzv[zz]]]/(images[idx].qtab[q+zz]+1);
              lcp[5]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));

              xe=(images[idx].qtab[q+zpos[zzu[zz]]]+1)*cbuf2[cpos_dc+zpos[zzu[zz]]]/(images[idx].qtab[q+zz]+1);
              lcp[6]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));
            }
            else
              lcp[4]=lcp[5]=lcp[6]=65535;
              lma=0;
            for (int i=0; i<7; ++i) {
            lma=(lma<<1)|(lcp[i]==65535);
            }
            int prev1=0,prev2=0,cnt1=0,cnt2=0,r=0,s=0;
            prev_coef_rs = cbuf[cpos-64];
            for (int i=0; i<acomp; i++) {
              xe=0;
              xe+=cbuf2[cpos-(acomp-i)*64];
              if (zz==0 && (norst || ls[i]==64)) xe-=cbuf2[cpos_dc-(acomp-i)*64-ls[i]];
              if (color[i]==color[acomp]-1) { prev1+=xe; cnt1++; r+=cbuf[cpos-(acomp-i)*64]>>4; s+=cbuf[cpos-(acomp-i)*64]&0xF; }
              if (color[acomp]>1 && color[i]==color[0]) { prev2+=xe; cnt2++; }
            }
            if (cnt1>0) prev1/=cnt1, r/=cnt1, s/=cnt1, prev_coef_rs=(r<<4)|s;
            if (cnt2>0) prev2/=cnt2;
            prev_coef=(prev1<0?-1:+1)*ilog(11*abs(prev1)+1)+(cnt1<<20);
            prev_coef2=(prev2<0?-1:+1)*ilog(11*abs(prev2)+1);
           
            if (column==0 && blockW[acomp]>64*acomp) run_pred[1]=run_pred[2], run_pred[0]=0, adv_pred[1]=adv_pred[2], adv_pred[0]=0;
            if (row==0 && blockN[acomp]>64*acomp) run_pred[1]=run_pred[0], run_pred[2]=0, adv_pred[1]=adv_pred[0], adv_pred[2]=0;
             ama=0;
            for (int i=0; i<3; ++i) 
            ama=ama|(((adv_pred[i]==0))<<i);
          } // !!!!

        }
      }
    }
  }
  // Update model

  // Estimate next bit probability
  if (!images[idx].jpeg || !images[idx].data) return images[idx].next_jpeg;//return 0;
  if (buf(1+(!x.bpos))==FF) {
    m.add(128);
    m.set(0,  9);
    m.set(0, 1025);
    m.set(buf(1), 1024);
    m.set(0, 512);
    x.Misses+=x.Misses ;
    m.set(0, 4096 ); 
    m.set(0, 64);
    m.set(0, 4096);
    m.set(0, 1024);
    return 2;
  }
  if (rstlen>0 && rstlen==column+row*width-rstpos && mcupos==0 && (int)huffcode==(1<<huffbits)-1) {
    m.add(2047);
    m.set(0,  9);
    m.set(0,  1025); 
    m.set(buf(1), 1024);
    m.set(0,  512 );
    x.Misses+=x.Misses;
    m.set(0, 4096 );
    m.set(0, 64);
    m.set(0, 4096);
    m.set(0, 1024);
    return 2;
  }
  if (val1==1) {if (++hbcount>2 ) hbcount=0;return /*skip++,*/ 1;  }
  m1.update();
  hmap.update();

  // Update context
  const int comp=color[mcupos>>6];
  const int coef=(mcupos&63)|comp<<6;
  const int hc=(huffcode*4+((mcupos&63)==0)*2+(comp==0))|1<<(huffbits+2);
  const bool firstcol=column==0 && blockW[mcupos>>6]>mcupos;
  const int hc2 = (1 << (huffbits - huffsize)) + ((huffcode & ((1 << (huffbits - huffsize)) - 1)) << 1) +  static_cast<int>(huffsize > 0);
  if (++hbcount>2 || huffbits==0) hbcount=0;
  jassert(coef>=0 && coef<256);
  const int zu=zzu[mcupos&63], zv=zzv[mcupos&63];
  zux=zux<<2;zux|=(zu>0)*2|zv>0;
    if (hbcount==0) {
    U32 n=hc*N;
    cxt[0]=hash(++n, coef, adv_pred[2]/12+(run_pred[2]<<8), ssum2>>6, prev_coef/72);
    cxt[1]=hash(++n, coef, adv_pred[0]/12+(run_pred[0]<<8), ssum2>>6, prev_coef/72);
    cxt[2]=hash(++n, coef, adv_pred[1]/11+(run_pred[1]<<8), ssum2>>6);
    cxt[3]=hash(++n, rs1, adv_pred[2]/7,adv_pred1[2]/11, adv_pred2[2]/11, adv_pred3[2]/11,run_pred[5]/2, prev_coef/10);
    cxt[4]=hash(++n, rs1, adv_pred[0]/7, adv_pred1[0]/11,adv_pred2[0]/11,adv_pred3[0]/11,run_pred[3]/2, prev_coef/10);
    cxt[5]=hash(++n, rs1, adv_pred[1]/11,adv_pred1[1]/11,adv_pred2[1]/11,adv_pred3[1]/11, run_pred[4]);
    cxt[6]=hash(++n, adv_pred[2]/14, run_pred[2], adv_pred[0]/14, run_pred[0]);
    cxt[7]=hash(++n, cbuf[cpos-blockN[mcupos>>6]]>>4, adv_pred[3]/17, run_pred[1], run_pred[5]);
    cxt[8]=hash(++n, cbuf[cpos-blockW[mcupos>>6]]>>4, adv_pred[3]/17, run_pred[1], run_pred[3]);
    cxt[9]=hash(++n, lcp[0]/(22/ccount), lcp[1]/(22/ccount), adv_pred[1]/7, run_pred[1]);
    cxt[10]=hash(++n, lcp[0]/(22/ccount), lcp[1]/(22/ccount), mcupos&63, lcp[4]/(30/ccount));
    cxt[11]=(zu)==0?hash(++n, prev_coef/22,prev_coef2/50):hash(++n, zu/2, lcp[0]/13, lcp[2]/30);//run_pred[2]
    cxt[12]=(zv)==0?hash(++n, ssum2>>5, mcupos&63):hash(++n, zv/2, lcp[1]/13, lcp[3]/30);
    cxt[13]=hash(++n, rs1, prev_coef/42, prev_coef2/34,  lcp[0]/(ccount>2?12:60),lcp[2]/14,lcp[1]/(ccount>2?12:60),lcp[3]/14 );
    cxt[14]=hash(++n, mcupos&63, column>>1);
    cxt[15]=hash(++n, column>>3, min(5+2*(!comp),zu+zv), lcp[0]/10,lcp[2]/40,lcp[1]/10,lcp[3]/40 );
    cxt[16]=hash(++n, ssum>>3, mcupos&63);
    cxt[17]=hash(++n, rs1, mcupos&63, run_pred[1]);
    cxt[18]=hash(++n, coef, ssum2>>5, adv_pred[3]/30, (comp)?hash(prev_coef/22,prev_coef2/50):ssum/((mcupos&0x3F)+1));
    cxt[19]=hash(++n, lcp[0]/40, lcp[1]/40, adv_pred[1]/28,   (comp)?prev_coef/40+((prev_coef2/40)<<20):lcp[4]/22, min(7,zu+zv), ssum/(2*(zu+zv)+1)   );
    cxt[20]=hash(++n, ccount>1?((zv<<8)|zu):zv , cbuf[cpos-blockN[mcupos>>6]], adv_pred[2]/28, run_pred[2]);//use (zv<<8)|zu for subsampling
    cxt[21]=hash(++n, ccount>1?((zv<<8)|zu):zu, cbuf[cpos-blockW[mcupos>>6]], adv_pred[0]/28, run_pred[0]);
    cxt[22]=hash(n, adv_pred[2]/7,adv_pred1[2]/7,adv_pred2[2]/7, run_pred[2]);
    cxt[23]=hash(n, adv_pred[0]/7,adv_pred1[0]/7,adv_pred2[0]/7, run_pred[0],ccount>2?(lcp[zu<zv]/14):-1);
    cxt[24]=hash(n, adv_pred[1]/7,adv_pred1[1]/7,adv_pred2[1]/7, run_pred[1],ccount>2?(lcp[zu<zv]/14):-1);
    cxt[25]=hash(++n,  ccount>1?((zv<<8)|zu):zv , lcp[1]/14, adv_pred[2]/16, run_pred[5]);
    cxt[26]=hash(++n,  ccount>1?((zv<<8)|zu):zu, lcp[0]/14, adv_pred[0]/16, run_pred[3]);
    cxt[27]=hash(++n, lcp[0]/14, lcp[1]/14, adv_pred[3]/16);
    cxt[28]=hash(++n, coef, prev_coef/10, prev_coef2/20);
    cxt[29]=hash(++n, coef, ssum>>2, prev_coef_rs);
    cxt[30]=hash(++n, coef, adv_pred[1]/17,  lcp[(zu<zv)]/24,lcp[2]/20,lcp[3]/24 );
    cxt[31]=hash(++n, coef, adv_pred[3]/11,  lcp[(zu<zv)]/(ccount>2?10:50),lcp[2+3*(zu*zv>1)]/(ccount>2?10:50),lcp[3+3*(zu*zv>1)]/(ccount>2?10:50) );
    cxt[32]=hash(++n,  coef, adv_pred[2]/17 , coef, adv_pred[1]/11 , ssum>>2,run_pred[0]);
    cxt[33]=hash(++n,  ccount>1?zux:-1,zv, run_pred[2]/2 ,  coef, run_pred[5]/2 , min(7,zu+zv),adv_pred[0]/12);
    cxt[34]=hash(++n, coef, adv_pred[2]/17,  lcp[(zu<zv)]/24,lcp[2]/20,lcp[3]/24 );
    cxt[35]=hash(hc,ccount>1?((zv<<8)|zu):zu,cbuf[cpos-blockN[mcupos>>6]], adv_pred[2]/28, run_pred[2],lcp[1]/14/* art*/ ,    adv_pred[0]/28, run_pred[0],lcp[4]/14/*art*/ ) ;
    cxt[36]=hash(++n, (zv<<8)|zu, lcp[0]/22, lcp[4]/30, ssum2>>6,prev_coef/28 ,  adv_pred[0]/30, adv_pred[1]/30,adv_pred[2]/30 ,  run_pred[0], run_pred[1],run_pred[2] );
    cxt[37]=hash(++n,hc, adv_pred[3] / 13, prev_coef / 11, static_cast<int>(zu + zv < 4));
    cxt[38]=hash(++n, lcp[1]/22, lcp[2]/22, adv_pred[2]/7, run_pred[2],ccount>1?(lcp[4]/14):-1);
    cxt[39]=hash(++n, lcp[0]/22, lcp[1]/22, lcp[4]/22, mcupos&63, lcp[3]/30);// a
    cxt[40]=hash(++n, coef, adv_pred[0]/11,  lcp[(zu<zv)]/(ccount>2?10:(ccount>1?25:50)),lcp[2+3*(zu*zv>1)]/(ccount>2?10:(ccount>1?25:50)),lcp[3+3*(zu*zv>1)]/(ccount>2?10:(ccount>1?25:50)) );//10 art
  }

  // Predict next bit
  m1.add(128);
  assert(hbcount<=2);
  int p;

if (slow==true) x.count=0;
    switch(hbcount) {
    case 0: {int p1=0;
            
            for (int i=0; i<N; ++i){  
                p1=p=hmap.ps(i,finalize64(cxt[i],32),y); 
                const int n0=hmap.sn0(), n1=hmap.sn1();
                m1.add((p-2048)>>3); 
                m1.add(p=stretch(p)); 
                m.add(p>>(1+hmap.siy())); 
                int p0=4095-p1;
                m.add((((p1&n0)-(p0&n1))*1)/(4*4));
                m1.add((((p1&n1)-(p0&n0))*1)/(4*4));
            }
        } break;
    case 1: { int hc=1+(huffcode&1)*3;int p1=0;
            
            for (int i=0; i<N; ++i){
                p1=p=hmap.p(i,hc,y); 
                const int n0=hmap.sn0(), n1=hmap.sn1();
                m1.add((p-2048)>>3); 
                m1.add(p=stretch(p)); 
                m.add(p>>(1+hmap.siy())); 
                int p0=4095-p1;
                m.add((((p1&n0)-(p0&n1))*1)/(4*4));
                m1.add((((p1&n1)-(p0&n0))*1)/(4*4));
            }
        } break;
    default: { int hc=1+(huffcode&1); int p1=0;
            
            for (int i=0; i<N; ++i){  
                p1=p=hmap.p(i,hc,y); 
                const int n0=hmap.sn0(), n1=hmap.sn1();
                m1.add((p-2048)>>3); 
                m1.add(p=stretch(p)); 
                m.add(p>>(1+hmap.siy())); 
                int p0=4095-p1;
                m.add((((p1&n0)-(p0&n1))*1)/(4*4));
                m1.add((((p1&n1)-(p0&n0))*1)/(4*4));
            }
        } break;
    }
    x.JPEG.state =0x1000u |
     ((hc2 & 0xFF) << 4) |
     (static_cast<int>(ama> 0) << 3) |
      (static_cast<int>(huffbits > 4) << 2) | 
      (static_cast<int>(comp == 0) << 1) | 
      static_cast<int>(zu + zv < 5);
  if( hbcount == 0 ) {
      int i=0;
      Map1[i++].set(hash(hc>> 2,coef, adv_pred[0]/11));  // for examp.
      Map1[i++].set(hash(hc>> 2,coef, adv_pred[1]/11));
      Map1[i++].set(hash(hc>> 2,coef, adv_pred[2]/11));
      Map1[i++].set(hash(hc>> 2,coef, adv_pred[3]/11));
      Map1[i++].set(hash(hc>> 2,coef, adv_pred1[0]/11));  
      Map1[i++].set(hash(hc>> 2,coef, adv_pred1[1]/11));
      Map1[i++].set(hash(hc>> 2,coef, adv_pred1[2]/11));
      Map1[i++].set(hash(hc>> 2,coef, lcp[0]/7));
      Map1[i++].set(hash(hc>> 2,coef, lcp[1]/7));
      if (ccount==1){
          Map1[i++].set(hash(hc>> 2,coef, run_pred[0])); 
          Map1[i++].set(hash(hc>> 2,coef, run_pred[1]));
          Map1[i++].set(hash(hc>> 2,coef, run_pred[2]));
          Map1[i++].set(hash(hc>> 2,coef, run_pred[3]));
          Map1[i++].set(hash(hc>> 2,coef, run_pred[4]));
      }else {
          Map1[i++].set(hash(hc>> 2,coef, lcp[2]/7));
          Map1[i++].set(hash(hc>> 2,coef, lcp[3]/7));
          Map1[i++].set(hash(hc>> 2,coef, lcp[4]/7));
          Map1[i++].set(hash(hc>> 2,coef, lcp[5]/7));
          Map1[i++].set(hash(hc>> 2,coef, lcp[6]/7));
      }
      Map1[i++].set(hash(hc>> 2,coef, rs1)); 
      Map1[i++].set(hash(hc>> 2,coef, prev_coef));
      Map1[i++].set(hash(hc>> 2,coef, prev_coef2));
      
      Map1[i++].set(hash(hc>> 2,coef, ssum>>2));
      Map1[i++].set(hash(hc>> 2,coef, ssum2>>2));
      Map1[i++].set(hash(hc>> 2,coef, ssum3>>2)); 
      Map1[i++].set(hash(hc>> 2,coef, prev_coef_rs));
      Map1[i++].set(hash(hc>> 2,adv_pred[0]/16,adv_pred1[0]/16,adv_pred2[0]/16,run_pred[0]));
      Map1[i++].set(hash(hc>> 2,adv_pred[1]/16,adv_pred1[0]/16,adv_pred2[0]/16,run_pred[1]));
      Map1[i++].set(hash(hc>> 2,lcp[0]/14, lcp[1]/14));
      Map1[i++].set(hash(hc>> 2,lcp[2]/14, lcp[3]/14));
      Map1[i++].set(hash(hc>> 2,lcp[3]/14, lcp[4]/14));
      Map1[i++].set(hash(hc>> 2, adv_pred[3]/17, run_pred[1], run_pred[5]));
      Map1[i++].set(hash(hc>> 2, adv_pred2[2]/17, run_pred[1], run_pred[3]));
      Map1[i++].set(hash(hc>> 2, adv_pred[2]/17, run_pred[0], run_pred[4]));
      Map1[i++].set(hash(hc>> 2, cbuf[cpos-blockN[mcupos>>6]]));
      Map1[i++].set(hash(hc>> 2, cbuf[cpos-blockW[mcupos>>6]]));
      Map1[i++].set(hash(hc>> 2, zu,zv));
      Map1[i++].set(hash(hc>> 2, ssum>>1, prev_coef2/10));
      Map1[i++].set(hash(hc>> 2, coef, ssum1>>2));
      Map1[i++].set(hash(hc>> 2, adv_pred[2]/17, run_pred[0], run_pred[4]));
      Map1[i++].set(hash(hc>> 2, adv_pred1[2]/17, run_pred[0], run_pred[2]));
      Map1[i++].set(hash(hc>> 2, adv_pred[1]/16,  run_pred[3]));
      Map1[i++].set(hash(hc>> 2, adv_pred[0]/16, run_pred[1]));
      Map1[i++].set(hash(hc>> 2, rs,rs1));
      Map1[i++].set(hash(hc>> 2, ssum2>>2,prev_coef2/42));
      Map1[i++].set(hash(hc>> 2, ssum3>>2,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred[1]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred[0]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred[2]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred[3]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred[0]/7,rs1));
      Map1[i++].set(hash(hc>> 2, adv_pred[2]/7,rs1));
      Map1[i++].set(hash(hc>> 2, rs1, mcupos&63));
      Map1[i++].set(hash(hc>> 2, coef,prev_coef/42));
      Map1[i++].set(hash(hc>> 2,lcp[0]/10, min(5+2*(!comp),zu+zv)));
      Map1[i++].set(hash(hc>> 2,coef,min(5+2*(!comp),zu+zv)));
      Map1[i++].set(hash(hc>> 2,rs1,run_pred[0]));
      Map1[i++].set(hash(hc>> 2,rs1,run_pred[1]));
      Map1[i++].set(hash(hc>> 2,rs1,run_pred[2]));
      Map1[i++].set(hash(hc>> 2,rs1,run_pred[3]));
      Map1[i++].set(hash(hc>> 2, adv_pred1[1]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred1[0]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred2[2]/16,prev_coef/42));
      
      Map1[i++].set(hash(hc>> 2, adv_pred1[2]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred2[2]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred2[1]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred2[0]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred2[1]/7,rs1));
      Map1[i++].set(hash(hc>> 2, adv_pred3[2]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred3[1]/16,prev_coef/42));
      Map1[i++].set(hash(hc>> 2, adv_pred3[0]/16,prev_coef/42));

      // etc
    MJPEGMap.set(hash(mcupos, column, row, hc >> 2));
  }
  
  for (int i=0; i<M; ++i){
      int p=stretch(Map1[i].mix(m1));
      
      m.add(p>>1);m1.add(p>>1);
  } 
  
  MJPEGMap.mix2( m1);

  int sd=((smx.p((hc)&0xffff,y)));
   m1.add(sd=stretch(sd));
   m.add(sd);
   
   m1.set(firstcol, 2,14,250);
   m1.set( coef+256*min(3,huffbits), 1024,13,250*2 );
   m1.set( (hc&0x3FE)*2+min(3,ilog2(zu+zv)), 2048,13,250*2 );
   m1.set(mcupos&63, 64 );
   int colCtx=(width>1024)?(min(1023, column/max(1, width/1024))):column;
   m1.set(colCtx, 1024); 
   m1.set(lma, 256); 
   m1.set(ama, 16); 

  int pr=m1.p(1,0);
   x.Misses+=x.Misses+((pr0>>11)!=y);
   jmiss+=jmiss+((pr0>>11)!=y);
  pr0=pr;
  m.add(stretch(pr)>>1);
  m.add((pr-2048)>>3);
  pr=apm[0].p(pr,hash(hc,abs(adv_pred[1])/16,abs(adv_pred1[1])/16,(x.Misses&0xf)?(x.Misses&0xf):(jmiss&0xf),(x.Misses&0xf)?0:(jmiss&0xf))&0x3FFFF, y,1023);
  m.add(stretch(pr)>>1);
  m.add((pr-2048)>>3);
  pr=apm[1].p(pr, hash(hc&0xffff,coef,x.Misses&0xf,hbcount)&0x3FFFF,y, 1023);

  m.add(stretch(pr)>>1);
  m.add((pr-2048)>>3);
  m.add(stretch((pr+pr0)>>1));
  pr=apm[2].p(pr0, hash(hc&511,abs(lcp[0])/14,abs(lcp[1])/(ccount>1?10:40),hbcount)&0x1FFFF  ,y, 1023);
     m.add(stretch(pr)>>2);
     m.add((pr-2048)>>3);
  pr=apm[3].p(pr, hash(hc&511,abs(lcp[2])/14,abs(lcp[3])/14,run_pred[1]/2,hbcount)&0x1FFFF  ,y, 1023);
     m.add(stretch(pr)>>2);
     m.add((pr-2048)>>3);
  pr=apm[4].p(pr, hash(hc&511,abs(adv_pred[2])/16,abs(adv_pred[3])/16,hbcount)&0x1FFFF    ,y, 1023);
     m.add(stretch(pr)>>2);
     m.add((pr-2048)>>3);
  pr=apm[5].p(pr, hash(hc&511,abs(lcp[4]/14),x.Misses&0xf,hbcount)&0x1FFFF    ,y, 1023);
     m.add(stretch(pr)>>2);
     m.add((pr-2048)>>3);
  pr=apm[6].p(pr, hash(hc&511,abs(lcp[(zu<zv)]/14),abs(lcp[2]/14),run_pred[0]/2,hbcount)&0x1FFFF    ,y, 1023);
     m.add(stretch(pr)>>2);
     m.add((pr-2048)>>3);
  pr=apm[7].p(pr, hash(hc&511,abs(adv_pred[0])/16,x.Misses&0xf,hbcount)&0x1FFFF    ,y, 1023);
     m.add(stretch(pr)>>1);
     m.add((pr-2048)>>3);
  pr=apm[8].p(pr, hash(rs,rs1,abs(lcp[0])/22,abs(lcp[1])/22,hbcount)&0x1FFFF,y, 1023);
     m.add(stretch(pr)>>1);
     m.add((pr-2048)>>3);   

  m.set( 1 + (zu+zv<5)+(huffbits>8)*2+firstcol*4, 9 ,15,250);
  m.set( 1 + (hc&0xFF) + 256*min(3,(zu+zv)/3), 1025,9 ,250);
  m.set( coef+256*min(3,huffbits/2), 1024 ,13);
  m.set((hc)&511, 512);

  m.set( (((abs(adv_pred[1]) / 16) )<<6) |(x.Misses&0x38)|((lma)!=0)*4|(comp == 0)*2 |(min(3,ilog2(zu+zv))>1), 4096);
  int colCtx1=(width>64)?(min(63, column/max(1, width/64))):column;
  m.set(colCtx1, 64,10);

  m.set(( (((abs(lcp[2]) / 16 )&15)<<8)| (((abs(lcp[1]) / 16 )&15)<<4) | (abs(lcp[0]) / 16)&15  ) , 4096,13 );
  m.set(( (((abs(adv_pred[1]) / 16) )<<6) |  (((abs(adv_pred[0]) / 16) )<<2) | min(3,huffbits)),1024,13);
  return 1;
  }

