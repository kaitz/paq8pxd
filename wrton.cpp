
// based on  "XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com"
//
#include <stdlib.h> 
#include <memory.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <string.h>


#define PRINT_CHARS(data) ;//printf data
#define PRINT_CODEWORDS(data); // printf data
#define PRINT_STACK(data) ;//printf data;
#define PRINT_DICT(data) ;//printf data;
#define PRINT_CONTAINERS(data) ;//printf data
//#define PRINT_STATUS(data) printf data;


#define CHAR_FIRSTUPPER     1   // for encode lower word with first capital letter
#define CHAR_UPPERWORD      2   // for encode upper word
#define CHAR_ESCAPE         3   // for encode reserved chars (CHAR_ESCAPE,CHAR_FIRSTUPPER,...)
#define CHAR_UTFUPPER       4
#define CHAR_EOL            5   // for enocde linefeed in EOLencoder not in wrt
#define BINARY_FIRST        128
#define BINARY_LAST         255


#define WORD_AVG_SIZE       8
#define WORD_MAX_SIZE       48
#define STRING_MAX_SIZE     255  // 1-byte for container.size()


#define HASH_TABLE_SIZE         (1<<20) //1MB*4
#define HASH_DOUBLE_MULT    37
#define HASH_MULT           23

enum ELetterType { LOWERCHAR, UPPERCHAR, UNKNOWNCHAR, RESERVEDCHAR, NUMBERCHAR };

class wrtDecoder
{
public:
    wrtDecoder(); 
    ~wrtDecoder();
    int WRT_start_decoding(int count,int size,U8* data);
    int WRT_decode(int WRTd_c,U8 *decodedText,int *txtlen);
    void defaultSettings(int n);
protected:
    
    inline void stringHash(const U8 *ptr, int len,int& hash);
    int addWord(U8* &mem,int &i);
    U8* loadDynamicDictionary(U8* mem,U8* mem_end);
    
    void initializeCodeWords(int word_count,bool initMem=true);
    bool initialize(bool encoding);
    void WRT_deinitialize();

    int* word_hash;
    bool fileCorrupted;


    std::vector<std::string> sortedDict;

    int sizeDict;
    U8* dictmem;
    U8* dictmem_end;
    U8* mem;
    
U8** dict=NULL;
U8* dictlen=NULL;
    int addSymbols[256]; // reserved symbols in output alphabet 
    int reservedSet[256]; // reserved symbols in input alphabet
    int outputSet[256];
    int codeword2sym[256]; 

    int dictionary,dict1size,dict2size,dict3size,dict4size,dict1plus2plus3,dict1plus2;
    int bound4,bound3,dict123size,dict12size,collision,quoteOpen,quoteClose,detectedSym;
    int maxMemSize;
    int sortedDictSize;
    //int wrtnum;
    int dindex;
    void read_dict(int count,int size,U8* data);
    inline int decodeCodeWord(U8* &s,int& c);
    int s_size;
int more;//=-1
public:
};
   
wrtDecoder::wrtDecoder( ) :  dictmem(NULL), dictmem_end(NULL),fileCorrupted(false),dindex(0), /*wrtnum(0),*/s_size(0),sizeDict(0),sortedDictSize(0),more(-1)
{ 
  dictionary=dict1size=dict2size=dict3size=dict4size=dict1plus2plus3=dict1plus2=0;
     bound4=bound3=dict123size=dict12size=collision=quoteOpen=quoteClose=detectedSym=0;
     memset(&addSymbols[0],0,256*sizeof(int));
      memset(&reservedSet[0],0,256*sizeof(int));
       memset(&outputSet[0],0,256*sizeof(int));
        memset(&codeword2sym[0],0,256*sizeof(int));
    maxMemSize=8*1024*1024;
    word_hash=new int[HASH_TABLE_SIZE];
    //word_hash=(int*)calloc(sizeof(int*)*(HASH_TABLE_SIZE+1),1);
    if (!word_hash)
    quit("Not enough memory!\n");
}
wrtDecoder::~wrtDecoder(){
    if (word_hash)
    delete(word_hash);
    WRT_deinitialize(); 
    //free(word_hash);
}

