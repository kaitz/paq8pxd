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
        uint32_t t=utf8_state_table[c];
        tp.UTF8State=utf8_state_table[256 + tp.UTF8State + t];

        if(tp.UTF8State==UTF8_ACCEPT) { // proper end of a valid utf8 sequence
            if (c==NEW_LINE || c==5) {
                //  if (((buf0>>8)&0xff) != CARRIAGE_RETURN)
                //    tp.setEOLType(2); // mixed or LF-only
                //  else 
                //    if (tp.EOLType()==0)tp.setEOLType(1); // CRLF-only
                //if (tp.validlength()>TEXT_MIN_SIZE*64) brute=false; //4mb
                if (tp.invalidCount) tp.invalidCount=0;
            }
            if (tp.invalidCount) tp.invalidCount=(tp.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE);
            
            if (tp.invalidCount==0) {
                tp.setEOLType(text.countNL>text.totalNL);
                tp.setend(i); // a possible end of block position
            }
        } else if (tp.UTF8State==UTF8_REJECT) { // illegal state
            if(text.totalNL/(text.countNL+1)==0) tp.invalidCount=0;
            tp.invalidCount=tp.invalidCount*(TEXT_ADAPT_RATE-1)/TEXT_ADAPT_RATE + TEXT_ADAPT_RATE;
            tp.UTF8State=UTF8_ACCEPT; // reset state
            if (tp.validlength()<TEXT_MIN_SIZE) {
                // printf("%d",tp.validlength());
                tp.reset(i+1); // it's not text (or not long enough) - start over
                text.countNumbers=0;
                state=NONE;
            }
            else if (tp.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE) {
                if (tp.validlength()<TEXT_MIN_SIZE)
                {  tp.reset(i+1); // it's not text (or not long enough) - start over
                    text.countNumbers=0;state=NONE;
                    }
                else // Commit text block validation
                {
                    tp.next(i+1);
                    uint64_t textstart=tp._start[0];
                    uint64_t textend=tp._end[0];
                    type=(tp._EOLType[0]==1)?EOLTEXT:TEXT;
                    //type=TEXT;
                    jstart=tp._start[0];
                    jend=tp._end[0];
                    uint64_t end=tp._end[0]+1; 
                    uint64_t nsize=tp.number();
                    if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
                    if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
                    state=END;
                    tp.reset(0);
                    memset(&text,0,sizeof(TextInfo));
                    return state;
                }
            }
        }
        
        
        //if(textend>end-1)textend=end-1;
        if(type==DEFAULT) {
            uint64_t textstart=tp._start[0];
            uint64_t textend=tp._end[0];
            if( textstart<textend) { // only DEFAULT blocks may be overridden
                uint64_t nsize=0;
                
                /* if(textstart==begin && textend == end-1) { // whole first block is text
        type=(tp._EOLType[0]==1)?EOLTEXT:TEXT; // DEFAULT -> TEXT
        uint64_t nsize=tp.number();
        if (nsize>(((end-begin)-nsize)>>1))type=TEXT0;
        //if (type==TEXT && tp.countUTF8>0xffff) type=TXTUTF8;
    }
    else*/ if (textend - textstart + 1 >= TEXT_MIN_SIZE && text.totalNL!=0 && text.totalNL!=0 ) { // we have one (or more) large enough text portion that splits DEFAULT
                    if ((text.countLetters+text.countNumbers+text.countUTF8)*2< TEXT_MIN_SIZE){
                    memset(&text,0,sizeof(TextInfo));
                    tp.reset(i+1);
                }else
                    /* if (textstart != begin) { // text is not the first block 
        end=textstart; // first block is still DEFAULT
        nextblock_type_bak=nextType;
        nextType=(tp._EOLType[0]==1)?EOLTEXT:TEXT; //next block is text
        //if (tp.number()>((end-begin)>>1))nextblock_type=TEXT0;
        nsize=tp.number();
        if (nsize>(((end-begin)-nsize)>>1))nextType=TEXT0; 
        tp.removefirst();
        } else*/ {
                        type=(tp._EOLType[0]==1)?EOLTEXT:TEXT; // first block is text
                        //type=TEXT;
                        uint64_t end=textend+1; 
                        nsize=tp.number();
                        if (nsize>(((end)-nsize)>>1))type=TEXT0;
                        state=INFO;
                    }
                }
                if (type==TEXT && tp.countUTF8>0xffff) type=TXTUTF8,info=0;
            }
        } 
        if (state==INFO) {
            if (((inSize+1)==len && last==true) || ((tp._end[0] - tp._start[0] + 1) >= TEXT_MIN_SIZE && tp.invalidCount >= TEXT_MAX_MISSES*TEXT_ADAPT_RATE)) {
                jstart=tp._start[0];
                jend=jstart+tp._end[0]+1;
                uint64_t end=tp._end[0]+1; 
                uint64_t nsize=tp.number();
                if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
                if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
                state=END;
                tp.reset(0);
                memset(&text,0,sizeof(TextInfo));
                return state;
            }
        }
        if (text.missCount>MAX_TEXT_MISSES) {
            jstart=tp._start[0];
            jend=jstart+tp._end[0];
            state=END;
            return state;
       /* } else if (state==INFO && i>( 0x40000000 )) { // Split at 1GB
            type=TEXT;
            jstart=tp._start[0];
            jend=jstart+tp._end[0]+1;
            uint64_t end=tp._end[0]+1; 
            uint64_t nsize=tp.number();
            if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
            if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
            state=END;
            tp.reset(0);
            memset(&text,0,sizeof(TextInfo));
            return state;*/
        } /*else if (text.totalNL!=0 && text.totalNL!=0 && state==INFO && i>0x2000000) { // At 32mb switch to 0 priority
            priority=0;
            printf("%d",tp.validlength());
        }*/
        inSize++;
        i++;
    }
    // Test if whole file was text
    /*if (len==inSize && last && tp._start[0]==0 && (tp._start[0]+tp._end[0]+1)==i && tp.invalidCount < TEXT_MAX_MISSES*TEXT_ADAPT_RATE){
        jstart=tp._start[0];
        jend=jstart+tp._end[0]+1;
        uint64_t nsize=tp.number();
        type=TEXT;
        if (nsize>((tp._end[0]-nsize)>>1)) type=TEXT0;
        if (tp.countUTF8>0xffff) type=TXTUTF8,info=0;
        state=END;
        //printf("Whole file text\n");
        return state;
    }*/
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
    priority=3;
    //txtStart=binLen=spaces=txtLen=0;
    //txtMinLen=65536;
    memset(&text,0,sizeof(TextInfo));
    buf0=0;
    tp.reset(0);
    priority=4;
}
