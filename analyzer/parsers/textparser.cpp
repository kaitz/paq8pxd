#include "textparser.hpp"

TextParser::TextParser() {
    priority=3;
    Reset();
    inpos=0;
    name="text";
}

TextParser::~TextParser() {
}

// loop over input block byte by byte and report state
DetectState TextParser::Parse(unsigned char *data, uint64_t len, uint64_t pos, bool last) {
    
    // To small? 
    if (pos==0 && len<128) return DISABLE;
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
        //text.countLetters+=(text.isLetter)?1:0;
        text.countNumbers+=(c>='0' && c<='9')?1:0;
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
        textparser.countUTF8+=((text.isUTF8 && !text.isLetter && (c>=0x80))?1:0);
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
        textparser.set_number(text.countNumbers);
        uint32_t t=utf8_state_table[c];
        textparser.UTF8State=utf8_state_table[256 + textparser.UTF8State + t];

        if(textparser.UTF8State==UTF8_ACCEPT) { // proper end of a valid utf8 sequence
            if (c==NEW_LINE || c==5) {
                //  if (((buf0>>8)&0xff) != CARRIAGE_RETURN)
                //    textparser.setEOLType(2); // mixed or LF-only
                //  else 
                //    if (textparser.EOLType()==0)textparser.setEOLType(1); // CRLF-only
                //if (textparser.validlength()>TEXT_MIN_SIZE*64) brute=false; //4mb
                if (textparser.invalidCount) textparser.invalidCount=0;
            }
            if (textparser.invalidCount) textparser.invalidCount=(textparser.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE);
            
            if (textparser.invalidCount==0) {
                textparser.setEOLType(text.countNL>text.totalNL);
                textparser.setend(i); // a possible end of block position
            }
        } else if (textparser.UTF8State==UTF8_REJECT) { // illegal state
            if(text.totalNL/(text.countNL+1)==0) textparser.invalidCount=0;
            textparser.invalidCount=textparser.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE + TEXT_ADAPT_RATE;
            textparser.UTF8State=UTF8_ACCEPT; // reset state
            if (textparser.validlength()<TEXT_MIN_SIZE) {
                // printf("%d",textparser.validlength());
                textparser.reset(i+1); // it's not text (or not long enough) - start over
                text.countNumbers=0;
            }
            else if (textparser.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE) {
                if (textparser.validlength()<TEXT_MIN_SIZE)
                {  textparser.reset(i+1); // it's not text (or not long enough) - start over
                    text.countNumbers=0;}
                else // Commit text block validation
                {
                    textparser.next(i+1);
                    uint64_t textstart=textparser._start[0];
                    uint64_t textend=textparser._end[0];
                    //type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT;
                    type=TEXT;
                    jstart=textparser._start[0];
                    jend=jstart+textparser._end[0];
                    uint64_t end=textparser._end[0]+1; 
                    uint64_t nsize=textparser.number();
                    if (nsize>((textparser._end[0]-nsize)>>1)) type=TEXT0;
                    if (textparser.countUTF8>0xffff) type=TXTUTF8,info=0;
                    state=END;
                    textparser.reset(0);
                    memset(&text,0,sizeof(TextInfo));
                    return state;
                }
            }
        }
        
        
        //if(textend>end-1)textend=end-1;
        if(type==DEFAULT) {
            uint64_t textstart=textparser._start[0];
            uint64_t textend=textparser._end[0];
            if( textstart<textend) { // only DEFAULT blocks may be overridden
                uint64_t nsize=0;
                /* if(textstart==begin && textend == end-1) { // whole first block is text
        type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // DEFAULT -> TEXT
        uint64_t nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        //if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8;
    }
    else*/ if (textend - textstart + 1 >= TEXT_MIN_SIZE) { // we have one (or more) large enough text portion that splits DEFAULT
                    /* if (textstart != begin) { // text is not the first block 
        end=textstart; // first block is still DEFAULT
        nextblock_type_bak=nextType;
        nextType=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; //next block is text
        //if (textparser.number()>((end-begin)>>1))nextblock_type=TEXT0;
        nsize=textparser.number();
        if (nsize>(((end-begin)-nsize)>>1))nextType=TEXT0; 
        textparser.removefirst();
        } else*/ {
                        //type=(textparser._EOLType[0]==1)?EOLTEXT:TEXT; // first block is text
                        type=TEXT;
                        uint64_t end=textend+1; 
                        nsize=textparser.number();
                        if (nsize>(((end)-nsize)>>1))type=TEXT0;
                        state=INFO;
                    }
                }
                if (type==TEXT && textparser.countUTF8>0xffff) type=TXTUTF8,info=0;
            }
        } 
        if (state==INFO) {
            if ((inSize+1)==len && last==true || ((textparser._end[0] - textparser._start[0] + 1) >= TEXT_MIN_SIZE && textparser.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE)) {
                jstart=textparser._start[0];
                jend=jstart+textparser._end[0]+1;
                uint64_t end=textparser._end[0]+1; 
                uint64_t nsize=textparser.number();
                if (nsize>((textparser._end[0]-nsize)>>1)) type=TEXT0;
                if (textparser.countUTF8>0xffff) type=TXTUTF8,info=0;
                state=END;
                textparser.reset(0);
                memset(&text,0,sizeof(TextInfo));
                return state;
            }
        }
        if (text.missCount>MAX_TEXT_MISSES) {
            jstart=textparser._start[0];
            jend=jstart+textparser._end[0];
            state=END;
            return state;
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

dType TextParser::getType(int i) {
    dType t;
    t.start=jstart;     // start pos of type data in block
    t.end=jend;       // end pos of type data in block
    t.info=info;      // info of the block if present
    t.rpos=0;      // pos where start was set in block
    t.type=type;
    t.recursive=false;
    return t;
}

int TextParser::TypeCount() {
    return 1;
}

void TextParser::Reset() {
    state=NONE,type=DEFAULT,jstart=jend=0;
    info=i=inSize=0;
    //txtStart=binLen=spaces=txtLen=0;
    //txtMinLen=65536;
    memset(&text,0,sizeof(TextInfo));
    buf0=0;
    textparser.reset(0);
}

void TextParser::SetEnd(uint64_t e) {
    jend=e;
}
