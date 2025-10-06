#include "witfilter.hpp"

// WIT
const char UTF8bytes[256]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 
    3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

// utf8 len
int utf8len(char *s) {
    return UTF8bytes[(U32)(U8)s[0]] + 1;
}

void wfgets(char *str, int count, File  *fp) {
    int c, i = 0;
    while (i<count-1 && ((c=fp->getc())!=EOF)) {
        str[i++]=c;
        if (c=='\n')
        break;
    }
    str[i]=0;
}
/*
void wfputs(   char *str,File *fp) {
    int c, i = 0;
i = strlen(str);
fp->blockwrite((U8*)  &str[0],   i  );

}
void wfputs1(   char *str,FileTmp *fp) {
    int c, i = 0;
i = strlen(str);
fp->blockwrite((U8*)  &str[0],   i  ); // killer of pagefaults

}*/
void wfputs(char *str,File *fp) {
    for (int i = 0; *str; str++){
        fp->putc(*str);
    }
}
void wfputs1(char *str,FileTmp *fp) {
    for (int i = 0; *str; str++){
        fp->putc(*str);
    }
}
// UTF8 to WC (USC) and reverse
int utf8towc(char *dest, U32 ch) {
    int val=0;
    if (ch==1) {
        val=(char)dest[0] ;
        return val;
    }
    if (ch==2) {
        val=dest[1]&0x3F;
        val=val|(dest[0]&0x1F)<<6;
        return val;
    }
    if (ch==3) {
        val|=(dest[0]&0x1F)<<12;
        val|=(dest[1]&0x3F)<<6;
        val=val|(dest[2]&0x3F);
        return val;
    }
    if (ch==4) {
        val|=(dest[0]&0xF)<<18;
        val|=(dest[1]&0x3F)<<12;
        val|=(dest[2]&0x3F)<<6;
        val=val|(dest[3]&0x3F);
        return val;
    }
    return 0;
}
int wctoutf8(char *dest, U32 ch) {
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

// get num string length terminated by ; or non number
int numlen(char *str) {
    int c, i = 0;
    for (i = 0; *str!=';'; str++){
        if (*str<'0' || *str>'9') return 0;
        i++;
    }   
    return i;
} 
// reduce
//re3
void hent1(char *in,char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;
        if (j=='&') {
            k=*in++;
            if (k=='&')  *(int*)out=0x3B706D61, out+=4;  else//';pma'
            if (k=='"')  *(int*)out=0x746F7571, out+=4, *out++=';';  else//'touq'
            if (k=='<')  *(int*)out=0x203B746C, out+=3;  else//' ;tl'
            if (k=='>')  *(int*)out=0x203B7467, out+=3;//' ;tg'
            else   *out++=k;
        }
    }
    while (j!=0);
}

