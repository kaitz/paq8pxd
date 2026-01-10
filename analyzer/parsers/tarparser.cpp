#include "tarparser.hpp"

TARParser::TARParser():tarF(0) {
    priority=2;
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
        if (rec && state==END && tarFiles==0) state=NONE,priority=2,rec=false; // revusive mode ended
    }
    
    while (inSize<len) {
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (((buf0)==0x61722020 || (buf0&0xffffff00)==0x61720000) && (buf1&0xffffff)==0x757374) ){
            state=START;
            tar=i-263,tarn=0,tarl=1,utar=263;
            tarFiles=0;
            flCount=0;
            tarF.clear();
        } else if (state==START|| state==INFO) {
            if (tarl) {
                const int p=int(i-tar);        
                if (p==512 && tarn==0 && tarl==1) {
                    TARheader &tarh=(TARheader&)data[inSize-512];
                    if (!tarchecksum((char*)&tarh)) {
                        tar=0,tarn=0,tarl=0;  
                    } else {
                        tarl=2;
                        int a=getoct(tarh.size,12);
                        int b=a&511;
                        if (b) tarn=tarn+512*2+(a/512)*512;
                        else if (a==0) tarn=tarn+512;
                        else tarn=tarn+512+a;
                        tarn=tarn+p;
                        state=INFO; 
                        //printf("Tar file: %s %d\n",tarh.name,a);
                        tarsi=0;
                        // Set file size info only if there is any
                        if (a) {
                            tarFiles++;
                            TARfile tf;
                            tf.start=i-(tarFiles==0?0:tar);
                            tf.size=a;//+(512-(a%512)); // to include pad
                            std::string fname=tarh.name;
                            ParserType etype=GetTypeFromExt(fname);
                            tf.p=etype;
                            tarF.push_back(tf);
                        }
                    }
                }else if (tarn && tarl==2 && (tarn+tar)==i) {
                    TARheader &tarh=(TARheader&)tars[0];
                    if (!tarchecksum((char*)&tarh)) {
                        state=NONE,tarl=jend=0;  
                    } 
                    if (tarend((char*)&tarh)==true) {
                        /*jstart=tar;
                        jend=i+512;
                        type=TAR;
                        state=END;
                        //printf("Tar files: %d\n",tarFiles);
                        return state;*/
                        // recursive mode
                        jstart=0;
                        jend=0;
                        relAdd=tar;
                        state=END;
                        rec=true;           // fall to recursive mode
                    } else if (state!=NONE) {
                        int a=getoct(tarh.size,12);
                        int b=a&511;
                        if (b) {
                            tarn=tarn+512*2+(a/512)*512;
                        }
                        else if (a==0) tarn=tarn+512;
                        else {
                            tarn=tarn+a+512;
                        }
                        state=INFO;
                        // Set file size info only if there is any
                        if (a) {
                            //printf("Tar file: %s %d\n",tarh.name,a);
                            TARfile tf;
                            tf.start=i-(tarFiles==0?0:tar);
                            tf.size=a;//+(512-(a%512)); // to include pad
                            std::string fname=tarh.name;
                            ParserType etype=GetTypeFromExt(fname);
                            tf.p=etype;
                            tarF.push_back(tf);
                            tarFiles++;
                        }
                        if (flCount++>1) priority=0; // after 2 files set priority to 0
                    }
                }
                tars[tarsi++]=c;
                tarsi&=511;
            }
        } else if (rec && state==END && tarFiles) { 
            // recursive mode, report all tar file ranges
            // file extension based type parser set in info
            TARfile tarfile=tarF[tarF.size()-tarFiles];
            jstart=tarfile.start-(relAdd-tar);
            jend=jstart+tarfile.size;
            //printf("File %d %d %d\n",tarF.size()-tarFiles,jstart,jend);
            relAdd+=jend-tar;
            relAdd-=tar;
            type=RECE;
            info=tarfile.p;
            tar=0;
            tarFiles--;
            if (tarFiles==0) tarF.clear(),rec=false;
            return state;        
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
    t.start=jstart;   // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;         // pos where start was set in block
    t.type=type;
    t.recursive=rec;
    return t;
}

int TARParser::TypeCount() {
    return 1;
}

void TARParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=0;
    tar=0,tarn=0,tarl=0,utar=0;tarsi=0;
    info=i=inSize=relAdd=0;
    memset(&tars[0], 0, 256);
    priority=2;
    flCount=tarFiles=0;
    rec=false;
    tarF.clear();
}
void TARParser::SetEnd(uint64_t e) {
    jend=e;
}
