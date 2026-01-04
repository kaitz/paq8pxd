#include "predictortext.hpp"
// Text predicor
extern bool slow;


PredictorTXTWRT::PredictorTXTWRT(): pr(16384),pr0(pr),order(0),rlen(0),ismatch(0),
Text{ {0x10000, 0x10000, 0x10000, 0x10000}, {{0x10000,x}, {0x10000,x}, {0x10000,x}} },
count(0),blenght(1024*4),StateMaps{  ( 0x7FFFFF+1)<<2},sse(x),
decodedTextLen(0),lasttag(0),counttags(0),lState(0){
   loadModels(activeModels,15);  
   // add extra 
   mixerInputs+=1+1;
   mixerNets+= 4096+ 64+ 256+ 256*8+ 256*8+ 256*8+ 1536+ 2048;
   mixerNetsCount+=8;
   sse.p(pr); // must
   // create mixer
   m=new Mixer(mixerInputs,  mixerNets,x, mixerNetsCount,0,3,2);
}
void PredictorTXTWRT::wrt(){
        U8 c1=x.c4;
        if (x.wrtLoaded==true) {
           if (c1=='<' && x.ishtml==true ) x.istex=false; // open
        }
    // load wrt dictionary from file
        // detect header//'!Pq8'
        if (x.c4==0x21507138 && x.blpos<16){ 
          x.wrtpos=x.blpos;
        }
        if (x.wrtpos>0)  { 
          if (x.wrtpos+4+4==x.blpos){ 
            x.wrtfile=(U64(bswap(x.c4))<<32)| bswap(x.c8);
          }
          //load size
          if (x.wrtpos+4+4+3==x.blpos) { 
          x.wrtsize=(x.c4&0xffffff);
          if (x.wrtsize<4) x.wrtpos=0;
          }
          //load count
          if (x.wrtpos+4+4+3+3==x.blpos) { 
            x.wrtcount=(x.c4&0xffffff);
            x.wrtcount=(x.wrtcount>>16)+((x.wrtcount&0xff)<<16)+(x.wrtcount&0xff00);
            printf("WRT dict count %d words.\n",x.wrtcount);
            x.wwords=x.wrtcount;
            if (x.wrtcount==0) x.wrtpos=x.wrtsize=0;
          }
          if (x.wrtsize>0 && x.wrtpos+4+4+3+3+x.wrtsize+3==x.blpos && x.wrtdata==0) { 
            if (x.buf(6)==5) {
              x.wrtdata=x.blpos+5;
            }
            else x.wrtLoaded==false,x.wrtpos=x.wrtdata=x.wrtsize=0;
          }
          if (x.wrtdata>0 && x.blpos>=x.wrtdata &&x.wrtLoaded==false) { 
            wr.WRT_start_decoding(x.wrtcount,x.wrtsize,&x.buf[x.buf.pos-5-3-x.wrtsize]);
            x.wrtLoaded=x.dictonline=true;
            printf("WRT dict online.\n");
          }
        }
        if (x.wrtLoaded==true) {
           // Returned wrtstatus values
           // codeword size   1-3
           // D_REGULAR_CHAR    0
           // D_WORD_DECODED   -1     
           // D_FIRSTUPPER     -2
           // D_UPPERWORD      -3
           // D_ESCAPE         -4
           // D_UTFUPPER       -5
          x.wrtstatus=wr.WRT_decode(x.buf(1),&x.wrtText[0],&x.wrtTextSize);
          x.wstat=  (x.wrtstatus==-1)||  (x.wrtstatus>0);
          x.wdecoded=false;
          if (x.wrtstatus==-1){
            x.wrtbytesize++;
            x.wdecoded=true;
            // print decoded word
            // for (int p=0;p<x.wrtTextSize;p++)printf("%c",x.wrtText[p]);
            models[M_TEXT]->setword(&x.wrtText[0],x.wrtTextSize);
            x.wlen=x.wrtbytesize;
             int y=0;
            if (x.wrtTextSize<5 && x.wrtText[0]>'z') y=utf8_check(&x.wrtText[0]); 
            if (y==x.wrtbytesize){
                x.utf8l=y;
                x.wrtc4=(x.wrtc4<<8)|x.wrtText[0];
                x.bufn[x.bufn.pos++]=x.wrtc4&0xff;
                x.bufn.pos=x.bufn.pos&x.bufn.poswr; //wrap
            }else {
            x.utf8l=0;
            for (int p=0;p<x.wrtTextSize;p++){
                U8 wc=x.wrtText[p];
                x.wrtc4=(x.wrtc4<<8)|wc;
                x.bufn[x.bufn.pos++]=wc;
                x.bufn.pos=x.bufn.pos&x.bufn.poswr; //wrap
            }
            }
            decodedTextLen=x.wrtTextSize;
            x.wrtbytesize=0;
            x.wrtTextSize=0;
          }else if (x.wrtstatus>0){
            x.wrtbytesize++;
          }
          // if line starts with <someword>
          if ((x.buf(decodedTextLen)=='<' || x.bufn(decodedTextLen+1)=='<' /*|| (x.bufn(decodedTextLen+1)=='<' && x.bufn(decodedTextLen+2)=='&')*/) && c1=='>'){
              counttags++;lasttag=x.blpos;
          }
          if ( c1 >'@' && lState==0)  lState=1;
          if ((c1=='<' && lState==0)) lState=2;
          if ((c1==10 || c1==5) && counttags && lState==2) {
              x.ishtml=true;
              counttags=0;
          } else if (c1==10 || c1==5){ 
               counttags=lState=0;
               if ((x.blpos-lasttag)>256*4 && x.ishtml==true) x.ishtml=false;// standard break 1k
          }
        }

        if (x.wrtstatus==0 && x.wrtLoaded==true){
          if (c1==5) c1=10;
          x.wrtc4=(x.wrtc4<<8)|c1;// printf("%c",c1);
          x.bufn[x.bufn.pos++]=c1;
          x.bufn.pos=x.bufn.pos&x.bufn.poswr; //wrap
        }
        if (c1=='>') x.istex=true;  //close
        //if (x.c4==0x10103d3d) x.ishtml=false,x.istex=true;  // w - fast break
}
void PredictorTXTWRT::update() {
    if (x.bpos==0) {
        int b1=x.buf(1);
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b4=x.b3;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12;//, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        
        x.tt=x.tt*8+WRT_mtt[b1];
        if (b1==32) --b1;
        x.f4=x.f4*16+(b1>>4);
        wrt();
        int d=WRT_wrd1[b1]; 
        x.bc4=x.bc4<<2|(d/64); 
    }
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>11)!=x.y);
    m->update();
    m->add(256);
    ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.bpos==0)x.count++;
    models[M_SPARSEMATCH]->p(*m);
    order=models[M_NORMAL]->p(*m);

        int state=models[M_XML]->p(*m);
        m->set(models[M_WORD]->p(*m,state&7)&0xfff, 4096);    
        models[M_SPARSE]->p(*m,ismatch,order);
        models[M_NEST]->p(*m);
        models[M_INDIRECT]->p(*m);
        models[M_DMC]->p(*m);
        rlen=models[M_RECORD]->p(*m);
        models[M_TEXT]->p(*m,(state&7));
        if (slow==true) models[M_PPM]->p(*m);
        if (slow==true) models[M_CHART]->p(*m);
        if (slow==true) models[M_LSTM]->p(*m);
        int dd=((pr>>9)>0?1:0)+((pr>>9)>14?1:0);
        dd=(dd<<7)|(x.bc4&127);
        dd=(dd<<8)|(x.buf(1));
        dd=(dd<<8)|x.c0;
         m->add((stretch(StateMaps[0].p( dd,x.y))+1)>>1);
        U32 c3=x.buf(3), c;
        c=(x.words>>1)&63;
        m->set(x.c0, 256);
        m->set(order << 3U | x.bpos, 64);
        m->set((x.bc4&3)*64+c+order*256, 256*8);
        m->set(256*order + (x.w4&240) + (x.b3>>4), 256*8);
        m->set((x.w4&255)+256*x.bpos, 256*8);
        if (x.bpos){
            c=x.c0<<(8-x.bpos); if (x.bpos==1)c+=x.b4/2;
            c=(min(x.bpos,5))*256+(x.tt&63)+(c&192);
        }
        else c=(x.words&12)*16+(x.tt&63);
        m->set(c, 1536);
        c=x.bpos*256+((x.c0<<(8-x.bpos))&255);
        c3 = (x.words<<x.bpos) & 255;
        m->set(c+(c3>>x.bpos), 2048);
        pr0=m->p(1,1);
        int limit=0x3FF>>((x.blpos<0xFFF)*2);
    int pr1, pr2, pr3;

    pr  = Text.APMs[0].p(pr0, (x.c0<<8)|(x.Text.mask&0xF)|((x.Misses&0xF)<<4), x.y, limit);
    pr1 = Text.APMs[1].p(pr0, x.c0^hash(x.bpos, x.Misses&3, x.buf(1), x.x5&0x80ff, x.Text.mask>>4)&0xFFFF, x.y, limit);
    pr2 = Text.APMs[2].p(pr0, x.c0^hash( x.Match.byte, min(3, ilog2(x.Match.length+1)))&0xFFFF, x.y, limit);
    pr3 = Text.APMs[3].p(pr0, x.c0^hash( x.c4&0xffff,   x.Text.firstLetter,x.wrtbytesize)&0xFFFF, x.y, limit);
    pr0 = (pr0+pr1+pr2+pr3+2)>>2;

    pr1 = Text.APM1s[0].p(pr0, x.c0^hash(x.Match.byte, min(3, ilog2(x.Match.length+1)), x.buf(1))&0xFFFF);
    pr2 = Text.APM1s[1].p(pr, x.c0^hash( x.c4&0x00ffffff)&0xFFFF, 6);
    pr3 = Text.APM1s[2].p(pr, x.c0^hash( x.c4&0xffffff00)&0xFFFF, 6);
    pr = (pr+pr1+pr2+pr3+2)>>2;
    pr = (pr+pr0)>>1;
    sse.update();
    pr = sse.p(pr);
}