void hent6(char *in, char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;

        if (j=='&') {
            k=*in++;
            if (k==5){// escape char
                k=utf8len(in);
                int nu=utf8towc(in,k);
                *out='&';
                sprintf(out+1, "#%d;",   nu);
                int a=numlen(out+1+1);
                out=out+2+a+1;
                in=in+k;
            }     
            else   *out++=k;
        }
    }
    while (j!=0);
}
//re4
void hent3(char *in, char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;
        if (j==';' && *(int*)(in-5)==0x706D6126) { //'pma&'
            k = *in++;
            if (k==3); else
            if (k=='"')  *(int*)out=0x746F7571, out+=4, *out++=';';  else//'touq'
            if (k=='<')  *(int*)out=0x203B746C, out+=3;  else//' ;tl'
            if (k=='>')  *(int*)out=0x203B7467, out+=3;  else//' ;tg'
            if (k=='!')  *(int*)out=0x7073626E, out+=4, *out++=';';  else//'psbn'
            if (k=='*')  *(int*)out=0x7361646E, out+=4, *out++='h', *out++=';'; else//'sadn'
            if (k=='^')  *(int*)out=0x7361646D, out+=4, *out++='h', *out++=';'; else //'sadm'
            if (U8(k)==0xc2 && U8(*(in))==0xae ) {  *(int*)out=0x3B676572, out+=4,in++;}else //'reg'    
            if (U8(k)==0xc2 && U8(*(in))==0xb0 ) {  *(int*)out=0x3B676564, out+=4,in++;}else //'deg'  old=enwik8/9 only
            if (U8(k)==0xc2 && U8(*(in))==0xb2 ) {  *(int*)out=0x32707573, out+=4,*out++=';',in++;}else //'sup2'  // old    
            if (U8(k)==0xc2 && U8(*(in))==0xb3 ) {  *(int*)out=0x33707573, out+=4,*out++=';',in++;}else //'sup3'  // old   
            if (U8(k)==0xe2 && U8(*(in))==0x82&& U8(*(in+1))==0xac ) {  *(int*)out=0x6F727565, out+=4,*out++=';',in+=2;}else //'euro'
            if (U8(k)==0xc3 && U8(*(in))==0x97  ) {  *(int*)out=0x656D6974, out+=4,*out++='s',*out++=';',in++;}else //'times' old
            if (U8(k)==0xe2 && U8(*(in))==0x88&& U8(*(in+1))==0x88 ) {  *(int*)out=0x6E697369, out+=4,*out++=';',in+=2;}else //'isin'
            if (U8(k)==0xe2 && U8(*(in))==0x86&& U8(*(in+1))==0x92 ) {  *(int*)out=0x72726172, out+=4,*out++=';',in+=2;}else //'rarr'
            if (U8(k)==0xe2 && U8(*(in))==0x88&& U8(*(in+1))==0x92 ) {  *(int*)out=0x756E696D, out+=4,*out++='s',*out++=';',in+=2;}  //'minus' old
            else *out++=k;
        }
    }
    while (j!=0);
}
//pre3
void hent(char *in, char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;

        if (j=='&') {
            k = *(int*)in;
            if (k==0x3B706D61) { //';pma'
                *out++='&', in+=4; k=*(in);
                if ((k=='"') || (k=='<') || (k=='>') || (k=='!') || (k=='*') || (k=='^') ||
                        (U8(k)==0xc2 && U8(*(in+1))==0xb2 ) || 
                        (U8(k)==0xc2 && U8(*(in+1))==0xb3 ) || 
                        (U8(k)==0xc2 && U8(*(in+1))==0xae )  ||
                        (U8(k)==0xc2 && U8(*(in+1))==0xb0 )  ||
                        (U8(k)==0xe2 && U8(*(in+1))==0x82&& U8(*(in+1+1))==0xac )  ||
                        (U8(k)==0xc3 && U8(*(in+1))==0x97  ) ||
                        (U8(k)==0xe2 && U8(*(in+1))==0x88&& U8(*(in+1+1))==0x92 )  ||
                        (U8(k)==0xe2 && U8(*(in+1))==0x88&& U8(*(in+1+1))==0x88 )  ||
                        (U8(k)==0xe2 && U8(*(in+1))==0x86&& U8(*(in+1+1))==0x92 )  
                        
                        )    *out++=3; }  else  // sup2, escape if char present
            if (k==0x746F7571 && *(in+4)==';')  *out++='"', in+=5;  else//'touq'
            {
                k = k*256 + ' ';
                if (k==0x3B746C20) *out++='<', in+=3;  else//';tl '
                if (k==0x3B746720) *out++='>', in+=3;//';tg '
            }
        }
    }
    while (j!=0);
}
//pre4
void hent2(char *in, char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;

        if (j=='&' && *(in-2)=='&') {
            k=*(int*)in;
            if (k==0x746F7571 && *(in+4)==';')  *out++='"', in+=5;  else//'touq'
            if (k==0x7073626E && *(in+4)==';')  *out++='!', in+=5;  else//'psbn'
            if (k==0x7361646E && *(in+4)=='h' && *(in+5)==';')  *out++='*', in+=6;  else//'sadn'
            if (k==0x7361646D && *(in+4)=='h' && *(in+5)==';')  *out++='^', in+=6;  else//'sadm'
            if (k==0x3B676572  ) { *out++=0xc2,*out++=0xae, in+=4;  }  else//'reg'
            if (k==0x3B676564  ) { *out++=0xc2,*out++=0xb0, in+=4;  }  else//'deg'
            if (k==0x6F727565  && *(in+4)==';' ) { *out++=0xe2,*out++=0x82,*out++=0xac, in+=5;  }  else//'euro'
            if (k==0x32707573  && *(in+4)==';' ) { *out++=0xc2,*out++=0xb2, in+=5;  }  else//'sup2'
            if (k==0x33707573  && *(in+4)==';' ) { *out++=0xc2,*out++=0xb3, in+=5;  }  else//'sup3'
            if (k==0x656D6974  && *(in+4)=='s'&& *(in+5)==';' ) { *out++=0xc3,*out++=0x97, in+=6;  }  else//'times'
            if (k==0x6E697369  && *(in+4)==';' ) { *out++=0xe2,*out++=0x88,*out++=0x88, in+=5;  }  else//'isin'
            if (k==0x72726172  && *(in+4)==';' ) { *out++=0xe2,*out++=0x86,*out++=0x92, in+=5;  }  else//'rarr'
            
            if (k==0x756E696D  && *(in+4)=='s'&& *(in+5)==';' ) { *out++=0xe2,*out++=0x88,*out++=0x92, in+=6;  }  else//'minus'
            {
                k = k*256 + ' ';
                if (k==0x3B746C20)  *out++='<', in+=3;  else//';tl '
                if (k==0x3B746720)  *out++='>', in+=3;//';tg '
            }
        }
    }
    while (j!=0);
}

//&#
void hent5(char *in, char *out) {
    int j, k;
    do {
        j=*in++; *out++=j;
        if (j=='&' && *(in)=='#' && *(in-2)=='&' && *(in+1)>'0'&& *(in+1)<='9') { //  &&#xxx; to &@UTF8
            int n=numlen(in+1);
            int d=atoi(&in[1]);
            if (d>255 && n){//>2 && n<6
                in++;
                *(out-1)=5;// escape char
                int e=wctoutf8(out,d);
                out=out+e;
                in=in+n+1;
                // printf("Numlen: %d value: %d utflen: %d\n",n,d,e);
            }
        }
    }
    while (j!=0);
}

void hent9(char *in, char *out) {
    int j, k;
    int t=0;

#define PROCESS(sym, src, dst, CONDITION) \
        {\
            char *t,  *p = src,  *q = dst,  *end = p + strlen(src);\
            while ( (t=strchr(p, sym)) != 0) {   \
                memcpy(q, p, t-p);  q+=t-p;      \
                int count = 0;                   \
                while(*t++ == sym)  ++count;     \
                if ((CONDITION) && (count==1 || count==2))  count = 3-count;\
                memset(q, sym, count);  q+=count;\
                p = t-1;\
            }\
            memcpy(q, p, end-p);  q[end-p]=0; \
        }
    // breaks wordmodel
    // PROCESS('[', in, out, 1)
    //  PROCESS(']', out, in, 1)
    /* PROCESS('&', in, out, 1)
    do {
        j=*in++; *out++=j;
        }
        while (j!=0); */


}
void skipline(char *in,char *out) {
    int j;
    do {
        j=*in++; *out++=j;
    }
    while (j!=0);
}
// &" -> "   for now
void removeamp( char *in,char *out ,int skip) {
    int j;
    for (int i=0;i<skip;i++) {j=*in++; *out++=j;
    }
    do {
        j=*in++; 
        if (j=='"' /*|| j=='<' || j=='>'*/)  { /*assert(p[-1]=='&');*/  --out; }
        *out++=j;
    }
    while (j!=0); 
}
void restoreamp( char *in,char *out,int skip) {
    int j;
    for (int i=0;i<skip;i++) {j=*in++; *out++=j;
    }
    do {
        j=*in++; 
        if (j=='"' /*|| j=='<' || j=='>'*/)  { /*assert(p[-1]=='&');*/  *out++='&'; }
        *out++=j;
    }
    while (j!=0); 
}

