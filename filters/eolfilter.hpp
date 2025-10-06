#pragma once
#include "filter.hpp"

//EOL
//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski

class EOLFilter: public Filter {
public:
    EOLFilter(std::string n, Filetype f=DEFAULT);
    ~EOLFilter();
    void encode(File *in, File *out, uint64_t size, uint64_t info);
    uint64_t decode(File *in, File *out, uint64_t size, uint64_t info);
};

enum EEOLType {UNDEFINED, CRLF, LF};

#define MAX_FREQ_ORDER1 255
#define ORDER1_STEP    4

class RangeCoder{
    U32 code, range, FFNum, Cache;
    U64 low;
    int mZero[MAX_FREQ_ORDER1];
    int mOne[MAX_FREQ_ORDER1];
    File*outeol;
public:
    inline void ShiftLow() {
        if ((low^0xFF000000)>0xFFFFFF) {
            outeol->putc( Cache + (low>>32));
            int c = 0xFF+(low>>32);
            while( FFNum ) outeol->putc(c), FFNum--;
            Cache = U32(low)>>24;
        } else FFNum++;
        low = U32(low)<<8;
    }

    void StartEncode(File*out ) {
        low=FFNum=Cache=0;
        range=0xffffffff;
        outeol=out;
    }

    void StartDecode(File*out) {
        outeol=out;
        code=0;
        range=0xffffffff;
        for (int i=0; i<5; i++) code=(code<<8) | outeol->getc();
    }

    void FinishEncode() {
        for (int i=0; i<5; i++) ShiftLow();
    }

    void Encode(U32 cumFreq, U32 freq, U32 totFreq) {
        low+=cumFreq*(range/= totFreq);
        range*=freq;
        while (range<(1<<24)) { ShiftLow(); range<<=8; }
    }

    inline U32 GetFreq (U32 totFreq) {
        return code / (range/= totFreq);
    }
    void Decode (U32 cumFreq, U32 freq, U32 totFreq) {
        code-=cumFreq*range;
        range*=freq;
        while (range<(1<<24)) code=(code<<8)|outeol->getc(), range<<=8;
    }

    inline void UpdateOrder1(int prev,int c, int step) {
        if (c==0) mZero[prev]+=step;
        else      mOne[prev]+=step;

        if (mZero[prev]+mOne[prev]>=1<<15) {
            mZero[prev]=(mZero[prev]+1)/2;
            mOne[prev]=(mOne[prev]+1)/2;
        }
    }

    inline void EncodeOrder1(int prev, int c) {
        if (c==0)  Encode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else       Encode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
    }

    inline int DecodeOrder1(int prev) {
        int c=GetFreq(mZero[prev]+mOne[prev]);

        if (c<mZero[prev]) c=0;
        else c=1;

        if (c==0) Decode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else      Decode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
        return c;
    }

    U32 DecodeOrder(U32 prev) {
        U32 result=DecodeOrder1(prev);
        UpdateOrder1(prev,result,ORDER1_STEP);
        return result;
    }
    void EncodeOrder(U32 prev, U32 result) {
        EncodeOrder1(prev,result);
        UpdateOrder1(prev,result,ORDER1_STEP);
    }

    RangeCoder() {
        for (int i=0; i<MAX_FREQ_ORDER1; i++) {
            mZero[i]=1;
            mOne[i]=1;
        }
    }
};


