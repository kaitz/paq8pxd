#pragma once
#include "types.hpp"
#include "helper.hpp"
#include <string>
#include <assert.h>
#ifdef WINDOWS
#include <windows.h>
#endif
//This is the base class.
//This is an abstract class for all the required file operations.
int putsize(std::string& archive, std::string& s, const char* fname, int base);
int expand(std::string& archive, std::string& s, const char* fname, int base);

class File {
public:
  virtual ~File(){};// = default;
  virtual bool open(const char* filename, bool must_succeed) = 0;
  virtual void create(const char* filename) = 0;
  virtual void close() = 0;
  virtual int getc() = 0;
  virtual void putc(U8 c) = 0;
  void append(const char* s) { for (int i = 0; s[i]; i++)putc(s[i]); }
  virtual U64 blockread(U8 *ptr, U64 count) = 0;
  virtual U64 blockwrite(U8 *ptr, U64 count) = 0;
  U32 get32() { return (getc() << 24) | (getc() << 16) | (getc() << 8) | (getc()); }
  void put32(U32 x){putc((x >> 24) & 255); putc((x >> 16) & 255); putc((x >> 8) & 255); putc(x & 255);}
  U64 get64() { 
  uint64_t u64val=0;
     for (int i=0; i<8; i++)u64val=(u64val<<8)+getc();
    return u64val; 
  }
  void put64(U64 x){putc((x >> 56) & 255);putc((x >> 48) & 255);putc((x >> 40) & 255);putc((x >> 32) & 255);putc((x >> 24) & 255); putc((x >> 16) & 255); putc((x >> 8) & 255); putc(x & 255);}
  /*U64 getVLI() {
      U64 i = 0;
      int k = 0;
      U8 b = 0;
      do {
          b = max(0, getc());
          i |= U64((b & 0x7FU) << k);
          k += 7;
      } while((b >> 7U) > 0 );
      return i;
  }
  void putVLI(U64 i) {
      while( i > 0x7F ) {
          putc(0x80U | (i & 0x7FU));
          i >>= 7U;
      }
      putc(U8(i));
  }*/
  virtual void setpos(U64 newpos) = 0;
  virtual void setend() = 0;
  virtual _off64_t curpos() = 0;
  virtual bool eof() = 0;
  virtual void dumpToDisk() = 0;
};
// This class is responsible for files on disk
// It simply passes function calls to the operating system

class FileDisk :public File {
protected:
  FILE *file;
public:
  FileDisk() {file=0;}
   ~FileDisk() {close();}
   void dumpToDisk(){}
  bool open(const char *filename, bool must_succeed) {
    assert(file==0); 
    file = fopen(filename, "rb"); 
    bool success=(file!=0);
    if(!success && must_succeed)printf("FileDisk: unable to open file (%s)\n", strerror(errno));
    return success; 
  }
  void create(const char *filename) { 
    assert(file==0); 
    makedirectories(filename); 
    file=fopen(filename, "wb+");
    if (!file) quit("FileDisk: unable to create file"); 
  }
  void createtmp() { 
    assert(file==0); 
    file = tmpfile2(); 
    if (!file) quit("FileDisk: unable to create temporary file"); 
  }
  void close() { if(file) fclose(file); file=0;}
  int getc() { return fgetc(file); }
  void putc(U8 c) { fputc(c, file); }
  U64 blockread(U8 *ptr, U64 count) {return fread(ptr,1,count,file);}
  U64 blockwrite(U8 *ptr, U64 count) {return fwrite(ptr,1,count,file);}
  void setpos(U64 newpos) { fseeko64(file, newpos, SEEK_SET); }
  void setend() { fseeko64(file, 0, SEEK_END); }
  _off64_t curpos() { return ftello64(file); }
  bool eof() { return feof(file)!=0; }
};

