gcc.exe -O3 -m64 -std=gnu99 -DWINDOWS -DMT -masm=intel -mavx2      -c zlib/adler32.c  zlib/crc32.c zlib/deflate.c  zlib/inffast.c zlib/inflate.c zlib/inftrees.c  zlib/zutil.c zlib/trees.c
ar r zlib.a adler32.o  crc32.o deflate.o  inffast.o inflate.o inftrees.o  zutil.o trees.o
ranlib zlib.a
del *.o
pause