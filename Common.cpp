#include "XWRT.h"
#include "Common.h"
#include <stdio.h>

FILE* XWRT_file;
FILE* XWRT_fileout;
unsigned char** dict=NULL;
int* dictfreq=NULL;
unsigned char* dictlen=NULL;
XWRT_Common::XWRT_Common(int fileBufferSize) :  dictmem(NULL),
	detect(false), dictmem_end(NULL),
	fileCorrupted(false)
{ 
	if (fileBufferSize<10)
		fileBufferSize=10; // 1 KB
	if (fileBufferSize>23)
		fileBufferSize=23; // 8 MB
	word_hash=new int[HASH_TABLE_SIZE];
	if (!word_hash)
		OUT_OF_MEMORY();
}
XWRT_Common::~XWRT_Common()
{
	if (word_hash)
		delete(word_hash);
	WRT_deinitialize(); 
}
int XWRT_Common::minSpacesFreq()
{
	return 300+200*(fileLenMB/5);
}

// make hash from string
inline void XWRT_Common::stringHash(const unsigned char *ptr, int len,int& hash)
{
	for (hash = 0; len>0; len--, ptr++)
	{
		hash *= HASH_MULT;
		hash += *ptr;
	}
	hash=hash&(HASH_TABLE_SIZE-1);
}
int XWRT_Common::addWord(unsigned char* &mem,int &i)
{
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
unsigned char* XWRT_Common::loadDynamicDictionary(unsigned char* mem,unsigned char* mem_end)
{
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
	if (mem<mem_end)
	{
		i=strlen("http://www.");
		memcpy(mem,"http://www.",i);
		if (addWord(mem,i)!=0)
			mem+=(i/4+1)*4;
	}
	PRINT_DICT(("count=%d sortedDict.size()=%d\n",count,sortedDictSize));
	sizeDynDict=sizeDict;
	return mem;
}

void XWRT_Common::initializeLetterSet()
{
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
void XWRT_Common::initializeCodeWords(int word_count,bool initMem)
{
	int c,charsUsed,i;
	detectedSym=0;
	for (c=0; c<256; c++)
	{
		addSymbols[c]=0;
		codeword2sym[c]=0;
		sym2codeword[c]=0;
		reservedSet[c]=0;
		outputSet[c]=0;
	}
	for (c=0; c<256; c++)
	{
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
	
	for (c=0; c<256; c++)
	{
		if (reservedSet[c] || addSymbols[c])
			outputSet[c]=1;
	}
	charsUsed=0;
	for (c=0; c<256; c++)
	{
		if (addSymbols[c])
		{
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
	
	{
		dict4size-=dict3size;
		dict3size-=dict2size;
		dict2size-=dict1size;
		if (dict1size<4 || dict2size<4 || dict3size<4 || dict4size<4)
		{
			dict2size=dict3size=dict4size=charsUsed/4;
			dict1size=charsUsed-dict4size*3;
			for (i=0; i<charsUsed/4; i++)
			{
				if (i*i*i*(charsUsed-i*3)>c)
				{
					dict1size=charsUsed-i*3;
					dict2size=i;
					dict3size=i;
					dict4size=i;
					break;
				}
			}
		}
	}
	
		
		{
			dictionary=(dict1size*dict2size*dict3size*dict4size+dict1size*dict2size*dict3size+dict1size*dict2size+dict1size);
			bound4=dict1size*dict2size*dict3size+dict1size*dict2size+dict1size;
			bound3=dict1size*dict2size+dict1size;
			dict123size=dict1size*dict2size*dict3size;
			dict12size=dict1size*dict2size;
		}
	dict1plus2=dict1size+dict2size;
	dict1plus2plus3=dict1size+dict2size+dict3size;
	if (initMem)
	{
		dict=(unsigned char**)calloc(sizeof(unsigned char*)*(dictionary+1),1);
		dictlen=(unsigned char*)calloc(sizeof(unsigned char)*(dictionary+1),1);
		if (!dict || !dictlen)
			OUT_OF_MEMORY();
	}
	PRINT_DICT((" %d %d %d %d(%d) charsUsed=%d sizeDict=%d\n",dict1size,dict2size,dict3size,dict4size,dictionary,charsUsed,sizeDict));
}
// read dictionary from files to arrays
bool XWRT_Common::initialize(bool encoding)
{
	int i,c,fileLen;
	FILE* file;
	WRT_deinitialize();
	memset(&word_hash[0],0,HASH_TABLE_SIZE*sizeof(word_hash[0]));
	
	{
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
	}
	
	return true;
}
void XWRT_Common::WRT_deinitialize()
{
	if (dict)
	{
		free(dict);
		dict=NULL;
	}
	if (dictlen)
	{
		free(dictlen);
		dictlen=NULL;
	}
	if (dictmem)
	{
		free(dictmem);
		dictmem=NULL;
	}
	if (dictfreq)
	{
		free(dictfreq);
		dictfreq=NULL;
	}
	sizeDict=0;
}

void XWRT_Common::defaultSettings()
{
	RESET_OPTIONS;
	TURN_ON(OPTION_TRY_SHORTER_WORD);
	maxMemSize=8*1024*1024;
	maxDynDictBuf=8*4;
	maxDictSize=65535*32700;
	tryShorterBound=4;
	minWordFreq=7*2; //64;
	
//maxDictSize=	//e
//minWordFreq=  // f
//maxMemSize  maxMemSize*=1024*1024;//m
//maxDynDictBuf //b

}

size_t fread_fast(unsigned char* dst, int len, FILE* file)
{
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
size_t fwrite_fast(unsigned char* dst, int len, FILE* file)
{
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
