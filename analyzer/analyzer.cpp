//#pragma once
#include "analyzer.hpp"

Analyser::Analyser():info(0),remaining(0),typefound(false) {
    AddParser( new DefaultParser());
    AddParser( new BMPParser());
    
}

void Analyser::AddParser(Parser *p) {
    parsers.push_back(p);
}

Analyser::~Analyser() {
}

dType Analyser::Detect(File* in, U64 n, int it) {
    
    //vec.erase(vec.begin());
    const int BLOCK=0x10000;  // block size
    U8 blk[BLOCK];
    Filetype type=DEFAULT;
    bool pri[5]={false};
    if (typefound==true) {
        typefound=false;
        return found;   
    }
    for (size_t j=0;j<parsers.size();j++) {
            parsers[j]->Reset();
    }
    remaining=n;
    while (remaining) {
        size_t reads=min(BLOCK, remaining);
        int ReadIn=in->blockread(&blk[0], reads);
        //
        for (size_t j=0;j<parsers.size();j++) {
            if (parsers[j]->state!=DISABLE) {
                //open type detection file and load into memory
                DetectState dstate=parsers[j]->Parse(&blk[0],BLOCK,n-remaining);
               /* if (dstate==START){
                    //printf("T=%d START\n",j);
                    //request current state data
                    int jst=vmDetect[j]->detect(buf0,REQUEST);
                    vTypes[j].state=START;
                    vTypes[j].start=jst; // save type start pos
                    vTypes[j].rpos=i;    // save relative pos
                }*/
                /*else*/ if (dstate==INFO) {
                    parsers[j]->state=INFO;
                    //parsers[j]->info=vmDetect[j]->detect(buf0,REQUEST);
                    dType t=parsers[j]->getType(0);
                    type=t.type;
                    printf("T=%d INFO %d, %d-%d\n",j,t.info,t.start,t.end);
                    pri[parsers[j]->priority]|=true;   // we have valid header, set priority true
                }
                else if (dstate==END){
                    printf("T=%d END\n",j);
                    // request current state data
                    parsers[j]->state=END;
                    typefound=true;
                   // foundblock=j;
                    //int jst=vmDetect[j]->detect(buf0,REQUEST);
                    //vTypes[j].end=jst-vTypes[j].start; // save type end pos
                }else if (dstate==DISABLE) {
                    printf("T=%d DISABLE\n",j);
                    parsers[j]->state=DISABLE;
                    }
            }
        }
        // Test valid parsers and disable lower priority ones.
        for (size_t i=0; i<3; i++) {
            bool p=pri[i];
            if (p) {
                for (size_t j=0; j<parsers.size(); j++) {
                    if (parsers[j]->priority>i && parsers[j]->state!=DISABLE) {
                        parsers[j]->state=DISABLE;
                        printf("T=%d parser %s DISABLED\n",j,parsers[j]->name.c_str());
                    }
                }
                break;
            }
        }
        // We found new type. Is some parser still detecting?
        if (typefound==true) {
            // Scan for overlaping types and report first enabled type
            for (size_t j=0; j<parsers.size(); j++) {
                if (parsers[j]->state==END) { // partial files?
                    dType d=parsers[0]->getType(0);
                    dType t=parsers[j]->getType(0);
                    found=t;
                    if (d.end>0 && t.start!=0) d.end=t.start,def=d;
                    printf("T=%d parser default\n Start %d,%d\n",0,d.start,d.end);
                    printf("T=%d parser %s\n Start %d,%d\n",j,parsers[j]->name.c_str(),t.start,t.end);
                    if (d.end) 
                        return def;
                    else {
                        typefound=false;
                        return found;
                    }
                }
            }
        }
        //
        remaining-=ReadIn;
    }
    def=parsers[0]->getType(0);
    def.end=n;
    printf("T=%d parser default\n Start %d,%d\n",0,def.start,def.end);
    return def;
   /* U32 buf0=0;  // last 8 bytes
    U64 start= ftell (in);
    info=-1;
    static int foundblock=-1;
    //int dstate=0;
    if (foundblock >-1) {
       // report type and set to default type
       info=vTypes[foundblock].info;
       fseek (in, start+vTypes[foundblock].end, SEEK_SET );
       foundblock=-1;
       return defaultType;
    }
    for (int j=0;j<vTypes.size32();j++){
        if ( vTypes[j].dsize!=-1){
            //reset states            
            vTypes[j].state=NONE;
            vmDetect[j]->detect(0,RESET);
        }
    }
    for (U64 i=0; i<n; ++i) {
        int c=fgetc(in);
        if (c==EOF) return (-1);
        buf0=buf0<<8|c;
        
        for (int j=0;j<vTypes.size32();j++) {
            if (vTypes[j].dsize!=int(-1) && vTypes[j].state!=DISABLE) {
                //open type detection file and load into memory
                int dstate=vmDetect[j]->detect(buf0,i);
                if (dstate==START && type==defaultType){
                    //printf("T=%d START\n",j);
                    //request current state data
                    int jst=vmDetect[j]->detect(buf0,REQUEST);
                    vTypes[j].state=START;
                    vTypes[j].start=jst; // save type start pos
                    vTypes[j].rpos=i;    // save relative pos
                }
                else if (dstate==INFO) {
                    vTypes[j].state=INFO;
                    vTypes[j].info=vmDetect[j]->detect(buf0,REQUEST);
                    //printf("T=%d INFO %d\n",j,vTypes[j].info);
                }
                else if (dstate==END){
                    // printf("T=%d END\n",j);
                    // request current state data
                    vTypes[j].state=END;
                    foundblock=j;
                    int jst=vmDetect[j]->detect(buf0,REQUEST);
                    vTypes[j].end=jst-vTypes[j].start; // save type end pos
                }
            }
        }
        if (foundblock >-1) {
            bool isrecursionType=false;
            // look for active recursive type
            for (int j=0;j<vTypes.size32();j++){
                if (vTypes[j].type<defaultType && vTypes[j].dsize!=-1 && (vTypes[j].state==END)) {
                   isrecursionType=true;
                   foundblock=j;
                   break;
                }
             }
            // search for type that still does detection
            for (int j=0;j<vTypes.size32();j++) {
                if  (isrecursionType==true && vTypes[j].type>defaultType) {
                //disable nonrecursive type
                 if ((vTypes[j].state==START || vTypes[j].state==INFO)) { //return active type
                  // printf("Type %d s=%d e=%d \n",foundblock,vTypes[foundblock].start, vTypes[foundblock].end);
                  // printf("Type %d s=%d e=%d r=%d \n",j,vTypes[j].start, vTypes[j].end , vTypes[j].rpos);
                  if (vTypes[foundblock].start>vTypes[j].rpos) {
                    // if have we real block with good size
                    // reset pos and set non-default type, restart 
                    foundblock=-1;
                    return  fseek(in, start+vTypes[j].start, SEEK_SET), j;
                  }
                 }
                 vTypes[j].state=DISABLE;
                 //   printf("T=%d DISABLE NON-RECURSIVE\n",j);
                }
                else if (vTypes[j].type>defaultType && vTypes[j].dsize!=-1 && 
                        (vTypes[j].state==START || vTypes[j].state==INFO) && (j!=foundblock)) {
                   vTypes[foundblock].state=DISABLE;
                    //printf("T=%d DISABLE TYPE\n",j);
                   foundblock=-1;
                   break;
                }
             }
             
             if (foundblock ==-1) continue;
             // if single full block then report back
             // printf("s=%d e=%d \n",vTypes[foundblock].start, vTypes[foundblock].end);
             return  fseek(in, start+vTypes[foundblock].start, SEEK_SET), foundblock;
        }
    }
    for (int j=0;j<vTypes.size32();j++) {
        if ( vTypes[j].state==START || vTypes[j].state==INFO) {
            foundblock=j;
            vTypes[j].end=n-vTypes[j].start;
            vTypes[j].state=END;            
            //printf("s=%d e=%d \n",vTypes[j].start, vTypes[j].end);
            return  fseek(in, start+vTypes[j].start, SEEK_SET), j;
        }
    }
     
    return type;*/
}
