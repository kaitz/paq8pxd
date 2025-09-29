#include "xml.hpp"

/*
====== XML model ======
*/

extern U8 level;

 
  XMLModel1::XMLModel1(BlockData& bd,U32 val):x(bd),buf(bd.buf), State(None), pState(None), c8(0),
   WhiteSpaceRun(0), pWSRun(0), IndentTab(0), IndentStep(2), LineEnding(2),lastState(0), cm(level>9?0x1000000:CMlimit(MEM()/4), 4,M_XML) {
       memset(&Cache, 0, sizeof(XMLTagCache));
       memset(&StateBH, 0, sizeof(StateBH));  
        
  }
  
  void XMLModel1::setContexts(){
      stateContext=0;
      U8 B = (U8)x.c4;
       pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ], Tag = &Cache.Tags[ Cache.Index&(CacheSize-1) ];
     Attribute = &((*Tag).Attributes.Items[ (*Tag).Attributes.Index&3 ]);
     Content = &(*Tag).Content;     
    //XMLTag *pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ], *Tag = &Cache.Tags[ Cache.Index&(CacheSize-1) ];
    //XMLAttribute *Attribute = &((*Tag).Attributes.Items[ (*Tag).Attributes.Index&3 ]);
    //XMLContent *Content = &(*Tag).Content;
    pState = State;
    c8 = (c8<<8)|buf(5);
    if ((B==0x09 || B==0x20) && (B==(U8)(x.c4>>8) || !WhiteSpaceRun)){
      WhiteSpaceRun++;
      IndentTab = (B==0x09);
    }
    else{
      if ((State==None || (State==ReadContent && (*Content).Length<=LineEnding+WhiteSpaceRun)) && WhiteSpaceRun>1+IndentTab && WhiteSpaceRun!=pWSRun){
        IndentStep=abs((int)(WhiteSpaceRun-pWSRun));
        pWSRun = WhiteSpaceRun;
      }
      WhiteSpaceRun=0;
    }
    if (B==0x0A ||(B==5 && ((U8)(x.c4>>8)!='&')) )
      LineEnding = 1+((U8)(x.c4>>8)==0x0D);
    if(State!=None) lastState=buf.pos;
    // if &< or &> or &</ then ignore and force to reading content, so wordmodel knows
    if (  (x.c4&&0xffff)==0x263c || (x.c4&&0xffff)==0x263e|| (x.c4&&0xffffff)==0x263c2f){
              State = ReadContent;
    }
    switch (State){
      case None : {
        if (B==0x3C){
          State = ReadTagName;
          memset(Tag, 0, sizeof(XMLTag));
          (*Tag).Level = ((*pTag).EndTag || (*pTag).Empty)?(*pTag).Level:(*pTag).Level+1;
        }
        if ((*Tag).Level>1)
          DetectContent();
        
        stateContext=(hash(pState, State, ((*pTag).Level+1)*IndentStep - WhiteSpaceRun));
        break;
      }
      case ReadTagName : {
        if ((*Tag).Length>0 && (B==0x09 || B==0x0A || B==0x0D || (B==5 && ((U8)(x.c4>>8)!='&')) || B==0x20))
          State = ReadTag;
        else if ((B>127)|| (B==0x3A || (B>='A' && B<='Z') || B==0x5F|| /*B==1|| B==2 ||*/ (B>='a' && B<='z')) || ((*Tag).Length>0 && (B==0x2D || B==0x2E || (B>='0' && B<='9')))){
          (*Tag).Length++;
          (*Tag).Name = (*Tag).Name * 263 * 32 + (B&0xDF);
        }
        else if (B == 0x3E){
          if ((*Tag).EndTag){
            State = None;
            Cache.Index++;
          }
          else
            State = ReadContent;
        }
        else if (B!=0x21 && B!=0x2D && B!=0x2F && B!=0x5B){
          State = None;
          Cache.Index++;
        }
        else if ((*Tag).Length==0){
          if (B==0x2F){
            (*Tag).EndTag = true;
            (*Tag).Level = max(0,(*Tag).Level-1);
          }
          else if (x.c4==0x3C212D2D){
            State = ReadComment;
            (*Tag).Level = max(0,(*Tag).Level-1);
          }
        }

        if ((*Tag).Length==1 && (x.c4&0xFFFF00)==0x3C2100){
          memset(Tag, 0, sizeof(XMLTag));
          State = None;
        }
        else if ((*Tag).Length==5 && c8==0x215B4344 && x.c4==0x4154415B){
          State = ReadCDATA;
          (*Tag).Level = max(0,(*Tag).Level-1);
        }
        
        int i = 1;
        do{
          pTag = &Cache.Tags[ (Cache.Index-i)&(CacheSize-1) ];
          i+=1+((*pTag).EndTag && Cache.Tags[ (Cache.Index-i-1)&(CacheSize-1) ].Name==(*pTag).Name);
        }
        while ( i<CacheSize && ((*pTag).EndTag || (*pTag).Empty) );

        stateContext=(hash(pState*8+State, (*Tag).Name, (*Tag).Level, (*pTag).Name, (*pTag).Level!=(*Tag).Level ));
        break;
      }
      case ReadTag : {
        if (B==0x2F)
          (*Tag).Empty = true;
        else if (B==0x3E){
          if ((*Tag).Empty){
            State = None;
            Cache.Index++;
          }
          else
            State = ReadContent;
        }
        else if (B!=0x09 && B!=0x0A && B!=0x0D && B!=0x20 && B!=5){
          State = ReadAttributeName;
          (*Attribute).Name = B&0xDF;
        }
        stateContext=(hash(pState, State, (*Tag).Name, B, (*Tag).Attributes.Index ));
        break;
      }
      case ReadAttributeName : {
        if ((x.c4&0xFFF0)==0x3D20 && (B==0x22 || B==0x27)){
          State = ReadAttributeValue;
          if ((c8&0xDFDF)==0x4852 && (x.c4&0xDFDF0000)==0x45460000)
            (*Content).Type |= Link;
        }
        else if (B!=0x22 && B!=0x27 && B!=0x3D)
          (*Attribute).Name = (*Attribute).Name * 263 * 32 + (B&0xDF);

        stateContext=(hash(pState*8+State, (*Attribute).Name, (*Tag).Attributes.Index, (*Tag).Name, (*Content).Type ));
        break;
      }
      case ReadAttributeValue : {
        if (B==0x22 || B==0x27){
          (*Tag).Attributes.Index++;
          State = ReadTag;
        }
        else{
          (*Attribute).Value = (*Attribute).Value* 263 * 32 + (B&0xDF);
          (*Attribute).Length++;
          if ((c8&0xDFDFDFDF)==0x48545450 && ((x.c4>>8)==0x3A2F2F || x.c4==0x733A2F2F)) // HTTP :// s://
            (*Content).Type |= URL;
        }
        stateContext=(hash(pState, State, (*Attribute).Name, (*Content).Type ));
        break;
      }
      case ReadContent : {
        if (B==0x3C && ((x.c4&0xffff)!=0x263c)){ // if new tag and not escape
          State = ReadTagName;
          Cache.Index++;
          memset(&Cache.Tags[ Cache.Index&(CacheSize-1) ], 0, sizeof(XMLTag));
          Cache.Tags[ Cache.Index&(CacheSize-1) ].Level = (*Tag).Level+1;
        }
        else{
          (*Content).Length++;
          (*Content).Data = (*Content).Data * 997*16 + (B&0xDF);

          DetectContent();
        }
        stateContext=(hash(pState, State, (*Tag).Name, x.c4&0xC0FF ));
        break;
      }
      case ReadCDATA : {
        if ((x.c4&0xFFFFFF)==0x5D5D3E){ // ]]>
          State = None;
          Cache.Index++;
        }
        stateContext=(hash(pState, State));
        break;
      }
      case ReadComment : {
        if ((x.c4&0xFFFFFF)==0x2D2D3E){ // -->
          State = None;
          Cache.Index++;
        }
        stateContext=(hash(pState, State));
        break;
      }
    }
    StateBH[pState] = (StateBH[pState]<<8)|B;
    pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ];
  }
