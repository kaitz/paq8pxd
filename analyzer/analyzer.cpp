//#pragma once
#include "analyzer.hpp"

// Parser types for virtual default group
// P_DECA not included
static ParserType vdefPT[P_LAST]={
    P_DEF,P_BMP,P_TXT, P_MRB, P_EXE, P_NES, P_MZIP,P_JPG, P_WAV,
    P_PNM, P_PLZW, P_GIF, P_DBS, P_AIFF, P_A85, P_B641, P_B642,
    P_MOD, P_SGI, P_TGA, P_ICD, P_MDF, P_UUE, P_TIFF, P_TAR, P_PNG,
    P_ZIP, P_GZIP, P_BZIP2, P_SZDD, P_MSCF, P_ZLIB, P_ZLIBP,P_ISO9960,P_ISCAB,P_PBIT,
    P_LAST
};

// Example: virtual file parser
// P_EXE is headerless parser,
// to add parser for .exe, .dll, .drv new value is needed (P_WEXE)
// This parser (ParserType) may contain EXE, IMAGE(n), AUDIO, etc types (Filetype)
// If SelectParser does not have this parser then it is ignored in case of Filetype=DEFAULT & ParserType=P_WDEFAULT
// So we can add our special case for it in GetTypeFromExt.
// If file needs more then one parser (excluding P_DEF) then it needs to be virtual.

Analyzer::Analyzer(int it, Filetype p, ParserType eparser, ParserType *vup):
  info(0),remaining(0),typefound(false),lastType(0),iter(it),ptype(p),BLOCK(0x10000),blk(BLOCK),vusrPT(vup),isUserPL(false) {
    assert(eparser<P_WLAST && eparser!=P_LAST && eparser>P_DEF);
    assert(p<TYPELAST);
    // Test for user defined parser list and use only that.
    if (vusrPT!=nullptr) {
        //printf("Analyzer: User defined parser list.\n");
        isUserPL=true;
        assert(vusrPT[0]==P_DEF);
    }
    if (isUserPL && p==DEFAULT && eparser==P_WDEFAULT) {
        for (int selp=P_DEF; static_cast<ParserType>(vusrPT[selp])!=P_LAST; selp++) {
            SelectParser(static_cast<ParserType>(vusrPT[selp]));
        }
    }
    // By default add all virtual defaul group parsers, also slowest detection
    else if (p==DEFAULT && eparser==P_WDEFAULT) {
        for (int selp=P_DEF; static_cast<ParserType>(vdefPT[selp])!=P_LAST; selp++) {
            SelectParser(static_cast<ParserType>(vdefPT[selp]));
        }
    // Custom definations for parent types
    } else if (p==MDF) {         // Parent based parser selection mdf->cd
        SelectParser(P_DEF);     // Audio?
        SelectParser(P_ICD);
    /*} else if (p==ZLIB) {        // Parent based parser selection (pdf ->) zlib -> bitmap image
        SelectParser(P_DEF);
        SelectParser(P_PBIT);    // eparser needs to be P_PDF, for now it is P_DEF
    */
   /* } else if (p==CD) {         //  cd->iso9960 - this will not work, iso file is split into multiple cd parts
        SelectParser(P_DEF);  
        SelectParser(P_ISO9960);*/
    // Custom definations for virtual files
    } else if (eparser==P_WEXE) { // Virtual file parser for exe drv dll ocx so
        SelectParser(P_DEF);
        SelectParser(P_EXE);
        SelectParser(P_BMP);
        SelectParser(P_DECA);    // this parser only in virtual file
        SelectParser(P_WAV);     // is something missing? P_TXT, P_GZIP,...
        SelectParser(P_MSCF);
        SelectParser(P_PNG);
        SelectParser(P_GZIP);
        //SelectParser(P_ZIP);
        //SelectParser(P_ZLIB);
        SelectParser(P_ZLIBP);
    } else if (eparser==P_WPDF) { // Virtual file parser for pdf
        SelectParser(P_DEF);
        SelectParser(P_TXT);
        SelectParser(P_A85);   // needs to be after text
        SelectParser(P_JPG);
        SelectParser(P_PLZW);
        SelectParser(P_ZLIB);
    } else if (eparser==P_WCAB) { // Virtual file parser for cab
        SelectParser(P_DEF);
        SelectParser(P_MSCF);
        SelectParser(P_MZIP);
        SelectParser(P_ISCAB);
    // This needs to be last
    } else if (eparser!=P_DEF && eparser!=P_WDEFAULT) {
        // We have extension based parser, use only that
        SelectParser(P_DEF);
        SelectParser(eparser);
    } else {                     // same as DEFAULT && P_WDEFAULT
        for (int selp=P_DEF; static_cast<ParserType>(vdefPT[selp])!=P_LAST; selp++) {
            SelectParser(static_cast<ParserType>(vdefPT[selp]));
        }
    }

    emptyType.start=0;
    emptyType.end=0;
    emptyType.info=0;
    emptyType.rpos=0;
    emptyType.type=DEFAULT;
    emptyType.recursive=false;
}