// This class is responsible for temporary files in RAM or on disk
// Initially it uses RAM for temporary file content.
// In case of the content size in RAM grows too large, it is written to disk, 
// the RAM is freed and all subsequent file operations will use the file on disk.
class FileTmp :public File {
private:
  //file content in ram
  Array<U8> *content_in_ram; //content of file
  U64 filepos;
  U64 filesize;
  void forget_content_in_ram()
  {
    if (content_in_ram) {
      delete content_in_ram;
      content_in_ram = 0;
      filepos = 0;
      filesize = 0;
    }
  }
  //file on disk
  FileDisk *file_on_disk;
  void forget_file_on_disk()
  {
    if (file_on_disk) {
      (*file_on_disk).close(); 
      delete file_on_disk;
      file_on_disk = 0;
    }
  }
  //switch: ram->disk
  const U32 MAX_RAM_FOR_TMP_CONTENT ; //64 MB (per file)
  void ram_to_disk()
  {
    assert(file_on_disk==0);
    file_on_disk = new FileDisk();
    (*file_on_disk).createtmp();
    if(filesize>0)
      (*file_on_disk).blockwrite(&((*content_in_ram)[0]), filesize);
    (*file_on_disk).setpos(filepos);
    forget_content_in_ram();
  }
public:
  FileTmp(): MAX_RAM_FOR_TMP_CONTENT( 64 * 1024 * 1024){content_in_ram=new Array<U8>(0); filepos=0; filesize=0; file_on_disk = 0;}
  ~FileTmp() {close();}
  bool open(const char *filename, bool must_succeed) { assert(false); return false; } //this method is forbidden for temporary files
  void create(const char *filename) { assert(false); } //this method is forbidden for temporary files
  void close() {
    forget_content_in_ram();
    forget_file_on_disk();
  }
  int getc() {
    if(content_in_ram)
    {
      if (filepos >= filesize)
        return EOF; 
      else {
        U8 c = (*content_in_ram)[(U32)filepos];
        filepos++; 
        return c; 
      }
    }
    else return (*file_on_disk).getc();
  }
  void putc(U8 c) {
    if(content_in_ram) {
      if (filepos < MAX_RAM_FOR_TMP_CONTENT) {
        if (filepos == filesize) { (*content_in_ram).push_back(c); filesize++; }
        else 
        (*content_in_ram)[(U32)filepos] = c;
        filepos++;
        //filesize++;
        return;
      }
      else ram_to_disk();
    }
    (*file_on_disk).putc(c);
  }
  U64 blockread(U8 *ptr, U64 count) {
    if(content_in_ram)
    {
      U64 available = filesize - filepos;
      if (available<count)count = available;
      if(count>0) memcpy(ptr, &((*content_in_ram)[(U32)filepos]), count);
      filepos += count;
      return count;
    }
    else return (*file_on_disk).blockread(ptr,count);
  }
  U64 blockwrite(U8 *ptr, U64 count) {
    if(content_in_ram) {
      if (filepos+count <= MAX_RAM_FOR_TMP_CONTENT) 
      { 
        (*content_in_ram).resize((U32)(filepos + count));
        if(count>0)memcpy(&((*content_in_ram)[(U32)filepos]), ptr, count);
        filesize += count;
        filepos += count;
        return count;
      }
      else ram_to_disk();
    }
    return (*file_on_disk).blockwrite(ptr,count);
  }
  void setpos(U64 newpos) { 
    if(content_in_ram) {
      /*if (newpos>filesize && newpos<MAX_RAM_FOR_TMP_CONTENT) (*content_in_ram).resize((U32)(newpos));
      else*/ if(newpos>filesize)ram_to_disk(); //panic: we don't support seeking past end of file - let's switch to disk
      else {filepos = newpos; return;}
    }  
     (*file_on_disk).setpos(newpos);
  }
  void dumpToDisk(){
      if (content_in_ram) ram_to_disk();
  }
  void setend() { 
    if(content_in_ram) filepos = filesize;
    else (*file_on_disk).setend();
  }
  _off64_t curpos() { 
    if(content_in_ram) return filepos;
    else return (*file_on_disk).curpos();
  }
  bool eof() { 
    if(content_in_ram)return filepos >= filesize;
    else return (*file_on_disk).eof();
  }
};

bool append(File* out, File* in);
