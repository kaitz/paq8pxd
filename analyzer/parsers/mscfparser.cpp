#include "mscfparser.hpp"

MSCFParser::MSCFParser() {
    priority=2;
    Reset();
    inpos=0;
    name="MSCF";
}

MSCFParser::~MSCFParser() {
}

// loop over input block byte by byte and report state
DetectState MSCFParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<1024*4) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf2==0x4D534346 && buf1==0 && buf0!=0) ){
            state=START;
            mscf=i-11;
            mscfs=bswap(buf0); // Total size of the cabinet file
            //printf("%d ",mscfs);
        } else if (state==START) {
            int p=i-mscf;
            if (p==15 && buf0!=0) state=NONE;     // Reserved
            else if (p==19) {                     // Offset of first entry
                mscfoff=bswap(buf0);
                //printf("Files offset %d ",mscfoff);
                state=buf0==0?NONE:state; 
                if (mscfoff>mscfs) state=NONE;
            } 
            else if (p==23 && buf0!=0) state=NONE; // Reserved
            else if (p==24 && c!=3) state=NONE;    // Version Minor
            else if (p==25 && c!=1) state=NONE;    // Version Major
            else if (p==27) {                      // Folders
                folders=(buf0&0xffff);
                folders=bswap16(folders);
                if (folders==0) state=NONE;
                //printf("Folders %d ",folders);    
            } else if (p==29) {                    // Files
                files=(buf0&0xffff);
                files=bswap16(files);
                if (files==0 || files>4000) state=NONE;
                //printf("Files %d \n",files);    
            }
            else if (mscfoff && (mscf+mscfoff+7)==i) {
                // Test first entry
                //printf("Uncompressed files size %d \n",bswap(buf1));
                //printf("zero %d \n",bswap(buf0));
                //if (buf0!=0) state=NONE;
                if (buf1>0x7FFFFFFF) state=NONE; // max 2g
            }
            else if (mscfoff && i>(mscf+mscfoff+7)) {
                // Ignore header when reporting
                jstart=mscf+mscfoff+files*4*4+files*10; // add file dir offset files*header files*~filenamelen
                jend=jstart+mscfs-mscfoff-files*4*4+files*10;
                type=CMP;
                state=END;
                pinfo=" MS Cab 1.03";
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

dType MSCFParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int MSCFParser::TypeCount() {
    return 1;
}

void MSCFParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=0;
    mscf=mscfs=mscfoff=0;
    files=folders=0;
    info=i=inSize=0;
    pinfo="";
}
void MSCFParser::SetEnd(uint64_t e) {
    jend=e;
}
