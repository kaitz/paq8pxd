
// based on  "XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com"
//
#include <stdlib.h> 
#include <memory.h>
#pragma warning(disable:4786)
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <string.h>
#if defined WIN32 || defined WIN64
#include <windows.h>
#include <conio.h>
#endif

#define PRINT_CHARS(data) ;//printf data
#define PRINT_CODEWORDS(data); // printf data
#define PRINT_STACK(data) ;//printf data;
#define PRINT_DICT(data) ;//printf data;
#define PRINT_CONTAINERS(data) ;//printf data
//#define PRINT_STATUS(data) printf data;

#pragma warning(disable:4244) //  '=' : conversion from ... to ..., possible loss of data
#pragma warning(disable:4786) // STL warnings
#pragma warning(disable:4996) // '_getch' was declared deprecated
#pragma warning(disable:4503) // STL
#pragma warning(disable:4390) // empty controlled statement found; is this the intent?
#pragma warning(disable:4018) // signed/unsigned mismatch
#define _CRT_SECURE_NO_DEPRECATE // VC++ 2005 deprecate warnings


#if defined WIN32 || defined WIN64
#define getch _getch
#else
#define getch getchar
#endif
#ifndef WINDOWS
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
#endif

#define CHAR_FIRSTUPPER		1	// for encode lower word with first capital letter
#define CHAR_UPPERWORD		2	// for encode upper word
#define CHAR_ESCAPE			3	// for encode reserved chars (CHAR_ESCAPE,CHAR_FIRSTUPPER,...)
#define BINARY_FIRST		128
#define BINARY_LAST			255

#define OPTION_TRY_SHORTER_WORD				4


#if !defined min
#define min(a,b) (((a)>(b))?(b):(a))
#endif
#define IF_OPTION(option) (preprocFlag & option) //, printf("%d",option)
#define OPTION(option) (xml_wrt.preprocFlag & option)
#define TURN_OFF(option) {if (preprocFlag & option) preprocFlag-=option;}
#define TURN_ON(option) {if ((preprocFlag & option)==0) preprocFlag+=option;}
#define RESET_OPTIONS (preprocFlag=0)

#define WORD_MIN_SIZE		2
#define WORD_AVG_SIZE		8
#define WORD_MAX_SIZE		48
#define STRING_MAX_SIZE		255  // 1-byte for container.size()

#define MAX_DYNAMIC_DICT_COUNT	(65536*256)
#define HASH_TABLE_SIZE			(1<<20) //1MB*4

//#define BYTES_TO_DETECT			(50*1024)

//#define NUM_BASE			256
#define HASH_DOUBLE_MULT	37
#define HASH_MULT			23
//#define CHARSET_COUNT		6


enum EWordType { LOWERWORD, FIRSTUPPER, UPPERWORD, VARWORD, NUMBER};
enum ELetterType { LOWERCHAR, UPPERCHAR, UNKNOWNCHAR, RESERVEDCHAR, NUMBERCHAR };


#define OUT_OF_MEMORY() \
	{ \
		printf("Not enough memory!\n");\
		exit(0); \
	}
FILE* XWRT_file;
FILE* XWRT_fileout;
unsigned char** dict=NULL;
int* dictfreq=NULL;
unsigned char* dictlen=NULL;

#define PUTC(c) { putc(c,XWRT_fileout); }
#define GETC(c) { c=getc(XWRT_file); }
size_t fread_fast(unsigned char* dst, int len, FILE* file);
size_t fwrite_fast(unsigned char* dst, int len, FILE* file);


/////////////////////////////////////////////////////////


#define OUTPUT_BUFFER_MIN_SIZE 10240


// Input/Output using dynamic memory allocation
class CMemoryBuffer
{
public:
	CMemoryBuffer(std::string mname="");
	~CMemoryBuffer();

	void OutTgtByte( unsigned char c );
	int InpSrcByte( void );
	inline int Size();
	inline int Allocated(); 
	inline void AllocSrcBuf( unsigned int len );
	inline void Clear();

	static unsigned int memsize;
	unsigned char* TargetBuf;
	unsigned char* SourceBuf;
	unsigned int SrcLen, TgtLen;
	unsigned int SrcPtr, TgtPtr;
	std::string name;

private:
	inline void AllocTgtBuf( unsigned int len = OUTPUT_BUFFER_MIN_SIZE );
	inline void ReallocTgtBuf(unsigned int len);
};

class CContainers
{
public:
	CContainers();
	void prepareMemBuffers();
	void writeMemBuffers(int preprocFlag);
	void readMemBuffers(int preprocFlag, int maxMemSize);
	void freeMemBuffers(bool freeMem);

	CMemoryBuffer *memout;
	unsigned char *bigBuffer;	

private:
	std::vector<CMemoryBuffer*> mem_stack;
	std::map<std::string,CMemoryBuffer*> memmap;
};

unsigned int CMemoryBuffer::memsize=0;

CMemoryBuffer::CMemoryBuffer(std::string mname) 
{ 
	name=mname;
	Clear(); 
	AllocTgtBuf(); 
};

CMemoryBuffer::~CMemoryBuffer() 
{ 
	if (TargetBuf)
	free(TargetBuf-3);
	
	if (SourceBuf)
	free(SourceBuf);
};

inline void CMemoryBuffer::Clear()
{
	TargetBuf=NULL; SourceBuf=NULL; SrcPtr=0; TgtPtr=0; SrcLen=0; TgtLen=0;
}

inline int CMemoryBuffer::Size() 
{
	return TgtPtr;
}

inline int CMemoryBuffer::Allocated() 
{
	return TgtLen;
}

void CMemoryBuffer::OutTgtByte( unsigned char c ) 
{ 
	memsize++;

	*(TargetBuf+(TgtPtr++))=c; 
	if (TgtPtr>TgtLen-1){
		if (TgtLen > (1<<19))  // 512 KB
		ReallocTgtBuf(TgtLen+(1<<19));
		else
		ReallocTgtBuf(TgtLen*2);
	}
}

int CMemoryBuffer::InpSrcByte( void ) 
{
	memsize++;

	if (SrcPtr>=SrcLen)
	return EOF;

	return *(SourceBuf+(SrcPtr++)); 
}

inline void CMemoryBuffer::AllocSrcBuf( unsigned int len ){
	SrcLen = len;
	SourceBuf = (unsigned char*) malloc(SrcLen);
	if (SourceBuf==NULL)
	OUT_OF_MEMORY();
}

inline void CMemoryBuffer::AllocTgtBuf( unsigned int len ){
	TgtLen = len;
	TargetBuf = (unsigned char*) malloc(len+6);
	if (TargetBuf==NULL)
	OUT_OF_MEMORY();
	TargetBuf += 3;
}

