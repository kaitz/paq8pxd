#if !defined membuffer_h_included
#define membuffer_h_included

#pragma warning(disable:4786)

#include <stdio.h>
#include <vector>
#include <string>
#include <map>

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

#endif
