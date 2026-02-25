#include "program.hpp"


#if defined(WINDOWS)      
HANDLE  hConsole;
#endif
void SetConColor(int color) {
#if defined(WINDOWS)      
    SetConsoleTextAttribute(hConsole, color);
#endif     
}
extern int level;
extern int minfq;
extern int verbose;
extern bool slow;
extern bool witmode;
extern bool doList;
extern bool doExtract;
extern int rdepth;
extern ParserType *userPT;
extern int userPTsize;
extern int topt;
// Print progress: n is the number of bytes compressed or decompressed
void printStatus(U64 n, U64 size,int tid=-1) {
    if (level>0 && tid>=0)  fprintf(stderr,"%2d %6.2f%%\b\b\b\b\b\b\b\b\b\b",tid, float(100)*n/(size+1)), fflush(stdout);
    else if (level>0)  fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}


void compressStream(int sid, U64 size, File* in, File* out, Streams *streams, Segment *segment) {
    U64 segmentsize;
    U64 segmentlen;
    U64 startpos=out->curpos();
    int segmentpos;
    int segmentinfo;
    Filetype segmenttype;
#ifndef NDEBUG 
    U64 scompsize=0;
#endif    
    segmentsize=size;
    segmentpos=0;
    segmentinfo=0;
    segmentlen=0;
    if (level==0) { // Uncompressed
        if ((sid>=0 && sid<=7) || sid==10 ||/*sid==8 || sid==9 ||*/sid==11|| sid==12) {
            while (segmentsize>0) {
                //#ifndef MT
                if (!(segmentsize&0x1fff)) printStatus(size-segmentsize, size, sid);
                //#endif
                out->putc(in->getc());
                segmentsize--;
            }
        }
        if (sid==8 || sid==9) {
            bool dictFail=false;
            FileTmp tm;
            TextFilter textf("text");
            textf.encode(in,&tm,segmentsize,sid==8);
            segmentlen=tm.curpos();
            streams->streams[sid]->streamsize=segmentlen;
            // -e0 option ignores larger wrt size
            if (segmentlen>=segmentsize && minfq!=0) {
                dictFail=true; //wrt size larger
                if (verbose>0) printf(" WRT larger: %d bytes. Ignoring\n",segmentlen-segmentsize); 
            } else {
                if (verbose>0) printf(" Total %0" PRIi64 " wrt: %0" PRIi64 "\n",segmentsize,segmentlen); 
            }
            tm.setpos(0);
            in->setpos(0);

            if (dictFail==true) {
                streams->streams[sid]->streamsize=segmentsize+1;
                segmentlen=segmentsize;
                out->putc(0xAA); //flag
            } else {
                streams->streams[sid]->streamsize=segmentlen+1;
                out->putc(0); //flag
            }
            #ifndef NDEBUG 
            scompsize=out->curpos();
            #endif
            for (U64 k=0; k<segmentlen; ++k) {
                if (!(k&0x1fff)) printStatus(k, segmentlen,sid);
                if (dictFail==false) out->putc(tm.getc());
                else                 out->putc(in->getc());
            }
            tm.close();
            //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,segmentlen+0.0,ftello(out)-scompsize+0.0);
            segmentlen=segmentsize=0;   
        }
    } else { // Compress
        Encoder* enc;
        Predictors* pred;
        // Select predictor for a stream (sid)
        switch(sid) {
        default:
        case 0: { pred=new Predictor(); break;}
        case 1: { pred=new PredictorJPEG(); break;}
        case 2: { pred=new PredictorIMG1(); break;}
        case 3: { pred=new PredictorIMG4(); break;}
        case 4: { pred=new PredictorIMG8(); break;}
        case 5: { pred=new PredictorIMG24(); break;}
        case 6: { pred=new PredictorAUDIO2(); break;}
        case 7: { pred=new PredictorEXE(); break;}
        case 8: 
        case 9: 
        case 10: { pred=new PredictorTXTWRT(); break;}
        case 11: { pred=new PredictorDEC(); break;}
        case 12: { pred=new Predictor(); break;}
        }
        enc=new Encoder (COMPRESS, out,*pred); 
        if ((sid>=0 && sid<=7) || sid==10 ||/*sid==8 || sid==9 ||*/sid==11|| sid==12) {
            while (segmentsize>0) {
                while (segmentlen==0) {
                    segmenttype=(Filetype)segment->operator()(segmentpos++);
                    for (int ii=0; ii<8; ii++) segmentlen<<=8,segmentlen+=segment->operator()(segmentpos++);
                    for (int ii=0; ii<4; ii++) segmentinfo=(segmentinfo<<8)+segment->operator()(segmentpos++);
                    if (!(streams->isStreamType(segmenttype,sid) )) segmentlen=0;
                    enc->predictor.x.filetype=segmenttype;
                    enc->predictor.x.blpos=0;
                    enc->predictor.x.finfo=segmentinfo;
                }
                for (U64 k=0; k<segmentlen; ++k) {
                    //#ifndef MT
                    if (!(segmentsize&0x1fff)) printStatus(size-segmentsize, size, sid);
                    //#endif
                    enc->compress(in->getc());
                    segmentsize--;
                }
                /* #ifndef NDEBUG 
                            printf("Stream(%d) block from %0lu to %0lu bytes\n",i,segmentlen, out->curpos()-scompsize);
                            scompsize= out->curpos();
                            #endif */
                segmentlen=0;
            }
            enc->flush();
        }
        if (sid==8 || sid==9) {
            bool dictFail=false;
            FileTmp tm;
            TextFilter textf("text");
            textf.encode(in,&tm,segmentsize,sid==8);
            segmentlen=tm.curpos();
            streams->streams[sid]->streamsize=segmentlen;
            // -e0 option ignores larger wrt size
            if (segmentlen>=segmentsize && minfq!=0) {
                dictFail=true; //wrt size larger
                if (verbose>0) printf(" WRT larger: %d bytes. Ignoring\n",segmentlen-segmentsize); 
            } else {
                if (verbose>0) printf(" Total %0" PRIi64 " wrt: %0" PRIi64 "\n",segmentsize,segmentlen); 
            }
            tm.setpos(0);
            in->setpos(0);
            enc->predictor.x.filetype=DICTTXT;
            enc->predictor.x.blpos=0;
            enc->predictor.x.finfo=-1;

            if (dictFail==true) {
                streams->streams[sid]->streamsize=segmentsize+1;
                segmentlen=segmentsize;
                enc->compress(0xAA); //flag
            }else {
                streams->streams[sid]->streamsize=segmentlen+1;
                enc->compress(0); //flag
            }
            #ifndef NDEBUG 
            scompsize=out->curpos();
            #endif
            for (U64 k=0; k<segmentlen; ++k) {
                if (!(k&0x1fff)) printStatus(k, segmentlen,sid);
                #ifndef NDEBUG 
                if (!(k&0x3ffff) && k) {
                    if (verbose>0) printf("Stream(%d) block pos %0lu compressed to %0lu bytes\n",sid,k, out->curpos()-scompsize);
                    scompsize= out->curpos();
                }
                #endif
                if (dictFail==false) enc->compress(tm.getc());
                else                 enc->compress(in->getc());
            }
            tm.close();
            enc->flush();
            //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,segmentlen+0.0,ftello(out)-scompsize+0.0);
            segmentlen=segmentsize=0;   
        }
        delete pred;
        delete enc;
    }
    
    printf("Stream(");
    SetConColor(sid+2);
    printf("%d",sid);
    SetConColor(7);

    printf(") compressed from %0" PRIi64 " to ",size);
    SetConColor(9);
    printf("%0" PRIi64 "", out->curpos()-startpos); // Without MT start pos is not zero
    SetConColor(7);
    printf(" bytes\n");
}

