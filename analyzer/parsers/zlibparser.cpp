#include "zlibparser.hpp"
    
zlibParser::zlibParser(bool b) {
    priority=b==true?3:2;
    Reset();
    inpos=0;
    name="zlib";
    brute=priority==2?false:true;
    strm=(z_stream*)calloc( 1,sizeof(z_stream));
    pdfim=0,pdfimw=0,pdfimh=0,pdfimb=0,pdfgray=0;
}

zlibParser::~zlibParser() {
    free(strm);
}

int zlibParser::parse_zlib_header(int header) {
    switch (header) {
    case 0x2815 : return 0;  case 0x2853 : return 1;  case 0x2891 : return 2;  case 0x28cf : return 3;
    case 0x3811 : return 4;  case 0x384f : return 5;  case 0x388d : return 6;  case 0x38cb : return 7;
    case 0x480d : return 8;  case 0x484b : return 9;  case 0x4889 : return 10; case 0x48c7 : return 11;
    case 0x5809 : return 12; case 0x5847 : return 13; case 0x5885 : return 14; case 0x58c3 : return 15;
    case 0x6805 : return 16; case 0x6843 : return 17; case 0x6881 : return 18; case 0x68de : return 19;
    case 0x7801 : return 20; case 0x785e : return 21; case 0x789c : return 22; case 0x78da : return 23;
    }
    return -1;
}

int zlibParser::zlib_inflateInit(z_streamp strm, int zh) {
    if (zh==-1) return inflateInit2(strm, -MAX_WBITS); else return inflateInit(strm);
}