inline void CMemoryBuffer::ReallocTgtBuf(unsigned int len){
	unsigned char* NewTargetBuf = (unsigned char*) malloc(len+6);

	if (NewTargetBuf==NULL)
	OUT_OF_MEMORY();

	NewTargetBuf += 3;
	memcpy(NewTargetBuf,TargetBuf,min(TgtPtr,len));
	TgtLen = len;
	delete(TargetBuf-3);
	TargetBuf=NewTargetBuf;
}

CContainers::CContainers() : bigBuffer(NULL) {};

void CContainers::prepareMemBuffers(){
	memout=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p("!data",memout);
	memmap.insert(p);	
}

void CContainers::writeMemBuffers(int preprocFlag){
	std::map<std::string,CMemoryBuffer*>::iterator it;

	int fileLen=0;
	int len=0;
	int lenCompr=0;
	int allocated=0;

	for (it=memmap.begin(); it!=memmap.end(); it++)
	{
		CMemoryBuffer* b=it->second;
		fileLen=b->Size();
		
		PRINT_CONTAINERS(("cont=%s fileLen=%d\n",it->first.c_str(),fileLen));

		if (fileLen>0)
		{
			allocated+=b->Allocated();
			len+=fileLen;
			
			PUTC((int)it->first.size());
			for (int i=0; i<(int)it->first.size(); i++)
			PUTC(it->first[i]);
			
			PUTC(fileLen>>24);
			PUTC(fileLen>>16);
			PUTC(fileLen>>8);
			PUTC(fileLen);

			fwrite_fast(it->second->TargetBuf,it->second->TgtPtr,XWRT_fileout);
			lenCompr+=fileLen;
		}
	}
	PUTC(0)
	PRINT_DICT(("dataSize=%d compr=%d allocated=%d\n",len,lenCompr,allocated));

	freeMemBuffers(true);
	prepareMemBuffers();
}

void CContainers::readMemBuffers(int preprocFlag, int maxMemSize){
	unsigned char* buf=NULL;
	unsigned int bufLen=0;
	unsigned int fileLen;
	unsigned int ui;
	int len=0;
	int lenCompr=0;
	int i,c;
	unsigned char s[STRING_MAX_SIZE];

	freeMemBuffers(true);
	prepareMemBuffers();
	CMemoryBuffer* memout_tmp=NULL;

	while (true){			
		GETC(i);

		if (i<=0)
		break;

		for (c=0; c<i; c++)
		GETC(s[c]);
		
		std::string str;
		str.append((char*)s,i);

		PRINT_CONTAINERS(("cont=%s\n",str.c_str()));

		if (str=="!data")
		memout_tmp=memout;
		else
		{
			memout_tmp=new CMemoryBuffer(str);
			std::pair<std::string,CMemoryBuffer*> p(str,memout_tmp);
			memmap.insert(p);
		}

		int c;
		for (i=0, fileLen=0; i<4; i++){
			GETC(c);
			fileLen=fileLen*256+c;
		}
		
		len+=fileLen;
		lenCompr+=fileLen;
		memout_tmp->AllocSrcBuf(fileLen);

		fread_fast(memout_tmp->SourceBuf,memout_tmp->SrcLen,XWRT_file);

		//printStatus(fileLen,0,false);
	}
	PRINT_DICT(("readMemBuffers() dataSize=%d compr=%d allocated=%d\n",len,lenCompr,maxMemSize+10240));
}

void CContainers::freeMemBuffers(bool freeMem){
	mem_stack.clear();

	std::map<std::string,CMemoryBuffer*>::iterator it;

	for (it=memmap.begin(); it!=memmap.end(); it++)
	{
		if (!freeMem)
		it->second->Clear();
		delete(it->second);			
	}

	memmap.clear();
}

/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////

class XWRT_Common
{
public:
	XWRT_Common(int fileBufferSize=17); // 128 kb
	~XWRT_Common();

	void defaultSettings();
	unsigned int flen( FILE* &f );
	
	CContainers cont;
	int preprocFlag;

protected:
	
	inline void stringHash(const unsigned char *ptr, int len,int& hash);
	int addWord(unsigned char* &mem,int &i);
	unsigned char* loadDynamicDictionary(unsigned char* mem,unsigned char* mem_end);
	void initializeLetterSet();
	void initializeCodeWords(int word_count,bool initMem=true);
	bool initialize(bool encoding);
	void WRT_deinitialize();

	void WRT_print_options();
	int minSpacesFreq();

	int* word_hash;
	bool decoding,fileCorrupted,detect,firstWarn;
	int maxDynDictBuf,minWordFreq,maxDictSize;
	int tryShorterBound,spaces,fileLenMB,beforeWord;
	int spacesCodeword[256];
	int spacesCont[256];
	std::vector<std::string> sortedDict;

	ELetterType letterType;
	ELetterType letterSet[256];

	int sizeDict,sizeDynDict;
	unsigned char* dictmem;
	unsigned char* dictmem_end;
	unsigned char* mem;
	
	int addSymbols[256]; // reserved symbols in output alphabet 
	int reservedSet[256]; // reserved symbols in input alphabet
	int outputSet[256];
	int wordSet[256]; 
	int sym2codeword[256]; 
	int codeword2sym[256]; 

	int dictionary,dict1size,dict2size,dict3size,dict4size,dict1plus2plus3,dict1plus2;
	int bound4,bound3,dict123size,dict12size,collision,quoteOpen,quoteClose,detectedSym;
	int maxMemSize;
	int sortedDictSize;
	

public:
};

XWRT_Common::XWRT_Common(int fileBufferSize) :  dictmem(NULL),
detect(false), dictmem_end(NULL),fileCorrupted(false)
{ 
	if (fileBufferSize<10)
	fileBufferSize=10; // 1 KB
	if (fileBufferSize>23)
	fileBufferSize=23; // 8 MB
	word_hash=new int[HASH_TABLE_SIZE];
	if (!word_hash)
	OUT_OF_MEMORY();
}
XWRT_Common::~XWRT_Common(){
	if (word_hash)
	delete(word_hash);
	WRT_deinitialize(); 
}
int XWRT_Common::minSpacesFreq(){
	return 300+200*(fileLenMB/5);
}