// make hash from string
inline void wrtDecoder::stringHash(const U8 *ptr, int len,int& hash){
    for (hash = 0; len>0; len--, ptr++){
        hash *= HASH_MULT;
        hash += *ptr;
    }
    hash=hash&(HASH_TABLE_SIZE-1);
}
int wrtDecoder::addWord(U8* &mem,int &i){
    int c,j;
    //if (i<=1 || sizeDict>=dictionary)
    //return -1;
    
    dictlen[sizeDict]=i;
    dict[sizeDict]=mem;
    
    mem[i]=0;
    sizeDict++;
    stringHash(mem,i,j);
    return 1;
    if (word_hash[j]!=0) {
        if (dictlen[sizeDict]!=dictlen[word_hash[j]] || memcmp(dict[sizeDict],dict[word_hash[j]],dictlen[sizeDict])!=0){
            c=(j+i*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
            if (word_hash[c]!=0)            {
                if (dictlen[sizeDict]!=dictlen[word_hash[c]] || memcmp(dict[sizeDict],dict[word_hash[c]],dictlen[sizeDict])!=0){
                    c=(j+i*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
                    if (word_hash[c]!=0) {
                        collision++;
                        return -1;
                    } else  {
                        word_hash[c]=sizeDict++;
                    }
                } else
                return -1; // word already exists
            } else  {
                word_hash[c]=sizeDict++;
            }
        } else
        return -1; // word already exists
    } else  {
        word_hash[j]=sizeDict++;
    }
    return 1;
}
U8* wrtDecoder::loadDynamicDictionary(U8* mem,U8* mem_end){
    int i;
    int count=sortedDictSize;
    for (i=0; i<count; i++) {
        int len=(int)sortedDict[i].size();
        //printf("%s ",sortedDict[i].c_str());
        memcpy(mem,sortedDict[i].c_str(),len+1);
        if (addWord(mem,len)==0)
        break;
        mem+=(len/4+1)*4;
        if (mem>mem_end)
        break;
    }
    
    PRINT_DICT(("count=%d sortedDict.size()=%d\n",count,sortedDictSize));
    return mem;
}

void wrtDecoder::initializeCodeWords(int word_count,bool initMem){
    int c,charsUsed,i;
    detectedSym=0;
    for (c=0; c<256; c++){
        addSymbols[c]=0;
        codeword2sym[c]=0;
        reservedSet[c]=0;
        outputSet[c]=0;
    }
    for (c=0; c<256; c++){
        if (c==CHAR_ESCAPE || c==CHAR_FIRSTUPPER || c==CHAR_UPPERWORD || c==CHAR_UTFUPPER)
        {
            reservedSet[c]=1;
            addSymbols[c]=0;
        }
    }
    for (c=0; c<256; c++)
    if (addSymbols[c])
    reservedSet[c]=1;
    
    for (c=BINARY_FIRST; c<=BINARY_LAST; c++)
    addSymbols[c]=1;
    
    for (c=0; c<256; c++){
        if (reservedSet[c] || addSymbols[c])
        outputSet[c]=1;
    }
    charsUsed=0;
    for (c=0; c<256; c++){
        if (addSymbols[c]){
            codeword2sym[c]=charsUsed;
            charsUsed++;
            {
                if (c<128+64)
                dict1size=charsUsed;
                if (c<128+64+32)
                dict2size=charsUsed;
                if (c<128+64+32+16)
                dict3size=charsUsed;
                if (c<128+64+32+16+16)
                dict4size=charsUsed;
            }
        }
    }
    c=word_count;
    
    dict4size-=dict3size;
    dict3size-=dict2size;
    dict2size-=dict1size;
    if (dict1size<4 || dict2size<4 || dict3size<4 || dict4size<4){
        dict2size=dict3size=dict4size=charsUsed/4;
        dict1size=charsUsed-dict4size*3;
        for (i=0; i<charsUsed/4; i++){
            if (i*i*i*(charsUsed-i*3)>c){
                dict1size=charsUsed-i*3;
                dict2size=i;
                dict3size=i;
                dict4size=i;
                break;
            }
        }
    }   
    
    dictionary=(dict1size*dict2size*dict3size*dict4size+dict1size*dict2size*dict3size+dict1size*dict2size+dict1size);
    bound4=dict1size*dict2size*dict3size+dict1size*dict2size+dict1size;
    bound3=dict1size*dict2size+dict1size;
    dict123size=dict1size*dict2size*dict3size;
    dict12size=dict1size*dict2size;

    dict1plus2=dict1size+dict2size;
    dict1plus2plus3=dict1size+dict2size+dict3size;
    if (initMem){
        dict=(U8**)calloc(sizeof(U8*)*(dictionary+1),1);
        dictlen=(U8*)calloc(sizeof(U8)*(dictionary+1),1);
        //printf("dictionary %d ",dictionary);
        if (!dict || !dictlen)
        printf("Not enough memory!\n");
    }
    PRINT_DICT((" %d %d %d %d(%d) charsUsed=%d sizeDict=%d\n",dict1size,dict2size,dict3size,dict4size,dictionary,charsUsed,sizeDict));
}
// read dictionary from files to arrays
bool wrtDecoder::initialize(bool encoding){
    WRT_deinitialize();
    memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));
    
    //printf(" dict size %d " ,sortedDictSize);
    dict123size=sortedDictSize;
    if (dict123size<20)
    dict123size=20;
    initializeCodeWords(dict123size);
    int dicsize=dictionary*WORD_AVG_SIZE*2;
    dictmem=(U8*)calloc(dicsize,1);
    dictmem_end=dictmem+dicsize-256;
    PRINT_DICT(("allocated memory=%d\n",dicsize));
    if (!dictmem)
    quit("Not enough memory!\n");
    sizeDict=1;
    mem=loadDynamicDictionary(dictmem,dictmem_end);
    
    
    return true;
}
void wrtDecoder::WRT_deinitialize(){
    if (dict){
        free(dict);
        dict=NULL;
    }
    if (dictlen){
        free(dictlen);
        dictlen=NULL;
    }
    if (dictmem){
        free(dictmem);
        dictmem=NULL;
    }    
    sizeDict=0;
}

