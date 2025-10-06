#include "base85filter.hpp"

int powers[5] = {85*85*85*85, 85*85*85, 85*85, 85, 1};

base85Filter::base85Filter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void base85Filter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
   int lfp=0;
    int tlf=0;
    int b64mem=(len>>2)*5+100;
    Array<U8,1> ptr(b64mem);
    int olen=5;
    int c, count = 0;
    uint32_t tuple = 0;
    for (int f=0;f<len;f++) {
        c = in->getc();
        if (olen+10>b64mem) {count = 0; break;} //!!
        if (c==13 || c==10) {
            if (lfp==0) lfp=f ,tlf=c;
            if (tlf!=c) tlf=0;
            continue;
        }
        if (c == 'z' && count == 0) {
            if (olen+10>b64mem) {count = 0; break;} //!!
            for (int i = 1; i < 5; i++) ptr[olen++]=0;
            continue;
        }
        /*    if (c == 'y' && count == 0) {
            for (int i = 1; i < 5; i++) fptr[olen++]=0x20;
            continue;
        }*/
        if (c == EOF) {  
        if (olen+10>b64mem) {count = 0; break;} //!!      
            if (count > 0) {
                
                tuple += powers[count-1];
                for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            }
            break;
        }
        tuple += (c - '!') * powers[count++];
        if (count == 5) {
           if (olen>b64mem+10) {count = 0; break;} //!!
            for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            tuple = 0;
            count = 0;
        }
    }
    if (count > 0) {
        
        tuple += powers[count-1];
        for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
    }
    ptr[0]=lfp&255; //nl lenght
    ptr[1]=len&255;
    ptr[2]=len>>8&255;
    ptr[3]=len>>16&255;
    if (tlf!=0) {
        if (tlf==10) ptr[4]=128;
        else ptr[4]=64;
    }
    else
    ptr[4]=len>>24&63; //1100 0000
    out->blockwrite(&ptr[0],   olen  );
} 

uint64_t base85Filter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    int i;
    int fle=0;
    int nlsize=0; 
    int outlen=0;
    int tlf=0;
    nlsize=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> ptr((outlen>>2)*5+10);
    tlf=(tlf&192);
    if (tlf==128)      tlf=10;        // LF: 10
    else if (tlf==64)  tlf=13;        // LF: 13
    else               tlf=0;
    int c, count = 0, lenlf = 0;
    uint32_t tuple = 0;

    while(fle<outlen){ 
        c = in->getc();
        if (c != EOF) {
            tuple |= ((U32)c) << ((3 - count++) * 8);
            if (count < 4) continue;
        }
        else if (count == 0) break;
        int i, lim;
        char out[5];
        if (tuple == 0 && count == 4) { // for 0x00000000
            if (nlsize && lenlf >= nlsize) {
                if (tlf) ptr[fle++]=(tlf);
                else ptr[fle++]=13,ptr[fle++]=10;
                lenlf = 0;
            }
            ptr[fle++]='z';
        }
        /*    else if (tuple == 0x20202020 && count == 4 ) {
            if (nlsize && lenlf >= nlsize) {
                if (tlf) fptr[fle++]=(tlf);
                else fptr[fle++]=13,fptr[fle++]=10;
                lenlf = 0;
            }
            fptr[fle++]='y',lenlf++;
        }*/
        else {
            for (i = 0; i < 5; i++) {
                out[i] = tuple % 85 + '!';
                tuple /= 85;
            }
            lim = 4 - count;
            for (i = 4; i >= lim; i--) {
                if (nlsize && lenlf >= nlsize && ((outlen-fle)>=5)) {//    skip nl if only 5 bytes left
                    if (tlf) ptr[fle++]=(tlf);
                    else ptr[fle++]=13,ptr[fle++]=10;
                    lenlf = 0;}
                ptr[fle++]=out[i],lenlf++;
            }
        }
        if (c == EOF) break;
        tuple = 0;
        count = 0;
    }
    out->blockwrite(&ptr[0],   outlen  );
    fsize=outlen;
    return fsize;
}

base85Filter::~base85Filter() {
}

