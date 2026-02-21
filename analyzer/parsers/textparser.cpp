#include "textparser.hpp"

TextParser::TextParser() {
    priority=4;
    Reset();
    inpos=0;
    name="text";
}

TextParser::~TextParser() {
}

// loop over input block byte by byte and report state
DetectState TextParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    
    // To small? 
    if (pos==0 && len<128 && last==false) return DISABLE;
    // Are we in new data block, if so reset inSize and restart
    if (inpos!=pos) {
        inSize=0,inpos=pos;
        i=pos;
    }
    
    while (inSize<len) {
        uint8_t c=data[inSize];
        buf0=(buf0<<8)+c;
        // Detect text, utf-8, eoltext and text0
        text.isLetter=tolower(c)!=toupper(c);
        text.countLetters+=(text.isLetter)?1:0;
        text.countNumbers+=(c>='0' && c<='9')?1:0;
        text.zeroRun+=(c==0?1:-text.zeroRun);
        if (text.zeroRun>16 && tp.validlength()>TEXT_MIN_SIZE)  tp.invalidCount+=TEXT_MAX_MISSES*TEXT_ADAPT_RATE;
        //text.isNumbertext=text.countLetters< text.countNumbers;
        text.isUTF8 = ((c!=0xC0 && c!=0xC1 && c<0xF5) && (
        (c<0x80) ||
        // possible 1st byte of UTF8 sequence
        ((buf0&0xC000)!=0xC000 && ((c&0xE0)==0xC0 || (c&0xF0)==0xE0 || (c&0xF8)==0xF0)) ||
        // possible 2nd byte of UTF8 sequence
        ((buf0&0xE0C0)==0xC080 && (buf0&0xFE00)!=0xC000) || (buf0&0xF0C0)==0xE080 || ((buf0&0xF8C0)==0xF080 && ((buf0>>8)&0xFF)<0xF5) ||
        // possible 3rd byte of UTF8 sequence
        (buf0&0xF0C0C0)==0xE08080 || ((buf0&0xF8C0C0)==0xF08080 && ((buf0>>16)&0xFF)<0xF5) ||
        // possible 4th byte of UTF8 sequence
        ((buf0&0xF8C0C0C0)==0xF0808080 && (buf0>>24)<0xF5)
        ));
        tp.countUTF8+=((text.isUTF8 && !text.isLetter && (c>=0x80))?1:0);
        if (text.lastNLpos==0 && c==NEW_LINE) text.lastNLpos=i;
        else if (text.lastNLpos>0 && c==NEW_LINE) {
            int tNL=i-text.lastNLpos;
            if (tNL<90 && tNL>45) 
            text.countNL++;          //Count if in range   
            else 
            text.totalNL+=tNL>3?1:0; //Total new line count
            text.lastNLpos=i;
        }
        text.lastNL=(c==NEW_LINE || c==CARRIAGE_RETURN ||c==10|| c==5)?0:text.lastNL+1;
        tp.set_number(text.countNumbers);
        if (((buf0>>8)&0xff) == CARRIAGE_RETURN && c==NEW_LINE)
                    tp.setEOLType(2); // mixed or LF-only
        uint32_t t=utf8_state_table[c];
        tp.UTF8State=utf8_state_table[256 + tp.UTF8State + t];
        if(tp.UTF8State==UTF8_ACCEPT|| text.isUTF8 ||(tp.invalidCount<TEXT_ADAPT_RATE*2 && c==0 && tp.validlength()>(TEXT_MIN_SIZE/2))  ) { // proper end of a valid utf8 sequence
           // if (c==NEW_LINE || c==5) {
                //  if (((buf0>>8)&0xff) == CARRIAGE_RETURN)
                //    tp.setEOLType(2); // mixed or LF-only
                //  else 
                //    if (tp.EOLType()==0)tp.setEOLType(1); // CRLF-only
                //if (tp.invalidCount) tp.invalidCount=0;
           // }
            
            if (tp.invalidCount) tp.invalidCount=(tp.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE);
            if (tp.invalidCount==0) {
                tp.setEOLType(text.countNL>text.totalNL);
                tp.setend(i); // a possible end of block position
                if (tp.validlength()>TEXT_ADAPT_RATE) {
                    jstart=tp.start();
                    state=INFO;
                }
            }
            if (text.zeroRun>0 && tp.validlength()==text.zeroRun) state=NONE,tp.reset(i+1);
        } else if (tp.UTF8State==UTF8_REJECT) { // illegal state
            //if (tp.validlength())printf("Lenght %d invalid %d Start %d end %d\n",tp.validlength(),tp.invalidCount,tp.start(),tp.end()); 
            if (tp.invalidCount >= TEXT_ADAPT_RATE*2) {
                if (tp.validlength()>TEXT_MIN_SIZE && text.countLetters>(TEXT_MIN_SIZE/2)) {
                    //printf("Start %d end %d\n",tp.start(),tp.end());
                    jstart=tp._start[0];
                    if (text.zeroRun>=(tp._end[0]-tp._start[0])) {
                        state=NONE;
                        jstart=jend=0;
                        tp.reset(i+1);
                        memset(&text,0,sizeof(TextInfo));
                        continue;
                    }
                    jend=tp._end[0]+1-text.zeroRun;
                    uint64_t end=tp._end[0]+1; 
                    uint64_t nsize=tp.number();
                    type=(tp._EOLType[0]==1)?EOLTEXT:TEXT;
                    if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
                    if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
                    state=END;
                    tp.reset(0);
                    memset(&text,0,sizeof(TextInfo));
                    return state;
                }
                tp.reset(i+1);
                memset(&text,0,sizeof(TextInfo));
            } else if (tp.validlength()>TEXT_ADAPT_RATE) {
                jstart=tp.start();
                state=INFO;
            }
            tp.invalidCount=tp.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE + TEXT_ADAPT_RATE/2;
            tp.UTF8State=UTF8_ACCEPT; // reset state
            tp.countUTF8/=2;
        }
        if (state==INFO) {
            if (((inSize+1)==len && last==true && tp.invalidCount<TEXT_ADAPT_RATE) || ((tp._end[0]-tp._start[0]+1) >= TEXT_MIN_SIZE && tp.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE)) {
                jstart=tp._start[0];
                if (text.zeroRun>=(tp._end[0]-tp._start[0])) {
                        state=NONE;
                        jstart=jend=0;
                        tp.reset(i+1);
                        memset(&text,0,sizeof(TextInfo));
                        continue;
                }
                jend=tp._end[0]+1-text.zeroRun;
                uint64_t end=tp._end[0]+1; 
                uint64_t nsize=tp.number();
                type=(tp._EOLType[0]==1)?EOLTEXT:TEXT;
                if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
                if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
                state=END;
                
                if (text.zeroRun>=TEXT_ADAPT_RATE  || (jend-jstart)<TEXT_MIN_SIZE && last==false) {
                    tp.reset(i+1);
                    state=NONE;
                    memset(&text,0,sizeof(TextInfo));
                } else {
                    memset(&text,0,sizeof(TextInfo));
                    return state;
                }
            }
        }
        if (state==INFO && i>0x2000000) { // At 32mb switch to 0 priority
            priority=0;
        }
        inSize++;
        i++;
    }

    if (state==INFO) return INFO;
    // Are we still reading data for our type
    if (state!=NONE)
    return DATA;
    else return NONE;
}

dType TextParser::getType() {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

void TextParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=0;
    info=i=inSize=0;
    priority=4;
    //txtStart=binLen=spaces=txtLen=0;
    //txtMinLen=65536;
    memset(&text,0,sizeof(TextInfo));
    buf0=0;
    tp.reset(0);
    priority=4;
}