#ifdef MT
void compress(const Job& job) {
    compressStream(job.streamid,job.datasegmentsize,job.in,job.out,job.st,job.sg);
}
#endif



void decompressStream(int sid, U64 size, File* in, File* out, Streams *streams, Segment *segment) {
    U64 datasegmentsize;
    U64 datasegmentlen;
    int datasegmentpos;
    int datasegmentinfo;
    Filetype datasegmenttype;

    datasegmentsize=size;
    U64 total=datasegmentsize;
    datasegmentpos=0;
    datasegmentinfo=0;
    datasegmentlen=0;
    printf("Decompressing %-12s stream(%d).\n", streams->streams[sid+1]->name.c_str(),sid);
    Predictors* predictord;
    Encoder *defaultencoder;

    switch(sid) {
    case 0: {
            predictord=new Predictor();     break;}
    case 1: {
            predictord=new PredictorJPEG(); break;}
    case 2: {
            predictord=new PredictorIMG1(); break;}
    case 3: {
            predictord=new PredictorIMG4(); break;}
    case 4: {
            predictord=new PredictorIMG8(); break;}
    case 5: {
            predictord=new PredictorIMG24(); break;}
    case 6: {
            predictord=new PredictorAUDIO2(); break;}
    case 7: {
            predictord=new PredictorEXE();    break;}
    case 8: {
            predictord=new PredictorTXTWRT(); break;}
    case 9:
    case 10: {
            predictord=new PredictorTXTWRT(); break;}
    case 11: {
            predictord=new PredictorDEC(); break;}
    case 12: {
            predictord=new Predictor(); break;}
    }
    defaultencoder=new Encoder (DECOMPRESS, in, *predictord); 
    if ((sid>=0 && sid<=7) /*||i==8 || i==9*/ || sid==10 || sid==11 || sid==12) {
        while (datasegmentsize>0) {
            while (datasegmentlen==0){
                datasegmenttype=(Filetype)segment->operator()(datasegmentpos++);
                for (int ii=0; ii<8; ii++) datasegmentlen=datasegmentlen<<8,datasegmentlen+=segment->operator()(datasegmentpos++);
                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment->operator()(datasegmentpos++);
                if (!(streams->isStreamType(datasegmenttype,sid) )) datasegmentlen=0;
                defaultencoder->predictor.x.filetype=datasegmenttype;
                defaultencoder->predictor.x.blpos=0;
                defaultencoder->predictor.x.finfo=datasegmentinfo;
            }
            for (U64 k=0; k<datasegmentlen; ++k) {
                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total, sid);
                out->putc(defaultencoder->decompress());
                datasegmentsize--;
            }
            datasegmentlen=0;
        }
    } else if (sid==8 || sid==9) {
        while (datasegmentsize>0) {
            FileTmp tm;
            bool doWRT=true;
            datasegmentlen=datasegmentsize;
            defaultencoder->predictor.x.filetype=DICTTXT;
            defaultencoder->predictor.x.blpos=0;
            defaultencoder->predictor.x.finfo=-1;
            for (U64 k=0; k<datasegmentlen; ++k) {
                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total, sid);
                U8 b=defaultencoder->decompress();
                if (k==0) {
                    if (b==0xAA) doWRT=false; // flag set?
                }
                else tm.putc(b);
                datasegmentsize--;
            }
            if (doWRT==true) {
                tm.setpos(0);
                TextFilter textf("text");
                textf.decode(&tm,out,datasegmentlen,0);
            } else {
                tm.setpos(0);
                for ( U64 ii=1; ii<datasegmentlen; ii++) {
                    U8 b=tm.getc(); 
                    out->putc(b);
                }
                tm.close();
            }
            datasegmentlen=datasegmentsize=0;
        }
    }
    delete predictord;
    delete defaultencoder;
}

