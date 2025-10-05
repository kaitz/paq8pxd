#include "helper.hpp"

extern U8 level;
// Error handler: print message if any, and exit
void quit(const char* message) {
  throw message; 
}

// strings are equal ignoring case?
int equals(const char* a, const char* b) {
  assert(a && b);
  while (*a && *b) {
    int c1=*a;
    if (c1>='A'&&c1<='Z') c1+='a'-'A';
    int c2=*b;
    if (c2>='A'&&c2<='Z') c2+='a'-'A';
    if (c1!=c2) return 0;
    ++a;
    ++b;
  }
  return *a==*b;
}

bool makedir(const char* dir) {
  struct stat status;
  bool success = stat(dir, &status)==0;
  if(success && (status.st_mode & S_IFDIR)!=0) return -1; //-1: directory already exists, no need to create
#ifdef WINDOWS
  return CreateDirectory(dir, 0)==TRUE;
#else
#ifdef UNIX
  return mkdir(dir, 0777)==0;
#else
  return false;
#endif
#endif
}

void makedirectories(const char* filename) {
  std::string path(filename);
  int start = 0;
  if(path[1]==':')start=2; //skip drive letter (c:)
  if(path[start] == '/' || path[start] == '\\')start++; //skip leading slashes (root dir)
  for (int i = start; path[i]; ++i) {
    if (path[i] == '/' || path[i] == '\\') {
      char savechar = path[i];
      path[i] = 0;
      const char* dirname = path.c_str();
      int created = makedir(dirname);
      if (created==0) {
        printf("Unable to create directory %s\n", dirname);
        quit("");
      }
      if(created==1)
       // printf("Created directory %s\n", dirname);
      path[i] = savechar;
    }
  }
}

/////////////////////////// File /////////////////////////////
// The main purpose of these classes is to keep temporary files in 
// RAM as mush as possible. The default behaviour is to simply pass 
// function calls to the operating system - except in case of temporary 
// files.

// Helper function: create a temporary file
//
// On Windows when using tmpfile() the temporary file may be created 
// in the root directory causing access denied error when User Account Control (UAC) is on.
// To avoid this issue with tmpfile() we simply use fopen() instead.
// We create the temporary file in the directory where the executable is launched from. 
// Luckily the MS C runtime library provides two (MS specific) fopen() flags: "T"emporary and "D"elete.


FILE* tmpfile2(void){
    FILE *f;
#if defined(WINDOWS)    
    int i;
    char temppath[MAX_PATH]; 
    char filename[MAX_PATH];
    
    //i=GetTempPath(MAX_PATH,temppath);          //store temp file in system temp path
    i=GetModuleFileName(NULL,temppath,MAX_PATH); //store temp file in program folder
    if ((i==0) || (i>MAX_PATH)) return NULL;
    char *p=strrchr(temppath, '\\');
    if (p==0) return NULL;
    p++;*p=0;
    if (GetTempFileName(temppath,"tmp",0,filename)==0) return NULL;
    f=fopen(filename,"w+bTD");
    if (f==NULL) unlink(filename);
    return f;
#else
    f=tmpfile();  // temporary file
    if (!f) return NULL;
    return f;
#endif
}
/*
FILE* maketmpfile(void) {
#if defined(WINDOWS)  
  char szTempFileName[MAX_PATH];
  UINT uRetVal = GetTempFileName(TEXT("."), TEXT("tmp"), 0, szTempFileName);
  if(uRetVal==0)return 0;
  return fopen(szTempFileName, "w+bTD");
#else
  return tmpfile();
#endif
}*/


U32 utf8_check(U8 *s) {
    int i=0;
    if (*s < 0x80)      /* 0xxxxxxx */
      return 0;
    else if ((s[0] & 0xe0) == 0xc0) {      /* 110XXXXx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 || (s[0] & 0xfe) == 0xc0)                        /* overlong? */
    return i;
      else
    i = 2;
    } else if ((s[0] & 0xf0) == 0xe0) {      /* 1110XXXX 10Xxxxxx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 ||
      (s[2] & 0xc0) != 0x80 ||
      (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||    /* overlong? */
      (s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||    /* surrogate? */
      (s[0] == 0xef && s[1] == 0xbf &&
       (s[2] & 0xfe) == 0xbe))                      /* U+FFFE or U+FFFF? */
    return i;
      else
    i = 3;
    } else if ((s[0] & 0xf8) == 0xf0) {      /* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
      if ((s[1] & 0xc0) != 0x80 ||
      (s[2] & 0xc0) != 0x80 ||
      (s[3] & 0xc0) != 0x80 ||
      (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||    /* overlong? */
      (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) /* > U+10FFFF? */
    return i;
      else
     i += 4;
    } else
      return i;
  return i;
}
 int min(int a, int b) {return a<b?a:b;}
 int max(int a, int b) {return a<b?b:a;}

U64 CMlimit(U64 size){
    //if (size>(0x100000000UL)) return (0x100000000UL); //limit to 4GB, using this will consume lots of memory above level 11
    if (size>(0x80000000UL)) return (0x80000000UL); //limit to 2GB
    return (size);
}

U64 MEM(){
     return 0x10000UL<<level;
}

U8 Clip(int const Px){
  if(Px>255)return 255;
  if(Px<0)return 0;
  return Px;
}



 
  U8 Clamp4(const int Px, const U8 n1, const U8 n2, const U8 n3, const U8 n4){
  int maximum=n1;if(maximum<n2)maximum=n2;if(maximum<n3)maximum=n3;if(maximum<n4)maximum=n4;
  int minimum=n1;if(minimum>n2)minimum=n2;if(minimum>n3)minimum=n3;if(minimum>n4)minimum=n4;
  if(Px<minimum)return minimum;
  if(Px>maximum)return maximum;
  return Px;
}
  U8 LogMeanDiffQt(const U8 a, const U8 b, const U8 limit ){
  if (a==b) return 0;
  U8 sign=a>b?8:0;
  return sign | min(limit, ilog2((a+b)/max(2,abs(a-b)*2)+1));
}
  U32 LogQt(const U8 Px, const U8 bits){
  return (U32(0x100|Px))>>max(0,(int)(ilog2(Px)-bits));
}


  U8 Paeth(U8 W, U8 N, U8 NW){
  int p = W+N-NW;
  int pW=abs(p-(int)W), pN=abs(p-(int)N), pNW=abs(p-(int)NW);
  if (pW<=pN && pW<=pNW) return W;
  else if (pN<=pNW) return N;
  return NW;
}

  bool CharInArray(const char c, const char a[], const int len) {
  if (*a==0)
    return false;
  int i=0;
  for (; i<len && c!=a[i]; i++);
  return i<len;
}
