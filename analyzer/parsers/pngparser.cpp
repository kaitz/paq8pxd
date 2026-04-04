#include "pngparser.hpp"

PNGParser::PNGParser():pngF(0) {
    priority=2;
    Reset();
    inpos=0;
    name="png";
}

PNGParser::~PNGParser() {
}

// Loop over input block byte by byte and report state
DetectState PNGParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    if (pos==0 && len<128) return DISABLE;
    
    // New data block - reset position tracking
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }

    while (inSize<len && rec==false) {
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (buf3==0x89504E47 && buf2==0x0D0A1A0A && buf1==0x0000000D && buf0==0x49484452) ) {
            // PNG signature + IHDR chunk detected
            state=START;
            png=i, pngtype=-1, lastchunk=buf3;
            pali=0; isPal=false;
            pfcount=0;
        } else if (state==NONE && buf0==0x49444154 && (uint64_t)inSize+2<len) { // Not safe, will conflict with MNG file
            // Detached IDAT sequence detection (smart scanning)
            // Validate ZLIB header immediately following IDAT type
            uint8_t cmf = data[inSize+1];
            uint8_t flg = data[inSize+2];
            if ( (cmf&0xF)==8 && (flg&0x20)==0 && (((int)cmf*256+flg)%31)==0 ) {
                state=INFO;
                png=i-100; // Fake offset to satisfy p>13 check later
                
                // i points to last byte of "IDAT" type
                jstart = i - 7; // Start of Length field
                type=PNG24; info=(PNG24<<24);
                pinfo="(IDAT stream)";
                
                idat_end = i + buf1 + 5; // End of this IDAT chunk (after CRC)
                idats = 1;
                jend = idat_end;
                
                // Next chunk's Type field ends at: current + data_len + CRC(4) + Length(4) + Type(4)
                nextchunk = i + buf1 + 12;
                lastchunk = 0x49444154;
            }
        } else if (state==START || state==INFO) {
            if (png) {
                if (isPal) { // fill pal buffer and test if grayscale
                    pal[pali++]=c;
                    if (pali==palcolors) {
                        pnggray=IsGrayscalePalette(&pal[0], palcolors/3);
                        isPal=false;
                        palcolors=pali=0;
                    }
                }
                if (isiCCP) { // read iCCP profile and look if it is compressed
                    pal[pali++]=c;
                    //characters (33-126 and 161-255) and spaces (32), but no leading, trailing, or consecutive spaces
                    if ((buf0&0xffff)==0 && pali<80) {
                        iCCPsize=iCCPsize-pali;
                        pali=0;
                        isiCCP=false;
                        iCCPinfo=iCCPsize+((i)<<16);
                        //printf("Profile %s\n %d\n",pal,iCCPsize);
                        PNGfile pf;
                        pf.start=i+1; // off by one
                        pf.size=i+1+iCCPsize;
                        pf.f=PREFLATE;
                        pf.info=(2<<16)+4; //Header (2) + Adler32 (4)
                        pf.pinfo="PNG - ";
                        pf.pinfo+= reinterpret_cast<char*>(pal);
                        pngF.push_back(pf);
                    } else if (pali>80) {
                        pali=0;
                        isiCCP=false;
                    }
                }
                const int p=i-png;
                if (p==13) { // This is not safe, IHDR not tested
                    // Parse IHDR data (offsets relative to IHDR type end)
                    auto getByte = [&](int offset) {
                        return data[inSize - (13 - offset)];
                    };

                    pngw = (getByte(1)<<24) | (getByte(2)<<16) | (getByte(3)<<8) | getByte(4);
                    pngh = (getByte(5)<<24) | (getByte(6)<<16) | (getByte(7)<<8) | getByte(8);
                    pngbps = getByte(9);
                    pngtype = getByte(10);
                    uint8_t interlace = getByte(13);
                    
                    pnggray = 0;
                    
                    bool valid = (pngw > 0 && pngh > 0 && 
                    (pngbps==1 || pngbps==2 || pngbps==4 || pngbps==8 || pngbps==16) && 
                    (pngtype==0 || pngtype==2 || pngtype==3 || pngtype==4 || pngtype==6) &&
                    interlace <= 1);
                    
                    if (!valid) {
                        png = 0;
                        state = NONE;
                    } else {
                        // Determine PNG subtype from color type
                        if (pngw<0x1000000 && pngh) {
                            if ((pngbps==8||pngbps==16) && pngtype==2) info=(IMAGE24<<24)|(pngw*3), type=PNG24;
                            else if ((pngbps==8||pngbps==16) && pngtype==6 ) info=(IMAGE32<<24)|(pngw*4), type=PNG32;
                            else if (pngbps==4 && pngtype==3) info=(IMAGE4<<24)|(pngw/2), type=PNG8; // 4bit image
                            else if (pngbps==2 && pngtype==3) info=(IMAGE4<<24)|(pngw/4), type=PNG8; // 2bit image // where ?
                            else if (pngbps==1 && pngtype==3) info=(IMAGE1<<24)|(pngw/8), type=PNG8; // 1bit image
                            else if ((pngbps==8||pngbps==16) && (!pngtype || pngtype==3)) info=(((!pngtype || pnggray)?IMAGE8GRAY:IMAGE8)<<24)|(pngw), type=(!pngtype || pnggray)?PNG8GRAY:PNG8;
                            else {
                             type=PNG24, info=(IMAGE24<<24)|(pngw*3);   
                             //gray with alfa?
                            }
                        } else {
                            type=PNG24;
                        }
                        pinfo="(width: "+ itos(info&0xffffff) +")";
                        // Next chunk's Type ends after: CRC(4) + Length(4) + Type(4) = 12 bytes from current
                        nextchunk = i + 12;
                    }
                } else if (p>13 && pngtype<0){
                    png = 0; state=NONE;    
                } else if (p>13 && i==nextchunk) {
                    state=INFO;
                    nextchunk+=buf1+4+8; // Advance past: data + CRC(4) + next Length(4) + next Type(4)
                    lastchunk = buf0;
                    if (lastchunk==0x49444154){ // IDAT
                        if (idats == 0) {
                            // First IDAT - set start to beginning of this chunk
                            jstart = i - 7;
                            
                            // Refine type based on IHDR parsing
                            if (pngw<0x1000000 && pngh) {
                                if (pngbps==8 && pngtype==2) info=(IMAGE24<<24)|(pngw*3), type=PNG24;
                                else if (pngbps==8 && pngtype==6 ) info=(IMAGE32<<24)|(pngw*4), type=PNG32;
                                else if (pngbps==4 && pngtype==3) info=(IMAGE4<<24)|(pngw/2), type=PNG8; // 4bit image
                                else if (pngbps==2 && pngtype==3) info=(IMAGE4<<24)|(pngw/4), type=PNG8; // 2bit image // where ?
                                else if (pngbps==1 && pngtype==3) info=(IMAGE1<<24)|(pngw/8), type=PNG8; // 1bit image
                                else if (pngbps==8 && (!pngtype || pngtype==3)) info=(((!pngtype || pnggray)?IMAGE8GRAY:IMAGE8)<<24)|(pngw), type=(!pngtype || pnggray)?PNG8GRAY:PNG8;
                                else {
                                 type=PNG24, info=(IMAGE24<<24)|(pngw*3);   
                                }
                            } else {
                                type=PNG24;
                            }
                            pinfo="(width: "+ itos(info&0xffffff) +")";
                        }
                        idats++;
                        // End of this IDAT = i + data_len + CRC(4) + 1 to get past last byte
                        idat_end = i + buf1 + 5; 
                        jend = idat_end;
                    } else if (lastchunk==0x504C5445) { //PLTE
                        isPal=true;
                        palcolors=buf1;
                        assert(palcolors<=768);
                        pali=0;
                        png*=(buf1%3==0);
                        if (png==0) state=NONE; // fail if bad size
                    } else if (lastchunk==0x69434350  || lastchunk==0x7A545874 ) { //iCCP zTXt
                        //characters (33-126 and 161-255) and spaces (32), but no leading, trailing, or consecutive spaces
                        iCCPsize=buf1;
                        pali=0;
                        isiCCP=true;
                    } else {
                        // Non-IDAT chunk after IDATs - end the sequence
                        if (idats > 0) {
                            jend = idat_end;
                            state=END;
                            if (pngw==0) {state=NONE; continue;}
                            if (pngF.size()==0) {
                                return state;
                            } else {
                                PNGfile pngfile=pngF[0];
                                PNGfile pf;
                                pf.start=jstart;
                                pf.size=jend;
                                pf.f=type;
                                pf.info=info;
                                pf.pinfo=pinfo;
                                pngF.push_back(pf);
                                //printf("Types: %d %d\n",pngF.size(),pf.start,pf.size);
                                pfcount=0;
                                rec=true;
                                break; //return state;
                            }
                        }
                    }
                    if (lastchunk==0x49454E44) { // IEND
                        if (idats > 0) {
                            jend = idat_end;
                            state=END;
                            if (pngw==0) {state=NONE; continue;}
                            if (pngF.size()==0) {
                                return state;
                            }else{
                                assert(pngF.size()==1);
                                PNGfile pngfile=pngF[0];
                                PNGfile pf;
                                pf.start=jstart;
                                pf.size=jend;
                                pf.f=type;
                                pf.info=info;
                                pf.pinfo=pinfo;
                                pngF.push_back(pf);
                                //printf("Types: %d\n",pngF.size());
                                pfcount=0;
                                if (pngw==0) state=NONE;
                                rec=true;
                                break; 
                            }
                        }
                        state=NONE;
                        return state;
                    }
                }
                
            }
        }

        inSize++;
        i++;
    }
    if (rec && state==END) {
        PNGfile pngfilel;
        pngfilel.size=0;
        if (pfcount) pngfilel=pngF[pfcount-1];
        PNGfile pngfile=pngF[pfcount++];
        jstart=pngfile.start-pngfilel.size;
        jend=pngfile.size-pngfile.start+jstart;
        //printf("%d %d %d pos %d\n",pfcount,jstart,jend,pngfilel.size);
        type=pngfile.f;
        info=pngfile.info;
        pinfo=pngfile.pinfo;
        //priority=0;
        if (pfcount==pngF.size()) {
            while (pngF.size())pngF.pop_back();
            rec=false,priority=2;
        } 
        return state;        
    }
    if (state==INFO) return INFO;
    if (state!=NONE)
    return DATA;
    else 
    return NONE;
}

dType PNGParser::getType() {
    dType t;
    t.start=jstart;
    t.end=jend;
    t.info=info;
    t.rpos=0;
    t.type=type;
    t.recursive=rec; 
    t.pinfo=pinfo;
    return t;
}

void PNGParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=0;
    png=lastchunk=nextchunk=0;               
    pngw=pngh=pngbps=pngtype=pnggray=0;
    info=i=inSize=0;
    idats=0;
    idat_end=0;
    priority=2;
    palcolors=0;
    pali=0;
    isPal=isiCCP=false;
    iCCPinfo=iCCPsize=pfcount=0;
    while (pngF.size()) pngF.pop_back();
    rec=false;
}