// make hash from string
inline void XWRT_Common::stringHash(const unsigned char *ptr, int len,int& hash){
	for (hash = 0; len>0; len--, ptr++){
		hash *= HASH_MULT;
		hash += *ptr;
	}
	hash=hash&(HASH_TABLE_SIZE-1);
}
int XWRT_Common::addWord(unsigned char* &mem,int &i){
	int c,j;
	if (i<=1 || sizeDict>=dictionary)
	return -1;
	
	dictlen[sizeDict]=i;
	dict[sizeDict]=mem;
	
	mem[i]=0;
	stringHash(mem,i,j);
	
	if (word_hash[j]!=0)
	{
		if (dictlen[sizeDict]!=dictlen[word_hash[j]] || memcmp(dict[sizeDict],dict[word_hash[j]],dictlen[sizeDict])!=0)
		{
			c=(j+i*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
			if (word_hash[c]!=0)
			{
				if (dictlen[sizeDict]!=dictlen[word_hash[c]] || memcmp(dict[sizeDict],dict[word_hash[c]],dictlen[sizeDict])!=0)
				{
					c=(j+i*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1);
					if (word_hash[c]!=0)
					{
						collision++;
						return -1;
					}
					else
					{
						word_hash[c]=sizeDict++;
					}
				}
				else
				return -1; // word already exists
			}
			else
			{
				word_hash[c]=sizeDict++;
			}
		}
		else
		return -1; // word already exists
	}
	else
	{
		word_hash[j]=sizeDict++;
	}
	return 1;
}
unsigned char* XWRT_Common::loadDynamicDictionary(unsigned char* mem,unsigned char* mem_end){
	int i;
	for (i=0; i<256; i++)
	spacesCodeword[i]=0;
	int count=sortedDictSize;
	for (i=0; i<count; i++)
	{
		std::string s=sortedDict[i];
		int len=(int)sortedDict[i].size();
		memcpy(mem,sortedDict[i].c_str(),len+1);
		if (addWord(mem,len)==0)
		break;
		mem+=(len/4+1)*4;
		if (mem>mem_end)
		break;
	}
	/*if (mem<mem_end)
	{
		i=strlen("http://www.");
		memcpy(mem,"http://www.",i);
		if (addWord(mem,i)!=0)
			mem+=(i/4+1)*4;
	}*/
	PRINT_DICT(("count=%d sortedDict.size()=%d\n",count,sortedDictSize));
	sizeDynDict=sizeDict;
	return mem;
}

void XWRT_Common::initializeLetterSet(){
	int c;
	for (c=0; c<256; c++)
	letterSet[c]=UNKNOWNCHAR;
	for (c='0'; c<='9'; c++)
	letterSet[c]=NUMBERCHAR;
	for (c='A'; c<='Z'; c++)
	letterSet[c]=UPPERCHAR;
	for (c='a'; c<='z'; c++)
	letterSet[c]=LOWERCHAR;
	for (c=0; c<256; c++)
	if (reservedSet[c])
	letterSet[c]=RESERVEDCHAR;
	for (c=0; c<256; c++)  //                                                - _ . , :
	if (c>127 || letterSet[c]==LOWERCHAR || letterSet[c]==UPPERCHAR || c==' ' /*|| c=='\''*/) // || c=='&') 
	wordSet[c]=1;
	else
	wordSet[c]=0;
}
void XWRT_Common::initializeCodeWords(int word_count,bool initMem){
	int c,charsUsed,i;
	detectedSym=0;
	for (c=0; c<256; c++){
		addSymbols[c]=0;
		codeword2sym[c]=0;
		sym2codeword[c]=0;
		reservedSet[c]=0;
		outputSet[c]=0;
	}
	for (c=0; c<256; c++){
		if (c==CHAR_ESCAPE || c==CHAR_FIRSTUPPER || c==CHAR_UPPERWORD )
		{
			reservedSet[c]=1;
			addSymbols[c]=0;
		}
	}
	for (c=0; c<256; c++)
	if (addSymbols[c])
	reservedSet[c]=1;
	initializeLetterSet();
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
			sym2codeword[charsUsed]=c;
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
		dict=(unsigned char**)calloc(sizeof(unsigned char*)*(dictionary+1),1);
		dictlen=(unsigned char*)calloc(sizeof(unsigned char)*(dictionary+1),1);
		if (!dict || !dictlen)
		OUT_OF_MEMORY();
	}
	PRINT_DICT((" %d %d %d %d(%d) charsUsed=%d sizeDict=%d\n",dict1size,dict2size,dict3size,dict4size,dictionary,charsUsed,sizeDict));
}
// read dictionary from files to arrays
bool XWRT_Common::initialize(bool encoding){
	int i,c,fileLen;
	FILE* file;
	WRT_deinitialize();
	memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));
	
	
	dict123size=sortedDictSize;
	if (dict123size<20)
	dict123size=20;
	initializeCodeWords(dict123size);
	int dicsize=dictionary*WORD_AVG_SIZE*2;
	dictmem=(unsigned char*)calloc(dicsize,1);
	dictmem_end=dictmem+dicsize-256;
	PRINT_DICT(("allocated memory=%d\n",dicsize));
	if (!dictmem)
	OUT_OF_MEMORY();
	sizeDict=1;
	mem=loadDynamicDictionary(dictmem,dictmem_end);
	
	
	return true;
}
void XWRT_Common::WRT_deinitialize(){
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
	if (dictfreq){
		free(dictfreq);
		dictfreq=NULL;
	}
	sizeDict=0;
}

void XWRT_Common::defaultSettings(){
	RESET_OPTIONS;
	TURN_ON(OPTION_TRY_SHORTER_WORD);
	maxMemSize=8*1024*1024;
	maxDynDictBuf=8*4;
	maxDictSize=65535*32700;
	tryShorterBound=3;//4
	minWordFreq=7*2; //7*2 64;
	
	//maxDictSize=	//e
	//minWordFreq=  // f
	//maxMemSize  maxMemSize*=1024*1024;//m
	//maxDynDictBuf //b
}

size_t fread_fast(unsigned char* dst, int len, FILE* file){
	return fread(dst,1,len,file);
	int rd;
	size_t sum=0;
	while (len > 1<<17) // 128 kb
	{
		rd=fread(dst,1,1<<17,file);
		dst+=rd;
		len-=rd;
		sum+=rd;
	}
	sum+=fread(dst,1,len,file);
	return sum;
}
size_t fwrite_fast(unsigned char* dst, int len, FILE* file){
	return fwrite(dst,1,len,file);
	int wt;
	size_t sum=0;
	while (len > 1<<17) // 128 kb
	{
		wt=fwrite(dst,1,1<<17,file);
		dst+=wt;
		len-=wt;
		sum+=wt;
	}
	sum+=fwrite(dst,1,len,file);
	return sum;
}
//////////////////////////////////////////

class XWRT_Decoder : public XWRT_Common
{
public:

	XWRT_Decoder();
	~XWRT_Decoder();

	int WRT_start_decoding(FILE* in);
	int WRT_decode();
private:

	inline void toUpper(unsigned char* s,int &s_size);
	void read_dict();
	inline int decodeCodeWord(unsigned char* &s,int& c);

	enum EUpperType { UFALSE, UTRUE, FORCE };

	int s_size,WRTd_c;
	int last_c;
	bool WRTd_upper;
	bool WRTd_initialized;
	unsigned char WRTd_data[STRING_MAX_SIZE];
	unsigned char *WRTd_s;
	EUpperType upperWord;

public:
}; // end class 

XWRT_Decoder::XWRT_Decoder() : WRTd_s(&WRTd_data[0]) 
{ 	

};

