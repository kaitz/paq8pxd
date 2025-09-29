#include "text.hpp"




  TextModel::TextModel(BlockData& bd, U64 Size) : N(37),buffer(bd.buf),  
  Map(CMlimit(MEM()*Size), N,M_TEXT,
  CM_RUN2+
  CM_RUN1+
  CM_MAIN1+
  CM_MAIN2+
  CM_MAIN3+
  CM_MAIN4+
  CM_M12
  ), Stemmers(Language::Count-1), Languages(Language::Count-1),
   WordPos(0x10000), State(Parse::Unknown), pState(State), Lang{ 0, 0, Language::Unknown, Language::Unknown }, Info{ 0 }, ParseCtx(0),doXML(false),firstwasspace(false) {
    Stemmers[Language::English-1] = new EnglishStemmer();
    Stemmers[Language::French-1] = new FrenchStemmer();
    Stemmers[Language::German-1] = new GermanStemmer();
    Languages[Language::English-1] = new English();
    Languages[Language::French-1] = new French();
    Languages[Language::German-1] = new German();
    cWord = &Words[Lang.Id](0);
    pWord = &Words[Lang.Id](1);
    cSegment = &Segments(0);
    cSentence = &Sentences(0);
    cParagraph = &Paragraphs(0);
    memset(&BytePos[0], 0,  sizeof(BytePos));
    memset(&Info, 0, sizeof(Info));
 }

  void TextModel::setword(U8 *w,int len)     {
      for (int i=0;i<len;i++) xWord+=w[i];
  }

  int TextModel::p(Mixer& mixer,int val1, int val2) {
    if (mixer.x.bpos==0) {
        if ((val1==0 || val1==1)&& doXML==true) doXML=false; // None ReadTag
        else if (val1==5) doXML=true;                        // ReadContent
      Update(buffer,mixer);
      SetContexts(buffer, mixer);
    }
    if (val2==-1) return 1;
    Map.mix(mixer);
    mixer.set(hash((Lang.Id!=Language::Unknown)?1+Stemmers[Lang.Id-1]->IsVowel(buffer(1)):0, Info.masks[1]&0xFF, mixer.x.c0)&0x3FF, 1024);
    mixer.set(hash(ilog2(Info.wordLength[0]+1), mixer.x.c0,
      (Info.lastDigit<Info.wordLength[0]+Info.wordGap)|
      ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
      ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<2)|
      ((Info.lastUpper<Info.wordLength[0])<<3)|
      ((Info.islink)<<4)|
      ((Info.istemplate)<<5)
    )&0x7FF, 2048);
    mixer.set(hash(Info.masks[1]&0x3FF, mixer.x.grp, Info.lastUpper<Info.wordLength[0], Info.lastUpper<Info.lastLetter+Info.wordLength[1])&0xFFF, 4096);
        mixer.set(hash(Info.spaces&0x1FF, mixer.x.grp,
      (Info.lastUpper<Info.wordLength[0])|
      ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
      ((Info.lastPunct<Info.lastLetter)<<2)|
      ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<3)|
      ((Info.lastPunct<Info.lastLetter+Info.wordLength[1]+Info.wordGap)<<4)|
      ((Info.linespace>4)<<5)
    )&0xFFF, 4096);
    mixer.set(hash(Info.firstLetter*(Info.wordLength[0]<4), min(6, Info.wordLength[0]), mixer.x.c0)&0x7FF, 2048);
    mixer.set(hash((*pWord)[0], (*pWord)(0), min(4, Info.wordLength[0]), Info.lastPunct<Info.lastLetter)&0x7FF, 2048);
    mixer.set(hash(min(4, Info.wordLength[0]),mixer.x.grp,
      Info.lastUpper<Info.wordLength[0],
      (Info.nestHash>0)?Info.nestHash&0xFF:0x100|(Info.firstLetter*(Info.wordLength[0]>0 && Info.wordLength[0]<4))
    )&0xFFF, 4096);
    mixer.set(hash(mixer.x.grp, Info.masks[4]&0x1F, (Info.masks[4]>>5)&0x1F)&0x1FFF, 8192);
    
    return 0;
  }


