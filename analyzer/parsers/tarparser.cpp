#include "tarparser.hpp"

TARParser::TARParser() {
    priority=1;
    Reset();
    inpos=0;
    name="tar";
}

TARParser::~TARParser() {
}

int TARParser::getoct(const char *p, int n){
    int i = 0;
    while (*p<'0' || *p>'7') ++p, --n;
    while (*p>='0' && *p<='7' && n>0) {
        i*=8;
        i+=*p-'0';
        ++p,--n;
    }
    return i;
}
int TARParser::tarchecksum(char *p){
    int u=0;
    for (int n = 0; n < 512; ++n) {
        if (n<148 || n>155) u+=((uint8_t *)p)[n];
        else u += 0x20;
    }
    return (u==getoct(p+148,8));
}
bool TARParser::tarend(const char *p){
    for (int n=511; n>=0; --n) if (p[n] != '\0') return false;
    return true;
}

// loop over input block byte by byte and report state
DetectState TARParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<1024) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (((buf0)==0x61722020 || (buf0&0xffffff00)==0x61720000) && (buf1&0xffffff)==0x757374) ){
            state=START;
            tar=i-263,tarn=0,tarl=1,utar=263;
        } /*else if (state==NONE && i==512) {
            TARheader &tarh=(TARheader&)data[0];
            if (tarchecksum((char*)&tarh)){
                tar=i,tarn=512,tarl=2,utar=0;
                int a=getoct(tarh.size,12);
                int b=a-(a/512)*512;
                if (b) tarn=tarn+512*2+(a/512)*512;
                else if (a==0) tarn=tarn+512;
                else tarn=tarn+512+(a/512)*512;
                tarn=tarn+int(i-tar+utar);
                state=INFO;
            }
        }*/ else if (state==START|| state==INFO) {
            if (tarl) {
                const int p=int(i-tar);        
                if (p==512 && tarn==0 && tarl==1) {
                    TARheader &tarh=(TARheader&)data[inSize-512];
                    if (!tarchecksum((char*)&tarh)) tar=0,tarn=0,tarl=0;
                    else{
                        tarl=2;
                        int a=getoct(tarh.size,12);
                        int b=a&511;
                        if (b) tarn=tarn+512*2+(a/512)*512;
                        else if (a==0) tarn=tarn+512;
                        else tarn=tarn+512+a;
                        tarn=tarn+p;
                        state=INFO;
                        tarsi=0;
                    }
                }else if (tarn && tarl==2 && (tarn+tar)==i) {
                    TARheader &tarh=(TARheader&)tars[0];
                    if (!tarchecksum((char*)&tarh))  state=NONE,tarl=0;
                    if (tarend((char*)&tarh)==true) {
                        jstart=tar;
                        jend=i+512;
                        type=TAR;
                        state=END;
                        return state;
                    } else{
                        int a=getoct(tarh.size,12);
                        int b=a&511;
                        //int pad=512-(a%512);
                        if (b) {
                            tarn=tarn+512*2+(a/512)*512;
                        }
                        else if (a==0) tarn=tarn+512;
                        else {
                            tarn=tarn+a+512;
                        }
                        state=INFO;
                    }
                }
                tars[tarsi++]=c;
                tarsi&=511;
            }
        }

        inSize++;
        i++;
    }
    if (state==INFO) {jend=i+1; return INFO;}
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType TARParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int TARParser::TypeCount() {
    return 1;
}

void TARParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    tar=0,tarn=0,tarl=0,utar=0;tarsi=0;
    info=i=inSize=0;
    memset(&tars[0], 0, 256);
}
void TARParser::SetEnd(uint64_t e) {
    jend=e;
}
