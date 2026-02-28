class PredictorFast: public Predictors {
    int cxt[4];
    int cxt1,cxt2,cxt3,cxt4;
    int pr0;
    StateMap *sm;
    Array<U8> t; 
    APM1 a;
    const int MAXLEN; // longest allowed match + 1
    Array<int> tm;    // hash table of pointers to contexts
    int h;            // hash of last 7 bytes
    U32 ptr;          // points to next byte of match if any
    int len;          // length of match, or 0 if no match
    int mm1,mm3;
    int match;
public:
    PredictorFast(Settings &set):Predictors(set), cxt1(0),cxt2(0),cxt3(0),cxt4(0),pr0(2048),t(0x80000),a(256,x),MAXLEN(65534), 
    tm(CMlimit(x.MEM()/4)),h(0), ptr(0),len(0),mm1(2048),mm3(2048),match(0){
        sm=new StateMap[4];
        cxt[0]=cxt[1]=cxt[2]=cxt[3]=0;     
    }

    int p() const { return pr0; }

    void update() {
        if (match!=x.y) len=0;  //bit mismatch
        if (x.bpos==0){
            cxt4=cxt3;
            cxt3=cxt2;
            cxt2=cxt1*256;
            cxt1=x.buf(1);
            //update match
            h=(h*997*8+cxt1+1)&(tm.size()-1);  // update context hash
            if (len) len+=len<MAXLEN, ++ptr;
            else {  // find match
                ptr=tm[h];
                if (ptr && x.buf.pos-ptr<x.buf.size())
                while (x.buf(len+1)==x.buf[ptr-len-1] && len<MAXLEN) ++len;
            }
            tm[h]=x.buf.pos;  // update hash table
            //result=;
            //    if (result>0 && !(result&0xfff)) printf("pos=%d len=%d ptr=%d\n", pos, len, ptr);
            int ilen=ilog(len)<<2;
            int mlen=min(len, 32)<<6;
            mm1=(squash(ilen)+squash(mlen))>>1;
            mm3=(squash(-ilen)+squash(-mlen))>>1;
        }
        for (int i=0; i<4; ++i){
            t[cxt[i]]=nex(t[cxt[i]],x.y);
        }
        cxt[0]=cxt1*256+x.c0;
        cxt[1]=cxt2+x.c0+0x10000;
        cxt[2]=cxt3+x.c0+0x20000;
        cxt[3]=cxt4+x.c0+0x40000;

        pr0=(sm[0].p(t[cxt[0]],x.y)+sm[1].p(t[cxt[1]],x.y)+sm[2].p(t[cxt[2]],x.y)+sm[3].p(t[cxt[3]],x.y))>>2;
        // predict match
        if (len){
            match=(x.buf[ptr]>>(7-x.bpos))&1;
            if (match) { //1
                pr0=mm1+(pr0);
            }else{
                pr0=mm3+(pr0);
            }
        }else{
            pr0=(((2047+2047)>>1)+(pr0));
        }
        pr0=(pr0)>>1;
        pr0=a.p(pr0, x.c0);
        pr0=(4096-pr0)*(32768/4096);
        if(pr0<1) pr0=1;
        if(pr0>32767) pr0=32767;
    }
};
