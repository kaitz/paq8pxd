#include "tiffparser.hpp"

TIFFParser::TIFFParser():dtf(0),ifd(0) {
    priority=2;
    inpos=0;
    name="tiff";
    Reset();
    tagT=(uint8_t*)calloc(sizeof(TiffTag)*256,1); // up to 256 entries
    tagCd=(uint8_t*)calloc(sizeof(uint8_t)*0x10000,1);
}

TIFFParser::~TIFFParser() {
    free(tagT);
    free(tagCd);
}
const std::string  TIFFParser::TIFFCompStr(int i) {
    if (i==1) return "Uncompressed";
    else if (i==2) return "CCITT 1D";
    else if (i==3) return "Group 3 Fax"; 
    else if (i==4) return "Group 4 Fax"; 
    else if (i==5) return "LZW"; 
    else if (i==6) return "\'Old style\' JPEG"; 
    else if (i==7) return "\'New style\' JPEG"; 
    else if (i==8) return "DEFLATE (zlib)"; 
    else if (i==9) return "JBIG"; 
    else if (i==10) return "JBIG"; 
    else if (i==32766) return "NeXT 2b RLE"; 
    else if (i==32767) return "Sony ARW"; 
    else if (i==32769) return "Packed RAW (Epson ERF)"; 
    else if (i==32770) return "Samsung SRW"; 
    else if (i==32771) return "CCITTRLEW"; 
    else if (i==32773) return "PackBits"; 
    else if (i==32809) return "ThunderScan"; 
    else if (i==32867) return "Kodak KDC"; 
    else if (i==32895) return "IT8CTPAD"; 
    else if (i==32896) return "IT8LW"; 
    else if (i==32897) return "IT8MP"; 
    else if (i==32898) return "IT8BL"; 
    else if (i==32908) return "PIXARFILM"; 
    else if (i==32909) return "PIXARLOG"; 
    else if (i==32946) return "DEFLATE (same as code 8)"; 
    else if (i==32947) return "Kodak DCS"; 
    else if (i==33003) return "Aperio SVS"; 
    else if (i==33005) return "Aperio SVS"; 
    else if (i==34661) return "JBIG"; 
    else if (i==34676) return "SGILOG 32b RLE"; 
    else if (i==34677) return "SGILOG24 24b packed"; 
    else if (i==34692) return "LuraDocument"; 
    else if (i==34712) return "JPEG 2000"; 
    else if (i==34713) return "Nikon NEF"; 
    else if (i==34715) return "JBIG2"; 
    else if (i==34718) return "MDI";  
    else if (i==34719) return "MDI"; 
    else if (i==34720) return "MDI"; 
    else if (i==34887) return "LERC"; 
    else if (i==34892) return "Lossy JPEG (DNG)"; 
    else if (i==34925) return "LZMA2"; 
    else  return "UNKNOWN";
}

