#include "bzip2parser.hpp"
#define BZ2BLOCK 100*1024*100

bzip2Parser::bzip2Parser() {
    priority=1;
    Reset();
    inpos=0;
    name="bzip2";
    bzout=(uint8_t*)calloc(BZ2BLOCK,1);
}

bzip2Parser::~bzip2Parser() {
    free(bzout);
}

// loop over input block byte by byte and report state
DetectState bzip2Parser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;
        
        if (i==7 && buf1==0x42534449 && buf0==0x46463430/*-35*/) isBSDIFF=true;
        if (state==NONE && (isBSDIFF==false && (buf0&0xffffff00)==0x425A6800) ){
            state=START;
            bzlevel=c-'0';
            BZip2=i-3;
            int csize=len-inSize+3;
            stream.bzalloc=NULL;
            stream.bzfree=NULL;
            stream.opaque=NULL;
            stream.avail_in=0;
            stream.next_in=NULL;
            int ret;
            blockz=csize?csize:0x10000;
            
            ret=BZ2_bzDecompressInit(&stream, 0, 0);
            if (ret!=BZ_OK) state=NONE;
            else{
                stream.avail_in=csize;
                stream.next_in=(char*)&data[inSize-3];

            stream.avail_out=BZ2BLOCK;
            stream.next_out=(char*)&bzout[0];
            ret=BZ2_bzDecompress(&stream);
            if ((ret!=BZ_OK) && (ret!=BZ_STREAM_END)) {
                (void)BZ2_bzDecompressEnd(&stream);
                state=NONE;
            }
            blockz=0x10000;
            if (state!=NONE) {
                state=INFO;
            return INFO;
        }
            }
        } else if (state==START || state==INFO) {
            int ret;
            stream.avail_in=min(len,blockz);
            if (stream.avail_in==0) break;
            stream.next_in=(char*)&data[0];
            stream.avail_out=BZ2BLOCK;
            stream.next_out=(char*)&bzout[0];
            ret=BZ2_bzDecompress(&stream);
            if ((ret!=BZ_OK) && (ret!=BZ_STREAM_END)) {
                (void)BZ2_bzDecompressEnd(&stream);
                state=NONE;
            }
            state=INFO;
            if (ret==BZ_STREAM_END) {
                (void)BZ2_bzDecompressEnd(&stream);
                jstart=BZip2;
                jend=jstart+(stream.total_in_lo32+(stream.total_in_hi32<<8));
                info=bzlevel;
                type=BZIP2;
                state=END;
                return state;
            }
            if (state==INFO) return INFO;
        }

        inSize++;
        i++;
    }
    if (state==INFO) return INFO;
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType bzip2Parser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int bzip2Parser::TypeCount() {
    return 1;
}

void bzip2Parser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=0;
    BZip2=blockz=0;
    bzlevel=0;
    isBSDIFF=false;
    info=i=inSize=0;
}
void bzip2Parser::SetEnd(uint64_t e) {
    jend=e;
}