void henttail( char *in,char *out,FileTmp *o){
    int c, i=0,  j=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0;
    unsigned char   *ps;

    ++lnu;
    j = strlen(in);
    for(i=0; i<j; ++i)  if (*(int*)&in[i]==0x7865743C)  b1 = lnu;//'xet<'
    // parse comment tag
    if (memcmp(&in[6],"<comment>",9)==0 && f==0) co=1;
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x6F632F3C && f==0 && co==1) {//'oc/<'
        co=0,lnu=0,b1=0; // </co mment
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
        return;
    }
    if (f==0) {
        
        if (in[0]=='[' && in[1]=='[') {
            ps = (unsigned char*)in + (in[2]==':' ? 1 : 0);
            for (c=2; c<j; ++c)  if ( (in[c]<'a' /*&& in[c]!='-'*/) || in[c]>'z') {
                break;
            } 
            if (c<j && in[c]==':' && !(in[3]==':' || in[2]==':') && co==0) {
                if ((memcmp(&ps[2],"http:",5)==0) ||(memcmp(&ps[2],"https:",6)==0) ||
                        (memcmp(&ps[2],"user:",5)==0) || (memcmp(&ps[2],"User:",5)==0) || 
                        (memcmp(&ps[2],"media:",6)==0) ||
                        (memcmp(&ps[2],"File:",5)==0) || (memcmp(&ps[2],"file:",5)==0) ||
                        (memcmp(&ps[3], "mage:",5)==0) ||
                        (memcmp(&ps[3], "ategory:",8)==0) ||
                        (memcmp(&ps[3], "iktionary:",10)==0) ||
                        (memcmp(&ps[3], "ikipedia:",9)==0) ||
                        (memcmp(&ps[2],"Kategoria:",10)==0) ||
                        (memcmp(&ps[7],     "gorie:",6)==0) ||
                        (memcmp(&ps[2],"imagem:",7)==0) ||
                        (memcmp(&ps[2],"wikt:",5)==0) ||
                        (memcmp(&ps[2],"Categor",7)==0) ||
                        (memcmp(&ps[2],"archivo:",8)==0) ||
                        (memcmp(&ps[2],"imagen:",7)==0) ||
                        (memcmp(&ps[2],"Archivo:",8)==0) ||
                        (memcmp(&ps[2],"Wikiproyecto:",13)==0) ||
                        (memcmp(&ps[2],"Utente:",7)==0) || (memcmp(&ps[2],"utente:",7)==0) ||
                        (memcmp(&ps[2],":Immagine:",10)==0) ||
                        (memcmp(&ps[2],"plik:",5)==0) ||
                        (memcmp(&ps[2],"iarchive:",9)==0) ||
                        (memcmp(&ps[2],"Datei:",6)==0) ||(memcmp(&ps[2],"datei:",6)==0) ||
                        (memcmp(&ps[2],"commons:",8)==0) ||
                        (memcmp(&ps[2],"wikisource:",11)==0) ||
                        (memcmp(&ps[2],"doi:",4)==0) ||
                        (memcmp(&ps[2],"fichier:",8)==0) ||
                        (memcmp(&ps[2],"utilisateur:",12)==0) ||
                        (memcmp(&ps[2],"hdl:",4)==0) ||
                        (memcmp(&ps[2],"irc:",4)==0) ||
                        (memcmp(&ps[2],"wikibooks:",10)==0) ||    
                        (memcmp(&ps[2],"meta:",5)==0) || 
                        (memcmp(&ps[2],"categoria:",10)==0) || 
                        (memcmp(&ps[2],"immagine:",9)==0) || 
                        (memcmp(&ps[3], "ikipedysta:",11)==0) || 
                        (memcmp(&ps[2],"wikia:",6)==0) || 
                        (memcmp(&ps[2],"incubator:",10)==0) || 
                        (memcmp(&ps[2],"ficheiro:",9)==0) || (memcmp(&ps[2],"Ficheiro:",9)==0) ||
                        (memcmp(&ps[2],"arquivo:",8)==0) ||
                        (memcmp(&ps[2],"foundation:",11)==0) ||
                        (memcmp(&ps[2],"template:",9)==0) ||
                        (memcmp(&ps[2],"wikinews:",9)==0) ||
                        (memcmp(&ps[2],"bild:",5)==0) ||
                        (memcmp(&ps[2],"fr:Wikipédia:Aide]]",20)==0) ||
                        (memcmp(&ps[2],"de:Boogie Down Produ",20)==0) ||
                        (memcmp(&ps[2],"da:Wikipedia:Hvordan",20)==0) ||
                        (memcmp(&ps[2],"sv:Indiska musikinstrument",26)==0) ||
                        (memcmp(&ps[2],"es:Coronel Sanders",18)==0) ||
                        ((memcmp(&ps[2],"fr:Wikip",8)==0)  &&  (memcmp(&ps[10+2],"dia:Aide",8)==0) ) ||
                        (memcmp(&ps[2],"pt:Wikipedia:Artigos cu",23)==0) ||
                        (lnu-b1<4) ){
                    // skip if not lang at end
                    do {
                        j=*in++; *out++=j;
                    }
                    while (j!=0); 
                    return;
                }
                f=1, lc=0;
                ++lc;
                for(i=0; i<j; i++)  if (*(int*)&in[i]==0x65742F3C && (*(in+4+i+2)=='>'))  f=0,lnu=0,b1=0;//'et/<'
                hent9(in,out);
                wfputs1(in,o);
                out[0]=0;
                return;
            }
        }
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
        return;
    }
    else if (f==1){
        ++lc;
        for(i=0; i<j; i++)  if (*(int*)&in[i]==0x65742F3C)  f=0,lnu=0,b1=0;//'et/<'
        hent9(in,out);
        wfputs1(in,o);
        out[0]=0;
    }
}

