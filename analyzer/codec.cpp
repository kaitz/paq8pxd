#include "codec.hpp"
#include "../filters/pngfilter.hpp"

extern int verbose;
extern bool witmode; //-w
extern int itcount;
extern U64 typenamess[datatypecount][6];
extern U32 typenamesc[datatypecount][6];
extern void SetConColor(int color);

Codec::Codec(FMode m, Streams *s, Segment *g):mode(m),streams(s),segment(g) {
    AddFilter( new Img24Filter( std::string("image 24bit"),IMAGE24));
    AddFilter( new Img32Filter( std::string("image 32bit"),IMAGE32));
    AddFilter( new ImgMRBFilter(std::string("image mrb"),MRBR));
    AddFilter( new ImgMRBFilter(std::string("image mrb4"),MRBR4));
    AddFilter( new ExeFilter(std::string("exe"),EXE));
    AddFilter( new TextFilter(std::string("text"),TEXT));
    AddFilter( new EOLFilter(std::string("eol"),EOLTEXT));
    AddFilter( new MDFFilter(std::string("mdf"),MDF));
    AddFilter( new CDFilter(std::string("cd"),CD));
    AddFilter( new gifFilter(std::string("gif"),GIF));
    AddFilter( new szddFilter(std::string("szdd"),SZDD));
    AddFilter( new base64Filter(std::string("base64"),BASE64));
    AddFilter( new witFilter(std::string("wit"),WIT));
    AddFilter( new uudFilter(std::string("uuencode"),UUENC));
    AddFilter( new preflateFilter(std::string("preflate"),PREFLATE));
    AddFilter( new zlibFilter(std::string("zlib"),ZLIB));
    AddFilter( new preflateFilter(std::string("zip"),ZIP));
    AddFilter( new preflateFilter(std::string("gzip"),GZIP));
    AddFilter( new bzip2Filter(std::string("bzip2"),BZIP2));
    AddFilter( new PNGFilter(std::string("png"),PNG24));
    AddFilter( new PNGFilter(std::string("png"),PNG32));
    AddFilter( new PNGFilter(std::string("png"),PNG8));
    AddFilter( new PNGFilter(std::string("png"),PNG8GRAY));
    AddFilter( new DecAFilter(std::string("dec alpha"),DECA));
    AddFilter( new rleFilter(std::string("rle tga"),RLE));
    AddFilter( new base85Filter(std::string("base85"),BASE85));
    AddFilter( new armFilter(std::string("arm"),ARM));
    AddFilter( new lzwFilter(std::string("lzw"),LZW));
    AddFilter( new TextFilter(std::string("TXTUTF8"),TXTUTF8));
    AddFilter( new TextFilter(std::string("BIGTEXT"),BIGTEXT));
    AddFilter( new DefaultFilter(std::string("default"),DEFAULT)); // must be last
}

Codec::~Codec() {
}

void Codec::AddFilter(Filter *f) {
    filters.push_back(f);
}

Filter* Codec::GetFilter(Filetype f) {
    for (uint64_t j=0; j<filters.size(); j++) {
        if (filters[j]->Type==f) return filters[j];
    }
    // or else default filter
    return filters[filters.size()-1];
}