// 1 = BYTE 8-bit unsigned integer.
// 2 = ASCII 8-bit byte that contains a 7-bit ASCII code; the last byte must be NUL (binary zero).
// 3 = SHORT 16-bit (2-byte) unsigned integer.
// 4 = LONG 32-bit (4-byte) unsigned integer.
// 5 = RATIONAL
// 6 = SBYTE An 8-bit signed (twos-complement) integer.
// 7 = UNDEFINED An 8-bit byte that may contain anything, depending on the definition of the field.
// 8 = SSHORT A 16-bit (2-byte) signed (twos-complement) integer.
// 9 = SLONG A 32-bit (4-byte) signed (twos-complement) integer.
// 10 = SRATIONAL Two SLONG’s: the first represents the numerator of a fraction, the second the denominator.
// 11 = FLOAT Single precision (4-byte) IEEE format.
// 12 = DOUBLE
const std::string  TIFFParser::TIFFTypeStr(int i) {
    if (i==1) return "BYTE";
    else if (i==2) return "ASCII";
    else if (i==3) return "SHORT";
    else if (i==4) return "LONG";
    else if (i==5) return "RATIONAL";
    else if (i==6) return "SBYTE";
    else if (i==7) return "UNDEFINED";
    else if (i==8) return "SSHORT";
    else if (i==9) return "SLONG";
    else if (i==10) return "SRATIONAL";
    else if (i==11) return "FLOAT";
    else if (i==12) return "DOUBLE";
    else  return "UNKNOWN";
}
// Get next smallest address index to read
const uint64_t TIFFParser::NextTagContent(uint64_t x) {
    uint64_t next=0;
    for (size_t k=0; k<tagC.size(); k++) {
        //if (tagC[k].TagId==259) { 
        //    printf("%5d %10s %10d %10d %10s\n",tagC[k].TagId,TIFFTypeStr(tagC[k].Type).c_str(),tagC[k].Count,tagC[k].Offset,TIFFCompStr(tagC[k].Offset).c_str());
        //} else
        //    printf("%5d %10s %10d %10d\n",tagC[k].TagId,TIFFTypeStr(tagC[k].Type).c_str(),tagC[k].Count,tagC[k].Offset);
        // Offsets are allready in order, replace this
        if (x==0) {
            tagCx=next=tagC[k].Offset;
            tagCs=(tagC[k].Type==3?2:tagC[k].Type);
            tagCc=tagC[k].Count;
            tagCi=tagC[k].TagId;
            break;
        } else if (tagC[k].Offset>tagCx){
            tagCx=next=tagC[k].Offset;
            tagCs=(tagC[k].Type==3?2:tagC[k].Type);
            tagCc=tagC[k].Count;
            tagCi=tagC[k].TagId;
            break;
        }
        
    }
    if (next==0) tagCx=0;
    //printf("List end %d\n",tagCx);
    return tagCx;
}
// loop over input block byte by byte and report state
DetectState TIFFParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    // To small? 
    if (pos==0 && len<128) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos || pos==0) {
        inSize=0,inpos=pos;
        i=pos;
        if (state==END && parseCount==0) state=NONE,rec=false;
    }
    uint64_t Tag279=0;
    while (inSize<len) {
        buf3=(buf3<<8)|(buf2>>24);
        buf2=(buf2<<8)|(buf1>>24);
        buf1=(buf1<<8)|(buf0>>24);
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;

        if (state==NONE && (
                    (buf1==0x49492a00 ||(buf1==0x4949524f && buf0==0x8000000  ) ) /*&& (i+len)>(i+(int)bswap(buf0))*/ || 
                    ((buf1==0x4d4d002a  ) /*&& (i+len)>(i+(int)(buf0) )*/
                        ))) {
            state=START;
            if (buf1==0x4d4d002a) tiffMM=true;
            tiffImageStart=0,tiffImages=-1;
            dirEntry=tiffMM==true?buf0:bswap(buf0);
            tiffi=i-7;
            //printf("First dir: %d\n",dirEntry);
            state=INFO;
            memset(&idfImg, 0, sizeof(idfImg));
        } else if (state==START || state==INFO) {
            if (i==(dirEntry+1+tiffi) && tagsIn==0) {
                tagsIn=tiffMM==true?(data[inSize-1]<<8|data[inSize]):(data[inSize-1]|(data[inSize]<<8));
                //printf("Tags dir: %d\n",tagsIn);
                if (tagsIn>255) state=NONE,rec=false; // just fail if more then 255 tags
            } else if (tagsIn && tagTx<tagsIn*12 && i>=(dirEntry+tiffi)){ // read in tif tags
                tagT[tagTx++]=c;
            }else if (tagCx && i>=(tagCx+tiffi)) {
                // Read in tag content if any and parse
                tagCd[tagCdi++]=c;
                if (tagCdi==(tagCs*tagCc)) {
                    //Number of bits per component
                    if (tagCi==258) {
                        int bits=0;
                        for (int k=0; k<tagCc; k++) {
                            uint16_t &tg=(uint16_t&)tagCd[sizeof(uint16_t)*(k)];
                            bits=bits+(tiffMM==true?bswap16(tg):tg); //=8?
                        }
                        //printf(" Bits: %d\n",bits);     //idfImg.bits1==bits/idfImg.bits1 ?
                        idfImg.bits=bits;
                        idfImg.bits1=tagCc;
                    // 273 For each strip, the byte offset of that strip.
                    // 324 For each tile, the byte offset of that tile, as compressed and stored on disk.
                    } else if (tagCi==273 || tagCi==324 ) {
                        uint32_t offs=0;
                        uint32_t &tg=(uint32_t&)tagCd[sizeof(uint32_t)*(0)];  // we only care for the first value - start offset
                        offs=(tiffMM==true?bswap(tg):tg);
                        //printf("\n%d Data size: %d\n",offs,tagCs*tagCc);
                        if (idfImg.offset==0) idfImg.offset=offs;
                        
                    //279 For each strip, the number of bytes in the strip after compression.
                    //325 For each tile, the number of (compressed) bytes in that tile.
                    } else if (tagCi==279 || tagCi==325) {
                        uint32_t size=0;
                        for (int k=0; k<tagCc; k++) {
                            if (tagCs==2){
                            uint16_t &tg=(uint16_t&)tagCd[sizeof(uint16_t)*(k)];
                            size=size+(tiffMM==true?bswap16(tg):tg);
                            }  else if (tagCs==4){
                                uint32_t &tg=(uint32_t&)tagCd[sizeof(uint32_t)*(k)];
                            size=size+(tiffMM==true?bswap(tg):tg);
                            }
                        }
                        //printf(" Size: %d Data size: %d\n",size,tagCs*tagCc);
                        if (idfImg.size==0) idfImg.size=size;
                    }
                    // Get next tag data offset
                    tagCx=NextTagContent(tagCx);
                    if (tagCx && (tagCx+tiffi)<i) state=NONE,rec=false;//, printf("Past tag data!\n");
                    tagCdi=0;
                    // Was last tag?
                    if (tagCx==0) {
                        if (idfImg.size==0)idfImg.size=idfImg.width*idfImg.bits1*idfImg.height;
                        if (dtf.size()==0) idfImg.offset+=tiffi; // for the first image add start pos
                        if (idfImg.offset!=0 && idfImg.size!=0) dtf.push_back(idfImg),rec=true;
                        //printf("Image(%d) offset: %d, size %d, bits %d\n",dtf.size(),idfImg.offset,idfImg.size,idfImg.bits);
                        if (dirEntry==0) parseCount=dtf.size(),jend=0,relAdd=tiffi,state=END; // Last entry?
                    }
                }
            } else if (tagsIn && i==(dirEntry+4+1+tagsIn*12+tiffi)) {
                TiffIFD d;
                d.NumDirEntries=tagsIn;
                d.CurrentIFD=dirEntry;
                dirEntry=(d.NextIFD=(tiffMM==true?buf0:bswap(buf0))); // ?
                //printf("Next: %d\n",d.NextIFD);
                ifd.push_back(d);
                //printf("Tag     Type          Count  Val/Offset\n");
                memset(&idfImg, 0, sizeof(idfImg));
                tagC.clear();
                tagCx=0;
                for (int k=tagsIn; k>0;k--) {
                    TiffTag &tg=(struct TiffTag&)tagT[sizeof(TiffTag)*(tagsIn-k)];
                    if (tiffMM==true) {
                        tg.TagId=bswap16(tg.TagId);
                        tg.Type=bswap16(tg.Type);
                        tg.Count=bswap(tg.Count);
                        if (tg.Type==3 && tg.Count==1)tg.Offset=bswap16(tg.Offset);
                        else 
                        tg.Offset=bswap(tg.Offset);
                    }
                    //if (tg.TagId==259) { 
                    //    printf("%5d %10s %10d %10d %10s\n",tg.TagId,TIFFTypeStr(tg.Type).c_str(),tg.Count,tg.Offset,TIFFCompStr(tg.Offset).c_str());
                    //} 
                    //else if (tg.Type==3 && tg.Count==1)
                    //printf("%5d %10s %10d %10d\n",tg.TagId,TIFFTypeStr(tg.Type).c_str(),tg.Count,tg.Offset&0xffff);
                    //else
                    //    printf("%5d %10s %10d %10d\n",tg.TagId,TIFFTypeStr(tg.Type).c_str(),tg.Count,tg.Offset);
                    ifd[count].Tags.push_back(tg);
                    // Read tag data for id=258,273,279,324,325
                    if (tg.Count!=1 && (tg.TagId==258 || tg.TagId==273 || tg.TagId==324|| tg.TagId==325 || tg.TagId==279) && ((tg.Type==3?2:tg.Type)*tg.Count)<0x10000) {
                        tagC.push_back(tg); // list of tag addresses to read
                    }
                    if (tg.TagId==256) idfImg.width=tg.Offset;
                    else if (tg.TagId==257) idfImg.height=tg.Offset;
                    else if (tg.TagId==259) idfImg.compression=tg.Offset==1?0:tg.Offset;
                    else if (tg.TagId==258) idfImg.bits=tg.Count==1?tg.Offset:0; // bits per component
                    else if (tg.TagId==273 && tg.Count==1) idfImg.offset=tg.Offset;
                    else if (tg.TagId==279 && tg.Count==1) idfImg.size=tg.Offset;
                    else if (tg.TagId==277) idfImg.bits1=tg.Offset;
                    else if (tg.TagId==513 && tg.Count==1) idfImg.offset=tg.Offset;
                    else if (tg.TagId==514 && tg.Count==1) idfImg.size=tg.Offset,idfImg.compression=6;
                    
                }
                count++; // IFD entry count
                tagsIn=tagTx=0; 
                memset(tagCd, 0, 0x10000);
                //printf(" IFD end %d\n",i+1);
                // Sort tag offsets so we do not skip any and get first
                std::sort(tagC.begin(), tagC.end(), [](const TiffTag &a, const TiffTag &b){
                    return (a.Offset < b.Offset);
                });
                tagCdi=0;
                // Get next tag data offset
                tagCx=NextTagContent(tagCx);
                // Was last tag?
                if (tagCx==0){
                    if (idfImg.size==0)idfImg.size=idfImg.width*idfImg.bits1*idfImg.height;
                    if (dtf.size()==0) idfImg.offset+=tiffi; // for the first image add start pos
                    if (idfImg.offset!=0 && idfImg.size!=0) dtf.push_back(idfImg),rec=true;
                    //printf("Image(%d) offset: %d, size %d, bits %d\n",dtf.size(),idfImg.offset,idfImg.size,idfImg.bits);
                    if (dirEntry==0) parseCount=dtf.size(),jend=0,relAdd=tiffi,state=END; // Last entry?
                }
            }
        } else if (state==END && parseCount) {
            if ((dtf.size()-parseCount)==0) {
                // Sort out of order data in ascending orders
                std::sort(dtf.begin(), dtf.end(), [](const detTIFF &a, const detTIFF &b) {
                    return (a.offset < b.offset);
                });
            }
            detTIFF image=dtf[dtf.size()-parseCount];
            jstart=image.offset-(relAdd-tiffi);
            jend=jstart+image.size;
            relAdd+=jend-tiffi;
            relAdd-=tiffi;
            tiffi=0;
            info=image.width*image.bits1;
            if (image.size==image.width*image.bits1*image.height && image.compression!=0) {
                image.compression=0;   // Uncompressed?
            }
            if (image.bits1==3) {
                type=IMAGE24;
            } else if (image.bits1==4) {
                type=IMAGE32;
            } else if (image.bits==1) {
                type=IMAGE1;info/=8;
            } else if (image.bits==4) {
                type=IMAGE4;//??
            } else if (image.bits==8) {
                type=IMAGE8;
            } else type=DEFAULT;
            if (image.compression==6 || image.compression==7) type=JPEG,info=0;
            else if (image.compression==8 && type==IMAGE24) type=ZLIB,info=(IMAGE24<<24)|image.width*3;
            else if (image.compression!=0) type=CMP,info=0;
            if (info) pinfo=" (width: "+ itos(info&0xffffff) +")";
            else pinfo=" "+TIFFCompStr(image.compression);
            //else pinfo="";
            parseCount--;
            if (parseCount==0) rec=false;
            return state;
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

dType TIFFParser::getType(int i) {
    dType t;
    t.start=jstart;   // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;         // pos where start was set in block
    t.type=type;
    t.recursive=rec;
    return t;
}

int TIFFParser::TypeCount() {
    return 1;
}

void TIFFParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=buf0=buf1=buf2=buf3=0;
    tiffMM=false;
    tiffImageStart=tiffImageEnd=0;
    dirEntry=0;tagsIn=0;
    count=0;
    tiffImages=-1;
    info=i=inSize=0;
    for (size_t k=0; k<ifd.size();k++) {
        ifd[k].Tags.clear();
    }
    ifd.clear(); // Found IDF's
    dtf.clear(); // list of images
    parseCount=0;
    tagTx=0;
    tagCx=0;
    rec=false;
    priority=2;
}
void TIFFParser::SetEnd(uint64_t e) {
    jend=e;
}
