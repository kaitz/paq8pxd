#if !defined decoder_h_included
#define decoder_h_included

#include "Common.h"


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

#endif