/*void wrtDecoder::defaultSettings(int n){    
    
    wrtnum=n;
     
}*/

// decode word using dictionary
#define DECODE_WORD1(i)\
    {\
        i++;\
        if (i>0 && i<sizeDict)\
        {\
            PRINT_CODEWORDS(("i=%d ",i)); \
            s_size=dictlen[i];\
            memcpy(s,dict[i],s_size+1);\
            PRINT_CODEWORDS(("%s\n",dict[i])); \
        }\
        else\
        {\
            s_size=0; \
            /*printf("File is corrupted %d/%d!\n",i,sizeDict);*/\
            fileCorrupted=true;\
        }\
    }

inline int wrtDecoder::decodeCodeWord(U8* &s,int& c){
    //static int i;
    int s_size=0;
 if (more==-1){

    if (codeword2sym[c]<dict1size){
        dindex=codeword2sym[c];
        DECODE_WORD1(dindex);
        more=-1;dindex=0;
        return s_size;
    }
    else
    if (codeword2sym[c]<dict1plus2)
    dindex=dict1size*(codeword2sym[c]-dict1size);
    else
    if (codeword2sym[c]<dict1plus2plus3){
        PRINT_CODEWORDS(("DC1b c=%d\n",codeword2sym[c]-dict1plus2));
        dindex=dict12size*(codeword2sym[c]-dict1plus2);
    }
    else
    dindex=dict123size*(codeword2sym[c]-dict1plus2plus3);
    more=1;
    PRINT_CODEWORDS(("DC1 c=%d i=%d\n",c,i));}
 else if (more==1){

    if (codeword2sym[c]<dict1size){
        dindex+=codeword2sym[c];
        dindex+=dict1size; //dictNo=2;
        DECODE_WORD1( dindex);
        more=-1;dindex=0;
        return s_size;
    }
    else
    if (codeword2sym[c]<dict1plus2){
        PRINT_CODEWORDS(("DC2b c=%d\n",codeword2sym[c]-dict1size));
        dindex+=dict1size*(codeword2sym[c]-dict1size);
    }
    else
    dindex+=dict12size*(codeword2sym[c]-dict1plus2);
    more=2;
    PRINT_CODEWORDS(("DC2 c=%d i=%d\n",c,i));}
 else if (more==2){

    if (codeword2sym[c]<dict1size){
        PRINT_CODEWORDS(("DC3b c=%d\n",codeword2sym[c]));
        dindex+=codeword2sym[c];
        dindex+=bound3; //dictNo=3;
        DECODE_WORD1( dindex);
        more=-1;dindex=0;
        return s_size;
    }
    else
    if (codeword2sym[c]<dict1plus2)
    dindex+=dict1size*(codeword2sym[c]-dict1size);

    more=3;
    PRINT_CODEWORDS(("DC3 c=%d i=%d\n",c,i));
    }
 else if (more==3){

    if (codeword2sym[c]<dict1size)
    dindex+=codeword2sym[c];
    //else 
    //printf("File is corrupted (codeword2sym[c]<dict1size)!\n");

    dindex+=bound4; //dictNo=4;
    DECODE_WORD1( dindex);
    more=-1;dindex=0;
    return s_size;
    }
    return 0;
}