void Codec::Status(uint64_t n, uint64_t size) {
    fprintf(stderr,"F%6.2f%%\b\b\b\b\b\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}

void Codec::DecodeFile(const char* filename, uint64_t filesize) {
    FMode mode=FDECOMPRESS;
    assert(filename && filename[0]);
    // Test if output file exists.  If so, then compare.
    FileDisk f;
    bool success=f.open(filename,false);
    if (success) {
        mode=FCOMPARE,printf("Comparing");
    } else {
        // Create file
        f.create(filename);
        mode=FDECOMPRESS, printf("Extracting");
    }
    printf(" %s %0.0f -> \n", filename, filesize+0.0);

    // Decompress/Compare
    U64 r=DecodeFromStream(&f, filesize, mode);
    if (mode==FCOMPARE && !r && f.getc()!=EOF) printf("file is longer\n");
    else if (mode==FCOMPARE && r) printf("differ at %0lu\n",r-1);
    else if (mode==FCOMPARE) printf("identical\n");
    else printf("done   \n");
    f.close();
}

void Codec::EncodeFile(const char* filename, uint64_t filesize) {
    assert(filename && filename[0]);
    FileDisk in;
    in.open(filename,true);
    if (verbose>2) printf("Block segmentation:\n");
    char blstr[32]="";
    std::string fname=filename;
    ParserType  etype=GetTypeFromExt(fname);
    EncodeFileRecursive(&in, filesize,  blstr, 0, DEFAULT, etype);
    in.close();
}

uint64_t Codec::DecodeFromStream(File *out, uint64_t size, FMode mode, int it) {
    Filetype type;
    U64 len,i,diffFound;
    len=i=diffFound=0L;
    int info=-1;
    File *in;
    while (i<size) {
        type=(Filetype)segment->Get();
        for (int k=0; k<8; k++) len=len<<8,len+=segment->Get();
        for (int k=info=0; k<4; ++k) info=(info<<8)+segment->Get();
        Streamtype srid=streams->GetStreamID(type);
        int ti=streams->GetTypeInfo(type);
        if (srid>STR_NONE) in=&streams->GetStreamFile(srid);
        #ifdef VERBOSE  
         printf(" %d  %-9s |%0lu [%0lu]\n",it, typenames[type],len,i );
        #endif
        Filter* dataf=GetFilter(type); 
        if (srid>STR_NONE && srid!=STR_TEXT0 && srid!=STR_TEXT && (ti&TR_TRANSFORM)==TR_TRANSFORM) {
            //printf("Filter: %s\n",dataf->name.c_str());
            diffFound=dataf->CompareFiles(in,out,len,uint64_t(info),mode);
            len=dataf->fsize;
        }  
        // LZW ?
        else if (srid==STR_NONE) {
            FileTmp tmp;
            DecodeFromStream(&tmp, len, FDECOMPRESS, it+1);
            if (mode!=FDISCARD) {
                tmp.setpos(0);
                diffFound=dataf->CompareFiles(&tmp,out,len,uint64_t(info),mode);
                len=dataf->fsize; // get decoded size
            }
            tmp.close();
        }
        else {
            for (U64 j=i; j<i+len; ++j) {
                //if (!(j&0x1ffff)) printStatus(j, s2);
                if (mode==FDECOMPRESS) out->putc(in->getc());
                else if (mode==FCOMPARE) {
                    int a=out->getc();
                    int b=in->getc();
                    if (a!=b && !diffFound) {
                        mode=FDISCARD;
                        diffFound=j+1;
                    }
                } else in->getc();
            }
        }
        i+=len;
    }
    return diffFound;
}

void Codec::EncodeFileRecursive(File*in, uint64_t n, char *blstr, int it, Filetype ptype, ParserType etype) {
    Filetype type=DEFAULT;
    int blnum=0;
    uint32_t info,info2;  // image width or audio type
    uint64_t begin=in->curpos(), end0=n;
    uint64_t end=0;
    uint64_t len;
    char b2[32];
    strcpy(b2, blstr);
    if (b2[0]) strcat(b2, "-");
    if (it==5) {
        direct_encode_blockstream(DEFAULT, in, n);
        return;
    }
    FileTmp* tmp=new FileTmp(64 * 1024 * 1024/2);
    Analyzer *an=new Analyzer(it, ptype, etype);
    bool found=false;
    // Transform and test in blocks
    while (n>0) {
        if (found==false){
            found=an->Detect(in,n,it);
        }
        dType block=an->GetNext();
        if (block.end==0) {
            found=false;
        }
        end=block.end;
        info=(block.info)&0xffffffff;
        info2=(block.info>>32);
        len=uint64_t(block.end-block.start);
        if (len>n) len=n;
        if (len>0) {
            type=block.type;
            in->setpos(begin);
            if (it>itcount)    itcount=it;
            typenamess[type][it]+=len,  typenamesc[type][it]++; 
            sprintf(blstr,"%s%d",b2,blnum++);
            if (verbose>2) printf(" %-16s |",blstr);
            int streamcolor=streams->GetStreamID(type)+1+1;
            if (streamcolor<1) streamcolor=7;
            SetConColor(streamcolor);
            if (verbose>2) printf(" %-9s ",typenames[type]);
            SetConColor(7);
            if (verbose>2) {
                printf("|%12.0f [%0.0f - %0.0f]%s \n",len+0.0,begin+0.0,(begin+len-1)+0.0,block.pinfo.c_str());
            }
            transform_encode_block(type, in, len, info,info2, blstr, it, begin,tmp);
            n-=len;

            begin+=len;
            assert(begin==in->curpos());
            Status(begin,end0);
        }
    }
    delete an;
    tmp->close();
}

void Codec::direct_encode_blockstream(Filetype type, File*in, U64 len, int info) {
    assert(itcount<6);
    segment->putdata(type,len,info);
    int srid=streams->GetStreamID(type);
    Stream *sout=streams->streams[srid];
    for (U64 j=0; j<len; ++j) sout->file.putc(in->getc());
}

void Codec::transform_encode_block(Filetype type, File*in, U64 len, int info, int info2, char *blstr, int it, U64 begin,File*tmp) {
    tmp->truncate();
    assert(it<5);
    if (streams->GetTypeInfo(type)&TR_TRANSFORM) {
        U64 diffFound=0;
        U32 winfo=0;
        Filter* dataf=GetFilter(type);
        if (type==IMAGE24) dataf->encode(in, tmp, len, uint64_t(info));
        else if (type==IMAGE32) dataf->encode(in, tmp, len, uint64_t(info));
        else if (type==MRBR) dataf->encode(in, tmp, len, uint64_t(info)+(uint64_t(info2)<<32));
        else if (type==MRBR4) dataf->encode(in, tmp, len, uint64_t(((info*4+15)/16)*2)+(uint64_t(info2)<<32));
        else if (type==RLE) dataf->encode(in, tmp, len, info);
        else if (type==LZW) dataf->encode(in, tmp, len, info);
        else if (type==EXE) dataf->encode(in, tmp, len, begin);
        else if (type==DECA) dataf->encode(in, tmp, len, 0);
        else if (type==ARM) dataf->encode(in, tmp, len, 0);
        else if (type==TEXT || type==TXTUTF8 || type==TEXT0 /*|| type==ISOTEXT*/) {
            if (type!=TXTUTF8) {
                dataf->encode(in, tmp, len,1);
                U64 txt0Size= tmp->curpos();
                //reset to text mode
                in->setpos(begin);
                tmp->truncate();
                dataf->encode(in, tmp, (len),0);
                U64 txtSize= tmp->curpos();
                in->setpos( begin);
                tmp->truncate();
                if (txt0Size<txtSize && (((txt0Size*100)/txtSize)<95)) {
                    in->setpos( begin);
                    dataf->encode(in, tmp, (len),1);
                    type=TEXT0,info=1;
                }else{
                    dataf->encode(in, tmp, (len),0);
                    type=TEXT,info=0;
                }
            }
            else dataf->encode(in, tmp, (len),info&1); 
        } else if (type==EOLTEXT) {
            dataf->encode(in, tmp, len,info&1);
            if (dataf->diffFound) {
                // if EOL size is below 25 then drop EOL transform and try TEXT type
                in->setpos(begin);
                type=TEXT;
                dataf=GetFilter(type);
                tmp->truncate();
                dataf->encode(in, tmp, len,info&1); 
            }
        } else if (type==BASE64) dataf->encode(in, tmp, len,0);
        else if (type==UUENC) dataf->encode(in, tmp, len,info);
        else if (type==BASE85) dataf->encode(in, tmp, len,info);
        else if (type==SZDD) {
            dataf->encode(in, tmp,0, info);
        } else if (type==ZLIB) {
            dataf->encode(in, tmp, len,0);   
            diffFound=dataf->diffFound;
        } else if (type==ZIP) {
            dataf->encode(in, tmp, len,0);   
            diffFound=dataf->diffFound;
        } else if (type==GZIP) {
            dataf->encode(in, tmp, len,0);   
            diffFound=dataf->diffFound;
        } else if (type==PREFLATE) {
            dataf->encode(in, tmp, len,0);   
            diffFound=dataf->diffFound;
        } else if (type==PNG24 || type==PNG32 || type==PNG8 || type==PNG8GRAY) {
            dataf->encode(in, tmp, len, 0);
            diffFound=dataf->diffFound;
        } else if (type==BZIP2){
            dataf->encode(in, tmp, len,info=info+256*17);
        } else if (type==CD) dataf->encode(in, tmp, (len), info);
        else if (type==MDF) {
            dataf->encode(in, tmp, len,0);
        } else if (type==GIF)  {
            dataf->encode(in, tmp, len,0);
            diffFound=dataf->diffFound;
        } else if (type==WIT) {
            dataf->encode(in, tmp, len,0);
            winfo=dataf->fsize;
        }
        
        const U64 tmpsize= tmp->curpos();
        int tfail=0;
        tmp->setpos(0);
        
        if (type==BZIP2 || type==CD || type==MDF|| type==SZDD || type==GIF || type==MRBR|| type==MRBR4|| type==RLE|| type==LZW||type==BASE85 ||
                type==BASE64 || type==UUENC|| type==DECA|| type==ARM || (type==WIT||type==TEXT || type==TXTUTF8 ||type==TEXT0)||type==EOLTEXT ){
            
            in->setpos(begin);
            if (type==BASE64 ) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==UUENC ) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==MDF ) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==BASE85 ) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==BZIP2  )     {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info=info+256*17), FCOMPARE);
            } else if (type==GIF && !diffFound) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==MRBR || type==MRBR4) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            } else if (type==RLE)           {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
           }  else if (type==CD)           {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==LZW) {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==DECA) {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==ARM) {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if (type==SZDD) {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if ((type==TEXT || (type==TXTUTF8 &&witmode==false) ||type==TEXT0) ) {
                diffFound=dataf->CompareFiles(tmp,in,tmpsize,uint64_t(info),FCOMPARE);
            } else if ((type==WIT) ) {
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(winfo), FCOMPARE);
            }
            else if ((type==TXTUTF8 &&witmode==true) ) tmp->setend(); //skips 2* input size reading from a file
            else if (type==EOLTEXT ){
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            }
            if (type==EOLTEXT && (diffFound )) {
                // if fail fall back to text
                diffFound=0,info=-1, in->setpos(begin),type=TEXT;
                dataf=GetFilter(type),
                tmp->truncate();
                dataf->encode(in, tmp, len,0); 
            }  else if (type==BZIP2 && (diffFound) ) {
                // maxLen was changed from 20 to 17 in bzip2-1.0.3 so try 20
                in->setpos(begin);
                tmp->setpos(0);
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info=(info&255)+256*20), FCOMPARE);
            }  /*else if (type==ZLIB && (diffFound) ) {
                type=PREFLATE;
                dataf=GetFilter(type);
                in->setpos(begin);
                tmp->setpos(0);
                diffFound=dataf->CompareFiles(tmp,in, tmpsize, uint64_t(info), FCOMPARE);
            }    */  
            tfail=(diffFound || tmp->getc()!=EOF); 
        }
        // Preflate types (ZLIB, PREFLATE, ZIP, GZIP, PNG) skip verification but check if encoding failed
        if ((type==ZLIB || type==PREFLATE || type==ZIP || type==GZIP || type==PNG24 || type==PNG32 || type==PNG8 || type==PNG8GRAY) && diffFound) {
            tfail = 1;
        }
        // Test fails, compress without transform
        if (tfail) {
            if (verbose>2) printf("(Transform fails at %0lu)\n", diffFound-1);
            in->setpos(begin);
            Filetype type2=DEFAULT;
            if (type==ZLIB || type==BZIP2) type2=CMP;
            
            direct_encode_blockstream(type2, in, len);
            typenamess[type][it]-=len,  typenamesc[type][it]--;       // if type fails set
            typenamess[type2][it]+=len,  typenamesc[type2][it]++; // default info
        } else {
            tmp->setpos(0);
            if (type==EXE) {
                direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==DECA || type==ARM) {
                direct_encode_blockstream(type, tmp, tmpsize);
            } else if (type==IMAGE24 || type==IMAGE32) {
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if (type==MRBR || type==MRBR4) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=( tmp->getc()<<8)+(tmp->getc());
                Filetype type2 =type==MRBR?IMAGE8:IMAGE4;
                hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==PNG24 || type==PNG32 || type==PNG8 || type==PNG8GRAY) {
                segment->putdata(type,tmpsize,0);
                Filetype type2 =(Filetype)(info>>24);
                type2=type2==IMAGE24?IMPNG24:type2==IMAGE32?IMPNG32:type2;
                if (it==itcount)    itcount=it+1;
                int hdrsize=dataf->hdrsize;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR,  tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==RLE) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=( 4);
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info);
            } else if (type==LZW) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=0;
                Filetype type2 =(Filetype)(info>>24);
                tmp->setpos(0);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==GIF) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=(tmp->getc()<<8)+tmp->getc();
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize);
                typenamess[info>>24][it+1]+=tmpsize-hdrsize,  typenamesc[IMAGE8][it+1]++;
                direct_encode_blockstream((Filetype)(info>>24), tmp, tmpsize-hdrsize, info&0xffffff);
            } else if (type==AUDIO) {
                segment->putdata(type,len,info2); //original lenght
                direct_encode_blockstream(type, tmp, tmpsize, info);
            } else if (type==TEXT || type==TXTUTF8 ||type==TEXT0) {
                if ( len>0xA00000) { //if WRT is smaller then original block 
                    if (tmpsize>(len-256) ) {
                        in->setpos( begin);
                        direct_encode_blockstream(NOWRT, in, len); }
                    else
                    direct_encode_blockstream(BIGTEXT, tmp, tmpsize);
                } else if (tmpsize< (len*2-len/2)||len) {
                    // encode as text without wrt transoform, 
                    // this will be done when stream is compressed
                    in->setpos( begin);
                    direct_encode_blockstream(type, in, len);
                } else {
                    // wrt size was bigger, encode as NOWRT and put in bigtext stream.
                    in->setpos(begin);
                    direct_encode_blockstream(NOWRT, in, len);
                }
            }else if (type==EOLTEXT) {
                segment->putdata(type,tmpsize,0);
                int hdrsize=tmp->get32();
                hdrsize=tmp->get32();
                hdrsize=hdrsize+12;
                tmp->setpos(0);
                typenamess[CMP][it+1]+=hdrsize,  typenamesc[CMP][it+1]++; 
                direct_encode_blockstream(CMP, tmp, hdrsize);
                typenamess[TEXT][it+1]+=tmpsize-hdrsize,  typenamesc[TEXT][it+1]++;
                FileTmp* treb=new FileTmp(64 * 1024 * 1024/2);
                transform_encode_block(TEXT,  tmp, tmpsize-hdrsize,  -1,-1, blstr, it, hdrsize,treb);
                treb->close();
            } else if (streams->GetTypeInfo(type)&TR_RECURSIVE) {
                int isinfo=0;
                if (type==SZDD || type==ZLIB/*||  type==PREFLATE */ || type==BZIP2) isinfo=info;
                else if (type==WIT) isinfo=winfo;
                segment->putdata(type,tmpsize,isinfo);
                if (type==ZLIB/*||  type==PREFLATE*/) {// PDF or PNG image && info
                    Filetype type2=(Filetype)(info>>24);
                    if (it==itcount) itcount=it+1;
                    int hdrsize=7+5*tmp->getc();
                    tmp->setpos(0);
                    typenamess[HDR][it+1]+=hdrsize, typenamesc[HDR][it+1]++; 
                    direct_encode_blockstream(HDR, tmp, hdrsize);
                    if (info) {
                        typenamess[type2][it+1]+=tmpsize-hdrsize, typenamesc[type2][it+1]++;
                        FileTmp* treb=new FileTmp(64 * 1024 * 1024/2);
                        transform_encode_block(type2, tmp, tmpsize-hdrsize, info&0xffffff,-1, blstr, it, hdrsize, treb);
                        treb->close();
                    } else {
                        EncodeFileRecursive(tmp, tmpsize-hdrsize, blstr, it+1, ZLIB);
                    }
                } else if (type==GZIP || type==ZIP) { 
                    if (it==itcount) itcount=it+1;
                    int hdrsize=4+tmp->get32(); // recon_info
                    tmp->setpos(0);
                    typenamess[HDR][it+1]+=hdrsize, typenamesc[HDR][it+1]++; 
                    direct_encode_blockstream(HDR, tmp, hdrsize);
                    EncodeFileRecursive(tmp, tmpsize-hdrsize, blstr, it+1, type);
                } else {
                    EncodeFileRecursive(tmp, tmpsize, blstr, it+1, type);
                    return;
                }    
            }
        }
    } else {
        // Fo recursion, copy content to tmp so parsers have the start offset 0, no transform.
        // We need to be careful, heavy recursion creates a large number of memory allocations.
        /*if (it>=4) {
            direct_encode_blockstream(DEFAULT, in, len);
            printf("Rec limit\n");
        } else*/ if (type==RECE) {
            FileTmp *treb=new FileTmp(len);
            Filter *dataf=GetFilter(DEFAULT);
            treb->setpos(0);
            dataf->encode(in, treb, len, uint64_t(0));
            treb->setpos(0);
            EncodeFileRecursive(treb, len, blstr, it+1, type, static_cast<ParserType>(info));
            treb->close();
        } else {
            const int i1=(streams->GetTypeInfo(type)&TR_INFO)?info:-1;
            direct_encode_blockstream(type, in, len, i1);
        }
    }
    
}
