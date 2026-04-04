#include "predictor.hpp"

// General predicor class
Predictor::Predictor(Settings &set):Predictors(set), pr(16384),pr0(pr),order(0),
ismatch(0), a(x),isCompressed(false),count(0),lastmiss(0),
sse(x) {
    loadModels(activeModels);
    // add extra 
    mixerInputs+=1;
    sse.p(pr);

    // Predictor contexts
    mxp.push_back( {    64,64,0,28,&mcxt[0],0,false} );
    mxp.push_back( {8+1024,64,0,28,&mcxt[1],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[2],0,false} );
    mxp.push_back( {   512,64,0,28,&mcxt[3],0,false} );
    mxp.push_back( {  2048,64,0,28,&mcxt[4],0,false} );
    mxp.push_back( {  2048,64,0,28,&mcxt[5],0,false} );
    mxp.push_back( {   256,64,0,28,&mcxt[6],0,false} );
    mxp.push_back( {  1536,64,0,28,&mcxt[7],0,false} );
    mxp.push_back( {     1, 8,7,14,&mcxt[8],0,false} ); // final mixer
    // create mixer
    m=new Mixers(x,mxp.size(),mixerInputs,mxp);
    mcxt[8]=0;
    einfo.reset();
    palactive=false;
    paloff=0;
}