int wrtDecoder::WRT_decode(int WRTd_c,U8 *decodedText,int *txtlen){
    //static int more=-1;
    
    if(more>0){
        if(more==4) {
            //ESCAPE
            more=-1;dindex=0;
            return -4;
        }
        s_size=decodeCodeWord(decodedText,WRTd_c);
        if (s_size>0 && more==-1){
            *txtlen=s_size;
            return -1;
        }   
        return more; //not done 
    }
    if (fileCorrupted) return 0;
        if (outputSet[WRTd_c]){
    
            switch (WRTd_c) {
            case CHAR_ESCAPE:
                more=4;dindex=0;
                return 0;
            case CHAR_FIRSTUPPER:dindex=0;
                return -2;
            case CHAR_UPPERWORD:dindex=0;
                return -3;
            case CHAR_UTFUPPER:dindex=0;
                return -5;
            }
         
            s_size=decodeCodeWord(decodedText,WRTd_c);

            if (s_size>0 && more==-1){
                *txtlen=s_size;
                return -1;
            } else  return more; 
        }

       
        if (WRTd_c>='0' && WRTd_c<='9'){dindex=0;
            return 0;
        }
    return 0;
}

void wrtDecoder::read_dict(int count,int size, U8* data){
    int i,c;
    U8* bound=(U8*)&word_hash[0] + HASH_TABLE_SIZE*sizeof(word_hash[0]) - 6;

    U8* bufferData=(U8*)&word_hash[0] + 3;
    // load dict    
    memcpy(bufferData,data,size);
    
    sortedDict.clear();
    
    PRINT_DICT(("count=%d\n",count));
    
    std::string s;
    std::string last_s;
    for (i=0; i<count; i++){
        while (bufferData[0]!=10){
            s.append(1,bufferData[0]);
            bufferData++;
            //if (s.size()>WORD_MAX_SIZE || bufferData>bound)
            //{
                //printf("File corrupted (s.size()>WORD_MAX_SIZE)!\n");
              //  quit("Not enough memory!\n");
            //}
        } 
        bufferData++;
        sortedDict.push_back(s);
        last_s=s;
        s.erase();
    }
    sortedDictSize=(int)sortedDict.size();
    PRINT_DICT(("read_dict count2=%d\n",count));
}

int wrtDecoder::WRT_start_decoding(int count,int size,U8* data){
    //last_c=0;
    s_size=0;
    collision=0;
    //defaultSettings(0); 
    read_dict( count,  size, data);
    WRT_deinitialize();
    if (!initialize(false))
    return 0;

    PRINT_CHARS(("WRT_start_decoding WRTd_c=%d ftell=%d\n",WRTd_c,ftell(XWRT_file)));

    return 0;
}
 
