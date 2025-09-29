#include "wordinfo.hpp"
extern bool witmode;

    Info::Info(BlockData& bd,U32 val, ContextMap  &contextmap):x(bd),buf(bd.buf), cm(contextmap),
    word0(0),word1(0),word2(0), word3(0),word4(0),word5(0),word6(0),word7(0),
    wrdhsh(0),
    xword0(0),xword1(0),xword2(0),xword3(0),cword0(0),ccword(0),fword(0),
    number0(0),number1(0),hq(0),
    text0(0),data0(0),type0(0),nl3(-5),nl2(-4),nl1(-3), nl(-2),mask(0),mask2(0),wpos(0x10000),wchk(0x10000),inkeyw(0x10000),inkeyw1(0x10000),w(0),doXML(false),
  lastLetter(0),firstLetter(0), lastUpper(0),lastDigit(0), wordGap(0) ,StemWords(4),StemIndex(0),same(0),linespace(0),islink(0),istemplate(0),numbers(0),numbercount(0),
  g_ascii_lo(0), g_ascii_hi(0), c(0), pC(0), ppC(0),c4(0),ccount(0),col1(0),firstwasspace(false),opened(0),line0(0),
  iword0(0),lastSpace(0),pdft(false),chk(0),above(0), above1(0), above2(0),fl(0),f(0),scount(0),scountset(false),wpword1(0) {
      reset();
    }
    void Info::reset() { 
       cWord=&StemWords[0], pWord=&StemWords[3];expr0=expr1=expr2=expr3=expr4=exprlen0=expr0chars=wchar2=wrth=0;wchar1=1;
       is_letter=is_letter_pC=is_letter_ppC=false;
       
       wcol=wabove=0,wnl2,wnl1=wnl=-1;
    }
    // forget it
    void Info::shrwords() { 
        word1=word2;
        word2=word3;
        word3=word4;
        word4=word5;
        word5=word6;
        word6=word7;
        word7=0;
    }
    void Info::killwords() { 
    word1=word2=word2=word3=word3=word4=0;
        word4=word5=word5=word6=word6=word7=0;
        word7=0;
    }
    void Info::process_char(const int is_extended_char,int val1,int val2) {
        ppC=pC; pC=c;
        is_letter_ppC=is_letter_pC; is_letter_pC=is_letter;
        c=x.c4&255;
        c4=(c4<<8)|c;
        int i=0,d=0;
        if (f) {
            word5=word4;
            word4=word3;
            word3=word2;
            word2=word1;
            word1='.';
        }
        f=0;
        if (x.spaces&0x80000000) --x.spacecount;
        if (x.words&0x80000000) --x.wordcount;
        if (numbers&0x80000000) --numbercount;
        numbers=numbers*2;
        x.spaces=x.spaces*2;
        x.words=x.words*2;
        lastUpper=min(lastUpper+1,255);
        lastLetter=min(lastLetter+1,255);
        lastSpace=min(lastSpace+1,1024*4);
        mask2<<=2;
        if (is_extended_char>0 || is_extended_char==-1) wchar1=(wchar1<<8)|c;
        if (is_extended_char==0 && wchar1!=1 ) wchar2=wchar1,wchar1=1;
        if (!(x.filetype==DEFAULT || x.filetype==DECA || x.filetype==IMGUNK || x.filetype==EXE || x.filetype==ARM)){
           d=WRT_wrd1[c]; 
           x.pwords=x.pwords*2;   
           x.pbc=x.pbc*2+d/64;
           if ((d)) {x.pwords++, wpword1=(wpword1<<8)+c;}//hash(wpword1, c);}
           else wpword1=d; 
          // x.bc4=d<<2; 
        }
        if (pC>='A' && pC<='Z') pC+='a'-'A';
        if (c>='A' && c<='Z') c+='a'-'A', lastUpper=0;
        if (x.wrtLoaded==true && c==1 ) lastUpper=0;
        if ((c>='a' && c<='z') || c=='\'' || c=='-')
           (*cWord)+=c;
        else if ((*cWord).Length()>0){
           StemmerEN.Stem(cWord);
           StemIndex=(StemIndex+1)&3;
           pWord=cWord;
           cWord=&StemWords[StemIndex];
           memset(cWord, 0, sizeof(Word));
        }
        if ((c4&0xFFFF)==0x5b5b ) scountset=true;
        if (islink && (c==SPACE || c==']' || c==10  )) islink=0; //disable if not in link
        if (scountset &&  c=='|')  scountset=false; 
        if ((c4&0xFFFF)==0x5d5d && cword0==-1)cword0=0;//']]'
        if (scountset &&  (c==']' ||x.frstchar=='[') )  scountset=false,scount=cword0=0; 
        if (istemplate && (c4&0xffff)==0x7d7d) istemplate=0; //'}}'
        if ((val1==0 || val1==1)&& doXML==true) doXML=false; // None ReadTag
        else if (val1==5) doXML=true;                        // ReadContent
        //if (doXML==true) printf("%c",c);
        is_letter=((c>='a' && c<='z') ||(c>='0' && c<='9' && witmode==true) || x.wstat==true|| (c>=128 &&(x.b3!=3)|| (c>0 && c<4 )));
        if (is_letter) {// if ((c>='a' && c<='z')||/*(c>='0' && c<='9') ||*/ (c>=128 /*&&(x.b3!=3)*/|| (c>0 && c<4))) {
            if (!x.wordlen){
                // model syllabification with:
                //       "+"  //book1 case +\n +\r\n
                //       "="  quoted-printable 
                if ((lastLetter=3 && (c4&0xFFFF00)==0x2B0A00 && buf(4)!=0x2B) || (lastLetter=4 && (c4&0xFFFFFF00)==0x2B0D0A00 && buf(5)!=0x2B) ||
                    (lastLetter=3 && (c4&0xFFFF00)==0x2D0A00 && buf(4)!=0x2D) || (lastLetter=4 && (c4&0xFFFFFF00)==0x2D0D0A00 && buf(5)!=0x2D) ||
                    (lastLetter=3 && (c4&0xFFFF00)==0x3D0A00 && buf(4)!=0x3D) || (lastLetter=4 && (c4&0xFFFFFF00)==0x3D0D0A00 && buf(5)!=0x3D)  ){
                    word0=word1;
                    shrwords();
                    x.wordlen = x.wordlen1;
                    if (c<128){
                       StemIndex=(StemIndex-1)&3;
                       cWord=pWord;
                       pWord=&StemWords[(StemIndex-1)&3];
                       memset(cWord, 0, sizeof(Word));
                       for (U32 j=0;j<=x.wordlen;j++)
                           (*cWord)+=tolower(buf(x.wordlen-j+1+2*(j!=x.wordlen)));
                    }
                }else{
                      wordGap = lastLetter;
                      firstLetter = c;
                      wrdhsh = 0;
                }
                if (pC==QUOTE /*||   (x.wrtc4&0xffffffffffff)==0x2671756F743B*/) { opened=QUOTE;}
            }
            lastLetter=0;
            if (c>4) word0^=hash(word0, c,0);
            text0=text0*997*16+c;
            ++x.words, ++x.wordcount;
            x.wordlen++;
            x.wordlen=min(x.wordlen,45);
            if (x.wstat==true && x.wdecoded==true) wrth=wrth+x.utf8l>0;
        
        if (x.wstat==false ) wrth++;
            if ((c=='a' || c=='e' || c=='i' || c=='o' || c=='u') || (c=='y' && (x.wordlen>0 && pC!='a' && pC!='e' && pC!='i' && pC!='o' && pC!='u'))){
                mask2++;
                wrdhsh=wrdhsh*997*8+(c/4-22);
            }else if (c>='b' && c<='z'){
                mask2+=2;
                wrdhsh=wrdhsh*271*32+(c-97);
            }else
                wrdhsh=wrdhsh*11*32+c;
            f=0;
            w=word0&(wpos.size()-1);
            chk=U16(word0>>16);
        } else {
            if (word0) { 
                type0 = (type0<<2)|1;
                if (scountset) scount=min(scount+1,5);
                if (scountset==false && scount>0&& cword0!=-1) { // wiki: xxx xxx [[yyy yyy|xxx xxx]] -> remove yyy words
                    x.f4=x.f4>>(4*scount);
                    x.tt=x.tt>>(2*scount);
                    while (scount-->0) shrwords();
                    scount=0;
                }
                else if (cword0==-1) { // wiki: xxx xxx [[yyy: yyy|yyy yyy]] -> remove yyy words
                    x.f4=x.f4>>(4*1);
                    x.tt=x.tt>>(2*1);
                    shrwords();
                    scount=0;
                }else{ 
                    word7=word6;
                    word6=word5;     
                    word5=word4;
                    word4=word3;
                    word3=word2;
                    word2=word1;
                    word1=word0;
                    x.wordlen1=x.wordlen;
                }
                x.wordlen=0;
                inkeyw[w]=iword0;
                if (fword==0) fword=word0;
                
                wpos[w]=x.blpos;
                wrth=0;
                wchk[w]=chk;
                if (c==':'|| c=='=') cword0=word0;
                if ((c==':' && scountset==true && scount==1)) cword0=-1,shrwords(); // kill  [[xxx: ...]]
                if (c==']'&& (x.frstchar!=':') &&  doXML==true) xword0=word0; // wiki 
                ccword=0;
                word0= 0;                
                if((c=='.'||c=='!'||c=='?' ||c=='}' ||c==')') && buf(2)!=10 && x.filetype!=EXE) f=1; 
            }
            if (c==SPACE &&  buf(2)!=SPACE) linespace++; // count spaces in one line, skip repeats
            if (c==SPACE || c==10 || (c==5 && pC!='&')) { 
              if (c==SPACE )lastSpace=0,++x.spaces, ++x.spacecount; //else  
              if (pC=='.' || (ppC=='.' &&( pC==0x27 || pC==0x22))  ){
              x.tt=0xfffffff8, x.f4=0;
              } else   if  (pC=='?' || ((ppC=='?'|| ppC=='!')&&( pC==0x27 || pC==0x22))  ){
              x.tt=(x.tt&0xfffffff8)+1, x.f4=0;
              } 
               if (/*c==10 &&*/ linespace==0 && x.frstchar==0x5B ){
                   xword1=xword2=xword3=iword0=0;
               }
               if (c==10 || (c==5 && pC!='&')) fword=linespace=line0=0,nl3=nl2,nl2=nl1,nl1=nl, nl=buf.pos-1,wnl2=wnl1,wnl1=wnl,wnl=x.bufn.pos-1;
            }
            else if ((x.wrtc4&0xffffffff)==0x266C743B ||
                            (x.wrtc4&0xffffffff)==0x2667743B ||
                    (x.wrtc4&0xffffffffffff)==0x2671756F743B ||
                    (x.wrtc4&    0xffffffffff)==0x26616D703B ||
                                 x.wrtc4==0x26616D703B6C743B ||
                                 x.wrtc4==0x26616D703B67743B ||
                                 x.wrtc4==0x6D703B6E6273703B ||
                                 x.wrtc4==0x6D703B71756F743B ||
                                 x.wrtc4==0x703B6E646173683B ||
                                 x.wrtc4==0x703B6D646173683B) { 
                shrwords(); 
       
            }//'&lt;' '&gt;' " & 
            else if (c=='.' || c=='!' || c=='?' || c==',' || c==';' || c==':' || c=='|'|| c=='=') x.spafdo=line0=0,ccword=c,mask2+=3;
            else { ++x.spafdo; x.spafdo=min(63,x.spafdo); }
            if (ccword=='=' || (ccword==':' && doXML==false)) iword0=word1;
        }
        if ((c4&0xffffff)==0x3A2F2F) { // ://
            islink=1,word2=13;
        }
        if ((c4&0xffff)==0x7b7b) { //'{{'
            istemplate=1;
        }
        if (doXML==true){
            if ((c4&0xFFFF)==0x3D3D && x.frstchar==0x3d) xword1=word1; // ,xword2=word2; // == wiki
            if ((c4&0xFFFF)==0x2727) xword2=word1;                     //,xword2=word2; // '' wiki
            if ((c4&0xFFFF)==0x7D7D) xword3=word1;                     //}} wiki
        }
        lastDigit=min(0xFF,lastDigit+1);
        if (c>='0' && c<='9') {
            ++numbers, ++numbercount;
            if (buf(3)>='0' && buf(3)<='9' && (buf(2)=='.'||buf(2)==',')&& number0==0) {number0=number1; number1=0;} // 0.4645
            number0^=hash(number0, c,1);
            lastDigit = 0;
        }
        else if (number0) {
            type0 = (type0<<2)|2;
            number1=number0;
            number0=0,ccword=0;
        }
        line0^=hash(line0, buf(1));
        if (!((c>='a' && c<='z') ||(c>='0' && c<='9') || (c>=128 ))){
            data0^=hash(data0, c,1);
        }else if (data0) {
            type0 = (type0<<2)|3;
            data0=0;
            }
        col1=x.col;
        x.col=min(255, buf.pos-nl);
        if (x.dictonline==true){
        
        x.wcol=wcol= x.bufn.pos-wnl; // (wrt mode)
        if (wcol<0) x.wcol=wcol=2;
        wabove=x.bufn[wnl1+wcol];  // text column context, first (wrt mode)
        int lasllen=(wnl-wnl1);
        if (wcol>lasllen)wabove=0;
        }
        else x.wcol=wabove=0;
         above=buf[nl1+x.col];if (x.col>(nl-nl1)) above=0; // text column context, first
         above1=buf[nl2+x.col];if (x.col>(nl1-nl2)) above1=0; // text column context, second
         above2=buf[nl3+x.col];if (x.col>(nl2-nl3)) above2=0; // text column context, 3
        if (val2) x.col=val2,above=buf[buf.pos-x.col],above1=buf[buf.pos-x.col*2];;
        if (x.col<=2) {x.frstchar=(x.col==2?min(c,128):0); firstwasspace=x.frstchar==' '?true:false;        }
        if (x.col>2 && firstwasspace && (x.frstchar==' ' || x.frstchar<4)  && buf(1)!=' ') x.frstchar=c;
        if (x.col>2 && firstwasspace && !((x.frstchar>='a' && x.frstchar<='z') || (x.frstchar>='0' && x.frstchar<='9')|| (x.frstchar>=128 &&(x.b3!=3))) ) x.frstchar=c;  //scan for real char
        if (firstwasspace==false && x.frstchar=='[' && c==32)  { if(buf(3)==']' || buf(4)==']' ){ x.frstchar=96,xword0=0; } }
          fl = 0;
        if ((c4&0xff) != 0) {
            if (isalpha(c4&0xff)) fl = 1;
            else if (ispunct(c4&0xff)) fl = 2;
            else if (isspace(c4&0xff)) fl = 3;
            else if ((c4&0xff) == 0xff) fl = 4;
            else if ((c4&0xff) < 16) fl = 5;
            else if ((c4&0xff) < 64) fl = 6;
            else fl = 7;
        }
        mask = (mask<<3)|fl;
        //0x2671756F743B
        if (c=='(' || c=='{' || c=='[' || c=='<' || (x.wrtc4&0xffffffff)==0x266C743B/*|| x.wrtc4==0x26616D703B6C743B*/ ) { opened=c;}
        else if (c==')' || c=='}' || c==']' || c=='>' || c==QUOTE || c==APOSTROPHE ||
        (x.wrtc4&0xffffffff)==0x2667743B /*||
            x.wrtc4==0x26616D703B67743B||   (x.wrtc4&0xffffffffffff)==0x2671756F743B*/) {opened=0;}
      const bool is_newline_pC = pC==NEW_LINE || pC==0;
        if( (c>='a' && c<='z')|| c>=128 || (c>0 && c<4))  {
        expr0chars = expr0chars<<8|c; // last 4 consecutive letters
        expr0=hash(expr0,c);
        exprlen0=min(exprlen0+1,48);//maxwordlen
      } else {
        expr0chars=0;
        exprlen0=0;
        if( (c==SPACE || c==10 || c==0) && (is_letter_pC || pC==APOSTROPHE || pC==QUOTE) ) {//
          expr4=expr3;
          expr3=expr2;
          expr2=expr1;
          expr1=expr0;
          expr0=0;
        } else if(c==APOSTROPHE || c==QUOTE || ( (c==10 || c==0) && pC==SPACE) || (c==SPACE && is_newline_pC) ) {//
          //ignore
        } else {
          expr4=expr3=expr2=expr1=expr0=0;
        }
       
    }
    }

    int Info::predict(const U8 pdf_text_parser_state,int val1,int val2) {
      if (x.bpos==0) {
       int i=0;
       const U32 dist =  llog(x.blpos-wpos[word0&(wpos.size()-1)]);
       bool istext=!(x.filetype==DEFAULT || x.filetype==DECA || x.filetype==IMGUNK || x.filetype==EXE || x.filetype==ARM);
        x.tmask=i=((c<5)<<5)|((0)<<4)| ((dist==0)<<3)| ((/*doXML=true?1:*/0)<<2)| ((opened)<<6)| ((istemplate)<<1)| islink;
        if (x.filetype==DEFAULT || x.filetype==DECA || x.filetype==IMGUNK) i=0;
        else i=(hash(6,i+pdf_text_parser_state*1024)>>16);

 if (val2!=-1){
        //256+ hash 513+ none
        if ((pdf_text_parser_state!=0 || x.filetype==DECA|| x.filetype==EXE|| x.filetype==IMGUNK)){
          if (pdf_text_parser_state!=0 ){           
             if(exprlen0>=1) {
             const int wl_wme_mbc=min(exprlen0,1+3)<<2;
             cm.set(hash(++i, wl_wme_mbc, c));
             }      else {cm.set();i++;}

             if(exprlen0>=2) {
             const int wl_wme_mbc=min(exprlen0,2+3)<<2;
             cm.set(hash(++i, wl_wme_mbc, expr0chars&0xffff));
             }      else {cm.set();i++;}

             if(exprlen0>=3) {
             const int wl_wme_mbc=min(exprlen0,3+3)<<2;
             cm.set(hash(++i, wl_wme_mbc, expr0chars&0xffffff));
             }      else {cm.set();i++;}

             if(exprlen0>=4) {
             const int wl_wme_mbc=min(exprlen0,4+3)<<2;
             cm.set(hash(++i, wl_wme_mbc, expr0chars));
             }   else {cm.set();i++;}
             cm.set(hash(++i,x.spaces, (x.words&255), (numbers&255), (x.pwords&255)));//spaces...
             for(int j=0;j<5;j++)  {cm.set();++i;}
          } else for(int j=0;j<10;j++)  {cm.set();++i;}
           
        }
        else {
            cm.set(hash(++i,x.spafdo, x.spaces&0x7ff ,ccword));
            cm.set(hash(++i,line0,inkeyw[w]));
            cm.set(hash(++i,x.frstchar, c));
            cm.set(hash(++i,x.col, x.frstchar, (lastUpper<x.col)*4+(mask2&3)));//?
            cm.set(hash(++i,x.spaces, (x.words&255), (numbers&255), (x.pwords&255)));//spaces...
        
            //cm.set(x.spaces&0x7ff);
           /* cm.set(hash(++i,word0, word2,wordGap, word6));
            cm.set(hash(++i,word1, c,ccword));  
            cm.set(hash(++i,word0, word1,inkeyw[w]));  
            cm.set(hash(++i,word0, word6, lastDigit<wordGap+x.wordlen));
            cm.set(hash(++i,word0, cword0,inkeyw[word1&(inkeyw.size()-1)]));*/
            cm.set(hash(++i,number0, word1,wordGap, 0));
            cm.set(hash(++i,number1, c,ccword));  
            cm.set(hash(++i,number0, number1,inkeyw[w]));  
            cm.set(hash(++i,word0, 0, lastDigit<wordGap+x.wordlen));
            cm.set(hash(++i,number0, cword0,inkeyw[word1&(inkeyw.size()-1)]));
        }
        U32 h=x.wordcount*64+x.spacecount+numbercount*128;
        if (x.filetype!=DECA){
        if ( scountset==false /*&&!(x.frstchar=='{' && istemplate!=0)*/   ){
       
            cm.set(hash(++i,x.wordlen,wrth,x.col));
            cm.set(hash(++i,c,x.spacecount/2,wordGap));
           // U32 h=x.wordcount*64+x.spacecount+x.numbercount*128;
            cm.set(hash(++i,c,h,ccword));
            cm.set(hash(++i,x.frstchar,h,lastLetter));
            cm.set(hash(data0,word1, number1,type0&0xFFF));//cm.set(hash(data0,word1,word2, type0&0xFFF));
            cm.set(h);
            cm.set(hash(++i,h,x.spafdo)); 
            U32 d=c4&0xf0ff;
            cm.set(hash(++i,d,x.frstchar,ccword));
        }else {
            for(int j=0;j<8;j++)  {cm.set();++i;}
        }
        }
        h=word0*271;
        h=h+buf(1);U32 isfword=x.filetype==DECA?-1:fword;
        if (/*x.filetype==DEFAULT||*/x.filetype==DECA){
        cm.set(hash(++i,h));
        cm.set(hash(++i,word0,0)); 
        cm.set(hash(++i,data0));// ,iCtx()
        }else if (pdf_text_parser_state>0){
            cm.set(hash(h,pdf_text_parser_state));
        cm.set(hash(++i,word0)); 
        cm.set(hash(++i,data0));//  
            }else{
        cm.set(hash(h));
        cm.set(hash(++i,word0,inkeyw[w],x.tmask)); 
        cm.set(hash(++i,data0,inkeyw[w],x.tmask));//  
        }
        //if (wrdhsh) cm.set(hash(++i,wrdhsh,buf(wpos[word1&(wpos.size()-1)]))); else  {cm.set();++i;}
        cm.set(hash(++i,data0,c, word1)); 
        cm.set(hash(++i,h, word1)); 
        cm.set(hash(++i,word0, word1));
        cm.set(hash(++i,h, word1,word2/*,lastUpper<x.wordlen*/));
        cm.set(hash(++i,text0&0xffffff));
        cm.set(text0&0xfffff);
         
        if (doXML==true && pdf_text_parser_state==0){
            if ( scountset==false ){
            cm.set(hash(++i,word0,number0,wpword1,xword0));
            cm.set(hash(++i,word0,number0,wpword1, xword1));  //wiki 
            cm.set(hash(++i,word0,number0,wpword1, xword2));  //
            cm.set(hash(++i,word0,number0,wpword1, xword3));  //
            cm.set(hash(++i,x.frstchar, xword2,inkeyw[w]));
            }else {
            for(int j=0;j<5;j++)  {cm.set();++i;}
        }
        }else{
            //for(int j=0;j<5;j++)  {cm.set();++i;}
            cm.set(hash(++i+512,nl1));    //chars occurring in this paragraph (order 0)
     cm.set();++i;//cm.set(hash(++i+512,nl1,c));  //chars occurring in this paragraph (order 1)
      cm.set(hash(++i+512,x.frstchar));   //chars occurring in a paragraph that began with frstchar (order 0)
      //cm.set(hash(++i+512,x.frstchar,c)); //chars occurring in a paragraph that began with frstchar (order 1)*/
      if (lastSpace<1024*4)       cm.set(hash(++i+512,h, word1, word4)); else cm.set();++i;
      cm.set(hash(++i+512,x.col,x.wcol,nl1-nl));//cm.set();   ++i;
    //  cm.set(hash(expr0,expr1,expr2,expr3,expr4));
           
       // cm.set(hash(++i,expr0,expr1,expr2));
        }
        
        cm.set(hash(++i,word0, cword0,isfword));
        if (lastSpace<1024*4){
            if (witmode==true) cm.set(hash(++i,h, word2));else cm.set(hash(++i,h, word2,isfword));//,isfword fp.log
            cm.set(hash(++i,h, word3));
            cm.set(hash(++i,h, word4));
            cm.set((istext==true)?wpword1:hash(++i,h, word5));
            cm.set(hash(++i,h, word1,word3));
            cm.set(hash(++i,h, word2,word3));
        }else{
            for(int j=0;j<6;j++)  {cm.set();++i;}
        }
        if  (x.filetype!=DECA ){
            if (witmode==true  )  {
                cm.set( (fword));++i;
            cm.set(hash(++i,isfword,c));
            }else{
            
            cm.set(hash(++i,isfword));
            cm.set(hash(++i,isfword,c));}
        }
        cm.set(hash(++i,opened,c,dist!=0,pdf_text_parser_state));
        //cm.set(hash(++i,opened,word0));
        if (x.dictonline==true) cm.set(hash(buf(1),wchar1,wchar2)); else {cm.set();}
        cm.set(x.f4&0x00000fff);
        cm.set(x.f4); 
        
        const U8 pC_above = (x.col>(nl-nl1))?0: buf[nl1+x.col-1];
        const bool is_new_line_start = x.col==0 && nl1>0;
        const bool is_prev_char_match_above = buf(1)==pC_above && x.col!=0 && nl1!=0;
        const U32 above_ctx = above<<1|U32(is_prev_char_match_above);
        if (pdf_text_parser_state==0 &&!(x.frstchar=='{' && istemplate!=0) &&scountset==false && x.col<((U32)max(255,val2))&& x.filetype!=DECA){
            cm.set(hash(++i,((wnl1-wnl2)<<16)|nl1-nl2,(wcol<<8)|x.col,buf(1),above_ctx));  
            cm.set(hash(++i,buf(1),above_ctx,above^above1,wabove ));
            cm.set(hash(++i,x.col,wcol,buf(1))); 
            cm.set(hash(++i,x.col,wcol,c==32));  
        } 
        else {
          if (x.filetype!=DECA )
            for(int j=0;j<4;j++)  {cm.set();++i;}
        }
 
        if (x.wordlen) cm.set(hash(++i, word0, dist>>4));    
        else  {cm.set();++i;}
        if (x.wordlen)cm.set(hash(++i,word1, data0,dist>>2));   
        else {cm.set();++i;}
        cm.set(hash(++i,buf(1),word0,llog(x.blpos-wpos[word2&(wpos.size()-1)])>>2));
       
            cm.set(hash(++i,mask)); 
            cm.set(hash(++i,mask,buf(1))); 
            if (pdf_text_parser_state==0&&scountset==false &&!(x.frstchar=='{' && istemplate!=0))    cm.set(hash(++i,mask&0x1ff,x.col,wcol)); else  {cm.set();++i;}
            cm.set(hash(++i,mask,c4&0x00ffff00)); 
            cm.set(hash(++i,mask&0x1ff,x.f4&0x00fff0)); 
            cm.set(hash(++i,h, llog(wordGap), mask&0x1FF, ///// dis>30
             ((istemplate)<<9)|  
             ((islink)<<8)|
             ((linespace > 4)<<7)|
             ((x.wordlen1 > 3)<<6)|
             ((x.wordlen > 0)<<5)|
             ((x.spafdo == x.wordlen + 2)<<4)|
             ((x.spafdo == x.wordlen + x.wordlen1 + 3)<<3)|
             ((x.spafdo >= lastLetter + x.wordlen1 + wordGap)<<2)|
             ((lastUpper < lastLetter + x.wordlen1)<<1)|
             (lastUpper < x.wordlen + x.wordlen1 + wordGap)
             ));
      
        
        if (x.wordlen1 && pdf_text_parser_state==0&&scountset==false &&!(x.frstchar=='{' && istemplate!=0))            cm.set(hash((wcol<<8)|x.col,x.wordlen1,above,above1,c4&0xfF)); else cm.set(); //wordlist 
        if (wrdhsh)          cm.set(hash(++i,mask2&0x3F, wrdhsh&0xFFF, (0x100|firstLetter)*(x.wordlen<6),(wordGap>4)*2+(x.wordlen1>5)) ); else i++,cm.set();//?
        hq=hash((wcol<<8)|x.col,above^above1,numbers&127,x.filetype==DECA?c4&0xfF:llog(x.blpos-wpos[word0&(wpos.size()-1)]));
        if (  pdf_text_parser_state==0&&scountset==false &&!(x.frstchar=='{' && istemplate!=0)) cm.set(hash(++i,(wcol<<8)|x.col,above^above1,above2 , ((islink)<<8)|
             ((linespace > 4)<<7)|
             ((x.wordlen1 > 3)<<6)|
             ((x.wordlen > 0)<<5)|
             ((x.spafdo == x.wordlen + 2)<<4)|
             ((x.spafdo == x.wordlen + x.wordlen1 + 3)<<3)|
             ((x.spafdo >= lastLetter + x.wordlen1 + wordGap)<<2)|
             ((lastUpper < lastLetter + x.wordlen1)<<1)|
             (lastUpper < x.wordlen + x.wordlen1 + wordGap)));   else  {cm.set();++i;}
        cm.set(hash((*pWord).Hash[2], h));
        
 }
    }
    if (val2==-1) return 1;
    
    if(x.bpos==0) {

    U32 g=c4&0xff; // group identifier
         if('0'<=g && g<='9') g='0'; //all digits are in one group
    else if('A'<=g && g<='Z') g='A'; //all uppercase letters are in one group
    else if('a'<=g && g<='z' || x.wstat==true) g='a'; //all lowercase letters are in one group
    else if(g>=128) g=128;

    const bool to_be_collapsed = (g=='0' || g=='A' || g=='a') && g == (g_ascii_lo&0xff);
    if(!to_be_collapsed) {
      g_ascii_hi <<= 8;
      g_ascii_hi  |= g_ascii_lo>>(64-8);
      g_ascii_lo <<= 8;
      g_ascii_lo  |= g;
    }
 if( linespace>0 || x.dictonline==true) {
    U64 i = to_be_collapsed*8;//i=i*65;
    U32 g_a_hi=(g_ascii_lo>>32),g_a_lo=g_ascii_lo&0x00000000ffffffff;
    cm.set(hash( (++i), g_a_hi, g_a_lo, g_ascii_hi&0x00000000ffffffff ));  // last 12 groups
    cm.set(hash( (++i), g_a_hi, g_a_lo));                                  // last 8 groups
    cm.set(hash( (++i), g_a_hi&0x0000ffff, g_a_lo ));                      // last 6 groups
    cm.set(hash( (++i), g_a_lo));                                          // last 4 groups
    cm.set(hash( (++i), g_a_lo&0x0000ffff));                               // last 2 groups
    cm.set(hash( (++i), g_a_hi&0x00ffffff, g_a_lo , c4&0x0000ffff ));    // last 7 groups + last 2 chars
    cm.set(hash( (++i), g_a_hi&0x000000ff, g_a_lo , c4&0x00ffffff ));    // last 5 groups + last 3 chars
 } else for(int i=0;i<7;i++) cm.set();
  }
     return hq;
    }