#ifdef MT
void decompress(const Job& job) {
    decompressStream(job.streamid,job.datasegmentsize,job.in,job.out,job.st,job.sg);
}
#endif


Program::Program(CLI &c, const std::string p):cli(c),mode(COMPRESS),progname(p),
archive(nullptr),
files(0),
streambit(0) {
#if defined(WINDOWS)      
    hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
#endif
};

Program::~Program() {
}

void Program::List() {
    archiveName=cli.files[0];
    mode=DECOMPRESS;

    // Compress: write archive header, get file names and sizes
    std::string header_string;
    std::string filenames;

    // Decompress: open archive for reading and store file names and sizes
    archive= new FileDisk();
    archive->open(archiveName.c_str(),true);
    // Check for proper format and get option
    std::string header;
    int len=strlen(progname.c_str())+1, c, i=0;
    header.resize(len+1);
    while (i<len && (c=archive->getc())!=EOF) {
        header[i]=c;
        i++;
    }
    header[i]=0;
    if (strncmp(header.c_str(), progname.c_str(), strlen(progname.c_str())+1))
    printf("%s: not a %s file\n", archiveName.c_str(), progname.c_str()), quit("");
    level=archive->getc();
    if (level&64) slow=true;
    if (level&128) witmode=true;
    level=level&0xf;
    
    // Read segment data from archive end
    U64 currentpos,datapos=0L;
    for (int i=0; i<8; i++) datapos=datapos<<8,datapos+=archive->getc();
    segment.hpos=datapos;
    U32 segpos=archive->get32();  //read segment data size
    segment.pos=archive->get32(); //read segment data size
    streambit=archive->getc()<<8; //get bitinfo of streams present
    streambit+=archive->getc();
    if (segment.hpos==0 || segment.pos==0) quit("Segment data not found.");
    segment.setsize(segment.pos);
    currentpos= archive->curpos();
    archive->setpos( segment.hpos);
    if (archive->blockread( &segment[0],   segment.pos  )<segment.pos) quit("Segment data corrupted.");
    // Decompress segment data 
    Encoder* segencode;
    Predictors* segpredict;
    FileTmp  tmp;
    tmp.blockwrite(&segment[0],   segment.pos  ); 
    tmp.setpos(0); 
    segpredict=new Predictor();
    segencode=new Encoder (DECOMPRESS, &tmp ,*segpredict);
    segment.pos=0;
    for (U32 k=0; k<segpos; ++k) {
        segment.put1( segencode->decompress());
    }
    delete segpredict;
    delete segencode;
    tmp.close();
    //read stream sizes if stream bit is set
    for (int i=0;i<streams.Count();i++){
        if ((streambit>>(streams.Count()-i))&1){
            streams.streams[i]->streamsize=archive->get64();
            streams.streams[i]->cstreamsize=archive->get64();
        }
    }
    archive->setpos(currentpos); 
    segment.pos=0; //reset to offset 0

    Encoder* en;
    Predictors* predictord;
    predictord=new Predictor();
    en=new Encoder(mode, archive,*predictord);
    
    // Deompress header
    if (en->decompress()!=0) printf("%s: header corrupted\n", archiveName.c_str()), quit("");
    int hdrlen=0;
    hdrlen+=en->decompress()<<24;
    hdrlen+=en->decompress()<<16;
    hdrlen+=en->decompress()<<8;
    hdrlen+=en->decompress();
    header_string.resize(hdrlen);
    for (int i=0; i<hdrlen; i++) {
        header_string[i]=en->decompress();
        if (header_string[i]=='\n') files++;
    }

    printf("File list of %s archive:\n%s", archiveName.c_str(), header_string.c_str());

    archive->close();
}

