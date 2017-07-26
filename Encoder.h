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

#endif
