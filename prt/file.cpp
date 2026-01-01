#include "file.hpp"
#ifdef UNIX
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#endif
//////////////////////////// User Interface ////////////////////////////


// int expand(String& archive, String& s, const char* fname, int base) {
// Given file name fname, print its length and base name (beginning
// at fname+base) to archive in format "%ld\t%s\r\n" and append the
// full name (including path) to String s in format "%s\n".  If fname
// is a directory then substitute all of its regular files and recursively
// expand any subdirectories.  Base initially points to the first
// character after the last / in fname, but in subdirectories includes
// the path from the topmost directory.  Return the number of files
// whose names are appended to s and archive.

// Same as expand() except fname is an ordinary file
int putsize(std::string& archive, std::string& s, const char* fname, int base) {
  int result=0;
  FileDisk f;
  bool success=f.open(fname,true);
  if (success) {
    f.setend();
    U64 len=f.curpos();
    if (len>=0) {
      static char blk[24];
      sprintf(blk, "%0.0f\t", len+0.0);
      archive+=blk;
      archive+=(fname+base);
      archive+="\n";
      s+=fname;
      s+="\n";
      ++result;
    }
    f.close();
  }
  return result;
}

#ifdef WINDOWS

int expand(std::string& archive, std::string& s, const char* fname, int base) {
  int result=0;
  DWORD attr=GetFileAttributes(fname);
  if ((attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
    WIN32_FIND_DATA ffd;
    std::string fdir(fname);
    fdir+="/*";
    HANDLE h=FindFirstFile(fdir.c_str(), &ffd);
    while (h!=INVALID_HANDLE_VALUE) {
      if (!equals(ffd.cFileName, ".") && !equals(ffd.cFileName, "..")) {
        std::string d(fname);
        d+="/";
        d+=ffd.cFileName;
        result+=expand(archive, s, d.c_str(), base);
      }
      if (FindNextFile(h, &ffd)!=TRUE) break;
    }
    FindClose(h);
  }
  else // ordinary file
    result=putsize(archive, s, fname, base);
  return result;
}

#else
#ifdef UNIX

int expand(std::string& archive, std::string& s, const char* fname, int base) {
  int result=0;
  struct stat sb;
  if (stat(fname, &sb)<0) return 0;

  // If a regular file and readable, get file size
  if (sb.st_mode & S_IFREG && sb.st_mode & 0400)
    result+=putsize(archive, s, fname, base);

  // If a directory with read and execute permission, traverse it
  else if (sb.st_mode & S_IFDIR && sb.st_mode & 0400 && sb.st_mode & 0100) {
    DIR *dirp=opendir(fname);
    if (!dirp) {
      perror("opendir");
      return result;
    }
    dirent *dp;
    while(errno=0, (dp=readdir(dirp))!=0) {
      if (!equals(dp->d_name, ".") && !equals(dp->d_name, "..")) {
        std::string d(fname);
        d+="/";
        d+=dp->d_name;
        result+=expand(archive, s, d.c_str(), base);
      }
    }
    if (errno) perror("readdir");
    closedir(dirp);
  }
  else printf("%s is not a readable file or directory\n", fname);
  return result;
}

#else  // Not WINDOWS or UNIX, ignore directories

int expand(std::string& archive, std::string& s, const char* fname, int base) {
  return putsize(archive, s, fname, base);
}

#endif
#endif

bool append(File* out, File* in) {
  if (!in) {
    quit("append in error\n");
    return false;
  }
  if (!out) {
    quit("append out error\n");
    return false;
  }
  const int BUFSIZE=4096;
  U8 buf[BUFSIZE];
  int n;
  while ((n=in->blockread(buf, BUFSIZE ))>0)
    out->blockwrite(buf,   n  );
  return true;
}