void Program::Compress() {
    archiveName=cli.files[0];
    archiveName+=".";
    archiveName+=progname;

    // Compress: write archive header, get file names and sizes
    std::string header_string;
    std::string filenames;
    
    segment.setsize(48); //inital segment buffer size (about 277 blocks)
    // Expand filenames to read later.  Write their base names and sizes
    // to archive.
    int i;
    for (i=0; i<cli.files.size(); ++i) {
        std::string name=cli.files[i];
        int len=name.size()-1;
        for (int j=0; j<=len; ++j)  // change \ to /
        if (name[j]=='\\') name[j]='/';
        while (len>0 && name[len-1]=='/')  // remove trailing /
        name[--len]=0;
        int base=len-1;
        while (base>=0 && name[base]!='/') --base;  // find last /
        ++base;
        if (base==0 && len>=2 && name[1]==':') base=2;  // chop "C:"
        int expanded=expand(header_string, filenames, name.c_str(), base);
        if (!expanded/* && (i>1||argc==2)*/)
        printf("%s: not found, skipping...\n", name.c_str());
        files+=expanded;
    }

    // If there is at least one file to compress
    // then create the archive header.
    if (files<1) quit("Nothing to compress\n");
    archive=new FileDisk();
    archive->create(archiveName.c_str());
    archive->append(progname.c_str());
    archive->putc(0);
    archive->putc(level|            ((slow==true)?64:0)|            ((witmode==true)?128:0));
    segment.hpos= archive->curpos();
    
    for (int i=0; i<12+4+2; i++) archive->putc(0); //space for segment size in header +streams info
    
    printf("Creating archive %s with %d file(s)...\n",
    archiveName.c_str(), files);

    Encoder* en;
    Predictors* predictord;
    predictord=new Predictor();
    en=new Encoder(mode, archive,*predictord);
    
    // Compress header
    int len=header_string.size();
    assert(en->getMode()==COMPRESS);
    U64 start=en->size();
    en->compress(0); // block type 0
    en->compress(len>>24); en->compress(len>>16); en->compress(len>>8); en->compress(len); // block length
    for (int i=0; i<len; i++) en->compress(header_string[i]);
    if (verbose){
        printf("File list compressed from %d to %0lu bytes.\n",len,en->size()-start);
    }

    // Fill fname[files], fsize[files] with input filenames and sizes
    fname.resize(files);
    fsize.resize(files);
    char *p=&header_string[0];
    char* q=&filenames[0];
    for (int i=0; i<files; ++i) {
        assert(p);
        fsize[i]=atoll(p);
        assert(fsize[i]>=0);
        while (*p!='\t') ++p; *(p++)='\0';
        fname[i]=mode==COMPRESS?q:p;
        while (*p!='\n') ++p; *(p++)='\0';
        while (*q!='\n') ++q; *(q++)='\0';
    }

    // Compress or decompress files
    assert(fname.size()==files);
    assert(fsize.size()==files);
    U64 total_size=0;  // sum of file sizes
    for (int i=0; i<files; ++i) total_size+=fsize[i];
    en->flush();
    delete en;
    delete predictord;
    Codec codec(FCOMPRESS, &streams, &segment, rdepth, userPTsize!=1?userPT:nullptr);
    if (verbose) printf("\nFiles:\n");
    for (int i=0; i<files; ++i) {
        if (verbose) printf("%d/%d %s (%0" PRIi64 " bytes)\n", i+1, files, fname[i], fsize[i]); 
        codec.EncodeFile(fname[i], fsize[i]);
    }
    segment.put1(0xff); //end marker
    //Display Level statistics
    if (verbose>1) {
        printf("\n Segment data size: %d bytes\n",segment.pos);
        codec.PrintStat(rdepth);
    }

    CompressStreams(archive,streambit);
    
    // Write out segment data
    U64 segmentpos;
    segmentpos= archive->curpos();  //get segment data offset
    archive->setpos( segment.hpos);
    archive->put64(segmentpos);     //write segment data offset
    //compress segment data
    Encoder* segencode;
    Predictors* segpredict;
    FileTmp tmp;                    // temporary encoded file
    segpredict=new Predictor();
    segencode=new Encoder (COMPRESS, &tmp ,*segpredict); 
    for (U64 k=0; k<segment.pos; ++k) {
        segencode->compress(segment[k]);
    }
    segencode->flush();
    delete segpredict;
    delete segencode;
    archive->put32(segment.pos);     // write segment data size
    if (verbose>0) printf(" Segment data compressed from %d",segment.pos);
    segment.pos=tmp.curpos();
    segment.setsize(segment.pos);
    if (verbose>0) printf(" to %d bytes\n ",segment.pos);
    tmp.setpos(0); 
    if (tmp.blockread(&segment[0], segment.pos)<segment.pos) quit("Segment data corrupted.");
    tmp.close();
    archive->put32(segment.pos);      // write  compressed segment data size
    archive->putc(streambit>>8&0xff); // write stream bit info
    archive->putc(streambit&0xff); 
    archive->setpos(segmentpos); 
    archive->blockwrite(&segment[0], segment.pos); //write out segment data
    //write stream size if present
    for (int i=0; i<streams.Count(); i++) {
        if (streams.streams[i]->streamsize>0) {
            archive->put64(streams.streams[i]->streamsize);
            archive->put64(streams.streams[i]->cstreamsize);
        }
    }
    printf("Total %0" PRIi64 " bytes compressed to %0" PRIi64 " bytes.\n", total_size,  archive->curpos()); 

    archive->close();
}