void henttail1( char *in,char *out,FileTmp *o,char *p2,int size) {
    int i, j, k;
    static int c=0 ,lnu=0, f=0;
    static  char *p4=p2;
    char su[8192*8];
    char *s=su;
    char ou[8192*8];
    ++lnu;
    j = strlen(in);
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x7865743C) c=1, f=lnu;//'xet<'
    for(i=0; i<j; i++) if (*(int*)&in[i]==0x65742F3C && (*(in+4+i+2)=='>' )) c=0; //'et/<' </te xt>

    if ((memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</sha1>",7)==0)&& c==1 && (lnu-f>=4) /*&& *p4!=0&& (p4-p2)<size*/) {
        c=0;
        do {
            k=(int)(strchr(p4,10)+1-(char*)p4);
            for (int i = 0; i<k;i++ ){
                su[i]=*p4++;
            }
            su[k]=0;
            // parse line 
            hent9(s,ou);
            if (!(strstr(s, "</text>") || strstr(s, "</revision>")|| strstr(s, "</page>")|| strstr(s, "</sha1>"))) {
                /*  int skip=0;
        char *p = strstr(s, "<text ");//, *w;
        if (p)  {   p = strchr(p, '>'); assert(p);  
        if(p[-1]=='/' || p[2]==0)  ;  
        else  skip= (char*)p-(char*)s; }*/
                
                restoreamp(s,ou,0);
                skipline(ou,s);
            } 
            
            hent6( s,ou);
            hent1( ou,s);
            hent3( s,ou);
            
            wfputs1(ou,o);
        }
        while (memcmp(&p4[-8],"</text>",7) );
        wfputs1(in,o); // out current line
        out[0]=0;
    }else{
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
    }
}

// combine this
// ,
void henttag1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=0;
    
    if (title==1 && move==0 && text) {
        move=1;
    }    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
        } else out[0]=0;
        wfputs1(in+k1,o);
        in[0]=0;
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}

int henttag1r( char *in,char *out,char *p2,int size,int title) {
    int i, j, k=0;
    static int c=0 ,lnu=0, f=0,move=0,text=0;;
    static  char *p4=p2;
    char *p8=out;

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=k=f=0,move=1;
    if (title==1 && move==0 && text)   {
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0/*||memcmp(&in[j-8],"</text>",7)==0*/){
            move=1;text=0;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 0; 
        }
    }

    
    if (move==0 && title && text && j) {
        if (f){
            // text line only
            for (int i = 0; i<k;i++ ) {
                out[i]=*in++;
            }
            f=0;
        }
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0) {
            move=text=title=0;
            return 0;
        } 
        return 1;
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
    
    return 0;
}

//file
void hentfiles1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=0;
    
    if (title && move==0 && text) {
        move=1;
    }
    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
            
        } else out[0]=0;
        wfputs1(in+k1,o);in[0]=0;

    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}
// :
int hentfiles1r( char *in,char *out,char *p2,int size,int title) {
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=k=f=0,move=1;
    if (title  && move==0 && text)   {
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0||memcmp(&in[j-8],"</text>",7)==0){
            move=1;text=0;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 0; 
        }
    }  
    
    if (move==0 && title && text && j) {
        if (f){
            // text line only
            for (int i = 0; i<k;i++ ){
                out[i]=*in++;
            }
            f=0;
            
        }
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0) {
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
    
    return 0;
}

//numbers
void hentnumbers1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 )) text=0;
    
    if (title && move==0 && text) {
        move=1;
    }
    
    if (move && title && j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            k1=(int)(strchr(in,'>')-(char*)p4);
            memcpy(out, in,k1);
            out[k1+1]=10;out[k1+2]=0;
            k1++;
            
        } else out[0]=0;
        wfputs1(in+k1,o);in[0]=0;

    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}
// :
int hentnumbers1r( char *in,char *out,char *p2,int size,int title) {
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(in+6,"<text ",6)==0  ) text=f=1,k=33,move=0;
    if (j>18 &&title && text==1 &&   (memcmp(&in[j-8],"</text>",7)==0 || memcmp(&in[j-4]," />",3)==0 ) ) text=k=f=0,move=1;
    if (title  && move==0 && text)   {
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</page>",7)==0||memcmp(&in[j-8],"</text>",7)==0) {
            move=1;text=0;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 0; 
        }
    }  
    
    if (move==0 && title && text && j) {
        if (f){
            // text line only
            for (int i = 0; i<k;i++ ){
                out[i]=*in++;
            }
            f=0;
            
        }
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i = 0; i<k1;i++ ) {
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0) {
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
    
    return 0;
}