void TextModel::Update(Buf& buffer,Mixer& mixer) {
    
  Info.lastUpper  = min(0xFF, Info.lastUpper+1), Info.maskUpper<<=1;
  Info.lastLetter = min(0x1F, Info.lastLetter+1);
  Info.lastDigit  = min(0xFF, Info.lastDigit+1);
  Info.lastPunct  = min(0x3F, Info.lastPunct+1);
  Info.lastNewLine++, Info.prevNewLine++, Info.lastNest++;
  Info.spaceCount-=(Info.spaces>>31), Info.spaces<<=1;
  Info.masks[0]<<=2, Info.masks[1]<<=2, Info.masks[2]<<=4, Info.masks[3]<<=3, Info.masks[4]<<=5;
  pState = State;  

  U8 c = buffer(1), pC=tolower(c), g = (c<0x80)?AsciiGroup[c]:31;
  if (g>4 || g!=(Info.masks[4]&0x1F))
    Info.masks[4]<<=5, Info.masks[4]|=g;
  BytePos[c] = mixer.x.blpos;
  if (c!=pC || mixer.x.wrtLoaded==true && c==1 ) {
    c = pC;
    Info.lastUpper = 0, Info.maskUpper|=1;
  }
  if (Info.islink && (c==SPACE || c==']' || c==10  )) Info.islink=0;
     if ((mixer.x.c4&0xffffff)==0x3A2F2F) {
        Info.islink=1;
       
    }
    if (Info.istemplate && (mixer.x.c4&0xffff)==0x7d7d) Info.istemplate=0; //'}}'
     if ((mixer.x.c4&0xffff)==0x7b7b) {//'{{'
        Info.istemplate=1;
       
    }
   /*  if (Info.isqoute && (mixer.x.c4&0xff)==0x22) Info.isqoute=0; //'"'
     if ((mixer.x.c4&0xffff)==0x1022 || (mixer.x.c4&0xffff)==0x0522 || (mixer.x.c4&0xffff)==0x2022) {//' "'
        Info.isqoute=1;
       
    }*/
    
  pC = buffer(2);
  ParseCtx = hash(State=Parse::Unknown, pWord->Hash[0], c, (ilog2(Info.lastNewLine)+1)*(Info.lastNewLine*3>Info.prevNewLine), Info.masks[1]&0xFC);

  if ((c>='a' && c<='z') || c=='\'' || c=='-' || c>0x7F) {    
    if (Info.wordLength[0]==0) {
      // check for hyphenation with "+"
      if (pC==NEW_LINE && ((Info.lastLetter==3 && buffer(3)=='+') || (Info.lastLetter==4 && buffer(3)==CARRIAGE_RETURN && buffer(4)=='+'))) {
        Info.wordLength[0] = Info.wordLength[1];
        for (int i=Language::Unknown; i<Language::Count; i++)
          Words[i]--;
        cWord = pWord, pWord = &Words[Lang.pId](1);
        memset(cWord, 0, sizeof(Word));
        for (U32 i=0; i<Info.wordLength[0]; i++)
          (*cWord)+=buffer(Info.wordLength[0]-i+Info.lastLetter);
        Info.wordLength[1] = (*pWord).Length();
        cSegment->WordCount--;
        cSentence->WordCount--;
      }
      else {
        Info.wordGap = Info.lastLetter;
        Info.firstLetter = c;
      }
    }
    Info.lastLetter = 0;
    Info.wordLength[0]++;
    Info.masks[0]+=(Lang.Id!=Language::Unknown)?1+Stemmers[Lang.Id-1]->IsVowel(c):1, Info.masks[1]++, Info.masks[3]+=Info.masks[0]&3;
    if (c=='\'') {
      Info.masks[2]+=12;
      if (Info.wordLength[0]==1) {
        if (Info.quoteLength==0 && pC==SPACE)
          Info.quoteLength = 1;
        else if (Info.quoteLength>0 && Info.lastPunct==1) {
          Info.quoteLength = 0;
          ParseCtx = hash(State=Parse::AfterQuote, pC);
        }
      }
    }
    (*cWord)+=c;
    cWord->GetHashes();
    ParseCtx = hash(State=Parse::ReadingWord, cWord->Hash[0]);
  }
  else {
    if (cWord->Length()>0) {
      if (Lang.Id!=Language::Unknown)
        memcpy(&Words[Language::Unknown](0), cWord, sizeof(Word));

      for (int i=Language::Count-1; i>Language::Unknown; i--) {
        Lang.Count[i-1]-=(Lang.Mask[i-1]>>63), Lang.Mask[i-1]<<=1;
        if (i!=Lang.Id)
          memcpy(&Words[i](0), cWord, sizeof(Word));
        if (Stemmers[i-1]->Stem((xWord.Length()>0)?&xWord:&Words[i](0)))//
          Lang.Count[i-1]++, Lang.Mask[i-1]|=1,(xWord.Length()>0)?Words[i](0).Type=xWord.Type:0;
      }      
      Lang.Id = Language::Unknown;
      U32 best = MIN_RECOGNIZED_WORDS;
      for (int i=Language::Count-1; i>Language::Unknown; i--) {
        if (Lang.Count[i-1]>=best) {
          best = Lang.Count[i-1] + (i==Lang.pId); //bias to prefer the previously detected language
          Lang.Id = i;
        }
        Words[i]++;
      }
      Words[Language::Unknown]++;
  /*  #ifndef NDEBUG
      if (Lang.Id!=Lang.pId) {
        switch (Lang.Id) {
          case Language::Unknown: { printf("[Language: Unknown, blpos: %d]\n",mixer.x.blpos); break; };
          case Language::English: { printf("[Language: English, blpos: %d]\n",mixer.x.blpos); break; };
          case Language::French : { printf("[Language: French, blpos: %d]\n",mixer.x.blpos);  break; };
          case Language::German : { printf("[Language: German,  blpos: %d]\n",mixer.x.blpos); break; };
        }
      }
    #endif */
      Lang.pId = Lang.Id;
      pWord = &Words[Lang.Id](1), cWord = &Words[Lang.Id](0);
      memset(cWord, 0, sizeof(Word));
      memset(&xWord,0, sizeof(Word));
      WordPos[pWord->Hash[0]&(WordPos.size()-1)] = mixer.x.blpos;
      if (cSegment->WordCount==0)
        memcpy(&cSegment->FirstWord, pWord, sizeof(Word));
      cSegment->WordCount++;
      if (cSentence->WordCount==0)
        memcpy(&cSentence->FirstWord, pWord, sizeof(Word));
      cSentence->WordCount++;
      Info.wordLength[1] = Info.wordLength[0], Info.wordLength[0] = 0;
      Info.wordLength[3] = Info.wordLength[2], Info.wordLength[2] = Info.wordLength[1];
      Info.quoteLength+=(Info.quoteLength>0);
      if (Info.quoteLength>0x1F)
        Info.quoteLength = 0;
        cSentence->VerbIndex++, cSentence->NounIndex++, cSentence->CapitalIndex++;
      if ((pWord->Type&Language::Verb)!=0) {
        cSentence->VerbIndex = 0;
        memcpy(&cSentence->lastVerb, pWord, sizeof(Word));
      }
      if ((pWord->Type&Language::Noun)!=0) {
        cSentence->NounIndex = 0;
        memcpy(&cSentence->lastNoun, pWord, sizeof(Word));
      }
      if (cSentence->WordCount>1 && Info.lastUpper<Info.wordLength[1]) {
        cSentence->CapitalIndex = 0;
        memcpy(&cSentence->lastCapital, pWord, sizeof(Word));
      }
    }
    bool skip = false;
    switch (c) {
      case '.': {
        if (Lang.Id!=Language::Unknown && Info.lastUpper==Info.wordLength[1] && Languages[Lang.Id-1]->IsAbbreviation(pWord)) {
          ParseCtx = hash(State=Parse::WasAbbreviation, pWord->Hash[0]);
          memset(&Info.TopicDescriptor, 0, sizeof(Word));
          break;
        }
      }
      case '?': case '!': {
        cSentence->Type = (c=='.')?Sentence::Types::Declarative:(c=='?')?Sentence::Types::Interrogative:Sentence::Types::Exclamative;
        cSentence->SegmentCount++;
        cParagraph->SentenceCount++;
        cParagraph->TypeCount[cSentence->Type]++;
        cParagraph->TypeMask<<=2, cParagraph->TypeMask|=cSentence->Type;
        cSentence = &Sentences.Next();
        Info.masks[3]+=3;
        skip = true;
       Info.lastPunct = 0;
      }
      case ',': case ';': case ':': {
        if (c==',') {
          Info.commas++;
          ParseCtx = hash(State=Parse::AfterComma, ilog2(Info.quoteLength+1), ilog2(Info.lastNewLine), Info.lastUpper<Info.lastLetter+Info.wordLength[1]);
        }
        else if (c==':')
          memcpy(&Info.TopicDescriptor, pWord, sizeof(Word));
        if (!skip) {
          cSentence->SegmentCount++;
          Info.masks[3]+=4;
        }
        Info.lastPunct = 0, Info.prevPunct = c;
        Info.masks[0]+=3, Info.masks[1]+=2, Info.masks[2]+=15;
        cSegment = &Segments.Next();
        break;
      }
      case 5:
      case NEW_LINE: {
          Info.linespace=0;
        Info.nl2=Info.nl1,Info.nl1=Info.nl, Info.nl=mixer.x.buf.pos;
        Info.prevNewLine = Info.lastNewLine, Info.lastNewLine = 0;
        Info.commas = 0;
        if (Info.prevNewLine==1 || (Info.prevNewLine==2 && (pC==CARRIAGE_RETURN || pC==5)))
          cParagraph = &Paragraphs.Next();
        else if ((Info.lastLetter==2 && pC=='+') || (Info.lastLetter==3 && pC==CARRIAGE_RETURN && buffer(3)=='+'))
          ParseCtx = hash(Parse::ReadingWord, pWord->Hash[0]), State=Parse::PossibleHyphenation;
      }
      case TAB: case CARRIAGE_RETURN: case SPACE: {
        if (c==SPACE && pC!=SPACE) Info.linespace++;
        Info.spaceCount++, Info.spaces|=1;
        Info.masks[1]+=3, Info.masks[3]+=5;
        if (c==SPACE && pState==Parse::WasAbbreviation) {
          ParseCtx = hash(State=Parse::AfterAbbreviation, pWord->Hash[0]);
        }
        break;
      }
      case '(' : Info.masks[2]+=1; Info.masks[3]+=6; Info.nestHash+=31; Info.lastNest=0; break;
      case '[' : Info.masks[2]+=2; Info.nestHash+=11; Info.lastNest=0; break;
      case '{' : Info.masks[2]+=3; Info.nestHash+=17; Info.lastNest=0; break;
      case '<' : Info.masks[2]+=4; Info.nestHash+=23; Info.lastNest=0; break;
      case 0xAB: Info.masks[2]+=5; break;
      case ')' : Info.masks[2]+=6; Info.nestHash-=31; Info.lastNest=0; break;
      case ']' : Info.masks[2]+=7; Info.nestHash-=11; Info.lastNest=0; break;
      case '}' : Info.masks[2]+=8; Info.nestHash-=17; Info.lastNest=0; break;
      case '>' : Info.masks[2]+=9; Info.nestHash-=23; Info.lastNest=0; break;
      case 0xBB: Info.masks[2]+=10; break;
      case '"': {
        Info.masks[2]+=11;
        // start/stop counting
        if (Info.quoteLength==0)
          Info.quoteLength = 1;
        else {
          Info.quoteLength = 0;
          ParseCtx = hash(State=Parse::AfterQuote, 0x100|pC);
        }
        break;
      }
      case '/' : case '-': case '+': case '*': case '=': case '%': Info.masks[2]+=13; break;
      case '\\': case '|': case '_': case '@': case '&': case '^': Info.masks[2]+=14; break;
    }
    if (Info.firstChar=='[' && c==32 && ( mixer.x.buf(3)==']' ||  mixer.x.buf(4)==']' ) ) memset(&Info.WikiHead0, 0, sizeof(Word));
    if (( mixer.x.c4&0xFFFF)==0x3D3D && Info.firstChar==0x3d && doXML==true) memcpy(&Info.WikiHead1, pWord, sizeof(Word));// ,xword2=word2; // == wiki
       if (( mixer.x.c4&0xFFFF)==0x2727 && doXML==true) memcpy(&Info.WikiHead2, pWord, sizeof(Word)); ;//,xword2=word2; // '' wiki
       if (( mixer.x.c4&0xFFFF)==0x7D7D && doXML==true) memcpy(&Info.WikiHead3, pWord, sizeof(Word));       //}} wiki
       if (c==']'&& (Info.firstChar!=':') && doXML==true) memcpy(&Info.WikiHead0, pWord, sizeof(Word));  // ]] wiki 
       if (( mixer.x.c4&0xFF)==0x3d && Info.firstChar!=0x3d && doXML==true) memcpy(&Info.WikiHead4, pWord, sizeof(Word));       //word= wiki
    if (c>='0' && c<='9') {
      Info.numbers[0] = Info.numbers[0]*10 + (c&0xF), Info.numLength[0] = min(19, Info.numLength[0]+1);
      Info.numHashes[0] = hash(Info.numHashes[0], c, Info.numLength[0]);
      Info.expectedDigit = -1;
      if (Info.numLength[0]<Info.numLength[1] && (pState==Parse::ExpectDigit || ((Info.numDiff&3)==0 && Info.numLength[0]<=1))) {
        U64 ExpectedNum = Info.numbers[1]+(Info.numMask&3)-2, PlaceDivisor=1;
        for (int i=0; i<Info.numLength[1]-Info.numLength[0]; i++, PlaceDivisor*=10);
        if (ExpectedNum/PlaceDivisor==Info.numbers[0]) {
          PlaceDivisor/=10;
          Info.expectedDigit = (ExpectedNum/PlaceDivisor)%10;
          State = Parse::ExpectDigit;
        }
      }
      else {
        U8 d = buffer(Info.numLength[0]+2);
        if (Info.numLength[0]<3 && buffer(Info.numLength[0]+1)==',' && d>='0' && d<='9')
          State = Parse::ExpectDigit;
      }
      Info.lastDigit = 0;
      Info.masks[3]+=7;
    }
    else if (Info.numbers[0]>0 /*&& c!='.'*/) {
      Info.numMask<<=2, Info.numMask|=1+(Info.numbers[0]>=Info.numbers[1])+(Info.numbers[0]>Info.numbers[1]);
      Info.numDiff<<=2, Info.numDiff|=min(3,ilog2(abs((int)(Info.numbers[0]-Info.numbers[1]))));
      Info.numbers[1] = Info.numbers[0], Info.numbers[0] = 0;
      Info.numbers[3] = Info.numbers[2], Info.numbers[2] =  Info.numbers[1];
      Info.numbers[5] = Info.numbers[4], Info.numbers[4] = 2;

      Info.numHashes[1] = Info.numHashes[0], Info.numHashes[0] = 0;
      Info.numHashes[3] = Info.numHashes[2], Info.numHashes[2] = Info.numHashes[1] ;
      Info.numHashes[5] = Info.numHashes[4], Info.numHashes[4] = 2;
      
      Info.numLength[1] = Info.numLength[0], Info.numLength[0] = 0;
      Info.numLength[3] = Info.numLength[2], Info.numLength[2] = Info.numLength[1];
      
      cSegment->NumCount++, cSentence->NumCount++;
    }
  }
  if (Info.lastNewLine==1) {
    Info.firstChar = (Lang.Id!=Language::Unknown)?c:min(c,96); firstwasspace=Info.firstChar==' '?true:false;        }
     
        if (Info.lastNewLine>1 && firstwasspace && (Info.firstChar==' ' || Info.firstChar<4)  && buffer(1)!=' ') Info.firstChar=c;
        if (Info.lastNewLine>1 && firstwasspace && !((Info.firstChar>='a' && Info.firstChar<='z') || (Info.firstChar>='0' && Info.firstChar<='9')|| (Info.firstChar>=128 &&(buffer(2)!=3))) ) Info.firstChar=c;  //world95

  if (Info.lastNest>512)
    Info.nestHash = 0;
  int leadingBitsSet = 0;
  while (((c>>(7-leadingBitsSet))&1)!=0) leadingBitsSet++;

  if (Info.UTF8Remaining>0 && leadingBitsSet==1)
    Info.UTF8Remaining--;
  else
    Info.UTF8Remaining = (leadingBitsSet!=1)?(c!=0xC0 && c!=0xC1 && c<0xF5)?(leadingBitsSet-(leadingBitsSet>0)):-1:0;
  Info.maskPunct = (BytePos[',']>BytePos['.'])|((BytePos[',']>BytePos['!'])<<1)|((BytePos[',']>BytePos['?'])<<2)|((BytePos[',']>BytePos[':'])<<3)|((BytePos[',']>BytePos[';'])<<4);
  mixer.x.Text.state = State;
    mixer.x.Text.lastPunct = min(0x1F, Info.lastPunct);
    mixer.x.Text.wordLength = min(0xF, Info.wordLength[0]);
    mixer.x.Text.boolmask = (Info.lastDigit<Info.wordLength[0]+Info.wordGap)|
                          ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
                          ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<2)|
                          ((Info.lastUpper<Info.wordLength[0])<<3);
    mixer.x.Text.firstLetter = Info.firstLetter;
    mixer.x.Text.mask = Info.masks[1]&0xFF;
}