void Program::CompressStreams(File *archive, uint16_t &streambit) {
    for (int i=0; i<streams.Count(); ++i) {
        U64 datasegmentsize;
        datasegmentsize= streams.streams[i]->file.curpos();    //get segment data offset
        streams.streams[i]->streamsize=datasegmentsize;
        streams.streams[i]->file.setpos(0);
        streambit=(streambit+(datasegmentsize>0))<<1; //set stream bit if streamsize >0
        if (datasegmentsize>0){                       //if segment contains data
            if (verbose>0) {
                SetConColor(i+2);
                printf("%-12s", streams.streams[i+1]->name.c_str());
                SetConColor(7);  
                printf("stream(%d).  Total %0" PRIi64 "\n",i,datasegmentsize);
            }
#ifdef MT
            // add streams to job list
            Job job;
            job.out=&streams.streams[i]->out;
            job.in=&streams.streams[i]->file;
            job.streamid=i;
            job.command=0; //0 compress
            job.datasegmentsize=datasegmentsize;
            job.st=&streams;
            job.sg=&segment;
            jobs.push_back(job);
#else
            compressStream(i,datasegmentsize,&streams.streams[i]->file,archive, &streams, &segment);
#endif
        }
    }
#ifdef MT
    run_jobs(jobs, topt);
    const uint64_t BLOCK=4096*4096*2; // 32mb
    Array<uint8_t> blk(BLOCK);
    // Append temporary files to archive if OK.
    for (U32 i=0; i<jobs.size(); ++i) {
        if (jobs[i].state==OK) {
            streams.streams[jobs[i].streamid]->out.setend();
            streams.streams[jobs[i].streamid]->cstreamsize=streams.streams[jobs[i].streamid]->out.curpos();
            streams.streams[jobs[i].streamid]->out.setpos(0);
            
            //append streams to archive
            uint64_t remaining=streams.streams[jobs[i].streamid]->cstreamsize;
            while (remaining) {
                size_t reads=min64(BLOCK, remaining);
                size_t ReadIn=streams.streams[jobs[i].streamid]->out.blockread(&blk[0], reads);
                archive->blockwrite(&blk[0], ReadIn);
                remaining-=ReadIn;
            }
            streams.streams[jobs[i].streamid]->out.close();
        }
    }

#endif

    for (int i=0; i<streams.Count(); ++i) {
        streams.streams[i]->file.close();
    }
}