//alb
void hentalb1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0 ) text=1;

    if (title && move==0 && text ) {
        move=1;
    }
    
    if (move && j) {
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            
        } else {
            out[0]=0;
            wfputs1(in+k1,o);in[0]=0;
        }
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}

int hentalb1r( char *in,char *out,char *p2,int size,int title) {
    int i, j, k=0;
    static int  move=0,text=0;
    static  char *p4=p2;
    // char *p8=out;
    j = strlen(in);

    if (title&& move==1 && text && j) {
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0) {
            move=text=title=0;
            return 0;
        } 
        return 1;    
    }
    else{
        if (j>13&&  size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "Album infobox |"))) {
            text=move=1;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 1;
        }
        
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
    }  
    return 0;
}

void hentdis1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    //char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;

    if (title && move==0 && text ) {
        move=1;
    }
    
    if (move &&  j){
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
        } else {
            out[0]=0;
            wfputs1(in+k1,o);in[0]=0;
        }
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}
int hentdis1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  move=0,text=0;
    static  char *p4=p2;
    //char *p8=out;
    j = strlen(in);

    if (title&& move==1 && text && j) {
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i=0; i<k1; i++) {
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0) {
            move=text=title=0;
            return 0;
        } 
        return 1;    
    } else {
        if (j>13&& size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "Disorder infobox |"))) {
            text=move=1;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 1;
        }
        
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
    }  
    return 0;
}

void hentclu1( char *in,char *out,FileTmp *o,int title) {
    int c, i=0,  j=0,k=0;
    static  int lnu=0,f=0, b1=0, lc=0,co=0,move=0,text=0;
    unsigned char   *ps;
    char *p4=in;
    char *p8=out;
    char s[8192*8];

    j = strlen(in);

    if (j>13&& title && text==0 &&  memcmp(p4+6,"<text ",6)==0  ) text=1;

    if (title && move==0 && text ) {
        move=1;
    }
    
    if (move && j) {
        if (memcmp(&in[j-12],"</revision>",11)==0 ||memcmp(&in[j-8],"</text>",7)==0) {
            move=text=title=0;
        } 
        int k1=0;
        if (j>13 && memcmp(p4+6,"<text ",6)==0  ) {
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            
        } else {
            out[0]=0;
            wfputs1(in+k1,o);in[0]=0;
        }
    }
    else
    do {
        j=*in++; *out++=j;
    }
    while (j!=0); 
}
int hentclu1r( char *in,char *out,char *p2,int size,int title){
    int i, j, k=0;
    static int  f=0,move=0,text=0;
    static  char *p4=p2;
    char *p8=out;
    j = strlen(in);

    if (title&& move==1 && text && j){
        int k1=(int)(strchr(p4,10)+1-(char*)p4);
        for (int i = 0; i<k1;i++ ){
            out[i+k]=*p4++;
        }
        out[k+k1]=0;
        j = strlen(out);
        if (memcmp(&out[j-12],"</revision>",11)==0 ||memcmp(&out[j-8],"</text>",7)==0){
            move=text=title=f=0;
            return 0;
        } 
        return 1;    
    } else {
        if (j>13&& size&& memcmp(in+6,"<text ",6)==0 &&  (strstr(in, "club infobox |"))) {
            text=f=1,move=1;
            do {
                j=*in++; *out++=j;
            }
            while (j!=0); 
            return 1;
        }
        
        do {
            j=*in++; *out++=j;
        }
        while (j!=0); 
    }  
    return 0;
}

witFilter::witFilter(std::string n, Filetype f) {  
    name=n;
    Type=f;
} 