XWRT_Decoder::~XWRT_Decoder(){ 
	if (cont.bigBuffer)
	{
		free(cont.bigBuffer);
		cont.bigBuffer=NULL;
		cont.freeMemBuffers(false);
	}
	else
	cont.freeMemBuffers(true);
}

#define DECODE_GETC(c)\
	{\
		if (cont.memout->memsize>maxMemSize) \
		{ \
			PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
			cont.readMemBuffers(preprocFlag,maxMemSize); \
			cont.memout->memsize=0; \
		} \
		\
		c=cont.memout->InpSrcByte(); \
	}

// decode word using dictionary
#define DECODE_WORD(dictNo,i)\
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


// convert lower string to upper
inline void XWRT_Decoder::toUpper(unsigned char* s,int &s_size){
	for (int i=0; i<s_size; i++)
	s[i]=toupper(s[i]); 
}

inline int XWRT_Decoder::decodeCodeWord(unsigned char* &s,int& c){
	int i,s_size;

	if (codeword2sym[c]<dict1size){
		i=codeword2sym[c];
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
	i=dict1size*(codeword2sym[c]-dict1size);
	else
	if (codeword2sym[c]<dict1plus2plus3){
		PRINT_CODEWORDS(("DC1b c=%d\n",codeword2sym[c]-dict1plus2));
		i=dict12size*(codeword2sym[c]-dict1plus2);
	}
	else
	i=dict123size*(codeword2sym[c]-dict1plus2plus3);

	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC1 c=%d i=%d\n",c,i));

	if (codeword2sym[c]<dict1size){
		i+=codeword2sym[c];
		i+=dict1size; //dictNo=2;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2){
		PRINT_CODEWORDS(("DC2b c=%d\n",codeword2sym[c]-dict1size));
		i+=dict1size*(codeword2sym[c]-dict1size);
	}
	else
	i+=dict12size*(codeword2sym[c]-dict1plus2);

	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC2 c=%d i=%d\n",c,i));

	if (codeword2sym[c]<dict1size){
		PRINT_CODEWORDS(("DC3b c=%d\n",codeword2sym[c]));
		i+=codeword2sym[c];
		i+=bound3; //dictNo=3;
		DECODE_WORD(dictNo, i);
		return s_size;
	}
	else
	if (codeword2sym[c]<dict1plus2)
	i+=dict1size*(codeword2sym[c]-dict1size);


	DECODE_GETC(c);
	PRINT_CODEWORDS(("DC3 c=%d i=%d\n",c,i));

	if (codeword2sym[c]<dict1size)
	i+=codeword2sym[c];
	//else 
	//printf("File is corrupted (codeword2sym[c]<dict1size)!\n");

	i+=bound4; //dictNo=4;
	DECODE_WORD(dictNo, i);
	return s_size;
}

int XWRT_Decoder::WRT_decode(){
	int rchar=0;
	int c;
	static int s_sizep=0;
	if (s_sizep<s_size && s_sizep!=0 ){
		rchar=WRTd_s[s_sizep];
		last_c=rchar;
		++s_sizep;
		return rchar;
	}
	if(s_sizep==s_size && s_sizep!=0 ){
		DECODE_GETC(WRTd_c);
		s_sizep=0;
		s_size=0;
	}
	while (1){
		if (fileCorrupted)
		return -1;

		PRINT_CHARS(("c=%d (%c)\n",WRTd_c,WRTd_c));

		if (outputSet[WRTd_c]){
			PRINT_CHARS(("addSymbols[%d] upperWord=%d\n",WRTd_c,upperWord));

			switch (WRTd_c)
			{
			case CHAR_ESCAPE:
				WRTd_upper=false;
				upperWord=UFALSE;
				DECODE_GETC(WRTd_c);
				PRINT_CHARS(("c==CHAR_ESCAPE, next=%x\n",WRTd_c));
				rchar=WRTd_c;
				last_c=rchar;
				DECODE_GETC(WRTd_c);
				return rchar;

			case CHAR_FIRSTUPPER:
				PRINT_CHARS(("c==CHAR_FIRSTUPPER\n"));

				WRTd_upper=true;
				upperWord=UFALSE;
				DECODE_GETC(WRTd_c);
				continue;

			case CHAR_UPPERWORD:
				PRINT_CHARS(("c==CHAR_UPPERWORD\n"));

				upperWord=FORCE;
				DECODE_GETC(WRTd_c);
				continue;

			}

			if (upperWord==FORCE)
			upperWord=UTRUE;
			else
			upperWord=UFALSE;

			s_size=decodeCodeWord(WRTd_s,WRTd_c);

			if (WRTd_upper){
				WRTd_upper=false;
				WRTd_s[0]=toupper(WRTd_s[0]);
			}
			
			if (upperWord!=UFALSE)
			toUpper(&WRTd_s[0],s_size);
			
			upperWord=UFALSE;
			
			if (s_size>0){
				s_sizep=1;
				rchar=WRTd_s[0];;
				last_c=rchar;
				return rchar;
			}
		}

		if (WRTd_c>='0' && WRTd_c<='9'){
			unsigned int no,mult;
			int c,i;
			no=0;
			mult=1;
			static int wType=0;
			rchar=WRTd_c;
			DECODE_GETC(WRTd_c);
			last_c=rchar;
			return rchar;
		}

		PRINT_CHARS(("other c=%d (%d) upperWord=%d\n",fileLenMB,upperWord));

		if (upperWord!=UFALSE){
			if (upperWord==FORCE)
			upperWord=UTRUE;

			if (WRTd_c>='a' && WRTd_c<='z')
			WRTd_c=toupper(WRTd_c);
			else
			upperWord=UFALSE;
		}
		else
		if (WRTd_upper){
			WRTd_upper=false;
			WRTd_c=toupper(WRTd_c);
		}
		rchar=WRTd_c;
		last_c=rchar;
		DECODE_GETC(WRTd_c);
		return rchar;
	}
}

void XWRT_Decoder::read_dict(){
	int i,c,count;
	unsigned char* bound=(unsigned char*)&word_hash[0] + HASH_TABLE_SIZE*sizeof(word_hash[0]) - 6;

	unsigned char* bufferData=(unsigned char*)&word_hash[0] + 3;
	
	for (i=0, count=0; i<3; i++){
		GETC(c);
		count=count*256+c;
	}

	fread_fast(bufferData,count,XWRT_file);
	

	count=bufferData[0]; bufferData++;
	count+=256*bufferData[0]; bufferData++;
	count+=65536*bufferData[0]; bufferData++;
	
	sortedDict.clear();
	
	PRINT_DICT(("count=%d\n",count));
	
	std::string s;
	std::string last_s;
	for (i=0; i<count; i++){
		if ( bufferData[0]>=128){
			s.append(last_s.c_str(),bufferData[0]-128);
			bufferData++;
		}

		while (bufferData[0]!=10){
			s.append(1,bufferData[0]);
			bufferData++;

			if (s.size()>WORD_MAX_SIZE || bufferData>bound)
			{
				//printf("File corrupted (s.size()>WORD_MAX_SIZE)!\n");
				OUT_OF_MEMORY();
			}
		}
		bufferData++;

		sortedDict.push_back(s);
		last_s=s;
		s.erase();
	}

	sortedDictSize=(int)sortedDict.size();
	PRINT_DICT(("read_dict count2=%d\n",count));

}


int XWRT_Decoder::WRT_start_decoding(FILE* in){
	int i,j,k,c;
	XWRT_file=in;
	last_c=0;
	WRTd_upper=false;
	upperWord=UFALSE;
	s_size=0;
	collision=0;

	defaultSettings(); 
	GETC(maxMemSize); 
	maxMemSize*=1024*1024;
	int fileLen;

	GETC(c);
	fileLen=c;
	GETC(c);
	fileLen=fileLen|(c<<8);
	GETC(c);
	fileLen=fileLen|(c<<16);
	GETC(c);
	fileLen=fileLen|(c<<24);
	fileLenMB=fileLen/(1024*1024);
	if (fileLenMB>255*256)
	fileLenMB=255*256;

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d\n",maxMemSize,fileLenMB));
	read_dict();

	cont.readMemBuffers(preprocFlag,maxMemSize);
	cont.memout->memsize=0;

	WRT_deinitialize();

	decoding=true;
	if (!initialize(false))
	return 0;

	DECODE_GETC(WRTd_c);
	PRINT_CHARS(("WRT_start_decoding WRTd_c=%d ftell=%d\n",WRTd_c,ftell(XWRT_file)));

	return fileLen;
}


/////////////////////////////////////////////////////////////////////
///////////
//////////////////////////////////////////////////////////////////////


class XWRT_Encoder : public XWRT_Common
{
public:

	XWRT_Encoder();
	~XWRT_Encoder();

	void WRT_start_encoding(FILE* in, FILE* out,unsigned int fileLen,bool type_detected);

private:

	void WRT_encode( int filelen);
	inline void encodeCodeWord(int &i);
	inline void encodeSpaces();
	inline void encodeWord(unsigned char* s,int s_size,EWordType wordType,int& c);
	inline void encodeAsText(unsigned char* &s,int &s_size,EWordType wordType);
	inline int findShorterWord(unsigned char* &s,int &s_size);
	inline void toLower(unsigned char* s,int &s_size);
	inline void toUpper(unsigned char* s,int &s_size);
	inline void checkWord(unsigned char* &s,int &s_size,int& c);
	
	inline void checkHashExactly(unsigned char* &s,int &s_size,int& i);
	inline int checkHash(unsigned char* &s,int &s_size,int h);
	inline void stringHash(const unsigned char *ptr, int len,int& hash);
	
	void encodeMixed(unsigned char* s,int s_size,int& c);
	void sortDict(int size);

	void write_dict();
	int WRT_detectFileType(int filelen);
	void WRT_detectFinish();

	int s_size;
	int last_c_bak,last_c,last_last_c;
	int filelento;


	unsigned char* dynmem;
	unsigned char *dictbound;

public:
}; // end class 

int compare_freq( const void *arg1, const void *arg2 );


XWRT_Encoder::XWRT_Encoder() :  last_c_bak(0),filelento(0)
{ 	
};
XWRT_Encoder::~XWRT_Encoder(){ 
	
}
#define ENCODE_PUTC(c)\
	{ \
		if (!detect) \
		{ \
			if (cont.memout->memsize>maxMemSize) \
			{ \
				PRINT_DICT(("%d maxMemSize=%d\n",cont.memout->memsize,maxMemSize)); \
				cont.writeMemBuffers(preprocFlag); \
				cont.memout->memsize=0; \
			} \
			\
			PRINT_CHARS(("output=%d (%c)\n",c,c)); \
			cont.memout->OutTgtByte(c); \
		} \
	}

#define ENCODE_GETC(c) \
	{ \
		last_last_c=last_c; \
		last_c=last_c_bak; \
		\
		c=getc(XWRT_file); \
		filelento++;\
		last_c_bak=c; \
	}


// encode word (should be lower case) using n-gram array (when word doesn't exist in the dictionary)
inline void XWRT_Encoder::encodeAsText(unsigned char* &s,int &s_size,EWordType wordType){
	int i=0;
	for (i=0; i<s_size; i++)
	{
		if (addSymbols[s[i]])
		ENCODE_PUTC(CHAR_ESCAPE);
		ENCODE_PUTC(s[i]);
	}
	return;
}
inline void XWRT_Encoder::encodeCodeWord(int &i){
	int first,second,third,fourth;
	first=i-1;
	if (first>=bound4){
		first-=bound4;
		fourth=first/dict123size;
		first=first%dict123size;
		third=first/dict12size;		
		first=first%dict12size;
		second=first/dict1size;		
		first=first%dict1size;
		ENCODE_PUTC(sym2codeword[dict1plus2plus3+fourth]);
		PRINT_CODEWORDS(("1st=%d ",sym2codeword[dict1plus2plus3+fourth]));
		ENCODE_PUTC(sym2codeword[dict1plus2+third]);
		PRINT_CODEWORDS(("2nd=%d ",sym2codeword[dict1plus2+third]));
		ENCODE_PUTC(sym2codeword[dict1size+second]);
		PRINT_CODEWORDS(("3rd=%d ",sym2codeword[dict1size+second]));
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("4th=%d ",sym2codeword[first]));
	}
	else
	if (first>=bound3){
		first-=bound3;
		third=first/dict12size;		
		first=first%dict12size;
		second=first/dict1size;		
		first=first%dict1size;
		ENCODE_PUTC(sym2codeword[dict1plus2+third]);
		PRINT_CODEWORDS(("1st=%d(%d) ",sym2codeword[dict1plus2+third],third));
		ENCODE_PUTC(sym2codeword[dict1size+second]);
		PRINT_CODEWORDS(("2nd=%d(%d) ",sym2codeword[dict1size+second],second));
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("3rd=%d(%d) ",sym2codeword[first],first));
	}
	else
	if (first>=dict1size){
		first-=dict1size;
		second=first/dict1size;		
		first=first%dict1size;
		ENCODE_PUTC(sym2codeword[dict1size+second]);
		PRINT_CODEWORDS(("1st=%d ",sym2codeword[dict1size+second]));
		
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("2nd=%d ",sym2codeword[first]));
	}
	else{
		ENCODE_PUTC(sym2codeword[first]);
		PRINT_CODEWORDS(("1st=%d ",sym2codeword[first]));
	}

}