void Program::Decompress() {
    archiveName=cli.files[0];
    mode=DECOMPRESS;

    // Compress: write archive header, get file names and sizes
    std::string header_string;
    std::string filenames;

    // Decompress: open archive for reading and store file names and sizes
    archive= new FileDisk();
    archive->open(archiveName.c_str(),true);
    // Check for proper format and get option
    std::string header;
    int len=strlen(progname.c_str())+1, c, i=0;
    header.resize(len+1);
    while (i<len && (c=archive->getc())!=EOF) {
        header[i]=c;
        i++;
    }
    header[i]=0;
    if (strncmp(header.c_str(), progname.c_str(), strlen(progname.c_str())+1))
    printf("%s: not a %s file\n", archiveName.c_str(), progname.c_str()), quit("");
    level=archive->getc();
    if (level&64) slow=true;
    if (level&128) witmode=true;
    level=level&0xf;
    
    // Read segment data from archive end
    U64 currentpos,datapos=0L;
    for (int i=0; i<8; i++) datapos=datapos<<8,datapos+=archive->getc();
    segment.hpos=datapos;
    U32 segpos=archive->get32();  //read segment data size
    segment.pos=archive->get32(); //read segment data size
    streambit=archive->getc()<<8; //get bitinfo of streams present
    streambit+=archive->getc();
    if (segment.hpos==0 || segment.pos==0) quit("Segment data not found.");
    segment.setsize(segment.pos);
    currentpos= archive->curpos();
    archive->setpos( segment.hpos);
    if (archive->blockread( &segment[0],   segment.pos  )<segment.pos) quit("Segment data corrupted.");
    // Decompress segment data 
    Encoder* segencode;
    Predictors* segpredict;
    FileTmp  tmp;
    tmp.blockwrite(&segment[0],   segment.pos  ); 
    tmp.setpos(0); 
    segpredict=new Predictor();
    segencode=new Encoder (DECOMPRESS, &tmp ,*segpredict);
    segment.pos=0;
    for (U32 k=0; k<segpos; ++k) {
        segment.put1( segencode->decompress());
    }
    delete segpredict;
    delete segencode;
    tmp.close();
    //read stream sizes if stream bit is set
    for (int i=0;i<streams.Count();i++){
        if ((streambit>>(streams.Count()-i))&1){
            streams.streams[i]->streamsize=archive->get64();
            streams.streams[i]->cstreamsize=archive->get64();
        }
    }
    archive->setpos(currentpos); 
    segment.pos=0; //reset to offset 0

    Encoder* en;
    Predictors* predictord;
    predictord=new Predictor();
    en=new Encoder(mode, archive,*predictord);
    
    // Deompress header
    if (en->decompress()!=0) printf("%s: header corrupted\n", archiveName.c_str()), quit("");
    int hdrlen=0;
    hdrlen+=en->decompress()<<24;
    hdrlen+=en->decompress()<<16;
    hdrlen+=en->decompress()<<8;
    hdrlen+=en->decompress();
    header_string.resize(hdrlen);
    for (int i=0; i<hdrlen; i++) {
        header_string[i]=en->decompress();
        if (header_string[i]=='\n') files++;
    }
    
    // Fill fname[files], fsize[files] with input filenames and sizes
    fname.resize(files);
    fsize.resize(files);
    char *p=&header_string[0];
    char* q=&filenames[0];
    for (int i=0; i<files; ++i) {
        assert(p);
        fsize[i]=atoll(p);
        assert(fsize[i]>=0);
        while (*p!='\t') ++p; *(p++)='\0';
        fname[i]=mode==COMPRESS?q:p;
        while (*p!='\n') ++p; *(p++)='\0';
    }

    // Compress or decompress files
    assert(fname.size()==files);
    assert(fsize.size()==files);
    U64 total_size=0;  // sum of file sizes
    for (int i=0; i<files; ++i) total_size+=fsize[i];

    assert(cli.argcount>=2);
    std::string dir(cli.files.size()==2?cli.files[1]:cli.files[0]);
    if (cli.files.size()==1) {  // chop "/archive.paq8pxd"
        int i;
        for (i=dir.size()-2; i>=0; --i) {
            if (dir[i]=='/' || dir[i]=='\\') {
                dir[i]=0;
                break;
            }
            if (i==1 && dir[i]==':') {  // leave "C:"
                dir[i+1]=0;
                break;
            }
        }
        if (i==-1) dir=".";  // "/" not found
    }
    dir=dir.c_str();
    if (dir[0] && (dir.size()!=3 || dir[1]!=':')) dir+="/";
    
    delete en;
    delete predictord;
    DecompressStreams(archive);
    // Set datastream file pointers to beginning
    for (int i=0; i<streams.Count(); ++i)
    streams.streams[i]->file.setpos(0);
    segment.pos=0;
    Codec codec(FDECOMPRESS, &streams, &segment);
    for (int i=0; i<files; ++i) {
        std::string out(dir.c_str());
        out+=fname[i];
        //printf("Reading file %s\n",out.c_str());
        codec.DecodeFile(out.c_str(), fsize[i]);
    }
    codec.PrintResult();
    int d=segment(segment.pos++);
    if (d!=0xff) printf("Segment end marker not found.\n");
    for (int i=0; i<streams.Count(); ++i) {
        streams.streams[i]->file.close();
    }
    // }
    archive->close();
}