void Analyzer::SelectParser(ParserType p) {
    assert(p<P_WLAST);
    if (isUserPL) {
        bool isParserDefined=false;
        for (int selp=P_DEF; static_cast<ParserType>(vusrPT[selp])!=P_LAST; selp++) {
            if (static_cast<ParserType>(vusrPT[selp])==p) {
                isParserDefined=true;
                break;
            }
        }
        if (isParserDefined==false) return;
        //printf("User parser enabled: %d\n",p);
    }
    switch (p) {
    case P_DEF:
        AddParser( new DefaultParser());
        return;
    case P_BMP:
        AddParser( new BMPParser());
        return;
    case P_TXT:
        AddParser( new TextParser());
        return;
    case P_DECA:
        AddParser( new DECaParser());
        return;
    case P_MRB:
        AddParser( new mrbParser());
        return;
    case P_EXE:
        AddParser( new EXEParser());
        return;
    case P_NES:
        AddParser( new NesParser());
        return;
    case P_MZIP:
        AddParser( new MSZIPParser());
        return;
    case P_JPG:
        AddParser( new JPEGParser());
        return;
    case P_WAV:
        AddParser( new WAVParser());
        return;
    case P_PNM:
        AddParser( new PNMParser());
        return;
    case P_PLZW:
        AddParser( new PDFLzwParser());
        return;
    case P_GIF:
        AddParser( new GIFParser());
        return;
    case P_DBS:
        AddParser( new dBaseParser());
        return;
    case P_PBIT:
        AddParser( new pdfBiParser());
        return;
    case P_AIFF:
        AddParser( new AIFFParser());
        return;
    case P_A85:
        AddParser( new ascii85Parser());
        return;
    case P_B641:
        AddParser( new base64_1Parser());  // stream
        return;
    case P_B642:
        AddParser( new base64_2Parser());  // multiline
        return;
    case P_MOD:
        AddParser( new MODParser());
        return;
    case P_SGI:
        AddParser( new sgiParser());
        return;
    case P_TGA:
        AddParser( new TGAParser());
        return;
    case P_ICD:
        AddParser( new cdParser());
        return;
    case P_MDF:
        AddParser( new mdfParser());
        return;
    case P_ISO9960:
        AddParser( new ISO9960Parser());
        return;
    case P_UUE:
        AddParser( new uueParser());
        return;
    case P_TIFF:
        AddParser( new TIFFParser());
        return;
    case P_TAR:
        AddParser( new TARParser());
        return;
    case P_PNG:
        AddParser( new PNGParser());
        return;
    case P_ZIP:
        AddParser( new ZIPParser());
        return;
    case P_GZIP:
        AddParser( new GZIPParser());
        return;
    case P_BZIP2:
        AddParser( new bzip2Parser());
        return;
    case P_SZDD:
        AddParser( new SZDDParser());
        return;
    case P_MSCF:
        AddParser( new MSCFParser());
        return;
    case P_ISCAB:
        AddParser( new ISCABParser());
        return;
    case P_ZLIB:
        AddParser( new zlibParser(false));    // brute=false, high priority
        return;
    case P_ZLIBP:
        AddParser( new zlibParser());   // brute=true, low priority
        return;
    default:
        //printf("Must be virtual file. %d\n",p);
        return;
    }
}

void Analyzer::AddParser(Parser *p) {
    parsers.push_back(p);
}