void witFilter::encode(File *in, File *out, uint64_t size, uint64_t info) {
    char s[8192*8];
    char o[8192*8];
    FileTmp out1; // lang
    FileTmp out2; // tag
    FileTmp out3; // header
    FileTmp out4; // files
    FileTmp out5; // number
    FileTmp out6; // alb
    FileTmp out7; // dis
    FileTmp out8; // clu
    int i, j, f = 0, lastID = 0,tf=0;
    int ti=0,files=0,number=0,alb=0,diso=0,clu=0;
    do {
        wfgets(s, 8192*8, in);
        j = strlen(s);
        if (f==2) {
            if (*(int*)&s[4]==0x3E736E3C) { // ns '>sn<'
                char *p =strchr(s, '>');
                if (p) {
                    p = strchr(p+1, '<');
                    if (p)  p[0] = 10,  p[1] = 0;
                }
                if (s[0]!=' ' ||s[1]!=' ' ||s[2]!=' ' ||s[3]!=' ') {            
                    fsize=0;return;// just fail
                }
                wfputs1(s+5,&out3);
                continue;
            }
            // id
            int curID = atoi(&s[8]);
            if (*(int*)&s[4] != 0x3E64693C)  {            
                fsize=0;return;
            }// just fail it '>di<'
            //if (curID <= lastID)        return 0;
            sprintf(o,  ">%d%c", curID - lastID, 10);
            wfputs1(o,&out3);
            lastID = curID;
            f = 1;
            continue;
        }
        if (f) {
            if (*(int*)&s[6]==0x6D69743C) {//'mit<'
                int year   = atoi(&s[17]);
                int month  = atoi(&s[22]);
                int day    = atoi(&s[25]);
                int hour   = atoi(&s[28]);
                int minute = atoi(&s[31]);
                int second = atoi(&s[34]);
                sprintf(o, "timestamp>%02d%d:%d%c", 
                year-2001, month*31+day-32, hour*3600+minute*60+second, 10);
                wfputs1(o,&out3);
                continue;
            }
            char *p =strchr(s, '>');
            if (p) {
                p = strchr(p+1, '<');
                if (p)  p[0] = 10,  p[1] = 0;
            }
            if (s[0]!=' ' ||s[1]!=' ' ||s[2]!=' ' ||s[3]!=' ')
            {            
                fsize=0;return;// just fail
            }
            int s2=0; //lenght
            if (f==3) {
                if (*(int*)&s[6]==0x6F632F3C)  s2=7;//'oc/<'
                else                       s2=9;
            } else {
                if (*(int*)&s[4]==0x7665723C || *(int*)&s[4]==0x7365723C|| *(int*)&s[4]==0x6465723C) s2=5;//'ver<' 'ser<' 'der<'
                else                                             s2=7;
                if (*(int*)&s[6]==0x6E6F633C) {//'noc<'
                    f=3;
                    if (f==3 && *(int*)&s[6+4+4+4]==0x6C656420)  f=0; //'led 'special case "deleted"
                }
            }
            if (s2){
                wfputs1(s+s2,&out3);
            }
        }
        else  {hent(s,o);
            hent2(o,s);
            hent5(s,o);
            
            int skip=0;
            char *p = strstr(o, "<text ");
            if (p)  { tf=1, p = strchr(p, '>'); assert(p);  skip= (char*)p+1-(char*)o;
                if(p[-1]=='/' /*|| p[2]==0*/) tf=0;  
                if (strstr(o, "Disorder infobox |")) diso=1,ti=files=number=0;
                if (strstr(o, "Album infobox |")) alb=1,ti=files=number=0;
                if (strstr(o, "club infobox |")) clu=1,ti=files=number=0;
            }
            
            if (strstr(o, "</text>") || strstr(o, "</revision>")|| strstr(o, "</page>")|| strstr(o, "</sha1>"))  tf=0;
            if (tf) {
                removeamp(o,s,skip);
                skipline(s,o);
            }
            henttail(o,s,&out1);
            
            henttag1(s,o,&out2,ti);     if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hentfiles1(s,o,&out4,files);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hentnumbers1(s,o,&out5,number);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hentalb1(s,o,&out6,alb);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hentdis1(s,o,&out7,diso);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hentclu1(s,o,&out8,clu);if (s[0]==0)wfputs(o,out);if (o[0]==0|| s[0]==0) continue;
            hent9(o,s);
            wfputs(o,out);
        }
        for(i=0; i<j; i++)
        if (*(int*)&s[i]==0x69742F3C && *(int*)&s[i+4]==0x3E656C74&&*(int*)s==0x20202020) {f=2;//'it/<' '>elt'
            ti=files=number=0;
            alb=diso=clu=0;
            for(int i=0; i<j; i++){
                if (s[i]==',' )  ti=1;      
                if (s[i]==':' )  files=2;// article about files
                char *listsp = strstr(s, "List of ");
                if (listsp)files=2;
                if (s[i]=='>' && s[i+1]!=10 && s[i+1]>'0'&& s[i+1]<='9')  {
                    number = atoi(&s[i+1]);
                    if (number<10  )number=0;
                    else {number=1;break;}
                } 
            }
            if (ti && files==2) ti=0;
            if (number) ti=files=0;
            if (alb ||diso||clu)ti=files=number=0;
        }
        for(i=0; i<j; i++)
        if (*(int*)&s[i]==0x6F632F3C && *(int*)&s[i+4]==0x6972746E) f=0;//'oc/<' 'irtn'
    }
    while (!in->eof());
    // output tail to main file and report tail size as info
    U64 msize=out->curpos();
    int tsize=out1.curpos();
    int tagsize=out2.curpos();
    int headersize=out3.curpos();
    int filesize=out4.curpos();
    int numbersize=out5.curpos();
    int albsize=out6.curpos();
    int disosize=out7.curpos();  
    int clusize=out8.curpos();
    /*FileDisk aaa,bbb,ccc,ddd,eee,fff,ggg;
aaa.create("xxxxxxxtag");
bbb.create("xxxxxxxlang");
ccc.create("xxxxxxxheader");
ddd.create("xxxxxxxfiles");
eee.create("xxxxxxxalb");
fff.create("xxxxxxxdiso");
ggg.create("xxxxxxxclu");*/
    out1.setpos(0);
    out2.setpos(0);
    out3.setpos(0);
    out4.setpos(0);
    out5.setpos(0);
    out6.setpos(0);
    out7.setpos(0);
    out8.setpos(0);
    sprintf(o, "%d%c", headersize, 10);
    j=strlen(o);
    wfputs(o,out); //header
    sprintf(o, "%d%c", tagsize, 10);
    j=j+strlen(o);
    wfputs(o,out); //tag
    sprintf(o, "%d%c", filesize, 10);
    j=j+strlen(o);
    wfputs(o,out); //numbersize
    sprintf(o, "%d%c", numbersize, 10);
    j=j+strlen(o);
    wfputs(o,out); //filesize
    sprintf(o, "%d%c", tsize, 10);
    j=j+strlen(o);
    wfputs(o,out); //lang
    sprintf(o, "%d%c", albsize, 10);
    j=j+strlen(o);
    wfputs(o,out); //alb
    sprintf(o, "%d%c", disosize, 10);
    j=j+strlen(o);
    wfputs(o,out); //diso
    sprintf(o, "%d%c", clusize, 10);
    j=j+strlen(o);
    wfputs(o,out); //clu
    for(U64 i=0; i<clusize; i++) {
        int a=out8.getc();
        out->putc(a);
        //      ggg.putc(a);
    }
    for(U64 i=0; i<disosize; i++) {
        int a=out7.getc();
        out->putc(a);
        //      fff.putc(a);
    }
    for(U64 i=0; i<albsize; i++) {
        int a=out6.getc();
        out->putc(a);
        //      eee.putc(a);
    }
    for(U64 i=0; i<filesize; i++) {
        int a=out4.getc();
        out->putc(a);
        //     ddd.putc(a);
    }

    for(U64 i=0; i<numbersize; i++) {
        int a=out5.getc();
        out->putc(a);
        //     ddd.putc(a);
    }
    for(U64 i=0; i<tagsize; i++) {
        int a=out2.getc();
        out->putc(a);
        //     aaa.putc(a);
    }

    for(U64 i=0; i<headersize; i++) {
        int a=out3.getc();
        out->putc(a);
        //     ccc.putc(a);
    }

    for(U64 i=0; i<tsize; i++) {
        int a=out1.getc();
        out->putc(a);
        //    bbb.putc(a);
    }

    //aaa.close(); bbb.close();ccc.close();ddd.close();eee.close();fff.close();ggg.close();
    out8.close();
    out7.close();
    out6.close();
    out5.close();
    out4.close();
    out3.close();
    out2.close();
    out1.close();
    /*printf("Main size: %d kb\n",U32(msize/1024));
printf("tags size: %d kb\n",U32(tagsize/1024));
printf("file size: %d kb\n",U32(filesize/1024));
printf("number size: %d kb\n",U32(numbersize/1024));
printf("header size: %d kb\n",U32(headersize/1024));
printf("Langs size: %d kb\n",U32(tsize/1024));
printf("Alb size: %d kb\n",U32(albsize/1024));
printf("Diso size: %d kb\n",U32(disosize/1024));
printf("Clu size: %d kb\n",U32(clusize/1024));*/
    tsize=tsize+j+tagsize+headersize+filesize+numbersize+albsize+disosize+clusize;
    fsize=tsize;
} 