void Program::DecompressStreams(File *archive) {
    printf("Reading streams...\n");
    const uint64_t BLOCK=4096*4096*2; // 32mb
    Array<uint8_t> blk(BLOCK);
    if (level==0) {
        for (int i=0; i<streams.Count(); ++i) {
            uint64_t datasegmentsize=(streams.streams[i]->streamsize); // get segment data offset
            if (datasegmentsize>0) {      
                printf("%-12s stream(%d).\n", streams.streams[i+1]->name.c_str(),i);
                uint64_t remaining=datasegmentsize;
                if ((i>=0 && i<=7) /*||i==8 || i==9*/ || i==10 || i==11 || i==12) {
                    while (remaining) {
                        size_t reads=min64(BLOCK, remaining);
                        uint64_t ReadIn=archive->blockread(&blk[0], reads);
                        streams.streams[i]->file.blockwrite(&blk[0], ReadIn);
                        remaining-=ReadIn;
                    }
                    streams.streams[i]->file.setpos(0);
                } else if (i==8 || i==9) {
                    FileTmp tm;
                    bool doWRT=true;
                    remaining--;
                    U8 b=archive->getc();
                    if (b==0xAA) doWRT=false; // flag set?
                    
                    if (doWRT==true) {
                        while (remaining) {
                            size_t reads=min64(BLOCK, remaining);
                            uint64_t ReadIn=archive->blockread(&blk[0], reads);
                            tm.blockwrite(&blk[0], ReadIn);
                            remaining-=ReadIn;
                        }
                        tm.setpos(0);
                        TextFilter textf("text");
                        textf.decode(&tm,&streams.streams[i]->file,0,0);
                        streams.streams[i]->file.setpos(0);
                    } else {
                        while (remaining) {
                            size_t reads=min64(BLOCK, remaining);
                            uint64_t ReadIn=archive->blockread(&blk[0], reads);
                            streams.streams[i]->file.blockwrite(&blk[0], ReadIn);
                            remaining-=ReadIn;
                        }
                        
                    }
                }
            }
        }
        return;
    } else {
        for (int i=0; i<streams.Count(); ++i) {
            uint64_t datasegmentsize=(streams.streams[i]->streamsize); // get segment data offset
            if (datasegmentsize>0) {              // if segment contains data
                uint64_t remaining=streams.streams[i]->cstreamsize;
                while (remaining) {
                    size_t reads=min64(BLOCK, remaining);
                    uint64_t ReadIn=archive->blockread(&blk[0], reads);
                    streams.streams[i]->out.blockwrite(&blk[0], ReadIn);
                    remaining-=ReadIn;
                }
                streams.streams[i]->out.setpos(0);
                
                #ifdef MT
                // add streams to job list
                Job job;
                job.out=&streams.streams[i]->file;
                job.in=&streams.streams[i]->out;
                job.streamid=i;
                job.command=1; //1 decompress
                job.datasegmentsize=datasegmentsize;
                job.st=&streams;
                job.sg=&segment;
                jobs.push_back(job);
                
#else
                decompressStream(i,datasegmentsize,&streams.streams[i]->out, &streams.streams[i]->file, &streams, &segment);
#endif
            }
        }
    }
    run_jobs(jobs, topt);
}