void zlibParser::SetPdfImageInfo() {
    if (pdfimw>0 && pdfimw<0x1000000 && pdfimh>0) {
        if (pdfimb==8 && (int)strm->total_out==pdfimw*pdfimh) pinfo=" 8b image ",info=((pdfgray?IMAGE8GRAY:IMAGE8)<<24)|pdfimw;
        if (pdfimb==8 && (int)strm->total_out==pdfimw*pdfimh*3) pinfo=" 24b image ",info=(IMAGE24<<24)|pdfimw*3;
        if (pdfimb==8 && (int)strm->total_out==pdfimw*pdfimh*4) pinfo=" 32b image ",info=(IMAGE32<<24)|pdfimw*4;
        if (pdfimb==4 && (int)strm->total_out==((pdfimw+1)/2)*pdfimh) pinfo=" 4b image ",info=(IMAGE4<<24)|((pdfimw+1)/2);
        if (pdfimb==1 && (int)strm->total_out==((pdfimw+7)/8)*pdfimh) pinfo=" 1b image ",info=(IMAGE1<<24)|((pdfimw+7)/8);
        if (info) pinfo+=" (width: "+ itos(info&0xffffff) +")";
        else pinfo="";
        pdfgray=0;
    }
}
// loop over input block byte by byte and report state
DetectState zlibParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<16) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        int c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE){
            // ZLIB stream detection
            histogram[c]++;
            if (i>=256)
            histogram[zbuf[zbufpos]]--;
            zbuf[zbufpos] = c;
            if (zbufpos<32)
            zbuf[zbufpos+256] = c;
            zbufpos=(zbufpos+1)&0xFF;
            int zh=parse_zlib_header(((int)zbuf[(zbufpos-32)&0xFF])*256+(int)zbuf[(zbufpos-32+1)&0xFF]);
            bool valid = (i>=31 && zh!=-1);
            if (!valid && brute && i>=255){
                uint8_t BTYPE = (zbuf[zbufpos]&7)>>1;
                if ((valid=(BTYPE==1 || BTYPE==2))){
                    int maximum=0, used=0, offset=zbufpos;
                    for (int i=0;i<4;i++,offset+=64){
                        for (int j=0;j<64;j++){
                            int freq = histogram[zbuf[(offset+j)&0xFF]];
                            used+=(freq>0);
                            maximum+=(freq>maximum);
                        }
                        if (maximum>=((12+i)<<i) || used*(6-i)<(i+1)*64){
                            valid = false;
                            break;
                        }
                    }
                }
            }
            if (zh==-1 && zbuf[(zbufpos-32)&0xFF]=='P' && zbuf[(zbufpos-32+1)&0xFF]=='K' && zbuf[(zbufpos-32+2)&0xFF]=='\x3'
                    && zbuf[(zbufpos-32+3)&0xFF]=='\x4' && zbuf[(zbufpos-32+8)&0xFF]=='\x8' && zbuf[(zbufpos-32+9)&0xFF]=='\0') {
                int nlen=(int)zbuf[(zbufpos-32+26)&0xFF]+((int)zbuf[(zbufpos-32+27)&0xFF])*256
                +(int)zbuf[(zbufpos-32+28)&0xFF]+((int)zbuf[(zbufpos-32+29)&0xFF])*256;
                if (nlen<256 && i+30+nlen<len/*n*/) zzippos=i+30+nlen;
            }
            if (i-pdfimp>1024) pdfim=pdfimw=pdfimh=pdfimb=pdfgray=0;
            if (pdfim>1 && !(isspace(c) || isdigit(c))) pdfim=1;
            if (pdfim==2 && isdigit(c)) pdfimw=pdfimw*10+(c-'0');
            if (pdfim==3 && isdigit(c)) pdfimh=pdfimh*10+(c-'0');
            if (pdfim==4 && isdigit(c)) pdfimb=pdfimb*10+(c-'0');
            if ((buf0&0xffff)==0x3c3c) pdfimp=i,pdfim=1; // <<
            if (pdfim && (buf1&0xffff)==0x2f57 && buf0==0x69647468) pdfim=2,pdfimw=0; // /Width
            if (pdfim && (buf1&0xffffff)==0x2f4865 && buf0==0x69676874) pdfim=3,pdfimh=0; // /Height
            if (pdfim && buf3==0x42697473 && buf2==0x50657243 && buf1==0x6f6d706f
                    && buf0==0x6e656e74 && zbuf[(zbufpos-32+15)&0xFF]=='/') pdfim=4,pdfimb=0; // /BitsPerComponent
            if (pdfim && (buf2&0xFFFFFF)==0x2F4465 && buf1==0x76696365 && buf0==0x47726179) pdfgray=1; // /DeviceGray
            if (valid || zzippos==i || state==START) {
                // look for MS ZIP header, if found disable zlib
                int j=i-(brute?255:31);
                if (j<len && data[j-4]==0x00 && data[j-3]==0x80 && data[j-2]==0x43 && data[j-1]==0x4b/* && ((data[j]>>1)&3)!=3 */) {
                    valid=false,state=DISABLE;
                }
                if (state!=DISABLE){
                    
                    int streamLength=0, ret=0, brute=(zh==-1 && zzippos!=i);
                    // Quick check possible stream by decompressing first 32 bytes
                    strm->zalloc=Z_NULL; strm->zfree=Z_NULL; strm->opaque=Z_NULL;
                    strm->next_in=Z_NULL; strm->avail_in=0;
                    if (zlib_inflateInit(strm,zh)==Z_OK) {
                        strm->next_in=&zbuf[(zbufpos-(brute?0:32))&0xFF]; strm->avail_in=32;
                        strm->next_out=zout; strm->avail_out=1<<16;
                        ret=inflate(strm, Z_FINISH);
                        ret=(inflateEnd(strm)==Z_OK && (ret==Z_STREAM_END || ret==Z_BUF_ERROR) && strm->total_in>=16);
                    }
                    if (ret) {
                        // Verify valid stream and determine stream length
                        strm->zalloc=Z_NULL; strm->zfree=Z_NULL; strm->opaque=Z_NULL;
                        strm->next_in=Z_NULL; strm->avail_in=0; strm->total_in=strm->total_out=0;
                        if (zlib_inflateInit(strm,zh)==Z_OK) {
                            for (uint64_t j=(i%0x10000)-(brute?255:31); j<len; j+=1<<16) {
                                unsigned int blsize=min(len-j,1<<16);
                                memcpy(&zin[0], &data[j], blsize);
                                strm->next_in=zin; strm->avail_in=blsize;
                                do {
                                    strm->next_out=zout; strm->avail_out=1<<16;
                                    ret=inflate(strm, Z_FINISH);
                                } while (strm->avail_out==0 && ret==Z_BUF_ERROR);
                                if (ret==Z_STREAM_END) streamLength=strm->total_in;
                                if (ret==Z_BUF_ERROR) {
                                    // Our input block ended so report info
                                    if (jstart=(i%0x10000)-(brute?255:31)+blsize==len){
                                        jstart=i-(brute?255:31);
                                        state=INFO;
                                        return state;
                                    }
                                    state=NONE;
                                }
                            }
                            if (inflateEnd(strm)!=Z_OK) streamLength=0;
                        }
                        /*if (streamLength>(brute<<7)&&i!=0) {
                        type=DEFAULT;
                        jend=i;
                        state=INFO;
                    }*/
                    }
                    if (streamLength>(brute<<7)) {
                        info=0;
                        SetPdfImageInfo();
                        jstart=i-(brute?255:31);
                        jend=jstart+streamLength;
                        state=END;
                        type=ZLIB;
                        return state;
                    }
                    state=NONE;
                }
                
            }
        } else if (state==INFO) {
            // continue larger zlib block
            int streamLength=0, ret=0;
            strm->next_in=data; strm->avail_in=len;
            do {
                strm->next_out=zout; strm->avail_out=1<<16;
                ret=inflate(strm, Z_FINISH);
            } while (strm->avail_out==0 && ret==Z_BUF_ERROR);
            if (ret==Z_STREAM_END) streamLength=strm->total_in;
            if (ret==Z_BUF_ERROR) {
                return DATA;
            }
            if (inflateEnd(strm)!=Z_OK) streamLength=0;
            info=0;
            SetPdfImageInfo();
            if (streamLength>(brute<<7)) {
                jend=jstart+streamLength;
                state=END;
                type=ZLIB;
                return state;
            }
        }
        inSize++;
        i++;
    }
    
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType zlibParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int zlibParser::TypeCount() {
    return 1;
}

void zlibParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=0;
    //pdfim=0,pdfimw=0,pdfimh=0,pdfimb=0,pdfgray=0;
    //memset( &strm,0,sizeof(z_stream));
    memset( &zbuf[0],0,256+32);
    memset( &zin[0],0,1<<16);
    memset( &zout[0],0,1<<16);
    zbufpos=0, histogram[256]={};
    pdfimp=0;zzippos=-1;
    info=i=inSize=0;
    priority=brute==true?3:2;
}
void zlibParser::SetEnd(uint64_t e) {
    jend=e;
}