#define TOLOWER(c)    ((c>='A' && c<='Z')?(c+32):c)
//#define TOUPPER(c)    ((c>='a' && c<='z')?(c-32):c)
class EOLEncoderCoder {
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int lastEOL,lastChar;
public:
    EOLEncoderCoder (File*out ) {
        coder.StartEncode(out);
    }
    inline int ContextEncode(int leftChar,int c,int rightChar,int distance) {
        U32 prev,result;

        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z')
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\''  /*&& leftChar!='>'*/) || rightChar<'a' || rightChar>'z')
        return c;

        if (c==32)
        result=0;
        else
        result=1;

        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        coder.EncodeOrder(prev,result);
        return 32;
    }
    void EncodeEOLformat(EEOLType EOLType) {
        if(EOLType==CRLF) coder.Encode(0,1,2);
        else coder.Encode(1,1,2);
    }

    void EOLencode(File* file,File* fileout,int fileLen) {
        int xc=0;
        int last_c,c,next_c;
        last_c=0;
        lastEOL=0;
        EOLType=UNDEFINED;
        lastEOL=0;
        c=file->getc(),fpos++;
        fpos=0;
        while (fpos<fileLen) {
            next_c=file->getc(),fpos++;
            if (c==32 || c==10 || (c==13 && next_c==10)) {
                if (c==13) {
                    if (EOLType==CRLF || EOLType==UNDEFINED) {
                        c=next_c;
                        if (fpos<fileLen) {
                            next_c=file->getc(),fpos++;
                        } else {
                            next_c=0,fpos++;
                        }
                        lastEOL++;
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c) {
                            EOLType=CRLF;
                            EncodeEOLformat(EOLType);
                        }
                        lastEOL=fpos;
                        if (last_c==10)  xc=5;//LF marker
                        else xc=last_c;
                    }
                    else
                        xc=c;
                }
                else{
                    if (c==10 && EOLType==CRLF) {
                        xc=c;
                    } else {
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c) {
                            EOLType=LF;
                            EncodeEOLformat(EOLType);
                        }
                        xc=last_c;
                    }
                    if (c==10) lastEOL=fpos;
                }
            } else {
                xc=c;
            }
            last_c=c;
            c=xc;
            fileout->putc(c);
            c=next_c;
        }
        coder.FinishEncode();
    }
};

class EOLDecoderCoder{
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int bufChar,lastEOL,lastChar;
public:

    inline int ContextDecode(int leftChar,int rightChar,int distance) {
        U32 prev,result;
        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z'  )
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\''/* && leftChar!='>'*/) || rightChar<'a' || rightChar>'z')
        return 32;

        if(leftChar>96||leftChar==',') leftChar=122;
        if(leftChar<96) leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        result=coder.DecodeOrder(prev);
        if (result==0) return 32;
        else return 10;
    }

    EEOLType DecodeEOLformat() {
        int c=coder.GetFreq(2);
        if (c<1) {
            coder.Decode(0,1,2);
            return CRLF;
        } else {
            coder.Decode(1,1,2);
            return LF;
        }
    }

    void hook_putc(int c,File* out,int maxlen) {
        if (bufChar<0 && c==' ') {
            bufChar=c;
            return;
        }
        if (bufChar>=0) {
            bufChar=ContextDecode(TOLOWER(lastChar),TOLOWER(c),fpos-lastEOL);
            if (bufChar==10) {
                if (EOLType==UNDEFINED)
                EOLType=DecodeEOLformat();
                if (EOLType==CRLF) {
                    lastChar=13;
                    if (fpos==maxlen) return;
                    out->putc(lastChar),fpos++;
                }
                lastEOL=fpos;
            }
            if (fpos==maxlen) return;
            out->putc(bufChar),fpos++;
            if (c==' ') {
                lastChar=bufChar;
                bufChar=c;
                return;
            }
            bufChar=-1;
        }
        if (c==10)
           lastEOL=fpos;
        lastChar=c;
        if (c==EOF) return;
        if (fpos==maxlen) return;
        out->putc(c),fpos++;
    }

    void EOLdecode(File* in,File* out,int size,File*outeol,File*wd,int len) {
        int c=0;
        bufChar=-1;
        lastEOL=-1;
        EOLType=UNDEFINED;
        fpos=0;
        coder.StartDecode(outeol);

        for ( int i=0; i<size; i++) {
            c=wd->getc();
            if (c==5) {
                hook_putc(13,out,len);
                hook_putc(10,out,len);
            } else {
                hook_putc(c,out,len);
            }
        }
    }
};