uint64_t witFilter::decode(File *in, File *out, uint64_t size, uint64_t info) {
    uint64_t len=size;
    int winfo=info;
    if (winfo==0) return 0; //fail
    FileTmp out1;
    char s[8192*8];
    char o[8192*8];
    int i, j, f = 0, lastID = 0,tf=0;
    U64 tsize=size-winfo; // winfo <- tail lenght
    in->setpos(tsize); // tail data pos
    //header
    wfgets(s, 16, in);    
    int headerlenght=atoi(&s[0]);
    char *h1=(char*)calloc(headerlenght+1,1);
    char *h1p=h1;
    // tag
    wfgets(s, 16, in);    
    int taglenght=atoi(&s[0]);
    char *t1=(char*)calloc(taglenght+1,1);
    // files
    wfgets(s, 16, in);    
    int fileslenght=atoi(&s[0]);
    char *f1=(char*)calloc(fileslenght+1,1);
    // numbers
    wfgets(s, 16, in);    
    int numberslenght=atoi(&s[0]);
    char *n1=(char*)calloc(numberslenght+1,1);
    //lang
    wfgets(s, 16, in);    
    int langlenght=atoi(&s[0]);
    char *p1=(char*)calloc(langlenght+1,1);
    //alb
    wfgets(s, 16, in);    
    int alblenght=atoi(&s[0]);
    char *a1=(char*)calloc(alblenght+1,1);
    //diso
    wfgets(s, 16, in);    
    int disolenght=atoi(&s[0]);
    char *d1=(char*)calloc(disolenght+1,1);
    //clu
    wfgets(s, 16, in);    
    int clulenght=atoi(&s[0]);
    char *l1=(char*)calloc(clulenght+1,1);
    
    if(clulenght)     in->blockread((U8*)l1,U64(clulenght));  //read clu
    if(disolenght)    in->blockread((U8*)d1,U64(disolenght));  //read dis
    if(alblenght)     in->blockread((U8*)a1,U64(alblenght));  //read alb
    if(fileslenght)   in->blockread((U8*)f1,U64(fileslenght));  //read files
    if(numberslenght) in->blockread((U8*)n1,U64(numberslenght));  //read numbers
    if(taglenght)     in->blockread((U8*)t1,U64(taglenght));  //read sens    
    if(headerlenght)in->blockread((U8*)h1,U64(headerlenght));  //read header
    if(langlenght)  in->blockread((U8*)p1,U64(langlenght));  //read lang
    
    in->setpos(0);
    int header=0,title=0,files=0,number=0,alb=0,diso=0,clu=0;
    do {
        wfgets(s, 8192*8, in);    
        
        j = strlen(s);
        if (in->curpos()>tsize) {
            j=j-(in->curpos()-tsize); // cut tail
            s[j]=0;
        }
        
        if (header==1) {
            int n=0;
            int cont=0;
            do {
                *(int*)o = 0x20202020;
                j=4;
                //'ider' 'iver' 'tser'
                if (*(int*)&h1p[0]!=0x69646572 &&*(int*)&h1p[0]!=0x69766572&& *(int*)&h1p[0]!=0x74736572 && n!=0) *(int*)(o+j)= 0x20202020,j=j+2;
                if (cont==1) *(int*)(o+j)= 0x20202020,j=j+2;
                if (*(int*)&h1p[0]==0x6E6F632F) cont=0;//'noc/'
                int k=(int)(strchr(h1p,10)+1-(char*)h1p);

                // id  
                if ( n==0){
                    if ( (*(int*)&h1p[0]&0xffffff)==0x3E736E){//'>sn'
                        o[j]='<';
                        memcpy(o+j+1, h1p, k);
                        int e=(strchr(h1p, '>')-h1p)+2;
                        if (e!=k || cont==1) { //end tag
                            o[k+j++]='<';
                            o[k+j++]='/';
                            memcpy(o+j+k, h1p, k);
                            j=j+(strchr(h1p, '>')-h1p)+1;
                        }
                        o[k+j++]=10;
                        o[k+j]=0;
                        wfputs1(o,&out1);
                    } else{
                        
                        n++;
                        lastID = lastID+ atoi(&h1p[1]);
                        sprintf(o+j, "<id>%d</id>%c",   lastID, 10);
                        wfputs1(o,&out1);
                    }
                }
                else if (*(int*)&h1p[0]==0x656D6974 ){//'emit'
                    char *p = strchr(h1p, ':');
                    int d = atoi(&h1p[19-7]), hms = atoi(p+1), h = hms/3600;
                    o[0]=h1p[17-7],o[1]=h1p[18-7],o[2]=' ';
                    int y=atoi(&o[0]);
                    sprintf(o, "      <timestamp>%d-%02d-%02dT%02d:%02d:%02dZ</timestamp>%c",
                    y + 2001, d/31+1, d%31+1, h, hms/60 - h*60, hms%60, 10);
                    wfputs1(o,&out1);  
                }
                else if (n || cont==1){
                    o[j]='<';
                    memcpy(o+j+1, h1p, k);
                    int e=(strchr(h1p, '>')-h1p)+2;
                    if (e!=k || cont==1) { //end tag
                        o[k+j++]='<';
                        o[k+j++]='/';
                        memcpy(o+j+k, h1p, k);
                        j=j+(strchr(h1p, '>')-h1p)+1;
                        
                    }
                    o[k+j++]=10;
                    o[k+j]=0;
                    wfputs1(o,&out1);
                    if (*(int*)&h1p[0]==0x746E6F63) cont=1;//'tnoc'
                }
                h1p=h1p+k;
                if (memcmp(&h1p[0],"contributor dele",16)==0)break;
            }
            while (memcmp(&h1p[0],"/contributor>",13)    );
            *(int*)o = 0x20202020;
            j=4;
            *(int*)(o+j)= 0x20202020,j=j+2;
            int k=(int)(strchr(h1p,10)+1-(char*)h1p);
            o[j]='<';
            memcpy(o+j+1, h1p, k);
            o[k+j++]=10;
            o[k+j]=0;
            wfputs1(o,&out1);
            h1p=h1p+k;
            header=0;
        }

        
            if (memcmp(&s[j-9],"</title>",8)==0 && *(int*)s==0x20202020) {header=1,title= files=number=0;
                for(i=0; i<j; i++) {

                    if (s[i]==',' )  title=1;
                    if (s[i]==':' )  files=2;
                    char *listsp = strstr(s, "List of ");
                    if (listsp)files=2;
                    if (s[i]=='>' && s[i+1]!=10 && s[i+1]>'0'&& s[i+1]<='9'/*&& atoi(&s[i+1])*/)  {
                        number = atoi(&s[i+1]);
                        //int nlen=numlen(&s[i+1]);
                        if (number<10 )number=0;//printf("%d, %d, %s",number,nlen,s);
                        else {number=1;break;}
                    } 
                }
                if (title && files==2) title=0;//,printf("%s",s); // active reset to files only
                if (files==1) files=0;
                if (number) title=files=0;
            }
        
        if (strstr(s, "<text ") && strstr(s, "Disorder infobox |")) diso=1,number= title=files=0;
        if (strstr(s, "<text ") && strstr(s, "Album infobox |")) alb=1,number= title=files=0;
        if (strstr(s, "<text ") && strstr(s, "club infobox |")) clu=1,number= title=files=0;
        int m=1,n=1,q=1,w=1,z=1,x=1;
        while (m||n||q||w||z||x){        // loop over until not
            m=henttag1r(s,o,t1,taglenght,title);
            n=hentfiles1r(o,s,f1,fileslenght,files);
            q=hentnumbers1r(s,o,n1,numberslenght,number);
            w=hentalb1r(o,s,a1,alblenght,alb);
            z=hentdis1r(s,o,d1,disolenght,diso);
            x=hentclu1r(o,s,l1,clulenght,clu);
            skipline(s,o);
            hent9(o,s);
            henttail1(o,s,&out1,p1,langlenght);
            if (s[0]==0) {   break;       }
            
            int skip=0;
            char *p = strstr(s, "<text ");//, *w;
            if (p)  { tf=1, p = strchr(p, '>'); assert(p);  skip= (char*)p+1-(char*)s; 
                if(p[-1]=='/' /*|| p[2]==0*/) tf=0;  
                
            }
            
            if (strstr(s, "</text>") || strstr(s, "</revision>")|| strstr(s, "</page>")|| strstr(s, "</sha1>"))  tf=alb=diso=clu=0;
            if (tf) {
                restoreamp(s,o,skip);
                skipline(o,s);
            }
            hent6( s,o);
            hent1( o,s);
            hent3( s,o);
            wfputs1(o,&out1);
            // printf("%s",o );
        }
    }
    while (in->curpos() < tsize);
    if(taglenght)   free(t1);
    if(fileslenght)   free(f1);
    if(numberslenght)  free(n1);
    if(headerlenght)free(h1);
    if(langlenght)  free(p1);
    if(alblenght)  free(a1);    
    if(disolenght)  free(d1);
    if(clulenght)  free(l1);
    int b=0,c=0;
    U64 bb=0L;
    bb=out1.curpos();
    out1.setpos(0);
    for (U64 i=0L; i<bb; i++) { //replace
        b=out1.getc();
        out->putc(b);
    }
    out1.close();
    in->setend(); // or it fails

    fsize=bb;

    return fsize;
}

witFilter::~witFilter() {
}

