#include "word.hpp"
//////////////////////////// wordModel /////////////////////////

// Model English text (words and columns/end of line)
  wordModel1::wordModel1( BlockData& bd,U32 val): x(bd),buf(bd.buf),N(64+7),cm(CMlimit(MEM()*val), N,M_WORD),cm1(CMlimit(MEM()), 1,M_WORD),pdf_text_parser_state(0),math_state(0),pre_state(0),
  info_normal(bd,0,cm), info_pdf(bd,0,cm), math(bd,0,cm),pre(bd,0,cm),xhtml(bd,0,cm),hq(0){
  
   }
  

  
   int wordModel1::p(Mixer& m,int val1,int val2)  {
    if (x.bpos==0) {
      const U8 c1=x.c4;
      //extract text from pdf
      if((x.wrtc4&0xffffffff)==0x0A62740A&& x.wrtstatus==0 && x.wrtLoaded==true||x.c4==0x0a42540a) {//  "\nBT\n"  "\nbt\n" 
          pdf_text_parser_state=1;x.inpdf=true;
      } // Begin Text
      else if ((x.wrtc4&0xffffffff)==0x0A65740A && x.wrtstatus==0 && x.wrtLoaded==true||x.c4==0x0a45540a) {// "\nET\n" "\net\n" 
          pdf_text_parser_state=0;x.inpdf=false;
      } // End Text
      bool do_pdf_process=true;
      if(pdf_text_parser_state!=0) {
        const U8 pC=x.c4>>8;
        if(pC!='\\') {
             if(c1=='[') {pdf_text_parser_state|=2;} //array begins
        else if(c1==']') {pdf_text_parser_state&=(255-2);}
        else if(c1=='(') {pdf_text_parser_state|=4; do_pdf_process=false;} //signal: start text extraction
        else if(c1==')') {pdf_text_parser_state&=(255-4);} //signal: start pdf gap processing
        }
      }
      // ':&<math& >' '|&<math& >' ' &<math& >' '\n&<math& >'
      if(x.wrtc4==0x3A263C6D61746826|| x.wrtc4==0x20263C6D61746826||x.wrtc4==0x10263C6D61746826||/*(x.wrtc4&0xffffffff)==0x6D617468||*/ x.wrtc4==0x7C263C6D61746826 ) {//  
          math_state=2;         
      } // Begin math
      else if (math_state &&(x.wrtc4&0xff)==0x26  ||(x.wrtc4&0xff)==0x20) {//  
          math_state=0; 
      } // End math
      //else if (math_state==1 && ((x.c4>>8)&0xff)=='>') {//  
      //    math_state=2; 
      //} // End math
      //&<pre&>
      if((x.wrtc4&0xffffffffffffff)==0x263C707265263E ) {//  
          pre_state=4; 
      } // Begin pre
      else if (pre_state &&(x.wrtc4&0xffffff)==0x263C2F  ) {// '</p' 
          pre_state=0; 
      } // End pre
      const bool is_pdftext = (pdf_text_parser_state&4)!=0;
      if(is_pdftext) {
        if(do_pdf_process) {
          //printf("%c ",c1);  //debug: print the extracted pdf text
          const bool is_extended_char =x.wrtstatus==-1 || c1>=128|| (c1 <4 && c1>0 && x.wrtLoaded==true && x.wrtstatus==0);
          info_pdf.process_char(x.wrtstatus,val1,val2);
        }
        hq=info_pdf.predict(pdf_text_parser_state,val1,val2);
      } else   if(math_state==2) {
          math.process_char(x.wrtstatus,val1,val2);
          hq=math.predict(math_state,val1,val2);
      } else   if(pre_state==4) {
          pre.process_char(x.wrtstatus,val1,val2);
          hq=pre.predict(pre_state,val1,val2);
      } else   if( x.istex==false && x.ishtml==true) {
          xhtml.process_char(x.wrtstatus,val1,val2);
          hq=xhtml.predict(0,val1,val2);
      } else {
        const bool is_textblock = (x.filetype==TEXT||x.filetype==TEXT0|| x.filetype==TXTUTF8||x.filetype==EOLTEXT||x.filetype==DICTTXT||x.filetype==BIGTEXT||x.filetype==NOWRT);
        const bool is_extended_char = is_textblock && c1>=128;
        info_normal.process_char(x.wrtstatus,val1,val2);
        hq=info_normal.predict(pdf_text_parser_state,val1,val2);
      }      
      cm1.set(x.w4);
    }
    cm.mix(m);
    cm1.mix(m);
    return hq;
  }
  
