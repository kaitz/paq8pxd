#include "mrbfilter.hpp"

ImgMRBFilter::ImgMRBFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
}

// simple color transform (b, g, r) -> (g, g-r, g-b)
void ImgMRBFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    int width=int(info&0xffffffff);
    int height=int(info>>32);
  U64 savepos= in->curpos();
    int totalSize=(width)*height;
    Array<U8,1> ptrin(totalSize+4);
    Array<U8,1> ptr(size+4);
    Array<U32> diffpos(4096);
    U32 count=0;
    U8 value=0; 
    int diffcount=0;
    // decode RLE
    for(int i=0;i<totalSize; ++i){
        if((count&0x7F)==0)    {
            count=in->getc();
            value=in->getc();
        }
        else if(count&0x80)    {
            value=in->getc();
        }
        count--;
        ptrin[i] =value; 
    }
    // encode RLE
    int a=encodeRLE(&ptr[0],&ptrin[0],totalSize,size);
    assert(a<(size+4));
    // compare to original and output diff data
    in->setpos(savepos);
    for(int i=0;i<size;i++){
        U8 b=ptr[i],c=in->getc();
        if (diffcount==4095 ||  diffcount>(size/2)||i>0xFFFFFF) return; // fail
        if (b!=c ) {
            if (diffcount<4095)diffpos[diffcount++]=c+(i<<8);
        }
    }
    out->putc((diffcount>>8)&255); out->putc(diffcount&255);
    if (diffcount>0)
    out->blockwrite((U8*)&diffpos[0], diffcount*4);
    out->put32(size);
    out->blockwrite(&ptrin[0], totalSize);
} 

uint64_t ImgMRBFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    if (size==0) {
        diffFound=1;
        return 0;
    }
    Array<U32> diffpos(4096);
    int diffcount=0;
    diffcount=(in->getc()<<8)+in->getc();
    if (diffcount>0) in->blockread((U8*)&diffpos[0], diffcount*4);
    int len=in->get32();
    Array<U8,1> fptr(size+4);
    Array<U8,1> ptr(size+4);
    in->blockread(&fptr[0], size);
    encodeRLE(&ptr[0],&fptr[0],size-2-4-diffcount*4,len); //size - header
    //Write out or compare
    int diffo=diffpos[0]>>8;
    int diffp=0;
    for(int i=0;i<len;i++) {
        if (i==diffo && diffcount) {             
            ptr[i]=diffpos[diffp]&255,diffp++,diffo=diffpos[diffp]>>8 ;
        }
    }    
    out->blockwrite(&ptr[0], len);
    assert(len<size);
    fsize=len;
    return len;
}

int ImgMRBFilter::encodeRLE(U8 *dst, U8 *ptr, int src_end, int maxlen) {
    int i=0;
    int ind=0;
    for(ind=0;ind<src_end; ) {
        if (i>maxlen) return i;
        if (ptr[ind+0]!=ptr[ind+1] || ptr[ind+1]!=ptr[ind+2]) {
            // Guess how many non repeating bytes we have
            int j=0;
            for( j=ind+1;j<(src_end);j++)
            if ((ptr[j+0]==ptr[j+1] && ptr[j+2]==ptr[j+0]) || ((j-ind)>=127)) break;
            int pixels=j-ind;
            if (j+1==src_end && pixels<8)pixels++;
            dst[i++]=0x80 |pixels;
            for(int cnt=0;cnt<pixels;cnt++) { 
                dst[i++]=ptr[ind+cnt]; 
                if (i>maxlen) return i;              
            }
            ind=ind+pixels;
        } else {
            // Get the number of repeating bytes
            int j=0;
            for (j=ind+1;j<(src_end);j++)
                if (ptr[j+0]!=ptr[j+1]) break;
            int pixels=j-ind+1;          
            if (j==src_end && pixels<4) {
                pixels--;              
                dst[i]=U8(0x80 |pixels);
                i++ ;
                if (i>maxlen) return i;
                for(int cnt=0;cnt<pixels;cnt++) { 
                    dst[i]=ptr[ind+cnt]; 
                    i++;
                    if (i>maxlen) return i;
                }
                ind=ind+pixels;
            } else { 
                j=pixels;  
                while (pixels>127) {
                    dst[i++]=127;                
                    dst[i++]=ptr[ind];  
                    if (i>maxlen) return i;                     
                    pixels=pixels-127;
                }
                if (pixels>0) { 
                    if (j==src_end) pixels--;
                    dst[i++]=pixels;         
                    dst[i++]=ptr[ind];
                    if (i>maxlen) return i;
                }
                ind=ind+j;
            }
        }
    }
    return i;
}

ImgMRBFilter::~ImgMRBFilter() {
}