Analyzer::~Analyzer() {
  while(parsers.size()) {
      delete parsers[parsers.size()-1];
      parsers.pop_back();
  }
}
void Analyzer::Status(uint64_t n, uint64_t size) {
    if (n%0x1000000==0) fprintf(stderr,"P%6.2f%%\b\b\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}
bool Analyzer::Detect(File* in, U64 n, int it) {
    Filetype type=DEFAULT;
    bool pri[MAX_PRI]={false};
    typefound=zeroParser=false;
    zpID=-1;                        // Set zero parser ID to none
    // Reset all parsers exept recursive
    int recP=0;
    for (size_t j=0; j<parsers.size(); j++) {
        dType t=parsers[j]->getType();
        if (t.recursive==false) parsers[j]->Reset(),parsers[j]->state=NONE;
        else recP=j;
        
    }
    if (recP>0)
    for (size_t j=1; j<parsers.size(); j++) {
        if (j!=recP) parsers[j]->state=DISABLE;//,printf("T%d disabled\n",j);
    }
    remaining=n;
    while (remaining) {
        size_t reads=min64(BLOCK, remaining);
        uint64_t ReadIn=in->blockread(&blk[0], reads);
        // Pass the data block to the parsers one by one
        for (size_t j=0; j<parsers.size(); j++) {
            if (parsers[j]->state!=DISABLE) {
                parsers[j]->file_handle = in;  // Give parser access to file for probing. This should not be necessary.
                //printf("T=%d parser %s PARSE\n",j,parsers[j]->name.c_str());
                //open type detection file and load into memory
                DetectState dstate=parsers[j]->Parse(&blk[0],ReadIn,n-remaining,(remaining-ReadIn)==0?true:false);
                if (dstate==INFO) {
                    parsers[j]->state=INFO;
                    dType t=parsers[j]->getType();
                    type=t.type;
                    //if (parsers[j]->priority==0 && zeroParser==false) printf("T=%d INFO %d, %d-%d %s\n",j,t.info,t.start,t.end,parsers[j]->name.c_str());
                    assert(parsers[j]->priority>=0 && parsers[j]->priority<MAX_PRI);
                    pri[parsers[j]->priority]|=true;   // we have valid header, set priority true
                    /*if (parsers[j]->priority==0 && zpID!=-1 && zpID!=j) {
                        printf("T=%d ERROR, parser %s enabled. Parser %s ignored.\n",j,parsers[zpID]->name.c_str(),parsers[j]->name.c_str());
                    }*/
                    if (parsers[j]->priority==0 && zeroParser==false) zeroParser=true,zpID=j;
                    // INFO means "might be something" - don't break, let other parsers check too
                } else if (dstate==END){
                    //if (parsers[j]->priority==0 && zeroParser==false) zeroParser=true,zpID=j;
                    //printf("T=%zu END %s\n",j,parsers[j]->name.c_str());
                    // request current state data
                    parsers[j]->state=END;
                    pri[parsers[j]->priority]|=true;
                    typefound=true; // we should ignore pri=4 in some cases?
                    //break;
                } else if (dstate==DISABLE) {
                    //printf("T=%d DISABLE\n",j);
                    parsers[j]->state=DISABLE;
                } else if (j==zpID && parsers[j]->state==NONE) {
                    // Priority 0 parser fail, re-enable all
                    // What if fail is in the next block and some parser has valid data?
                    //printf("T=%zu PARSER FAIL %s. Enable all.\n",zpID,parsers[j]->name.c_str());
                    for (size_t i=0; i<parsers.size(); i++) {
                        parsers[i]->Reset();
                        parsers[i]->state=NONE;
                    }
                    for (size_t i=0; i<(MAX_PRI-1); i++) pri[i]=false; // Reset all exept default
                    zeroParser=false;
                    zpID=-1;
                }
            }
        }
        // Test valid parsers and disable lower priority ones that are not detected a new type. 
        // Priority MAX_PRI-1 not tested as it is default
        if (typefound==false) {
            for (size_t i=0; i<(MAX_PRI-1); i++) {
                bool p=pri[i];
                if (p) {
                    for (size_t j=0; j<parsers.size(); j++) {
                        if (j!=zpID && parsers[j]->priority>=i && parsers[j]->state!=DISABLE && parsers[j]->state!=END&& parsers[j]->state!=INFO) {
                            //if (parsers[j]->priority!=(MAX_PRI-1)) printf("T=%d parser %s DISABLED\n",j,parsers[j]->name.c_str());
                            dType t=parsers[j]->getType();
                            if (parsers[j]->priority!=(MAX_PRI-1))parsers[j]->state=DISABLE; // ignore default type or lowest priority
                            //printf("T=%d parser %s DISABLED\n",j,parsers[j]->name.c_str());
                        }
                    }
                    /*if (pri[0]) { // Show 0 priority parser
                        for (size_t j=0; j<parsers.size(); j++) {
                            if (parsers[j]->state!=DISABLE) {
                                printf("T=%d parser %s ENABLED pri %d\n",j,parsers[j]->name.c_str(),parsers[j]->priority);
                            }
                        }
                    }*/
                    break;
                }
            }
        }
        // We found new type. Is some parser still detecting?
        if (typefound==true) {
            // Scan for overlaping types and report first detected type
            // TODO: multiplie type detection (default-text-xType)
            int largeP=0;int P=MAX_PRI-1;
            uint64_t minP=-1,maxP=0;
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==INFO || parsers[j]->state==END) { // partial/damaged files?
                    dType t=parsers[j]->getType();
                    // Clip jend to file size if parser set it larger
                    if (t.end > n) t.end = n;
                    //printf("T=%zu parser %s start=%lu end=%lu pri=%d\n",j,parsers[j]->name.c_str(),t.start,t.end,parsers[j]->priority);
                    if (t.end && (t.start<minP || P>parsers[j]->priority)) {
                        if (largeP>0) parsers[largeP]->state=DISABLE;
                        P=parsers[j]->priority;
                       /* if (P!=MAX_PRI-1 && parsers[j]->state==END) {
                            dType p=t;
                            if ((p.end-p.start)!=0 ) {
                                int add=p.end;
                                p.end+=gpos;
                                p.start+=gpos;
                                gpos+=add;
                            printf("SET=%zu %s END %d-%d pri %d\n",j,parsers[j]->name.c_str(),p.start,p.end,P);
                            typesg.push_back(p);
                        }
                            
                        }*/
                        largeP=j;
                        maxP=t.end;
                        minP=t.start;
                        //printf("largeP=%zu set to %s pri %d\n",largeP,parsers[j]->name.c_str(),P);
                
                    }
                }
            }

            /*for (size_t j=0; j<blocks.size(); j++) {
                    dType n=blocks[j];
                    if (n.start<minP) {
                        printf(" P=%d %d-%d %s pri %d R\n",n.pID,n.start,n.end,parsers[n.pID]->name.c_str(),parsers[n.pID]->priority);
                    }
                    //printf("P=%d %d-%d %s pri %d\n",j,t.start,t.end,parsers[t.pID]->name.c_str(),parsers[t.pID]->priority);
            }*/
            /*for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==END) {
                    dType t=parsers[j]->getType();
                    printf("P=%d %d-%d %s\n",j,t.start,t.end,parsers[j]->name.c_str());
                }
            }*/
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==END && j==largeP) { // partial/damaged files?
                    dType d=parsers[0]->getType();
                    dType t=parsers[j]->getType();
                    t.pinfo=parsers[j]->pinfo;
                    d.pinfo=parsers[0]->pinfo;
                    int p=parsers[j]->priority;
                    // Do we have default type? If so add.
                    if (d.end>0 && t.start!=0) {
                        d.end=t.start;
                        if (d.start>d.end) d.start=0;
                        types.push_back(d);
                        //printf("T=%d parser default\n Start %d,%d\n",0,d.start,d.end);
                    } else {
                        d.end=0;
                    }
                    // Look for all parsers still detecting
                    // and limit type end to start of any good priority parser still detecting
                    /*for (size_t j=0; j<parsers.size(); j++) {
                        if (parsers[j]->state==INFO && parsers[j]->priority<=p) {
                            dType det=parsers[j]->getType(0);
                            if (t.end>det.start && det.start) {
                                t.end=det.start;
                                printf("T=%d parser %s LIMITED\n",j,parsers[j]->name.c_str());
                            }
                        }
                    }*/
                    //printf("T=%d parser %s\n Start %d,%d\n",j,parsers[j]->name.c_str(),t.start,t.end);
                    // Clip jend to file size before adding
                    // For GZIP, subtract 8 for the CRC32+ISIZE trailer
                    if (t.type == GZIP && t.end > n - 8) t.end = n - 8;
                    else if (t.end > n) t.end = n;
                    types.push_back(t);
                    return true;
                }
            }
        }
        //
        remaining-=ReadIn;
        
        Status(n-remaining,n);
    }
    // Nothing found so add default type
    dType def=parsers[0]->getType();
    def.pinfo=parsers[0]->pinfo;
    def.end=n;
    types.push_back(def);
    //printf("T=%d parser default\n Start %d,%d\n",0,def.start,def.end);
    return true;
}

dType Analyzer::GetNext() {
    if (types.size()>lastType) {
        dType currentType=types[lastType];
        lastType++;
        return currentType;
    } else {
        lastType=0;
        types.clear();
        return emptyType;
    }
}

std::string &Analyzer::GetInfo() {
    return *pinfo;
}
