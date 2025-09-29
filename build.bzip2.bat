gcc.exe -O3 -m64 -std=gnu99 -DWINDOWS -DMT -masm=intel -mavx2      -c bzip2/blocksort.c  bzip2/bzlib.c bzip2/compress.c  bzip2/crctable.c bzip2/decompress.c bzip2/huffman.c  bzip2/randtable.c 
ar r bzip2.a blocksort.o  bzlib.o compress.o crctable.o decompress.o huffman.o  randtable.o 
ranlib bzip2.a
del *.o
pause