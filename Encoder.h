#if !defined encoder_h_included
#define encoder_h_included

#include "Common.h"


class XWRT_Encoder : public XWRT_Common
{
public:

	XWRT_Encoder();
	~XWRT_Encoder();

	void WRT_start_encoding(FILE* in, FILE* out,unsigned int fileLen,bool type_detected);

private:

	void WRT_encode(size_t bufferSize);
	inline void encodeCodeWord(int &i);
	inline void encodeSpaces();
	inline void encodeWord(unsigned char* s,int s_size,EWordType wordType,int& c);
	inline void encodeCodeWord_PPM(int &i);
	inline void encodeAsText(unsigned char* &s,int &s_size,EWordType wordType);
	inline int findShorterWord(unsigned char* &s,int &s_size);
	inline void toLower(unsigned char* s,int &s_size);
	inline void toUpper(unsigned char* s,int &s_size);
	inline void checkWord(unsigned char* &s,int &s_size,int& c);
	inline void setSpaces(int c);
	inline void checkHashExactly(unsigned char* &s,int &s_size,int& i);
	inline int checkHash(unsigned char* &s,int &s_size,int h);
	inline void stringHash(const unsigned char *ptr, int len,int& hash);
	inline int unicode2utf8(unsigned int cp, unsigned char* result);
	void encodeMixed(unsigned char* s,int s_size,int& c);
	void sortDict(int size);

	void write_dict(int comprLevel);
	int WRT_detectFileType();
	void WRT_detectFinish();
	void WRT_get_options(int& c,int& c2);
	inline void readGetcBuffer(FILE* &file,int &c);

	unsigned char utf8buff[4];
	int utf8cached;
	int utf8pos;

	int s_size,binCount;
	int last_c_bak,last_c,last_last_c,quotes,lastAll;
	//EXMLState XMLState;

	unsigned char* getcBufferData;
	size_t getcBufferSize;
	size_t getcBufferSizeBak;
	int	getcBufferDataParts;
	unsigned char* getcBuffer;
	

	unsigned char* dynmem;
	unsigned char *dictbound;

public:




}; // end class 

int compare_freq( const void *arg1, const void *arg2 );

#endif
