#include "XWRT.h"
#include "MemBuffer.h"
#include <stdlib.h> 
#include <memory.h>
#include "Common.h"

unsigned int CMemoryBuffer::memsize=0;
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}

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
	if (TgtPtr>TgtLen-1)
	{
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


inline void CMemoryBuffer::AllocSrcBuf( unsigned int len )
{
	SrcLen = len;
	SourceBuf = (unsigned char*) malloc(SrcLen);
	if (SourceBuf==NULL)
		OUT_OF_MEMORY();
}
	
inline void CMemoryBuffer::AllocTgtBuf( unsigned int len )
{
	TgtLen = len;
	TargetBuf = (unsigned char*) malloc(len+6);
	if (TargetBuf==NULL)
		OUT_OF_MEMORY();
	TargetBuf += 3;
}

inline void CMemoryBuffer::ReallocTgtBuf(unsigned int len)
{
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

void CContainers::prepareMemBuffers()
{
	memout=new CMemoryBuffer();
	std::pair<std::string,CMemoryBuffer*> p("!data",memout);
	memmap.insert(p);

	
}

void CContainers::writeMemBuffers(int preprocFlag)
{
	std::map<std::string,CMemoryBuffer*>::iterator it;

	int fileLen=0;
	int len=0;
	int lenCompr=0;
	int allocated=0;


	{
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
	}

	PRINT_DICT(("dataSize=%d compr=%d allocated=%d\n",len,lenCompr,allocated));

	freeMemBuffers(true);
	prepareMemBuffers();
}

void CContainers::readMemBuffers(int preprocFlag, int maxMemSize)
{
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

 	while (true)
	{	
		
		{
			GETC(i);

			if (i<=0)
				break;

			for (c=0; c<i; c++)
				GETC(s[c]);
		}
		

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


		
		{



			{
				int c;
				for (i=0, fileLen=0; i<4; i++)
				{
					GETC(c);
					fileLen=fileLen*256+c;
				}
		
				len+=fileLen;
				lenCompr+=fileLen;
				memout_tmp->AllocSrcBuf(fileLen);

				fread_fast(memout_tmp->SourceBuf,memout_tmp->SrcLen,XWRT_file);

				//printStatus(fileLen,0,false);
			}
		}

	}



	PRINT_DICT(("readMemBuffers() dataSize=%d compr=%d allocated=%d\n",len,lenCompr,maxMemSize+10240));
}

void CContainers::freeMemBuffers(bool freeMem)
{
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