inline void XWRT_Encoder::encodeSpaces(){
	if (spaces==1){
		ENCODE_PUTC(' ');
	}
	else
	if (spaces>0){
		while (spaces>0){
			int sp=spaces;
			if (spaces>=256)
			sp=255;
			
			while (sp>0 && spacesCodeword[sp]==0) sp--;
			if (spacesCodeword[sp])	{		
				encodeCodeWord(spacesCodeword[sp]);
				spaces-=sp;
			}
			else{
				ENCODE_PUTC(' ');
				spaces--;
			}
		}
	}
	spaces=0;
}
// make hash from string
inline void XWRT_Encoder::stringHash(const unsigned char *ptr, int len,int& hash){
	for (hash = 0; len>0; len--, ptr++){
		hash *= HASH_MULT;
		hash += *ptr;
	}
	hash=hash&(HASH_TABLE_SIZE-1);
}
// check if word "s" does exist in the dictionary 
inline void XWRT_Encoder::checkHashExactly(unsigned char* &s,int &s_size,int& i){
	int h;
	stringHash(s,s_size,h);
	i=word_hash[h];
	if (i>0){
		if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0){
			i=word_hash[(h+s_size*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
			if (i>0){
				if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0){
					i=word_hash[(h+s_size*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
					if (i>0){
						if (dictlen[i]!=s_size || memcmp(dict[i],s,s_size)!=0)
						i=-1;
					}
					else
					i=-1;
				}
			}
			else
			i=-1;
		}
	}
	else
	i=-1;
	if (i>=dictionary)
	i=-1;
}
// check if word "s" (prefix of original word) does exist in the dictionary using hash "h" 
inline int XWRT_Encoder::checkHash(unsigned char* &s,int &s_size,int h){
	int i=word_hash[h];
	if (i>0){
		if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0){
			i=word_hash[(h+s_size*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
			if (i>0){
				if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0){
					i=word_hash[(h+s_size*HASH_DOUBLE_MULT*HASH_DOUBLE_MULT)&(HASH_TABLE_SIZE-1)];
					if (i>0){
						if (dictlen[i]>s_size || memcmp(dict[i],s,s_size)!=0)
						i=-1;
					}
					else
					i=-1;
				}
			}
			else
			i=-1;
		}
	}
	else
	i=-1;
	if (i>=dictionary)
	i=-1;
	return i;
}
// check if word "s" or prefix of word "s" does exist in the dictionary using hash "h" 
inline int XWRT_Encoder::findShorterWord(unsigned char* &s,int &s_size){
	int ret;
	int i;
	int best;
	unsigned int hash;
	hash = 0;
	for (i=0; i<WORD_MIN_SIZE+tryShorterBound; i++)
	hash = HASH_MULT * hash + s[i];

	best=-1;
	for (i=WORD_MIN_SIZE+tryShorterBound; i<s_size; i++){
		ret=checkHash(s,i,hash&(HASH_TABLE_SIZE-1));	
		if (ret>=0)
		best=ret;
		hash = HASH_MULT*hash + s[i];
	}
	return best;
}
// convert lower string to upper
inline void XWRT_Encoder::toUpper(unsigned char* s,int &s_size){
	for (int i=0; i<s_size; i++)
	s[i]=toupper(s[i]); 
}
// convert upper string to lower
inline void XWRT_Encoder::toLower(unsigned char* s,int &s_size){
	for (int i=0; i<s_size; i++)
	s[i]=tolower(s[i]);
}
void XWRT_Encoder::encodeMixed(unsigned char* s,int s_size,int& old_c){
	int c,size,start,ptr=0;
	EWordType wordType;
	unsigned char* s2;
	do
	{
		start=ptr;
		do
		{
			c=s[ptr++];
			letterType=letterSet[c];
		}
		while (ptr<s_size && letterType==NUMBERCHAR);
		
		if (letterType!=NUMBERCHAR)
		ptr--;
		wordType=NUMBER;
		encodeWord(s+start,ptr-start,wordType,old_c);
		
		if (ptr>=s_size)
		break;
		
		start=ptr;
		do
		{
			c=s[ptr++];
			letterType=letterSet[c];
		}
		while (ptr<s_size && letterType!=NUMBERCHAR);
		
		if (letterType==NUMBERCHAR)
		ptr--;
		wordType=VARWORD;
		s2=s+start;
		size=ptr-start;
		encodeAsText(s2,size,wordType);
	}
	while (ptr<s_size);
}
// encode word "s" using dictionary
void XWRT_Encoder::encodeWord(unsigned char* s,int s_size,EWordType wordType,int& c){
	if (detect)	{
		if (s_size>0 && wordType!=3){
			//s[s_size+1]=0;
			//printf("%s %d %d\n",s,s_size,wordType);
			toLower(s,s_size);
		}
		checkWord(s,s_size,c);
		return;
	}
	if (s_size<1){
		encodeSpaces();
		return;
	}
	int i=-1;
	int size=0;
	int flagToEncode=-1;
	bool justAdded=false;
	
	if (s_size>=WORD_MIN_SIZE){
		checkHashExactly(s,s_size,i);
		PRINT_CODEWORDS(("i=%d/%d %s(%d)\n",i,sizeDynDict,s,s_size));
		
		if (i>=0)// && codeWordSize(i)<=s_size)
		wordType=LOWERWORD;
		
		if (i<0){
			if (wordType==FIRSTUPPER || wordType==UPPERWORD){
				if (wordType==FIRSTUPPER){
					flagToEncode=CHAR_FIRSTUPPER;
					s[0]=tolower(s[0]);
				}
				else // wordType==UPPERWORD
				{
					flagToEncode=CHAR_UPPERWORD;
					toLower(s,s_size);
				}
				checkHashExactly(s,s_size,i);
				PRINT_CODEWORDS(("checkHashExactly i=%d %d=%s\n",i,s_size,s));
			}
			
			
			if (i<0 ){
				// try to find shorter version of word in dictionary
				i=findShorterWord(s,s_size);
				PRINT_CODEWORDS(("findShorterWord i=%d\n",i));
				//s[s_size+1]=0;
				//if (i>0 ) printf("findShorterWord i=%d %s\n",i, s);
				if (i>=0){
					size=dictlen[i];
					if (wordType==UPPERWORD){
						int ss=s_size-size;
						toUpper(s+size,ss);
					}
				}
			}
		}
	}
	if (i>=0){
		encodeSpaces();
		if (wordType==FIRSTUPPER || wordType==UPPERWORD){
			ENCODE_PUTC(flagToEncode);
		}
		encodeCodeWord(i);
		if (size>0){
			if (wordType==FIRSTUPPER)
			wordType=LOWERWORD;
			unsigned char* s2=s+size;
			int s_size2=s_size-size;
			i=-1;
			if (s_size2>(tryShorterBound+1)){  //try remainig word
				// try to find shorter version of word in dictionary
				i=findShorterWord(s2,s_size2);
				PRINT_CODEWORDS(("findShorterWord i=%d\n",i));
			}
			if (i>=0 && wordType!=UPPERWORD){
				size=dictlen[i];
				//encodeSpaces();
				encodeCodeWord(i);
				s2=s2+size;
				s_size2=s_size2-size;
				if (s_size2>0) encodeAsText(s2,s_size2,wordType);
			}
			else encodeAsText(s2,s_size2,wordType);
		}
	}
	else
	{
		if (wordType==FIRSTUPPER)
		s[0]=toupper(s[0]);
		else if (wordType==UPPERWORD)
		toUpper(s,s_size);
		encodeSpaces();
		encodeAsText(s,s_size,wordType);
	}
	return;
}
// process the file
void XWRT_Encoder::WRT_encode(int filelen){
	unsigned char s[STRING_MAX_SIZE];
	EWordType wordType;
	int c;
	spaces=0;
	s_size=0;
	last_c=0;
	filelento=-1;
	wordType=LOWERWORD;
	ENCODE_GETC(c);
	while (true) 
	{
		if (filelento==filelen)
		break;
		PRINT_CHARS(("c=%c (%d) last=%c \n",c,c,last_c));
		
		if (detect){
			letterType=letterSet[c];
		}
		else
		{
			if (c==13){
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				ENCODE_GETC(c);
				if (addSymbols[13])
				ENCODE_PUTC(CHAR_ESCAPE);
				ENCODE_PUTC(13);
				continue;
			}
			
			letterType=letterSet[c];
			
			if (letterType==RESERVEDCHAR){
				PRINT_CHARS(("reservedSet[c] c=%d (%c)\n",c,c));
				
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				
				PRINT_CHARS(("out CHAR_ESCAPE=%d\n",CHAR_ESCAPE));
				ENCODE_PUTC(CHAR_ESCAPE);	
				ENCODE_PUTC(c);
				
				ENCODE_GETC(c);
				continue;
			}
			
			
			if (letterType==NUMBERCHAR){	
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				ENCODE_PUTC(c);
				ENCODE_GETC(c);
				//	wordType=LOWERWORD;
				continue;
			}		
			
		}
		if (wordSet[c]){
			if (c!=' '){
				if (s_size==0){
					if (last_c!=' ')
					beforeWord=last_c;
					else
					beforeWord=last_last_c;
					if (letterType==LOWERCHAR)
					wordType=LOWERWORD;
					else
					if (letterType==UPPERCHAR)
					wordType=FIRSTUPPER;
					else
					wordType=VARWORD;
				}
				else
				{
					switch (wordType)
					{
					case LOWERWORD:
						if (letterType!=LOWERCHAR)
						wordType=VARWORD;
						break;
					case UPPERWORD:
						if (letterType!=UPPERCHAR)
						wordType=VARWORD;
						break;
					case FIRSTUPPER:
						if (letterType!=LOWERCHAR)
						{
							if (s_size==1 && letterType==UPPERCHAR)
							wordType=UPPERWORD;
							else
							wordType=VARWORD;
						}
						break;
					}
				}
			}
			else
			{
				encodeWord(s,s_size,wordType,c);
				s_size=0;
				spaces++;
				while (true){
					ENCODE_GETC(c);
					if (c!=' ')
					break;
					spaces++;
				}
				continue;
			}
			//detect words like and split. HiTerraMonda
			if(s_size>2 && letterType==UPPERCHAR && letterSet[last_c]==LOWERCHAR){
				if (s_size>2 && wordType==VARWORD){
					if (letterSet[s[0]]==UPPERCHAR){
						wordType=FIRSTUPPER;
						for(int i=1;i<=s_size;i++){
							if (letterSet[s[i]]==UPPERCHAR){
								wordType==VARWORD;
								break;
							}
						}
					}
				}
				encodeWord(s,s_size,wordType,c);
				//s[s_size++]=0;
				//printf("%s %d %d\n",s,s_size,wordType);
				s_size=0;
				
				continue;
			}
			s[s_size++]=c;
			if (s_size>=STRING_MAX_SIZE-2){
				encodeWord(s,s_size,wordType,c);
				s_size=0;
			}
			ENCODE_GETC(c);
			continue;
		}
		encodeWord(s,s_size,wordType,c);
		s_size=0;
		ENCODE_PUTC(c);
		ENCODE_GETC(c);
	}
	encodeWord(s,s_size,wordType,c);
	s_size=0;
}
inline int common(const char* offset1,const char* offset2, int bound){
	int lp=0;
	while (offset1[lp]==offset2[lp] && lp<bound)
	lp++;
	return lp;
}
void XWRT_Encoder::write_dict(){
	int i,count=0;
	unsigned char *bound=(unsigned char*)&word_hash[0]+HASH_TABLE_SIZE*sizeof(word_hash[0])-WORD_MAX_SIZE;
	unsigned char *writeBuffer=(unsigned char*)&word_hash[0]; //putcBuffer;
	unsigned char *bufferData=writeBuffer+3;
	
	unsigned char *count_header=bufferData;
	bufferData+=3;
	PRINT_DICT(("sortedDict.size()=%d\n",sortedDict.size()));
	int cmn;
	count=(int)sortedDict.size();
	for (i=0; i<count; i++){
		cmn=0;
		//if (i>0)
		//	cmn=common(sortedDict[i-1].c_str(),sortedDict[i].c_str(),min(sortedDict[i].size(),sortedDict[i-1].size()));
		if ((cmn>0 || (unsigned char)(sortedDict[i][0])>=128))
		bufferData+=sprintf((char*)bufferData,"%c%s\n",128+cmn,sortedDict[i].c_str()+cmn);
		else
		bufferData+=sprintf((char*)bufferData,"%s\n",sortedDict[i].c_str());
		if (bufferData>bound)
		break;
	}
	sortedDictSize=(int)i; // i<=count
	PRINT_DICT(("sortedDictCount=%d\n",sortedDictSize));
	count_header[0]=sortedDictSize%256;
	count_header[1]=(sortedDictSize/256)%256;
	count_header[2]=sortedDictSize/65536;
	count=(int)(bufferData-(writeBuffer+3));
	PRINT_DICT(("write_dict count=%d\n",count));
	PUTC(count>>16);
	PUTC(count>>8);
	PUTC(count);
	fwrite_fast((unsigned char*)writeBuffer+3,count,XWRT_fileout);
}

void XWRT_Encoder::WRT_start_encoding(FILE* in, FILE* out,unsigned int fileLen,bool type_detected){
	collision=0;
	XWRT_file=in;
	XWRT_fileout=out;

	fileLenMB=fileLen/(1024*1024);
	if (fileLenMB>255*256)
	fileLenMB=255*256;
	
	cont.prepareMemBuffers();
	cont.memout->memsize=0;
	//if (fileLenMB>64) minWordFreq-=4,tryShorterBound=2;
	
	/*	if (fileLenMB>64) minWordFreq-=16;
	if (fileLenMB<16)
		minWordFreq-=7;
	//if (fileLenMB<6)
	//	minWordFreq=minWordFreq+15;*/
if (fileLenMB<1)
		minWordFreq=minWordFreq*6;
	//if (fileLenMB<1)		minWordFreq=9,tryShorterBound=4;
	//if (fileLen<256*1024)		minWordFreq=7,tryShorterBound=3;
	int pos=ftell(XWRT_file);
	if (!type_detected)
	WRT_detectFileType(fileLen);

	fseek(XWRT_file, pos, SEEK_SET );

	PUTC(maxMemSize/(1024*1024));
	PUTC(fileLen&0xFF);
	PUTC((fileLen>>8)&0xFF);
	PUTC((fileLen>>16)&0xFF);
	PUTC((fileLen>>24)&0xFF);

	PRINT_DICT(("maxMemSize=%d fileLenMB=%d\n",maxMemSize,fileLenMB));
	write_dict(); // przed initialize()
	decoding=false;
	WRT_deinitialize();
	if (!initialize(true))
	return;
	WRT_encode(fileLen);
	cont.writeMemBuffers(preprocFlag);
	cont.freeMemBuffers(true);
}

inline void XWRT_Encoder::checkWord(unsigned char* &s,int &s_size,int& c){
	if (s_size<1){
		spaces=0;
		return;
	}
	if (s_size>WORD_MAX_SIZE)
	s_size=WORD_MAX_SIZE; 
	
	spaces=0;
	if (s_size<WORD_MIN_SIZE){
		spaces=0;
		return;
	} 
	int i;
	checkHashExactly(s,s_size,i);
	if (i<0){
		if (dynmem>dictbound){
			if (firstWarn){
				//printf("warning: dictionary too big\n"); //-b option
				firstWarn=false;
			}
			return;
		}
		memcpy(dynmem,s,s_size);
		if (addWord(dynmem,s_size)==1){
			dynmem+=(s_size/4+1)*4;
			dictfreq[sizeDict-1]=1;
		}
	}
	else
	{
		dictfreq[i]++;
	}
}
int XWRT_Encoder::WRT_detectFileType(int filelen){
	detect=true;
	//memset(value,0,sizeof(value));
	memset(addSymbols,0,sizeof(addSymbols));
	memset(reservedSet,0,sizeof(reservedSet));
	memset(spacesCont,0,sizeof(spacesCont));
	spaces=0;
	firstWarn=true;
	sizeDict=1;
	PRINT_DICT(("maxDynDictBuf=%d maxMemSize=%d\n",maxDynDictBuf,maxMemSize));
	dictionary=maxDynDictBuf*(MAX_DYNAMIC_DICT_COUNT/256);  // 512k, dblp=372k
	dictmem=(unsigned char*)calloc(dictionary*WORD_AVG_SIZE,1);
	dictbound=dictmem+dictionary*WORD_AVG_SIZE-WORD_MAX_SIZE;
	dict=(unsigned char**)calloc(sizeof(unsigned char*)*(dictionary+1),1);
	dictlen=(unsigned char*)calloc(sizeof(unsigned char)*(dictionary+1),1);
	dictfreq=(int*)calloc(sizeof(int)*(dictionary+1),1);
	memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));
	dynmem=dictmem;
	PRINT_DICT(("maxDict=%d allocatedMemory=%d hashTable=%d\n",dictionary,dictionary*WORD_AVG_SIZE+sizeof(unsigned char*)*(dictionary+1)+sizeof(unsigned char)*(dictionary+1)+sizeof(int)*(dictionary+1),HASH_TABLE_SIZE*sizeof(word_hash[0])));
	if (dictmem && dict && dictlen && dictfreq){
		initializeLetterSet();
		WRT_encode(filelen);
		WRT_detectFinish();
	}
	WRT_deinitialize();
	if (collision>0)
	PRINT_DICT(("warning: hash collisions=%d\n",collision));
	detect=false;
	return preprocFlag;
}
int compare_str( const void *arg1, const void *arg2 ){
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	return strcmp((char*)dict[a],(char*)dict[b]);
}
int compare_str_rev( const void *arg1, const void *arg2 ){
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	int minv=min(dictlen[a],dictlen[b]);
	for (int i=1; i<=minv; i++){
		if (dict[a][dictlen[a]-i]!=dict[b][dictlen[b]-i])
		return dict[a][dictlen[a]-i] - dict[b][dictlen[b]-i];
	}
	return dictlen[a] - dictlen[b];
}
int compare_freq( const void *arg1, const void *arg2 ){
	int a=*(int*)arg1;
	int b=*(int*)arg2;
	return dictfreq[b]-dictfreq[a];
}
void XWRT_Encoder::sortDict(int size){
	int i,add;
	size--;
	if (size<20)
	return;
	initializeCodeWords(size,false);
	add=0;
	dict1size-=add;
	bound3-=add;
	bound4-=add;
	int* inttable=new int[size];
	if (!inttable)
	OUT_OF_MEMORY();
	for (i=0; i<size; i++)
	inttable[i]=i+1;
	qsort(&inttable[0],size,sizeof(inttable[0]),compare_freq);
	qsort(&inttable[0],min(size,dict1size),sizeof(inttable[0]),compare_str); //compare_str
	
	if (size>dict1size)
	qsort(&inttable[dict1size],min(size,bound3)-dict1size,sizeof(inttable[0]),compare_str);//compare_str
	if (size>bound3)
	qsort(&inttable[bound3],min(size,bound4)-bound3,sizeof(inttable[0]),compare_str);//compare_str
	if (size>bound4)
	qsort(&inttable[bound4],size-bound4,sizeof(inttable[0]),compare_str);//compare_str
	
	for (i=0; i<size; i++){
		std::string str=(char*)dict[inttable[i]];
		sortedDict.push_back(str);
	}
	delete(inttable);
}
void XWRT_Encoder::WRT_detectFinish(){	
	int i,j;
	PRINT_DICT(("%d words ",sizeDict-1));
	sortedDict.clear();
	int num;
	int minWordFreq2;
	if (minWordFreq<6)
	minWordFreq2=minWordFreq;
	else
	minWordFreq2=minWordFreq-2;
	for (i=1; i<sizeDict-1; i++){
		num=dictfreq[i];
		if (num>=minWordFreq || (num>=minWordFreq2 && (dictlen[i]>=7))) 
		;
		else
		dictfreq[i]=0;
	}
	for (i=1, j=sizeDict-2; i<j; i++){
		if (dictfreq[i]>0)
		continue;
		while (j>0 && dictfreq[j]==0) j--;
		if (i>j)
		break;
		dict[i]=dict[j];
		dictlen[i]=dictlen[j];
		dictfreq[i]=dictfreq[j];
		dictfreq[j--]=0;
	}
	sizeDict=i;
	if (sizeDict>maxDictSize)
	sizeDict=maxDictSize;
	PRINT_DICT(("reduced to %d words (freq>=%d)\n",sizeDict,minWordFreq));
	sortDict(sizeDict);
}