void TextModel::SetContexts(Buf& buffer,Mixer& mixer) {
  U8 c = buffer(1), lc = tolower(c), m2 = Info.masks[2]&0xF, column = min(0xFF, Info.lastNewLine);;
  U16 w = ((State==Parse::ReadingWord)?cWord->Hash[0]:pWord->Hash[0]);
  U32 h = ((State==Parse::ReadingWord)?cWord->Hash[1]:pWord->Hash[2])*271+c;
  int i = State<<6;

  Map.set(ParseCtx);
  Map.set(hash(i++, cWord->Hash[0], pWord->Hash[0],
    (Info.lastUpper<Info.wordLength[0])|
    ((Info.lastDigit<Info.wordLength[0]+Info.wordGap)<<1)|
     ((doXML)<<2)
  )); 
  Map.set(hash(i++, cWord->Hash[1], Words[Lang.pId](2).Hash[1], min(10,ilog2((U32)Info.numbers[0])),
    (Info.lastUpper<Info.lastLetter+Info.wordLength[1])|
    ((Info.lastLetter>3)<<1)|
    ((Info.lastLetter>0 && Info.wordLength[1]<3)<<2) 
  ));
   Map.set(hash(i++,min(10,ilog2((U32)Info.numbers[2])),Info.numHashes[2],(cSentence->VerbIndex<cSentence->WordCount)?cSentence->lastVerb.Hash[1]:0));//

  //Map.set(hash(i++,min(10,ilog2((U32)Info.numbers[4])),Info.numHashes[4],Words[Lang.pId](2).Hash[3]));

  Map.set(hash(i++,min(10,ilog2((U32)Info.numbers[3])),Info.firstLetter,(cSentence->VerbIndex<cSentence->WordCount)?cSentence->lastVerb.Hash[1]:0));//

  Map.set(hash(i++,min(3,ilog2(cSegment->WordCount+1)),min(10,ilog2((U32)Info.numbers[4])),Words[Lang.pId](2).Hash[3]));

  Map.set(hash(i++, cWord->Hash[0], Info.masks[1]&0x3FF, Words[Lang.pId](3).Hash[1],
    (Info.lastDigit<Info.wordLength[0]+Info.wordGap)|
    ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
    ((Info.spaces&0x7F)<<2)/*|
     ((Info.linespace>4 && (doXML==false))<<3 )*/
  ));
  Map.set(hash(++i, cWord->Hash[0], pWord->Hash[1]));
  Map.set(hash(i++, cWord->Hash[0], pWord->Hash[1], Words[Lang.pId](2).Hash[1]));
  Map.set(hash(i++, h, Words[Lang.pId](2).Hash[0], Words[Lang.pId](3).Hash[0]));
  Map.set(hash(i++, cWord->Hash[0], c, (cSentence->VerbIndex<cSentence->WordCount)?cSentence->lastVerb.Hash[0]:0));
  Map.set(hash(i++, pWord->Hash[1], Info.masks[1]&0xFC, lc, Info.wordGap));
  Map.set(hash(i++, (Info.lastLetter==0)?cWord->Hash[0]:pWord->Hash[0], c, cSegment->FirstWord.Hash[1], min(3,ilog2(cSegment->WordCount+1))));
  Map.set(hash(i++, cWord->Hash[0], c, Segments(1).FirstWord.Hash[1]));
  Map.set(hash(i++, max(31,lc), Info.masks[1]&0xFFC, (Info.spaces&0xFE)|(Info.lastPunct<Info.lastLetter), (Info.maskUpper&0xFF)|(((0x100|Info.firstLetter)*(Info.wordLength[0]>1))<<8)));
  Map.set(hash(i++, column,mixer.x.wcol, min(7,ilog2(Info.lastUpper+1)), ilog2(Info.lastPunct+1)));
  Map.set(hash(++i,
    (((mixer.x.dictonline==true)?mixer.x.wcol:column)&0xF8)|(Info.masks[1]&3)|((Info.prevNewLine-Info.lastNewLine>63)<<2)|
    (min(3, Info.lastLetter)<<8)|
    (Info.firstChar<<10)|
    ((Info.commas>4)<<18)|
    ((m2>=1 && m2<=5)<<19)|
    ((m2>=6 && m2<=10)<<20)|
    ((m2==11 || m2==12)<<21)|
    ((Info.lastUpper<column)<<22)|
    ((Info.lastDigit<column)<<23)|
    ((column<Info.prevNewLine-Info.lastNewLine)<<24)|
    ((Info.linespace>4)<<25)
  ));
  Map.set(hash(
    (2*((mixer.x.dictonline==true)?mixer.x.wcol:column))/3,
    min(13, Info.lastPunct)+(Info.lastPunct>16)+(Info.lastPunct>32)+Info.maskPunct*16,
    ilog2(Info.lastUpper+1),
    ilog2(Info.prevNewLine-Info.lastNewLine),
    ((Info.masks[1]&3)==0)|
    ((m2<6)<<1)|
    ((m2<11)<<2)
  ));
   
  Map.set(hash(i++, column>>1,mixer.x.wcol, Info.spaces&0xF));
  Map.set(hash(
    Info.masks[3]&0x3F,
    min((max(Info.wordLength[0],3)-2)*(Info.wordLength[0]<8),3),
    Info.firstLetter*(Info.wordLength[0]<5),
    w&0x3FF,
    (c==buffer(2))|
    ((Info.masks[2]>0)<<1)|
    ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<2)|
    ((Info.lastUpper<Info.wordLength[0])<<3)|
    ((Info.lastDigit<Info.wordLength[0]+Info.wordGap)<<4)|
    ((Info.lastPunct<2+Info.wordLength[0]+Info.wordGap+Info.wordLength[1])<<5)|
    ((Info.linespace>4)<<6)
  ));
  Map.set(hash(i++,Info.numHashes[4],    min((max(Info.wordLength[0],3)-2)*(Info.wordLength[0]<8),3),(Info.lastLetter>0)?c:0x100));//
  
  Map.set(hash(i++, w, c, Info.numHashes[1]));
  Map.set(hash(i++, w, c, llog(mixer.x.blpos-WordPos[w&(WordPos.size()-1)])>>1));
  Map.set(hash(i++, w, c, Info.TopicDescriptor.Hash[0]));
  
    if (doXML==true){
        Map.set(hash(i++, w, c, Info.WikiHead0.Hash[0]));// [[word]] ?
        Map.set(hash(i++, w, c, Info.WikiHead1.Hash[0]));//  ==word==
        Map.set(hash(i++, w, c, Info.WikiHead2.Hash[0]));// ''word''
        Map.set(hash(i++, w, c, Info.WikiHead3.Hash[0]));// }} - table
         Map.set(hash(i++, w, c, Info.WikiHead4.Hash[0]));// }} - table
    }else{
        Map.set(0), Map.set(0), Map.set(0), Map.set(0), Map.set(0); // 
    }
  Map.set(hash(i++, Info.numLength[0], c, Info.TopicDescriptor.Hash[0]));
  Map.set(hash(i++, (Info.lastLetter>0)?c:0x100, Info.masks[1]&0xFFC, Info.nestHash&0x7FF));
        int above=mixer.x.buf[Info.nl1+column];  
        int above1=mixer.x.buf[Info.nl2+column]; 
  if(Info.wordLength[1]) Map.set(hash(i++,
    (column>0)|
    ((Info.wordLength[1]>0)<<1)|
    ((above==above1)<<2),
    above, c)); else Map.set(0);
           
  Map.set(hash(i++, w, c, Info.masks[3]&0x1FF,
    ((cSentence->VerbIndex==0 && cSentence->lastVerb.Length()>0)<<6)|
    ((Info.wordLength[1]>3)<<5)|
    ((cSegment->WordCount==0)<<4)|
    ((cSentence->SegmentCount==0 && cSentence->WordCount<2)<<3)|
    ((Info.lastPunct>=Info.lastLetter+Info.wordLength[1]+Info.wordGap)<<2)|
    ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
    (Info.lastUpper<Info.wordLength[0]+Info.wordGap+Info.wordLength[1])
  ));
  Map.set(hash(i++, c, pWord->Hash[1], Info.firstLetter*(Info.wordLength[0]<6),
    ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<1)|
    (Info.lastPunct>=Info.lastLetter+Info.wordLength[1]+Info.wordGap)
  ));
  Map.set(hash(i++, w, c, Words[Lang.pId](1+(Info.wordLength[0]==0)).Letters[Words[Lang.pId](1+(Info.wordLength[0]==0)).Start], Info.firstLetter*(Info.wordLength[0]<7)));
  Map.set(hash(i++, column, mixer.x.wcol,Info.spaces&7, Info.nestHash&0x7FF)); 
  Map.set(hash(i++, cWord->Hash[0], (Info.lastUpper<column)|((Info.lastUpper<Info.wordLength[0])<<1), min(5, Info.wordLength[0])));
}