void Predictor::update()  {
    pr=(32768-pr)/(32768/4096);
    if(pr<1) pr=1;
    if(pr>4095) pr=4095;
    x.Misses+=x.Misses+((pr>>11)!=x.y);
    if (einfo.stat(pr,x.y)) {
        const int el=(14-einfo.rates)*16;
        m->setErrLimit(el,einfo.rates*2);
    }
    // predict pal reordered bytes
    if (palactive==true && x.blpos>=paloff) {
        pr=(((pal[(x.blpos-paloff)&255]>>(7-x.bpos))&1)*4096);
        pr=(4096-pr)*(32768/4096);
        if (pr<1) pr=1;
        if (pr>32767) pr=32767;
        if (x.blpos==(TCOLORS-1+paloff)&&x.bpos==7) palactive=false;
        return;
    }
    if (x.blpos==1 && palactive==false && x.bpos==7) {
        if (x.filetype==HDR && (x.finfo==RLE || x.finfo==MRBR || x.finfo==IMAGE4 || (x.finfo&255)==GIF || x.finfo==IMAGE8 || x.finfo==BM8_OS2 || x.finfo==IMAGE8GRAY)) {
            TCOLORS=256;
            int alpha=1;
            int gifpal=0;
            int paldoff=1; // start of pal data in buffer, 1= if pal was just before image data
            if (x.finfo==MRBR) paloff=1;
            else if ((x.finfo&255)==GIF) alpha=0,TCOLORS=((x.finfo/256)&1023),gifpal=(x.finfo/256)/1024, paloff=2,paldoff=1+(gifpal-TCOLORS*3);
            else if (x.finfo==IMAGE4 || x.finfo==IMAGE8 || x.finfo==IMAGE8GRAY) paloff=2;
            else if (x.finfo==BM8_OS2) paloff=2,alpha=0;
            else if (x.finfo==RLE) paloff=4,alpha=0; // tga
            if (x.finfo==IMAGE4) TCOLORS=16;
            int i=0;
            struct ColorRGBA {
                union {
                    uint32_t c;
                    uint8_t  rgba[4];
                };
                uint8_t   i;
            };
            std::vector<ColorRGBA> bmcolor;
            for (int j=0; j<TCOLORS*(3+alpha); j=j+(3+alpha)) {
                uint32_t c=x.buf(TCOLORS*(3+alpha)+paldoff-j);
                c=c*256+x.buf(TCOLORS*(3+alpha)+paldoff-j-1);
                c=c*256+x.buf(TCOLORS*(3+alpha)+paldoff-j-2);
                c=c*256+alpha*x.buf(TCOLORS*(3+alpha)+paldoff-j-3);
                ColorRGBA colori;
                colori.c=c;
                //printf("%x ",c);
                colori.i=i++;
                bmcolor.push_back(colori);
            }
            for (int j=TCOLORS*3; j<256*3; j=j+3) {
                                    ColorRGBA colori;
                                    uint32_t c=0xffffffff;
                                    colori.c=c;
                                    colori.i=i++;
                                    bmcolor.push_back(colori);
                                }
            // Sort colors by Cartesian distance
            std::sort(bmcolor.begin(), bmcolor.end(), [](const ColorRGBA &a, const ColorRGBA &b) {
                int a1=std::sqrt(a.rgba[3] + (a.rgba[1]*a.rgba[1]) + (a.rgba[2]*a.rgba[2]));
                int b1=std::sqrt(b.rgba[3] + (b.rgba[1]*b.rgba[1]) + (b.rgba[2]*b.rgba[2]));
                return (a1 < b1);
            });
            // Map to new order
            for (int i=0; i<TCOLORS; ++i) {
                for (int j=0; j<TCOLORS; ++j) {
                    ColorRGBA colori=bmcolor[j];
                    if (colori.i==i) {
                        pal[i]=j;
                        break;
                    } 
                }
            }
            palactive=true;
        }
    } // end pal
    if (x.bpos==0) {
        lastmiss=x.Misses&0xFF?x.blpos:lastmiss;
        int b1=x.buf(1);
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1];
        x.f4=x.f4*16+(b1>>4);
        if(x.blpos==1) {
            isCompressed=(x.filetype==CMP || x.filetype==MSZIP)?true:false;
            
        }
    }
    m->update();
    m->add(256);
    int rlen=0/*,Valid=0*/,xmlstate=0;
    ismatch=models[M_MATCH]->p(*m);
    models[M_MATCH1]->p(*m);
    if (x.bpos==0)x.count++;
    models[M_SPARSEMATCH]->p(*m);
    order=models[M_NORMAL]->p(*m);
    if ( isCompressed==false){        
        int dataRecordLen=(x.filetype==DBASE)?x.finfo:(x.filetype==IMGUNK)?x.finfo:0; //force record length 
        rlen=models[M_RECORD]->p(*m,dataRecordLen);
        models[M_WORD]->p(*m,0,x.finfo>0?x.finfo:0); //col
        models[M_SPARSE_Y]->p(*m,ismatch,order);
        models[M_DISTANCE]->p(*m);
        models[M_INDIRECT]->p(*m);
        models[M_NEST]->p(*m);
        models[M_DMC]->p(*m);
        if (x.settings.slow==true) models[M_PPM]->p(*m); 
        if (x.settings.slow==true) models[M_CHART]->p(*m);
        if (x.settings.slow==true) models[M_LSTM]->p(*m);
        xmlstate=models[M_XML]->p(*m);
        models[M_TEXT]->p(*m);
        //Valid=models[M_EXE]->p(*m);
        
        if (!(x.filetype==DBASE/*||x.filetype==BINTEXT*/||x.filetype==HDR ||x.inpdf==false )) models[M_LINEAR]->p(*m);
    } 
    mcxt[0]=(order<<3)|x.bpos;
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    mcxt[1]=8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512;
    mcxt[2]=x.c0;
    uint32_t bt = x.filetype==DEFAULT ? 0 : ( x.filetype==DBASE) ? 2 : /*Valid ? 1 :*/ 3;
    mcxt[3]=order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+128*bt;
    U8 d=x.c0<<(8-x.bpos);
    mcxt[4]=((xmlstate&3)>0)*1024+(x.bpos>0)*512+(order>3)*256+(x.w4&240)+(x.b3>>4);
    mcxt[5]=x.bpos*256+((x.words<<x.bpos&255)>>x.bpos|(d&255));
    mcxt[6]=ismatch;
    if (x.bpos) {
        c=d; if (x.bpos==1)c+=c3/2;
        c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    mcxt[7]=c;
    pr0=m->p(1,1);
    pr=a.p2(pr0,pr,7);
    sse.update();
    pr = sse.p(pr);
}