int XMLModel1::p(Mixer& m,int val1,int val2){
    if (x.filetype==DBASE ||x.filetype==HDR ||x.filetype==DECA || x.filetype==ARM  || x.filetype==IMGUNK|| x.filetype==BINTEXT){
        if (val2==-1) return 1;
        for (int i=0; i<inputs(); ++i)
        m.add(0);
        return 0;
    }
    
    if (x.bpos==0) {
    setContexts();
    if (val2==-1) return 1;
     pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ], Tag = &Cache.Tags[ Cache.Index&(CacheSize-1) ];
     //*Attribute = &((*Tag).Attributes.Items[ (*Tag).Attributes.Index&3 ]);
    // *Content = &(*Tag).Content;     
    cm.set(stateContext);
    // set context if last state was less then 256 bytes ago
    if ((buf.pos-lastState)<256){ 
        cm.set(hash(State, (*Tag).Level, pState*2+(*Tag).EndTag, (*Tag).Name));
        cm.set(hash((*pTag).Name, State*2+(*pTag).EndTag, (*pTag).Content.Type, (*Tag).Content.Type));
        cm.set(hash(State*2+(*Tag).EndTag, (*Tag).Name, (*Tag).Content.Type, x.c4&0xE0FF));
    }else {
        cm.set();
        cm.set();
        cm.set();
    } 
  }
   if (val2==0)  cm.mix(m);

  U8 s = ((StateBH[State]>>(28-x.bpos))&0x08) |
         ((StateBH[State]>>(21-x.bpos))&0x04) |
         ((StateBH[State]>>(14-x.bpos))&0x02) |
         ((StateBH[State]>>( 7-x.bpos))&0x01) |
         ((x.bpos)<<4);
  return (s<<3)|State;
}

