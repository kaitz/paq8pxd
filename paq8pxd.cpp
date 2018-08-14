/* paq8pxd file compressor/archiver.  Release by Kaido Orav, Mar. 4, 2018

    Copyright (C) 2008-2014 Matt Mahoney, Serge Osnach, Alexander Ratushnyak,
    Bill Pettis, Przemyslaw Skibinski, Matthew Fite, wowtiger, Andrew Paterson,
    Jan Ondrus, Andreas Morphis, Pavel L. Holoborodko, Kaido Orav, Simon Berger,
    Neill Corlett

    LICENSE

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details at
    Visit <http://www.gnu.org/copyleft/gpl.html>.

To install and use in Windows:

- To install, put paq8pxd.exe or a shortcut to it on your desktop.
- To compress a file or folder, drop it on the paq8pxd icon.
- To decompress, drop a .paq8pxd file on the icon.

A .paq8pxd extension is added for compression, removed for decompression.
The output will go in the same folder as the input.

While paq8pxd is working, a command window will appear and report
progress.  When it is done you can close the window by pressing
ENTER or clicking [X].


COMMAND LINE INTERFACE

- To install, put paq8pxd.exe somewhere in your PATH.
- To compress:      paq8pxd [-N] file1 [file2...]
- To decompress:    paq8pxd [-d] file1.paq8pxd [dir2]
- To view contents: paq8pxd -l file1.paq8pxd

The compressed output file is named by adding ".paq8pxd" extension to
the first named file (file1.paq8pxd).  Each file that exists will be
added to the archive and its name will be stored without a path.
The option -N specifies a compression level ranging from -0
(fastest) to -8 (smallest).  The default is -5.  If there is
no option and only one file, then the program will pause when
finished until you press the ENTER key (to support drag and drop).
If file1.paq8pxd exists then it is overwritten. Level -0 only
transforms or decompresses data.

If the first named file ends in ".paq8pxd" then it is assumed to be
an archive and the files within are extracted to the same directory
as the archive unless a different directory (dir2) is specified.
The -d option forces extraction even if there is not a ".paq8pxd"
extension.  If any output file already exists, then it is compared
with the archive content and the first byte that differs is reported.
No files are overwritten or deleted.  If there is only one argument
(no -d or dir2) then the program will pause when finished until
you press ENTER.

For compression, if any named file is actually a directory, then all
files and subdirectories are compressed, preserving the directory
structure, except that empty directories are not stored, and file
attributes (timestamps, permissions, etc.) are not preserved.
During extraction, directories are created as needed.  For example:

  paq8pxd -4 c:\tmp\foo bar

compresses foo and bar (if they exist) to c:\tmp\foo.paq8pxd at level 4.

  paq8pxd -d c:\tmp\foo.paq8pxd .

extracts foo and compares bar in the current directory.  If foo and bar
are directories then their contents are extracted/compared.

There are no commands to update an existing archive or to extract
part of an archive.  Files and archives larger than 2GB are not
supported (but might work on 64-bit machines, not tested).
File names with nonprintable characters are not supported (spaces
are OK).


TO COMPILE

There are 2 files: paq8pxd.cpp (C++) and wrtpre.cpp (C++).
paq8pxd.cpp recognizes the following compiler options:

  -DWINDOWS           (to compile in Windows)
  -DUNIX              (to compile in Unix, Linux, etc)
  -DMT                (to compile with multithreading support)
  -DDEFAULT_OPTION=N  (to change the default compression level from 5 to N).

If you compile without -DWINDOWS or -DUNIX, you can still compress files,
but you cannot compress directories or create them during extraction.
You can extract directories if you manually create the empty directories
first.

Use -DEFAULT_OPTION=N to change the default compression level to support
drag and drop on machines with less than 256 MB of memory.  Use
-DDEFAULT_OPTION=4 for 128 MB, 3 for 64 MB, 2 for 32 MB, etc.


Recommended compiler commands and optimizations:

  MINGW g++ (x86,x64):
   with multithreading:
    g++ paq8pxd.cpp -DWINDOWS -DMT -msse2 -O3 -s -static -o paq8pxd.exe 
   without multithreading:
    g++ paq8pxd.cpp -DWINDOWS -msse2 -O3 -s -static -o paq8pxd.exe 

  UNIX/Linux (PC x86,x64):
   with multithreading:
    g++ paq8pxd.cpp -DUNIX -DMT -msse2 -O3 -s -static -lpthread -o paq8pxd
   without multithreading:
    g++ paq8pxd.cpp -DUNIX -msse2 -O3 -s -static -lpthread -o paq8pxd

  Non PC (e.g. PowerPC under MacOS X)
    g++ paq8pxd.cpp -O2 -DUNIX -s -o paq8pxd


ARCHIVE FILE FORMAT

An archive has the following format.  

  paq8pxd -N 
  segment offset 8 bytes
  segment size  2 bytes
  compressed segment size 2 bytes
  streams (0b00000xxx xxxxxxxx) 2 bytes
  \0 file list size
  compressed file list(
    size TAB filename CR LF
    size TAB filename CR LF
    ...)
  compressed binary data
  file segmentation data
  stream data sizes[11]

-N is the option (-0 to -15) and mode, even if a default was used.
00LMNNNN bit M is set if fast mode, 
         bit L is set if quick mode,
         if L or M are not set default to slow mode.

segment size is total size of file(s) 
compressed segment size is compressed segmentation data in bytes
at segmnet offset after compressed binary data.

file segmentation data is full list of detected blocks:
type size info
type size info
type size 
type size info
.....

info is present if block type needs extra info like in image or audio.

streams - if bit is set then stream is present. Right to left order
for stream 10 to 0. If bit is set store stream lengt to archive.

Plain file names are stored without a path.  Files in compressed
directories are stored with path relative to the compressed directory
(using UNIX style forward slashes "/").  For example, given these files:

  123 C:\dir1\file1.txt
  456 C:\dir2\file2.txt

Then

  paq8pxd archive \dir1\file1.txt \dir2

will create archive.paq8pxd 

The command:

  paq8pxd archive.paq8pxd C:\dir3

will create the files:

  C:\dir3\file1.txt
  C:\dir3\dir2\file2.txt

Decompression will fail if the first 10 bytes are not "paq8pxd -".  Sizes
are stored as decimal numbers.  CR, LF, TAB are ASCII codes
13, 10, 9 respectively.


ARITHMETIC CODING

The binary data is arithmetic coded as the shortest base 256 fixed point
number x = SUM_i x_i 256^-1-i such that p(<y) <= x < p(<=y), where y is the
input string, x_i is the i'th coded byte, p(<y) (and p(<=y)) means the
probability that a string is lexicographcally less than (less than
or equal to) y according to the model, _ denotes subscript, and ^ denotes
exponentiation.

The model p(y) for y is a conditional bit stream,
p(y) = PROD_j p(y_j | y_0..j-1) where y_0..j-1 denotes the first j
bits of y, and y_j is the next bit.  Compression depends almost entirely
on the ability to predict the next bit accurately.


MODEL MIXING

paq8pxd uses a neural network to combine a large number of models.  The
i'th model independently predicts
p1_i = p(y_j = 1 | y_0..j-1), p0_i = 1 - p1_i.
The network computes the next bit probabilty

  p1 = squash(SUM_i w_i t_i), p0 = 1 - p1                        (1)

where t_i = stretch(p1_i) is the i'th input, p1_i is the prediction of
the i'th model, p1 is the output prediction, stretch(p) = ln(p/(1-p)),
and squash(s) = 1/(1+exp(-s)).  Note that squash() and stretch() are
inverses of each other.

After bit y_j (0 or 1) is received, the network is trained:

  w_i := w_i + eta t_i (y_j - p1)                                (2)

where eta is an ad-hoc learning rate, t_i is the i'th input, (y_j - p1)
is the prediction error for the j'th input but, and w_i is the i'th
weight.  Note that this differs from back propagation:

  w_i := w_i + eta t_i (y_j - p1) p0 p1                          (3)

which is a gradient descent in weight space to minimize root mean square
error.  Rather, the goal in compression is to minimize coding cost,
which is -log(p0) if y = 1 or -log(p1) if y = 0.  Taking
the partial derivative of cost with respect to w_i yields (2).


MODELS

Most models are context models.  A function of the context (last few
bytes) is mapped by a lookup table or hash table to a state which depends
on the bit history (prior sequence of 0 and 1 bits seen in this context).
The bit history is then mapped to p1_i by a fixed or adaptive function.
There are several types of bit history states:

- Run Map. The state is (b,n) where b is the last bit seen (0 or 1) and
  n is the number of consecutive times this value was seen.  The initial
  state is (0,0).  The output is computed directly:

    t_i = (2b - 1)K log(n + 1).

  where K is ad-hoc, around 4 to 10.  When bit y_j is seen, the state
  is updated:

    (b,n) := (b,n+1) if y_j = b, else (y_j,1).

- Stationary Map.  The state is p, initially 1/2.  The output is
  t_i = stretch(p).  The state is updated at ad-hoc rate K (around 0.01):

    p := p + K(y_j - p)

- Nonstationary Map.  This is a compromise between a stationary map, which
  assumes uniform statistics, and a run map, which adapts quickly by
  discarding old statistics.  An 8 bit state represents (n0,n1,h), initially
  (0,0,0) where:

    n0 is the number of 0 bits seen "recently".
    n1 is the number of 1 bits seen "recently".
    n = n0 + n1.
    h is the full bit history for 0 <= n <= 4,
      the last bit seen (0 or 1) if 5 <= n <= 15,
      0 for n >= 16.

  The primaty output is t_i := stretch(sm(n0,n1,h)), where sm(.) is
  a stationary map with K = 1/256, initialized to
  sm(n0,n1,h) = (n1+(1/64))/(n+2/64).  Four additional inputs are also
  be computed to improve compression slightly:

    p1_i = sm(n0,n1,h)
    p0_i = 1 - p1_i
    t_i   := stretch(p_1)
    t_i+1 := K1 (p1_i - p0_i)
    t_i+2 := K2 stretch(p1) if n0 = 0, -K2 stretch(p1) if n1 = 0, else 0
    t_i+3 := K3 (-p0_i if n1 = 0, p1_i if n0 = 0, else 0)
    t_i+4 := K3 (-p0_i if n0 = 0, p1_i if n1 = 0, else 0)

  where K1..K4 are ad-hoc constants.

  h is updated as follows:
    If n < 4, append y_j to h.
    Else if n <= 16, set h := y_j.
    Else h = 0.

  The update rule is biased toward newer data in a way that allows
  n0 or n1, but not both, to grow large by discarding counts of the
  opposite bit.  Large counts are incremented probabilistically.
  Specifically, when y_j = 0 then the update rule is:

    n0 := n0 + 1, n < 29
          n0 + 1 with probability 2^(27-n0)/2 else n0, 29 <= n0 < 41
          n0, n = 41.
    n1 := n1, n1 <= 5
          round(8/3 lg n1), if n1 > 5

  swapping (n0,n1) when y_j = 1.

  Furthermore, to allow an 8 bit representation for (n0,n1,h), states
  exceeding the following values of n0 or n1 are replaced with the
  state with the closest ratio n0:n1 obtained by decrementing the
  smaller count: (41,0,h), (40,1,h), (12,2,h), (5,3,h), (4,4,h),
  (3,5,h), (2,12,h), (1,40,h), (0,41,h).  For example:
  (12,2,1) 0-> (7,1,0) because there is no state (13,2,0).

- Match Model.  The state is (c,b), initially (0,0), where c is 1 if
  the context was previously seen, else 0, and b is the next bit in
  this context.  The prediction is:

    t_i := (2b - 1)Kc log(m + 1)

  where m is the length of the context.  The update rule is c := 1,
  b := y_j.  A match model can be implemented efficiently by storing
  input in a buffer and storing pointers into the buffer into a hash
  table indexed by context.  Then c is indicated by a hash table entry
  and b can be retrieved from the buffer.


CONTEXTS

High compression is achieved by combining a large number of contexts.
Most (not all) contexts start on a byte boundary and end on the bit
immediately preceding the predicted bit.  The contexts below are
modeled with both a run map and a nonstationary map unless indicated.

- Order n.  The last n bytes, up to about 16.  For general purpose data.
  Most of the compression occurs here for orders up to about 6.
  An order 0 context includes only the 0-7 bits of the partially coded
  byte and the number of these bits (255 possible values).

- Sparse.  Usually 1 or 2 of the last 8 bytes preceding the byte containing
  the predicted bit, e.g (2), (3),..., (8), (1,3), (1,4), (1,5), (1,6),
  (2,3), (2,4), (3,6), (4,8).  The ordinary order 1 and 2 context, (1)
  or (1,2) are included above.  Useful for binary data.

- Text.  Contexts consists of whole words (a-z, converted to lower case
  and skipping other values).  Contexts may be sparse, e.g (0,2) meaning
  the current (partially coded) word and the second word preceding the
  current one.  Useful contexts are (0), (0,1), (0,1,2), (0,2), (0,3),
  (0,4).  The preceding byte may or may not be included as context in the
  current word.

- Formatted text.  The column number (determined by the position of
  the last linefeed) is combined with other contexts: the charater to
  the left and the character above it.

- Fixed record length.  The record length is determined by searching for
  byte sequences with a uniform stride length.  Once this is found, then
  the record length is combined with the context of the bytes immediately
  preceding it and the corresponding byte locations in the previous
  one or two records (as with formatted text).

- Context gap.  The distance to the previous occurrence of the order 1
  or order 2 context is combined with other low order (1-2) contexts.

- FAX.  For 2-level bitmapped images.  Contexts are the surrounding
  pixels already seen.  Image width is assumed to be 1728 bits (as
  in calgary/pic).

- Image.  For uncompressed 8,24-bit color BMP, TIFF and TGA images.
  Contexts are the high order bits of the surrounding pixels and linear
  combinations of those pixels, including other color planes.  The
  image width is detected from the file header.  When an image is
  detected, other models are turned off to improve speed.

- JPEG.  Files are further compressed by partially uncompressing back
  to the DCT coefficients to provide context for the next Huffman code.
  Only baseline DCT-Huffman coded files are modeled.  (This ia about
  90% of images, the others are usually progresssive coded).  JPEG images
  embedded in other files (quite common) are detected by headers.  The
  baseline JPEG coding process is:
  - Convert to grayscale and 2 chroma colorspace.
  - Sometimes downsample the chroma images 2:1 or 4:1 in X and/or Y.
  - Divide each of the 3 images into 8x8 blocks.
  - Convert using 2-D discrete cosine transform (DCT) to 64 12-bit signed
    coefficients.
  - Quantize the coefficients by integer division (lossy).
  - Split the image into horizontal slices coded independently, separated
    by restart codes.
  - Scan each block starting with the DC (0,0) coefficient in zigzag order
    to the (7,7) coefficient, interleaving the 3 color components in
    order to scan the whole image left to right starting at the top.
  - Subtract the previous DC component from the current in each color.
  - Code the coefficients using RS codes, where R is a run of R zeros (0-15)
    and S indicates 0-11 bits of a signed value to follow.  (There is a
    special RS code (EOB) to indicate the rest of the 64 coefficients are 0).
  - Huffman code the RS symbol, followed by S literal bits.
  The most useful contexts are the current partially coded Huffman code
  (including S following bits) combined with the coefficient position
  (0-63), color (0-2), and last few RS codes.

- Match.  When a context match of 400 bytes or longer is detected,
  the next bit of the match is predicted and other models are turned
  off to improve speed.

- Exe.  When a x86 file (.exe, .obj, .dll) is detected, sparse contexts
  with gaps of 1-12 selecting only the prefix, opcode, and the bits
  of the modR/M byte that are relevant to parsing are selected.
  This model is turned off otherwise.

- Indirect.  The history of the last 1-3 bytes in the context of the
  last 1-2 bytes is combined with this 1-2 byte context.

- DMC. A bitwise n-th order context is built from a state machine using
  DMC, described in http://plg.uwaterloo.ca/~ftp/dmc/dmc.c
  The effect is to extend a single context, one bit at a time and predict
  the next bit based on the history in this context.  The model here differs
  in that two predictors are used.  One is a pair of counts as in the original
  DMC.  The second predictor is a bit history state mapped adaptively to
  a probability as as in a Nonstationary Map.

ARCHITECTURE

The context models are mixed by several of several hundred neural networks
selected by a low-order context.  The outputs of these networks are
combined using a second neural network, then fed through several stages of
adaptive probability maps (APM) before arithmetic coding.

For images, only one neural network is used and its context is fixed.

An APM is a stationary map combining a context and an input probability.
The input probability is stretched and divided into 32 segments to
combine with other contexts.  The output is interpolated between two
adjacent quantized values of stretch(p1).  There are 2 APM stages in series:

  p1 := (p1 + 3 APM(order 0, p1)) / 4.
  p1 := (APM(order 1, p1) + 2 APM(order 2, p1) + APM(order 3, p1)) / 4.

PREPROCESSING

paq8pxd uses preprocessing transforms on certain data types to improve
compression.  To improve reliability, the decoding transform is
tested during compression to ensure that the input file can be
restored.  If the decoder output is not identical to the input file
due to a bug, then the transform is abandoned and the data is compressed
without a transform so that it will still decompress correctly.

The input is split into blocks with the format <type> <decoded size> <info>
where <type> is 1 byte (0 = no transform), <decoded size> is the size
of the data after decoding, which may be different than the size of <data>.
Data is stored uncompressed after compressed data ends.
The preprocessor has 3 parts:

- Detector.  Splits the input into smaller blocks depending on data type.

- Coder.  Input is a block to be compressed.  Output is a temporary
  file.  The coder determines whether a transform is to be applied
  based on file type, and if so, which one.  A coder may use lots
  of resources (memory, time) and make multiple passes through the
  input file.  The file type is stored (as one byte) during compression.

- Decoder.  Performs the inverse transform of the coder.  It uses few
  resorces (fast, low memory) and runs in a single pass (stream oriented).
  It takes input either from a file or the arithmetic decoder.  Each call
  to the decoder returns a single decoded byte.

The following transforms are used:

- EXE:  CALL (0xE8) and JMP (0xE9) address operands are converted from
  relative to absolute address.  The transform is to replace the sequence
  E8/E9 xx xx xx 00/FF by adding file offset modulo 2^25 (signed range,
  little-endian format).  Data to transform is identified by trying the
  transform and applying a crude compression test: testing whether the
  byte following the E8/E8 (LSB of the address) occurred more recently
  in the transformed data than the original and within 4KB 4 times in
  a row.  The block ends when this does not happen for 4KB.

- JPEG: detected by SOI and SOF and ending with EOI or any nondecodable
  data.  No transform is applied.  The purpose is to separate images
  embedded in execuables to block the EXE transform, and for a future
  place to insert a transform.
  
- BASE64: Decodes BASE64 encoded data and recursively transformed
  up to level 5. Input can be full stream or end-of-line coded.
  
- BASE85: Decodes Ascii85 encoded data and recursively transformed
  up to level 5. Input can be full stream or end-of-line coded.
  Supports: https://en.wikipedia.org/wiki/Ascii85#Adobe_version

- 24-bit images: 24-bit image data uses simple color transform
  (b, g, r) -> (g, g-r, g-b)
  
- ZLIB:  Decodes zlib encoded data and recursively transformed
  up to level 5. Supports zlib compressed images (4/8/24 bit) in pdf

- GIF:  Gif (8 bit) image  recompression. 
  
- CD: mode 1 and mode 2 form 1+2 - 2352 bytes

- MDF: wraped around CD, re-arranges subchannel  and CD data

- TEXT: All detected text blocks are transformed using dynamic dictionary
  preprocessing (based on XWRT). If transformed block is larger from original
  then transform is skipped.
  
- LZSS: (Haruhiko Okumura's LZSS): Decompresses Microsoft compress.exe archives.

- TTA: audio filter 

- MRB: 8 bit images with RLE compression

- EOL: 


IMPLEMENTATION

Hash tables are designed to minimize cache misses, which consume most
of the CPU time.

Most of the memory is used by the nonstationary context models.
Contexts are represented by 32 bits, possibly a hash.  These are
mapped to a bit history, represented by 1 byte.  The hash table is
organized into 64-byte buckets on cache line boundaries.  Each bucket
contains 7 x 7 bit histories, 7 16-bit checksums, and a 2 element LRU
queue packed into one byte.  Each 7 byte element represents 7 histories
for a context ending on a 3-bit boundary plus 0-2 more bits.  One
element (for bits 0-1, which have 4 unused bytes) also contains a run model
consisting of the last byte seen and a count (as 1 byte each).

Run models use 4 byte hash elements consisting of a 2 byte checksum, a
repeat count (0-255) and the byte value.  The count also serves as
a priority.

Stationary models are most appropriate for small contexts, so the
context is used as a direct table lookup without hashing.

The match model maintains a pointer to the last match until a mismatching
bit is found.  At the start of the next byte, the hash table is referenced
to find another match.  The hash table of pointers is updated after each
whole byte.  There is no checksum.  Collisions are detected by comparing
the current and matched context in a rotating buffer.

The inner loops of the neural network prediction (1) and training (2)
algorithms are implemented in MMX assembler, which computes 4 elements
at a time.  Using assembler is 8 times faster than C++ for this code
and 1/3 faster overall.  (However I found that SSE2 code on an AMD-64,
which computes 8 elements at a time, is not any faster).


DIFFERENCES FROM PAQ8PXD_V50
-better detection for NES
-changes from paq8px_v155
-Estonian stemmer (minimal), mostly conflicts with French
-add compiler info for gcc, clang
-removed dmcModel from jpegModel
-detect ARM executables, do address translation
*/

#define PROGNAME "paq8pxd51"  // Please change this if you change the program.
#define SIMD_GET_SSE  //uncomment to use SSE2 in ContexMap
//#define MT            //uncomment for multithreading, compression only
#define SIMD_CM_R       // SIMD ContextMap byterun

#ifdef WINDOWS                       
#ifdef MT
//#define PTHREAD       //uncomment to force pthread to igore windows native threads
#endif
#endif

#ifdef UNIX
#ifdef MT 
#define PTHREAD 1
#endif
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "zlib.h"
#define NDEBUG  // remove for debugging (turns on Array bound checks)
#include <assert.h>

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <cstdio>
#include <ctype.h>
#include <sys/cdefs.h>
#include <dirent.h>
#include <errno.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#ifndef DEFAULT_OPTION
#define DEFAULT_OPTION 8
#endif
#include <stdint.h>
#ifdef _MSC_VER

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#endif
// 8, 16, 32 bit unsigned types (adjust as appropriate)
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;
typedef uint64_t   U64;
typedef signed char int8_t;

// min, max functions
#if  !defined(WINDOWS) || !defined (min)
inline int min(int a, int b) {return a<b?a:b;}
inline int max(int a, int b) {return a<b?b:a;}
#endif

#if defined(WINDOWS) || defined(_MSC_VER)
    #define atoll(S) _atoi64(S)
#endif

#ifdef _MSC_VER  
#define fseeko(a,b,c) _fseeki64(a,b,c)
#define ftello(a) _ftelli64(a)
#else
#ifndef UNIX
#ifndef fseeko
#define fseeko(a,b,c) fseeko64(a,b,c)
#endif
#ifndef ftello
#define ftello(a) ftello64(a)
#endif
#endif
#endif


#ifdef PTHREAD
#include "pthread.h"
#endif

// Error handler: print message if any, and exit
void quit(const char* message=0) {
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

//////////////////////// Program Checker /////////////////////

// Track time and memory used
class ProgramChecker {
    private:
  U64 memused;  // bytes allocated by Array<T> now
  U64 maxmem;   // most bytes allocated ever
  clock_t start_time;  // in ticks
public:
   void alloc(U64 n) {  // report memory allocated
    memused+=n;
    if (memused>maxmem) maxmem=memused;
  }
  void free(U64 n) {  // free memory 
  if (memused<n) memused=0;
  else  memused-=n;
  }
  ProgramChecker(): memused(0), maxmem(0) {
    start_time=clock();
    assert(sizeof(U8)==1);
    assert(sizeof(U16)==2);
    assert(sizeof(U32)==4);
    assert(sizeof(U64)==8);
    assert(sizeof(short)==2);
    assert(sizeof(int)==4);  
  }
  void print() const {  // print time and memory used
    printf("Time %1.2f sec, used %0lu MB (%0lu bytes) of memory\n",
      double(clock()-start_time)/CLOCKS_PER_SEC, ((maxmem)/1024)/1024,(maxmem));
  }
} programChecker;

//////////////////////////// Array ////////////////////////////

// Array<T, ALIGN> a(n); creates n elements of T initialized to 0 bits.
// Constructors for T are not called.
// Indexing is bounds checked if assertions are on.
// a.size() returns n.
// a.resize(n) changes size to n, padding with 0 bits or truncating.
// a.push_back(x) appends x and increases size by 1, reserving up to size*2.
// a.pop_back() decreases size by 1, does not free memory.
// Copy and assignment are not supported.
// Memory is aligned on a ALIGN byte boundary (power of 2), default is none.

template <class T, int ALIGN=0> class Array {
private:
  U32 n;     // user size
  U32 reserved;  // actual size
  char *ptr; // allocated memory, zeroed
  T* data;   // start of n elements of aligned data
  void create(U32 i);  // create with size i
public:
  explicit Array(U32 i=0) {create(i);}
  ~Array();
  T& operator[](U32 i) {
#ifndef NDEBUG
  //  if (i<0 || i>=n)    fprintf(stderr, "%d out of bounds %d\n", i, n), quit();
#endif
    return data[i];
  }
  const T& operator[](U32 i) const {
#ifndef NDEBUG
  //  if (i<0 || i>=n)    fprintf(stderr, "%d out of bounds %d\n", i, n), quit();
#endif
    return data[i];
  }
  U32 size() const {return n;}
  void resize(U32 i);  // change size to i
  void pop_back() {if (n>0) --n;}  // decrement size
  void push_back(const T& x);  // increment size, append x
  Array(const Array&) { assert(false); } 
private:
  //Array(const Array&);  // no copy or assignment
  Array& operator=(const Array&);
};

template<class T, int ALIGN> void Array<T, ALIGN>::resize(U32 i) {
  if (i<=reserved) {
    n=i;
    return;
  }
  char *saveptr=ptr;
  T *savedata=data;
  int saven=n;
  create(i);
  if (saveptr) {
    if (savedata) {
      memcpy(data, savedata, sizeof(T)*min(i, saven));
      programChecker.free(U64(ALIGN+n*sizeof(T)));
    }
    free(saveptr);
  }
}

template<class T, int ALIGN> void Array<T, ALIGN>::create(U32 i) {
  n=reserved=i;
  if (i<=0 || ALIGN&(ALIGN-1)) {
    data=0;
    ptr=0;
    return;
  }

  const size_t sz=ALIGN+n*sizeof(T);
  assert(sz>0);
//#ifndef NDEBUG 
//printf("\nAlloc: Try to allocate %0.0f bytes\n",sz+0.0);
//#endif
  ptr = (char*)calloc(sz, 1);
  if (!ptr) printf("\nError: Allocating %0lu bytes\n",sz),programChecker.print(), quit("Out of memory");
  programChecker.alloc(U64(sz));
  data = (ALIGN ? (T*)(ptr+ALIGN-(((long long)ptr)&(ALIGN-1))) : (T*)ptr);
  assert((char*)data>=ptr && (char*)data<=ptr+ALIGN);
}

template<class T, int ALIGN> Array<T, ALIGN>::~Array() {
  programChecker.free(U64(ALIGN+n*sizeof(T)));
  free(ptr);
}

template<class T, int ALIGN> void Array<T, ALIGN>::push_back(const T& x) {
  if (n==reserved) {
    int saven=n;
    resize(max(1, n*2));
    n=saven;
  }
  data[n++]=x;
}

/////////////////////////// String /////////////////////////////

// A tiny subset of std::string
// size() includes NUL terminator.

class String: public Array<char> {
public:
  const char* c_str() const {return &(*this)[0];}
  void operator=(const char* s) {
    resize(strlen(s)+1);
    strcpy(&(*this)[0], s);
  }
  void operator+=(const char* s) {
    assert(s);
    pop_back();
    while (*s) push_back(*s++);
    push_back(0);
  }
  String(const char* s=""): Array<char>(1) {
    (*this)+=s;
  }
};

/////////////////////////// IO classes //////////////////////////
// These classes  take the responsibility for all the file/folder
// operations.

/////////////////////////// Folders /////////////////////////////
/*
int makedir(const char* dir) {
  struct stat status;
  stat(dir, &status);
  if (status.st_mode & S_IFDIR) return -1; //-1: directory already exists, no need to create
  #ifdef WINDOWS
    bool created = (CreateDirectory(dir, 0) == TRUE);
  #else
  #ifdef UNIX
    bool created = (mkdir(dir, 0777) == 0);
  #else
    #error Unknown target system
  #endif
  #endif
  return created?1:0; //0: failed, 1: created successfully
};
*/
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
  String path(filename);
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
        quit();
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


//This is the base class.
//This is an abstract class for all the required file operations.
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
  U64 get64() { return ((U64)getc() << 56) | ((U64)getc() << 48) | ((U64)getc() << 40) | ((U64)getc() << 32) | (getc() << 24) | (getc() << 16) | (getc() << 8) | (getc()); }
  void put64(U64 x){putc((x >> 56) & 255);putc((x >> 48) & 255);putc((x >> 40) & 255);putc((x >> 32) & 255);putc((x >> 24) & 255); putc((x >> 16) & 255); putc((x >> 8) & 255); putc(x & 255);}
  virtual void setpos(U64 newpos) = 0;
  virtual void setend() = 0;
  virtual U64 curpos() = 0;
  virtual bool eof() = 0;
};

// This class is responsible for files on disk
// It simply passes function calls to the operating system
class FileDisk :public File {
protected:
  FILE *file;
public:
  FileDisk() {file=0;}
   ~FileDisk() {close();}
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
  void setpos(U64 newpos) { fseeko(file, newpos, SEEK_SET); }
  void setend() { fseeko(file, 0, SEEK_END); }
  U64 curpos() { return ftello(file); }
  bool eof() { return feof(file)!=0; }
};

// This class is responsible for temporary files in RAM or on disk
// Initially it uses RAM for temporary file content.
// In case of the content size in RAM grows too large, it is written to disk, 
// the RAM is freed and all subsequent file operations will use the file on disk.
class FileTmp :public File {
private:
  //file content in ram
  Array<U8, 1> *content_in_ram; //content of file
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
  FileTmp(): MAX_RAM_FOR_TMP_CONTENT( 16 * 1024 * 1024){content_in_ram=new Array<U8,1>(MAX_RAM_FOR_TMP_CONTENT); filepos=0; filesize=0; file_on_disk = 0;}
  ~FileTmp() {close();}
  bool open(const char *filename, bool must_succeed) { assert(true); return false; } //this method is forbidden for temporary files
  void create(const char *filename) { assert(true); } //this method is forbidden for temporary files
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
        if (filepos == filesize) { /*(*content_in_ram).push_back(c); */filesize++; }
        //else 
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
      if(newpos>filesize)ram_to_disk(); //panic: we don't support seeking past end of file - let's switch to disk
      else {filepos = newpos; return;}
    }  
    else (*file_on_disk).setpos(newpos);
  }
  void setend() { 
    if(content_in_ram) filepos = filesize;
    else (*file_on_disk).setend();
  }
  U64 curpos() { 
    if(content_in_ram) return filepos;
    else return (*file_on_disk).curpos();
  }
  bool eof() { 
    if(content_in_ram)return filepos >= filesize;
    else return (*file_on_disk).eof();
  }
};

//////////////////////////// rnd ///////////////////////////////

// 32-bit pseudo random number generator
class Random{
  Array<U32> table;
  int i;
public:
  Random(): table(64) {
    table[0]=123456789;
    table[1]=987654321;
    for (int j=0; j<62; j++) table[j+2]=table[j+1]*11+table[j]*23/16;
    i=0;
  }
  U32 operator()() {
    return ++i, table[i&63]=table[(i-24)&63]^table[(i-55)&63];
  }
} ;

////////////////////////////// Buf /////////////////////////////

// Buf(n) buf; creates an array of n bytes (must be a power of 2).
// buf[i] returns a reference to the i'th byte with wrap (no out of bounds).
// buf(i) returns i'th byte back from pos (i > 0)
// buf.size() returns n.

class Buf {
  Array<U8> b;
public:
int pos;  // Number of input bytes in buf (is wrapped)
int poswr; //wrapp
  Buf(U32 i=0): b(i),pos(0) {poswr=i-1;}
  void setsize(int i) {
    if (!i) return;
    assert(i>0 && (i&(i-1))==0);
    poswr=i-1;
    b.resize(i);
  }
  U8& operator[](int i) {
    return b[i&(b.size()-1)];
  }
  int operator()(int i) const {
    assert(i<b.size());
    //assert(i>0);
    return b[(pos-i)&(b.size()-1)];
  }
  int size() const {
    return b.size();
  }
};

// IntBuf(n) is a buffer of n int (must be a power of 2).
// intBuf[i] returns a reference to i'th element with wrap.

class IntBuf {
  Array<int> b;
public:
  IntBuf(U32 i=0): b(i) {}
  int& operator[](U32 i) {
    return b[i&(b.size()-1)];
  }
};

// Buffer for file segment info 
// type size info(if not -1)
class Segment {
  Array<U8> b;
public:
    U32 pos;  //size of buffer
    U64 hpos; //header pos points to segment info at archive end
    //int count; //count of segments
  Segment(int i=0): b(i),pos(0),hpos(0)/*,count(0)*/ {}
  void setsize(int i) {
    if (!i) return;
    assert(i>0);
    b.resize(i);
  }
  U8& operator[](U32 i) {
      if (i>=b.size()) setsize(i+1);
    return b[i];
  }
  U8 operator()(U32 i) const {
    assert(i>=0);
    assert(i<=b.size());
    return b[i];
  }
  
  // put 8 bytes to segment buffer
  void put8(U64 num){
    if ((pos+8)>=b.size()) setsize(pos+8);
    b[pos++]=(num>>56)&0xff;
    b[pos++]=(num>>48)&0xff;
    b[pos++]=(num>>40)&0xff;
    b[pos++]=(num>>32)&0xff;
    b[pos++]=(num>>24)&0xff;
    b[pos++]=(num>>16)&0xff;
    b[pos++]=(num>>8)&0xff;
    b[pos++]=num&0xff;  
  }
  void put4(U32 num){
    if ((pos+4)>=b.size()) setsize(pos+4);
    b[pos++]=(num>>24)&0xff;
    b[pos++]=(num>>16)&0xff;
    b[pos++]=(num>>8)&0xff;
    b[pos++]=num&0xff;  
  }
  void put1(U8 num){
    if (pos>=b.size()) setsize(pos+1);
    b[pos++]=num;
  }
  int size() const {
    return b.size();
  }
};

/////////////////////// Global context /////////////////////////
//bool modeFast=false;
bool modeQuick=false;
U64 level=DEFAULT_OPTION;  // Compression level 0 to 15
U64 MEM(){
     return 0x10000UL<<level;
}

Segment segment; //for file segments type size info(if not -1)
const int streamc=12;
File * filestreams[streamc];
const int datatypecount=39;
typedef enum {DEFAULT=0,BINTEXT,DBASE, JPEG, HDR,IMGUNK, IMAGE1,IMAGE4, IMAGE8,IMAGE8GRAY, IMAGE24,IMAGE32, AUDIO, EXE,DECA,ARM,
              CD, TEXT,TEXT0, TXTUTF8,NESROM, BASE64, BASE85, GIF ,SZDD,MRBR,
              ZLIB,MDF,MSZIP,EOLTEXT,DICTTXT,BIGTEXT,NOWRT,TAR,PNG8, PNG8GRAY,PNG24, PNG32,TEXT0PDF,TYPELAST} Filetype;
typedef enum {INFO=0, STREAM,RECURSIVE} Filetypes;
const char* typenames[datatypecount]={"default","bintext","dBase", "jpeg", "hdr", "imgunk","1b-image", "4b-image", "8b-image","8b-gimage", "24b-image","32b-image", "audio",
                                "exe","DECa","ARM", "cd", "text","text0","utf-8","nes","base64","base85","gif","SZDD","mrb","zlib","mdf","mszip","eoltxt",
                                "","","","tar","PNG8","PNG8G","PNG24","PNG32","TEXT0PDF"};
static const int typet[TYPELAST][3]={
 // info, stream, recursive
  { 0, 0, 0},// DEFAULT, 
  { 1, 0, 0},// BINTEXT, 
  { 1, 0, 0},// DBASE, 
  { 0, 1, 0},// JPEG,
  { 0, 0, 0},// HDR,  
  { 1, 0, 0},// IMGUNK
  { 1, 2, 0},// IMAGE1,  
  { 1, 3, 0},// IMAGE4, 
  { 1, 4, 0},// IMAGE8,    
  { 1, 4, 0},// IMAGE8GRAY,
  { 1, 5, 0},// IMAGE24,
  { 1, 5, 0},// IMAGE32,
  { 1, 6, 0},// AUDIO, 
  { 0, 7, 0},// EXE,
  { 0, 11, 0},// DECA, 
   { 0, 0, 0},// ARM, 
  { 0,-1, 1},//  CD,
  { 0, 9, 0},// TEXT,  
  { 0, 8, 0},// TEXT0,
  { 0, 9, 0},// TXTUTF8,  
  { 0, 0, 0},// NESROM,  
  { 0,-1, 1},// BASE64, 
  { 0,-1, 1},// BASE85,   
  { 0,-1, 0},// GIF,    
  { 1,-1, 1},// SZDD,  
  { 0,-1, 0},// MRBR, 
  { 1,-1, 1},// ZLIB, 
  { 0,-1, 1},// MDF, 
  { 0, 0, 0},// MSZIP,   
  { 0,-1, 1},// EOLTEXT,
  { 0, 9, 0},// DICTTXT,
  { 0,10, 0},// BIGTEXT,
  { 0,10, 0},// NOWRT, 
  { 0,-1, 0},// TAR,  
  { 1, 4, 0},// PNG8,
  { 1, 4, 0},// PNG8GRAY,
  { 1, 5, 0},// PNG24,
  { 1, 5, 0},// PNG32,
   { 1, -1, 1}// TEXT0PDF,
   /*if (type==DEFAULT || type==HDR || type==NESROM || type==MSZIP|| type==DECA || type==BINTEXT || type==DBASE||type==IMGUNK) return 0;
    else if (type==JPEG ) return  1;
    else if (type==IMAGE1) return 2;
    else if (type==IMAGE4) return 3;
    else if (type==IMAGE8|| type==IMAGE8GRAY ||type==PNG8|| type==PNG8GRAY) return 4;
    else if (type==IMAGE24|| type==IMAGE32 ||type==PNG24|| type==PNG32) return 5;
    else if (type==AUDIO) return 6;
    else if (type==EXE) return 7;
    else if (type==TEXT0) return 8; //text stream with lots of 0-9
    else if (type==TXTUTF8  || type== DICTTXT || type==TEXT) return 9;
    else if (type==BIGTEXT|| type== NOWRT) return 10;
    //else if (type==NESROM) return 11;*/
  };
const U32 WRT_mpw[16]= { 4, 4, 3, 2, 2, 2, 1, 1,  1, 1, 1, 1, 0, 0, 0, 0 };
const U32 WRT_mtt[16]= { 0, 0, 1, 2, 3, 4, 5, 5,  6, 6, 6, 6, 7, 7, 7, 7 };
const U32 tri[4]={0,4,3,7}, trj[4]={0,6,6,12};

#define PNGFlag (1<<31)
#define GrayFlag (1<<30)
// Contain all global data usable between models
class BlockData {
public: 
    Segment segment; //for file segments type size info(if not -1)
    int y; // Last bit, 0 or 1, set by encoder
    int c0; // Last 0-7 bits of the partial byte with a leading 1 bit (1-255)
    U32 c4; // Last 4 whole bytes, packed.  Last byte is bits 0-7.
    int bpos; // bits in c0 (0 to 7)
    Buf buf;  // Rotating input queue set by Predictor
    Buf bufn;  // Rotating input queue set by Predictor
    int blpos; // Relative position in block
    int rm1;
    Filetype filetype;
    U32 b2,b3,b4,w4;
    U32 w5,f4,tt;
    U32 col;
    U32 x4,s4;
    int finfo;
    U32 fails, failz, failcount,x5;
    U32 frstchar,spafdo,spaces,spacecount, words,wordcount,wordlen,wordlen1;
    U32 clevel; //level in predictor
     struct {
    U8 state:3;
    U8 lastPunct:5;
    U8 wordLength:4;
    U8 boolmask:4;
    U8 firstLetter;
    U8 mask;
  } Text;
  struct {
    U32 length;
    U8 byte;
  } Match;
  struct {
    struct {
      U8 WW, W, NN, N, Wp1, Np1;
    } pixels;
    U8 plane;
    U8 ctx;
  } Image;
  U64 Misses;
BlockData():y(0), c0(1), c4(0),bpos(0),blpos(0),rm1(1),filetype(DEFAULT),
    b2(0),b3(0),b4(0),w4(0), w5(0),f4(0),tt(0),col(0),x4(0),s4(0),finfo(0),fails(0),failz(0),
    failcount(0),x5(0), frstchar(0),spafdo(0),spaces(0),spacecount(0), words(0),wordcount(0),wordlen(0),wordlen1(0),Text{0},Match{0},Image{0},Misses(0){
        clevel=(U32)level;
        // Set globals according to option
        assert(level<=15);
        bufn.setsize(0x10000);
        if (level>=9) buf.setsize(0x10000000); //limit 256mb
        else buf.setsize(MEM()*8);
        #ifndef NDEBUG 
        printf("\n Buf size %d bytes\n", buf.poswr);
        #endif
        //if (modeFast) clevel=0;
    }
    ~BlockData(){ }
};
///////////////////////////// ilog //////////////////////////////

// ilog(x) = round(log2(x) * 16), 0 <= x < 64K
class Ilog {
  Array<U8> t;
public:
  int operator()(U16 x) const {return t[x];}
  Ilog();
} ilog;

// Compute lookup table by numerical integration of 1/x
Ilog::Ilog(): t(65536) {
  U32 x=14155776;
  for (int i=2; i<65536; ++i) {
    x+=774541002/(i*2-1);  // numerator is 2^29/ln 2
    t[i]=x>>24;
  }
}

// llog(x) accepts 32 bits
inline int llog(U32 x) {
  if (x>=0x1000000)
    return 256+ilog(x>>16);
  else if (x>=0x10000)
    return 128+ilog(x>>8);
  else
    return ilog(x);
}
inline U32 BitCount(U32 v) {
  v -= ((v >> 1) & 0x55555555);
  v = ((v >> 2) & 0x33333333) + (v & 0x33333333);
  v = ((v >> 4) + v) & 0x0f0f0f0f;
  v = ((v >> 8) + v) & 0x00ff00ff;
  v = ((v >> 16) + v) & 0x0000ffff;
  return v;
}

// ilog2
// returns floor(log2(x)), e.g. 30->4  31->4  32->5,  33->5
#ifdef _MSC_VER
#include <intrin.h>
inline U32 ilog2(U32 x) {
  DWORD tmp=0;
  if(x!=0)_BitScanReverse(&tmp,x);
  return tmp;
}
#elif __GNUC__
inline U32 ilog2(U32 x) {
  if(x!=0)x=31-__builtin_clz(x);
  return x;
}
#else
inline U32 ilog2(U32 x) {
  //copy the leading "1" bit to its left (0x03000000 -> 0x03ffffff)
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >>16);
  //how many trailing bits do we have (except the first)? 
  return BitCount(x >> 1);
}
#endif
///////////////////////// state table ////////////////////////

// State table:
//   nex(state, 0) = next state if bit y is 0, 0 <= state < 256
//   nex(state, 1) = next state if bit y is 1
//   nex(state, 2) = number of zeros in bit history represented by state
//   nex(state, 3) = number of ones represented
//
// States represent a bit history within some context.
// State 0 is the starting state (no bits seen).
// States 1-30 represent all possible sequences of 1-4 bits.
// States 31-252 represent a pair of counts, (n0,n1), the number
//   of 0 and 1 bits respectively.  If n0+n1 < 16 then there are
//   two states for each pair, depending on if a 0 or 1 was the last
//   bit seen.
// If n0 and n1 are too large, then there is no state to represent this
// pair, so another state with about the same ratio of n0/n1 is substituted.
// Also, when a bit is observed and the count of the opposite bit is large,
// then part of this count is discarded to favor newer data over old.

#if 1 // change to #if 0 to generate this table at run time (4% slower)
static const U8 State_table[256][4]={
  {  1,  2, 0, 0},{  3,  5, 1, 0},{  4,  6, 0, 1},{  7, 10, 2, 0}, // 0-3
  {  8, 12, 1, 1},{  9, 13, 1, 1},{ 11, 14, 0, 2},{ 15, 19, 3, 0}, // 4-7
  { 16, 23, 2, 1},{ 17, 24, 2, 1},{ 18, 25, 2, 1},{ 20, 27, 1, 2}, // 8-11
  { 21, 28, 1, 2},{ 22, 29, 1, 2},{ 26, 30, 0, 3},{ 31, 33, 4, 0}, // 12-15
  { 32, 35, 3, 1},{ 32, 35, 3, 1},{ 32, 35, 3, 1},{ 32, 35, 3, 1}, // 16-19
  { 34, 37, 2, 2},{ 34, 37, 2, 2},{ 34, 37, 2, 2},{ 34, 37, 2, 2}, // 20-23
  { 34, 37, 2, 2},{ 34, 37, 2, 2},{ 36, 39, 1, 3},{ 36, 39, 1, 3}, // 24-27
  { 36, 39, 1, 3},{ 36, 39, 1, 3},{ 38, 40, 0, 4},{ 41, 43, 5, 0}, // 28-31
  { 42, 45, 4, 1},{ 42, 45, 4, 1},{ 44, 47, 3, 2},{ 44, 47, 3, 2}, // 32-35
  { 46, 49, 2, 3},{ 46, 49, 2, 3},{ 48, 51, 1, 4},{ 48, 51, 1, 4}, // 36-39
  { 50, 52, 0, 5},{ 53, 43, 6, 0},{ 54, 57, 5, 1},{ 54, 57, 5, 1}, // 40-43
  { 56, 59, 4, 2},{ 56, 59, 4, 2},{ 58, 61, 3, 3},{ 58, 61, 3, 3}, // 44-47
  { 60, 63, 2, 4},{ 60, 63, 2, 4},{ 62, 65, 1, 5},{ 62, 65, 1, 5}, // 48-51
  { 50, 66, 0, 6},{ 67, 55, 7, 0},{ 68, 57, 6, 1},{ 68, 57, 6, 1}, // 52-55
  { 70, 73, 5, 2},{ 70, 73, 5, 2},{ 72, 75, 4, 3},{ 72, 75, 4, 3}, // 56-59
  { 74, 77, 3, 4},{ 74, 77, 3, 4},{ 76, 79, 2, 5},{ 76, 79, 2, 5}, // 60-63
  { 62, 81, 1, 6},{ 62, 81, 1, 6},{ 64, 82, 0, 7},{ 83, 69, 8, 0}, // 64-67
  { 84, 71, 7, 1},{ 84, 71, 7, 1},{ 86, 73, 6, 2},{ 86, 73, 6, 2}, // 68-71
  { 44, 59, 5, 3},{ 44, 59, 5, 3},{ 58, 61, 4, 4},{ 58, 61, 4, 4}, // 72-75
  { 60, 49, 3, 5},{ 60, 49, 3, 5},{ 76, 89, 2, 6},{ 76, 89, 2, 6}, // 76-79
  { 78, 91, 1, 7},{ 78, 91, 1, 7},{ 80, 92, 0, 8},{ 93, 69, 9, 0}, // 80-83
  { 94, 87, 8, 1},{ 94, 87, 8, 1},{ 96, 45, 7, 2},{ 96, 45, 7, 2}, // 84-87
  { 48, 99, 2, 7},{ 48, 99, 2, 7},{ 88,101, 1, 8},{ 88,101, 1, 8}, // 88-91
  { 80,102, 0, 9},{103, 69,10, 0},{104, 87, 9, 1},{104, 87, 9, 1}, // 92-95
  {106, 57, 8, 2},{106, 57, 8, 2},{ 62,109, 2, 8},{ 62,109, 2, 8}, // 96-99
  { 88,111, 1, 9},{ 88,111, 1, 9},{ 80,112, 0,10},{113, 85,11, 0}, // 100-103
  {114, 87,10, 1},{114, 87,10, 1},{116, 57, 9, 2},{116, 57, 9, 2}, // 104-107
  { 62,119, 2, 9},{ 62,119, 2, 9},{ 88,121, 1,10},{ 88,121, 1,10}, // 108-111
  { 90,122, 0,11},{123, 85,12, 0},{124, 97,11, 1},{124, 97,11, 1}, // 112-115
  {126, 57,10, 2},{126, 57,10, 2},{ 62,129, 2,10},{ 62,129, 2,10}, // 116-119
  { 98,131, 1,11},{ 98,131, 1,11},{ 90,132, 0,12},{133, 85,13, 0}, // 120-123
  {134, 97,12, 1},{134, 97,12, 1},{136, 57,11, 2},{136, 57,11, 2}, // 124-127
  { 62,139, 2,11},{ 62,139, 2,11},{ 98,141, 1,12},{ 98,141, 1,12}, // 128-131
  { 90,142, 0,13},{143, 95,14, 0},{144, 97,13, 1},{144, 97,13, 1}, // 132-135
  { 68, 57,12, 2},{ 68, 57,12, 2},{ 62, 81, 2,12},{ 62, 81, 2,12}, // 136-139
  { 98,147, 1,13},{ 98,147, 1,13},{100,148, 0,14},{149, 95,15, 0}, // 140-143
  {150,107,14, 1},{150,107,14, 1},{108,151, 1,14},{108,151, 1,14}, // 144-147
  {100,152, 0,15},{153, 95,16, 0},{154,107,15, 1},{108,155, 1,15}, // 148-151
  {100,156, 0,16},{157, 95,17, 0},{158,107,16, 1},{108,159, 1,16}, // 152-155
  {100,160, 0,17},{161,105,18, 0},{162,107,17, 1},{108,163, 1,17}, // 156-159
  {110,164, 0,18},{165,105,19, 0},{166,117,18, 1},{118,167, 1,18}, // 160-163
  {110,168, 0,19},{169,105,20, 0},{170,117,19, 1},{118,171, 1,19}, // 164-167
  {110,172, 0,20},{173,105,21, 0},{174,117,20, 1},{118,175, 1,20}, // 168-171
  {110,176, 0,21},{177,105,22, 0},{178,117,21, 1},{118,179, 1,21}, // 172-175
  {110,180, 0,22},{181,115,23, 0},{182,117,22, 1},{118,183, 1,22}, // 176-179
  {120,184, 0,23},{185,115,24, 0},{186,127,23, 1},{128,187, 1,23}, // 180-183
  {120,188, 0,24},{189,115,25, 0},{190,127,24, 1},{128,191, 1,24}, // 184-187
  {120,192, 0,25},{193,115,26, 0},{194,127,25, 1},{128,195, 1,25}, // 188-191
  {120,196, 0,26},{197,115,27, 0},{198,127,26, 1},{128,199, 1,26}, // 192-195
  {120,200, 0,27},{201,115,28, 0},{202,127,27, 1},{128,203, 1,27}, // 196-199
  {120,204, 0,28},{205,115,29, 0},{206,127,28, 1},{128,207, 1,28}, // 200-203
  {120,208, 0,29},{209,125,30, 0},{210,127,29, 1},{128,211, 1,29}, // 204-207
  {130,212, 0,30},{213,125,31, 0},{214,137,30, 1},{138,215, 1,30}, // 208-211
  {130,216, 0,31},{217,125,32, 0},{218,137,31, 1},{138,219, 1,31}, // 212-215
  {130,220, 0,32},{221,125,33, 0},{222,137,32, 1},{138,223, 1,32}, // 216-219
  {130,224, 0,33},{225,125,34, 0},{226,137,33, 1},{138,227, 1,33}, // 220-223
  {130,228, 0,34},{229,125,35, 0},{230,137,34, 1},{138,231, 1,34}, // 224-227
  {130,232, 0,35},{233,125,36, 0},{234,137,35, 1},{138,235, 1,35}, // 228-231
  {130,236, 0,36},{237,125,37, 0},{238,137,36, 1},{138,239, 1,36}, // 232-235
  {130,240, 0,37},{241,125,38, 0},{242,137,37, 1},{138,243, 1,37}, // 236-239
  {130,244, 0,38},{245,135,39, 0},{246,137,38, 1},{138,247, 1,38}, // 240-243
  {140,248, 0,39},{249,135,40, 0},{250, 69,39, 1},{ 80,251, 1,39}, // 244-247
  {140,252, 0,40},{249,135,41, 0},{250, 69,40, 1},{ 80,251, 1,40}, // 248-251
  {140,252, 0,41}};  // 252, 253-255 are reserved

#define nex(state,sel) State_table[state][sel]

// The code used to generate the above table at run time (4% slower).
// To print the table, uncomment the 4 lines of print statements below.
// In this code x,y = n0,n1 is the number of 0,1 bits represented by a state.
#else

class StateTable {
  Array<U8> ns;  // state*4 -> next state if 0, if 1, n0, n1
  enum {B=5, N=64}; // sizes of b, t
  static const int b[B];  // x -> max y, y -> max x
  static U8 t[N][N][2];  // x,y -> state number, number of states
  int num_states(int x, int y);  // compute t[x][y][1]
  void discount(int& x);  // set new value of x after 1 or y after 0
  void next_state(int& x, int& y, int b);  // new (x,y) after bit b
public:
  int operator()(int state, int sel) {return ns[state*4+sel];}
  StateTable();
} nex;

const int StateTable::b[B]={42,41,13,6,5};  // x -> max y, y -> max x
U8 StateTable::t[N][N][2];

int StateTable::num_states(int x, int y) {
  if (x<y) return num_states(y, x);
  if (x<0 || y<0 || x>=N || y>=N || y>=B || x>=b[y]) return 0;

  // States 0-30 are a history of the last 0-4 bits
  if (x+y<=4) {  // x+y choose x = (x+y)!/x!y!
    int r=1;
    for (int i=x+1; i<=x+y; ++i) r*=i;
    for (int i=2; i<=y; ++i) r/=i;
    return r;
  }

  // States 31-255 represent a 0,1 count and possibly the last bit
  // if the state is reachable by either a 0 or 1.
  else
    return 1+(y>0 && x+y<16);
}

// New value of count x if the opposite bit is observed
void StateTable::discount(int& x) {
  if (x>2) x=ilog(x)/6-1;
}

// compute next x,y (0 to N) given input b (0 or 1)
void StateTable::next_state(int& x, int& y, int b) {
  if (x<y)
    next_state(y, x, 1-b);
  else {
    if (b) {
      ++y;
      discount(x);
    }
    else {
      ++x;
      discount(y);
    }
    while (!t[x][y][1]) {
      if (y<2) --x;
      else {
        x=(x*(y-1)+(y/2))/y;
        --y;
      }
    }
  }
}

// Initialize next state table ns[state*4] -> next if 0, next if 1, x, y
StateTable::StateTable(): ns(1024) {

  // Assign states
  int state=0;
  for (int i=0; i<256; ++i) {
    for (int y=0; y<=i; ++y) {
      int x=i-y;
      int n=num_states(x, y);
      if (n) {
        t[x][y][0]=state;
        t[x][y][1]=n;
        state+=n;
      }
    }
  }

  // Print/generate next state table
  state=0;
  for (int i=0; i<N; ++i) {
    for (int y=0; y<=i; ++y) {
      int x=i-y;
      for (int k=0; k<t[x][y][1]; ++k) {
        int x0=x, y0=y, x1=x, y1=y;  // next x,y for input 0,1
        int ns0=0, ns1=0;
        if (state<15) {
          ++x0;
          ++y1;
          ns0=t[x0][y0][0]+state-t[x][y][0];
          ns1=t[x1][y1][0]+state-t[x][y][0];
          if (x>0) ns1+=t[x-1][y+1][1];
          ns[state*4]=ns0;
          ns[state*4+1]=ns1;
          ns[state*4+2]=x;
          ns[state*4+3]=y;
        }
        else if (t[x][y][1]) {
          next_state(x0, y0, 0);
          next_state(x1, y1, 1);
          ns[state*4]=ns0=t[x0][y0][0];
          ns[state*4+1]=ns1=t[x1][y1][0]+(t[x1][y1][1]>1);
          ns[state*4+2]=x;
          ns[state*4+3]=y;
        }
          // uncomment to print table above
//        printf("{%3d,%3d,%2d,%2d},", ns[state*4], ns[state*4+1],
//          ns[state*4+2], ns[state*4+3]);
//        if (state%4==3) printf(" // %d-%d\n  ", state-3, state);
        assert(state>=0 && state<256);
        assert(t[x][y][1]>0);
        assert(t[x][y][0]<=state);
        assert(t[x][y][0]+t[x][y][1]>state);
        assert(t[x][y][1]<=6);
        assert(t[x0][y0][1]>0);
        assert(t[x1][y1][1]>0);
        assert(ns0-t[x0][y0][0]<t[x0][y0][1]);
        assert(ns0-t[x0][y0][0]>=0);
        assert(ns1-t[x1][y1][0]<t[x1][y1][1]);
        assert(ns1-t[x1][y1][0]>=0);
        ++state;
      }
    }
  }
//  printf("%d states\n", state); exit(0);  // uncomment to print table above
}

#endif

///////////////////////////// Squash //////////////////////////////
// return p = 1/(1 + exp(-d)), d scaled by 8 bits, p scaled by 12 bits
class Squash {
  Array<U16> t;
public:
  Squash();
  int operator()(int p) const {
   if (p<-2047) return  0; 
   if (p>2047) return  4095;
   return t[p+2048];
  }
} squash;

Squash::Squash(): t(4096) {
int ts[33]={1,2,3,6,10,16,27,45,73,120,194,310,488,747,1101,
    1546,2047,2549,2994,3348,3607,3785,3901,3975,4022,
    4050,4068,4079,4085,4089,4092,4093,4094};
  int w,d;
  for (int i=-2047; i<=2047; ++i){
    w=i&127;
  d=(i>>7)+16;
  t[i+2048]=(ts[d]*(128-w)+ts[(d+1)]*w+64) >> 7;
    }
}
//////////////////////////// Stretch ///////////////////////////////

// Inverse of squash. d = ln(p/(1-p)), d scaled by 8 bits, p by 12 bits.
// d has range -2047 to 2047 representing -8 to 8.  p has range 0 to 4095.

class Stretch {
  Array<short> t;
public:
  Stretch();
  int operator()(int p) const {
    assert(p>=0 && p<4096);
    return t[p];
  }
} stretch;

Stretch::Stretch(): t(4096) {
  int pi=0;
  for (int x=-2047; x<=2047; ++x) {  // invert squash()
    int i=squash(x);
    for (int j=pi; j<=i; ++j)
      t[j]=x;
    pi=i+1;
  }
  t[4095]=2047;
}

//////////////////////////// Mixer /////////////////////////////

// Mixer m(N, M, S=1, w=0) combines models using M neural networks with
//   N inputs each, of which up to S may be selected.  If S > 1 then
//   the outputs of these neural networks are combined using another
//   neural network (with parameters S, 1, 1).  If S = 1 then the
//   output is direct.  The weights are initially w (+-32K).
//   It is used as follows:
// m.update() trains the network where the expected output is the
//   last bit (in the global variable y).
// m.add(stretch(p)) inputs prediction from one of N models.  The
//   prediction should be positive to predict a 1 bit, negative for 0,
//   nominally +-256 to +-2K.  The maximum allowed value is +-32K but
//   using such large values may cause overflow if N is large.
// m.set(cxt, range) selects cxt as one of 'range' neural networks to
//   use.  0 <= cxt < range.  Should be called up to S times such
//   that the total of the ranges is <= M.
// m.p() returns the output prediction that the next bit is 1 as a
//   12 bit number (0 to 4095).

#if !defined(__GNUC__)

#if (2 == _M_IX86_FP) // 2 if /arch:SSE2 was used.
# define __SSE2__
#elif (1 == _M_IX86_FP) // 1 if /arch:SSE was used.
# define __SSE__
#endif

#endif /* __GNUC__ */

#if defined(__AVX2__)
#include <smmintrin.h>
#define OPTIMIZE "AVX2-"
#elif defined(__SSE4_1__)  || defined(__SSSE3__)
#include<immintrin.h>

#elif defined(__SSE2__) 
#include <emmintrin.h>
#define OPTIMIZE "SSE2-"

#elif defined(__SSE__)
#include <xmmintrin.h>
#define OPTIMIZE "SSE-"
#endif
/**
 * Vector product a*b of n signed words, returning signed integer scaled down by 8 bits.
 * n is rounded up to a multiple of 8.
 */
//static int dot_product (const short* const t, const short* const w, int n);

/**
 * Train n neural network weights w[n] on inputs t[n] and err.
 * w[i] += ((t[i]*2*err)+(1<<16))>>17 bounded to +- 32K.
 * n is rounded up to a multiple of 8.
 */
//static void train (const short* const t, short* const w, int n, const int e);
struct ErrorInfo {
  U32 Data[2], Sum, Mask, Collected;
};

inline U32 SQR(U32 x) {
  return x*x;
}
typedef __m128i XMM;
#define DEFAULT_LEARNING_RATE 7
class Mixer {
private: 
  const int N, M, S;   // max inputs, max contexts, max context sets
  Array<short, 32> tx; // N inputs from add()  
  Array<short, 32> wx; // N*M weights
  Array<int> cxt;  // S contexts
  int ncxt;        // number of contexts (0 to S)
  int base;        // offset of next context
  int nx;          // Number of inputs in tx, 0 to N  
  Mixer* mp;       // points to a Mixer to combine results
  Array<int> pr;   // last result (scaled 12 bits)
  bool doText; 
  Array<ErrorInfo> info; 
  Array<int> rates; // learning rates
public:  
  BlockData& x;
  Mixer(int n, int m,BlockData& bd, int s=1, int w=0);
  
#if defined(__AVX2__)
//this probably is not working
 int dot_product (const short* const t, const short* const w, int n) {
  assert(n == ((n + 15) & -16));
  __m256i sum = _mm256_setzero_si256 ();
  while ((n -= 16) >= 0) { // Each loop sums 16 products
    __m256i tmp = _mm256_madd_epi16 (*(__m256i *) &t[n], *(__m256i *) &w[n]); // t[n] * w[n] + t[n+1] * w[n+1]
    tmp = _mm256_srai_epi32 (tmp, 8); //                                        (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
    sum = _mm256_add_epi32 (sum, tmp); //                                sum += (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
  } 
  //  adds sums  
  sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ()); 
  sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());
  sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());
 return _mm_cvtsi128_si32 (_mm256_extracti128_si256(sum,0)); //                   ...  and scale back to integer
}

 void train (const short* const t, short* const w, int n, const int e) {
  assert(n == ((n + 15) & -16));
  if (e) {
    const __m256i one = _mm256_set1_epi16 (1);
    const __m256i err = _mm256_set1_epi16 (short(e));
    while ((n -= 16) >= 0) { // Each iteration adjusts 16 weights
      __m256i tmp = _mm256_adds_epi16 (*(__m256i *) &t[n], *(__m256i *) &t[n]); // t[n] * 2
      tmp = _mm256_mulhi_epi16 (tmp, err); //                                     (t[n] * 2 * err) >> 16
      tmp = _mm256_adds_epi16 (tmp, one); //                                     ((t[n] * 2 * err) >> 16) + 1
      tmp = _mm256_srai_epi16 (tmp, 1); //                                      (((t[n] * 2 * err) >> 16) + 1) >> 1
      tmp = _mm256_adds_epi16 (tmp, *(__m256i *) &w[n]); //                    ((((t[n] * 2 * err) >> 16) + 1) >> 1) + w[n]
      *(__m256i *) &w[n] = tmp; //                                          save the new eight weights, bounded to +- 32K
    }
  }
}

#elif defined(__SSE2__) || defined(__SSSE3__)
 int dot_product (const short* const t, const short* const w, int n) {
  assert(n == ((n + 15) & -16));
  XMM sum = _mm_setzero_si128 ();
  while ((n -= 8) >= 0) { // Each loop sums eight products
    XMM tmp = _mm_madd_epi16 (*(XMM *) &t[n], *(XMM *) &w[n]); // t[n] * w[n] + t[n+1] * w[n+1]
    tmp = _mm_srai_epi32 (tmp, 8); //                                        (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
    sum = _mm_add_epi32 (sum, tmp); //                                sum += (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
  }
  #if  defined(__SSSE3__)
  sum =_mm_hadd_epi32(sum,_mm_setzero_si128 ()); //horizontal add 
  sum =_mm_hadd_epi32(sum,_mm_setzero_si128 ());
  #else
  sum = _mm_add_epi32 (sum, _mm_srli_si128 (sum, 8)); // Add eight sums together ...
  sum = _mm_add_epi32 (sum, _mm_srli_si128 (sum, 4));
  #endif
  return _mm_cvtsi128_si32 (sum); //                     ...  and scale back to integer
}

 void train (const short* const t, short* const w, int n, const int e) {
  assert(n == ((n + 15) & -16));
  if (e) {
    const XMM one = _mm_set1_epi16 (1);
    const XMM err = _mm_set1_epi16 (short(e));
    while ((n -= 8) >= 0) { // Each iteration adjusts eight weights
      XMM tmp = _mm_adds_epi16 (*(XMM *) &t[n], *(XMM *) &t[n]); // t[n] * 2
      tmp = _mm_mulhi_epi16 (tmp, err); //                                     (t[n] * 2 * err) >> 16
      tmp = _mm_adds_epi16 (tmp, one); //                                     ((t[n] * 2 * err) >> 16) + 1
      tmp = _mm_srai_epi16 (tmp, 1); //                                      (((t[n] * 2 * err) >> 16) + 1) >> 1
      tmp = _mm_adds_epi16 (tmp, *(XMM *) &w[n]); //                    ((((t[n] * 2 * err) >> 16) + 1) >> 1) + w[n]
      *(XMM *) &w[n] = tmp; //                                          save the new eight weights, bounded to +- 32K
    }
  }
}

#elif defined(__SSE__)
 int dot_product (const short* const t, const short* const w, int n) {
  assert(n == ((n + 15) & -16));
  __m64 sum = _mm_setzero_si64 ();
  while ((n -= 8) >= 0) { // Each loop sums eight products
    __m64 tmp = _mm_madd_pi16 (*(__m64 *) &t[n], *(__m64 *) &w[n]); //   t[n] * w[n] + t[n+1] * w[n+1]
    tmp = _mm_srai_pi32 (tmp, 8); //                                    (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
    sum = _mm_add_pi32 (sum, tmp); //                            sum += (t[n] * w[n] + t[n+1] * w[n+1]) >> 8

    tmp = _mm_madd_pi16 (*(__m64 *) &t[n + 4], *(__m64 *) &w[n + 4]); // t[n+4] * w[n+4] + t[n+5] * w[n+5]
    tmp = _mm_srai_pi32 (tmp, 8); //                                    (t[n+4] * w[n+4] + t[n+5] * w[n+5]) >> 8
    sum = _mm_add_pi32 (sum, tmp); //                            sum += (t[n+4] * w[n+4] + t[n+5] * w[n+5]) >> 8
  }
  sum = _mm_add_pi32 (sum, _mm_srli_si64 (sum, 32)); // Add eight sums together ...
  const int retval = _mm_cvtsi64_si32 (sum); //                     ...  and scale back to integer
  _mm_empty(); // Empty the multimedia state
  return retval;
}

 void train (const short* const t, short* const w, int n, const int e) {
  assert(n == ((n + 15) & -16));
  if (e) {
    const __m64 one = _mm_set1_pi16 (1);
    const __m64 err = _mm_set1_pi16 (short(e));
    while ((n -= 8) >= 0) { // Each iteration adjusts eight weights
      __m64 tmp = _mm_adds_pi16 (*(__m64 *) &t[n], *(__m64 *) &t[n]); //   t[n] * 2
      tmp = _mm_mulhi_pi16 (tmp, err); //                                 (t[n] * 2 * err) >> 16
      tmp = _mm_adds_pi16 (tmp, one); //                                 ((t[n] * 2 * err) >> 16) + 1
      tmp = _mm_srai_pi16 (tmp, 1); //                                  (((t[n] * 2 * err) >> 16) + 1) >> 1
      tmp = _mm_adds_pi16 (tmp, *(__m64 *) &w[n]); //                  ((((t[n] * 2 * err) >> 16) + 1) >> 1) + w[n]
      *(__m64 *) &w[n] = tmp; //                                       save the new four weights, bounded to +- 32K

      tmp = _mm_adds_pi16 (*(__m64 *) &t[n + 4], *(__m64 *) &t[n + 4]); // t[n+4] * 2
      tmp = _mm_mulhi_pi16 (tmp, err); //                                 (t[n+4] * 2 * err) >> 16
      tmp = _mm_adds_pi16 (tmp, one); //                                 ((t[n+4] * 2 * err) >> 16) + 1
      tmp = _mm_srai_pi16 (tmp, 1); //                                  (((t[n+4] * 2 * err) >> 16) + 1) >> 1
      tmp = _mm_adds_pi16 (tmp, *(__m64 *) &w[n + 4]); //              ((((t[n+4] * 2 * err) >> 16) + 1) >> 1) + w[n]
      *(__m64 *) &w[n + 4] = tmp; //                                   save the new four weights, bounded to +- 32K
    }
    _mm_empty(); // Empty the multimedia state
  }
}
#else

// dot_product returns dot product t*w of n elements.  n is rounded
// up to a multiple of 8.  Result is scaled down by 8 bits.
int dot_product(short *t, short *w, int n) {
  int sum=0;
  n=(n+15)&-16;
  for (int i=0; i<n; i+=2)
    sum+=(t[i]*w[i]+t[i+1]*w[i+1]) >> 8;
  return sum;
}

// Train neural network weights w[n] given inputs t[n] and err.
// w[i] += t[i]*err, i=0..n-1.  t, w, err are signed 16 bits (+- 32K).
// err is scaled 16 bits (representing +- 1/2).  w[i] is clamped to +- 32K
// and rounded.  n is rounded up to a multiple of 8.

void train(short *t, short *w, int n, int err) {
  n=(n+15)&-16;
  for (int i=0; i<n; ++i) {
    int wt=w[i]+(((t[i]*err*2>>16)+1)>>1);
    if (wt<-32768) wt=-32768;
    if (wt>32767) wt=32767;
    w[i]=wt;
  }
}
#endif 

  // Adjust weights to minimize coding cost of last prediction
  void update() {
    for (int i=0; i<ncxt; ++i) {
      int err=((x.y<<12)-pr[i])*7;
      assert(err>=-32768 && err<32768);
      train(&tx[0], &wx[cxt[i]*N], nx, err);
    }
    nx=base=ncxt=0;
  }
   void update1() {
    int target=x.y<<12;
    if(nx>0)
    for (int i=0; i<ncxt; ++i) {
      int err=target-pr[i];
      train(&tx[0], &wx[cxt[i]*N], nx, err*rates[i]);

        U32 logErr=min(0xF,ilog2(abs(err)));
        info[i].Sum-=SQR(info[i].Data[1]>>28);
        info[i].Data[1]<<=4; info[i].Data[1]|=info[i].Data[0]>>28;
        info[i].Data[0]<<=4; info[i].Data[0]|=logErr;
        info[i].Sum+=SQR(logErr);
        info[i].Collected+=info[i].Collected<4096;
        info[i].Mask<<=1; info[i].Mask|=(logErr<=((info[i].Data[0]>>4)&0xF));
        U32 count=BitCount(info[i].Mask);
        if (info[i].Collected>=64 && (info[i].Sum>1500+U32(rates[i])*64 || count<9 || (info[i].Mask&0xFF)==0)){
          rates[i]=DEFAULT_LEARNING_RATE;
          memset(&info[i], 0, sizeof(ErrorInfo));
        }
        else if (info[i].Collected==4096 && info[i].Sum>=56 && info[i].Sum<=144 && count>28-U32(rates[i]) && ((info[i].Mask&0xFF)==0xFF)){
          rates[i]-=rates[i]>2;
          memset(&info[i], 0, sizeof(ErrorInfo));
        }
    }
    reset();
  }
    void reset() {
    nx=base=ncxt=0;
  }
  void update2() {
      if (x.filetype==EXE || x.filetype==IMAGE24 || x.filetype==DECA )update1();
    else     if(doText==true) train(&tx[0], &wx[0], nx, ((x.y<<12)-base)*3/2), reset();
    else             update();
  }
  // Input x (call up to N times)
  void add(int x) {
    assert(nx<N);
    tx[nx++]=x;
  }
  void add32(U32 a){
     ((U32 *) &tx[nx],a);
    nx=nx+2;
  }
  void add64(U64 a){
     ((U64 *) &tx[nx],a);
    nx=nx+4;
  }
  void addXMM(XMM a){
    _mm_storeu_si128 ((XMM *) &tx[nx],a);
    nx=nx+8;
  }
  // Set a context (call S times, sum of ranges <= M)
  void set(int cx, int range) {
    assert(range>=0);
    assert(ncxt<S);
    assert(cx>=0);
    assert(cx<range);
    assert(base+cx<M);
    cxt[ncxt++]=base+cx;
    base+=range;
  }
  // predict next bit
  int p(const int shift0=0, const int shift1=0) {
    while (nx&15) tx[nx++]=0;  // pad
    if (mp) {  // combine outputs
      mp->update2();
      for (int i=0; i<ncxt; ++i) {
          int dp=((dot_product(&tx[0], &wx[cxt[i]*N], nx)));//*7)>>8);
          if(doText==true) dp=(dp*9)>>9;  
          else             dp=dp>>(5+shift0);
          pr[i]=squash(dp);
          mp->add(dp);
      }
     if(doText==false) mp->set(0, 1);
      return mp->p(shift0, shift1);
    }
    else {  // S=1 context
    if(doText==false)  
    return pr[0]=squash(dot_product(&tx[0], &wx[0], nx)>>(8+shift1));
      int z=dot_product(&tx[0], &wx[0], nx);
    base=squash( (z*16) >>13);
    return squash(z>>9);
    }
  }
  // do prediction for text mode
  void setText(bool t){
      doText=t;
      if (mp) mp->setText(t);
  }
  ~Mixer();
};

Mixer::~Mixer() {
  delete mp;
}

Mixer::Mixer(int n, int m, BlockData& bd, int s, int w):
    N((n+15)&-16), M(m), S(s), wx(N*M),
    cxt(S), ncxt(0), base(0), pr(S), mp(0),tx(N),nx(0),x(bd),doText(false), info(S), rates(S) {
  assert(n>0 && N>0 && (N&15)==0 && M>0);
   int i;
  for (i=0; i<S; ++i){
    pr[i]=2048; //initial p=0.5
    rates[i] = DEFAULT_LEARNING_RATE;
    memset(&info[i], 0, sizeof(ErrorInfo));
  }

  for (i=0; i<N*M; ++i)
    wx[i]=w;
  if (S>1) mp=new Mixer(S, 1,x ,1);
}


//////////////////////////// APM1 //////////////////////////////

// APM1 maps a probability and a context into a new probability
// that bit y will next be 1.  After each guess it updates
// its state to improve future guesses.  Methods:
//
// APM1 a(N) creates with N contexts, uses 66*N bytes memory.
// a.p(pr, cx, rate=7) returned adjusted probability in context cx (0 to
//   N-1).  rate determines the learning rate (smaller = faster, default 7).
//   Probabilities are scaled 12 bits (0-4095).

class APM1 {
  int index;     // last p, context
  const int N;   // number of contexts
  Array<U16> t;  // [N][33]:  p, context -> p
  BlockData& x;
public:
  APM1(int n,BlockData& x);
  int p(int pr=2048, int cxt=0, int rate=7) {
    assert(pr>=0 && pr<4096 && cxt>=0 && cxt<N && rate>0 && rate<32);
    pr=stretch(pr);
    int g=(x.y<<16)+(x.y<<rate)-x.y-x.y;
    t[index] += (g-t[index]) >> rate;
    t[index+1] += (g-t[index+1]) >> rate;
    const int w=pr&127;  // interpolation weight (33 points)
    index=((pr+2048)>>7)+cxt*33;
    return (t[index]*(128-w)+t[index+1]*w) >> 11;
  }
};

// maps p, cxt -> p initially
APM1::APM1(int n,BlockData& bd): index(0), N(n), t(n*33),x(bd) {
  for (int i=0; i<N; ++i)
    for (int j=0; j<33; ++j)
      t[i*33+j] = i==0 ? squash((j-16)*128)*16 : t[j];
}

//////////////////////////// StateMap, APM //////////////////////////

// A StateMap maps a context to a probability.  Methods:
//
// Statemap sm(n) creates a StateMap with n contexts using 4*n bytes memory.
// sm.p(y, cx, limit) converts state cx (0..n-1) to a probability (0..4095).
//     that the next y=1, updating the previous prediction with y (0..1).
//     limit (1..1023, default 1023) is the maximum count for computing a
//     prediction.  Larger values are better for stationary sources.

static int dt[1024];  // i -> 16K/(i+i+3)

class StateMap {
protected:
  const int N;  // Number of contexts
  int cxt;      // Context of last prediction
  Array<U32> t;       // cxt -> prediction in high 22 bits, count in low 10 bits
  inline void update(int y, int limit) {
    assert(cxt>=0 && cxt<N);
    assert(y==0 || y==1);
    U32 *p=&t[cxt], p0=p[0];
    int n=p0&1023, pr=p0>>10;  // count, prediction
    //if (n<limit) ++p0;
    //else p0=(p0&0xfffffc00)|limit;
    p0+=(n<limit);
    p0+=(((y<<22)-pr)>>3)*dt[n]&0xfffffc00;
    p[0]=p0;
  }

public:
  StateMap(int n=256);
  void Reset(int Rate=0){
    for (int i=0; i<N; ++i)
      t[i]=(t[i]&0xfffffc00)|min(Rate, t[i]&0x3FF);
  }
  // update bit y (0..1), predict next bit in context cx
  int p(int cx, int y,int limit=1023) {
    assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    assert(y==0 || y==1);
    update(y,limit);
    return t[cxt=cx]>>20;
  }
};

StateMap::StateMap(int n): N(n), cxt(0), t(n) {
  for (int i=0; i<N; ++i)
    t[i]=1<<31;
}

// An APM maps a probability and a context to a new probability.  Methods:
//
// APM a(n) creates with n contexts using 4*STEPS*n bytes memory.
// a.pp(y, pr, cx, limit) updates and returns a new probability (0..4095)
//     like with StateMap.  pr (0..4095) is considered part of the context.
//     The output is computed by interpolating pr into STEPS ranges nonlinearly
//     with smaller ranges near the ends.  The initial output is pr.
//     y=(0..1) is the last bit.  cx=(0..n-1) is the other context.
//     limit=(0..1023) defaults to 255.

template <int steps> class APM : public StateMap {
  static_assert(steps>4, "APM: number of steps must be a positive integer bigger than 4");
public:
  APM(int n) : StateMap(n*steps) {
    for (int i=0; i<N; ++i) {
      int p = ((i%steps*2+1)*4096)/(steps*2)-2048;
      t[i] = (U32(squash(p))<<20)+6; //initial count: 6
    }
  }
  int p(int pr, int cx, int y,const int limit=0xFF) {
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/steps);
    assert(limit>0 && limit<1024);
    //adapt (update prediction from previous pass)
    update(y,limit);
    //predict
    pr = (stretch(pr)+2048)*(steps-1);
    int wt = pr&0xfff;  // interpolation weight (0..4095)
    cx = cx*steps+(pr>>12);
    assert(cx>=0 && cx<N-1);
    cxt = cx+(wt>>11);
    pr = ((t[cx]>>13)*(4096-wt)+(t[cx+1]>>13)*wt)>>19;
    return pr;
  }
};


//////////////////////////// hash //////////////////////////////

// Hash 2-5 ints.
inline U32 hash(U32 a, U32 b, U32 c=0xffffffff, U32 d=0xffffffff,
    U32 e=0xffffffff) {
  U32 h=a*200002979u+b*30005491u+c*50004239u+d*70004807u+e*110002499u;
  return h^h>>9^a>>2^b>>3^c>>4^d>>5^e>>6;
}

///////////////////////////// BH ////////////////////////////////

// A BH maps a 32 bit hash to an array of B bytes (checksum and B-2 values)
//
// BH bh(N); creates N element table with B bytes each.
//   N must be a power of 2.  The first byte of each element is
//   reserved for a checksum to detect collisions.  The remaining
//   B-1 bytes are values, prioritized by the first value.  This
//   byte is 0 to mark an unused element.
//
// bh[i] returns a pointer to the i'th element, such that
//   bh[i][0] is a checksum of i, bh[i][1] is the priority, and
//   bh[i][2..B-1] are other values (0-255).
//   The low lg(n) bits as an index into the table.
//   If a collision is detected, up to M nearby locations in the same
//   cache line are tested and the first matching checksum or
//   empty element is returned.
//   If no match or empty element is found, then the lowest priority
//   element is replaced.

// 2 byte checksum with LRU replacement (except last 2 by priority)
template <int B> class BH {
  enum {M=8};  // search limit
  Array<U8, 64> t; // elements
  //Array<U8> tmp;
  U8 tmp[B];
  U32 n; // size-1
public:
  BH(int i): t(i*B), n(i-1) {
    //printf("BH %0.0f, i %d B %d power %d\n",(i*B)+0.0,i,B,(i&(i-1))==0);
    assert(B>=2 && i>0 && (i&(i-1))==0); // size a power of 2?
    
  }
  U8* operator[](U32 i);
};

template <int B>
inline  U8* BH<B>::operator[](U32 i) {
  U16 chk=(i>>16^i)&0xffff;
  i=i*M&n;
  U8 *p;
  U16 *cp;
  int j;
  for (j=0; j<M; ++j) {
    p=&t[(i+j)*B];
    cp=(U16*)p;
    if (p[2]==0) {*cp=chk;break;}
    if (*cp==chk) break;  // found
  }
  if (j==0) return p+1;  // front
  //static U8 tmp[B];  // element to move to front
  if (j==M) {
    --j;
    memset(&tmp, 0, B);
    memmove(&tmp, &chk, 2);
    if (M>2 && t[(i+j)*B+2]>t[(i+j-1)*B+2]) --j;
  }
  else memcpy(&tmp, cp, B);
  memmove(&t[(i+1)*B], &t[i*B], j*B);
  memcpy(&t[i*B], &tmp, B);
  return &t[i*B+1];
}
 
/////////////////////////// ContextMap /////////////////////////
//
// A ContextMap maps contexts to a bit histories and makes predictions
// to a Mixer.  Methods common to all classes:
//
// ContextMap cm(M, C); creates using about M bytes of memory (a power
//   of 2) for C contexts.
// cm.set(cx);  sets the next context to cx, called up to C times
//   cx is an arbitrary 32 bit value that identifies the context.
//   It should be called before predicting the first bit of each byte.
// cm.mix(m) updates Mixer m with the next prediction.  Returns 1
//   if context cx is found, else 0.  Then it extends all the contexts with
//   global bit y.  It should be called for every bit:
//
//     if (bpos==0)
//       for (int i=0; i<C; ++i) cm.set(cxt[i]);
//     cm.mix(m);
//
// The different types are as follows:
//
// - RunContextMap.  The bit history is a count of 0-255 consecutive
//     zeros or ones.  Uses 4 bytes per whole byte context.  C=1.
//     The context should be a hash.
// - SmallStationaryContextMap.  0 <= cx < M/512.
//     The state is a 16-bit probability that is adjusted after each
//     prediction.  C=1.
// - ContextMap.  For large contexts, C >= 1.  Context need not be hashed.



// A RunContextMap maps a context into the next byte and a repeat
// count up to M.  Size should be a power of 2.  Memory usage is 3M/4.
class RunContextMap {
  BH<4> t;
  U8* cp;
  BlockData& x;
public:
  RunContextMap(int m,BlockData& bd): t(m/4),x(bd) {cp=t[0]+1;}
  void set(U32 cx) {  // update count
    if (cp[0]==0 || cp[1]!=x.buf(1)) cp[0]=1, cp[1]=x.buf(1);
    else if (cp[0]<255) ++cp[0];
    cp=t[cx]+1;
  }
  int p() {  // predict next bit
    if ((cp[1]+256)>>(8-x.bpos)==x.c0)
      return ((cp[1]>>(7-x.bpos)&1)*2-1)*ilog(cp[0]+1)*8;
    else
      return 0;
  }
  int mix(Mixer& m) {  // return run length
    m.add(p());
    return cp[0]!=0;
  }
};

 

/*
Map for modelling contexts of (nearly-)stationary data.
The context is looked up directly. For each bit modelled, a 16bit prediction is stored.
The adaptation rate is controlled by the caller, see mix().

- BitsOfContext: How many bits to use for each context. Higher bits are discarded.
- BitsPerContext: How many bits [1..8] of input are to be modelled for each context.
New contexts must be set at those intervals.

Uses 2^(BitsOfContext+BitsPerContext+1) bytes of memory.
*/
class SmallStationaryContextMap {
  Array<U16> Data;
  int Context, Mask, bCount, B;
  U16 *cp;
public:
  SmallStationaryContextMap(int BitsOfContext, int BitsPerContext = 8) : Data(1ull<<(BitsOfContext+BitsPerContext)), Context(0), Mask(1<<BitsPerContext), bCount(1), B(1) {
    assert(BitsOfContext<=16);
    assert(BitsPerContext && BitsPerContext<=8);
    Reset();
    cp=&Data[0];
  }
  void set(U32 ctx) {
    Context = (ctx*Mask)&(Data.size() - Mask);
    bCount=B=1;
  }
  void Reset() {
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=0x7FFF;
  }
  void mix(Mixer& m, const int rate = 7, const int Multiplier = 1, const int Divisor = 4) {
    *cp+=((m.x.y<<16)-(*cp)+(1<<(rate-1)))>>rate;
    B+=(m.x.y && B>1);
    cp = &Data[Context+B];
    int Prediction = (*cp)>>4;
    m.add((stretch(Prediction)*Multiplier)/Divisor);
    m.add(((Prediction-2048)*Multiplier)/(Divisor*2));
    bCount<<=1; B<<=1;
    if (bCount==Mask) {
      bCount=1;
      B=1;
    }
  }
};
 
/*
  Map for modelling contexts of (nearly-)stationary data.
  The context is looked up directly. For each bit modelled, a 32bit element stores
  a 22 bit prediction and a 10 bit adaptation rate offset.

  - BitsOfContext: How many bits to use for each context. Higher bits are discarded.
  - BitsPerContext: How many bits [1..8] of input are to be modelled for each context.
    New contexts must be set at those intervals.
  - Rate: Initial adaptation rate offset [0..1023]. Lower offsets mean faster adaptation.
    Will be increased on every occurrence until the higher bound is reached.

  Uses 2^(BitsOfContext+BitsPerContext+2) bytes of memory.
*/

class StationaryMap {
  Array<U32> Data;
  int Context, Mask, bCount, B;
  U32 *cp;
public:
  StationaryMap(int BitsOfContext, int BitsPerContext = 8, int Rate = 0): Data(1<<(BitsOfContext+BitsPerContext)), Context(0), Mask(1<<BitsPerContext), bCount(1), B(1) {
    assert(BitsOfContext<=16);
    assert(BitsPerContext && BitsPerContext<=8);
    Reset(Rate);
    cp=&Data[0];
  }
  void set(U32 ctx) {
    Context = (ctx*Mask)&(Data.size()-Mask);
    bCount=B=1;
  }
  void Reset( int Rate = 0 ){
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=(0x7FF<<20)|min(0x3FF,Rate);
  }
    void mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 0x3FF) {
    U32 Count = min(min(Limit,0x3FF), ((*cp)&0x3FF)+1);
    int Prediction = (*cp)>>10, Error = (m.x.y<<22)-Prediction;
    Error = ((Error/8)*dt[Count])/1024;
    Prediction = min(0x3FFFFF,max(0,Prediction+Error));
    *cp = (Prediction<<10)|Count;
    B+=(m.x.y && B>1);
    cp=&Data[Context+B];
    Prediction = (*cp)>>20;
    m.add((stretch(Prediction)*Multiplier)/Divisor);
    m.add(((Prediction-2048)*Multiplier)/(Divisor*2));
    bCount<<=1; B<<=1;
    if (bCount==Mask){
      bCount=1;
      B=1;
    }
  }
};

// Context map for large contexts.  Most modeling uses this type of context
// map.  It includes a built in RunContextMap to predict the last byte seen
// in the same context, and also bit-level contexts that map to a bit
// history state.
//
// Bit histories are stored in a hash table.  The table is organized into
// 64-byte buckets alinged on cache page boundaries.  Each bucket contains
// a hash chain of 7 elements, plus a 2 element queue (packed into 1 byte)
// of the last 2 elements accessed for LRU replacement.  Each element has
// a 2 byte checksum for detecting collisions, and an array of 7 bit history
// states indexed by the last 0 to 2 bits of context.  The buckets are indexed
// by a context ending after 0, 2, or 5 bits of the current byte.  Thus, each
// byte modeled results in 3 main memory accesses per context, with all other
// accesses to cache.
//
// On bits 0, 2 and 5, the context is updated and a new bucket is selected.
// The most recently accessed element is tried first, by comparing the
// 16 bit checksum, then the 7 elements are searched linearly.  If no match
// is found, then the element with the lowest priority among the 5 elements
// not in the LRU queue is replaced.  After a replacement, the queue is
// emptied (so that consecutive misses favor a LFU replacement policy).
// In all cases, the found/replaced element is put in the front of the queue.
//
// The priority is the state number of the first element (the one with 0
// additional bits of context).  The states are sorted by increasing n0+n1
// (number of bits seen), implementing a LFU replacement policy.
//
// When the context ends on a byte boundary (bit 0), only 3 of the 7 bit
// history states are used.  The remaining 4 bytes implement a run model
// as follows: <count:7,d:1> <b1> <unused> <unused> where <b1> is the last byte
// seen, possibly repeated.  <count:7,d:1> is a 7 bit count and a 1 bit
// flag (represented by count * 2 + d).  If d=0 then <count> = 1..127 is the
// number of repeats of <b1> and no other bytes have been seen.  If d is 1 then
// other byte values have been seen in this context prior to the last <count>
// copies of <b1>.
//
// As an optimization, the last two hash elements of each byte (representing
// contexts with 2-7 bits) are not updated until a context is seen for
// a second time.  This is indicated by <count,d> = <1,0> (2).  After update,
// <count,d> is updated to <2,0> or <1,1> (4 or 3).

inline U64 CMlimit(U64 size){
    if (size>0x80000000UL) return 0x80000000UL; //limit to 4GB
    return (size);
}
class ContextMap {
  const int C;  // max number of contexts
  class E {  // hash element, 64 bytes
    U16 chk[7];  // byte context checksums
    U8 last;     // last 2 accesses (0-6) in low, high nibble
  public:
    U8 bh[7][7]; // byte context, 3-bit context -> bit history state
      // bh[][0] = 1st bit, bh[][1,2] = 2nd bit, bh[][3..6] = 3rd bit
      // bh[][0] is also a replacement priority, 0 = empty
    U8* get(U16 chk);  // Find element (0-6) matching checksum.
      // If not found, insert or replace lowest priority (not last).
  };
  Array<E, 64> t;  // bit histories for bits 0-1, 2-4, 5-7
    // For 0-1, also contains a run count in bh[][4] and value in bh[][5]
    // and pending update count in bh[7]
  Array<U8*> cp;   // C pointers to current bit history
  Array<U8*> cp0;  // First element of 7 element array containing cp[i]
  Array<U32> cxt;  // C whole byte contexts (hashes)
  Array<U8*> runp; // C [0..3] = count, value, unused, unused
  Array<short, 16>  r0;   //for rle 
  Array<short, 16>  r1;
  Array<short, 16>  r0i;
  Array<short, 16>  rmask; // mask for skiped context
  StateMap *sm;    // C maps of state -> p
  int cn;          // Next context to set by set()
  //void update(U32 cx, int c);  // train model that context cx predicts c
  Random rnd;
  int mix1(Mixer& m, int cc, int bp, int c1, int y1);
    // mix() with global context passed as arguments to improve speed.
    
public:
  ContextMap(U64 m, int c=1);  // m = memory in bytes, a power of 2, C = c
  ~ContextMap();
  void set(U32 cx, int next=-1);   // set next whole byte context to cx
    // if next is 0 then set order does not matter
  int mix(Mixer& m) {return mix1(m, m.x.c0, m.x.bpos, m.x.buf(1), m.x.y);}
    int inputs();
};

#if defined(SIMD_GET_SSE) 

#define xmmbshl(x,n)  _mm_slli_si128(x,n) // xm <<= 8*n  -- BYTE shift left
#define xmmbshr(x,n)  _mm_srli_si128(x,n) // xm >>= 8*n  -- BYTE shift right
#define xmmshl64(x,n) _mm_slli_epi64(x,n) // xm.hi <<= n, xm.lo <<= n
#define xmmshr64(x,n) _mm_srli_epi64(x,n) // xm.hi >>= n, xm.lo >>= n
#define xmmand(a,b)   _mm_and_si128(a,b)
#define xmmor(a,b)    _mm_or_si128(a,b)
#define xmmxor(a,b)   _mm_xor_si128(a,b)
#define xmmzero       _mm_setzero_si128()
#ifdef _MSC_VER
#include <intrin.h>
U32 __inline clz(U32 value){ //asume newer version of vc
    unsigned long leading_zero = 0;
    if (_BitScanReverse( &leading_zero, value)) return 31-leading_zero;
    else return 32;
}
  U32 __inline ctz(U32 x ){
    unsigned long leading_zero = 0;
   if (_BitScanForward(&leading_zero, x )) {
       return  leading_zero;
    }
    else{
         return 0; // Same remarks as above
    }
}

#elif ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))
U32 __inline clz(U32 value){ 
    return __builtin_clz(value);
}
U32 __inline ctz(U32 value){ 
    return __builtin_ctz(value);
}
#endif
#endif

// Find or create hash element matching checksum ch
inline U8* ContextMap::E::get(U16 ch) {
    
  if (chk[last&15]==ch) return &bh[last&15][0];
  int b=0xffff, bi=0;
#if defined(SIMD_GET_SSE)   
  const XMM xmmch=_mm_set1_epi16 (short(ch)); //fill 8 ch values
//load 8 values, discard last one as only 7 are needed.
//reverse order and compare 7 chk values to ch
//get mask is set get first index and return value  
  XMM tmp=_mm_load_si128 ((XMM *) &chk[0]); //load 8 values (8th will be discarded)
#if defined(__SSE2__) 
  tmp=_mm_shufflelo_epi16(tmp,0x1B); //swap order for mask  (0,1,2,3)
  tmp=_mm_shufflehi_epi16(tmp,0x1B);                      //(0,1,2,3)
  tmp=_mm_shuffle_epi32(tmp,0x4E);                        //(1,0,3,2)   
#elif defined(__SSSE3__)
#include <immintrin.h>
  const XMM vm=_mm_setr_epi8(14,15,12,13,10,11,8,9,6,7,4,5,2,3,0,1);// initialise vector mask 
  tmp=_mm_shuffle_epi8(tmp,vm);        
#endif
  tmp=_mm_cmpeq_epi16 (tmp,xmmch); //compare ch values
  tmp=_mm_packs_epi16(tmp,xmmzero); //pack result
  U32 t=(_mm_movemask_epi8(tmp))>>1; //get mask of comparsion, bit is set if eq, discard 8th bit
  U32 a;    //index into bh or 7 if not found
  if(t){
      a=(clz(t)-1)&7;
      return last=last<<4|a, (U8*)&bh[a][0];
  }

 XMM   lastl=_mm_set1_epi8((last&15));
 XMM   lasth=_mm_set1_epi8((last>>4));
 XMM   one1  =_mm_set1_epi8(1);
 XMM   vm=_mm_setr_epi8(0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);

 XMM   lastx=_mm_unpacklo_epi64(lastl,lasth); //last&15 last>>4
 XMM   eq0  =_mm_cmpeq_epi8 (lastx,vm); //compare   values

 eq0=_mm_or_si128(eq0,_mm_srli_si128 (eq0, 8));    //or low values with high

 lastx = _mm_and_si128(one1, eq0);                //set to 1 if eq
 XMM sum1 = _mm_sad_epu8(lastx,xmmzero);        //cout values, abs(a0 - b0) + abs(a1 - b1) .... up to b8
 const U32 pcount=_mm_cvtsi128_si32(sum1); //population count
/*for (int i=0; i<7; ++i) {
   bh[i][0]=i+1;
    
  }*/
 U32 t0=(~_mm_movemask_epi8(eq0));
for (int i=pcount; i<7; ++i) {
    int bitt =ctz(t0);     //get index 
//#if ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))    
//asm("btr %1,%0" : "+r"(t0) : "r"(bitt)); // clear bit set and test again https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47769
//#else
    t0 &= ~(1 << bitt); // clear bit set and test again
//#endif 
   int pri=bh[bitt][0];
    if (pri<b  ) b=pri, bi=bitt;


  }  
  /*
  //uncomment above SIMD version and comment out code below to use full SIMD (SSE2) version
  for (int i=0; i<7; ++i) {
    int pri=bh[i][0];
    if (pri<b && (last&15)!=i && (last>>4)!=i) b=pri, bi=i;
  }*/
  return last=0xf0|bi, chk[bi]=ch, (U8*)memset(&bh[bi][0], 0, 7);
#else
  for (int i=0; i<7; ++i) {
    if (chk[i]==ch) return last=last<<4|i, (U8*)&bh[i][0];
    int pri=bh[i][0];
    if (pri<b && (last&15)!=i && last>>4!=i) b=pri, bi=i;
  }
  return last=0xf0|bi, chk[bi]=ch, (U8*)memset(&bh[bi][0], 0, 7);
#endif
}

// Construct using m bytes of memory for c contexts(c+7)&-8
ContextMap::ContextMap(U64 m, int c): C(c),  t(m>>6), cp(C), cp0(C),
    cxt(C), runp(C), r0(C),r1(C),r0i(C),rmask(C),cn(0) {
  assert(m>=64 && (m&m-1)==0);  // power of 2?
  assert(sizeof(E)==64);
  sm=new StateMap[C];
  for (int i=0; i<C; ++i) {
    cp0[i]=cp[i]=&t[0].bh[0][0];
    runp[i]=cp[i]+3;
  }
  #ifndef NDEBUG 
  printf("ContextMap t %0.2f mbytes\n",((m+0.0)/1024)/1024);
  #endif
}

ContextMap::~ContextMap() {
  delete[] sm;
}

// Set the i'th context to cx
inline void ContextMap::set(U32 cx, int next) {
  int i=cn++;
  i&=next;
  assert(i>=0 && i<C);
  if (cx==0){ cxt[i]=0; rmask[i]=0;}
  else{
  cx=cx*987654323+i;  // permute (don't hash) cx to spread the distribution
  cx=cx<<16|cx>>16;
  cxt[i]=cx*123456791+i;
  rmask[i]=-1;
  }
}
// Predict to mixer m from bit history state s, using sm to map s to
// a probability.

inline int mix2(Mixer& m, int s, StateMap& sm) {
  int p1=sm.p(s,m.x.y);
  int n0=-!nex(s,2);
  int n1=-!nex(s,3);
  int st=stretch(p1)>>2;
  m.add(st);
  p1>>=4;
  int p0=255-p1;
  if (m.x.rm1)  m.add(p1-p0); else m.add(0);
  m.add(st*abs(n1-n0));
  m.add((p1&n0)-(p0&n1));
  m.add((p1&n1)-(p0&n0));
  return s>0;
}


// Update the model with bit y1, and predict next bit to mixer m.
// Context: cc=c0, bp=bpos, c1=buf(1), y1=y.
int ContextMap::mix1(Mixer& m, int cc, int bp, int c1, int y1) {
  // Update model with y
  int result=0;

  for (int i=0; i<cn; ++i) {
   if(cxt[i]){
    if (cp[i]) {
      assert(cp[i]>=&t[0].bh[0][0] && cp[i]<=&t[t.size()-1].bh[6][6]);
      assert(((long long)(cp[i])&63)>=15);
      int ns=nex(*cp[i], y1);
      if (ns>=204 && rnd() << ((452-ns)>>3)) ns-=4;  // probabilistic increment
      *cp[i]=ns;
    }

    // Update context pointers
    if (m.x.bpos>1 && runp[i][0]==0)
    {
     cp[i]=0;
    }
    else
    {
     U16 chksum=cxt[i]>>16;
     U64 tmask=t.size()-1;
     switch(m.x.bpos)
     {
      case 1: case 3: case 6: cp[i]=cp0[i]+1+y1/*(cc&1)*/; break;
      case 4: case 7: cp[i]=cp0[i]+3+(cc&3); break;
      case 2: case 5: cp0[i]=cp[i]=t[(cxt[i]+cc)&tmask].get(chksum); break;
      default:
      {
       cp0[i]=cp[i]=t[(cxt[i]+cc)&tmask].get(chksum);
       // Update pending bit histories for bits 2-7
       if (cp0[i][3]==2) {
         const int c=cp0[i][4]+256;
         U8 *p=t[(cxt[i]+(c>>6))&tmask].get(chksum);
         p[0]=1+((c>>5)&1);
         p[1+((c>>5)&1)]=1+((c>>4)&1);
         p[3+((c>>4)&3)]=1+((c>>3)&1);
         p=t[(cxt[i]+(c>>3))&tmask].get(chksum);
         p[0]=1+((c>>2)&1);
         p[1+((c>>2)&1)]=1+((c>>1)&1);
         p[3+((c>>1)&3)]=1+(c&1);
         cp0[i][6]=0;
       }
       // Update run count of previous context
       if (runp[i][0]==0)  // new context
         runp[i][0]=2, runp[i][1]=c1;
       else if (runp[i][1]!=c1)  // different byte in context
         runp[i][0]=1, runp[i][1]=c1;
       else if (runp[i][0]<254)  // same byte in context
         runp[i][0]+=2;
       else if (runp[i][0]==255)
         runp[i][0]=128;
       runp[i]=cp0[i]+3;
      } break;
     }
    }
    // predict from bit context
    int s = 0;
    if (cp[i]) s = *cp[i];
    if (s>0) result++;
    mix2(m, s, sm[i]);


  }else{
    //for (int i=0; i<(inputs()-1); i++)
     m.add64(0);
   //  m.add32(0);
        m.add(0);     
  }
  }
    // predict from last byte in context
     
     for (int i=0; i<cn; ++i) {
         U8 a=runp[i][0];
         U8 b=runp[i][1];
         r0[i]=a;
         r1[i]=b;
         r0i[i]=ilog(a+1);
     }

#if defined(SIMD_CM_R ) && defined(__SSE2__) 
    int cnc=(cn/8)*8;
    XMM xcc=_mm_set1_epi16(cc);
    XMM  x0=_mm_setzero_si128();
    XMM  x1=_mm_set1_epi16(1);
    XMM  lasth=_mm_set1_epi16(256);
    const int bsh=(8-bp);
    const int bsh1=(7-bp);
    for (int i=0; i<(cnc); i=i+8) {
        XMM   b=_mm_load_si128 ((XMM  *) &r1[i]);
        //(r1[i  ]+256)>>(8-bp)==cc
        XMM  xr1=_mm_add_epi16 (b, lasth);
        xr1=_mm_srli_epi16 (xr1, bsh);
        xr1=_mm_cmpeq_epi16(xr1,xcc); //(a == b) ? 0xffff : 0x0
        //b                           //((r1[i  ]>>(7-bp)&1)*2-1) 
        XMM xb=_mm_srli_epi16 (b, bsh1); //>>(7-bp)
        xb=_mm_and_si128(xb,x1);         //&1
        xb=_mm_slli_epi16(xb, 1);        //<<1
        xb= _mm_sub_epi16(xb,x1);        //-1
        //c                                                       //((r0i[i  ])<<(2+(~r0[i  ]&1)))
        XMM xr0i=_mm_add_epi16 (*(XMM  *) &r0i[i], x0);           //r0i[i]
        XMM  c=_mm_add_epi16 (*(XMM  *) &r0[i], x0);              //~r0[i]&1
        XMM xc=_mm_andnot_si128 (c,x1);  
        XMM r0ia= _mm_add_epi16 (x1,xc);                          //1+(~r0[i]&1) result is 2 or 1 for multiplay |
        xc=_mm_slli_epi16(xr0i, 2);                               //r0i[i]<<2                                  | 
        xc=_mm_mullo_epi16(xc,r0ia);                              //(r0i[i]<<2*)  ~r0[i]&1?1+(~r0[i]&1):1      <-
        //b*c                                                     // (r0i[i  ])<<(2+(~r0[i  ]&1)))
        XMM xr=_mm_mullo_epi16(xc,xb); 
        XMM xresult=_mm_and_si128(xr,xr1);   //(r1[i  ]+256)>>(8-bp)==cc?xr:0
        XMM   runm=_mm_load_si128 ((XMM  *) &rmask[i]);
        xresult=_mm_and_si128(xresult,runm); //mask out skiped context
        //store result
        m.addXMM(xresult);
    }
    //do remaining 
    for (int i=cnc; i<cn; ++i) {
        if (rmask[i] &&( (r1[i  ]+256)>>(8-bp)==cc)) {
            m.add(((r1[i  ]>>(7-bp)&1)*2-1) *((r0i[i  ])<<(2+(~r0[i  ]&1)))); }
        else   m.add(0);
    }
#else          
    for (int i=0; i<cn; ++i) {
        if (rmask[i] && ((r1[i  ]+256)>>(8-bp)==cc)) {
            m.add(((r1[i  ]>>(7-bp)&1)*2-1) *((r0i[i  ])<<(2+(~r0[i  ]&1)))); }
        else   m.add(0);
      }
#endif    
   
  if (bp==7) cn=0;
  return result;
}
int ContextMap::inputs() {
    return 6;
}

//////////////////////////// Text modelling /////////////////////////

inline bool CharInArray(const char c, const char a[], const int len) {
  if (*a==0)
    return false;
  int i=0;
  for (; i<len && c!=a[i]; i++);
  return i<len;
}

#define MAX_WORD_SIZE 64
#define TAB 0x09
#define NEW_LINE 0x0A
#define CARRIAGE_RETURN 0x0D
#define SPACE 0x20

class Word {
public:
  U8 Letters[MAX_WORD_SIZE];
  U8 Start, End;
  U32 Hash[4], Type, Language;
  Word() : Start(0), End(0),  Type(0), Language(0) {
    memset(&Letters[0], 0, sizeof(U8)*MAX_WORD_SIZE);
    memset(&Hash[0], 0, sizeof(U32)*4);
  }
  bool operator==(const char *s) const {
    size_t len=strlen(s);
    return ((size_t)(End-Start+(Letters[Start]!=0))==len && memcmp(&Letters[Start], s, len)==0);
  }
  bool operator!=(const char *s) const {
    return !operator==(s);
  }
  void operator+=(const char c) {
    if (End<MAX_WORD_SIZE-1){
      End+=(Letters[End]>0);
      Letters[End]=tolower(c);
    }
  }
  U8 operator[](U8 i) const {
    return (End-Start>=i)?Letters[Start+i]:0;
  }
  U8 operator()(U8 i) const {
    return (End-Start>=i)?Letters[End-i]:0;
  }
  U32 Length() const {
    if (Letters[Start]!=0)
      return End-Start+1;
    return 0;
  }
  void GetHashes() {
    Hash[0] = 0xc01dflu, Hash[1] = ~Hash[0];
    for (int i=Start; i<=End; i++) {
      U8 l = Letters[i];
      Hash[0]^=hash(Hash[0], l, i);
      Hash[1]^=hash(Hash[1], 
        ((l&0x80)==0)?l&0x5F:
        ((l&0xC0)==0x80)?l&0x3F:
        ((l&0xE0)==0xC0)?l&0x1F:
        ((l&0xF0)==0xE0)?l&0xF:l&0x7
      );
    }
    Hash[2] = (~Hash[0])^Hash[1];
    Hash[3] = (~Hash[1])^Hash[0];
  }
  bool ChangeSuffix(const char *OldSuffix, const char *NewSuffix) {
    size_t len=strlen(OldSuffix);
    if (Length()>len && memcmp(&Letters[End-len+1], OldSuffix, len)==0){
      size_t n=strlen(NewSuffix);
      if (n>0){
        memcpy(&Letters[End-int(len)+1], NewSuffix, min(MAX_WORD_SIZE-1,End+int(n))-End);
        End=min(MAX_WORD_SIZE-1, End-int(len)+int(n));
      }
      else
        End-=U8(len);
      return true;
    }
    return false;
  }
  bool MatchesAny(const char* a[], const int count) {
    int i=0;
    size_t len = (size_t)Length();
    for (; i<count && (len!=strlen(a[i]) || memcmp(&Letters[Start], a[i], len)!=0); i++);
    return i<count;
  }
  bool EndsWith(const char *Suffix) const {
    size_t len=strlen(Suffix);
    return (Length()>len && memcmp(&Letters[End-len+1], Suffix, len)==0);
  }
  bool StartsWith(const char *Prefix) const {
    size_t len=strlen(Prefix);
    return (Length()>len && memcmp(&Letters[Start], Prefix, len)==0);
  }
  void print() const {
    for(int r=Start;r<=End;r++)
      printf("%c",(char)Letters[r]);
    printf("\n");
  }
};

class Segment1 {
public:
  Word FirstWord; // useful following questions
  U32 WordCount;
  U32 NumCount;
};

class Sentence : public Segment1 {
public:
  enum Types { // possible sentence types, excluding Imperative
    Declarative,
    Interrogative,
    Exclamative,
    Count
  };
  Types Type;
  U32 SegmentCount;
  U32 VerbIndex; // relative position of last detected verb
  U32 NounIndex; // relative position of last detected noun
  U32 CapitalIndex; // relative position of last capitalized word, excluding the initial word of this sentence
  Word lastVerb, lastNoun, lastCapital;
};

class Paragraph {
public:
  U32 SentenceCount, TypeCount[Sentence::Types::Count], TypeMask;
};

class Language {
public:
   enum Flags {
    Verb                   = (1<<0),
    Noun                   = (1<<1)
  };
  enum Ids {
    Unknown,
    English,
    French,
    German,
    Estonian,
    Count
  };
  virtual ~Language() {};
  virtual bool IsAbbreviation(Word *W) = 0;
};

class English: public Language {
private:
  static const int NUM_ABBREV = 6;
  const char *Abbreviations[NUM_ABBREV]={ "mr","mrs","ms","dr","st","jr" };
public:
  enum Flags {
   
    Adjective              = (1<<2),
    Plural                 = (1<<3),
    Male                   = (1<<4),
    Female                 = (1<<5),
    Negation               = (1<<6),
    PastTense              = (1<<7)|Verb,
    PresentParticiple      = (1<<8)|Verb,
    AdjectiveSuperlative   = (1<<9)|Adjective,
    AdjectiveWithout       = (1<<10)|Adjective,
    AdjectiveFull          = (1<<11)|Adjective,
    AdverbOfManner         = (1<<12),
    SuffixNESS             = (1<<13),
    SuffixITY              = (1<<14)|Noun,
    SuffixCapable          = (1<<15),
    SuffixNCE              = (1<<16),
    SuffixNT               = (1<<17),
    SuffixION              = (1<<18),
    SuffixAL               = (1<<19)|Adjective,
    SuffixIC               = (1<<20)|Adjective,
    SuffixIVE              = (1<<21),
    SuffixOUS              = (1<<22)|Adjective,
    PrefixOver             = (1<<23),
    PrefixUnder            = (1<<24)
  };
  bool IsAbbreviation(Word *W) { return W->MatchesAny(Abbreviations, NUM_ABBREV); };
};

class French: public Language {
private:
  static const int NUM_ABBREV = 2;
  const char *Abbreviations[NUM_ABBREV]={ "m","mm" };
public:
  enum Flags {
    
    Adjective              = (1<<2),
    Plural                 = (1<<3)
  };
  bool IsAbbreviation(Word *W) { return W->MatchesAny(Abbreviations, NUM_ABBREV); };
};

class German : public Language {
private:
  static const int NUM_ABBREV = 3;
  const char *Abbreviations[NUM_ABBREV]={ "fr","hr","hrn" };
public:
  enum Flags {
    Adjective              = (1<<2),
    Plural                 = (1<<3),
    Female                 = (1<<4)
  };
  bool IsAbbreviation(Word *W) { return W->MatchesAny(Abbreviations, NUM_ABBREV); };
};


class Estonian : public Language {
private:
  static const int NUM_ABBREV = 3;
  const char *Abbreviations[NUM_ABBREV]={ "hr","pr","prl" };
public:
  enum Flags {
    Adjective              = (1<<2),
    Plural                 = (1<<3),
    PreffixNoun            = (1<<4)|Noun
  };
  bool IsAbbreviation(Word *W) { return W->MatchesAny(Abbreviations, NUM_ABBREV); };
};
//////////////////////////// Stemming routines /////////////////////////

class Stemmer {
    protected:
  U32 GetRegion(const Word *W, const U32 From) {
    bool hasVowel = false;
    for (int i=W->Start+From; i<=W->End; i++) {
      if (IsVowel(W->Letters[i])) {
        hasVowel = true;
        continue;
      }
      else if (hasVowel)
        return i-W->Start+1;
    }
    return W->Start+W->Length();
  }
  bool SuffixInRn(const Word *W, const U32 Rn, const char *Suffix) {
    return (W->Start!=W->End && Rn<=W->Length()-strlen(Suffix));
  }
public:
  virtual ~Stemmer() {};
  virtual bool IsVowel(const char c) = 0;
  virtual void Hash(Word *W) = 0;
  virtual bool Stem(Word *W) = 0;
};


class EnglishStemmer: public Stemmer {
private:
  static const int NUM_VOWELS = 6;
  const char Vowels[NUM_VOWELS]={'a','e','i','o','u','y'};
  static const int NUM_DOUBLES = 9;
  const char Doubles[NUM_DOUBLES]={'b','d','f','g','m','n','p','r','t'};
  static const int NUM_LI_ENDINGS = 10;
  const char LiEndings[NUM_LI_ENDINGS]={'c','d','e','g','h','k','m','n','r','t'};
  static const int NUM_NON_SHORT_CONSONANTS = 3;
  const char NonShortConsonants[NUM_NON_SHORT_CONSONANTS]={'w','x','Y'};
  static const int NUM_MALE_WORDS = 9;
  const char *MaleWords[NUM_MALE_WORDS]={"he","him","his","himself","man","men","boy","husband","actor"};
  static const int NUM_FEMALE_WORDS = 8;
  const char *FemaleWords[NUM_FEMALE_WORDS]={"she","her","herself","woman","women","girl","wife","actress"};
  static const int NUM_COMMON_WORDS = 12;
  const char *CommonWords[NUM_COMMON_WORDS]={"the","be","to","of","and","in","that","you","have","with","from","but"};
  static const int NUM_SUFFIXES_STEP0 = 3;
  const char *SuffixesStep0[NUM_SUFFIXES_STEP0]={"'s'","'s","'"};
  static const int NUM_SUFFIXES_STEP1b = 6;
  const char *SuffixesStep1b[NUM_SUFFIXES_STEP1b]={"eedly","eed","ed","edly","ing","ingly"};
  const U32 TypesStep1b[NUM_SUFFIXES_STEP1b]={English::AdverbOfManner,0,English::PastTense,English::AdverbOfManner|English::PastTense,English::PresentParticiple,English::AdverbOfManner|English::PresentParticiple};
  static const int NUM_SUFFIXES_STEP2 = 22;
  const char *(SuffixesStep2[NUM_SUFFIXES_STEP2])[2]={
    {"ization", "ize"},
    {"ational", "ate"},
    {"ousness", "ous"},
    {"iveness", "ive"},
    {"fulness", "ful"},
    {"tional", "tion"},
    {"lessli", "less"},
    {"biliti", "ble"},
    {"entli", "ent"},
    {"ation", "ate"},
    {"alism", "al"},
    {"aliti", "al"},
    {"fulli", "ful"},
    {"ousli", "ous"},
    {"iviti", "ive"},
    {"enci", "ence"},
    {"anci", "ance"},
    {"abli", "able"},
    {"izer", "ize"},
    {"ator", "ate"},
    {"alli", "al"},
    {"bli", "ble"}
  };
  const U32 TypesStep2[NUM_SUFFIXES_STEP2]={
    English::SuffixION,
    English::SuffixION|English::SuffixAL,
    English::SuffixNESS,
    English::SuffixNESS,
    English::SuffixNESS,
    English::SuffixION|English::SuffixAL,
    English::AdverbOfManner,
    English::AdverbOfManner|English::SuffixITY,
    English::AdverbOfManner,
    English::SuffixION,
    0,
    English::SuffixITY,
    English::AdverbOfManner,
    English::AdverbOfManner,
    English::SuffixITY,
    0,
    0,
    English::AdverbOfManner,
    0,
    0,
    English::AdverbOfManner,
    English::AdverbOfManner
  };
  static const int NUM_SUFFIXES_STEP3 = 8;
  const char *(SuffixesStep3[NUM_SUFFIXES_STEP3])[2]={
    {"ational", "ate"},
    {"tional", "tion"},
    {"alize", "al"},
    {"icate", "ic"},
    {"iciti", "ic"},
    {"ical", "ic"},
    {"ful", ""},
    {"ness", ""}
  };
  const U32 TypesStep3[NUM_SUFFIXES_STEP3]={English::SuffixION|English::SuffixAL,English::SuffixION|English::SuffixAL,0,0,English::SuffixITY,English::SuffixAL,English::AdjectiveFull,English::SuffixNESS};
  static const int NUM_SUFFIXES_STEP4 = 20;
  const char *SuffixesStep4[NUM_SUFFIXES_STEP4]={"al","ance","ence","er","ic","able","ible","ant","ement","ment","ent","ou","ism","ate","iti","ous","ive","ize","sion","tion"};
  const U32 TypesStep4[NUM_SUFFIXES_STEP4]={
    English::SuffixAL,
    English::SuffixNCE,
    English::SuffixNCE,
    0,
    English::SuffixIC,
    English::SuffixCapable,
    English::SuffixCapable,
    English::SuffixNT,
    0,
    0,
    English::SuffixNT,
    0,
    0,
    0,
    English::SuffixITY,
    English::SuffixOUS,
    English::SuffixIVE,
    0,
    English::SuffixION,
    English::SuffixION
  };
  static const int NUM_EXCEPTION_REGION1 = 3;
  const char *ExceptionsRegion1[NUM_EXCEPTION_REGION1]={"gener","arsen","commun"};
  static const int NUM_EXCEPTIONS1 = 19;
  const char *(Exceptions1[NUM_EXCEPTIONS1])[2]={
    {"skis", "ski"},
    {"skies", "sky"},
    {"dying", "die"},
    {"lying", "lie"},
    {"tying", "tie"},
    {"idly", "idl"},
    {"gently", "gentl"},
    {"ugly", "ugli"},
    {"early", "earli"},
    {"only", "onli"},
    {"singly", "singl"},
    {"sky", "sky"},
    {"news", "news"},
    {"howe", "howe"},
    {"atlas", "atlas"},
    {"cosmos", "cosmos"},
    {"bias", "bias"},
    {"andes", "andes"},
    {"texas", "texas"}
  };
  const U32 TypesExceptions1[NUM_EXCEPTIONS1]={
    English::Noun|English::Plural,
    English::Noun|English::Plural,
    English::PresentParticiple,
    English::PresentParticiple,
    English::PresentParticiple,
    English::AdverbOfManner,
    English::AdverbOfManner,
    English::Adjective,
    English::Adjective|English::AdverbOfManner,
    0,
    English::AdverbOfManner,
    English::Noun,
    English::Noun,
    0,
    English::Noun,
    English::Noun,
    English::Noun,
    English::Noun|English::Plural,
    English::Noun
  };
  static const int NUM_EXCEPTIONS2 = 8;
  const char *Exceptions2[NUM_EXCEPTIONS2]={"inning","outing","canning","herring","earring","proceed","exceed","succeed"};
  const U32 TypesExceptions2[NUM_EXCEPTIONS2]={English::Noun,English::Noun,English::Noun,English::Noun,English::Noun,English::Verb,English::Verb,English::Verb}; 
  inline bool IsConsonant(const char c){
    return !IsVowel(c);
  }
  inline bool IsShortConsonant(const char c){
    return !CharInArray(c, NonShortConsonants, NUM_NON_SHORT_CONSONANTS);
  }
  inline bool IsDouble(const char c){
    return CharInArray(c, Doubles, NUM_DOUBLES);
  }
  inline bool IsLiEnding(const char c){
    return CharInArray(c, LiEndings, NUM_LI_ENDINGS);
  }
   
  U32 GetRegion1(const Word *W){
    for (int i=0; i<NUM_EXCEPTION_REGION1; i++){
      if (W->StartsWith(ExceptionsRegion1[i]))
        return U32(strlen(ExceptionsRegion1[i]));
    }
    return GetRegion(W, 0);
  }
   
  bool EndsInShortSyllable(const Word *W){
    if (W->End==W->Start)
      return false;
    else if (W->End==W->Start+1)
      return IsVowel((*W)(1)) && IsConsonant((*W)(0));
    else
      return (IsConsonant((*W)(2)) && IsVowel((*W)(1)) && IsConsonant((*W)(0)) && IsShortConsonant((*W)(0)));
  }
  bool IsShortWord(const Word *W){
    return (EndsInShortSyllable(W) && GetRegion1(W)==W->Length());
  }
  inline bool HasVowels(const Word *W){
    for (int i=W->Start; i<=W->End; i++){
      if (IsVowel(W->Letters[i]))
        return true;
    }
    return false;
  }
  bool TrimStartingApostrophe(Word *W){
    bool result=false;
    //trim all apostrophes from the beginning
    int cnt=0;
    while(W->Start!=W->End && (*W)[0]=='\'') {
      result=true;
      W->Start++;
      cnt++;
    }
    //trim the same number of apostrophes from the end (if there are)
    while(W->Start!=W->End && (*W)(0)=='\'') {
      if(cnt==0)break;
      W->End--;
      cnt--;
    }
    return result;
  }

  void MarkYsAsConsonants(Word *W){
    if ((*W)[0]=='y')
      W->Letters[W->Start]='Y';
    for (int i=W->Start+1; i<=W->End; i++){
      if (IsVowel(W->Letters[i-1]) && W->Letters[i]=='y')
        W->Letters[i]='Y';
    }
  }
  bool ProcessPrefixes(Word *W){
    if (W->StartsWith("irr") && W->Length()>5 && ((*W)[3]=='a' || (*W)[3]=='e'))
      W->Start+=2, W->Type|=English::Negation;
    else if (W->StartsWith("over") && W->Length()>5)
      W->Start+=4, W->Type|=English::PrefixOver;
    else if (W->StartsWith("under") && W->Length()>6)
      W->Start+=5, W->Type|=English::PrefixUnder;
    else if (W->StartsWith("unn") && W->Length()>5)
      W->Start+=2, W->Type|=English::Negation;
    else if (W->StartsWith("non") && W->Length()>(U32)(5+((*W)[3]=='-')))
      W->Start+=2+((*W)[3]=='-'), W->Type|=English::Negation;
    else
      return false;
    return true;
  }
  bool ProcessSuperlatives(Word *W){
    if (W->EndsWith("est") && W->Length()>4){
      U8 i=W->End;
      W->End-=3;
      W->Type|=English::AdjectiveSuperlative;

      if ((*W)(0)==(*W)(1) && (*W)(0)!='r' && !(W->Length()>=4 && memcmp("sugg",&W->Letters[W->End-3],4)==0)){
        W->End-= ( ((*W)(0)!='f' && (*W)(0)!='l' && (*W)(0)!='s') ||
                   (W->Length()>4 && (*W)(1)=='l' && ((*W)(2)=='u' || (*W)(3)=='u' || (*W)(3)=='v'))) &&
                   (!(W->Length()==3 && (*W)(1)=='d' && (*W)(2)=='o'));
        if (W->Length()==2 && ((*W)[0]!='i' || (*W)[1]!='n'))
          W->End = i, W->Type&=~English::AdjectiveSuperlative;
      }
      else{
        switch((*W)(0)){
          case 'd': case 'k': case 'm': case 'y': break;
          case 'g': {
            if (!( W->Length()>3 && ((*W)(1)=='n' || (*W)(1)=='r') && memcmp("cong",&W->Letters[W->End-3],4)!=0 ))
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            else
              W->End+=((*W)(2)=='a');
            break;
          }
          case 'i': {W->Letters[W->End]='y'; break;}
          case 'l': {
            if (W->End==W->Start+1 || memcmp("mo",&W->Letters[W->End-2],2)==0)
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            else
              W->End+=IsConsonant((*W)(1));
            break;
          }
          case 'n': {
            if (W->Length()<3 || IsConsonant((*W)(1)) || IsConsonant((*W)(2)))
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            break;
          }
          case 'r': {
            if (W->Length()>3 && IsVowel((*W)(1)) && IsVowel((*W)(2)))
              W->End+=((*W)(2)=='u') && ((*W)(1)=='a' || (*W)(1)=='i');
            else
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            break;
          }
          case 's': {W->End++; break;}
          case 'w': {
            if (!(W->Length()>2 && IsVowel((*W)(1))))
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            break;
          }
          case 'h': {
            if (!(W->Length()>2 && IsConsonant((*W)(1))))
              W->End = i, W->Type&=~English::AdjectiveSuperlative;
            break;
          }
          default: {
            W->End+=3;
            W->Type&=~English::AdjectiveSuperlative;
          }
        }
      }
    }
    return (W->Type&English::AdjectiveSuperlative)>0;
  }
  bool Step0(Word *W){
    for (int i=0; i<NUM_SUFFIXES_STEP0; i++){
      if (W->EndsWith(SuffixesStep0[i])){
        W->End-=U8(strlen(SuffixesStep0[i]));
        W->Type|=English::Plural;
        return true;
      }
    }
    return false;
  }
  bool Step1a(Word *W){
    if (W->EndsWith("sses")){
      W->End-=2;
      W->Type|=English::Plural;
      return true;
    }
    if (W->EndsWith("ied") || W->EndsWith("ies")){
      W->Type|=((*W)(0)=='d')?English::PastTense:English::Plural;
      W->End-= W->Length()>4 ? 2 : 1;
      return true;
    }
    if (W->EndsWith("us") || W->EndsWith("ss"))
      return false;
    if ((*W)(0)=='s' && W->Length()>2){
      for (int i=W->Start;i<=W->End-2;i++){
        if (IsVowel(W->Letters[i])){
          W->End--;
          W->Type|=English::Plural;
          return true;
        }
      }
    }
    if (W->EndsWith("n't") && W->Length()>4){
      switch ((*W)(3)){
        case 'a': {
          if ((*W)(4)=='c')
            W->End-=2;
          else
            W->ChangeSuffix("n't","ll");
          break;
        }
        case 'i': {W->ChangeSuffix("in't","m"); break;}
        case 'o': {
          if ((*W)(4)=='w')
            W->ChangeSuffix("on't","ill");
          else
            W->End-=3;
          break;
        }
        default: W->End-=3;
      }
      W->Type|=English::Negation;
      return true;
    }
    if (W->EndsWith("hood") && W->Length()>7){
      W->End-=4;
      return true;
    }
    return false;
  }
  bool Step1b(Word *W, const U32 R1){
    for (int i=0; i<NUM_SUFFIXES_STEP1b; i++){
      if (W->EndsWith(SuffixesStep1b[i])){
        switch(i){
          case 0: case 1: {
            if (SuffixInRn(W, R1, SuffixesStep1b[i]))
              W->End-=1+i*2;
            break;
          }
          default: {
            U8 j=W->End;
            W->End-=U8(strlen(SuffixesStep1b[i]));
            if (HasVowels(W)){
              if (W->EndsWith("at") || W->EndsWith("bl") || W->EndsWith("iz") || IsShortWord(W))
                (*W)+='e';
              else if (W->Length()>2){
                if ((*W)(0)==(*W)(1) && IsDouble((*W)(0)))
                  W->End--;
                else if (i==2 || i==3){
                  switch((*W)(0)){
                    case 'c': case 's': case 'v': {W->End+=!(W->EndsWith("ss") || W->EndsWith("ias")); break;}
                    case 'd': {
                      static const char nAllowed[4] = {'a','e','i','o'};
                      W->End+=IsVowel((*W)(1)) && (!CharInArray((*W)(2), nAllowed, 4)); break;
                    }
                    case 'k': {W->End+=W->EndsWith("uak"); break;}
                    case 'l': {
                      static const char Allowed1[10] = {'b','c','d','f','g','k','p','t','y','z'};
                      static const char Allowed2[4] = {'a','i','o','u'};
                      W->End+= CharInArray((*W)(1), Allowed1, 10) ||
                                (CharInArray((*W)(1), Allowed2, 4) && IsConsonant((*W)(2)));
                      break;
                    }
                  }
                }
                else if (i>=4){
                  switch((*W)(0)){
                    case 'd': {
                      if (IsVowel((*W)(1)) && (*W)(2)!='a' && (*W)(2)!='e' && (*W)(2)!='o')
                        (*W)+='e';
                      break;
                    }
                    case 'g': {
                      static const char Allowed[7] = {'a','d','e','i','l','r','u'};
                      if (
                        CharInArray((*W)(1), Allowed, 7) || (
                         (*W)(1)=='n' && (
                          (*W)(2)=='e' ||
                          ((*W)(2)=='u' && (*W)(3)!='b' && (*W)(3)!='d') ||
                          ((*W)(2)=='a' && ((*W)(3)=='r' || ((*W)(3)=='h' && (*W)(4)=='c'))) ||
                          (W->EndsWith("ring") && ((*W)(4)=='c' || (*W)(4)=='f'))
                         )
                        ) 
                      )
                        (*W)+='e';
                      break;
                    }
                    case 'l': {
                      if (!((*W)(1)=='l' || (*W)(1)=='r' || (*W)(1)=='w' || (IsVowel((*W)(1)) && IsVowel((*W)(2)))))
                        (*W)+='e';
                      if (W->EndsWith("uell") && W->Length()>4 && (*W)(4)!='q')
                        W->End--;
                      break;
                    }
                    case 'r': {
                      if ((
                        ((*W)(1)=='i' && (*W)(2)!='a' && (*W)(2)!='e' && (*W)(2)!='o') ||
                        ((*W)(1)=='a' && (!((*W)(2)=='e' || (*W)(2)=='o' || ((*W)(2)=='l' && (*W)(3)=='l')))) ||
                        ((*W)(1)=='o' && (!((*W)(2)=='o' || ((*W)(2)=='t' && (*W)(3)!='s')))) ||
                        (*W)(1)=='c' || (*W)(1)=='t') && (!W->EndsWith("str"))
                      )
                        (*W)+='e';
                      break;
                    }
                    case 't': {
                      if ((*W)(1)=='o' && (*W)(2)!='g' && (*W)(2)!='l' && (*W)(2)!='i' && (*W)(2)!='o')
                        (*W)+='e';
                      break;
                    }
                    case 'u': {
                      if (!(W->Length()>3 && IsVowel((*W)(1)) && IsVowel((*W)(2))))
                        (*W)+='e';
                      break;
                    }
                    case 'z': {
                      if (W->EndsWith("izz") && W->Length()>3 && ((*W)(3)=='h' || (*W)(3)=='u'))
                        W->End--;
                      else if ((*W)(1)!='t' && (*W)(1)!='z')
                        (*W)+='e';
                      break;
                    }
                    case 'k': {
                      if (W->EndsWith("uak"))
                        (*W)+='e';
                      break;
                    }
                    case 'b': case 'c': case 's': case 'v': {
                      if (!(
                        ((*W)(0)=='b' && ((*W)(1)=='m' || (*W)(1)=='r')) ||
                        W->EndsWith("ss") || W->EndsWith("ias") || (*W)=="zinc"
                      ))
                        (*W)+='e';
                      break;
                    }
                  }
                }
              }
            }
            else{
              W->End=j;
              return false;
            }
          }
        }
        W->Type|=TypesStep1b[i];
        return true;
      }
    }
    return false;
  }
  bool Step1c(Word *W){
    if (W->Length()>2 && tolower((*W)(0))=='y' && IsConsonant((*W)(1))){
      W->Letters[W->End]='i';
      return true;
    }
    return false;
  }
  bool Step2(Word *W, const U32 R1){
    for (int i=0; i<NUM_SUFFIXES_STEP2; i++){
      if (W->EndsWith(SuffixesStep2[i][0]) && SuffixInRn(W, R1, SuffixesStep2[i][0])){
        W->ChangeSuffix(SuffixesStep2[i][0], SuffixesStep2[i][1]);
        W->Type|=TypesStep2[i];
        return true;
      }
    }
    if (W->EndsWith("logi") && SuffixInRn(W, R1, "ogi")){
      W->End--;
      return true;
    }
    else if (W->EndsWith("li")){
      if (SuffixInRn(W, R1, "li") && IsLiEnding((*W)(2))){
        W->End-=2;
        W->Type|=English::AdverbOfManner;
        return true;
      }
      else if (W->Length()>3){
        switch((*W)(2)){
            case 'b': {
              W->Letters[W->End]='e';
              W->Type|=English::AdverbOfManner;
              return true;              
            }
            case 'i': {
              if (W->Length()>4){
                W->End-=2;
                W->Type|=English::AdverbOfManner;
                return true;
              }
              break;
            }
            case 'l': {
              if (W->Length()>5 && ((*W)(3)=='a' || (*W)(3)=='u')){
                W->End-=2;
                W->Type|=English::AdverbOfManner;
                return true;
              }
              break;
            }
            case 's': {
              W->End-=2;
              W->Type|=English::AdverbOfManner;
              return true;
            }
            case 'e': case 'g': case 'm': case 'n': case 'r': case 'w': {
              if (W->Length()>(U32)(4+((*W)(2)=='r'))){
                W->End-=2;
                W->Type|=English::AdverbOfManner;
                return true;
              }
            }
        }
      }
    }
    return false;
  }
  bool Step3(Word *W, const U32 R1, const U32 R2){
    bool res=false;
    for (int i=0; i<NUM_SUFFIXES_STEP3; i++){
      if (W->EndsWith(SuffixesStep3[i][0]) && SuffixInRn(W, R1, SuffixesStep3[i][0])){
        W->ChangeSuffix(SuffixesStep3[i][0], SuffixesStep3[i][1]);
        W->Type|=TypesStep3[i];
        res=true;
        break;
      }
    }
    if (W->EndsWith("ative") && SuffixInRn(W, R2, "ative")){
      W->End-=5;
      W->Type|=English::SuffixIVE;
      return true;
    }
    if (W->Length()>5 && W->EndsWith("less")){
      W->End-=4;
      W->Type|=English::AdjectiveWithout;
      return true;
    }
    return res;
  }
  bool Step4(Word *W, const U32 R2){
    bool res=false;
    for (int i=0; i<NUM_SUFFIXES_STEP4; i++){
      if (W->EndsWith(SuffixesStep4[i]) && SuffixInRn(W, R2, SuffixesStep4[i])){
        W->End-=U8(strlen(SuffixesStep4[i])-(i>17));
        if (!(i==10 /* ent */ && (*W)(0)=='m'))
          W->Type|=TypesStep4[i];
        if (i==0 && W->EndsWith("nti")){
          W->End--;
          res=true;
          continue;
        }
        return true;
      }
    }
    return res;
  }
  bool Step5(Word *W, const U32 R1, const U32 R2){
    if ((*W)(0)=='e' && (*W)!="here"){
      if (SuffixInRn(W, R2, "e"))
        W->End--;
      else if (SuffixInRn(W, R1, "e")){
        W->End--;
        W->End+=!EndsInShortSyllable(W);
      }
      else
        return false;
      return true;
    }
    else if (W->Length()>1 && (*W)(0)=='l' && SuffixInRn(W, R2, "l") && (*W)(1)=='l'){
      W->End--;
      return true;
    }
    return false;
  }
public:
  inline bool IsVowel(const char c) final {
    return CharInArray(c, Vowels, NUM_VOWELS);
  }
  inline void Hash(Word *W) final {
    W->Hash[2] = W->Hash[3] = 0xb0a710ad;
    for (int i=W->Start; i<=W->End; i++) {
      U8 l = W->Letters[i];
      W->Hash[2]=W->Hash[2]*263*32 + l;
      if (IsVowel(l))
        W->Hash[3]=W->Hash[3]*997*8 + (l/4-22);
      else if (l>='b' && l<='z')
        W->Hash[3]=W->Hash[3]*271*32 + (l-97);
      else
        W->Hash[3]=W->Hash[3]*11*32 + l;
    }
  }
  bool Stem(Word *W){
    if (W->Length()<2){
      Hash(W);
      return false;
    }
    bool res = TrimStartingApostrophe(W);
    res|=ProcessPrefixes(W);
    res|=ProcessSuperlatives(W);
    for (int i=0; i<NUM_EXCEPTIONS1; i++){
      if ((*W)==Exceptions1[i][0]){
        if (i<11){
          size_t len=strlen(Exceptions1[i][1]);
          memcpy(&W->Letters[W->Start], Exceptions1[i][1], len);
          W->End=W->Start+U8(len-1);
        }
        Hash(W);
        W->Type|=TypesExceptions1[i];
        W->Language = Language::English;
        return (i<11);
      }
    }

    // Start of modified Porter2 Stemmer
    MarkYsAsConsonants(W);
    U32 R1=GetRegion1(W), R2=GetRegion(W,R1);
    res|=Step0(W);
    res|=Step1a(W);
    for (int i=0; i<NUM_EXCEPTIONS2; i++){
      if ((*W)==Exceptions2[i]){
        Hash(W);
        W->Type|=TypesExceptions2[i];
        W->Language = Language::English;
        return res;
      }
    }
    res|=Step1b(W, R1);
    res|=Step1c(W);
    res|=Step2(W, R1);
    res|=Step3(W, R1, R2);
    res|=Step4(W, R2);
    res|=Step5(W, R1, R2);

    for (U8 i=W->Start; i<=W->End; i++){
      if (W->Letters[i]=='Y')
        W->Letters[i]='y';
    }
    if (!W->Type || W->Type==English::Plural) {
      if (W->MatchesAny(MaleWords, NUM_MALE_WORDS))
        res = true, W->Type|=English::Male;
      else if (W->MatchesAny(FemaleWords, NUM_FEMALE_WORDS))
        res = true, W->Type|=English::Female;
    }
    if (!res)
      res=W->MatchesAny(CommonWords, NUM_COMMON_WORDS);
    Hash(W);
    if (res)
      W->Language = Language::English;
    return res;
  }
};


class FrenchStemmer: public Stemmer {
private:
  static const int NUM_VOWELS = 17;
  const char Vowels[NUM_VOWELS]={'a','e','i','o','u','y','\xE2','\xE0','\xEB','\xE9','\xEA','\xE8','\xEF','\xEE','\xF4','\xFB','\xF9'};
  static const int NUM_COMMON_WORDS = 10;
  const char *CommonWords[NUM_COMMON_WORDS]={"de","la","le","et","en","un","une","du","que","pas"};
  static const int NUM_EXCEPTIONS = 3;
  const char *(Exceptions[NUM_EXCEPTIONS])[2]={
    {"monument", "monument"},
    {"yeux", "oeil"},
    {"travaux", "travail"},
  };
  const U32 TypesExceptions[NUM_EXCEPTIONS]={
    French::Noun,
    French::Noun|French::Plural,
    French::Noun|French::Plural
  };
  static const int NUM_SUFFIXES_STEP1 = 39;
  const char *SuffixesStep1[NUM_SUFFIXES_STEP1]={
    "ance","iqUe","isme","able","iste","eux","ances","iqUes","ismes","ables","istes", //11
    "atrice","ateur","ation","atrices","ateurs","ations", //6
    "logie","logies", //2
    "usion","ution","usions","utions", //4
    "ence","ences", //2
    "issement","issements", //2
    "ement","ements", //2
    "it\xE9","it\xE9s", //2
    "if","ive","ifs","ives", //4
    "euse","euses", //2
    "ment","ments" //2
  };
  static const int NUM_SUFFIXES_STEP2a = 35;
  const char *SuffixesStep2a[NUM_SUFFIXES_STEP2a]={
    "issaIent", "issantes", "iraIent", "issante",
    "issants", "issions", "irions", "issais",
    "issait", "issant", "issent", "issiez", "issons",
    "irais", "irait", "irent", "iriez", "irons",
    "iront", "isses", "issez", "\xEEmes",
    "\xEEtes", "irai", "iras", "irez", "isse",
    "ies", "ira", "\xEEt", "ie", "ir", "is",
    "it", "i"
  };
  static const int NUM_SUFFIXES_STEP2b = 38;
  const char *SuffixesStep2b[NUM_SUFFIXES_STEP2b]={
    "eraIent", "assions", "erions", "assent",
    "assiez", "\xE8rent", "erais", "erait",
    "eriez", "erons", "eront", "aIent", "antes",
    "asses", "ions", "erai", "eras", "erez",
    "\xE2mes", "\xE2tes", "ante", "ants",
    "asse", "\xE9""es", "era", "iez", "ais",
    "ait", "ant", "\xE9""e", "\xE9s", "er",
    "ez", "\xE2t", "ai", "as", "\xE9", "a"
  };
  static const int NUM_SET_STEP4 = 6;
  const char SetStep4[NUM_SET_STEP4]={'a','i','o','u','\xE8','s'};
  static const int NUM_SUFFIXES_STEP4 = 7;
  const char *SuffixesStep4[NUM_SUFFIXES_STEP4]={"i\xE8re","I\xE8re","ion","ier","Ier","e","\xEB"};
  static const int NUM_SUFFIXES_STEP5 = 5;
  const char *SuffixesStep5[NUM_SUFFIXES_STEP5]={"enn","onn","ett","ell","eill"};
  inline bool IsConsonant(const char c){
    return !IsVowel(c);
  }
    void ConvertUTF8(Word *W) {
    for (int i=W->Start; i<W->End; i++) {
      U8 c = W->Letters[i+1]+((W->Letters[i+1]<0xA0)?0x60:0x40);
      if (W->Letters[i]==0xC3 && (IsVowel(c) || (W->Letters[i+1]&0xDF)==0x87)) {
        W->Letters[i] = c;
        if (i+1<W->End)
          memmove(&W->Letters[i+1], &W->Letters[i+2], W->End-i-1);
        W->End--;
      }
    }
  }
  void MarkVowelsAsConsonants(Word *W){
    for (int i=W->Start; i<=W->End; i++){
      switch (W->Letters[i]) {
        case 'i': case 'u': {
          if (i>W->Start && i<W->End && (IsVowel(W->Letters[i-1]) || (W->Letters[i-1]=='q' && W->Letters[i]=='u')) && IsVowel(W->Letters[i+1]))
            W->Letters[i] = toupper(W->Letters[i]);
          break;
        }
        case 'y': {
          if ((i>W->Start && IsVowel(W->Letters[i-1])) || (i<W->End && IsVowel(W->Letters[i+1])))
            W->Letters[i] = toupper(W->Letters[i]);
        }
      }
    }
  }
  U32 GetRV(Word *W) {
    U32 len = W->Length(), res = W->Start+len;
    if (len>=3 && ((IsVowel(W->Letters[W->Start]) && IsVowel(W->Letters[W->Start+1])) || W->StartsWith("par") || W->StartsWith("col") || W->StartsWith("tap") ))
      return W->Start+3;
    else {
      for (int i=W->Start+1;i<=W->End;i++) {
        if (IsVowel(W->Letters[i]))
          return i+1;
      }
    }
    return res;
  }
     bool Step1(Word *W, const U32 RV, const U32 R1, const U32 R2, bool *ForceStep2a) {
    int i = 0;
    for (; i<11; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R2, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        if (i==3 /*able*/)
          W->Type|=French::Adjective;
        return true;
      }
    }
    for (; i<17; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R2, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        if (W->EndsWith("ic"))
          W->ChangeSuffix("c", "qU");
        return true;
      }
    }
    for (; i<25;i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R2, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]))-1-(i<19)*2;
        if (i>22) {
          W->End+=2;
          W->Letters[W->End]='t';
        }
        return true;
      }
    }
    for (; i<27; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R1, SuffixesStep1[i]) && IsConsonant((*W)((U8)strlen(SuffixesStep1[i])))) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        return true;
      }
    }
    for (; i<29; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, RV, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        if (W->EndsWith("iv") && SuffixInRn(W, R2, "iv")) {
          W->End-=2;
          if (W->EndsWith("at") && SuffixInRn(W, R2, "at"))
            W->End-=2;
        }
        else if (W->EndsWith("eus")) {
          if (SuffixInRn(W, R2, "eus"))
            W->End-=3;
          else if (SuffixInRn(W, R1, "eus"))
            W->Letters[W->End]='x';
        }
        else if ((W->EndsWith("abl") && SuffixInRn(W, R2, "abl")) || (W->EndsWith("iqU") && SuffixInRn(W, R2, "iqU")))
          W->End-=3;
        else if ((W->EndsWith("i\xE8r") && SuffixInRn(W, RV, "i\xE8r")) || (W->EndsWith("I\xE8r") && SuffixInRn(W, RV, "I\xE8r"))) {
          W->End-=2;
          W->Letters[W->End]='i';
        }
        return true;
      }
    }
    for (; i<31; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R2, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        if (W->EndsWith("abil")) {
          if (SuffixInRn(W, R2, "abil"))
            W->End-=4;
          else
            W->End--, W->Letters[W->End]='l';
        }
        else if (W->EndsWith("ic")) {
          if (SuffixInRn(W, R2, "ic"))
            W->End-=2;
          else
            W->ChangeSuffix("c", "qU");
        }
        else if (W->EndsWith("iv") && SuffixInRn(W, R2, "iv"))
          W->End-=2;
        return true;
      }
    }
    for (; i<35; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R2, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        if (W->EndsWith("at") && SuffixInRn(W, R2, "at")) {
          W->End-=2;
          if (W->EndsWith("ic")) {
            if (SuffixInRn(W, R2, "ic"))
              W->End-=2;
            else
              W->ChangeSuffix("c", "qU");
          }
        }
        return true;
      }
    }
    for (; i<37; i++) {
      if (W->EndsWith(SuffixesStep1[i])) {
        if (SuffixInRn(W, R2, SuffixesStep1[i])) {
          W->End-=U8(strlen(SuffixesStep1[i]));
          return true;
        }
        else if (SuffixInRn(W, R1, SuffixesStep1[i])) {
          W->ChangeSuffix(SuffixesStep1[i], "eux");
          return true;
        }
      }
    }
    for (; i<NUM_SUFFIXES_STEP1; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, RV+1, SuffixesStep1[i]) && IsVowel((*W)((U8)strlen(SuffixesStep1[i])))) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        (*ForceStep2a) = true;
        return true;
      }
    }
    if (W->EndsWith("eaux") || (*W)=="eaux") {
      W->End--;
      W->Type|=French::Plural;
      return true;
    }
    else if (W->EndsWith("aux") && SuffixInRn(W, R1, "aux")) {
      W->End--, W->Letters[W->End] = 'l';
      W->Type|=French::Plural;
      return true;
    }
    else if (W->EndsWith("amment") && SuffixInRn(W, RV, "amment")) {
      W->ChangeSuffix("amment", "ant");
      (*ForceStep2a) = true;
      return true;
    }
    else if (W->EndsWith("emment") && SuffixInRn(W, RV, "emment")) {
      W->ChangeSuffix("emment", "ent");
      (*ForceStep2a) = true;
      return true;
    }
    return false;
  }
  bool Step2a(Word *W, const U32 RV) {
    for (int i=0; i<NUM_SUFFIXES_STEP2a; i++) {
      if (W->EndsWith(SuffixesStep2a[i]) && SuffixInRn(W, RV+1, SuffixesStep2a[i]) && IsConsonant((*W)((U8)strlen(SuffixesStep2a[i])))) {
        W->End-=U8(strlen(SuffixesStep2a[i]));
        if (i==31 /*ir*/)
          W->Type|=French::Verb;
        return true;
      }
    }
    return false;
  }
  bool Step2b(Word *W, const U32 RV, const U32 R2) {
    for (int i=0; i<NUM_SUFFIXES_STEP2b; i++) {
      if (W->EndsWith(SuffixesStep2b[i]) && SuffixInRn(W, RV, SuffixesStep2b[i])) {
        switch (SuffixesStep2b[i][0]) {
          case 'a': case '\xE2': {
            W->End-=U8(strlen(SuffixesStep2b[i]));
            if (W->EndsWith("e") && SuffixInRn(W, RV, "e"))
              W->End--;
            return true;
          }
          default: {
            if (i!=14 || SuffixInRn(W, R2, SuffixesStep2b[i])) {
              W->End-=U8(strlen(SuffixesStep2b[i]));
              return true;
            }
          }
        }        
      }
    }
    return false;
  }
  void Step3(Word *W) {
    char *final = (char *)&W->Letters[W->End];
    if ((*final)=='Y')
      (*final) = 'i';
    else if ((*final)=='\xE7')
      (*final) = 'c';
  }
  bool Step4(Word *W, const U32 RV, const U32 R2) {
    bool res = false;
    if (W->Length()>=2 && W->Letters[W->End]=='s' && !CharInArray((*W)(1), SetStep4, NUM_SET_STEP4)) {
      W->End--;
      res = true;
    }
    for (int i=0; i<NUM_SUFFIXES_STEP4; i++) {
      if (W->EndsWith(SuffixesStep4[i]) && SuffixInRn(W, RV, SuffixesStep4[i])) {
        switch (i) {
          case 2: { //ion
            char prec = (*W)(3);
            if (SuffixInRn(W, R2, SuffixesStep4[i]) && SuffixInRn(W, RV+1, SuffixesStep4[i]) && (prec=='s' || prec=='t')) {
              W->End-=3;
              return true;
            }
            break;
          }
          case 5: { //e
            W->End--;
            return true;
          }
          case 6: { //\xEB
            if (W->EndsWith("gu\xEB")) {
              W->End--;
              return true;
            }
            break;
          }
          default: {
            W->ChangeSuffix(SuffixesStep4[i], "i");
            return true;
          }
        }
      }
    }
    return res;
  }
  bool Step5(Word *W) {
    for (int i=0; i<NUM_SUFFIXES_STEP5; i++) {
      if (W->EndsWith(SuffixesStep5[i])) {
        W->End--;
        return true;
      }
    }
    return false;
  }
  bool Step6(Word *W) {
    for (int i=W->End; i>=W->Start; i--) {
      if (IsVowel(W->Letters[i])) {
        if (i<W->End && (W->Letters[i]&0xFE)==0xE8) {
          W->Letters[i] = 'e';
          return true;
        }
        return false;
      }
    }
    return false;
  }
public:
  inline bool IsVowel(const char c) final {
    return CharInArray(c, Vowels, NUM_VOWELS);
  }
  inline void Hash(Word *W) final {
    W->Hash[2] = W->Hash[3] = ~0xeff1cace;
    for (int i=W->Start; i<=W->End; i++) {
      U8 l = W->Letters[i];
      W->Hash[2]=W->Hash[2]*251*32 + l;
      if (IsVowel(l))
        W->Hash[3]=W->Hash[3]*997*16 + l;
      else if (l>='b' && l<='z')
        W->Hash[3]=W->Hash[3]*271*32 + (l-97);
      else
        W->Hash[3]=W->Hash[3]*11*32 + l;
    }
  }
  bool Stem(Word *W) {
    ConvertUTF8(W);
    if (W->Length()<2) {
      Hash(W);
      return false;
    }
    for (int i=0; i<NUM_EXCEPTIONS; i++) {
      if ((*W)==Exceptions[i][0]) {
        size_t len=strlen(Exceptions[i][1]);
        memcpy(&W->Letters[W->Start], Exceptions[i][1], len);
        W->End=W->Start+U8(len-1);
        Hash(W);
        W->Type|=TypesExceptions[i];
        W->Language = Language::French;
        return true;
      }
    }
    MarkVowelsAsConsonants(W);
    U32 RV=GetRV(W), R1=GetRegion(W, 0), R2=GetRegion(W, R1);
    bool DoNextStep=false, res=Step1(W, RV, R1, R2, &DoNextStep);
    DoNextStep|=!res;
    if (DoNextStep) {
      DoNextStep = !Step2a(W, RV);
      res|=!DoNextStep;
      if (DoNextStep)
        res|=Step2b(W, RV, R2);
    }
    if (res)
      Step3(W);
    else
      res|=Step4(W, RV, R2);
    res|=Step5(W);
    res|=Step6(W);
    for (int i=W->Start; i<=W->End; i++)
      W->Letters[i] = tolower(W->Letters[i]);
    if (!res)
      res=W->MatchesAny(CommonWords, NUM_COMMON_WORDS);
    Hash(W);
    if (res)
      W->Language = Language::French;
    return res;
  }
};

class GermanStemmer : public Stemmer {
private:
  static const int NUM_VOWELS = 9;
  const char Vowels[NUM_VOWELS]={'a','e','i','o','u','y','\xE4','\xF6','\xFC'};
  static const int NUM_COMMON_WORDS = 10;
  const char *CommonWords[NUM_COMMON_WORDS]={"der","die","das","und","sie","ich","mit","sich","auf","nicht"};
  static const int NUM_ENDINGS = 10;
  const char Endings[NUM_ENDINGS]={'b','d','f','g','h','k','l','m','n','t'}; //plus 'r' for words ending in 's'
  static const int NUM_SUFFIXES_STEP1 = 6;
  const char *SuffixesStep1[NUM_SUFFIXES_STEP1]={"em","ern","er","e","en","es"};
  static const int NUM_SUFFIXES_STEP2 = 3;
  const char *SuffixesStep2[NUM_SUFFIXES_STEP2]={"en","er","est"};
  static const int NUM_SUFFIXES_STEP3 = 7;
  const char *SuffixesStep3[NUM_SUFFIXES_STEP3]={"end","ung","ik","ig","isch","lich","heit"};
  void ConvertUTF8(Word *W) {
    for (int i=W->Start; i<W->End; i++) {
      U8 c = W->Letters[i+1]+((W->Letters[i+1]<0x9F)?0x60:0x40);
      if (W->Letters[i]==0xC3 && (IsVowel(c) || c==0xDF)) {
        W->Letters[i] = c;
        if (i+1<W->End)
          memmove(&W->Letters[i+1], &W->Letters[i+2], W->End-i-1);
        W->End--;
      }
    }
  }
  void ReplaceSharpS(Word *W) {
    for (int i=W->Start; i<=W->End; i++) {
      if (W->Letters[i]==0xDF) {
        W->Letters[i]='s';
        if (i+1<MAX_WORD_SIZE) {
          memmove(&W->Letters[i+2], &W->Letters[i+1], MAX_WORD_SIZE-i-2);
          W->Letters[i+1]='s';
          W->End+=(W->End<MAX_WORD_SIZE-1);
        }
      }
    }
  }    
  void MarkVowelsAsConsonants(Word *W) {
    for (int i=W->Start+1; i<W->End; i++) {
      U8 c = W->Letters[i];
      if ((c=='u' || c=='y') && IsVowel(W->Letters[i-1]) && IsVowel(W->Letters[i+1]))
        W->Letters[i] = toupper(c);
    }
  }
  inline bool IsValidEnding(const char c, const bool IncludeR = false) {
    return CharInArray(c, Endings, NUM_ENDINGS) || (IncludeR && c=='r');
  }
  bool Step1(Word *W, const U32 R1) {
    int i = 0;
    for (; i<3; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R1, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        return true;
      }
    }
    for (; i<NUM_SUFFIXES_STEP1; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R1, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        W->End-=U8(W->EndsWith("niss"));
        return true;
      }
    }
    if (W->EndsWith("s") && SuffixInRn(W, R1, "s") && IsValidEnding((*W)(1), true)) {
      W->End--;
      return true;
    }
    return false;
  }
  bool Step2(Word *W, const U32 R1) {
    for (int i=0; i<NUM_SUFFIXES_STEP2; i++) {
      if (W->EndsWith(SuffixesStep2[i]) && SuffixInRn(W, R1, SuffixesStep2[i])) {
        W->End-=U8(strlen(SuffixesStep2[i]));
        return true;
      }
    }
    if (W->EndsWith("st") && SuffixInRn(W, R1, "st") && W->Length()>5 && IsValidEnding((*W)(2))) {
      W->End-=2;
      return true;
    }
    return false;
  }
  bool Step3(Word *W, const U32 R1, const U32 R2) {
    int i = 0;
    for (; i<2; i++) {
      if (W->EndsWith(SuffixesStep3[i]) && SuffixInRn(W, R2, SuffixesStep3[i])) {
        W->End-=U8(strlen(SuffixesStep3[i]));
        if (W->EndsWith("ig") && (*W)(2)!='e' && SuffixInRn(W, R2, "ig"))
          W->End-=2;
        if (i)
          W->Type|=German::Noun;
        return true;
      }
    }
    for (; i<5; i++) {
      if (W->EndsWith(SuffixesStep3[i]) && SuffixInRn(W, R2, SuffixesStep3[i]) && (*W)((U8)strlen(SuffixesStep3[i]))!='e') {
        W->End-=U8(strlen(SuffixesStep3[i]));
        if (i>2)
          W->Type|=German::Adjective;
        return true;
      }
    }
    for (; i<NUM_SUFFIXES_STEP3; i++) {
      if (W->EndsWith(SuffixesStep3[i]) && SuffixInRn(W, R2, SuffixesStep3[i])) {
        W->End-=U8(strlen(SuffixesStep3[i]));
        if ((W->EndsWith("er") || W->EndsWith("en")) && SuffixInRn(W, R1, "e?"))
          W->End-=2;
        if (i>5)
          W->Type|=German::Noun|German::Female;
        return true;
      }
    }
    if (W->EndsWith("keit") && SuffixInRn(W, R2, "keit")) {
      W->End-=4;
      if (W->EndsWith("lich") && SuffixInRn(W, R2, "lich"))
        W->End-=4;
      else if (W->EndsWith("ig") && SuffixInRn(W, R2, "ig"))
        W->End-=2;
      W->Type|=German::Noun|German::Female;
      return true;
    }
    return false;
  }
public:
  inline bool IsVowel(const char c) final {
    return CharInArray(c, Vowels, NUM_VOWELS);
  }
  inline void Hash(Word *W) final {
    W->Hash[2] = W->Hash[3] = ~0xbea7ab1e;
    for (int i=W->Start; i<=W->End; i++) {
      U8 l = W->Letters[i];
      W->Hash[2]=W->Hash[2]*263*32 + l;
      if (IsVowel(l))
        W->Hash[3]=W->Hash[3]*997*16 + l;
      else if (l>='b' && l<='z')
        W->Hash[3]=W->Hash[3]*251*32 + (l-97);
      else
        W->Hash[3]=W->Hash[3]*11*32 + l;
    }
  }
  bool Stem(Word *W) {
    ConvertUTF8(W);
    if (W->Length()<2) {
      Hash(W);
      return false;
    }
    ReplaceSharpS(W);
    MarkVowelsAsConsonants(W);
    U32 R1=GetRegion(W, 0), R2=GetRegion(W, R1);
    R1 = min(3, R1);
    bool res = Step1(W, R1);
    res|=Step2(W, R1);
    res|=Step3(W, R1, R2);
    for (int i=W->Start; i<=W->End; i++) {
      switch (W->Letters[i]) {
        case 0xE4: { W->Letters[i] = 'a'; break; }
        case 0xF6: case 0xFC: { W->Letters[i]-=0x87; break; }
        default: W->Letters[i] = tolower(W->Letters[i]);
      }
    }
    if (!res)
      res=W->MatchesAny(CommonWords, NUM_COMMON_WORDS);
    Hash(W);
    if (res)
      W->Language = Language::German;
    return res;
  }
};


class EstonianStemmer : public Stemmer {
private:
  static const int NUM_VOWELS = 9;
  const char Vowels[NUM_VOWELS]={'a','e','i','o','u','\xE4','\xF5','\xF6','\xFC'};
  static const int NUM_COMMON_WORDS = 18;
  const char *CommonWords[NUM_COMMON_WORDS]={"ja","ta","see","ei","et","kui","ma","mis","aga","oma","nad","ise","nagu","siis","kes","nii","ka","ning"};
  static const int NUM_SUFFIXES_STEP1 = 38;
  const char *SuffixesStep1[NUM_SUFFIXES_STEP1]={"d","te","de","e","id","sid","tesse","desse","esse","tes","des","es","test","dest","est","tele","dele","ele",
  "tel","del","el","telt","delt","elt","teke","deks","eks","teni","deni","eni","tena","dena","ena","teta","deta","tega","dega","ega"}; //plural
  static const int NUM_SUFFIXES_STEP2 = 16;
  const char *SuffixesStep2[NUM_SUFFIXES_STEP2]={"t","d","tt","de","tte","sse","s","st","le","l","lt","ks","ni","na","ta","ga"}; //singular
  static const int NUM_PREFFIXES_STEP1 = 23;
  const char *PreffixesStep1[NUM_PREFFIXES_STEP1]={"ala","all","ase","eel","ees","era","eri","igi","j\xE4rel","kaas","kald","k\xF5rval","liba","piki","risti","sega",
  "taga","ulgu","vaeg","vastas","vastu","\xFCla","\xFCli"}; //prefix+noun
  void ConvertUTF8(Word *W) {
    for (int i=W->Start; i<W->End; i++) {
      U8 c = W->Letters[i+1]+((W->Letters[i+1]<0x9F)?0x60:0x40);
      if (W->Letters[i]==0xC3 && (IsVowel(c))) {
        W->Letters[i] = c;
        if (i+1<W->End)
          memmove(&W->Letters[i+1], &W->Letters[i+2], W->End-i-1);
        W->End--;
      }
    }
  }
   inline bool IsConsonant(const char c){
    return !IsVowel(c);
  }
  void MarkVowelsAsConsonants(Word *W) {
    for (int i=W->Start+1; i<W->End; i++) {
      U8 c = W->Letters[i];
      if ((c=='u' || c=='y') && IsVowel(W->Letters[i-1]) && IsVowel(W->Letters[i+1]))
        W->Letters[i] = toupper(c);
    }
  }
  bool ProcessPrefixes(Word *W){
      int i=0;
      for (; i<NUM_PREFFIXES_STEP1; i++) {
          int plen=strlen(PreffixesStep1[i]);
           if (W->StartsWith(PreffixesStep1[i]) && 
           (W->Length()-plen)>2&&
           !(IsConsonant(W->Letters[plen+1]) &&IsConsonant(W->Letters[plen+2]))){
               W->Start+=strlen(PreffixesStep1[i]);
               W->Type|=Estonian::PreffixNoun;
               return true;
           }
      }
      return false;
   }
  bool Step1(Word *W, const U32 R1) {
    int i = 4;
    for (; i<NUM_SUFFIXES_STEP1; i++) {
      if (W->EndsWith(SuffixesStep1[i]) && SuffixInRn(W, R1, SuffixesStep1[i])) {
        W->End-=U8(strlen(SuffixesStep1[i]));
        W->Type|=Estonian::Plural;
        return true;
      }
    }
    return false;
  }
  bool Step2(Word *W, const U32 R1) {
    for (int i=0; i<NUM_SUFFIXES_STEP2; i++) {
      if (W->EndsWith(SuffixesStep2[i]) && SuffixInRn(W, R1, SuffixesStep2[i])) {
        W->End-=U8(strlen(SuffixesStep2[i]));
        
        return true;
      }
    }
    return false;
  }
  
public:
  inline bool IsVowel(const char c) final {
    return CharInArray(c, Vowels, NUM_VOWELS);
  }
  inline void Hash(Word *W) final {
    W->Hash[2] = W->Hash[3] = ~0xcea7be1e;
    for (int i=W->Start; i<=W->End; i++) {
      U8 l = W->Letters[i];
      W->Hash[2]=W->Hash[2]*263*32 + l;
      if (IsVowel(l))
        W->Hash[3]=W->Hash[3]*997*16 + l;
      else if (l>='b' && l<='z')
        W->Hash[3]=W->Hash[3]*251*32 + (l-97);
      else
        W->Hash[3]=W->Hash[3]*11*32 + l;
    }
  }
  bool Stem(Word *W) {
    ConvertUTF8(W);
    if (W->Length()<2) {
      Hash(W);
      return false;
    }
    
   bool res=ProcessPrefixes(W);
    U32 R1=GetRegion(W, 0), R2=GetRegion(W, R1);
    R1 = min(3, R1);
    res |= Step1(W, R1);
    res |= Step2(W, R1);
    for (int i=W->Start; i<=W->End; i++) {
      switch (W->Letters[i]) {
        case 0xE4: { W->Letters[i] = 'a'; break; }
        case 0xF5: { W->Letters[i] = 'o'; break; }
        case 0xF6: { W->Letters[i] = 'o'; break; }
        case 0xFC: { W->Letters[i] = 'u'; break; }
        default: W->Letters[i] = tolower(W->Letters[i]);
      }
    }
    if (!res)
      res=W->MatchesAny(CommonWords, NUM_COMMON_WORDS);
    Hash(W);
    if (res){
          W->Language = Language::Estonian;
    }

  return res;
  }
};

bool CAlpha2(U32 ins) {
  int function=0;
  int opc=ins >> 26;
      if (opc==0)   goto op_00;   
      if (opc==1)   return false;
      if (opc==2)   return false;
      if (opc==3)   return false;
      if (opc==4)   return false;
      if (opc==5)   return false;
      if (opc==6)   return false;
      if (opc==7)   return false;
      if (opc==0x08) return true; 
      if (opc==0x09) return true; 
      if (opc==0x0a) return true; 
      if (opc==0x0b) return true; 
      if (opc==0x0c) return true; 
      if (opc==0x0d) return true; 
      if (opc==0x0e) return true; 
      if (opc==0x0f) return true; 
      if (opc==0x10) goto op_10;
      if (opc==0x11) goto op_11;
      if (opc==0x12) goto op_12;
      if (opc==0x13) goto op_13;
      if (opc==0x14) goto op_14;
      if (opc==0x15) goto op_15;
      if (opc==0x16) goto op_16;
      if (opc==0x17) goto op_17;
      if (opc==0x18) goto op_18;
      if (opc==0x19) return true;
      if (opc==0x1a) return true;
      if (opc==0x1b) return true;
      if (opc==0x1c) goto op_1c;
      if (opc>=0x1d && opc<=0x3f) return true;
      return false;

op_00:  // PAL
  function = ins & 0x1fffffff;
  if( ((function > 0x3f) && (function < 0x80)) || (function > 0xbf))                             
  {                                                              
    return false;                                                   
  } 
  return true;

op_10:  // INTA
  function = (ins >> 5) & 0x7f;
  switch(function) {
  case 0x40: 
  case 0x00: 
  case 0x02: 
  case 0x49: 
  case 0x09: 
  case 0x0b: 
  case 0x0f: 
  case 0x12: 
  case 0x1b: 
  case 0x1d: 
  case 0x60: 
  case 0x20: 
  case 0x22: 
  case 0x69: 
  case 0x29: 
  case 0x2b: 
  case 0x2d: 
  case 0x32: 
  case 0x3b: 
  case 0x3d: 
  case 0x4d: 
  case 0x6d: return true;
  default:   return false; 
  }

op_11:  // INTL
  function = (ins >> 5) & 0x7f;
  
  switch(function) {
  case 0x00: 
  case 0x08: 
  case 0x14: 
  case 0x16: 
  case 0x20: 
  case 0x24: 
  case 0x26: 
  case 0x28: 
  case 0x40: 
  case 0x44: 
  case 0x46: 
  case 0x48: 
  case 0x61: 
  case 0x64: 
  case 0x66: 
  case 0x6c:  return true;
  default:    return false; 
  }

op_12:  // INTS
  function = (ins >> 5) & 0x7f;
  
  switch(function) {
  case 0x02:  
  case 0x06:  
  case 0x0b:  
  case 0x12:  
  case 0x16:  
  case 0x1b:  
  case 0x22:  
  case 0x26:  
  case 0x2b:  
  case 0x30:  
  case 0x31:  
  case 0x32:  
  case 0x34:  
  case 0x36:  
  case 0x39:  
  case 0x3b:  
  case 0x3c:  
  case 0x52:  
  case 0x57:  
  case 0x5a:  
  case 0x62:  
  case 0x67:  
  case 0x6a:  
  case 0x72:  
  case 0x77:  
  case 0x7a:  return true;
  default:    return false; 
  }

op_13:  // INTM

  function = (ins >> 5) & 0x7f;
  switch(function) 
  {
  case 0x40: 
  case 0x00: 
  case 0x60: 
  case 0x20: 
  case 0x30:  return true;
  default:   return false; 
  }

op_14:   // ITFP
  function = (ins >> 5) & 0x7ff;
  
  switch(function) {
  case 0x004:
  case 0x00a:
  case 0x08a:
  case 0x10a:
  case 0x18a:
  case 0x40a:
  case 0x48a:
  case 0x50a:
  case 0x58a:
  case 0x00b:
  case 0x04b:
  case 0x08b:
  case 0x0cb:
  case 0x10b:
  case 0x14b:
  case 0x18b:
  case 0x1cb:
  case 0x50b:
  case 0x54b:
  case 0x58b:
  case 0x5cb:
  case 0x70b:
  case 0x74b:
  case 0x78b:
  case 0x7cb:
  case 0x014:
  case 0x024:
  case 0x02a:
  case 0x0aa:
  case 0x12a:
  case 0x1aa:
  case 0x42a:
  case 0x4aa:
  case 0x52a:
  case 0x5aa:
  case 0x02b:
  case 0x06b:
  case 0x0ab:
  case 0x0eb:
  case 0x12b:
  case 0x16b:
  case 0x1ab:
  case 0x1eb:
  case 0x52b:
  case 0x56b:
  case 0x5ab:
  case 0x5eb:
  case 0x72b:
  case 0x76b:
  case 0x7ab:
  case 0x7eb:
    return true;

  default:
    return false; 
  }

op_15:     // FLTV
  function = (ins >> 5) & 0x7ff;
  
  switch(function) {
  case 0x0a5:
  case 0x4a5:
  case 0x0a6:
  case 0x4a6:
  case 0x0a7:
  case 0x4a7:
  case 0x03c:
  case 0x0bc:
  case 0x03e:
  case 0x0be:
    return true;

  default:
    if(function & 0x200)  return false; 

    switch(function & 0x7f) {
    case 0x000: 
    case 0x001: 
    case 0x002: 
    case 0x003: 
    case 0x01e: 
    case 0x020: 
    case 0x021: 
    case 0x022: 
    case 0x023: 
    case 0x02c: 
    case 0x02d: 
    case 0x02f: return true;
    default:  return false; 
    }
    break;
  }

op_16:    // FLTI
  function = (ins >> 5) & 0x7ff;  
  switch(function) {
  case 0x0a4:
  case 0x5a4:
  case 0x0a5:
  case 0x5a5:
  case 0x0a6:
  case 0x5a6:
  case 0x0a7:
  case 0x5a7:
  case 0x2ac:
  case 0x6ac:
    return true;

  default:
    if(((function & 0x600) == 0x200) || ((function & 0x500) == 0x400)) return false; 

    switch(function & 0x3f) {
    case 0x00: 
    case 0x01: 
    case 0x02: 
    case 0x03: 
    case 0x20: 
    case 0x21: 
    case 0x22: 
    case 0x23: 
    case 0x2c: 
    case 0x2f:  return true;
    case 0x3c:  if((function & 0x300) == 0x100){ return false;} return true;
    case 0x3e:  if((function & 0x300) == 0x100){ return false;} return true;
    default:    return false; 
    }
    break;
  }

op_17:     // FLTL
  function = (ins >> 5) & 0x7ff;
  switch(function) {
  case 0x010:
  case 0x020:
  case 0x021:
  case 0x022:
  case 0x024:
  case 0x025:
  case 0x02a:
  case 0x02b:
  case 0x02c:
  case 0x02d:
  case 0x02e:
  case 0x02f:
  case 0x030:
  case 0x130:
  case 0x530:
   return true;

  default:
   return false; 
  }

op_18:    // MISC

  function = (ins & 0xffff);
  switch(function) {
  case 0x0000: 
  case 0x0400: 
  case 0x4000: 
  case 0x4400: 
  case 0x8000: 
  case 0xA000: 
  case 0xC000: 
  case 0xE000: 
  case 0xE800: 
  case 0xF000: 
  case 0xF800: 
  case 0xFC00: return true;
  default:     return false; 
  }

op_1c:   // FPTI
  function = (ins >> 5) & 0x7f;
  
  switch(function) {
  case 0x00: 
  case 0x01: 
  case 0x30: 
  case 0x31: 
  case 0x32: 
  case 0x33: 
  case 0x34: 
  case 0x35: 
  case 0x36: 
  case 0x37: 
  case 0x38: 
  case 0x39: 
  case 0x3a: 
  case 0x3b: 
  case 0x3c: 
  case 0x3d: 
  case 0x3e: 
  case 0x3f: 
  case 0x70: 
  case 0x78:  return true;
  default:    return false; 
  }

  return false;
}
//////////////////////////// Models //////////////////////////////

// All of the models below take a Mixer as a parameter and write
// predictions to it.


template <class T, const U32 Size> class Cache {
  static_assert(Size>1 && (Size&(Size-1))==0, "Cache size must be a power of 2 bigger than 1");
private:
  Array<T> Data;
  U32 Index;
public:
  explicit Cache() : Data(Size) { Index=0; }
  T& operator()(U32 i) {
    return Data[(Index-i)&(Size-1)];
  }
  void operator++(int) {
    Index++;
  }
  void operator--(int) {
    Index--;
  }
  T& Next() {
    return Index++, *((T*)memset(&Data[Index&(Size-1)], 0, sizeof(T)));
  }
};
  
class Model {
public:
  virtual  int p(Mixer& m,int val1,int val2)=0;
  virtual  int inputs()=0;
};

class TextModel: public Model {
private:
  const U32 MIN_RECOGNIZED_WORDS = 4;
  U32 N;
   Buf& buffer;
  ContextMap Map;
  Array<Stemmer*> Stemmers;
  Array<Language*> Languages;
  Cache<Word, 8> Words[Language::Count];
  Cache<Segment1, 4> Segments;
  Cache<Sentence, 4> Sentences;
  Cache<Paragraph, 2> Paragraphs;
  Array<U32> WordPos;
  U32 BytePos[256];
  Word *cWord, *pWord; // current word, previous word
  Segment1 *cSegment; // current segment
  Sentence *cSentence; // current sentence
  Paragraph *cParagraph; // current paragraph  
  
  enum Parse {
    Unknown,
    ReadingWord,
    PossibleHyphenation,
    WasAbbreviation,
    AfterComma,
    AfterQuote,
    AfterAbbreviation,
    ExpectDigit
  } State, pState;
  struct {
    U32 Count[Language::Count-1]; // number of recognized words of each language in the last 64 words seen
    U64 Mask[Language::Count-1];  // binary mask with the recognition status of the last 64 words for each language
    int Id;  // current language detected
    int pId; // detected language of the previous word
  } Lang;
  struct {
    U64 numbers[6];   // last 2 numbers seen
    U32 numHashes[6]; // hashes of the last 2 numbers seen
    U8  numLength[6]; // digit length of last 2 numbers seen
    U32 numMask;      // binary mask of the results of the arithmetic comparisons between the numbers seen
    U32 numDiff;      // log2 of the consecutive differences between the last 16 numbers seen, clipped to 2 bits per difference
    U32 lastUpper;    // distance to last uppercase letter
    U32 maskUpper;    // binary mask of uppercase letters seen (in the last 32 bytes)
    U32 lastLetter;   // distance to last letter
    U32 lastDigit;    // distance to last digit
    U32 lastPunct;    // distance to last punctuation character
    U32 lastNewLine;  // distance to last new line character
    U32 prevNewLine;  // distance to penultimate new line character
    U32 wordGap;      // distance between the last words
    U32 spaces;       // binary mask of whitespace characters seen (in the last 32 bytes)
    U32 spaceCount;   // count of whitespace characters seen (in the last 32 bytes)
    U32 lastSpace;
    U32 commas;       // number of commas seen in this line (not this segment/sentence)
    U32 quoteLength;  // length (in words) of current quote
    U32 maskPunct;    // mask of relative position of last comma related to other punctuation
    U32 nestHash;     // hash representing current nesting state
    U32 lastNest;     // distance to last nesting character
    U32 nl;
    U32 nl1;
    U32 nl2;
    U32 masks[4],
        wordLength[4];
    int UTF8Remaining;// remaining bytes for current UTF8-encoded Unicode code point (-1 if invalid byte found)
    U8 firstLetter;   // first letter of current word
    U8 firstChar;     // first character of current line
    U8 expectedDigit; // next expected digit of detected numerical sequence
        U8 prevPunct;     // most recent punctuation character seen
    Word TopicDescriptor; // last word before ':'
    Word WikiHead0;
    Word WikiHead1;
    Word WikiHead2;
    Word WikiHead3;
     Word WikiHead4;
  } Info;
  U32 ParseCtx;
   bool doXML;
  void Update(Buf& buffer,Mixer& mixer);
  void SetContexts(Buf& buffer,Mixer& mixer);
public:
  TextModel(BlockData& bd, U64 Size) : N(21+3+1+1+5+5+1),buffer(bd.buf),  Map(CMlimit(MEM()*Size), N), Stemmers(Language::Count-1), Languages(Language::Count-1),
   WordPos(0x10000), State(Parse::Unknown), pState(State), Lang{ 0, 0, Language::Unknown, Language::Unknown }, Info{ 0 }, ParseCtx(0),doXML(false) {
    Stemmers[Language::English-1] = new EnglishStemmer();
    Stemmers[Language::French-1] = new FrenchStemmer();
    Stemmers[Language::German-1] = new GermanStemmer();
    Stemmers[Language::Estonian-1] = new EstonianStemmer();
    Languages[Language::English-1] = new English();
    Languages[Language::French-1] = new French();
    Languages[Language::German-1] = new German();
    Languages[Language::Estonian-1] = new Estonian();
    cWord = &Words[Lang.Id](0);
    pWord = &Words[Lang.Id](1);
    cSegment = &Segments(0);
    cSentence = &Sentences(0);
    cParagraph = &Paragraphs(0);
    memset(&BytePos[0], 0,  sizeof(BytePos));
    memset(&Info, 0, sizeof(Info));
 }
  virtual ~TextModel() {
    for (int i=0; i<Language::Count-1; i++) {
      delete Stemmers[i];
      delete Languages[i];
    }
  }
   int inputs() {return N*6;}
  int p(Mixer& mixer,int val1=0, int val2=0) {
    if (mixer.x.bpos==0) {
        if ((val1==0 || val1==1)&& doXML==true) doXML=false; // None ReadTag
        else if (val1==5) doXML=true;                        // ReadContent
      Update(buffer,mixer);
      SetContexts(buffer, mixer);
    }
    Map.mix(mixer);
    mixer.set(hash((Lang.Id!=Language::Unknown)?1+Stemmers[Lang.Id-1]->IsVowel(buffer(1)):0, Info.masks[1]&0xFF, mixer.x.c0)&0x3FF, 1024);
mixer.set(hash(ilog2(Info.wordLength[0]+1), mixer.x.c0,
      (Info.lastDigit<Info.wordLength[0]+Info.wordGap)|
      ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
      ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<2)|
      ((Info.lastUpper<Info.wordLength[0])<<3)
    )&0x7FF, 2048);
    mixer.set(hash(Info.masks[1]&0x3FF, Info.lastUpper<Info.wordLength[0], Info.lastUpper<Info.lastLetter+Info.wordLength[1])&0x7FF, 2048);
        mixer.set(hash(Info.spaces&0x1FF,
      (Info.lastUpper<Info.wordLength[0])|
      ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
      ((Info.lastPunct<Info.lastLetter)<<2)|
      ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<3)|
      ((Info.lastPunct<Info.lastLetter+Info.wordLength[1]+Info.wordGap)<<4)
    )&0x7FF, 2048);
   //  )&0x3FF, 1024);
    mixer.set(hash(Info.firstLetter*(Info.wordLength[0]<4), min(6, Info.wordLength[0]), mixer.x.c0)&0x7FF, 2048);
    mixer.set(hash((*pWord)[0], (*pWord)(0), min(4, Info.wordLength[0]), Info.lastPunct<Info.lastLetter)&0x7FF, 2048);
   mixer.set(hash(min(4, Info.wordLength[0]), mixer.x.c0,
      Info.lastUpper<Info.wordLength[0],
      (Info.nestHash>0)?Info.nestHash&0xFF:0x100|(Info.firstLetter*(Info.wordLength[0]>0 && Info.wordLength[0]<4))
    )&0xFFF, 4096);
    return 0;
  }
};

void TextModel::Update(Buf& buffer,Mixer& mixer) {
    
  Info.lastUpper  = min(0xFF, Info.lastUpper+1), Info.maskUpper<<=1;
  Info.lastLetter = min(0x1F, Info.lastLetter+1);
  Info.lastDigit  = min(0xFF, Info.lastDigit+1);
  Info.lastPunct  = min(0x3F, Info.lastPunct+1);
  Info.lastNewLine++, Info.prevNewLine++, Info.lastNest++;
  Info.spaceCount-=(Info.spaces>>31), Info.spaces<<=1;
  Info.masks[0]<<=2, Info.masks[1]<<=2, Info.masks[2]<<=4, Info.masks[3]<<=3;
  pState = State;  

  U8 c = buffer(1), pC=tolower(c);
  BytePos[c] = mixer.x.buf.pos;
  if (c!=pC) {
    c = pC;
    Info.lastUpper = 0, Info.maskUpper|=1;
  }
  pC = buffer(2);
  ParseCtx = hash(State=Parse::Unknown, pWord->Hash[1], c, (ilog2(Info.lastNewLine)+1)*(Info.lastNewLine*3>Info.prevNewLine), Info.masks[1]&0xFC);

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
    ParseCtx = hash(State=Parse::ReadingWord, cWord->Hash[1]);
  }
  else {
    if (cWord->Length()>0) {
      if (Lang.Id!=Language::Unknown)
        memcpy(&Words[Language::Unknown](0), cWord, sizeof(Word));

      for (int i=Language::Count-1; i>Language::Unknown; i--) {
        Lang.Count[i-1]-=(Lang.Mask[i-1]>>63), Lang.Mask[i-1]<<=1;
        if (i!=Lang.Id)
          memcpy(&Words[i](0), cWord, sizeof(Word));
        if (Stemmers[i-1]->Stem(&Words[i](0)))
          Lang.Count[i-1]++, Lang.Mask[i-1]|=1;
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
    #ifndef NDEBUG
      if (Lang.Id!=Lang.pId) {
        switch (Lang.Id) {
          case Language::Unknown: { printf("[Language: Unknown, blpos: %d]\n",mixer.x.blpos); break; };
          case Language::English: { printf("[Language: English, blpos: %d]\n",mixer.x.blpos); break; };
          case Language::French : { printf("[Language: French, blpos: %d]\n",mixer.x.blpos);  break; };
          case Language::German : { printf("[Language: German,  blpos: %d]\n",mixer.x.blpos); break; };
          case Language::Estonian : { printf("[Language: Estonian,  blpos: %d]\n",mixer.x.blpos); break; };
        }
      }
    #endif
      Lang.pId = Lang.Id;
      pWord = &Words[Lang.Id](1), cWord = &Words[Lang.Id](0);
      memset(cWord, 0, sizeof(Word));
      WordPos[pWord->Hash[1]&(WordPos.size()-1)] = mixer.x.buf.pos;
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
          ParseCtx = hash(State=Parse::WasAbbreviation, pWord->Hash[1]);
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
        Info.nl2=Info.nl1,Info.nl1=Info.nl, Info.nl=mixer.x.buf.pos;
        Info.prevNewLine = Info.lastNewLine, Info.lastNewLine = 0;
        Info.commas = 0;
        if (Info.prevNewLine==1 || (Info.prevNewLine==2 && (pC==CARRIAGE_RETURN || pC==5)))
          cParagraph = &Paragraphs.Next();
        else if ((Info.lastLetter==2 && pC=='+') || (Info.lastLetter==3 && pC==CARRIAGE_RETURN && buffer(3)=='+'))
          ParseCtx = hash(Parse::ReadingWord, pWord->Hash[1]), State=Parse::PossibleHyphenation;
      }
      case TAB: case CARRIAGE_RETURN: case SPACE: {
        Info.spaceCount++, Info.spaces|=1;
        Info.masks[1]+=3, Info.masks[3]+=5;
        if (c==SPACE && pState==Parse::WasAbbreviation) {
          ParseCtx = hash(State=Parse::AfterAbbreviation, pWord->Hash[1]);
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
  if (Info.lastNewLine==1)
    Info.firstChar = (Lang.Id!=Language::Unknown)?c:min(c,96);
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
  U16 w = ((State==Parse::ReadingWord)?cWord->Hash[1]:pWord->Hash[1])&0xFFFF;
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

  Map.set(hash(i++,min(10,ilog2((U32)Info.numbers[4])),Info.numHashes[4],Words[Lang.pId](2).Hash[3]));

  Map.set(hash(i++,min(10,ilog2((U32)Info.numbers[3])),Info.firstLetter,(cSentence->VerbIndex<cSentence->WordCount)?cSentence->lastVerb.Hash[1]:0));//

  Map.set(hash(i++,min(3,ilog2(cSegment->WordCount+1)),min(10,ilog2((U32)Info.numbers[4])),Words[Lang.pId](2).Hash[3]));

  Map.set(hash(i++, cWord->Hash[1]&0xFFF, Info.masks[1]&0x3FF, Words[Lang.pId](3).Hash[2],
    (Info.lastDigit<Info.wordLength[0]+Info.wordGap)|
    ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
    ((Info.spaces&0x7F)<<2)
  ));
  Map.set(hash(i++, cWord->Hash[1], pWord->Hash[3], Words[Lang.pId](2).Hash[3]));
  Map.set(hash(i++, h&0x7FFF, Words[Lang.pId](2).Hash[1]&0xFFF, Words[Lang.pId](3).Hash[1]&0xFFF));
  Map.set(hash(i++, cWord->Hash[1], c, (cSentence->VerbIndex<cSentence->WordCount)?cSentence->lastVerb.Hash[1]:0));
  Map.set(hash(i++, pWord->Hash[2], Info.masks[1]&0xFC, lc, Info.wordGap));
  Map.set(hash(i++, (Info.lastLetter==0)?cWord->Hash[1]:pWord->Hash[1], c, cSegment->FirstWord.Hash[2], min(3,ilog2(cSegment->WordCount+1))));
  Map.set(hash(i++, cWord->Hash[1], c, Segments(1).FirstWord.Hash[3]));
  Map.set(hash(i++, max(31,lc), Info.masks[1]&0xFFC, (Info.spaces&0xFE)|(Info.lastPunct<Info.lastLetter), (Info.maskUpper&0xFF)|(((0x100|Info.firstLetter)*(Info.wordLength[0]>1))<<8)));
  Map.set(hash(i++, column, min(7,ilog2(Info.lastUpper+1)), ilog2(Info.lastPunct+1)));
  Map.set(
    (column&0xF8)|(Info.masks[1]&3)|((Info.prevNewLine-Info.lastNewLine>63)<<2)|
    (min(3, Info.lastLetter)<<8)|
    (Info.firstChar<<10)|
    ((Info.commas>4)<<18)|
    ((m2>=1 && m2<=5)<<19)|
    ((m2>=6 && m2<=10)<<20)|
    ((m2==11 || m2==12)<<21)|
    ((Info.lastUpper<column)<<22)|
    ((Info.lastDigit<column)<<23)|
    ((column<Info.prevNewLine-Info.lastNewLine)<<24)
  );
  Map.set(hash(
    (2*column)/3,
    min(13, Info.lastPunct)+(Info.lastPunct>16)+(Info.lastPunct>32)+Info.maskPunct*16,
    ilog2(Info.lastUpper+1),
    ilog2(Info.prevNewLine-Info.lastNewLine),
    ((Info.masks[1]&3)==0)|
    ((m2<6)<<1)|
    ((m2<11)<<2)
  ));
   
  Map.set(hash(i++, column>>1, Info.spaces&0xF));
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
    ((Info.lastPunct<2+Info.wordLength[0]+Info.wordGap+Info.wordLength[1])<<5)
  ));
  Map.set(hash(i++,Info.numHashes[4],    min((max(Info.wordLength[0],3)-2)*(Info.wordLength[0]<8),3),(Info.lastLetter>0)?c:0x100));//
  
  Map.set(hash(i++, w, c, Info.numHashes[1]));
  Map.set(hash(i++, w, c, llog(mixer.x.buf.pos-WordPos[w])>>1));
  Map.set(hash(i++, w, c, Info.TopicDescriptor.Hash[1]&0x7FFF));
  
    if (doXML==true){
        Map.set(hash(i++, w, c, Info.WikiHead0.Hash[1]));// [[word]] ?
        Map.set(hash(i++, w, c, Info.WikiHead1.Hash[1]));//  ==word==
        Map.set(hash(i++, w, c, Info.WikiHead2.Hash[1]));// ''word''
        Map.set(hash(i++, w, c, Info.WikiHead3.Hash[1]));// }} - table
         Map.set(hash(i++, w, c, Info.WikiHead4.Hash[1]));// }} - table
    }else{
        Map.set(0), Map.set(0), Map.set(0), Map.set(0), Map.set(0); // 
    }
  Map.set(hash(i++, Info.numLength[0], c, Info.TopicDescriptor.Hash[1]&0x7FFF));
  Map.set(hash(i++, (Info.lastLetter>0)?c:0x100, Info.masks[1]&0xFFC, Info.nestHash&0x7FF));
        int above=mixer.x.buf[Info.nl1+column];  
        int above1=mixer.x.buf[Info.nl2+column]; 
  if(Info.wordLength[1]) Map.set(hash(i++,
    (column>0)|
    ((Info.wordLength[1]>0)<<1)|
    ((above==above1)<<2),
    above, c)); else Map.set(0);
           
  Map.set(hash(i++, w*17+c, Info.masks[3]&0x1FF,
    ((cSentence->VerbIndex==0 && cSentence->lastVerb.Length()>0)<<6)|
    ((Info.wordLength[1]>3)<<5)|
    ((cSegment->WordCount==0)<<4)|
    ((cSentence->SegmentCount==0 && cSentence->WordCount<2)<<3)|
    ((Info.lastPunct>=Info.lastLetter+Info.wordLength[1]+Info.wordGap)<<2)|
    ((Info.lastUpper<Info.lastLetter+Info.wordLength[1])<<1)|
    (Info.lastUpper<Info.wordLength[0]+Info.wordGap+Info.wordLength[1])
  ));
  Map.set(hash(i++, c, pWord->Hash[2], Info.firstLetter*(Info.wordLength[0]<6),
    ((Info.lastPunct<Info.wordLength[0]+Info.wordGap)<<1)|
    (Info.lastPunct>=Info.lastLetter+Info.wordLength[1]+Info.wordGap)
  ));
  Map.set(hash(i++, w*23+c, Words[Lang.pId](1+(Info.wordLength[0]==0)).Letters[Words[Lang.pId](1+(Info.wordLength[0]==0)).Start], Info.firstLetter*(Info.wordLength[0]<7)));
  Map.set(hash(i++, column, Info.spaces&7, Info.nestHash&0x7FF)); 
  Map.set(hash(i++, cWord->Hash[1], (Info.lastUpper<column)|((Info.lastUpper<Info.wordLength[0])<<1), min(5, Info.wordLength[0])));
}


//////////////////////////// matchModel ///////////////////////////

class matchModel1: public Model {
private:
    BlockData& x;
    Buf& buffer;
    const int Size;
  enum Parameters : U32{
    MaxLen = 0xFFFF,        // longest allowed match when not in bypass mode
    MinLen = 2,         // minimum required match
    DeltaLen = 5,       // minimum length to switch to delta mode
    MaxBypass = 0xFFFF, // absolute longest allowed match
    NumCtxs = 5,        // number of contexts used
    NumHashes = 2       // number of hashes used
  };
  Array<U32> Table;
  StateMap StateMaps[NumCtxs];
  SmallStationaryContextMap SCM[2];
  StationaryMap Map;
  U32 hashes[NumHashes];
  U32 ctx[NumCtxs];
  U32 length;    // length of match, or 0 if no match
  U32 index;     // points to next byte of match, if any
  U32 mask;
  bool delta;
  bool canBypass;
  void Update() {
    delta = false;
    if (length==0 && Bypass)
      Bypass = false; // can only quit bypass mode on byte boundary
    // update hashes
    hashes[0] = (hashes[0]*(3<<3) + buffer(1))&mask;
    hashes[1] = (hashes[1]*(5<<5) + buffer(1))&mask;
    // extend current match, if available
    if (length) {
      index++;
      if (length<MaxLen)
        length++;
    }
    // or find a new match
    else {
      for (U32 i=0; i<NumHashes && length<MinLen; i++){
        index = Table[hashes[i]];
        if (index && index!=(U32)buffer.pos && (U64)buffer.pos<(index+buffer.size())) {
          while (length<MaxLen && (index-length-1)!=(U32)buffer.pos && buffer(length+1)==buffer[index-length-1])
            length++;
        }
      }
    }
    for (U32 i=0; i<NumHashes; i++)
      Table[hashes[i]] = buffer.pos;
    SCM[0].set(buffer[index]);
    SCM[1].set(buffer.pos);
    Map.set((buffer[index]<<8)|buffer(1));
    x.Match.byte = (length)?buffer[index]:0;
  }
public:
  bool Bypass;
  U16 BypassPrediction; // prediction for bypass mode
  virtual ~matchModel1(){ }
  int inputs() {return  2+NumCtxs+2+2;}
  matchModel1(BlockData& bd, U32 val1=0) :
    x(bd),buffer(bd.buf),Size( CMlimit(MEM()*4)),
    Table(Size/sizeof(U32)),
    StateMaps{ 56<<8, 0x80800, 0x10100, 0x10100, 0x10100 },
    SCM{ {8,8},{8,8} },
    Map{ 16 },
    hashes{ 0 },
    ctx{ 0 },
    length(0),
    mask((Size-1)/sizeof(U32)),
    delta(false),
    canBypass(true),
    Bypass(false),
    BypassPrediction(2047)
  {
    assert((Size&(Size-1))==0);
  }
  int p(Mixer& mixer,int val1=0,int val2=0) {
    if (x.bpos==0)
      Update();
    int bit = (buffer[index]>>(7-x.bpos))&1;
    if (canBypass && Bypass) {
      if (length>0 && ((buffer[index]+256)>>(8-x.bpos))!=x.c0)
        length = 0;
    }
    else {
      ctx[0] = ctx[1] = ctx[2] = ctx[3] = ctx[4] = x.c0;
      ctx[2]+= (buffer(1)+1)<<8;
      if (!delta) {
        if (length) {
          if (buffer(1)==buffer[index-1] && x.c0==((buffer[index] + 256)>>(8-x.bpos))) { // bits match?
            if (length<16)
              ctx[0] = length*2 + bit;
            else
              ctx[0] = (min(length, 63)>>2)*2 + bit + 24;
            ctx[0] = (ctx[0]<<8) + buffer(1);
            ctx[1] = ((buffer[index]+1)<<11)|(x.bpos<<8)|buffer(1);
            mixer.add((min(length, 32)<<5)*(2*bit-1));
            mixer.add((ilog(length)<<2)*(2*bit-1));
          }
          else // we have a mismatch
          {
            delta = length>DeltaLen;
            length = 0;
            mixer.add32(0);
          }
        }
        else{        
          mixer.add32(0);
        }
      }
      else { //delta mode
        ctx[3] = ctx[2];
        ctx[4]+= (buffer[index]+1)<<8;
        mixer.add32(0);
      }
      for (U32 i=0; i<NumCtxs; i++)
        mixer.add(stretch(StateMaps[i].p(ctx[i],x.y))/2);
      SCM[0].mix(mixer, 7, 1, 4);
      SCM[1].mix(mixer, 5);
      Map.mix(mixer, 1, 4, 0xFF);
    }
    BypassPrediction = (length)?bit*0xFFF:0x7FF;
    x.Match.length = length;
    return length;
  }
};
//////////////////////////// wordModel /////////////////////////
#define bswap(x) \
+   ((((x) & 0xff000000) >> 24) | \
+    (((x) & 0x00ff0000) >>  8) | \
+    (((x) & 0x0000ff00) <<  8) | \
+    (((x) & 0x000000ff) << 24))
#define SPACE 0x20
// Model English text (words and columns/end of line)
class wordModel1: public Model {
   BlockData& x;
   Buf& buf;
    U32 word0, word1, word2, word3, word4, word5,word6;  // hashes
    U32 wrdhsh;
    U32 xword0,xword1,xword2,xword3,cword0,ccword;
    U32 number0, number1;  // hashes
    U32 text0,N,data0,type0;  // hash stream of letters
    ContextMap cm;
    int nl2,nl1, nl;  // previous, current newline position
    U32 mask,mask2;
    Array<int> wpos;  // last position of word
    int w;
    bool doXML;
    U32 lastLetter,firstLetter, lastUpper, lastDigit,wordGap;
    Array<Word> StemWords;
    Word *cWord, *pWord;
    EnglishStemmer StemmerEN;
    int StemIndex,same;
public:
  wordModel1( BlockData& bd,U32 val=0): x(bd),buf(bd.buf),word0(0),word1(0),word2(0),
  word3(0),word4(0),word5(0),word6(0),wrdhsh(0),xword0(0),xword1(0),xword2(0),xword3(0),cword0(0),ccword(0),number0(0),
  number1(0),text0(0),N(57+1+3+1+1-1),data0(0),type0(0),cm(CMlimit(MEM()*16), N),nl2(-4),nl1(-3), nl(-2),mask(0),mask2(0),wpos(0x10000),w(0),doXML(false),
  lastLetter(0),firstLetter(0), lastUpper(0),lastDigit(0), wordGap(0) ,StemWords(4),StemIndex(0),same(0){
      cWord=&StemWords[0], pWord=&StemWords[3];
   }
   int inputs() {return N*cm.inputs();}
   int p(Mixer& m,int val1=0,int val2=0)  {
 
    // Update word hashes
    if (x.bpos==0) {
        int c=x.c4&255,pC=(U8)(x.c4>>8),f=0;
        if (x.spaces&0x80000000) --x.spacecount;
        if (x.words&0x80000000) --x.wordcount;
        x.spaces=x.spaces*2;
        x.words=x.words*2;
        lastUpper=min(lastUpper+1,255);
        lastLetter=min(lastLetter+1,255);
        mask2<<=2;
        if (c>='A' && c<='Z') c+='a'-'A', lastUpper=0;
        if ((c>='a' && c<='z') || c=='\'' || c=='-')
           (*cWord)+=c;
        else if ((*cWord).Length()>0){
           StemmerEN.Stem(cWord);
           StemIndex=(StemIndex+1)&3;
           pWord=cWord;
           cWord=&StemWords[StemIndex];
           memset(cWord, 0, sizeof(Word));
        }
        if ((val1==0 || val1==1)&& doXML==true) doXML=false; // None ReadTag
        else if (val1==5) doXML=true;                        // ReadContent
        if ((c>='a' && c<='z') ||  (c>=128 &&(x.b3!=3) || (c>0 && c<4 ))) {
            if (!x.wordlen){
                // model syllabification with "+"  //book1 case +\n +\r\n
                if ((lastLetter=3 && (x.c4&0xFFFF00)==0x2B0A00 && buf(4)!=0x2B) || (lastLetter=4 && (x.c4&0xFFFFFF00)==0x2B0D0A00 && buf(5)!=0x2B) ||
                    (lastLetter=3 && (x.c4&0xFFFF00)==0x2D0A00 && buf(4)!=0x2D) || (lastLetter=4 && (x.c4&0xFFFFFF00)==0x2D0D0A00 && buf(5)!=0x2D)  ){
                    word0=word1;
                    word1=word2;
                    word2=word3;
                    word3=word4;
                    word4=word5;
                    word5=0;
                    x.wordlen = x.wordlen1;
                    if (c<128){
                       StemIndex=(StemIndex-1)&3;
                       cWord=pWord;
                       pWord=&StemWords[(StemIndex-1)&3];
                       memset(cWord, 0, sizeof(Word));
                       for (U32 i=0;i<=x.wordlen;i++)
                           (*cWord)+=tolower(buf(x.wordlen-i+1+2*(i!=x.wordlen)));
                    }
                }else{
                      wordGap = lastLetter;
                      firstLetter = c;
                      wrdhsh = 0;
                }   
            }
            lastLetter=0;
            ++x.words, ++x.wordcount;
             if (c>4  )word0^=hash(word0, c,0);
             
            text0=text0*997*16+c;
            x.wordlen++;
            x.wordlen=min(x.wordlen,45);
            f=0;
            w=word0&(wpos.size()-1);
            if ((c=='a' || c=='e' || c=='i' || c=='o' || c=='u') || (c=='y' && (x.wordlen>0 && pC!='a' && pC!='e' && pC!='i' && pC!='o' && pC!='u'))){
                mask2++;
                wrdhsh=wrdhsh*997*8+(c/4-22);
            }else if (c>='b' && c<='z'){
                mask2+=2;
                wrdhsh=wrdhsh*271*32+(c-97);
            }else
                wrdhsh=wrdhsh*11*32+c;
        } else {
            if (word0) { 
            type0 = (type0<<2)|1;
                word5=word4;
                word4=word3;
                word3=word2;
                word2=word1;
                word1=word0;
                x.wordlen1=x.wordlen;
                wpos[w]=x.blpos;
                if (c==':'|| c=='=') cword0=word0;
                //if (c==']'&& (x.frstchar!=':' || x.frstchar!='*')) xword0=word0;
                if (c==']'&& (x.frstchar!=':') && doXML==true) xword0=word0; // wiki 
                ccword=0;
                word0=x.wordlen=0;                
                if((c=='.'||c=='!'||c=='?' ||c=='}' ||c==')') && buf(2)!=10 && x.filetype!=EXE) f=1; 
            }
            
            if (c==SPACE || c==10 || c==5) { ++x.spaces, ++x.spacecount; if (c==10 || c==5) nl2=nl1,nl1=nl, nl=buf.pos-1;}
            else if (c=='.' || c=='!' || c=='?' || c==',' || c==';' || c==':') x.spafdo=0,ccword=c,mask2+=3;//,type0 = (type0<<2);
            else { ++x.spafdo; x.spafdo=min(63,x.spafdo); }
        }
        
        if ((x.c4&0xFFFF)==0x3D3D && x.frstchar==0x3d && doXML==true) xword1=word1;// ,xword2=word2; // == wiki
        if ((x.c4&0xFFFF)==0x2727 && doXML==true) xword2=word1 ;//,xword2=word2; // '' wiki
        if ((x.c4&0xFFFF)==0x7D7D && doXML==true) xword3=word1;       //}} wiki
        
        lastDigit=min(0xFF,lastDigit+1);
        if (c>='0' && c<='9') {
            if (buf(3)>='0' && buf(3)<='9' && (buf(2)=='.')&& number0==0) {number0=number1; number1=0;} // 0.4645
            number0^=hash(number0, c,1);
            lastDigit = 0;
        }
        else if (number0) {
            type0 = (type0<<2)|2;
            number1=number0;
            number0=0,ccword=0;
        }
        
        if (!((c>='a' && c<='z') ||(c>='0' && c<='9') || (c>=128 ))){
            data0^=hash(data0, c,1);
        }else if (data0) {
            type0 = (type0<<2)|3;
            data0=0;
            }
        
        x.col=min(255, buf.pos-nl);
        int above=buf[nl1+x.col];  // text column context, first
        int above1=buf[nl2+x.col]; // text column context, second
        if (val2) x.col=val2,above=buf[buf.pos-x.col],above1=buf[buf.pos-x.col*2];;
        if (x.col<=2) x.frstchar=(x.col==2?min(c,126):0);
        if (x.frstchar=='[' && c==32)    {if(buf(3)==']' || buf(4)==']' ) x.frstchar=96,xword0=0;}
        //256+ hash 513+ none
        if (x.filetype==DEFAULT || x.filetype==DECA|| x.filetype==EXE){
            for(int i=0;i<6;i++) cm.set(0);
        }
        else {
            cm.set(hash(513,x.spafdo, x.spaces,ccword));
            cm.set(hash(514,x.frstchar, c));
            cm.set(hash(515,x.col, x.frstchar, (lastUpper<x.col)*4+(mask2&3)));//?
            cm.set(hash(516,x.spaces, (x.words&255)));
        
            cm.set(x.spaces&0x7fff);
            cm.set(x.spaces&0xff);
        } 

        if (x.filetype==DEFAULT|| x.filetype==DECA|| x.filetype==EXE) {
            for(int i=0;i<5;i++) cm.set(0);
        }
        else{
            cm.set(hash(257,number0, word1,wordGap, word6));
            cm.set(hash(258,number1, c,ccword));  
            cm.set(hash(259,number0, number1,wordGap));  
            cm.set(hash(260,word0, number1, lastDigit<wordGap+x.wordlen));
            cm.set(hash(274,number0, cword0));
        }
        U32 h=x.wordcount*64+x.spacecount;
        if (x.filetype!=DECA){
            cm.set(hash(518,x.wordlen1,x.col));
            cm.set(hash(519,c,x.spacecount/2,wordGap));
            U32 h=x.wordcount*64+x.spacecount;
            cm.set(hash(520,c,h,ccword));
            cm.set(hash(517,x.frstchar,h,lastLetter));
            cm.set(hash(data0,word1, number1,type0&0xFFF));
            cm.set(h);
            cm.set(hash(521,h,x.spafdo)); 
            U32 d=x.c4&0xf0ff;
            cm.set(hash(522,d,x.frstchar,ccword));
        }else {
            for(int i=0;i<8;i++) cm.set(0);
        }
        h=word0*271;
        h=h+buf(1);
     
        cm.set(hash(262,h, 0));
        cm.set(hash( number0*271+buf(1), 0));
        cm.set(hash(263,word0, 0)); 
        if (wrdhsh) cm.set(hash(wrdhsh,buf(wpos[word1&(wpos.size()-1)]))); else cm.set(0);
        cm.set(hash(264,h, word1)); 
        cm.set(hash(265,word0, word1));
        cm.set(hash(266,h, word1,word2,lastUpper<x.wordlen));
        cm.set(hash(267,text0&0xffffff,0));
        cm.set(text0&0xfffff);
        if (doXML==true){
            cm.set(hash(269,word0,number0, xword0));
            cm.set(hash(270,word0,number0, xword1));  //wiki 
            cm.set(hash(271,word0,number0, xword2));  //
            cm.set(hash(268,word0,number0, xword3));  //
            cm.set(hash(272,x.frstchar, xword2));
        }else{
            for(int i=0;i<5;i++) cm.set(0);
        }
        cm.set(hash(273,word0, cword0));
        cm.set(hash(275,h, word2));
        cm.set(hash(276,h, word3));
        cm.set(hash(277,h, word4));
        cm.set(hash(278,h, word5));
        cm.set(hash(279,h, word1,word3));
        cm.set(hash(280,h, word2,word3));

        cm.set(buf(1)|buf(3)<<8|buf(5)<<16); 
        cm.set(buf(2)|buf(4)<<8|buf(6)<<16);  
        cm.set(buf(1)|buf(4)<<8|buf(7)<<16);  
        if (f) {
            word5=word4;//*29;
            word4=word3;//*31;
            word3=word2;//*37;
            word2=word1;//*41;
            word1='.';
        }
        if (x.col<((U32)max(255,val2))&& x.filetype!=DECA){
            cm.set(hash(523,x.col,buf(1),above));  
            cm.set(hash(524,buf(1),above,above^above1 )); 
            cm.set(hash(525,x.col,buf(1))); 
            cm.set(hash(526,x.col,c==32));  
        } 
        else {
          cm.set(0); cm.set(0); cm.set(0); cm.set(0); 
        }
 
        if (x.wordlen) cm.set(hash(281, word0, llog(x.blpos-wpos[word1&(wpos.size()-1)])>>4));    
        else cm.set(0);

        cm.set(hash(282,buf(1),llog(x.blpos-wpos[word1&(wpos.size()-1)])>>2));   
        cm.set(hash(283,buf(1),word0,llog(x.blpos-wpos[word2&(wpos.size()-1)])>>2));

        int fl = 0;
        if ((x.c4&0xff) != 0) {
            if (isalpha(x.c4&0xff)) fl = 1;
            else if (ispunct(x.c4&0xff)) fl = 2;
            else if (isspace(x.c4&0xff)) fl = 3;
            else if ((x.c4&0xff) == 0xff) fl = 4;
            else if ((x.c4&0xff) < 16) fl = 5;
            else if ((x.c4&0xff) < 64) fl = 6;
            else fl = 7;
        }
        mask = (mask<<3)|fl;
        if (x.filetype!=DEFAULT  ) { 
            cm.set(mask); 
            cm.set(hash(529,mask,buf(1))); 
            cm.set(hash(530,mask&0xff,x.col)); 
            cm.set(hash(531,mask,buf(2),buf(3))); 
            cm.set(hash(532,mask&0x1ff,x.f4&0x00fff0)); 
            cm.set(hash(h, llog(wordGap), mask&0x1FF, 
             ((x.wordlen1 > 3)<<6)|
             ((x.wordlen > 0)<<5)|
             ((x.spafdo == x.wordlen + 2)<<4)|
             ((x.spafdo == x.wordlen + x.wordlen1 + 3)<<3)|
             ((x.spafdo >= lastLetter + x.wordlen1 + wordGap)<<2)|
             ((lastUpper < lastLetter + x.wordlen1)<<1)|
             (lastUpper < x.wordlen + x.wordlen1 + wordGap)
             ),type0&0xFFF);
        } else {
            cm.set(mask); 
            cm.set(hash(529,mask,buf(1)));
            cm.set(0);cm.set(0);cm.set(0);cm.set(0);
        }
        
        if (x.wordlen1)    cm.set(hash(x.col,x.wordlen1,above,above1,x.c4&0xfF)); else cm.set(0); //wordlist
        if (wrdhsh)  cm.set(hash(mask2&0x3F, wrdhsh&0xFFF, (0x100|firstLetter)*(x.wordlen<6),(wordGap>4)*2+(x.wordlen1>5)) ); else cm.set(0);//?
        cm.set(hash((*pWord).Hash[2], h));
    }
    cm.mix(m);
    return 0;
}
 virtual ~wordModel1(){ }
};



//////////////////////////// recordModel ///////////////////////

// Model 2-D data with fixed record length.  Also order 1-2 models
// that include the distance to the last match.


inline U8 Clip(int const Px){
  if(Px>255)return 255;
  if(Px<0)return 0;
  return Px;
}

class recordModel1: public Model {
   BlockData& x;
   Buf& buf;
   Array<int> cpos1, cpos2, cpos3, cpos4;
   Array<int> wpos1; // buf(1..2) -> last position
   Array<int> rlen;//, rlen1, rlen2, rlen3,rlenl;  // run length and 2 candidates
   Array<int>  rcount;//, rcount2,rcount3;  // candidate counts
    U8 padding; // detected padding byte
   int prevTransition,nTransition  ; // position of the last padding transition
   int col;
   int mxCtx;
   bool MayBeImg24b;
   ContextMap cm, cn, co, cp;
   StationaryMap Map0, Map1;
public:
  recordModel1( BlockData& bd,U64 msize=CMlimit(MEM()*2) ): x(bd),buf(bd.buf),  cpos1(256) , cpos2(256),
    cpos3(256), cpos4(256),wpos1(0x10000), rlen(3), rcount(2),padding(0),prevTransition(0),nTransition(0), col(0),mxCtx(0),
    MayBeImg24b(false),cm(32768, 3), cn(32768/2, 3), co(32768*2, 3), cp(CMlimit(msize), 7),Map0(10), Map1(10) {
        // run length and 2 candidates
        rlen[0] = 2; 
        rlen[1] = 3; 
        rlen[2] = 4; 
        // candidate counts
        rcount[0] = 0;
        rcount[1] = 0; 
   }
  int inputs() {return (3+3+3+7)*cm.inputs()+2*2;}
  int p(Mixer& m,int rrlen=0,int val2=0) {
   // Find record length
  if (!x.bpos) {
   int w=x.c4&0xffff, c=w&255, d=w>>8;
    if (rrlen==0){
      int r=buf.pos-cpos1[c];
      if ( r>1) {
        if ( r==cpos1[c]-cpos2[c]  && r==cpos2[c]-cpos3[c] && (r>32 || r==cpos3[c]-cpos4[c])
          && (r>10 || ((c==buf(r*5+1)) && c==buf(r*6+1)))) {      
          if (r==rlen[1]) ++rcount[0];
          else if (r==rlen[2]) ++rcount[1];
          else if (rcount[0]>rcount[1]) rlen[2]=r, rcount[1]=1;
          else rlen[1]=r, rcount[0]=1;
        }
      }    

    // check candidate lengths
    for (int i=0; i < 2; i++) {
      if (rcount[i] > max(0,12-(int)ilog2(rlen[i+1]))){
        if (rlen[0] != rlen[i+1]){
            if (MayBeImg24b && rlen[i+1]==3){
              rcount[0]>>=1;
              rcount[1]>>=1;
              continue;
            }
            else if ( (rlen[i+1] > rlen[0]) && (rlen[i+1] % rlen[0] == 0) ){
            // maybe we found a multiple of the real record size..?
            // in that case, it is probably an immediate multiple (2x).
            // that is probably more likely the bigger the length, so
            // check for small lengths too
            if ((rlen[0] > 32) && (rlen[i+1] == rlen[0]*2)){
              rcount[0]>>=1;
              rcount[1]>>=1;
              continue;
            }
          }
          rlen[0] = rlen[i+1];
          rcount[i] = 0;
          MayBeImg24b = (rlen[0]>30 && (rlen[0]%3)==0);
          nTransition = 0;
        }
        else
          // we found the same length again, that's positive reinforcement that
          // this really is the correct record size, so give it a little boost
          rcount[i]>>=2;

        // if the other candidate record length is orders of
        // magnitude larger, it will probably never have enough time
        // to increase its counter before it's reset again. and if
        // this length is not a multiple of the other, than it might
        // really be worthwhile to investigate it, so we won't set its
        // counter to 0
        if (rlen[i+1]<<4 > rlen[1+(i^1)])
          rcount[i^1] = 0;
      }
    }
    }
    else rlen[0]=rrlen,rcount[0]=rcount[1]=0;
    
    // Set 2 dimensional contexts
    assert(rlen[0]>0);
    cm.set(c<<8| (min(255, buf.pos-cpos1[c])/4));
    cm.set(w<<9| llog(buf.pos-wpos1[w])>>2);
    cm.set(rlen[0]|buf(rlen[0])<<10|buf(rlen[0]*2)<<18);

    cn.set(w|rlen[0]<<8);
    cn.set(d|rlen[0]<<16);
    cn.set(c|rlen[0]<<8);

    co.set(buf(1)<<8|min(255, buf.pos-cpos1[buf(1)]));
    co.set(buf(1)<<17|buf(2)<<9|llog(buf.pos-wpos1[w])>>2);
    co.set(buf(1)<<8|buf(rlen[0]));

    col=buf.pos%rlen[0];
    if (!col)
      nTransition = 0;
      
    cp.set(hash(0,rlen[0]|buf(rlen[0])<<10|col<<18));
    cp.set(hash(1,rlen[0]|buf(1)<<10|col<<18));
    cp.set(hash(2,col|rlen[0]<<12));

    /*
    Consider record structures that include fixed-length strings.
    These usually contain the text followed by either spaces or 0's,
    depending on whether they're to be trimmed or they're null-terminated.
    That means we can guess the length of the string field by looking
    for small repetitions of one of these padding bytes followed by a
    different byte. By storing the last position where this transition
    ocurred, and what was the padding byte, we are able to model these
    runs of padding bytes.
    Special care is taken to skip record structures of less than 9 bytes,
    since those may be little-endian 64 bit integers. If they contain
    relatively low values (<2^40), we may consistently get runs of 3 or
    even more 0's at the end of each record, and so we could assume that
    to be the general case. But with integers, we can't be reasonably sure
    that a number won't have 3 or more 0's just before a final non-zero MSB.
    And with such simple structures, there's probably no need to be fancy
    anyway
    */

    if ((((U16)(x.c4>>8) == (U16)(SPACE*0x010101)) && (c != SPACE)) || (!(x.c4>>8) && c && ((padding != SPACE) || (buf.pos-prevTransition > rlen[0])))){
      prevTransition = buf.pos;
      nTransition+=(nTransition<31);
      padding = (U8)d;
    }
    if (rlen[0]>8){
      cp.set( hash( min(min(0xFF,rlen[0]),buf.pos-prevTransition), min(0x3FF,col), (w&0xF0F0)|(w==((padding<<8)|padding)), nTransition ) );
      cp.set( hash( w, (buf(rlen[0]+1)==padding && buf(rlen[0])==padding), col/max(1,rlen[0]/32) ) );
    }
    else
      cp.set(0), cp.set(0);
      
    int last4 = (buf(rlen[0]*4)<<24)|(buf(rlen[0]*3)<<16)|(buf(rlen[0]*2)<<8)|buf(rlen[0]);
    cp.set(hash(3, (last4&0xFF)|((last4&0xF000)>>4)|((last4&0xE00000)>>9)|((last4&0xE0000000)>>14)|((col/max(1,rlen[0]/16))<<18) ));
    cp.set(hash(4, (last4&0xF8F8)|(col<<16) ));
    
    int i=0x300;
    if (MayBeImg24b)
      i = (col%3)<<8, Map0.set(Clip(((U8)(x.c4>>16))+c-(x.c4>>24))|i);
    else
      Map0.set(Clip(c*2-d)|i);
    Map1.set(Clip(c+buf(rlen[0])-buf(rlen[0]+1))|i);
    // update last context positions
    cpos4[c]=cpos3[c];
    cpos3[c]=cpos2[c];
    cpos2[c]=cpos1[c];
    cpos1[c]=buf.pos;
    wpos1[w]=buf.pos;
    mxCtx = (rlen[0]>128)?(min(0x7F,col/max(1,rlen[0]/128))):col;
  }
  cm.mix(m);
  cn.mix(m);
  co.mix(m);
  cp.mix(m);
  
  if(x.filetype==AUDIO ||x.filetype==EXE || x.filetype==BINTEXT || x.filetype==DECA){
      //for (int i=0;i<4;i++)
      m.add64(0);
     // m.add32(0);
      m.set(0, 512 );
  }
  else{
      Map0.mix(m);
      Map1.mix(m);
      m.set( ((buf(rlen[0])^((U8)(x.c0<<(8-x.bpos))))>>4)|(min(0x1F,col/max(1,rlen[0]/32))<<4), 512 );
  }
  
  return (rlen[0]>2)*( (x.bpos<<7)|mxCtx );//1024 
  }
 virtual ~recordModel1(){ }
};

class recordModelx: public Model {
  BlockData& x;
  Buf& buf;
  Array<int> cpos1;
  Array<int> wpos1;// buf(1..2) -> last position
  Array<int> wpos2;// buf(1..3) -> last position
  ContextMap cm, cn, co,cp, cq;
public:
  recordModelx(BlockData& bd,U32 val=0):x(bd),buf(bd.buf),cpos1(256),wpos1(0x10000),wpos2(0x10000),
  cm(32768, 3-1), cn(32768/2, 4+1), co(32768*4, 4),cp(32768*2, 3), cq(32768*2, 3) {
  }
  int inputs() {return (2+1+5+4+3+3)*cm.inputs();}
  int p(Mixer& m,int val1=0,int val2=0){
   // Find record length
  if (!x.bpos) {
    int w=x.c4&0xffff, c=w&255, d=w&0xf0ff,e=x.c4&0xffffff,f=((e>>8)&0xff00)+c;
   
    cm.set(c<<8| (min(255, buf.pos-cpos1[c])/4));
    cm.set(w<<9| llog(buf.pos-wpos1[w])>>2);
   // cm.set(f<<10| llog(buf.pos-wpos2[f])>>2);
    cn.set(w);
    cn.set(d<<8);
    cn.set(c<<16);
    cn.set((x.f4&0xfffff)); 
    int col=buf.pos&3;
    cn.set(col|2<<12);

    co.set(c    );
    co.set(w<<8 );
    co.set(x.w5&0x3ffff);
    co.set(e<<3);
    
    cp.set(d    );
    cp.set(c<<8 );
    cp.set(w<<16);

    cq.set(w<<3 );
    cq.set(c<<19);
    cq.set(e);
    // update last context positions
    cpos1[c]=buf.pos;
    wpos1[w]=buf.pos;
   // wpos2[f]=buf.pos;
  }
  x.rm1=0;
  cm.mix(m);
  cn.mix(m);
  co.mix(m);
  cq.mix(m);
  cp.mix(m);
  x.rm1=1;
  return 0;
  }
 virtual ~recordModelx(){ }
};
//////////////////////////// sparseModel ///////////////////////

// Model order 1-2 contexts with gaps.
class sparseModely: public Model {
  BlockData& x;
  Buf& buf;
  ContextMap cm;
public:
  sparseModely(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), cm(CMlimit(MEM()*2), 42) {
  }
  int inputs() {return 42*cm.inputs();}
  int p(Mixer& m,int seenbefore,int howmany){//match order
  int j=0;
  if (x.bpos==0) {
    cm.set(hash(j++,seenbefore));
    cm.set(hash(j++,howmany));
    cm.set(hash(j++,buf(1)|buf(5)<<8));
    cm.set(hash(j++,buf(1)|buf(6)<<8));
    cm.set(hash(j++,buf(3)|buf(6)<<8));
    cm.set(hash(j++,buf(4)|buf(8)<<8));
    cm.set(hash(j++,buf(1)|buf(3)<<8|buf(5)<<16));
    cm.set(hash(j++,buf(2)|buf(4)<<8|buf(6)<<16));
    if (x.c4==0){
        for (int i=0; i<11; ++i) cm.set(0); 
    }else{
    cm.set(hash(j++,x.c4&0x00f0f0ff));
    cm.set(hash(j++,x.c4&0x00ff00ff));
    cm.set(hash(j++,x.c4&0xff0000ff));
    cm.set(hash(j++,x.c4&0x00f8f8f8));
    cm.set(hash(j++,x.c4&0xf8f8f8f8));
    cm.set(hash(j++,x.c4&0x00e0e0e0));
    cm.set(hash(j++,x.c4&0xe0e0e0e0));
    cm.set(hash(j++,x.c4&0x810000c1));
    cm.set(hash(j++,x.c4&0xC3CCC38C));
    cm.set(hash(j++,x.c4&0x0081CC81));
    cm.set(hash(j++,x.c4&0x00c10081));
    }
    cm.set(hash(j++,x.f4&0x00000fff));
    cm.set(hash(j++,x.f4));
    for (int i=1; i<8; ++i) {
      cm.set(hash(j++,seenbefore|buf(i)<<8)); 
      cm.set(hash(j++,(buf(i+2)<<8)|buf(i+1)));
      cm.set(hash(j++,(buf(i+3)<<8)|buf(i+1)));
    }
   
  }
  cm.mix(m);
  return 0;
}
virtual ~sparseModely(){ }
};
//////////////////////////// distanceModel ///////////////////////

// Model for modelling distances between symbols
class distanceModel1: public Model {
  ContextMap cr;
  int pos00,pos20,posnl;
  BlockData& x;
  Buf& buf;
public:
  distanceModel1(BlockData& bd): cr(CMlimit(MEM()), 3), pos00(0),pos20(0),posnl(0), x(bd),buf(bd.buf) {
    }
    int inputs() {return 3*cr.inputs();}
  int p(Mixer& m,int val1=0,int val2=0){
  if (x.bpos == 0) {
    int c=x.c4&0xff;
    if (c==0x00) pos00=x.buf.pos;
    if (c==0x20) pos20=x.buf.pos;
    if (c==0xff||c=='\r'||c=='\n') posnl=x.buf.pos;
    cr.set(min(llog(buf.pos-pos00),255)|(c<<8));
    cr.set(min(llog(buf.pos-pos20),255)|(c<<8));
    cr.set(min(llog(buf.pos-posnl),255)|((c<<8)+234567));
  }
  cr.mix(m);
  return 0;
}
virtual ~distanceModel1(){ }
};

inline U8 Clamp4(const int Px, const U8 n1, const U8 n2, const U8 n3, const U8 n4){
  int maximum=n1;if(maximum<n2)maximum=n2;if(maximum<n3)maximum=n3;if(maximum<n4)maximum=n4;
  int minimum=n1;if(minimum>n2)minimum=n2;if(minimum>n3)minimum=n3;if(minimum>n4)minimum=n4;
  if(Px<minimum)return minimum;
  if(Px>maximum)return maximum;
  return Px;
}
inline U8 LogMeanDiffQt(const U8 a, const U8 b, const U8 limit = 7){
  if (a==b) return 0;
  U8 sign=a>b?8:0;
  return sign | min(limit, ilog2((a+b)/max(2,abs(a-b)*2)+1));
}
inline U32 LogQt(const U8 Px, const U8 bits){
  return (U32(0x100|Px))>>max(0,(int)(ilog2(Px)-bits));
}
class RingBuffer {
  Array<U8> b;
  U32 offset;
public:
  RingBuffer(const int i=0): b(i), offset(0) {}
  void Fill(const U8 B) {
    memset(&b[0], B, b.size());
  }
  void Add(const U8 B){
    b[offset&(b.size()-1)] = B;
    offset++;
  }
  int operator()(const int i) const {
    return b[(offset-i)&(b.size()-1)];
  }
};

inline U8 Paeth(U8 W, U8 N, U8 NW){
  int p = W+N-NW;
  int pW=abs(p-(int)W), pN=abs(p-(int)N), pNW=abs(p-(int)NW);
  if (pW<=pN && pW<=pNW) return W;
  else if (pN<=pNW) return N;
  return NW;
}
//////////////////////////// im24bitModel /////////////////////////////////
// Model for 24-bit image data

#define NMAPS 94
#define nSCMaps 59

class im24bitModel1: public Model {

 int inpts;
 ContextMap cm;
 int col, color,stride;
 int ctx[2];
 int padding, x;
 int columns[2];
 int column[2];
 BlockData& xx;
 Buf& buf; 
 RingBuffer buffer;// internal rotating buffer for PNG unfiltered pixel data
 U8 WWW, WW, W,NWW, NW, N, NE, NEE, NNWW, NNW, NN, NNE, NNEE, NNN; //pixel neighborhood
 U8 px  ; // current PNG filter prediction
 int filter, w, line, isPNG,R1, R2;
 bool filterOn;
 U32& c4;
 int& c0;
 int& bpos;
 int lastWasPNG;
 U8 WWp1, Wp1, p1, NWp1, Np1, NEp1, NNp1 ;
 U8 WWp2, Wp2, p2, NWp2, Np2, NEp2, NNp2;
 U32 lastw,lastpos,curpos;
    SmallStationaryContextMap SCMap[nSCMaps] = { {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4}, {9,4},
                                                      {9,4}, {9,4}, {0,8}};
    StationaryMap Map[NMAPS] ={      8,      8,      8,      2,      0, {13,4}, {13,4}, {13,4}, {13,4}, {13,4},
                                     {15,4}, {15,4}, {15,4}, {15,4}, {11,4}, {11,4}, {11,4}, {11,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4}, { 9,4},
                                     { 9,4}, { 9,4}, { 9,4}, { 9,4}};
public:
  im24bitModel1(BlockData& bd): inpts(47),cm(CMlimit(MEM()*4), inpts), col(0) ,color(-1),stride(3), padding(0), x(0),xx(bd),
   buf(bd.buf), buffer(0x100000),WWW(0), WW(0), W(0),NWW(0),NW(0) ,N(0), NE(0), NEE(0), NNWW(0), NNW(0),
   NN(0), NNE(0), NNEE(0), NNN(0), px(0),filter(0),  w(0), line(0), isPNG(0),R1(0), R2(0),filterOn(false),
   c4(bd.c4),c0(bd.c0),bpos(bd.bpos),lastWasPNG(0), WWp1(0), Wp1(0), p1(0), NWp1(0),
   Np1(0), NEp1(0), NNp1(0),p2(0),lastw(0),lastpos(0),curpos(0){
  
    columns[0] = 1, columns[1]=1;
    column[0]=0,column[1]=0;
    ctx[0]=0,ctx[1]=0;
    }
   
  int inputs() {return inpts*cm.inputs()+nSCMaps*2+NMAPS*2+1;}
  int p(Mixer& m,int info,int val2=0){
  if (!bpos) {
    if (xx.blpos==1  ){
      const int alpha=xx.filetype==IMAGE32?1:xx.filetype==PNG32?1:0;
      stride = 3+alpha;
      lastpos=curpos;
      curpos=buf.pos;
      lastw=w;
      w = info&0xFFFFFF;
      
      isPNG =(xx.filetype==PNG24?1:xx.filetype==PNG32?1:0);
      padding = w%stride;
      
      x =1; color = line =0;
       filterOn = false;
      columns[0] = max(1,w/max(1,ilog2(w)*3));
      columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
      if ( lastWasPNG!=isPNG){
        for (int i=0;i<NMAPS;i++)
          Map[i].Reset();
      }
      lastWasPNG = isPNG;
      buffer.Fill(0x7F);
    }
    else{
      x++;
      if(x>=w+isPNG){x=0;line++;}
    }

    if (x==1 && isPNG)
      filter = (U8)c4;
    else{
      
      if (x+padding<w)
        color*=(++color)<stride;
      else
        color=(padding>0)*(stride+1);

      if (isPNG){
          U8 B = (U8)c4;
        switch (filter){
          case 1: {
            buffer.Add((U8)( B + buffer(stride)*(x>stride+1 || !x) ) );
            filterOn = x>stride;
            px = buffer(stride);
            break;
          }
          case 2: {
            buffer.Add((U8)( B + buffer(w)*(filterOn=(line>0)) ) );
            px = buffer(w);
            break;
          }
          case 3: {
            buffer.Add((U8)( B + (buffer(w)*(line>0) + buffer(stride)*(x>stride+1 || !x))/2 ) );
            filterOn = (x>stride || line>0);
            px = (buffer(stride)*(x>stride)+buffer(w)*(line>0))/2;
            break;
          }
          case 4: {
            buffer.Add((U8)( B + Paeth(buffer(stride)*(x>stride+1 || !x), buffer(w)*(line>0), buffer(w+stride)*(line>0 && (x>stride+1 || !x))) ) );
            filterOn = (x>stride || line>0);
            px = Paeth(buffer(stride)*(x>stride),buffer(w)*(line>0),buffer(w+stride)*(x>stride && line>0));
            break;
          }
          default: buffer.Add(B);
            filterOn = false;
            px = 0;
        }
         if(!filterOn)px=0;
      }
      else
        buffer.Add(c4 & 0xFF);
    }

    if (x || !isPNG){
      int i=color<<5;
      column[0]=(x-isPNG)/columns[0];
      column[1]=(x-isPNG)/columns[1];
      WWW=buffer(3*stride), WW=buffer(2*stride), W=buffer(stride), NWW=buffer(w+2*stride), NW=buffer(w+stride), N=buffer(w), NE=buffer(w-stride), NEE=buffer(w-2*stride), NNWW=buffer((w+stride)*2), NNW=buffer(w*2+stride), NN=buffer(w*2), NNE=buffer(w*2-stride), NNEE=buffer((w-stride)*2), NNN=buffer(w*3);
      WWp1=buffer(stride*2+1), Wp1=buffer(stride+1), p1=buffer(1), NWp1=buffer(w+stride+1), Np1=buffer(w+1), NEp1=buffer(w-stride+1), NNp1=buffer(w*2+1);
      WWp2=buffer(stride*2+2), Wp2=buffer(stride+2), p2=buffer(2), NWp2=buffer(w+stride+2), Np2=buffer(w+2), NEp2=buffer(w-stride+2), NNp2=buffer(w*2+2);

      if (!isPNG){
         
        int mean=W+NW+N+NE;
        const int var=(W*W+NW*NW+N*N+NE*NE-mean*mean/4)>>2;
        mean>>=2;
        const int logvar=ilog(var);

        cm.set(hash((N+1)>>1, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash((W+1)>>1, LogMeanDiffQt(W,Clip(WW*2-WWW))));
        cm.set(hash(Clamp4(W+N-NW,W,NW,N,NE), LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))));
        cm.set(hash((NNN+N+4)/8, Clip(N*3-NN*3+NNN)>>1 ));
        cm.set(hash((WWW+W+4)/8, Clip(W*3-WW*3+WWW)>>1 ));
        cm.set(hash(++i, (W+Clip(NE*3-NNE*3+buffer(w*3-stride)))/4, LogMeanDiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i, Clip((-buffer(4*stride)+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+buffer(w*3-stride)*4-buffer(w*4-stride),N,NE,buffer(w-2*stride),buffer(w-3*stride)))/5)/4));
        cm.set(hash(Clip(NEE+N-NNEE), LogMeanDiffQt(W,Clip(NW+NE-NNE))));
        cm.set(hash(Clip(NN+W-NNW), LogMeanDiffQt(W,Clip(NNW+WW-NNWW))));
        cm.set(hash(++i, p1));
        cm.set(hash(++i, p2));
        cm.set(hash(++i, Clip(W+N-NW)/2, Clip(W+p1-Wp1)/2));
        cm.set(hash(Clip(N*2-NN)/2, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(Clip(W*2-WW)/2, LogMeanDiffQt(W,Clip(WW*2-WWW))));
        cm.set(Clamp4(N*3-NN*3+NNN,W,NW,N,NE)/2);
        cm.set(Clamp4(W*3-WW*3+WWW,W,N,NE,NEE)/2);
        cm.set(hash(++i, LogMeanDiffQt(W,Wp1), Clamp4((p1*W)/(Wp1<1?1:Wp1),W,N,NE,NEE))); //using max(1,Wp1) results in division by zero in VC2015
        cm.set(hash(++i, Clamp4(N+p2-Np2,W,NW,N,NE)));
        cm.set(hash(++i, Clip(W+N-NW), column[0]));
        cm.set(hash(++i, Clip(N*2-NN), LogMeanDiffQt(W,Clip(NW*2-NNW))));
        cm.set(hash(++i, Clip(W*2-WW), LogMeanDiffQt(N,Clip(NW*2-NWW))));
        cm.set(hash( (W+NEE)/2, LogMeanDiffQt(W,(WW+NE)/2) ));
        cm.set(Clamp4(Clip(W*2-WW) + Clip(N*2-NN) - Clip(NW*2-NNWW), W, NW, N, NE));
        cm.set(hash(++i, W, p2 ));
        cm.set(hash( N, NN&0x1F, NNN&0x1F ));
        cm.set(hash( W, WW&0x1F, WWW&0x1F ));
        cm.set(hash(++i, N, column[0] ));
        cm.set(hash(++i, Clip(W+NEE-NE), LogMeanDiffQt(W,Clip(WW+NE-N))));
        cm.set(hash( NN, buffer(w*4)&0x1F, buffer(w*6)&0x1F, column[1] ));
        cm.set(hash( WW, buffer(stride*4)&0x1F, buffer(stride*6)&0x1F, column[1] ));
        cm.set(hash( NNN, buffer(w*6)&0x1F, buffer(w*9)&0x1F, column[1] ));
        cm.set(hash(++i, column[1]));
        
        cm.set(hash(++i, W, LogMeanDiffQt(W,WW)));
        cm.set(hash(++i, W, p1));
        cm.set(hash(++i, W/4, LogMeanDiffQt(W,p1), LogMeanDiffQt(W,p2) ));
        cm.set(hash(++i, N, LogMeanDiffQt(N,NN)));
        cm.set(hash(++i, N, p1));
        cm.set(hash(++i, N/4, LogMeanDiffQt(N,p1), LogMeanDiffQt(N,p2) ));
        cm.set(hash(++i, (W+N)>>3, p1>>4, p2>>4));
        cm.set(hash(++i, p1/2, p2/2));
        cm.set(hash(++i, W, p1-Wp1));
        cm.set(hash(++i, W+p1-Wp1));
        cm.set(hash(++i, N, p1-Np1));
        cm.set(hash(++i, N+p1-Np1));
        cm.set(hash(++i, buf(w*3-stride), buf(w*3-stride*2)));
        cm.set(hash(++i, buf(w*3+stride), buf(w*3+stride*2)));

        cm.set(hash(++i, mean, logvar>>4));

        ctx[0] = (min(color,stride-1)<<9)|((abs(W-N)>3)<<8)|((W>N)<<7)|((W>NW)<<6)|((abs(N-NW)>3)<<5)|((N>NW)<<4)|((abs(N-NE)>3)<<3)|((N>NE)<<2)|((W>WW)<<1)|(N>NN);
        ctx[1] = ((LogMeanDiffQt(p1,Clip(Np1+NEp1-buffer(w*2-stride+1)))>>1)<<5)|((LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))>>1)<<2)|min(color,stride-1);
      }
      else{
        i|=(filterOn)?((0x100|filter)<<8):0;
        int residuals[5] = { ((int8_t)buf(stride+(x<=stride)))+128,
                             ((int8_t)buf(1+(x<2)))+128,
                             ((int8_t)buf(stride+1+(x<=stride)))+128,
                             ((int8_t)buf(2+(x<3)))+128,
                             ((int8_t)buf(stride+2+(x<=stride)))+128
                           };
        R1 = (residuals[1]*residuals[0])/max(1,residuals[2]);
        R2 = (residuals[3]*residuals[0])/max(1,residuals[4]);
       
        cm.set(hash(++i, Clip(W+N-NW)-px, Clip(W+p1-Wp1)-px, R1));
        cm.set(hash(++i, Clip(W+N-NW)-px, LogMeanDiffQt(p1,Clip(Wp1+Np1-NWp1))));
        cm.set(hash(++i, Clip(W+N-NW)-px, LogMeanDiffQt(p2,Clip(buffer(stride+2)+buffer(w+2)-buffer(w+stride+2))), R2/4));
        cm.set(hash(++i, Clip(W+N-NW)-px, Clip(N+NE-NNE)-Clip(N+NW-NNW)));
        cm.set(hash(++i, Clip(W+N-NW+p1-(Wp1+Np1-NWp1)), px, R1 ));
        cm.set(hash(++i, Clamp4(W+N-NW,W,NW,N,NE)-px, column[0]));
        cm.set(hash(i>>8, Clamp4(W+N-NW,W,NW,N,NE)/8, px));
        cm.set(hash(++i, N-px, Clip(N+p1-Np1)-px));
        cm.set(hash(++i, Clip(W+p1-Wp1)-px, R1));
        cm.set(hash(++i, Clip(N+p1-Np1)-px));
        cm.set(hash(++i, Clip(N+p1-Np1)-px, Clip(N+p2-buffer(w+2))-px));
        cm.set(hash(++i, Clip(W+p1-Wp1)-px, Clip(W+p2-buffer(stride+2))-px));
        cm.set(hash(++i, Clip(NW+p1-NWp1)-px));
        cm.set(hash(++i, Clip(NW+p1-NWp1)-px, column[0]));
        cm.set(hash(++i, Clip(NE+p1-NEp1)-px, column[0]));
        cm.set(hash(++i, Clip(NE+N-NNE)-px, Clip(NE+p1-NEp1)-px));
        cm.set(hash(i>>8, Clip(N+NE-NNE)-px, column[0]));
        cm.set(hash(++i, Clip(NW+N-NNW)-px, Clip(NW+p1-NWp1)-px));
        cm.set(hash(i>>8, Clip(N+NW-NNW)-px, column[0]));
        cm.set(hash(i>>8, Clip(NN+W-NNW)-px, LogMeanDiffQt(N,Clip(NNN+NW-buffer(w*3+stride)))));
        cm.set(hash(i>>8, Clip(W+NEE-NE)-px, LogMeanDiffQt(W,Clip(WW+NE-N))));
        cm.set(hash(++i, Clip(N+NN-NNN+buffer(1+(!color))-Clip(buffer(w+1+(!color))+buffer(w*2+1+(!color))-buffer(w*3+1+(!color))))-px));
        cm.set(hash(i>>8, Clip(N+NN-NNN)-px, Clip( 5*N-10*NN+10*NNN-5*buffer(w*4)+buffer(w*5) )-px ));
        cm.set(hash(++i, Clip(N*2-NN)-px, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(++i, Clip(W*2-WW)-px, LogMeanDiffQt(W,Clip(WW*2-WWW))));
        cm.set(hash(i>>8, Clip(N*3-NN*3+NNN)-px));
        cm.set(hash(++i, Clip(N*3-NN*3+NNN)-px, LogMeanDiffQt(W,Clip(NW*2-NNW))));
        cm.set(hash(i>>8, Clip(W*3-WW*3+WWW)-px));
        cm.set(hash(++i, Clip(W*3-WW*3+WWW)-px, LogMeanDiffQt(N,Clip(NW*2-NWW))));
        cm.set(hash(i>>8, Clip( (35*N-35*NNN+21*buffer(w*5)-5*buffer(w*7))/16 )-px ));
        cm.set(hash(++i, (W+Clip(NE*3-NNE*3+buffer(w*3-stride)))/2-px, R2));
        cm.set(hash(++i, (W+Clamp4(NE*3-NNE*3+buffer(w*3-stride),W,N,NE,NEE))/2-px, LogMeanDiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i, (W+NEE)/2-px, R1/2));
        cm.set(hash(++i, Clamp4(Clip(W*2-WW)+Clip(N*2-NN)-Clip(NW*2-NNWW),W,NW,N,NE)-px));
        cm.set(hash(++i, buf(stride+(x<=stride)), buf(1+(x<2)), buf(2+(x<3))));
        cm.set(hash(++i, buf(1+(x<2)), px));
        cm.set(hash(i>>8, buf(w+1), buf((w+1)*2), buf((w+1)*3), px));                                                       
        cm.set(~0x5ca1ab1e);
        for (int j=0;j<9;j++)cm.set(0);

        ctx[0] = (min(color,stride-1)<<9)|((abs(W-N)>3)<<8)|((W>N)<<7)|((W>NW)<<6)|((abs(N-NW)>3)<<5)|((N>NW)<<4)|((N>NE)<<3)|min(5, filterOn?filter+1:0);
        ctx[1] = ((LogMeanDiffQt(p1,Clip(Np1+NEp1-buffer(w*2-stride+1)))>>1)<<5)|((LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))>>1)<<2)|min(color,stride-1);
      }
       i=0;
      Map[i++].set((W&0xC0)|((N&0xC0)>>2)|((WW&0xC0)>>4)|(NN>>6));
      Map[i++].set((N&0xC0)|((NN&0xC0)>>2)|((NE&0xC0)>>4)|(NEE>>6));
      Map[i++].set(buf(1+(isPNG && x<2)));
      Map[i++].set(min(color, stride-1));
      
        xx.Image.plane =  min(color, stride-1);
        xx.Image.pixels.W = W;
        xx.Image.pixels.N = N;
        xx.Image.pixels.NN = NN;
        xx.Image.pixels.WW = WW;
        xx.Image.pixels.Wp1 = Wp1;
        xx.Image.pixels.Np1 = Np1;
        xx.Image.ctx = ctx[0]>>3;
      
    }
  }
  if ((bpos==0 || bpos==4) && (x>0 || !isPNG)) {
    U8 B=(c0<<(8-bpos)), lsb=(bpos>0);
    int i=5;

    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*2+lsb)|(LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))<<9));
    Map[i++].set((((U8)(Clip(N*2-NN)-px-B))*2+lsb)|(LogMeanDiffQt(W,Clip(NW*2-NNW))<<9));
    Map[i++].set((((U8)(Clip(W*2-WW)-px-B))*2+lsb)|(LogMeanDiffQt(N,Clip(NW*2-NWW))<<9));
    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*2+lsb)|(LogMeanDiffQt(p1,Clip(Wp1+Np1-NWp1))<<9));
    Map[i++].set((((U8)(Clip(W+N-NW)-px-B))*2+lsb)|(LogMeanDiffQt(p2,Clip(Wp2+Np2-NWp2))<<9));
    Map[i++].set(hash(W-px-B,N-px-B)*2+lsb);
    Map[i++].set(hash(W-px-B,WW-px-B)*2+lsb);
    Map[i++].set(hash(N-px-B,NN-px-B)*2+lsb);
    Map[i++].set(hash(Clip(N+NE-NNE)-px-B, Clip(N+NW-NNW)-px-B)*2+lsb);
    Map[i++].set((min(color,stride-1)<<9)|(((U8)(Clip(N+p1-Np1)-px-B))*2+lsb));
    Map[i++].set((min(color,stride-1)<<9)|(((U8)(Clip(N+p2-Np2)-px-B))*2+lsb));
    Map[i++].set((min(color,stride-1)<<9)|(((U8)(Clip(W+p1-Wp1)-px-B))*2+lsb));
    Map[i++].set((min(color,stride-1)<<9)|(((U8)(Clip(W+p2-Wp2)-px-B))*2+lsb));
    Map[i++].set((Clamp4(N+p1-Np1,W,NW,N,NE)-px-B)*2+lsb);
    Map[i++].set((Clamp4(N+p2-Np2,W,NW,N,NE)-px-B)*2+lsb);

    Map[i++].set(((W+Clamp4(NE*3-NNE*3+buffer(w*3-stride),W,N,NE,NEE))/2-px-B)*2+lsb);
    Map[i++].set((Clamp4((W+Clip(NE*2-NNE))/2,W,NW,N,NE)-px-B)*2+lsb);
    Map[i++].set(((W+NEE)/2-px-B)*2+lsb);
    Map[i++].set((Clip((WWW-4*WW+6*W+Clip(NE*4-NNE*6+buffer(w*3-stride)*4-buffer(w*4-stride)))/4)-px-B)*2+lsb);
    Map[i++].set((Clip((-buffer(4*stride)+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+buffer(w*3-stride)*4-buffer(w*4-stride),N,NE,buffer(w-2*stride),buffer(w-3*stride)))/5)-px-B)*2+lsb);
    Map[i++].set((Clip((-4*WW+15*W+10*Clip(NE*3-NNE*3+buffer(w*3-stride))-Clip(buffer(w-3*stride)*3-buffer(w*2-3*stride)*3+buffer(w*3-3*stride)))/20)-px-B)*2+lsb);
    Map[i++].set((Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+buffer(w*3-stride*2),NE,NEE,buffer(w-3*stride),buffer(w-4*stride)))/6)-px-B)*2+lsb);
    Map[i++].set((Clip((W+Clip(NE*2-NNE))/2+p1-(Wp1+Clip(NEp1*2-buffer(w*2-stride+1)))/2)-px-B)*2+lsb);
    Map[i++].set((Clip((W+Clip(NE*2-NNE))/2+p2-(Wp2+Clip(NEp2*2-buffer(w*2-stride+2)))/2)-px-B)*2+lsb);
    Map[i++].set((Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6 +p1-(-3*WWp1+8*Wp1+Clip(buffer(w-stride*2+1)*2-buffer(w*2-stride*2+1)))/6)-px-B)*2+lsb);
    Map[i++].set((Clip((-3*WW+8*W+Clip(NEE*2-NNEE))/6 +p2-(-3*WWp2+8*Wp2+Clip(buffer(w-stride*2+2)*2-buffer(w*2-stride*2+2)))/6)-px-B)*2+lsb);
    Map[i++].set((Clip((W+NEE)/2+p1-(Wp1+buffer(w-stride*2+1))/2)-px-B)*2+lsb);
    Map[i++].set((Clip((W+NEE)/2+p2-(Wp2+buffer(w-stride*2+2))/2)-px-B)*2+lsb);
    Map[i++].set((Clip((WW+Clip(NEE*2-NNEE))/2+p1-(WWp1+Clip(buffer(w-stride*2+1)*2-buffer(w*2-stride*2+1)))/2)-px-B)*2+lsb);
    Map[i++].set((Clip((WW+Clip(NEE*2-NNEE))/2+p2-(WWp2+Clip(buffer(w-stride*2+2)*2-buffer(w*2-stride*2+2)))/2)-px-B)*2+lsb);
    Map[i++].set((Clip(WW+NEE-N+p1-Clip(WWp1+buffer(w-stride*2+1)-Np1))-px-B)*2+lsb);
    Map[i++].set((Clip(WW+NEE-N+p2-Clip(WWp2+buffer(w-stride*2+2)-Np2))-px-B)*2+lsb);

    Map[i++].set((Clip(W+N-NW)-px-B)*2+lsb);
    Map[i++].set((Clip(W+N-NW+p1-Clip(Wp1+Np1-NWp1))-px-B)*2+lsb);
    Map[i++].set((Clip(W+N-NW+p2-Clip(Wp2+Np2-NWp2))-px-B)*2+lsb);
    Map[i++].set((Clip(W+NE-N)-px-B)*2+lsb);
    Map[i++].set((Clip(N+NW-NNW)-px-B)*2+lsb);
    Map[i++].set((Clip(N+NW-NNW+p1-Clip(Np1+NWp1-buffer(w*2+stride+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(N+NW-NNW+p2-Clip(Np2+NWp2-buffer(w*2+stride+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(N+NE-NNE)-px-B)*2+lsb);
    Map[i++].set((Clip(N+NE-NNE+p1-Clip(Np1+NEp1-buffer(w*2-stride+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(N+NE-NNE+p2-Clip(Np2+NEp2-buffer(w*2-stride+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(N+NN-NNN)-px-B)*2+lsb);
    Map[i++].set((Clip(N+NN-NNN+p1-Clip(Np1+NNp1-buffer(w*3+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(N+NN-NNN+p2-Clip(Np2+NNp2-buffer(w*3+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(W+WW-WWW)-px-B)*2+lsb);
    Map[i++].set((Clip(W+WW-WWW+p1-Clip(Wp1+WWp1-buffer(stride*3+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(W+WW-WWW+p2-Clip(Wp2+WWp2-buffer(stride*3+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(W+NEE-NE)-px-B)*2+lsb);
    Map[i++].set((Clip(W+NEE-NE+p1-Clip(Wp1+buffer(w-stride*2+1)-NEp1))-px-B)*2+lsb);
    Map[i++].set((Clip(W+NEE-NE+p2-Clip(Wp2+buffer(w-stride*2+2)-NEp2))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+p1-NNp1)-px-B)*2+lsb);
    Map[i++].set((Clip(NN+p2-NNp2)-px-B)*2+lsb);
    Map[i++].set((Clip(NN+W-NNW)-px-B)*2+lsb);
    Map[i++].set((Clip(NN+W-NNW+p1-Clip(NNp1+Wp1-buffer(w*2+stride+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+W-NNW+p2-Clip(NNp2+Wp2-buffer(w*2+stride+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NW-buffer(w*3+stride))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NW-buffer(w*3+stride)+p1-Clip(NNp1+NWp1-buffer(w*3+stride+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NW-buffer(w*3+stride)+p2-Clip(NNp2+NWp2-buffer(w*3+stride+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NE-buffer(w*3-stride))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NE-buffer(w*3-stride)+p1-Clip(NNp1+NEp1-buffer(w*3-stride+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+NE-buffer(w*3-stride)+p2-Clip(NNp2+NEp2-buffer(w*3-stride+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+buffer(w*4)-buffer(w*6))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+buffer(w*4)-buffer(w*6)+p1-Clip(NNp1+buffer(w*4+1)-buffer(w*6+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(NN+buffer(w*4)-buffer(w*6)+p2-Clip(NNp2+buffer(w*4+2)-buffer(w*6+2)))-px-B)*2+lsb);
    Map[i++].set((Clip(WW+p1-WWp1)-px-B)*2+lsb);
    Map[i++].set((Clip(WW+p2-WWp2)-px-B)*2+lsb);
    Map[i++].set((Clip(WW+buffer(stride*4)-buffer(stride*6))-px-B)*2+lsb);
    Map[i++].set((Clip(WW+buffer(stride*4)-buffer(stride*6)+p1-Clip(WWp1+buffer(stride*4+1)-buffer(stride*6+1)))-px-B)*2+lsb);
    Map[i++].set((Clip(WW+buffer(stride*4)-buffer(stride*6)+p2-Clip(WWp2+buffer(stride*4+2)-buffer(stride*6+2)))-px-B)*2+lsb);

    Map[i++].set((Clip(N*2-NN+p1-Clip(Np1*2-NNp1))-px-B)*2+lsb);
    Map[i++].set((Clip(N*2-NN+p2-Clip(Np2*2-NNp2))-px-B)*2+lsb);
    Map[i++].set((Clip(W*2-WW+p1-Clip(Wp1*2-WWp1))-px-B)*2+lsb);
    Map[i++].set((Clip(W*2-WW+p2-Clip(Wp2*2-WWp2))-px-B)*2+lsb);
    Map[i++].set((Clip(N*3-NN*3+NNN)-px-B)*2+lsb);
    Map[i++].set((Clamp4(N*3-NN*3+NNN,W,NW,N,NE)-px-B)*2+lsb);
    Map[i++].set((Clamp4(W*3-WW*3+WWW,W,NW,N,NE)-px-B)*2+lsb);
    Map[i++].set((Clamp4(N*2-NN,W,NW,N,NE)-px-B)*2+lsb);
    Map[i++].set((Clip((buffer(w*5)-6*buffer(w*4)+15*NNN-20*NN+15*N+Clamp4(W*4-NWW*6+buffer(w*2+3*stride)*4-buffer(w*3+4*stride),W,NW,N,NN))/6)-px-B)*2+lsb);
    Map[i++].set((Clip((buffer(w*3-3*stride)-4*NNEE+6*NE+Clip(W*4-NW*6+NNW*4-buffer(w*3+stride)))/4)-px-B)*2+lsb);

    Map[i++].set((Clip(((N+3*NW)/4)*3-((NNW+NNWW)/2)*3+(buffer(w*3+2*stride)*3+buffer(w*3+3*stride))/4)-px-B)*2+lsb);
    Map[i++].set((Clip((W*2+NW)-(WW+2*NWW)+buffer(w+3*stride))-px-B)*2+lsb);
    Map[i++].set(((Clip(W*2-NW)+Clip(W*2-NWW)+N+NE)/4-px-B)*2+lsb);

    Map[i++].set((buffer(w*6)-px-B)*2+lsb);
    Map[i++].set(((buffer(w-4*stride)+buffer(w-6*stride))/2-px-B)*2+lsb);
    Map[i++].set(((buffer(stride*6)+buffer(stride*4))/2-px-B)*2+lsb);
    Map[i++].set((((W+N)*3-NW*2)/4-px-B)*2+lsb);
    Map[i++].set((N-px-B)*2+lsb);
    Map[i++].set((NN-px-B)*2+lsb);

    i=0;
    SCMap[i++].set((N+p1-Np1-px-B)*2+lsb);
    SCMap[i++].set((N+p2-Np2-px-B)*2+lsb);
    SCMap[i++].set((W+p1-Wp1-px-B)*2+lsb);
    SCMap[i++].set((W+p2-Wp2-px-B)*2+lsb);
    SCMap[i++].set((NW+p1-NWp1-px-B)*2+lsb);
    SCMap[i++].set((NW+p2-NWp2-px-B)*2+lsb);
    SCMap[i++].set((NE+p1-NEp1-px-B)*2+lsb);
    SCMap[i++].set((NE+p2-NEp2-px-B)*2+lsb);
    SCMap[i++].set((NN+p1-NNp1-px-B)*2+lsb);
    SCMap[i++].set((NN+p2-NNp2-px-B)*2+lsb);
    SCMap[i++].set((WW+p1-WWp1-px-B)*2+lsb);
    SCMap[i++].set((WW+p2-WWp2-px-B)*2+lsb);
    SCMap[i++].set((W+N-NW-px-B)*2+lsb);
    SCMap[i++].set((W+N-NW+p1-Wp1-Np1+NWp1-px-B)*2+lsb);
    SCMap[i++].set((W+N-NW+p2-Wp2-Np2+NWp2-px-B)*2+lsb);
    SCMap[i++].set((W+NE-N-px-B)*2+lsb);
    SCMap[i++].set((W+NE-N+p1-Wp1-NEp1+Np1-px-B)*2+lsb);
    SCMap[i++].set((W+NE-N+p2-Wp2-NEp2+Np2-px-B)*2+lsb);
    SCMap[i++].set((W+NEE-NE-px-B)*2+lsb);
    SCMap[i++].set((W+NEE-NE+p1-Wp1-buffer(w-stride*2+1)+NEp1-px-B)*2+lsb);
    SCMap[i++].set((W+NEE-NE+p2-Wp2-buffer(w-stride*2+2)+NEp2-px-B)*2+lsb);
    SCMap[i++].set((N+NN-NNN-px-B)*2+lsb);
    SCMap[i++].set((N+NN-NNN+p1-Np1-NNp1+buffer(w*3+1)-px-B)*2+lsb);
    SCMap[i++].set((N+NN-NNN+p2-Np2-NNp2+buffer(w*3+2)-px-B)*2+lsb);
    SCMap[i++].set((N+NE-NNE-px-B)*2+lsb);
    SCMap[i++].set((N+NE-NNE+p1-Np1-NEp1+buffer(w*2-stride+1)-px-B)*2+lsb);
    SCMap[i++].set((N+NE-NNE+p2-Np2-NEp2+buffer(w*2-stride+2)-px-B)*2+lsb);
    SCMap[i++].set((N+NW-NNW-px-B)*2+lsb);
    SCMap[i++].set((N+NW-NNW+p1-Np1-NWp1+buffer(w*2+stride+1)-px-B)*2+lsb);
    SCMap[i++].set((N+NW-NNW+p2-Np2-NWp2+buffer(w*2+stride+2)-px-B)*2+lsb);
    SCMap[i++].set((NE+NW-NN-px-B)*2+lsb);
    SCMap[i++].set((NE+NW-NN+p1-NEp1-NWp1+NNp1-px-B)*2+lsb);
    SCMap[i++].set((NE+NW-NN+p2-NEp2-NWp2+NNp2-px-B)*2+lsb);
    SCMap[i++].set((NW+W-NWW-px-B)*2+lsb);
    SCMap[i++].set((NW+W-NWW+p1-NWp1-Wp1+buffer(w+stride*2+1)-px-B)*2+lsb);
    SCMap[i++].set((NW+W-NWW+p2-NWp2-Wp2+buffer(w+stride*2+2)-px-B)*2+lsb);
    SCMap[i++].set((W*2-WW-px-B)*2+lsb);
    SCMap[i++].set((W*2-WW+p1-Wp1*2+WWp1-px-B)*2+lsb);
    SCMap[i++].set((W*2-WW+p2-Wp2*2+WWp2-px-B)*2+lsb);
    SCMap[i++].set((N*2-NN-px-B)*2+lsb);
    SCMap[i++].set((N*2-NN+p1-Np1*2+NNp1-px-B)*2+lsb);
    SCMap[i++].set((N*2-NN+p2-Np2*2+NNp2-px-B)*2+lsb);
    SCMap[i++].set((NW*2-NNWW-px-B)*2+lsb);
    SCMap[i++].set((NW*2-NNWW+p1-NWp1*2+buffer(w*2+stride*2+1)-px-B)*2+lsb);
    SCMap[i++].set((NW*2-NNWW+p2-NWp2*2+buffer(w*2+stride*2+2)-px-B)*2+lsb);
    SCMap[i++].set((NE*2-NNEE-px-B)*2+lsb);
    SCMap[i++].set((NE*2-NNEE+p1-NEp1*2+buffer(w*2-stride*2+1)-px-B)*2+lsb);
    SCMap[i++].set((NE*2-NNEE+p2-NEp2*2+buffer(w*2-stride*2+2)-px-B)*2+lsb);
    SCMap[i++].set((N*3-NN*3+NNN+p1-Np1*3+NNp1*3-buffer(w*3+1)-px-B)*2+lsb);
    SCMap[i++].set((N*3-NN*3+NNN+p2-Np2*3+NNp2*3-buffer(w*3+2)-px-B)*2+lsb);
    SCMap[i++].set((N*3-NN*3+NNN-px-B)*2+lsb);
    SCMap[i++].set(((W+NE*2-NNE)/2-px-B)*2+lsb);
    SCMap[i++].set(((W+NE*3-NNE*3+buffer(w*3-stride))/2-px-B)*2+lsb);
    SCMap[i++].set(((W+NE*2-NNE)/2+p1-(Wp1+NEp1*2-buffer(w*2-stride+1))/2-px-B)*2+lsb);
    SCMap[i++].set(((W+NE*2-NNE)/2+p2-(Wp2+NEp2*2-buffer(w*2-stride+2))/2-px-B)*2+lsb);
    SCMap[i++].set((NNE+NE-buffer(w*3-stride)-px-B)*2+lsb);
    SCMap[i++].set((NNE+W-NN-px-B)*2+lsb);
    SCMap[i++].set((NNW+W-NNWW-px-B)*2+lsb);

  }

  // Predict next bit
  if (x || !isPNG){
    cm.mix(m);
    for (int i=0;i<NMAPS;i++)
      Map[i].mix(m);
    for (int i=0;i<nSCMaps;i++)
      SCMap[i].mix(m,9);

    m.add(0);
    if (++col>=stride*8) col=0;
    m.set(5, 6);
    m.set(min(63,column[0])+((ctx[0]>>3)&0xC0), 256);
    m.set(min(127,column[1])+((ctx[0]>>2)&0x180), 512);
    m.set((ctx[0]&0x7FC)|(bpos>>1), 2048);
    m.set(col+(isPNG?(ctx[0]&7)+1:(c0==((0x100|((N+W)/2))>>(8-bpos))))*32, 8*32);
    m.set(((isPNG?p1:0)>>4)*stride+(x%stride) + min(5,filterOn?filter+1:0)*64, 6*64);
    m.set(c0+256*(isPNG && abs(R1-128)>8), 256*2);
    m.set((ctx[1]<<2)|(bpos>>1), 1024);
    m.set(hash(LogMeanDiffQt(W,WW,5), LogMeanDiffQt(N,NN,5), LogMeanDiffQt(W,N,5), ilog2(W), color)&0x1FFF, 8192);
    m.set(hash(ctx[0], column[0]/8)&0x1FFF, 8192);
    m.set(hash(LogQt(N,5), LogMeanDiffQt(N,NN,3), c0)&0x1FFF, 8192);
    m.set(hash(LogQt(W,5), LogMeanDiffQt(W,WW,3), c0)&0x1FFF, 8192);
  }
  else{
    for (int j=0;j<nSCMaps;j++)m.add(0);
    m.add( -2048+((filter>>(7-bpos))&1)*4096 );
    m.set(min(4,filter),6);
  /*  m.set(0, 256);
    m.set(0, 512);
    m.set(0, 2048);
    m.set(0, 8*32);
    m.set(0, 6*64);
    m.set(0, 256*2);
    m.set(0, 1024);
    m.set(0, 8192);
    m.set(0, 8192);
    m.set(0, 8192);
    m.set(0, 8192); */
  }
  return 0;
}

  // Square buf(i)
inline int sqrbuf(int i) {
  assert(i>0);
  return buf(i)*buf(i);
}
  virtual ~im24bitModel1(){ }
 
};

//////////////////////////// im8bitModel /////////////////////////////////
 
// Model for 8-bit image data

 
class im8bitModel1: public Model {
 int inpts;
 ContextMap cm;
 int col;
 BlockData& xx;
 Buf& buf;
 int ctx,  lastWasPNG, line, x, filter, gray,isPNG,jump;// columns, column,
 
 U32& c4;
 int& c0;
 int& bpos;
 const int nMaps  ;
 StationaryMap Map[57] = { 12, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       8, 8, 8, 8, 8, 8, 8, 8,
                                       0 };
 U8 px , res;// current PNG filter prediction, expected residual
 RingBuffer buffer;// internal rotating buffer for PNG unfiltered pixel data
 U8 WWW, WW, W, NWW, NW, N, NE, NEE, NNWW, NNW, NN, NNE, NNEE, NNN;//pixel neighborhood
 bool filterOn;
 int columns[2]   , column[2];
 Array<short> jumps;
public:
  im8bitModel1( BlockData& bd):  inpts(49),cm(CMlimit(MEM()*16), inpts),col(0),xx(bd),buf(bd.buf), ctx(0), lastWasPNG(0),line(0), x(0),
   filter(0),gray(0),isPNG(0),jump(0), c4(bd.c4),c0(bd.c0),bpos(bd.bpos), nMaps(57),/*Map(nMaps),*/ px (0),
   res (0),buffer(0x100000),WWW(0),WW(0),W(0),NWW(0),NW(0),N(0),NE(0),NEE(0),NNWW(0),NNW(0),
   NN(0),NNE(0),NNEE(0),NNN(0),filterOn(false),jumps(0x8000){
  
   //for(int i=0;i<nMaps;i++)  
   //    Map[i] =new StationaryMap(im8Bcxt[i]);
   columns[0] =1;
   columns[1] =1;
   column[0] =1;
   column[1] =1;
  }
  int inputs() {return inpts*cm.inputs()+nMaps*2;}
int p(Mixer& m,int w,int val2=0){
  assert(w>3); 
  if (!bpos) {
    if (xx.blpos==1){
      isPNG=  (xx.filetype==PNG8?1:xx.filetype==PNG8GRAY?1:0);
      gray=xx.filetype==PNG8GRAY?1:xx.filetype==IMAGE8GRAY?1:0;
      x =1; line = jump =  px= 0;
      filterOn = false;
      columns[0] = max(1,w/max(1,ilog2(w)*2));
      columns[1] = max(1,columns[0]/max(1,ilog2(columns[0])));
      if (gray){
        if (lastWasPNG!=isPNG){
          for (int i=0;i<nMaps;i++)
            Map[i].Reset();
        }
        lastWasPNG = isPNG;
      }
      buffer.Fill(0x7F);
    }
    else{
      x++;
      if(x>=w+isPNG){x=0;line++;}
    }

    if (isPNG){
      if (x==1)
        filter = (U8)c4;
      else{
        U8 B = (U8)c4;

        switch (filter){
          case 1: {
            buffer.Add((U8)( B + buffer(1)*(x>2 || !x) ) );
            filterOn = x>1;
            px = buffer(1);
            break;
          }
          case 2: {
            buffer.Add((U8)( B + buffer(w)*(filterOn=(line>0)) ) );
            px = buffer(w);
            break;
          }
          case 3: {
            buffer.Add((U8)( B + (buffer(w)*(line>0) + buffer(1)*(x>2 || !x))/2 ) );
            filterOn = (x>1 || line>0);
            px = (buffer(1)*(x>1)+buffer(w)*(line>0))/2;
            break;
          }
          case 4: {
            buffer.Add((U8)( B + Paeth(buffer(1)*(x>2 || !x), buffer(w)*(line>0), buffer(w+1)*(line>0 && (x>2 || !x))) ) );
            filterOn = (x>1 || line>0);
            px = Paeth(buffer(1)*(x>1),buffer(w)*(line>0),buffer(w+1)*(x>1 && line>0));
            break;
          }
          default: buffer.Add(B);
            filterOn = false;
            px = 0;
        }
        if(!filterOn)px=0;
      }
    }  
    else {
      buffer.Add((U8)c4);
      if (x==0) {
        memset(&jumps[0], 0, sizeof(short)*jumps.size());
        if (line>0 && w>8) {
          U8 bMask = 0xFF-((1<<gray)-1);
          U32 pMask = bMask*0x01010101u;
          U32 left=0, right=0;
          int l=min(w, jumps.size()), end=l-4;
          do {
            left = ((buffer(l-x)<<24)|(buffer(l-x-1)<<16)|(buffer(l-x-2)<<8)|buffer(l-x-3))&pMask;
            int i = end;
            while (i>=x+4) {
              right = ((buffer(l-i-3)<<24)|(buffer(l-i-2)<<16)|(buffer(l-i-1)<<8)|buffer(l-i))&pMask;
              if (left==right) {
                int j=(i+3-x-1)/2, k=0;
                for (; k<=j; k++) {
                  if (k<4 || (buffer(l-x-k)&bMask)==(buffer(l-i-3+k)&bMask)) {
                    jumps[x+k] = -(x+(l-i-3)+2*k);
                    jumps[i+3-k] = i+3-x-2*k;
                  }
                  else
                    break;
                }
                x+=k;
                end-=k;
                break;
              }
              i--;
            }
            x++;
            if (x>end)
              break;
          } while (x+4<l);
          x = 0;
        }
      }
    }

    if (x || !isPNG){
      int i=filterOn ? (filter+1)<<6 : 0;
      column[0]=(x-isPNG)/columns[0];
      column[1]=(x-isPNG)/columns[1];
      WWW=buffer(3), WW=buffer(2), W=buffer(1), NWW=buffer(w+2), NW=buffer(w+1), N=buffer(w), NE=buffer(w-1), NEE=buffer(w-2), NNWW=buffer(w*2+2), NNW=buffer(w*2+1), NN=buffer(w*2), NNE=buffer(w*2-1), NNEE=buffer(w*2-2), NNN=buffer(w*3);
      jump = jumps[min(x,jumps.size()-1)];
      cm.set(hash(++i, (jump!=0)?(0x100|buffer(abs(jump)))*(1-2*(jump<0)):N, line&3));
      if (!gray){
        cm.set(hash(++i, W, px));
        cm.set(hash(++i, W, px, column[0]));
        cm.set(hash(++i, N, px));
        cm.set(hash(++i, N, px, column[0]));
        cm.set(hash(++i, NW, px));
        cm.set(hash(++i, NW, px, column[0]));
        cm.set(hash(++i, NE, px));
        cm.set(hash(++i, NE, px, column[0]));
        cm.set(hash(++i, NWW, px));
        cm.set(hash(++i, NEE, px));
        cm.set(hash(++i, WW, px));
        cm.set(hash(++i, NN, px));
        cm.set(hash(++i, W, N, px));
        cm.set(hash(++i, W, NW, px));
        cm.set(hash(++i, W, NE, px));
        cm.set(hash(++i, W, NEE, px));
        cm.set(hash(++i, W, NWW, px));
        cm.set(hash(++i, N, NW, px));
        cm.set(hash(++i, N, NE, px));
        cm.set(hash(++i, NW, NE, px));
        cm.set(hash(++i, W, WW, px));
        cm.set(hash(++i, N, NN, px));
        cm.set(hash(++i, NW, NNWW, px));
        cm.set(hash(++i, NE, NNEE, px));
        cm.set(hash(++i, NW, NWW, px));
        cm.set(hash(++i, NW, NNW, px));
        cm.set(hash(++i, NE, NEE, px));
        cm.set(hash(++i, NE, NNE, px));
        cm.set(hash(++i, N, NNW, px));
        cm.set(hash(++i, N, NNE, px));
        cm.set(hash(++i, N, NNN, px));
        cm.set(hash(++i, W, WWW, px));
        cm.set(hash(++i, WW, NEE, px));
        cm.set(hash(++i, WW, NN, px));
        cm.set(hash(++i, W, buffer(w-3), px));
        cm.set(hash(++i, W, buffer(w-4), px));
        cm.set(hash(++i, W, hash(N,NW)&0x7FF, px));
        cm.set(hash(++i, N, hash(NN,NNN)&0x7FF, px));
        cm.set(hash(++i, W, hash(NE,NEE)&0x7FF, px));
        cm.set(hash(++i, W, hash(NW,N,NE)&0x7FF, px));
        cm.set(hash(++i, N, hash(NE,NN,NNE)&0x7FF, px));
        cm.set(hash(++i, N, hash(NW,NNW,NN)&0x7FF, px));
        cm.set(hash(++i, W, hash(WW,NWW,NW)&0x7FF, px));
        cm.set(hash(++i, W, hash(NW,N)&0xFF, hash(WW,NWW)&0xFF, px));
        cm.set(hash(++i, px, column[0]));
        cm.set(hash(++i, px));
        cm.set(hash(++i, N, px, column[1] ));
        cm.set(hash(++i, W, px, column[1] ));

        ctx = min(0x1F,(x-1)/min(0x20,columns[0]));
        res = W;
      }
      else{
        cm.set(hash(++i, N, px));
        cm.set(hash(++i, N-px));
        cm.set(hash(++i, W, px));
        cm.set(hash(++i, NW, px));
        cm.set(hash(++i, NE, px));
        cm.set(hash(++i, N, NN, px));
        cm.set(hash(++i, W, WW, px));
        cm.set(hash(++i, NE, NNEE, px ));
        cm.set(hash(++i, NW, NNWW, px ));
        cm.set(hash(++i, W, NEE, px));
        cm.set(hash(++i, (Clamp4(W+N-NW,W,NW,N,NE)-px)/2, LogMeanDiffQt(Clip(N+NE-NNE), Clip(N+NW-NNW))));
        cm.set(hash(++i, (W-px)/4, (NE-px)/4, column[0]));
        cm.set(hash(++i, (Clip(W*2-WW)-px)/4, (Clip(N*2-NN)-px)/4));
        cm.set(hash(++i, (Clamp4(N+NE-NNE,W,N,NE,NEE)-px)/4, column[0]));
        cm.set(hash(++i, (Clamp4(N+NW-NNW,W,NW,N,NE)-px)/4, column[0]));
        cm.set(hash(++i, (W+NEE)/4, px, column[0]));
        cm.set(hash(++i, Clip(W+N-NW)-px, column[0]));
        cm.set(hash(++i, Clamp4(N*3-NN*3+NNN,W,N,NN,NE), px, LogMeanDiffQt(W,Clip(NW*2-NNW))));
        cm.set(hash(++i, Clamp4(W*3-WW*3+WWW,W,N,NE,NEE), px, LogMeanDiffQt(N,Clip(NW*2-NWW))));
        cm.set(hash(++i, (W+Clamp4(NE*3-NNE*3+(isPNG?buffer(w*3-1):buf(w*3-1)),W,N,NE,NEE))/2, px, LogMeanDiffQt(N,(NW+NE)/2)));
        cm.set(hash(++i, (N+NNN)/8, Clip(N*3-NN*3+NNN)/4, px));
        cm.set(hash(++i, (W+WWW)/8, Clip(W*3-WW*3+WWW)/4, px));
        cm.set(hash(++i, Clip((-buffer(4)+5*WWW-10*WW+10*W+Clamp4(NE*4-NNE*6+buffer(w*3-1)*4-buffer(w*4-1),N,NE,buffer(w-2),buffer(w-3)))/5)-px));
        cm.set(hash(++i, Clip(N*2-NN)-px, LogMeanDiffQt(N,Clip(NN*2-NNN))));
        cm.set(hash(++i, Clip(W*2-WW)-px, LogMeanDiffQt(NE,Clip(N*2-NW))));
         //cm.set(hash(++i, buf(w*6-1), buf(w*3-1*2),px)); //best for artifical
       // cm.set(hash(++i, buf(w*6+1), buf(w*3+1*2),px));
     
        cm.set(~0xde7ec7ed);

        i=0;
        Map[i++].set(((U8)(Clip(W+N-NW)-px))|(LogMeanDiffQt(Clip(N+NE-NNE),Clip(N+NW-NNW))<<8));
        Map[i++].set(Clamp4(W+N-NW,W,NW,N,NE)-px);
        Map[i++].set(Clip(W+N-NW)-px);
        Map[i++].set(Clamp4(W+NE-N,W,NW,N,NE)-px);
        Map[i++].set(Clip(W+NE-N)-px);
        Map[i++].set(Clamp4(N+NW-NNW,W,NW,N,NE)-px);
        Map[i++].set(Clip(N+NW-NNW)-px);
        Map[i++].set(Clamp4(N+NE-NNE,W,N,NE,NEE)-px);
        Map[i++].set(Clip(N+NE-NNE)-px);
        Map[i++].set((W+NEE)/2-px);
        Map[i++].set(Clip(N*3-NN*3+NNN)-px);
        Map[i++].set(Clip(W*3-WW*3+WWW)-px);
        Map[i++].set((W+Clip(NE*3-NNE*3+buffer(w*3-1)))/2-px);
        Map[i++].set((W+Clip(NEE*3-buffer(w*2-3)*3+buffer(w*3-4)))/2-px);
        Map[i++].set(Clip(NN+buffer(w*4)-buffer(w*6))-px);
        Map[i++].set(Clip(WW+buffer(4)-buffer(6))-px);
        Map[i++].set(Clip((buffer(w*5)-6*buffer(w*4)+15*NNN-20*NN+15*N+Clamp4(W*2-NWW,W,NW,N,NN))/6)-px);
        Map[i++].set(Clip((-3*WW+8*W+Clamp4(NEE*3-NNEE*3+buffer(w*3-2),NE,NEE,buffer(w-3),buffer(w-4)))/6)-px);
        Map[i++].set(Clip(NN+NW-buffer(w*3+1))-px);
        Map[i++].set(Clip(NN+NE-buffer(w*3-1))-px);
        Map[i++].set(Clip((W*2+NW)-(WW+2*NWW)+buffer(w+3))-px);
        Map[i++].set(Clip(((NW+NWW)/2)*3-buffer(w*2+3)*3+(buffer(w*3+4)+buffer(w*3+5))/2)-px);
        Map[i++].set(Clip(NEE+NE-buffer(w*2-3))-px);
        Map[i++].set(Clip(NWW+WW-buffer(w+4))-px);
        Map[i++].set(Clip(((W+NW)*3-NWW*6+buffer(w+3)+buffer(w*2+3))/2)-px);
        Map[i++].set(Clip((NE*2+NNE)-(NNEE+buffer(w*3-2)*2)+buffer(w*4-3))-px);
        Map[i++].set(buffer(w*6)-px);
        Map[i++].set((buffer(w-4)+buffer(w-6))/2-px);
        Map[i++].set((buffer(4)+buffer(6))/2-px);
        Map[i++].set((W+N+buffer(w-5)+buffer(w-7))/4-px);
        Map[i++].set(Clip(buffer(w-3)+W-NEE)-px);
        Map[i++].set(Clip(4*NNN-3*buffer(w*4))-px);
        Map[i++].set(Clip(N+NN-NNN)-px);
        Map[i++].set(Clip(W+WW-WWW)-px);
        Map[i++].set(Clip(W+NEE-NE)-px);
        Map[i++].set(Clip(WW+NEE-N)-px);
        Map[i++].set((Clip(W*2-NW)+Clip(W*2-NWW)+N+NE)/4-px);
        Map[i++].set(Clamp4(N*2-NN,W,N,NE,NEE)-px);
        Map[i++].set((N+NNN)/2-px);
        Map[i++].set(Clip(NN+W-NNW)-px);
        Map[i++].set(Clip(NWW+N-NNWW)-px);
        Map[i++].set(Clip((4*WWW-15*WW+20*W+Clip(NEE*2-NNEE))/10)-px);
        Map[i++].set(Clip((buffer(w*3-3)-4*NNEE+6*NE+Clip(W*3-NW*3+NNW))/4)-px);
        Map[i++].set(Clip((N*2+NE)-(NN+2*NNE)+buffer(w*3-1))-px);
        Map[i++].set(Clip((NW*2+NNW)-(NNWW+buffer(w*3+2)*2)+buffer(w*4+3))-px);
        Map[i++].set(Clip(NNWW+W-buffer(w*2+3))-px);
        Map[i++].set(Clip((-buffer(w*4)+5*NNN-10*NN+10*N+Clip(W*4-NWW*6+buffer(w*2+3)*4-buffer(w*3+4)))/5)-px);
        Map[i++].set(Clip(NEE+Clip(buffer(w-3)*2-buffer(w*2-4))-buffer(w-4))-px);
        Map[i++].set(Clip(NW+W-NWW)-px);
        Map[i++].set(Clip((N*2+NW)-(NN+2*NNW)+buffer(w*3+1))-px);
        Map[i++].set(Clip(NN+Clip(NEE*2-buffer(w*2-3))-NNE)-px);
        Map[i++].set(Clip((-buffer(4)+5*WWW-10*WW+10*W+Clip(NE*2-NNE))/5)-px);
        Map[i++].set(Clip((-buffer(5)+4*buffer(4)-5*WWW+5*W+Clip(NE*2-NNE))/4)-px);
        Map[i++].set(Clip((WWW-4*WW+6*W+Clip(NE*3-NNE*3+buffer(w*3-1)))/4)-px);
        Map[i++].set(Clip((-NNEE+3*NE+Clip(W*4-NW*6+NNW*4-buffer(w*3+1)))/3)-px);
        Map[i++].set(((W+N)*3-NW*2)/4-px);
        if (isPNG)
          ctx = ((abs(W-N)>8)<<10)|((W>N)<<9)|((abs(N-NW)>8)<<8)|((N>NW)<<7)|((abs(N-NE)>8)<<6)|((N>NE)<<5)|((W>WW)<<4)|((N>NN)<<3)|min(5,filterOn?filter+1:0);
        else
          ctx = min(0x1F,x/max(1,w/min(32,columns[0])))|( ( ((abs(W-N)*16>W+N)<<1)|(abs(N-NW)>8) )<<5 )|((W+N)&0x180);

        res = Clamp4(W+N-NW,W,NW,N,NE)-px;
      }
    }
  }

  // Predict next bit
  if (x || !isPNG){
    cm.mix(m);
    if (gray){
      for (int i=0;i<nMaps;i++)
        Map[i].mix(m);
    }
    col=(col+1)&7;
    m.set(5+ctx, 2048+5);
    m.set(col*2+(isPNG && c0==((0x100|res)>>(8-bpos))) + min(5,filterOn?filter+1:0)*16, 6*16);
    m.set(((isPNG?px:N+W)>>4) + min(5,filterOn?filter+1:0)*32, 6*32);
    m.set(c0, 256);
    m.set( ((abs((int)(W-N))>4)<<9)|((abs((int)(N-NE))>4)<<8)|((abs((int)(W-NW))>4)<<7)|((W>N)<<6)|((N>NE)<<5)|((W>NW)<<4)|((W>WW)<<3)|((N>NN)<<2)|((NW>NNWW)<<1)|(NE>NNEE), 1024 );
    m.set(min(63,column[0]), 64);
    m.set(min(127,column[1]), 128);
  }
  else{
    m.add( -2048+((filter>>(7-bpos))&1)*4096 );
    m.set(min(4,filter),5);
  }
  return 0; //8 8 32 256 512 1792
  }
  // Square buf(i)
inline int sqrbuf(int i) {
  assert(i>0);
  return buf(i)*buf(i);
}
  virtual ~im8bitModel1(){ }
 
};

//////////////////////////// im4bitModel /////////////////////////////////

// Model for 4-bit image data
class im4bitModel1: public Model {
    BlockData& x;
    Buf& buf;
    BH<16> t;
    const int S; // number of contexts
    Array<U8*> cp;
    StateMap *sm;
    U8 WW, W, NWW, NW, N, NE, NEE, NNWW, NNW, NN, NNE, NNEE;
    int col, line, run, prevColor, px;
    BH<4> t1;
    U8* cpr;
    public:
 im4bitModel1( BlockData& bd,U32 val=0 ): x(bd),buf(bd.buf),t(CMlimit(MEM()/16)),S(11),cp(S), WW(0), W(0), NWW(0), NW(0), N(0), NE(0),
  NEE(0), NNWW(0), NNW(0), NN(0), NNE(0), NNEE(0),col(0), line(0), run(0), prevColor(0), px(0),t1(65536/2) {
   sm=new StateMap[S];
   cpr=t1[0]+1;
   for (int i=0;i<S;i++)
      cp[i]=t[263*i]+1;
   }
  int inputs() {return S+2+1;}
int p(Mixer& m,int w=0,int val2=0)  {
  int i;
  if (x.blpos==1){//helps only on bigger files+1024kb
      //t.reset();
      for (i=0;i<S;i++)
      cp[i]=t[263*i]+1;
  }
  for (i=0;i<S;i++)
    *cp[i]=nex(*cp[i],x.y);

  if (x.bpos==0 || x.bpos==4){
      WW=W, NWW=NW, NW=N, N=NE, NE=NEE, NNWW=NWW, NNW=NN, NN=NNE, NNE=NNEE;
      if (x.bpos==0)
        {
        W=x.c4&0xF, NEE=buf(w-1)>>4, NNEE=buf(w*2-1)>>4;}
      else
       {
        W=x.c0&0xF, NEE=buf(w-1)&0xF, NNEE=buf(w*2-1)&0xF; }
      run=(W!=WW || col==0)?(prevColor=WW,0):min(0xFFF,run+1);
      if (cpr[0]==0 || cpr[1]!=W) cpr[0]=1, cpr[1]=W;
      else if (cpr[0]<255) ++cpr[0];
      cpr=t1[hash(W,NW,N)]+1;
  
      px=1, i=0;

      cp[i++]=t[hash(W,NW,N)]+1;
      cp[i++]=t[hash(N, min(0xFFF, col/8))]+1;
      cp[i++]=t[hash(W,NW,N,NN,NE)]+1;
      cp[i++]=t[hash(W, N, NE+NNE*16, NEE+NNEE*16)]+1;
      cp[i++]=t[hash(W, N, NW+NNW*16, NWW+NNWW*16)]+1;
      cp[i++]=t[hash(W, ilog2(run+1), prevColor, col/max(1,w/2) )]+1;
      cp[i++]=t[hash(NE, min(0x3FF, (col+line)/max(1,w*8)))]+1;
      cp[i++]=t[hash(NW, (col-line)/max(1,w*8))]+1;
      cp[i++]=t[hash(WW*16+W,NN*16+N,NNWW*16+NW)]+1;
      cp[i++]=t[N+NN*16]+1;
      cp[i++]=t[-1]+1;
      
      col++;
      if(col==w*2){col=0;line++;}
  }
  else{
    px+=px+x.y;
    int j=(x.y+1)<<(x.bpos&3);
    for (i=0;i<S;i++)
      cp[i]+=j;
  }
  if ((cpr[1]+16)>>(x.bpos&3)==px<<(x.bpos&3))
      m.add(((cpr[1]&1)*2-1)*ilog(cpr[0]+1)*8);
  else
      m.add(0);
  // predict
  for (i=0; i<S; i++)
    m.add(stretch(sm[i].p(*cp[i],x.y)));
 
  m.set(W*16+px, 256);
  m.set(min(31,col/max(1,w/16))+N*32, 512);
  m.set((x.bpos&3)+4*W+64*min(7,ilog2(run+1)), 512);
  m.set(W+NE*16+(x.bpos&3)*256, 1024);
  m.set(px, 16);
  m.set(cpr[0],256);
  return 0;
}
 virtual ~im4bitModel1(){  delete[] sm;}
 
};

//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data
class im1bitModel1: public Model {
   BlockData& x;
   Buf& buf;
   U32 r0, r1, r2, r3;  // last 4 rows, bit 8 is over current pixel
   Array<U8> t;  // model: cxt -> state
   const int N;  // number of contexts
   Array<int>  cxt;  // contexts
   StateMap* sm;
   BH<4> t1;
  U8* cp;
public:
  im1bitModel1( BlockData& bd,U32 val=0 ): x(bd),buf(bd.buf),r0(0),r1(0),r2(0),r3(0), 
    t(0x23000),N(11), cxt(N),t1(65536/2) {
   sm=new StateMap[N];
   cp=t1[0]+1;
   }
  int inputs() {return N+2;}
int p(Mixer& m,int w=0,int val2=0)  {
  // update the model
  int i;
  for (i=0; i<N; i++)
    t[cxt[i]]=nex(t[cxt[i]],x.y);
  //count run
  if (cp[0]==0 || cp[1]!=x.y) cp[0]=1, cp[1]=x.y;
  else if (cp[0]<255) ++cp[0];
  cp=t1[x.c4]+1;
  // update the contexts (pixels surrounding the predicted one)
  r0+=r0+x.y;
  r1+=r1+((x.buf(w-1)>>(7-x.bpos))&1);
  r2+=r2+((x.buf(w+w-1)>>(7-x.bpos))&1);
  r3+=r3+((x.buf(w+w+w-1)>>(7-x.bpos))&1);
  cxt[0]=(r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0);
  cxt[1]=0x100+   ((r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80));
  cxt[2]=0x200+   ((r0&1)|(r1>>4&0x1d)|(r2>>1&0x60)|(r3&0xC0));
  cxt[3]=0x300+   ((r0&1)|((r0<<1)&4)|((r1>>1)&0xF0)|((r2>>3)&0xA));//
  cxt[4]=0x400+   ((r0>>4&0x2AC)|(r1&0xA4)|(r2&0x349)|(!(r3&0x14D)));
  cxt[5]=0x800+   ((r0&1)|((r1>>4)&0xE)|((r2>>1)&0x70)|((r3<<2)&0x380));//
  cxt[6]=0xC00+   (((r1&0x30)^(r3&0x0c0c))|(r0&3));
  cxt[7]=0x1000+  ((!(r0&0x444))|(r1&0xC0C)|(r2&0xAE3)|(r3&0x51C));
  cxt[8]=0x2000+  ((r0&7)|((r1>>1)&0x3F8)|((r2<<5)&0xC00));//
  cxt[9]=0x3000+  ((r0&0x3f)^(r1&0x3ffe)^(r2<<2&0x7f00)^(r3<<5&0xf800));
  cxt[10]=0x13000+((r0&0x3e)^(r1&0x0c0c)^(r2&0xc800));
 
  // predict
  for (i=0; i<N; i++) m.add(stretch(sm[i].p(t[cxt[i]],x.y)));
  //run
  if (cp[1]==x.y)
      m.add(((cp[1]&1)*2-1)*ilog(cp[0]+1)*8);
  else
      m.add(0);
  m.set((r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0), 256);
  m.set(((r1&0x30)^(r3&0x0c))|(r0&3),256);
  m.set((r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80), 256);
  m.set((r0&0x3e)^((r1>>8)&0x0c)^((r2>>8)&0xc8),256);
  m.set(cp[0],256);
  return 0;
}
 virtual ~im1bitModel1(){  delete[] sm;}
 
};
//////////////////////////// jpegModel /////////////////////////

// Model JPEG. Return 1 if a JPEG file is detected or else 0.
// Only the baseline and 8 bit extended Huffman coded DCT modes are
// supported.  The model partially decodes the JPEG image to provide
// context for the Huffman coded symbols.

// Print a JPEG segment at buf[p...] for debugging
/*
void dump(const char* msg, int p) {
  printf("%s:", msg);
  int len=buf[p+2]*256+buf[p+3];
  for (int i=0; i<len+2; ++i)
    printf(" %02X", buf[p+i]);
  printf("\n");
}
*/
#define finish(success){ \
  int length = buf.pos - images[idx].offset; \
  if (success && idx && buf.pos-lastPos==1) \
    /*printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bEmbedded JPEG at offset %d, size: %d bytes, level %d\nCompressing... ", images[idx].offset-buf.pos+x.blpos, length, idx), fflush(stdout); */\
  memset(&images[idx], 0, sizeof(JPEGImage)); \
  mcusize=0,dqt_state=-1; \
  idx-=(idx>0); \
  images[idx].app-=length; \
  if (images[idx].app < 0) \
    images[idx].app = 0; \
}
// Detect invalid JPEG data.  The proper response is to silently
// fall back to a non-JPEG model.
#define jassert(x) if (!(x)) { \
  /*printf("JPEG error at %d, line %d: %s\n", buf.pos, __LINE__, #x);*/ \
  if (idx>0) \
    finish(false) \
  else \
    images[idx].jpeg=0; \
  return images[idx].next_jpeg;}
// Standard Huffman tables (cf. JPEG standard section K.3)
  // IMPORTANT: these are only valid for 8-bit data precision
  const static U8 bits_dc_luminance[16] = {
    0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
  };
  const static U8 values_dc_luminance[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
  };

  const static U8 bits_dc_chrominance[16] = {
    0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
  };
  const static U8 values_dc_chrominance[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
  };

  const static U8 bits_ac_luminance[16] = {
    0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d
  };
  const static U8 values_ac_luminance[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
  };

  const static U8 bits_ac_chrominance[16] = {
    0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77
  };
  const static U8 values_ac_chrominance[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
  };
  
    const static U8 zzu[64]={  // zigzag coef -> u,v
    0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
    3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
  const static U8 zzv[64]={
    0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
    4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};
struct HUF {U32 min, max; int val;}; // Huffman decode tables
  // huf[Tc][Th][m] is the minimum, maximum+1, and pointer to codes for
  // coefficient type Tc (0=DC, 1=AC), table Th (0-3), length m+1 (m=0-15)

struct JPEGImage{
  int offset, // offset of SOI marker
  jpeg, // 1 if JPEG is header detected, 2 if image data
  next_jpeg, // updated with jpeg on next byte boundary
  app, // Bytes remaining to skip in this marker
  sof, sos, data, // pointers to buf
  htsize; // number of pointers in ht
  //mcusize, // number of coefficients in an MCU
  //linesize; // width of image in MCU
  //int hufsel[2][10];  // DC/AC, mcupos/64 -> huf decode table
  int ht[8]; // pointers to Huffman table headers
  //HUF huf[128]; // Tc*64+Th*16+m -> min, max, val
  //U8 hbuf[2048]; // Tc*1024+Th*256+hufcode -> RS
  U8 qtab[256]; // table
  int qmap[10]; // block -> table number
};
class jpegModelx: public Model {
     int MaxEmbeddedLevel ;
   JPEGImage  images[3];
   int idx;
   int lastPos;
   // State of parser
   enum {SOF0=0xc0, SOF1, SOF2, SOF3, DHT, RST0=0xd0, SOI=0xd8, EOI, SOS, DQT,
    DNL, DRI, APP0=0xe0, COM=0xfe, FF};  // Second byte of 2 byte codes
   int jpeg;  // 1 if JPEG is header detected, 2 if image data
  //static int next_jpeg=0;  // updated with jpeg on next byte boundary
   int app;  // Bytes remaining to skip in APPx or COM field
   int sof, sos, data;  // pointers to buf
   Array<int> ht;  // pointers to Huffman table headers
   int htsize;  // number of pointers in ht

  // Huffman decode state
   U32 huffcode;  // Current Huffman code including extra bits
   int huffbits;  // Number of valid bits in huffcode
   int huffsize;  // Number of bits without extra bits
   int rs;  // Decoded huffcode without extra bits.  It represents
    // 2 packed 4-bit numbers, r=run of zeros, s=number of extra bits for
    // first nonzero code.  huffcode is complete when rs >= 0.
    // rs is -1 prior to decoding incomplete huffcode.
   int mcupos;  // position in MCU (0-639).  The low 6 bits mark
    // the coefficient in zigzag scan order (0=DC, 1-63=AC).  The high
    // bits mark the block within the MCU, used to select Huffman tables.

  // Decoding tables
   Array<HUF> huf;  // Tc*64+Th*16+m -> min, max, val
   int mcusize;  // number of coefficients in an MCU
   int linesize; // width of image in MCU
   int hufsel[2][10];  // DC/AC, mcupos/64 -> huf decode table
   Array<U8> hbuf;  // Tc*1024+Th*256+hufcode -> RS

  // Image state
   Array<int> color;  // block -> component (0-3)
   Array<int> pred;  // component -> last DC value
   int dc;  // DC value of the current block
   int width;  // Image width in MCU
   int row, column;  // in MCU (column 0 to width-1)
   Buf cbuf; // Rotating buffer of coefficients, coded as:
    // DC: level shifted absolute value, low 4 bits discarded, i.e.
    //   [-1023...1024] -> [0...255].
    // AC: as an RS code: a run of R (0-15) zeros followed by an S (0-15)
    //   bit number, or 00 for end of block (in zigzag order).
    //   However if R=0, then the format is ssss11xx where ssss is S,
    //   xx is the first 2 extra bits, and the last 2 bits are 1 (since
    //   this never occurs in a valid RS code).
   int cpos;  // position in cbuf
  //static U32 huff1=0, huff2=0, huff3=0, huff4=0;  // hashes of last codes
   int rs1;//, rs2, rs3, rs4;  // last 4 RS codes
   int ssum, ssum1, ssum2, ssum3;
    // sum of S in RS codes in block and sum of S in first component

   IntBuf cbuf2;
   Array<int> adv_pred, run_pred, sumu, sumv ;
   Array<int> ls;  // block -> distance to previous block
   Array<int> lcp, zpos;
   Array<int> blockW, blockN,/* nBlocks,*/ SamplingFactors;
    //for parsing Quantization tables
   int dqt_state , dqt_end , qnum;
   Array<U8> qtab; // table
   Array<int> qmap; // block -> table number

   // Context model
   const int N; // size of t, number of contexts
   BH<9> t;  // context hash -> bit history
    // As a cache optimization, the context does not include the last 1-2
    // bits of huffcode if the length (huffbits) is not a multiple of 3.
    // The 7 mapped values are for context+{"", 0, 00, 01, 1, 10, 11}.
   Array<U32> cxt;  // context hashes
   Array<U8*> cp;  // context pointers
   StateMap *sm;  
   Mixer m1;
   APM<24> a1, a2;
   BlockData& x;
   Buf& buf;
   int hbcount;
   int prev_coef,prev_coef2, prev_coef_rs;
   int rstpos,rstlen; // reset position
public:
  jpegModelx(BlockData& bd):  MaxEmbeddedLevel(3),idx(-1),
   lastPos(0), jpeg(0),app(0),sof(0),sos(0),data(0),ht(8),htsize(0),huffcode(0),
  huffbits(0),huffsize(0),rs(-1), mcupos(0), huf(128), mcusize(0),linesize(0),
  hbuf(2048),color(10), pred(4), dc(0),width(0), row(0),column(0),cbuf(0x20000),
  cpos(0), rs1(0), ssum(0), ssum1(0), ssum2(0), ssum3(0),cbuf2(0x20000),adv_pred(4), run_pred(6),
  sumu(8), sumv(8), ls(10),lcp(7), zpos(64), blockW(10), blockN(10), /*nBlocks(4),*/ SamplingFactors(4),dqt_state(-1),dqt_end(0),qnum(0),
  qtab(256),qmap(10),N(32),t(level>11?0x10000000:(CMlimit(MEM()*2))),cxt(N),cp(N),m1(32+32,2050+3 /*770*/,bd, 3),
  a1(0x8000),a2(0x20000),x(bd),buf(bd.buf),hbcount(2),prev_coef(0),prev_coef2(0), prev_coef_rs(0), rstpos(0),rstlen(0) {
  sm=new StateMap[N];
  }
  int inputs() {return 7+N*2;}
  int p(Mixer& m,int val1=0,int val2=0){

 if (idx < 0){
    memset(&images[0], 0, sizeof(images));
    idx = 0;
    lastPos = buf.pos;
  }
  if (!x.bpos && !x.blpos) images[idx].next_jpeg=0;
  
  // Be sure to quit on a byte boundary
  if (!x.bpos) images[idx].next_jpeg=images[idx].jpeg>1;
  if (x.bpos && !images[idx].jpeg) return images[idx].next_jpeg;
  if (!x.bpos && images[idx].app>0){
    --images[idx].app;
    if (idx<MaxEmbeddedLevel && buf(4)==FF && (buf(3)==SOI && buf(2)==FF && (buf(1)==0xC0 || buf(1)==0xC4 || (buf(1)>=0xDB && buf(1)<=0xFE)) ))
      memset(&images[++idx], 0, sizeof(JPEGImage));
  }
  if (images[idx].app>0) return images[idx].next_jpeg;
  if (!x.bpos) {


    // Parse.  Baseline DCT-Huffman JPEG syntax is:
    // SOI APPx... misc... SOF0 DHT... SOS data EOI
    // SOI (= FF D8) start of image.
    // APPx (= FF Ex) len ... where len is always a 2 byte big-endian length
    //   including the length itself but not the 2 byte preceding code.
    //   Application data is ignored.  There may be more than one APPx.
    // misc codes are DQT, DNL, DRI, COM (ignored).
    // SOF0 (= FF C0) len 08 height width Nf [C HV Tq]...
    //   where len, height, width (in pixels) are 2 bytes, Nf is the repeat
    //   count (1 byte) of [C HV Tq], where C is a component identifier
    //   (color, 0-3), HV is the horizontal and vertical dimensions
    //   of the MCU (high, low bits, packed), and Tq is the quantization
    //   table ID (not used).  An MCU (minimum compression unit) consists
    //   of 64*H*V DCT coefficients for each color.
    // DHT (= FF C4) len [TcTh L1...L16 V1,1..V1,L1 ... V16,1..V16,L16]...
    //   defines Huffman table Th (1-4) for Tc (0=DC (first coefficient)
    //   1=AC (next 63 coefficients)).  L1..L16 are the number of codes
    //   of length 1-16 (in ascending order) and Vx,y are the 8-bit values.
    //   A V code of RS means a run of R (0-15) zeros followed by S (0-15)
    //   additional bits to specify the next nonzero value, negative if
    //   the first additional bit is 0 (e.g. code x63 followed by the
    //   3 bits 1,0,1 specify 7 coefficients: 0, 0, 0, 0, 0, 0, 5.
    //   Code 00 means end of block (remainder of 63 AC coefficients is 0).
    // SOS (= FF DA) len Ns [Cs TdTa]... 0 3F 00
    //   Start of scan.  TdTa specifies DC/AC Huffman tables (0-3, packed
    //   into one byte) for component Cs matching C in SOF0, repeated
    //   Ns (1-4) times.
    // EOI (= FF D9) is end of image.
    // Huffman coded data is between SOI and EOI.  Codes may be embedded:
    // RST0-RST7 (= FF D0 to FF D7) mark the start of an independently
    //   compressed region.
    // DNL (= FF DC) 04 00 height
    //   might appear at the end of the scan (ignored).
    // FF 00 is interpreted as FF (to distinguish from RSTx, DNL, EOI).

    // Detect JPEG (SOI followed by a valid marker)
    if (!images[idx].jpeg && buf(4)==FF && buf(3)==SOI && buf(2)==FF && ((buf(1)==0xC0 || buf(1)==0xC4 || (buf(1)>=0xDB && buf(1)<=0xFE))||(buf(1)>>4==0xe || buf(1)==0xdb)) ){
      images[idx].jpeg=1;
      images[idx].offset = buf.pos-4;
      images[idx].sos=images[idx].sof=images[idx].htsize=images[idx].data=0, images[idx].app=(buf(1)>>4==0xE)*2;
      mcusize=huffcode=huffbits=huffsize=mcupos=cpos=0, rs=-1;
      memset(&huf[0], 0, sizeof(huf));
      memset(&pred[0], 0, pred.size()*sizeof(int));
      rstpos=rstlen=0;
    }

    // Detect end of JPEG when data contains a marker other than RSTx
    // or byte stuff (00), or if we jumped in position since the last byte seen
    if (images[idx].jpeg && images[idx].data && ((buf(2)==FF && buf(1) && (buf(1)&0xf8)!=RST0) || (buf.pos-lastPos>1)) ) {
      jassert((buf(1)==EOI) || (buf.pos-lastPos>1));
      finish(true);
      //images[idx].jpeg=images[idx].next_jpeg; // ??
    }
    lastPos = buf.pos;
    if (!images[idx].jpeg) return images[idx].next_jpeg;
     //if (!images[idx].jpeg) return 0; //??
    // Detect APPx or COM field
    if (!images[idx].data && !images[idx].app && buf(4)==FF && (((buf(3)>=0xC1) && (buf(3)<=0xCF) && (buf(3)!=DHT)) || ((buf(3)>=0xDC) && (buf(3)<=0xFE)))){
//&& (buf(3)>>4==0xe || buf(3)==COM))
      images[idx].app=buf(2)*256+buf(1)+2;
   if (idx)
        jassert( buf.pos + images[idx].app < images[idx].offset + images[idx-1].app );
    }
    // Save pointers to sof, ht, sos, data,
    if (buf(5)==FF && buf(4)==SOS) {
      int len=buf(3)*256+buf(2);
      if (len==6+2*buf(1) && buf(1) && buf(1)<=4)  // buf(1) is Ns
        images[idx].sos=buf.pos-5, images[idx].data=images[idx].sos+len+2, images[idx].jpeg=2;
    }
    if (buf(4)==FF && buf(3)==DHT && images[idx].htsize<8) images[idx].ht[images[idx].htsize++]=buf.pos-4;
    if (buf(4)==FF && buf(3)==SOF0) images[idx].sof=buf.pos-4;
    if (buf(4)==FF && buf(3)==0xc3) images[idx].jpeg=images[idx].next_jpeg=0; //sof3
    // Parse Quantizazion tables
    if (buf(4)==FF && buf(3)==DQT)
      dqt_end=buf.pos+buf(2)*256+buf(1)-1, dqt_state=0;
    else if (dqt_state>=0) {
      if (buf.pos>=dqt_end)
        dqt_state = -1;
      else {
        if (dqt_state%65==0)
          qnum = buf(1);
        else {
          jassert(buf(1)>0);
          jassert(qnum>=0 && qnum<4);
          images[idx].qtab[qnum*64+((dqt_state%65)-1)]=buf(1)-1;
        }
        dqt_state++;
      }
    }

    // Restart
    if (buf(2)==FF && (buf(1)&0xf8)==RST0) {
      huffcode=huffbits=huffsize=mcupos=0, rs=-1;
      memset(&pred[0], 0, pred.size()*sizeof(int));
      rstlen=column+row*width-rstpos;
      rstpos=column+row*width;
    }
  }

  {
    // Build Huffman tables
    // huf[Tc][Th][m] = min, max+1 codes of length m, pointer to byte values
    if (buf.pos==images[idx].data && x.bpos==1) {
      //jassert(htsize>0);
      int i;
      for (i=0; i<images[idx].htsize; ++i) {
        int p=images[idx].ht[i]+4;  // pointer to current table after length field
        int end=p+buf[p-2]*256+buf[p-1]-2;  // end of Huffman table
        int count=0;  // sanity check
        while (p<end && end<buf.pos && end<p+2100 && ++count<10) {
          int tc=buf[p]>>4, th=buf[p]&15;
          if (tc>=2 || th>=4) break;
          jassert(tc>=0 && tc<2 && th>=0 && th<4);
          HUF* h=&huf[tc*64+th*16]; // [tc][th][0];
          int val=p+17;  // pointer to values
          int hval=tc*1024+th*256;  // pointer to RS values in hbuf
          int j;
          for (j=0; j<256; ++j) // copy RS codes
            hbuf[hval+j]=buf[val+j];
          int code=0;
          for (j=0; j<16; ++j) {
            h[j].min=code;
            h[j].max=code+=buf[p+j+1];
            h[j].val=hval;
            val+=buf[p+j+1];
            hval+=buf[p+j+1];
            code*=2;
          }
          p=val;
          jassert(hval>=0 && hval<2048);
        }
        jassert(p==end);
      }
      huffcode=huffbits=huffsize=0, rs=-1;
// load default tables
      if (!images[idx].htsize){
        for (int tc = 0; tc < 2; tc++) {
          for (int th = 0; th < 2; th++) {
            HUF* h = &huf[tc*64+th*16];
            int hval = tc*1024 + th*256;
            int code = 0, c = 0, xd = 0;

            for (int i = 0; i < 16; i++) {
              switch (tc*2+th) {
                case 0: xd = bits_dc_luminance[i]; break;
                case 1: xd = bits_dc_chrominance[i]; break;
                case 2: xd = bits_ac_luminance[i]; break;
                case 3: xd = bits_ac_chrominance[i];
              }

              h[i].min = code;
              h[i].max = (code+=xd);
              h[i].val = hval;
              hval+=xd;
              code+=code;
              c+=xd;
            }

            hval = tc*1024 + th*256;
            c--;

            while (c >= 0){
              switch (tc*2+th) {
                case 0: xd = values_dc_luminance[c]; break;
                case 1: xd = values_dc_chrominance[c]; break;
                case 2: xd = values_ac_luminance[c]; break;
                case 3: xd = values_ac_chrominance[c];
              }

              hbuf[hval+c] = xd;
              c--;
            }
          }
        }
        images[idx].htsize = 4;
      }
      // Build Huffman table selection table (indexed by mcupos).
      // Get image width.
      if (!images[idx].sof && images[idx].sos) return  images[idx].next_jpeg;//return 0;
      int ns=buf[images[idx].sos+4];
      int nf=buf[images[idx].sof+9];
      jassert(ns<=4 && nf<=4);
      mcusize=0;  // blocks per MCU
      int hmax=0;  // MCU horizontal dimension
      for (i=0; i<ns; ++i) {
        for (int j=0; j<nf; ++j) {
          if (buf[images[idx].sos+2*i+5]==buf[images[idx].sof+3*j+10]) { // Cs == C ?
            int hv=buf[images[idx].sof+3*j+11];  // packed dimensions H x V
            SamplingFactors[j] = hv;
            if (hv>>4>hmax) hmax=hv>>4;
            hv=(hv&15)*(hv>>4);  // number of blocks in component C
            jassert(hv>=1 && hv+mcusize<=10);
            while (hv) {
              jassert(mcusize<10);
              hufsel[0][mcusize]=buf[images[idx].sos+2*i+6]>>4&15;
              hufsel[1][mcusize]=buf[images[idx].sos+2*i+6]&15;
              jassert (hufsel[0][mcusize]<4 && hufsel[1][mcusize]<4);
              color[mcusize]=i;
              int tq=buf[images[idx].sof+3*j+12];  // quantization table index (0..3)
              jassert(tq>=0 && tq<4);
              images[idx].qmap[mcusize]=tq; // quantizazion table mapping
              --hv;
              ++mcusize;
            }
          }
        }
      }
      jassert(hmax>=1 && hmax<=10);
      int j;
      for (j=0; j<mcusize; ++j) {
        ls[j]=0;
        for (int i=1; i<mcusize; ++i) if (color[(j+i)%mcusize]==color[j]) ls[j]=i;
        ls[j]=(mcusize-ls[j])<<6;
        //blockW[j] = ls[j];
      }
      for (j=0; j<64; ++j) zpos[zzu[j]+8*zzv[j]]=j;
      width=buf[images[idx].sof+7]*256+buf[images[idx].sof+8];  // in pixels
      width=(width-1)/(hmax*8)+1;  // in MCU
      jassert(width>0);
      mcusize*=64;  // coefficients per MCU
      row=column=0;
      
      
      // we can have more blocks than components then we have subsampling
      int x=0, y=0; 
      for (j = 0; j<(mcusize>>6); j++) {
        int i = color[j];
        int w = SamplingFactors[i]>>4, h = SamplingFactors[i]&0xf;
        blockW[j] = x==0?mcusize-64*(w-1):64;
        blockN[j] = y==0?mcusize*width-64*w*(h-1):w*64;
        x++;
        if (x>=w) { x=0; y++; }
        if (y>=h) { x=0; y=0; }
      }
    }
  }


  // Decode Huffman
  {
    if (mcusize && buf(1+(!x.bpos))!=FF) {  // skip stuffed byte
      jassert(huffbits<=32);
      huffcode+=huffcode+x.y;
      ++huffbits;
      if (rs<0) {
        jassert(huffbits>=1 && huffbits<=16);
        const int ac=(mcupos&63)>0;
        jassert(mcupos>=0 && (mcupos>>6)<10);
        jassert(ac==0 || ac==1);
        const int sel=hufsel[ac][mcupos>>6];
        jassert(sel>=0 && sel<4);
        const int i=huffbits-1;
        jassert(i>=0 && i<16);
        const HUF *h=&huf[ac*64+sel*16]; // [ac][sel];
        jassert(h[i].min<=h[i].max && h[i].val<2048 && huffbits>0);
        if (huffcode<h[i].max) {
          jassert(huffcode>=h[i].min);
          int k=h[i].val+huffcode-h[i].min;
          jassert(k>=0 && k<2048);
          rs=hbuf[k];
          huffsize=huffbits;
        }
      }
      if (rs>=0) {
        if (huffsize+(rs&15)==huffbits) { // done decoding
          rs1=rs;
          int xe=0;  // decoded extra bits
          if (mcupos&63) {  // AC
            if (rs==0) { // EOB
              mcupos=(mcupos+63)&-64;
              jassert(mcupos>=0 && mcupos<=mcusize && mcupos<=640);
              while (cpos&63) {
                cbuf2[cpos]=0;
                cbuf[cpos]=(!rs)?0:(63-(cpos&63))<<4; cpos++; rs++;
              }
            }
            else {  // rs = r zeros + s extra bits for the next nonzero value
                    // If first extra bit is 0 then value is negative.
              jassert((rs&15)<=10);
              const int r=rs>>4;
              const int s=rs&15;
              jassert(mcupos>>6==(mcupos+r)>>6);
              mcupos+=r+1;
              xe=huffcode&((1<<s)-1);
              if (s && !(xe>>(s-1))) xe-=(1<<s)-1;
              for (int i=r; i>=1; --i) {
                cbuf2[cpos]=0;
                cbuf[cpos++]=i<<4|s;
              }
              cbuf2[cpos]=xe;
              cbuf[cpos++]=(s<<4)|(huffcode<<2>>s&3)|12;
              ssum+=s;
            }
          }
          else {  // DC: rs = 0S, s<12
            jassert(rs<12);
            ++mcupos;
            xe=huffcode&((1<<rs)-1);
            if (rs && !(xe>>(rs-1))) xe-=(1<<rs)-1;
            jassert(mcupos>=0 && mcupos>>6<10);
            const int comp=color[mcupos>>6];
            jassert(comp>=0 && comp<4);
            dc=pred[comp]+=xe;
            jassert((cpos&63)==0);
            cbuf2[cpos]=dc;
            cbuf[cpos++]=(dc+1023)>>3;
            if ((mcupos>>6)==0) {
              ssum1=0;
              ssum2=ssum3;
            } else {
              if (color[(mcupos>>6)-1]==color[0]) ssum1+=(ssum3=ssum);
              ssum2=ssum1;
            }
            ssum=rs;
          }
          jassert(mcupos>=0 && mcupos<=mcusize);
          if (mcupos>=mcusize) {
            mcupos=0;
            if (++column==width) column=0, ++row;
          }
          huffcode=huffsize=huffbits=0, rs=-1;

          // UPDATE_ADV_PRED !!!!
          {
            const int acomp=mcupos>>6, q=64*images[idx].qmap[acomp];
            const int zz=mcupos&63, cpos_dc=cpos-zz;
            const bool norst=rstpos!=column+row*width;
            if (zz==0) {
              for (int i=0; i<8; ++i) sumu[i]=sumv[i]=0;
              // position in the buffer of first (DC) coefficient of the block
              // of this same component that is to the west of this one, not
              // necessarily in this MCU
              int offset_DC_W = cpos_dc - blockW[acomp];
              // position in the buffer of first (DC) coefficient of the block
              // of this same component that is to the north of this one, not
              // necessarily in this MCU
              int offset_DC_N = cpos_dc - blockN[acomp];
              for (int i=0; i<64; ++i) {
                sumu[zzu[i]]+=(zzv[i]&1?-1:1)*(zzv[i]?16*(16+zzv[i]):185)*(images[idx].qtab[q+i]+1)*cbuf2[offset_DC_N+i];
                sumv[zzv[i]]+=(zzu[i]&1?-1:1)*(zzu[i]?16*(16+zzu[i]):185)*(images[idx].qtab[q+i]+1)*cbuf2[offset_DC_W+i];
              }
            }
            else {
              sumu[zzu[zz-1]]-=(zzv[zz-1]?16*(16+zzv[zz-1]):185)*(images[idx].qtab[q+zz-1]+1)*cbuf2[cpos-1];
              sumv[zzv[zz-1]]-=(zzu[zz-1]?16*(16+zzu[zz-1]):185)*(images[idx].qtab[q+zz-1]+1)*cbuf2[cpos-1];
            }

            for (int i=0; i<3; ++i)
            {
              run_pred[i]=run_pred[i+3]=0;
              for (int st=0; st<10 && zz+st<64; ++st) {
                const int zz2=zz+st;
                int p=sumu[zzu[zz2]]*i+sumv[zzv[zz2]]*(2-i);
                p/=(images[idx].qtab[q+zz2]+1)*185*(16+zzv[zz2])*(16+zzu[zz2])/128;
                if (zz2==0 && (norst || ls[acomp]==64)) p-=cbuf2[cpos_dc-ls[acomp]];
                p=(p<0?-1:+1)*ilog(abs(p)+1);
                if (st==0) {
                  adv_pred[i]=p;
                }
                else if (abs(p)>abs(adv_pred[i])+2 && abs(adv_pred[i]) < 210) {
                  if (run_pred[i]==0) run_pred[i]=st*2+(p>0);
                  if (abs(p)>abs(adv_pred[i])+21 && run_pred[i+3]==0) run_pred[i+3]=st*2+(p>0);
                }
              }
            }
            xe=0;
            for (int i=0; i<8; ++i) xe+=(zzu[zz]<i)*sumu[i]+(zzv[zz]<i)*sumv[i];
            xe=(sumu[zzu[zz]]*(2+zzu[zz])+sumv[zzv[zz]]*(2+zzv[zz])-xe*2)*4/(zzu[zz]+zzv[zz]+16);
            xe/=(images[idx].qtab[q+zz]+1)*185;
            if (zz==0 && (norst || ls[acomp]==64)) xe-=cbuf2[cpos_dc-ls[acomp]];
            adv_pred[3]=(xe<0?-1:+1)*ilog(abs(xe)+1);

            for (int i=0; i<4; ++i) {
              const int a=(i&1?zzv[zz]:zzu[zz]), b=(i&2?2:1);
              if (a<b) xe=65535;
              else {
                const int zz2=zpos[zzu[zz]+8*zzv[zz]-(i&1?8:1)*b];
                xe=(images[idx].qtab[q+zz2]+1)*cbuf2[cpos_dc+zz2]/(images[idx].qtab[q+zz]+1);
                xe=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));
              }
              lcp[i]=xe;
            }
            if ((zzu[zz]*zzv[zz])!=0){
              const int zz2=zpos[zzu[zz]+8*zzv[zz]-9];
              xe=(images[idx].qtab[q+zz2]+1)*cbuf2[cpos_dc+zz2]/(images[idx].qtab[q+zz]+1);
              lcp[4]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));

              xe=(images[idx].qtab[q+zpos[8*zzv[zz]]]+1)*cbuf2[cpos_dc+zpos[8*zzv[zz]]]/(images[idx].qtab[q+zz]+1);
              lcp[5]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));

              xe=(images[idx].qtab[q+zpos[zzu[zz]]]+1)*cbuf2[cpos_dc+zpos[zzu[zz]]]/(images[idx].qtab[q+zz]+1);
              lcp[6]=(xe<0?-1:+1)*(ilog(abs(xe)+1)+(xe!=0?17:0));
            }
            else
              lcp[4]=lcp[5]=lcp[6]=65535;

            int prev1=0,prev2=0,cnt1=0,cnt2=0,r=0,s=0;
            prev_coef_rs = cbuf[cpos-64];
            for (int i=0; i<acomp; i++) {
              xe=0;
              xe+=cbuf2[cpos-(acomp-i)*64];
              if (zz==0 && (norst || ls[i]==64)) xe-=cbuf2[cpos_dc-(acomp-i)*64-ls[i]];
              if (color[i]==color[acomp]-1) { prev1+=xe; cnt1++; r+=cbuf[cpos-(acomp-i)*64]>>4; s+=cbuf[cpos-(acomp-i)*64]&0xF; }
              if (color[acomp]>1 && color[i]==color[0]) { prev2+=xe; cnt2++; }
            }
            if (cnt1>0) prev1/=cnt1, r/=cnt1, s/=cnt1, prev_coef_rs=(r<<4)|s;
            if (cnt2>0) prev2/=cnt2;
            prev_coef=(prev1<0?-1:+1)*ilog(11*abs(prev1)+1)+(cnt1<<20);
            prev_coef2=(prev2<0?-1:+1)*ilog(11*abs(prev2)+1);
           
            if (column==0 && blockW[acomp]>64*acomp) run_pred[1]=run_pred[2], run_pred[0]=0, adv_pred[1]=adv_pred[2], adv_pred[0]=0;
            if (row==0 && blockN[acomp]>64*acomp) run_pred[1]=run_pred[0], run_pred[2]=0, adv_pred[1]=adv_pred[0], adv_pred[2]=0;
          } // !!!!

        }
      }
    }
  }

  // Estimate next bit probability
  if (!images[idx].jpeg || !images[idx].data) return images[idx].next_jpeg;//return 0;
  if (buf(1+(!x.bpos))==FF) {
    m.add(128);
    m.set(0, 9);
    m.set(0, 1025);
    m.set(buf(1), 1024);
    return 1;
  }
  if (rstlen>0 && rstlen==column+row*width-rstpos && mcupos==0 && (int)huffcode==(1<<huffbits)-1) {
    m.add(2047);
    m.set(0, 9);
    m.set(0, 1025); 
    m.set(buf(1), 1024);
    return 1;
  }
  // Update model
  if (cp[N-1]) {
    for (int i=0; i<N; ++i)
      *cp[i]=nex(*cp[i],x.y);
  }
  m1.update();

  // Update context
  const int comp=color[mcupos>>6];
  const int coef=(mcupos&63)|comp<<6;
  const int hc=(huffcode*4+((mcupos&63)==0)*2+(comp==0))|1<<(huffbits+2);
  const bool firstcol=column==0 && blockW[mcupos>>6]>mcupos;
  if (++hbcount>2 || huffbits==0) hbcount=0;
  jassert(coef>=0 && coef<256);
  const int zu=zzu[mcupos&63], zv=zzv[mcupos&63];
    if (hbcount==0) {
    int n=hc*32;
    cxt[0]=hash(++n, coef, adv_pred[2]/12+(run_pred[2]<<8), ssum2>>6, prev_coef/72);
    cxt[1]=hash(++n, coef, adv_pred[0]/12+(run_pred[0]<<8), ssum2>>6, prev_coef/72);
    cxt[2]=hash(++n, coef, adv_pred[1]/11+(run_pred[1]<<8), ssum2>>6);
    cxt[3]=hash(++n, rs1, adv_pred[2]/7, run_pred[5]/2, prev_coef/10);
    cxt[4]=hash(++n, rs1, adv_pred[0]/7, run_pred[3]/2, prev_coef/10);
    cxt[5]=hash(++n, rs1, adv_pred[1]/11, run_pred[4]);
    cxt[6]=hash(++n, adv_pred[2]/14, run_pred[2], adv_pred[0]/14, run_pred[0]);
    cxt[7]=hash(++n, cbuf[cpos-blockN[mcupos>>6]]>>4, adv_pred[3]/17, run_pred[1], run_pred[5]);
    cxt[8]=hash(++n, cbuf[cpos-blockW[mcupos>>6]]>>4, adv_pred[3]/17, run_pred[1], run_pred[3]);
    cxt[9]=hash(++n, lcp[0]/22, lcp[1]/22, adv_pred[1]/7, run_pred[1]);
    cxt[10]=hash(++n, lcp[0]/22, lcp[1]/22, mcupos&63, lcp[4]/30);
    cxt[11]=hash(++n, zu/2, lcp[0]/13, lcp[2]/30, prev_coef/40+((prev_coef2/28)<<20));
    cxt[12]=hash(++n, zv/2, lcp[1]/13, lcp[3]/30, prev_coef/40+((prev_coef2/28)<<20));
    cxt[13]=hash(++n, rs1, prev_coef/42, prev_coef2/34, hash(lcp[0]/60,lcp[2]/14,lcp[1]/60,lcp[3]/14));
    cxt[14]=hash(++n, mcupos&63, column>>1);
    cxt[15]=hash(++n, column>>3, min(5+2*(!comp),zu+zv), hash(lcp[0]/10,lcp[2]/40,lcp[1]/10,lcp[3]/40));
    cxt[16]=hash(++n, ssum>>3, mcupos&63);
    cxt[17]=hash(++n, rs1, mcupos&63, run_pred[1]);
    cxt[18]=hash(++n, coef, ssum2>>5, adv_pred[3]/30, (comp)?hash(prev_coef/22,prev_coef2/50):ssum/((mcupos&0x3F)+1));
    cxt[19]=hash(++n, lcp[0]/40, lcp[1]/40, adv_pred[1]/28, hash( (comp)?prev_coef/40+((prev_coef2/40)<<20):lcp[4]/22, min(7,zu+zv), ssum/(2*(zu+zv)+1) ) );
    cxt[20]=hash(++n, zv, cbuf[cpos-blockN[mcupos>>6]], adv_pred[2]/28, run_pred[2]);
    cxt[21]=hash(++n, zu, cbuf[cpos-blockW[mcupos>>6]], adv_pred[0]/28, run_pred[0]);
    cxt[22]=hash(++n, adv_pred[2]/7, run_pred[2]);
    cxt[23]=hash(n, adv_pred[0]/7, run_pred[0]);
    cxt[24]=hash(n, adv_pred[1]/7, run_pred[1]);
    cxt[25]=hash(++n, zv, lcp[1]/14, adv_pred[2]/16, run_pred[5]);
    cxt[26]=hash(++n, zu, lcp[0]/14, adv_pred[0]/16, run_pred[3]);
    cxt[27]=hash(++n, lcp[0]/14, lcp[1]/14, adv_pred[3]/16);
    cxt[28]=hash(++n, coef, prev_coef/10, prev_coef2/20);
    cxt[29]=hash(++n, coef, ssum>>2, prev_coef_rs);
    cxt[30]=hash(++n, coef, adv_pred[1]/17, hash(lcp[(zu<zv)]/24,lcp[2]/20,lcp[3]/24));
    cxt[31]=hash(++n, coef, adv_pred[3]/11, hash(lcp[(zu<zv)]/50,lcp[2+3*(zu*zv>1)]/50,lcp[3+3*(zu*zv>1)]/50));
  }

  // Predict next bit
  m1.add(128);
  assert(hbcount<=2);
  int p;
 switch(hbcount)
  {
   case 0: for (int i=0; i<N; ++i){ cp[i]=t[cxt[i]]+1, p=sm[i].p(*cp[i],x.y); m.add((p-2048)>>3); m1.add(p=stretch(p)); m.add(p>>1);} break;
   case 1: { int hc=1+(huffcode&1)*3; for (int i=0; i<N; ++i){ cp[i]+=hc, p=sm[i].p(*cp[i],x.y); m.add((p-2048)>>3); m1.add(p=stretch(p)); m.add(p>>1); }} break;
   default: { int hc=1+(huffcode&1); for (int i=0; i<N; ++i){ cp[i]+=hc, p=sm[i].p(*cp[i],x.y); m.add((p-2048)>>3); m1.add(p=stretch(p)); m.add(p>>1); }} break;
  }

   m1.set(firstcol, 2);
   m1.set( coef+256*min(3,huffbits), 1024 );
   m1.set( (hc&0x1FE)*2+min(3,ilog2(zu+zv)), 1024 );
  int pr=m1.p(1,1);
  m.add(stretch(pr)>>1);
  m.add((pr>>2)-511);
  pr=a1.p(pr, (hc&511)|(((adv_pred[1]/16)&63)<<9), x.y,1023);
  m.add(stretch(pr)>>1);
  m.add((pr>>2)-511);
  pr=a2.p(pr, (hc&511)|(coef<<9),x.y, 1023);
  
  m.add(stretch(pr)>>1);
  m.add((pr>>2)-511);
  m.set( 1 + (zu+zv<5)+(huffbits>8)*2+firstcol*4, 9 );
  m.set( 1 + (hc&0xFF) + 256*min(3,(zu+zv)/3), 1025 );
  m.set( coef+256*min(3,huffbits/2), 1024 );
  return 1;
  }
  virtual ~jpegModelx(){
  delete[] sm;
   }
 
};

//////////////////////////// wavModel /////////////////////////////////

// Model a 16/8-bit stereo/mono uncompressed .wav file.
// Based on 'An asymptotically Optimal Predictor for Stereo Lossless Audio Compression'
// by Florin Ghido.

class wavModel1: public Model {
  int pr[3][2], n[2], counter[2];
double F[49][49][2],L[49][49];
  
   long  double sum;
  //const double a,a2;
  const int SC;
  SmallStationaryContextMap scm1, scm2, scm3, scm4, scm5, scm6, scm7;
  ContextMap cm;
  int bits, channels, w,rlen;
  int z1, z2, z3, z4, z5, z6, z7;
 // int winfo;
  int col;
  int S,D;
  int wmode;
  recordModel1* recModel;
  BlockData& x;
  Buf& buf;
  int ch;
public:
  wavModel1(BlockData& bd): SC(0x20000),scm1(8,8), scm2(8,8), scm3(8,8),
   scm4(8,8), scm5(8,8), scm6(8,8), scm7(8,8),cm(CMlimit(MEM()*4), 10+1),rlen(0),col(0),x(bd),buf(bd.buf),ch(0){
  /*  bits=((winfo%4)/2)*8+8;
    channels=winfo%2+1;
    w=channels*(bits>>3);
    wmode=winfo;
    rlen=(bits/8*channels);
    if (channels==1) S=48,D=0; else S=36,D=12;
    for (int j=0; j<channels; j++) {
      for (k=0; k<=S+D; k++) for (l=0; l<=S+D; l++) F[k][l][j]=0, L[k][l]=0;
      F[1][0][j]=1;
      n[j]=counter[j]=pr[2][j]=pr[1][j]=pr[0][j]=0;
      z1=z2=z3=z4=z5=z6=z7=0;
    }*/
   // printf("%d",sizeof(long long int));
    recModel=0;
    if (level>=4)recModel=new recordModel1(bd,CMlimit(MEM()));
}
int inputs() {return 11*cm.inputs()+7*2+(level>=4?recModel->inputs():0);}
int p(Mixer& m,int info,int val2=0){
    int j,k,l,i=0;
     const double a=0.996,a2=1/a;
  if (!x.blpos && x.bpos==1) {
    bits=((info%4)/2)*8+8;
    channels=info%2+1;
    w=channels*(bits>>3);
    wmode=info;
    rlen=(bits/8*channels);
    if (channels==1) S=48,D=0; else S=36,D=12;
    for (int j=0; j<channels; j++) {
      for (k=0; k<=S+D; k++) for (l=0; l<=S+D; l++) F[k][l][j]=0, L[k][l]=0;
      F[1][0][j]=1;
      n[j]=counter[j]=pr[2][j]=pr[1][j]=pr[0][j]=0;
      z1=z2=z3=z4=z5=z6=z7=0;
    }
  }
  
// Select previous samples and predicted sample as context
  if (!x.bpos && x.blpos>=w) {
      
    ch=(x.blpos)%w;
    const int msb=ch%(bits>>3);
    const int chn=ch/(bits>>3);
    if (!msb) {
      z1=X1(1), z2=X1(2), z3=X1(3), z4=X1(4), z5=X1(5);
      k=X1(1);
      for (l=0; l<=min(S,counter[chn]-1); l++) { F[0][l][chn]*=a; F[0][l][chn]+=X1(l+1)*k; }
      for (l=1; l<=min(D,counter[chn]); l++) { F[0][l+S][chn]*=a; F[0][l+S][chn]+=X2(l+1)*k; }
      if (channels==2) {
        k=X2(2);
        for (l=1; l<=min(D,counter[chn]); l++) { F[S+1][l+S][chn]*=a; F[S+1][l+S][chn]+=X2(l+1)*k; }
        for (l=1; l<=min(S,counter[chn]-1); l++) { F[l][S+1][chn]*=a; F[l][S+1][chn]+=X1(l+1)*k; }
        z6=X2(1)+X1(1)-X2(2), z7=X2(1);
      } else z6=2*X1(1)-X1(2), z7=X1(1);
      if (++n[chn]==(256>>(level<9?level:8))) {
        if (channels==1) for (k=1; k<=S+D; k++) for (l=k; l<=S+D; l++) F[k][l][chn]=(F[k-1][l-1][chn]-X1(k)*X1(l))*a2;
        else for (k=1; k<=S+D; k++) if (k!=S+1) for (l=k; l<=S+D; l++) if (l!=S+1) F[k][l][chn]=(F[k-1][l-1][chn]-(k-1<=S?X1(k):X2(k-S))*(l-1<=S?X1(l):X2(l-S)))*a2;
        for (i=1; i<=S+D; i++) {
           sum=F[i][i][chn];
           for (k=1; k<i; k++) sum-=L[i][k]*L[i][k];
           sum=floor(sum+0.5);
           sum=1/sum;
           if (sum>0) {
             L[i][i]=sqrt(sum);
             for (j=(i+1); j<=S+D; j++) {
               sum=F[i][j][chn];
               for (k=1; k<i; k++) sum-=L[j][k]*L[i][k];
               sum=floor(sum+0.5);
               L[j][i]=sum*L[i][i];
             }
           } else break;
        }
        if (i>S+D && counter[chn]>S+1) {
          for (k=1; k<=S+D; k++) {
            F[k][0][chn]=F[0][k][chn];
            for (j=1; j<k; j++) F[k][0][chn]-=L[k][j]*F[j][0][chn];
            F[k][0][chn]*=L[k][k];
          }
          for (k=S+D; k>0; k--) {
            for (j=k+1; j<=S+D; j++) F[k][0][chn]-=L[j][k]*F[j][0][chn];
            F[k][0][chn]*=L[k][k];
          }
        }
        n[chn]=0;
      }
      sum=0;
      for (l=1; l<=S+D; l++) sum+=F[l][0][chn]*(l<=S?X1(l):X2(l-S));
      pr[2][chn]=pr[1][chn];
      pr[1][chn]=pr[0][chn];
      pr[0][chn]=int(floor(sum));
      counter[chn]++;
    }
    const int y1=pr[0][chn], y2=pr[1][chn], y3=pr[2][chn];
    int x1=buf(1), x2=buf(2), x3=buf(3);
    if (wmode==4 || wmode==5) x1^=128, x2^=128;
    if (bits==8) x1-=128, x2-=128;
    const int t=((bits==8) || ((!msb)^(wmode<6)));
    i=ch<<4;
    if ((msb)^(wmode<6)) {
      cm.set(hash(++i, y1&0xff));
      cm.set(hash(++i, y1&0xff, ((z1-y2+z2-y3)>>1)&0xff));
      cm.set(hash(++i, x1, y1&0xff));
      cm.set(hash(++i, x1, x2>>3, x3));
      if (bits==8)        
        cm.set(hash(++i, y1&0xFE, ilog2(abs((int)(z1-y2)))*2+(z1>y2) ));
      else  cm.set(hash(++i, (y1+z1-y2)&0xff));
      cm.set(hash(++i, x1));
      cm.set(hash(++i, x1, x2));
      cm.set(hash(++i, z1&0xff));
      cm.set(hash(++i, (z1*2-z2)&0xff));
      cm.set(hash(++i, z6&0xff));
      cm.set(hash( ++i, y1&0xFF, ((z1-y2+z2-y3)/(bits>>3))&0xFF ));
    } else {
      cm.set(hash(++i, (y1-x1+z1-y2)>>8));
      cm.set(hash(++i, (y1-x1)>>8));
      cm.set(hash(++i, (y1-x1+z1*2-y2*2-z2+y3)>>8));
      cm.set(hash(++i, (y1-x1)>>8, (z1-y2+z2-y3)>>9));
      cm.set(hash(++i, z1>>12));
      cm.set(hash(++i, x1));
      cm.set(hash(++i, x1>>7, x2, x3>>7));
      cm.set(hash(++i, z1>>8));
      cm.set(hash(++i, (z1*2-z2)>>8));
      cm.set(hash(++i, y1>>8));
      cm.set(hash( ++i, (y1-x1)>>6 ));
    }
    scm1.set(t*ch);
    scm2.set(t*((z1-x1+y1)>>9)&0xff);
    scm3.set(t*((z1*2-z2-x1+y1)>>8)&0xff);
    scm4.set(t*((z1*3-z2*3+z3-x1)>>7)&0xff);
    scm5.set(t*((z1+z7-x1+y1*2)>>10)&0xff);
    scm6.set(t*((z1*4-z2*6+z3*4-z4-x1)>>7)&0xff);
    scm7.set(t*((z1*5-z2*10+z3*10-z4*5+z5-x1+y1)>>9)&0xff);
  }

  // Predict next bit
  scm1.mix(m);
  scm2.mix(m);
  scm3.mix(m);
  scm4.mix(m);
  scm5.mix(m);
  scm6.mix(m);
  scm7.mix(m);
  cm.mix(m);
  if (level>=4 &&  (rlen>1)) recModel->p(m,rlen);
  if (++col>=w*8) col=0;
  //m.set(3, 8);
  m.set( ch+4*ilog2(col&(bits-1)), 4*8 );
  m.set(col%bits<8, 2);
  m.set(col%bits, bits);
  m.set(col, w*8);
  m.set(x.c0, 256);
  return 0;
  } 

inline int s2(int i) { return int(short(buf(i)+256*buf(i-1))); }
inline int t2(int i) { return int(short(buf(i-1)+256*buf(i))); }

inline int X1(int i) {
  switch (wmode) {
    case 0: return buf(i)-128;
    case 1: return buf(i<<1)-128;
    case 2: return s2(i<<1);
    case 3: return s2(i<<2);
    case 4: return (buf(i)^128)-128;
    case 5: return (buf(i<<1)^128)-128;
    case 6: return t2(i<<1);
    case 7: return t2(i<<2);
    default: return 0;
  }
}

inline int X2(int i) {
  switch (wmode) {
    case 0: return buf(i+S)-128;
    case 1: return buf((i<<1)-1)-128;
    case 2: return s2((i+S)<<1);
    case 3: return s2((i<<2)-2);
    case 4: return (buf(i+S)^128)-128;
    case 5: return (buf((i<<1)-1)^128)-128;
    case 6: return t2((i+S)<<1);
    case 7: return t2((i<<2)-2);
    default: return 0;
  }
}
  virtual ~wavModel1(){
  if (recModel!=0)delete recModel;
   }
 
};

#define CacheSize (1<<5)

#if (CacheSize&(CacheSize-1)) || (CacheSize<8)
  #error Cache size must be a power of 2 bigger than 4
#endif
//////////////////////////// exeModel /////////////////////////

// Model x86 code.  The contexts are sparse containing only those
// bits relevant to parsing (2 prefixes, opcode, and mod and r/m fields
// of modR/M byte).

// formats
enum InstructionFormat {
  // encoding mode
  fNM = 0x0,      // no ModRM
  fAM = 0x1,      // no ModRM, "address mode" (jumps or direct addresses)
  fMR = 0x2,      // ModRM present
  fMEXTRA = 0x3,  // ModRM present, includes extra bits for opcode
  fMODE = 0x3,    // bitmask for mode

  // no ModRM: size of immediate operand
  fNI = 0x0,      // no immediate
  fBI = 0x4,      // byte immediate
  fWI = 0x8,      // word immediate
  fDI = 0xc,      // dword immediate
  fTYPE = 0xc,    // type mask

  // address mode: type of address operand
  fAD = 0x0,      // absolute address
  fDA = 0x4,      // dword absolute jump target
  fBR = 0x8,      // byte relative jump target
  fDR = 0xc,      // dword relative jump target

  // others
  fERR = 0xf     // denotes invalid opcodes
};

// 1 byte opcodes
const static U8 Table1[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 0
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 2
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI, // 3

  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 4
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 5
  fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fDI,fMR|fDI,fNM|fBI,fMR|fBI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 6
  fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR, // 7

  fMR|fBI,fMR|fDI,fMR|fBI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 8
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fAM|fDA,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // 9
  fAM|fAD,fAM|fAD,fAM|fAD,fAM|fAD,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fBI,fNM|fDI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // a
  fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI,fNM|fDI, // b

  fMR|fBI,fMR|fBI,fNM|fWI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fBI,fMR|fDI,fNM|fBI,fNM|fNI,fNM|fWI,fNM|fNI,fNM|fNI,fNM|fBI,fERR   ,fNM|fNI, // c
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fBI,fNM|fBI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // d
  fAM|fBR,fAM|fBR,fAM|fBR,fAM|fBR,fNM|fBI,fNM|fBI,fNM|fBI,fNM|fBI,fAM|fDR,fAM|fDR,fAM|fAD,fAM|fBR,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // e
  fNM|fNI,fERR   ,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fMEXTRA,fMEXTRA,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fMEXTRA,fMEXTRA, // f
};

// 2 byte opcodes
const static U8 Table2[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fNM|fNI,fERR   ,fNM|fNI,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 0
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 2
  fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fERR   ,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 3

  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 4
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 5
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 6
  fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI, // 7

  fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR,fAM|fDR, // 8
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 9
  fNM|fNI,fNM|fNI,fNM|fNI,fMR|fNI,fMR|fBI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fBI,fMR|fNI,fERR   ,fMR|fNI, // a
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // b

  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI,fNM|fNI, // c
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // d
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // e
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   , // f
};

// 3 byte opcodes $0F38XX
const static U8 Table3_38[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   , // 0
  fMR|fNI,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fERR   , // 1
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   , // 2
  fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // 3
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 4
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 5
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 6
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 7
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 8
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 9
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // a
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // b
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // c
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // d
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // e
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // f
};

// 3 byte opcodes $0F3AXX
const static U8 Table3_3A[256] = {
  // 0       1       2       3       4       5       6       7       8       9       a       b       c       d       e       f
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI, // 0
  fERR   ,fERR   ,fERR   ,fERR   ,fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 1
  fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 2
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 3
  fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 4
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 5
  fMR|fBI,fMR|fBI,fMR|fBI,fMR|fBI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 6
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 7
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 8
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // 9
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // a
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // b
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // c
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // d
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // e
  fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // f
};

// escape opcodes using ModRM byte to get more variants
const static U8 TableX[32] = {
  // 0       1       2       3       4       5       6       7
  fMR|fBI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // escapes for 0xf6
  fMR|fDI,fERR   ,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI,fMR|fNI, // escapes for 0xf7
  fMR|fNI,fMR|fNI,fERR   ,fERR   ,fERR   ,fERR   ,fERR   ,fERR   , // escapes for 0xfe
  fMR|fNI,fMR|fNI,fMR|fNI,fERR   ,fMR|fNI,fERR   ,fMR|fNI,fERR   , // escapes for 0xff
};

const static U8 InvalidX64Ops[19] = {0x06, 0x07, 0x16, 0x17, 0x1E, 0x1F, 0x27, 0x2F, 0x37, 0x3F, 0x60, 0x61, 0x62, 0x82, 0x9A, 0xD4, 0xD5, 0xD6, 0xEA,};
const static U8 X64Prefixes[8] = {0x26, 0x2E, 0x36, 0x3E, 0x9B, 0xF0, 0xF2, 0xF3,};
enum InstructionCategory {
  OP_INVALID              =  0,
  OP_PREFIX_SEGREG        =  1,
  OP_PREFIX               =  2,
  OP_PREFIX_X87FPU        =  3,
  OP_GEN_DATAMOV          =  4,
  OP_GEN_STACK            =  5,
  OP_GEN_CONVERSION       =  6,
  OP_GEN_ARITH_DECIMAL    =  7,
  OP_GEN_ARITH_BINARY     =  8,
  OP_GEN_LOGICAL          =  9,
  OP_GEN_SHF_ROT          = 10,
  OP_GEN_BIT              = 11,
  OP_GEN_BRANCH           = 12,
  OP_GEN_BRANCH_COND      = 13,
  OP_GEN_BREAK            = 14,
  OP_GEN_STRING           = 15,
  OP_GEN_INOUT            = 16,
  OP_GEN_FLAG_CONTROL     = 17,
  OP_GEN_SEGREG           = 18,
  OP_GEN_CONTROL          = 19,
  OP_SYSTEM               = 20,
  OP_X87_DATAMOV          = 21,
  OP_X87_ARITH            = 22,
  OP_X87_COMPARISON       = 23,
  OP_X87_TRANSCENDENTAL   = 24,
  OP_X87_LOAD_CONSTANT    = 25,
  OP_X87_CONTROL          = 26,
  OP_X87_CONVERSION       = 27,
  OP_STATE_MANAGEMENT     = 28,
  OP_MMX                  = 29,
  OP_SSE                  = 30,
  OP_SSE_DATAMOV          = 31
};

const static U8 TypeOp1[256] = {
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //03
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //07
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //0B
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_STACK         , OP_PREFIX            , //0F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //13
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //17
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //1B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_STACK         , //1F
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //23
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //27
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //2B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //2F
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , //33
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //37
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //3B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_PREFIX_SEGREG     , OP_GEN_ARITH_DECIMAL , //3F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //43
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //47
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //4B
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //4F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //53
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //57
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //5B
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_STACK         , //5F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_BREAK         , OP_GEN_CONVERSION    , //63
  OP_PREFIX_SEGREG     , OP_PREFIX_SEGREG     , OP_PREFIX            , OP_PREFIX            , //67
  OP_GEN_STACK         , OP_GEN_ARITH_BINARY  , OP_GEN_STACK         , OP_GEN_ARITH_BINARY  , //6B
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //6F
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //73
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //77
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //7B
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //7F
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //83
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //87
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //8B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_STACK         , //8F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //93
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //97
  OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , OP_GEN_BRANCH        , OP_PREFIX_X87FPU     , //9B
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //A3
  OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , //A7
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_STRING        , OP_GEN_STRING        , //AB
  OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , OP_GEN_STRING        , //AF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //B3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //B7
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //BB
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //BF
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //C3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //C7
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //CB
  OP_GEN_BREAK         , OP_GEN_BREAK         , OP_GEN_BREAK         , OP_GEN_BREAK         , //CF
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , //D3
  OP_GEN_ARITH_DECIMAL , OP_GEN_ARITH_DECIMAL , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //D7
  OP_X87_ARITH         , OP_X87_DATAMOV       , OP_X87_ARITH         , OP_X87_DATAMOV       , //DB
  OP_X87_ARITH         , OP_X87_DATAMOV       , OP_X87_ARITH         , OP_X87_DATAMOV       , //DF
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //E3
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //E7
  OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_BRANCH        , //EB
  OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , OP_GEN_INOUT         , //EF
  OP_PREFIX            , OP_GEN_BREAK         , OP_PREFIX            , OP_PREFIX            , //F3
  OP_SYSTEM            , OP_GEN_FLAG_CONTROL  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , //F7
  OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , //FB
  OP_GEN_FLAG_CONTROL  , OP_GEN_FLAG_CONTROL  , OP_GEN_ARITH_BINARY  , OP_GEN_BRANCH         //FF
};

const static U8 TypeOp2[256] = {
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //03
  OP_INVALID           , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //07
  OP_SYSTEM            , OP_SYSTEM            , OP_INVALID           , OP_GEN_CONTROL       , //0B
  OP_INVALID           , OP_GEN_CONTROL       , OP_INVALID           , OP_INVALID           , //0F
  OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , //13
  OP_SSE               , OP_SSE               , OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , //17
  OP_SSE               , OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , //1B
  OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , OP_GEN_CONTROL       , //1F
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //23
  OP_SYSTEM            , OP_INVALID           , OP_SYSTEM            , OP_INVALID           , //27
  OP_SSE_DATAMOV       , OP_SSE_DATAMOV       , OP_SSE               , OP_SSE               , //2B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //2F
  OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , OP_SYSTEM            , //33
  OP_SYSTEM            , OP_SYSTEM            , OP_INVALID           , OP_INVALID           , //37
  OP_PREFIX            , OP_INVALID           , OP_PREFIX            , OP_INVALID           , //3B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //43
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //47
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //4B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //4F
  OP_SSE_DATAMOV       , OP_SSE               , OP_SSE               , OP_SSE               , //53
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //57
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //5B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //5F
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //63
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //67
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //6B
  OP_INVALID           , OP_INVALID           , OP_MMX               , OP_MMX               , //6F
  OP_SSE               , OP_MMX               , OP_MMX               , OP_MMX               , //73
  OP_MMX               , OP_MMX               , OP_MMX               , OP_MMX               , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_MMX               , OP_MMX               , //7F
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //83
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //87
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //8B
  OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , OP_GEN_BRANCH_COND   , //8F
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //93
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //97
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9B
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //9F
  OP_GEN_STACK         , OP_GEN_STACK         , OP_GEN_CONTROL       , OP_GEN_BIT           , //A3
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_INVALID           , OP_INVALID           , //A7
  OP_GEN_STACK         , OP_GEN_STACK         , OP_SYSTEM            , OP_GEN_BIT           , //AB
  OP_GEN_SHF_ROT       , OP_GEN_SHF_ROT       , OP_STATE_MANAGEMENT  , OP_GEN_ARITH_BINARY  , //AF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_BIT           , //B3
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , //B7
  OP_INVALID           , OP_GEN_CONTROL       , OP_GEN_BIT           , OP_GEN_BIT           , //BB
  OP_GEN_BIT           , OP_GEN_BIT           , OP_GEN_CONVERSION    , OP_GEN_CONVERSION    , //BF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_SSE               , OP_SSE               , //C3
  OP_SSE               , OP_SSE               , OP_SSE               , OP_GEN_DATAMOV       , //C7
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //CB
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , //CF
  OP_INVALID           , OP_MMX               , OP_MMX               , OP_MMX               , //D3
  OP_SSE               , OP_MMX               , OP_INVALID           , OP_SSE               , //D7
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //DB
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //DF
  OP_SSE               , OP_MMX               , OP_SSE               , OP_MMX               , //E3
  OP_SSE               , OP_MMX               , OP_INVALID           , OP_SSE               , //E7
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //EB
  OP_MMX               , OP_MMX               , OP_SSE               , OP_MMX               , //EF
  OP_INVALID           , OP_MMX               , OP_MMX               , OP_MMX               , //F3
  OP_SSE               , OP_MMX               , OP_SSE               , OP_SSE               , //F7
  OP_MMX               , OP_MMX               , OP_MMX               , OP_SSE               , //FB
  OP_MMX               , OP_MMX               , OP_MMX               , OP_INVALID            //FF
};

const static U8 TypeOp3_38[256] = {
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //03
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //07
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //0F
  OP_SSE               , OP_INVALID           , OP_INVALID           , OP_INVALID           , //13
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_SSE               , //17
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //1F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //23
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_INVALID           , //27
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //2B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //33
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_SSE               , //37
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //3B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //3F
  OP_SSE               , OP_SSE               , OP_INVALID           , OP_INVALID           , //43
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //47
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //53
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //57
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //63
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //67
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //73
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //83
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //87
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //93
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //97
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EF
  OP_GEN_DATAMOV       , OP_GEN_DATAMOV       , OP_INVALID           , OP_INVALID           , //F3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //FB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID            //FF
};

const static U8 TypeOp3_3A[256] = {
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //03
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //07
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0B
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //0F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //13
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //17
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //1F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //23
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //27
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //2F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //33
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //37
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //3F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_INVALID           , //43
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //47
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //4F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //53
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //57
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //5F
  OP_SSE               , OP_SSE               , OP_SSE               , OP_SSE               , //63
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //67
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //6F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //73
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //77
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //7F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //83
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //87
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //8F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //93
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //97
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9B
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //9F
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //A7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //AF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //B7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //BF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //C7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //CF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //D7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //DF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //E7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //EF
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F3
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //F7
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           , //FB
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID            //FF
};

const static U8 TypeOpX[32] = {
  // escapes for F6
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_ARITH_BINARY  ,
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  ,
  // escapes for F7
  OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_LOGICAL       , OP_GEN_ARITH_BINARY  ,
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  ,
  // escapes for FE
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_INVALID           , OP_INVALID           ,
  OP_INVALID           , OP_INVALID           , OP_INVALID           , OP_INVALID           ,
  // escapes for FF
  OP_GEN_ARITH_BINARY  , OP_GEN_ARITH_BINARY  , OP_GEN_BRANCH        , OP_GEN_BRANCH        ,
  OP_GEN_BRANCH        , OP_GEN_BRANCH        , OP_GEN_STACK         , OP_INVALID           
};
enum Prefixes {
  ES_OVERRIDE = 0x26,
  CS_OVERRIDE = 0x2E,
  SS_OVERRIDE = 0x36,
  DS_OVERRIDE = 0x3E,
  FS_OVERRIDE = 0x64,
  GS_OVERRIDE = 0x65,
  AD_OVERRIDE = 0x67,
  WAIT_FPU    = 0x9B,
  LOCK        = 0xF0,
  REP_N_STR   = 0xF2,
  REP_STR     = 0xF3
};

enum Opcodes {
  // 1-byte opcodes of special interest (for one reason or another)
  OP_2BYTE  = 0x0f,     // start of 2-byte opcode
  OP_OSIZE  = 0x66,     // operand size prefix
  OP_CALLF  = 0x9a,
  OP_RETNI  = 0xc2,     // ret near+immediate
  OP_RETN   = 0xc3,
  OP_ENTER  = 0xc8,
  OP_INT3   = 0xcc,
  OP_INTO   = 0xce,
  OP_CALLN  = 0xe8,
  OP_JMPF   = 0xea,
  OP_ICEBP  = 0xf1
};

enum ExeState {
  Start             =  0,
  Pref_Op_Size      =  1,
  Pref_MultiByte_Op =  2,
  ParseFlags        =  3,
  ExtraFlags        =  4,
  ReadModRM         =  5,
  Read_OP3_38       =  6,
  Read_OP3_3A       =  7,
  ReadSIB           =  8,
  Read8             =  9,
  Read16            = 10,
  Read32            = 11,
  Read8_ModRM       = 12,
  Read16_f          = 13,
  Read32_ModRM      = 14,
  Error             = 15
};

struct OpCache {
  U32 Op[CacheSize];
  U32 Index;
};

struct Instruction {
  U32 Data;
  U8 Prefix, Code, ModRM, SIB, REX, Flags, BytesRead, Size, Category;
  bool MustCheckREX, Decoding, o16, imm8;
};

#define CodeShift            3
#define CodeMask             (0xFF<<CodeShift)
#define ClearCodeMask        ((-1)^CodeMask)
#define PrefixMask           ((1<<CodeShift)-1)
#define OperandSizeOverride  (0x01<<(8+CodeShift))
#define MultiByteOpcode      (0x02<<(8+CodeShift))
#define PrefixREX            (0x04<<(8+CodeShift))
#define Prefix38             (0x08<<(8+CodeShift))
#define Prefix3A             (0x10<<(8+CodeShift))
#define HasExtraFlags        (0x20<<(8+CodeShift))
#define HasModRM             (0x40<<(8+CodeShift))
#define ModRMShift           (7+8+CodeShift)
#define SIBScaleShift        (ModRMShift+8-6)
#define RegDWordDisplacement (0x01<<(8+SIBScaleShift))
#define AddressMode          (0x02<<(8+SIBScaleShift))
#define TypeShift            (2+8+SIBScaleShift)
#define CategoryShift        5
#define CategoryMask         ((1<<CategoryShift)-1)
#define ModRM_mod            0xC0
#define ModRM_reg            0x38
#define ModRM_rm             0x07
#define SIB_scale            0xC0
#define SIB_index            0x38
#define SIB_base             0x07
#define REX_w                0x08

#define MinRequired          8 // minimum required consecutive valid instructions to be considered as code

class exeModel1: public Model {
  BlockData& x;
  Buf& buf;
  const int N1, N2;
 ContextMap cm;
    OpCache Cache;
    ExeState pState , State ;
    Instruction Op;
    U32 TotalOps, OpMask, OpCategMask, Context, BrkPoint , BrkCtx ;
    bool Valid;
    U32 StateBH[256];
public:
  exeModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf),N1(8), N2(10),cm(CMlimit(MEM()), N1+N2),
  pState (Start), State( Start), TotalOps(0), OpMask(0),OpCategMask(0), Context(0),BrkPoint(0),
  BrkCtx(0),Valid(false) {
      memset(&Cache, 0, sizeof(OpCache));
      memset(&Op, 0, sizeof(Instruction));
      memset(&StateBH, 0, sizeof(StateBH));
  }
  int inputs() {return (N1+N2)*cm.inputs();}
int p(Mixer& m,int val1=0,int val2=0){
    if (x.filetype==DBASE  ||x.filetype==HDR  ||x.filetype==DECA ){
        for (int i=0; i<inputs(); i++)
        m.add(0); 
        m.set(0, 1024);
        m.set(0, 1024);
        m.set(0, 1024);
        return false;
    }
  if (x.bpos==0 ) {
    pState = State;
    U8 B = (U8)x.c4;
    Op.Size++;
    switch (State){
      case Start: case Error: {
        // previous code may have just been a REX prefix
        bool Skip = false;
        if (Op.MustCheckREX){
          Op.MustCheckREX = false;
          // valid x64 code?
          if (!IsInvalidX64Op(B) && !IsValidX64Prefix(B)){
            Op.REX = Op.Code;
            Op.Code = B;
            Op.Data = PrefixREX|(Op.Code<<CodeShift)|(Op.Data&PrefixMask); 
            Skip = true;
          }
        }
        
        Op.ModRM = Op.SIB = Op.REX = Op.Flags = Op.BytesRead = 0;       
        if (!Skip){
          Op.Code = B;
          // possible REX prefix?
          Op.MustCheckREX = ((Op.Code&0xF0)==0x40) && (!(Op.Decoding && ((Op.Data&PrefixMask)==1)));
          
          // check prefixes
          Op.Prefix = (Op.Code==ES_OVERRIDE || Op.Code==CS_OVERRIDE || Op.Code==SS_OVERRIDE || Op.Code==DS_OVERRIDE) + //invalid in x64
                      (Op.Code==FS_OVERRIDE)*2 +
                      (Op.Code==GS_OVERRIDE)*3 +
                      (Op.Code==AD_OVERRIDE)*4 +
                      (Op.Code==WAIT_FPU)*5 +
                      (Op.Code==LOCK)*6 +
                      (Op.Code==REP_N_STR || Op.Code==REP_STR)*7;

          if (!Op.Decoding){
            TotalOps+=(Op.Data!=0)-(Cache.Index && Cache.Op[ Cache.Index&(CacheSize-1) ]!=0);
            OpMask = (OpMask<<1)|(State!=Error);
            OpCategMask = (OpCategMask<<CategoryShift)|(Op.Category);
            Op.Size = 0;
            
            Cache.Op[ Cache.Index&(CacheSize-1) ] = Op.Data;
            Cache.Index++;

            if (!Op.Prefix)
              Op.Data = Op.Code<<CodeShift;
            else{
              Op.Data = Op.Prefix;
              Op.Category = TypeOp1[Op.Code];
              Op.Decoding = true;
              BrkCtx = hash(1+(BrkPoint = 0), Op.Prefix, OpCategMask&CategoryMask);
              break;
            }
          }
          else{
            // we only have enough bits for one prefix, so the
            // instruction will be encoded with the last one
            if (!Op.Prefix){
              Op.Data|=(Op.Code<<CodeShift);
              Op.Decoding = false;
            }
            else{
              Op.Data = Op.Prefix;
              Op.Category = TypeOp1[Op.Code];
              BrkCtx = hash(1+(BrkPoint = 1), Op.Prefix, OpCategMask&CategoryMask);
              break;
            }
          }
        }

        if ((Op.o16=(Op.Code==OP_OSIZE)))
          State = Pref_Op_Size;
        else if (Op.Code==OP_2BYTE)
          State = Pref_MultiByte_Op;
        else
          ReadFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 2), State, Op.Code, (OpCategMask&CategoryMask), OpN(Cache,1)&((ModRM_mod|ModRM_reg|ModRM_rm)<<ModRMShift));
        break;
      }
      case Pref_Op_Size : {
        Op.Code = B;
        ApplyCodeAndSetFlag(Op, OperandSizeOverride);
        ReadFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 3), State);
        break;
      }
      case Pref_MultiByte_Op : {
        Op.Code = B;        
        Op.Data|=MultiByteOpcode;

        if (Op.Code==0x38)
          State = Read_OP3_38;
        else if (Op.Code==0x3A)
          State = Read_OP3_3A;
        else{
          ApplyCodeAndSetFlag(Op);
          Op.Flags = Table2[Op.Code];
          Op.Category = TypeOp2[Op.Code];
          CheckFlags(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 4), State);
        break;
      }
      case ParseFlags : {
        ProcessFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 5), State);
        break;
      }
      case ExtraFlags : case ReadModRM : {
        Op.ModRM = B;
        Op.Data|=(Op.ModRM<<ModRMShift)|HasModRM;
        Op.SIB = 0;
        if (Op.Flags==fMEXTRA){
          Op.Data|=HasExtraFlags;
          int i = ((Op.ModRM>>3)&0x07) | ((Op.Code&0x01)<<3) | ((Op.Code&0x08)<<1);
          Op.Flags = TableX[i];
          Op.Category = TypeOpX[i];
          if (Op.Flags==fERR){
            memset(&Op, 0, sizeof(Instruction));
            State = Error;
            BrkCtx = hash(1+(BrkPoint = 6), State);
            break;
          }
          ProcessFlags(Op, State);
          BrkCtx = hash(1+(BrkPoint = 7), State);
          break;
        }

        if ((Op.ModRM & ModRM_rm)==4 && Op.ModRM<ModRM_mod){
          State = ReadSIB;
          BrkCtx = hash(1+(BrkPoint = 8), State);
          break;
        }

        ProcessModRM(Op, State);
        BrkCtx = hash(1+(BrkPoint = 9), State, Op.Code );
        break;
      }
      case Read_OP3_38 : case Read_OP3_3A : {
        Op.Code = B;
        ApplyCodeAndSetFlag(Op, Prefix38<<(State-Read_OP3_38));
        if (State==Read_OP3_38){
          Op.Flags = Table3_38[Op.Code];
          Op.Category = TypeOp3_38[Op.Code];
        }
        else{
          Op.Flags = Table3_3A[Op.Code];
          Op.Category = TypeOp3_3A[Op.Code];
        }
        CheckFlags(Op, State);
        BrkCtx = hash(1+(BrkPoint = 10), State);
        break;
      }
      case ReadSIB : {
        Op.SIB = B;
        Op.Data|=((Op.SIB & SIB_scale)<<SIBScaleShift);
        ProcessModRM(Op, State);
        BrkCtx = hash(1+(BrkPoint = 11), State, Op.SIB&SIB_scale);
        break;
      }
      case Read8 : case Read16 : case Read32 : {
        if (++Op.BytesRead>=((State-Read8)<<int(Op.imm8+1))){
          Op.BytesRead = 0;
          Op.imm8 = false;
          State = Start;
        }
        BrkCtx = hash(1+(BrkPoint = 12), State, Op.Flags&fMODE, Op.BytesRead, ((Op.BytesRead>1)?(buf(Op.BytesRead)<<8):0)|((Op.BytesRead)?B:0) );
        break;
      }
      case Read8_ModRM : {
        ProcessMode(Op, State);
        BrkCtx = hash(1+(BrkPoint = 13), State);
        break;
      }
      case Read16_f : {
        if (++Op.BytesRead==2){
          Op.BytesRead = 0;
          ProcessFlags2(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 14), State);
        break;
      }
      case Read32_ModRM : {
        Op.Data|=RegDWordDisplacement;
        if (++Op.BytesRead==4){
          Op.BytesRead = 0;
          ProcessMode(Op, State);
        }
        BrkCtx = hash(1+(BrkPoint = 15), State);
        break;
      }
    }

    Valid = (TotalOps>2*MinRequired) && ((OpMask&((1<<MinRequired)-1))==((1<<MinRequired)-1));
    Context = State+16*Op.BytesRead+16*(Op.REX & REX_w);
    StateBH[Context] = (StateBH[Context]<<8)|B;

    if (Valid || val1){
      int mask=0, count0=0;
      for (int i=0, j=0; i<N1; ++i){
        if (i>1) mask=mask*2+(buf(i-1)==0), count0+=mask&1;
        j=(i<4)?i+1:5+(i-4)*(2+(i>6));
        cm.set(hash(execxt(j, buf(1)*(j>6)), ((1<<N1)|mask)*(count0*N1/2>=i), (0x08|(x.blpos&0x07))*(i<4)));
      }

      cm.set(BrkCtx);
      mask = PrefixMask|(0xF8<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A;
      cm.set(hash(OpN(Cache, 1)&(mask|RegDWordDisplacement|AddressMode), State+16*Op.BytesRead, Op.Data&mask, Op.REX, Op.Category));
      
      mask = 0x04|(0xFE<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A|((ModRM_mod|ModRM_reg)<<ModRMShift);
      cm.set(hash(
        OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, OpN(Cache, 3)&mask,
        Context+256*((Op.ModRM & ModRM_mod)==ModRM_mod),
        Op.Data&((mask|PrefixREX)^(ModRM_mod<<ModRMShift))
      ));
     
      mask = 0x04|CodeMask;
      cm.set(hash(OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, OpN(Cache, 3)&mask, OpN(Cache, 4)&mask, (Op.Data&mask)|(State<<11)|(Op.BytesRead<<15)));

      mask = 0x04|(0xFC<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A;
      cm.set(hash(State+16*Op.BytesRead, Op.Data&mask, Op.Category*8 + (OpMask&0x07), Op.Flags, ((Op.SIB & SIB_base)==5)*4+((Op.ModRM & ModRM_reg)==ModRM_reg)*2+((Op.ModRM & ModRM_mod)==0)));

      mask = PrefixMask|CodeMask|OperandSizeOverride|MultiByteOpcode|PrefixREX|Prefix38|Prefix3A|HasExtraFlags|HasModRM|((ModRM_mod|ModRM_rm)<<ModRMShift);
      cm.set(hash(Op.Data&mask, State+16*Op.BytesRead, Op.Flags));

      mask = PrefixMask|CodeMask|OperandSizeOverride|MultiByteOpcode|Prefix38|Prefix3A|HasExtraFlags|HasModRM;
      cm.set(hash(OpN(Cache, 1)&mask, State, Op.BytesRead*2+((Op.REX&REX_w)>0), Op.Data&((U16)(mask^OperandSizeOverride))));

      mask = 0x04|(0xFE<<CodeShift)|MultiByteOpcode|Prefix38|Prefix3A|(ModRM_reg<<ModRMShift);
      cm.set(hash(OpN(Cache, 1)&mask, OpN(Cache, 2)&mask, State+16*Op.BytesRead, Op.Data&(mask|PrefixMask|CodeMask)));

      cm.set(State+16*Op.BytesRead);

      cm.set(hash(
        (0x100|B)*(Op.BytesRead>0),
        State+16*pState+256*Op.BytesRead,
        ((Op.Flags&fMODE)==fAM)*16 + (Op.REX & REX_w) + (Op.o16)*4 + ((Op.Code & 0xFE)==0xE8)*2 + ((Op.Data & MultiByteOpcode)!=0 && (Op.Code & 0xF0)==0x80)
      ));
      
    }
  }
  if (Valid || val1){
    cm.mix(m);
  }else{
      for (int i=0; i<inputs(); ++i)
        m.add(0);
  }
  U8 s = ((StateBH[Context]>>(28-x.bpos))&0x08) |
         ((StateBH[Context]>>(21-x.bpos))&0x04) |
         ((StateBH[Context]>>(14-x.bpos))&0x02) |
         ((StateBH[Context]>>( 7-x.bpos))&0x01) |
         ((Op.Category==OP_GEN_BRANCH)<<4)|
         (((x.c0&((1<<x.bpos)-1))==0)<<5);

  m.set((Context*4+(s>>4)), 1024);
  m.set((State*64+x.bpos*8+(Op.BytesRead>0)*4+(s>>4)), 1024);
  m.set( (BrkCtx&0x1FF)|((s&0x20)<<4), 1024 );
  m.set(hash(Op.Code, State, OpN(Cache, 1)&CodeMask)&0x1FFF, 8192);
  m.set(hash(State, x.bpos, Op.Code, Op.BytesRead)&0x1FFF, 8192);
  m.set(hash(State, (x.bpos<<2)|(x.c0&3), OpCategMask&CategoryMask, ((Op.Category==OP_GEN_BRANCH)<<2)|(((Op.Flags&fMODE)==fAM)<<1)|(Op.BytesRead>0))&0x1FFF, 8192);
  
  return Valid;
}

inline bool IsInvalidX64Op(U8 Op){
  for (int i=0; i<19; i++){
    if (Op == InvalidX64Ops[i])
      return true;
  }
  return false;
}

inline bool IsValidX64Prefix(U8 Prefix){
  for (int i=0; i<8; i++){
    if (Prefix == X64Prefixes[i])
      return true;
  }
  return ((Prefix>=0x40 && Prefix<=0x4F) || (Prefix>=0x64 && Prefix<=0x67));
}

void ProcessMode(Instruction &Op, ExeState &State){
  if ((Op.Flags&fMODE)==fAM){
    Op.Data|=AddressMode;
    Op.BytesRead = 0;
    switch (Op.Flags&fTYPE){
      case fDR : Op.Data|=(2<<TypeShift);
      case fDA : Op.Data|=(1<<TypeShift);
      case fAD : {
        State = Read32;
        break;
      }
      case fBR : {
        Op.Data|=(2<<TypeShift);
        State = Read8;
      }
    }
  }
  else{
    switch (Op.Flags&fTYPE){
      case fBI : State = Read8; break;
      case fWI : {
        State = Read16;
        Op.Data|=(1<<TypeShift);
        Op.BytesRead = 0;
        break;
      }
      case fDI : {
        // x64 Move with 8byte immediate? [REX.W is set, opcodes 0xB8+r]
        Op.imm8=((Op.REX & REX_w)>0 && (Op.Code&0xF8)==0xB8);
        if (!Op.o16 || Op.imm8){
          State = Read32;
          Op.Data|=(2<<TypeShift);
        }
        else{
          State = Read16;
          Op.Data|=(3<<TypeShift);
        }
        Op.BytesRead = 0;
        break;
      }
      default: State = Start; /*no immediate*/
    }
  }
}

void ProcessFlags2(Instruction &Op, ExeState &State){
  //if arriving from state ExtraFlags, we've already read the ModRM byte
  if ((Op.Flags&fMODE)==fMR && State!=ExtraFlags){
    State = ReadModRM;
    return;
  }
  ProcessMode(Op, State);
}

void ProcessFlags(Instruction &Op, ExeState &State){
  if (Op.Code==OP_CALLF || Op.Code==OP_JMPF || Op.Code==OP_ENTER){
    Op.BytesRead = 0;
    State = Read16_f;
    return; //must exit, ENTER has ModRM too
  }
  ProcessFlags2(Op, State);
}

void CheckFlags(Instruction &Op, ExeState &State){
  //must peek at ModRM byte to read the REG part, so we can know the opcode
  if (Op.Flags==fMEXTRA)
    State = ExtraFlags;
  else if (Op.Flags==fERR){
    memset(&Op, 0, sizeof(Instruction));
    State = Error;
  }
  else
    ProcessFlags(Op, State);
}

void ReadFlags(Instruction &Op, ExeState &State){
  Op.Flags = Table1[Op.Code];
  Op.Category = TypeOp1[Op.Code];
  CheckFlags(Op, State);
}

void ProcessModRM(Instruction &Op, ExeState &State){
  if ((Op.ModRM & ModRM_mod)==0x40)
    State = Read8_ModRM; //register+byte displacement
  else if ((Op.ModRM & ModRM_mod)==0x80 || (Op.ModRM & (ModRM_mod|ModRM_rm))==0x05 || (Op.ModRM<0x40 && (Op.SIB & SIB_base)==0x05) ){
    State = Read32_ModRM; //register+dword displacement
    Op.BytesRead = 0;
  }
  else
    ProcessMode(Op, State);
}

void ApplyCodeAndSetFlag(Instruction &Op, U32 Flag = 0){
  Op.Data&=ClearCodeMask; \
  Op.Data|=(Op.Code<<CodeShift)|Flag;
}

inline U32 OpN(OpCache &Cache, U32 n){
  return Cache.Op[ (Cache.Index-n)&(CacheSize-1) ];
}

inline U32 OpNCateg(U32 &Mask, U32 n){
  return ((Mask>>(CategoryShift*(n-1)))&CategoryMask);
}

inline int pref(int i) { return (buf(i)==0x0f)+2*(buf(i)==0x66)+3*(buf(i)==0x67); }

// Get context at buf(i) relevant to parsing 32-bit x86 code
U32 execxt(int i, int x=0) {
  int prefix=0, opcode=0, modrm=0, sib=0;
  if (i) prefix+=4*pref(i--);
  if (i) prefix+=pref(i--);
  if (i) opcode+=buf(i--);
  if (i) modrm+=buf(i--)&(ModRM_mod|ModRM_rm);
  if (i&&((modrm&ModRM_rm)==4)&&(modrm<ModRM_mod)) sib=buf(i)&SIB_scale;
  return prefix|opcode<<4|modrm<<12|x<<20|sib<<(28-6);
}
virtual ~exeModel1(){ }
};

//////////////////////////// indirectModel /////////////////////

// The context is a byte string history that occurs within a
// 1 or 2 byte context.
class indirectModel1: public Model {
  BlockData& x;
  Buf& buf;
  ContextMap cm;
  Array<U32> t1;
  Array<U16> t2;
  Array<U16> t3;
  Array<U16> t4;
public:
  indirectModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf),cm(CMlimit(MEM()),12),t1(256),
   t2(0x10000), t3(0x8000),t4(0x8000){
  }
  int inputs() {return 12*cm.inputs();}
int p(Mixer& m,int val1=0,int val2=0){
    int j=0;
  if (!x.bpos) {
      
    U32 d=x.c4&0xffff, c=d&255, d2=(x.buf(1)&31)+32*(x.buf(2)&31)+1024*(x.buf(3)&31);
    U32 d3=(x.buf(1)>>3&31)+32*(x.buf(3)>>3&31)+1024*(x.buf(4)>>3&31);
    U32& r1=t1[d>>8];
    r1=r1<<8|c;
    U16& r2=t2[x.c4>>8&0xffff];
    r2=r2<<8|c;
    U16& r3=t3[(x.buf(2)&31)+32*(x.buf(3)&31)+1024*(x.buf(4)&31)];
    r3=r3<<8|c;
    U16& r4=t4[(x.buf(2)>>3&31)+32*(x.buf(4)>>3&31)+1024*(x.buf(5)>>3&31)];
    r4=r4<<8|c;
    const U32 t=c|t1[c]<<8;
    const U32 t0=d|t2[d]<<16;
    const U32 ta=d2|t3[d2]<<16;
    const U32 tc=d3|t4[d3]<<16;
    cm.set(hash(j++,t));
    cm.set(hash(j++,t0));
    cm.set(hash(j++,ta));  
    cm.set(hash(j++,tc));
    cm.set(hash(j++,t&0xff00));
    cm.set(hash(j++,t0&0xff0000));
    cm.set(hash(j++,ta&0xff0000));
    cm.set(hash(j++,tc&0xff0000));
    cm.set(hash(j++,t&0xffff));
    cm.set(hash(j++,t0&0xffffff));
    cm.set(hash(j++,ta&0xffffff));
    cm.set(hash(j++,tc&0xffffff));
  }
  cm.mix(m);
  return 0;
}
virtual ~indirectModel1(){ }
};

//////////////////////////// dmcModel //////////////////////////

// Model using DMC.  The bitwise context is represented by a state graph,
// initilaized to a bytewise order 1 model as in
// http://plg.uwaterloo.ca/~ftp/dmc/dmc.c but with the following difference:
// - It uses integer arithmetic.
// - The threshold for cloning a state increases as memory is used up.
// - Each state maintains both a 0,1 count and a bit history (as in a
//   context model).  The 0,1 count is best for stationary data, and the
//   bit history for nonstationary data.  The bit history is mapped to
//   a probability adaptively using a StateMap.  The two computed probabilities
//   are combined.
// - When memory is used up the state graph is reinitialized to a bytewise
//   order 1 context as in the original DMC.  However, the bit histories
//   are not cleared.

class dmcModel1: public Model {
  struct DMCNode {  // 12 bytes
  unsigned int nx[2];  // next pointers
  U8 state;  // bit history
  unsigned int c0:12, c1:12;  // counts * 256
  };
  BlockData& x;
  Buf& buf;
  U32 top, curr;  // allocated, current node
  Array<DMCNode> t;  // state graph
  StateMap sm;
  int threshold; 
public:
  dmcModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), top(0), curr(0),
   t(CMlimit(MEM()*sizeof(DMCNode))/sizeof(DMCNode)), threshold(256)  {
  }
  int inputs() {return 2;}
int p(Mixer& m,int val1=0,int val2=0){
  // clone next state
  if (top>0 && top<t.size()) {
    int next=t[curr].nx[x.y];
    int n=x.y?t[curr].c1:t[curr].c0;
    int nn=t[next].c0+t[next].c1;
    if (n>=threshold*2 && nn-n>=threshold*3) {
      int r=n*4096/nn;
      assert(r>=0 && r<=4096);
      t[next].c0 -= t[top].c0 = t[next].c0*r>>12;
      t[next].c1 -= t[top].c1 = t[next].c1*r>>12;
      t[top].nx[0]=t[next].nx[0];
      t[top].nx[1]=t[next].nx[1];
      t[top].state=t[next].state;
      t[curr].nx[x.y]=top;
      ++top;
      if (top==(t.size()*4)/8) { //        5/8, 4/8, 3/8
        threshold=512;
      } else if (top==(t.size()*6)/8) { // 6/8, 6/8, 7/8
        threshold=768;
      }
    }
  }

  // Initialize to a bytewise order 1 model at startup or when flushing memory
  if (top==t.size() && x.bpos==1) top=0;
  if (top==0) {
    assert(t.size()>=65536);
    for (int i=0; i<256; ++i) {
      for (int j=0; j<256; ++j) {
        if (i<127) {
          t[j*256+i].nx[0]=j*256+i*2+1;
          t[j*256+i].nx[1]=j*256+i*2+2;
        }
        else {
          t[j*256+i].nx[0]=(i-127)*256;
          t[j*256+i].nx[1]=(i+1)*256;
        }
        t[j*256+i].c0=128;
        t[j*256+i].c1=128;
      }
    }
    top=65536;
    curr=0;
    threshold=256;
  }

  // update count, state
   if (x.y) {
    if (t[curr].c1<=3840) t[curr].c1+=256;
   } else  if (t[curr].c0<=3840)   t[curr].c0+=256;
  t[curr].state=nex(t[curr].state, x.y);
  curr=t[curr].nx[x.y];

  // predict
  const int pr1=sm.p(t[curr].state,x.y);
  const int n1=t[curr].c1;
  const int n0=t[curr].c0;
  const int pr2=(n1+5)*4096/(n0+n1+10);
  m.add(stretch(pr1));
  m.add(stretch(pr2));
  return 0;
}
virtual ~dmcModel1(){ }
};

class nestModel1: public Model {
  BlockData& x;
  Buf& buf;
  int ic, bc, pc,vc, qc, lvc, wc,ac, ec, uc, sense1, sense2, w;
  ContextMap cm;
public:
  nestModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), ic(0), bc(0),
   pc(0),vc(0), qc(0), lvc(0), wc(0),ac(0), ec(0), uc(0), sense1(0), sense2(0), w(0), cm(CMlimit(MEM()/2), 12)  {
  }
  int inputs() {return 12*cm.inputs();}
int p(Mixer& m,int val1=0,int val2=0){
    if (x.filetype==DBASE ||x.filetype==HDR ||x.filetype==DECA){
        for (int i=0; i<inputs(); i++)
        m.add(0);
        return 0;
    }
    if (x.bpos==0) {
    int c=x.c4&255, matched=1, vv;
    w*=((vc&7)>0 && (vc&7)<3);
    if (c&0x80) w = w*11*32 + c;
    const int lc = (c >= 'A' && c <= 'Z'?c+'a'-'A':c);
    if (lc == 'a' || lc == 'e' || lc == 'i' || lc == 'o' || lc == 'u'){ vv = 1; w = w*997*8 + (lc/4-22); } else
    if (lc >= 'a' && lc <= 'z'){ vv = 2; w = w*271*32 + lc-97; } else
    if (lc == ' ' || lc == '.' || lc == ',' || lc == '\n'|| lc == 5) vv = 3; else
    if (lc >= '0' && lc <= '9') vv = 4; else
    if (lc == 'y') vv = 5; else
    if (lc == '\'') vv = 6; else vv=(c&32)?7:0;
    vc = (vc << 3) | vv;
    if (vv != lvc) {
      wc = (wc << 3) | vv;
      lvc = vv;
    }
    switch(c) {
      case ' ': qc = 0; break;
      case '(': ic += 31; break;
      case ')': ic -= 31; break;
      case '[': ic += 11; break;
      case ']': ic -= 11; break;
      case '<': ic += 23; qc += 34; break;
      case '>': ic -= 23; qc /= 5; break;
      case ':': pc = 20; break;
      case '{': ic += 17; break;
      case '}': ic -= 17; break;
      case '|': pc += 223; break;
      case '"': pc += 0x40; break;
      case '\'': pc += 0x42; if (c!=(U8)(x.c4>>8)) sense2^=1; else ac+=(2*sense2-1); break;
      case '\n': pc = qc = 0; break;
      case '.': pc = 0; break;
      case '!': pc = 0; break;
      case '?': pc = 0; break;
      case '#': pc += 0x08; break;
      case '%': pc += 0x76; break;
      case '$': pc += 0x45; break;
      case '*': pc += 0x35; break;
      case '-': pc += 0x3; break;
      case '@': pc += 0x72; break;
      case '&': qc += 0x12; break;
      case ';': qc /= 3; break;
      case '\\': pc += 0x29; break;
      case '/': pc += 0x11;
                if (buf.size() > 1 && buf(1) == '<') qc += 74;
                break;
      case '=': pc += 87; if (c!=(U8)(x.c4>>8)) sense1^=1; else ec+=(2*sense1-1); break;
      default: matched = 0;
    }
    if (x.c4==0x266C743B) uc=min(7,uc+1);
    else if (x.c4==0x2667743B) uc-=(uc>0);
    if (matched) bc = 0; else bc += 1;
    if (bc > 300) bc = ic = pc = qc = uc = 0;

    cm.set(hash( (vv>0 && vv<3)?0:(lc|0x100), ic&0x3FF, ec&0x7, ac&0x7, uc ));
    cm.set(hash(ic, w, ilog2(bc+1)));
    cm.set((3*vc+77*pc+373*ic+qc)&0xffff);
    cm.set((31*vc+27*pc+281*qc)&0xffff);
    cm.set((13*vc+271*ic+qc+bc)&0xffff);
    cm.set((17*pc+7*ic)&0xffff);
    cm.set((13*vc+ic)&0xffff);
    cm.set((vc/3+pc)&0xffff);
    cm.set((7*wc+qc)&0xffff);
    cm.set((vc&0xffff)|((x.f4&0xf)<<16));
    cm.set(((3*pc)&0xffff)|((x.f4&0xf)<<16));
    cm.set((ic&0xffff)|((x.f4&0xf)<<16));
  }
    
    cm.mix(m);
  
  return 0;
}
virtual ~nestModel1(){ }
};


/*
====== XML model ======
*/


struct XMLAttribute {
  U32 Name, Value, Length;
};

struct XMLContent {
  U32 Data, Length, Type;
};

struct XMLTag {
  U32 Name, Length;
  int Level;
  bool EndTag, Empty;
  XMLContent Content;
  struct XMLAttributes {
    XMLAttribute Items[4];
    U32 Index;
  } Attributes;    
};

struct XMLTagCache {
  XMLTag Tags[CacheSize];
  U32 Index;
};

enum ContentFlags {
  Text        = 0x001,
  Number      = 0x002,
  Date        = 0x004,
  Time        = 0x008,
  URL         = 0x010,
  Link        = 0x020,
  Coordinates = 0x040,
  Temperature = 0x080,
  ISBN        = 0x100
};

enum XMLState {
  None               = 0,
  ReadTagName        = 1,
  ReadTag            = 2,
  ReadAttributeName  = 3,
  ReadAttributeValue = 4,
  ReadContent        = 5,
  ReadCDATA          = 6,
  ReadComment        = 7
};


#define DetectContent(){ \
  if ((x.c4&0xF0F0F0F0)==0x30303030){ \
    int i = 0, j = 0; \
    while ((i<4) && ( (j=(x.c4>>(8*i))&0xFF)>=0x30 && j<=0x39 )) \
      i++; \
\
    if (i==4 && ( ((c8&0xFDF0F0FD)==0x2D30302D && buf(9)>=0x30 && buf(9)<=0x39) || ((c8&0xF0FDF0FD)==0x302D302D) )) \
      (*Content).Type |= Date; \
  } \
  else if (((c8&0xF0F0FDF0)==0x30302D30 || (c8&0xF0F0F0FD)==0x3030302D) && buf(9)>=0x30 && buf(9)<=0x39){ \
    int i = 2, j = 0; \
    while ((i<4) && ( (j=(c8>>(8*i))&0xFF)>=0x30 && j<=0x39 )) \
      i++; \
\
    if (i==4 && (x.c4&0xF0FDF0F0)==0x302D3030) \
      (*Content).Type |= Date; \
  } \
\
  if ((x.c4&0xF0FFF0F0)==0x303A3030 && buf(5)>=0x30 && buf(5)<=0x39 && ((buf(6)<0x30 || buf(6)>0x39) || ((c8&0xF0F0FF00)==0x30303A00 && (buf(9)<0x30 || buf(9)>0x39)))) \
    (*Content).Type |= Time; \
\
  if ((*Content).Length>=8 && (c8&0x80808080)==0 && (x.c4&0x80808080)==0) \
    (*Content).Type |= Text; \
\
  if ((c8&0xF0F0FF)==0x3030C2 && (x.c4&0xFFF0F0FF)==0xB0303027){ \
    int i = 2; \
    while ((i<7) && buf(i)>=0x30 && buf(i)<=0x39) \
      i+=(i&1)*2+1; \
\
    if (i==10) \
      (*Content).Type |= Coordinates; \
  } \
\
  if ((x.c4&0xFFFFFA)==0xC2B042 && B!=0x47 && (((x.c4>>24)>=0x30 && (x.c4>>24)<=0x39) || ((x.c4>>24)==0x20 && (buf(5)>=0x30 && buf(5)<=0x39)))) \
    (*Content).Type |= Temperature; \
\
  if (B>=0x30 && B<=0x39) \
    (*Content).Type |= Number; \
\
  if (x.c4==0x4953424E && buf(5)==0x20) \
    (*Content).Type |= ISBN; \
} 
class XMLModel1: public Model {
  BlockData& x;
  Buf& buf;
  XMLTagCache Cache;
  XMLState State, pState;
  U32 c8, WhiteSpaceRun, pWSRun, IndentTab, IndentStep, LineEnding,lastState;
  ContextMap cm;
  U32 StateBH[8];
public:
  XMLModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), State(None), pState(None), c8(0),
   WhiteSpaceRun(0), pWSRun(0), IndentTab(0), IndentStep(2), LineEnding(2),lastState(0), cm(CMlimit(MEM()/4), 4) {
       memset(&Cache, 0, sizeof(XMLTagCache));
       memset(&StateBH, 0, sizeof(StateBH));        
  }
  int inputs() {return 4*cm.inputs();}
int p(Mixer& m,int val1=0,int val2=0){
    if (x.filetype==DBASE ||x.filetype==HDR ||x.filetype==DECA ){
        for (int i=0; i<inputs(); ++i)
        m.add(0);
        return 0;
    }
    if (x.bpos==0) {
    U8 B = (U8)x.c4;
    XMLTag *pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ], *Tag = &Cache.Tags[ Cache.Index&(CacheSize-1) ];
    XMLAttribute *Attribute = &((*Tag).Attributes.Items[ (*Tag).Attributes.Index&3 ]);
    XMLContent *Content = &(*Tag).Content;
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
    if (B==0x0A)
      LineEnding = 1+((U8)(x.c4>>8)==0x0D);
    if(State!=None) lastState=buf.pos;
    switch (State){
      case None : {
        if (B==0x3C){
          State = ReadTagName;
          memset(Tag, 0, sizeof(XMLTag));
          (*Tag).Level = ((*pTag).EndTag || (*pTag).Empty)?(*pTag).Level:(*pTag).Level+1;
        }
        if ((*Tag).Level>1)
          DetectContent();
        
        cm.set(hash(pState, State, ((*pTag).Level+1)*IndentStep - WhiteSpaceRun));
        break;
      }
      case ReadTagName : {
        if ((*Tag).Length>0 && (B==0x09 || B==0x0A || B==0x0D || B==0x20))
          State = ReadTag;
        else if ((B>127)||(B==0x3A || (B>='A' && B<='Z') || B==0x5F|| /*B==1|| B==2 ||*/ (B>='a' && B<='z')) || ((*Tag).Length>0 && (B==0x2D || B==0x2E || (B>='0' && B<='9')))){
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

        cm.set(hash(pState*8+State, (*Tag).Name, (*Tag).Level, (*pTag).Name, (*pTag).Level!=(*Tag).Level ));
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
        else if (B!=0x09 && B!=0x0A && B!=0x0D && B!=0x20){
          State = ReadAttributeName;
          (*Attribute).Name = B&0xDF;
        }
        cm.set(hash(pState, State, (*Tag).Name, B, (*Tag).Attributes.Index ));
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

        cm.set(hash(pState*8+State, (*Attribute).Name, (*Tag).Attributes.Index, (*Tag).Name, (*Content).Type ));
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
        cm.set(hash(pState, State, (*Attribute).Name, (*Content).Type ));
        break;
      }
      case ReadContent : {
        if (B==0x3C){
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
        cm.set(hash(pState, State, (*Tag).Name, x.c4&0xC0FF ));
        break;
      }
      case ReadCDATA : {
        if ((x.c4&0xFFFFFF)==0x5D5D3E){ // ]]>
          State = None;
          Cache.Index++;
        }
        cm.set(hash(pState, State));
        break;
      }
      case ReadComment : {
        if ((x.c4&0xFFFFFF)==0x2D2D3E){ // -->
          State = None;
          Cache.Index++;
        }
        cm.set(hash(pState, State));
        break;
      }
    }
    StateBH[pState] = (StateBH[pState]<<8)|B;
    pTag = &Cache.Tags[ (Cache.Index-1)&(CacheSize-1) ];
    // set context if last state was less then 256 bytes ago
    if ((buf.pos-lastState)<256){ 
        cm.set(hash(State, (*Tag).Level, pState*2+(*Tag).EndTag, (*Tag).Name));
        cm.set(hash((*pTag).Name, State*2+(*pTag).EndTag, (*pTag).Content.Type, (*Tag).Content.Type));
        cm.set(hash(State*2+(*Tag).EndTag, (*Tag).Name, (*Tag).Content.Type, x.c4&0xE0FF));
    }else {
        cm.set(0);
        cm.set(0);
        cm.set(0);
    }
  }
    cm.mix(m);

  U8 s = ((StateBH[State]>>(28-x.bpos))&0x08) |
         ((StateBH[State]>>(21-x.bpos))&0x04) |
         ((StateBH[State]>>(14-x.bpos))&0x02) |
         ((StateBH[State]>>( 7-x.bpos))&0x01) |
         ((x.bpos)<<4);
  return (s<<3)|State;
}
virtual ~XMLModel1(){ }
};

class sparseModelx: public Model {
   ContextMap cm;
   SmallStationaryContextMap scm1, scm2, scm3,
   scm4, scm5,scm6, scma;
   BlockData& x;
   Buf& buf;
public:
  sparseModelx(BlockData& bd): cm(CMlimit(MEM()*4), 31),scm1(0x10000), scm2(0x20000), scm3(0x2000),
     scm4(0x8000), scm5(0x2000),scm6(0x2000), scma(0x10000),x(bd),buf(bd.buf) {
    }
    int inputs() {return 31*cm.inputs()+7*2;}
  int p(Mixer& m, int seenbefore, int howmany){
  if (x.bpos==0) {
    scm5.set(seenbefore);
    scm6.set(howmany);
    U32 h=x.x4<<6;
    U32 d=x.c4&0xffff;
    if (x.c4==0 && x.x4==0){
        cm.set(0);
        cm.set(0);
        cm.set(0);
        cm.set(0);
        cm.set(0);
        cm.set(0);
    }else{
    
    cm.set(buf(1)+(h&0xffffff00));
    cm.set(buf(1)+(h&0x00ffff00));
    cm.set(buf(1)+(h&0x0000ff00));
    
    h<<=6;
    cm.set(d+(h&0xffff0000));
    cm.set(d+(h&0x00ff0000));
    h<<=6, d=x.c4&0xffffff;
    cm.set(d+(h&0xff000000));
}
    for (int i=1; i<5; ++i) { 
      cm.set(seenbefore|buf(i)<<8);
      cm.set((x.buf(i+3)<<8)|buf(i+1));
    }
    cm.set(x.spaces&0x7fff);
    cm.set(x.spaces&0xff);
    cm.set(x.words&0x1ffff);
    cm.set(x.f4&0x000fffff);
    cm.set(x.tt&0x00000fff);
    h=x.w4<<6;
    cm.set(buf(1)+(h&0xffffff00));
    cm.set(buf(1)+(h&0x00ffff00));
    cm.set(buf(1)+(h&0x0000ff00));
    d=x.c4&0xffff;
    h<<=6;
    cm.set(d+(h&0xffff0000));
    cm.set(d+(h&0x00ff0000));
    h<<=6, d=x.c4&0xffffff;
    cm.set(d+(h&0xff000000));
    cm.set(x.w4&0xf0f0f0ff);
    
    cm.set((x.w4&63)*128+(5<<17));
    cm.set((x.f4&0xffff)<<11|x.frstchar);
    cm.set(x.spafdo*8*((x.w4&3)==1));
    
    scm1.set(x.words&127);
    scm2.set((x.words&12)*16+(x.w4&12)*4+(x.f4&0xf));
    scm3.set(x.w4&15);
    scm4.set(x.spafdo*((x.w4&3)==1));
    scma.set(x.frstchar);
  }
  x.rm1=0;
  cm.mix(m);
  x.rm1=1;
  scm1.mix(m);
  scm2.mix(m);
  scm3.mix(m);
  scm4.mix(m);
  scm5.mix(m);
  scm6.mix(m);
  scma.mix(m);
  return 0;
  }
 virtual ~sparseModelx(){ }
};

static const int primes[17]={ 0, 257,251,241,239,233,229,227,223,211,199,197,193,191,181,179,173};   
class normalModel1: public Model {
  BlockData& x;
  Buf& buf;
   ContextMap  cm;
  RunContextMap rcm7, rcm9, rcm10;
  Array<U32> cxt; // order 0-11 contexts
  int inpt;
public:
  normalModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), cm(CMlimit(MEM()*32), 9),rcm7(CMlimit(MEM()/4),bd),
  rcm9(CMlimit(MEM()/4),bd), rcm10(CMlimit(MEM()/2),bd), cxt(16){
 }
 int inputs() {return 9*cm.inputs() +3;}
int p(Mixer& m,int val1=0,int val2=0){  
  if (x.bpos==0) {
    int i;
    if((buf(2)=='.'||buf(2)=='!'||buf(2)=='?' ||buf(2)=='}') && buf(3)!=10 && 
    (x.filetype==DICTTXT || x.filetype==BIGTEXT)) for (i=15; i>0; --i) 
      cxt[i]=cxt[i-1]*primes[i];
      
    for (i=15; i>0; --i)  // update order 0-11 context hashes
      cxt[i]=cxt[i-1]*primes[i]+(x.c4&255)+1;
    for (i=0; i<7; ++i)
      cm.set(cxt[i]);

    rcm7.set(cxt[7]);
    cm.set(cxt[8]);
    rcm9.set(cxt[10]);
    
    rcm10.set(cxt[12]);
    cm.set(cxt[14]);
  }
  rcm7.mix(m);
  rcm9.mix(m);
  rcm10.mix(m);
  return cm.mix(m);
}
 virtual ~normalModel1(){}
};
class decModel1: public Model {
  BlockData& x;
  Buf& buf;
   ContextMap  cm; 
  U32 currentOp,currentFunc; // order 0-11 contexts
  U32 lastOp,lastFunc; // order 0-11 contexts
  int inpt;
  bool valid,pvalid;
  int opbyte;
public:
  decModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf), cm(CMlimit(MEM()), 4) ,currentOp(-1),currentFunc(-1), lastOp(-1),lastFunc(-1),valid(false),pvalid(false),opbyte(0){
 }
 int inputs() {return 1*cm.inputs();}
int p(Mixer& m,int val1=0,int val2=0){  
 if (x.bpos==0) {
    U32 function=-1;
    U32 ins=m.x.c4;
    U8 B=m.x.c4&0xff;
    U32 opc= ins>> 26;
    opbyte=(opbyte+1)&3;
    if(valid==true && opbyte==3 ){
        lastOp=currentOp;
        lastFunc=currentFunc;
    }
    pvalid=valid;

    if (opc==0)   {  // PAL
        function = ins & 0x1fffffff;
        if( ((function > 0x3f) && (function < 0x80)) || (function > 0xbf))                             
        {                                                              
            currentOp=-1;   
            function=-1;                                                
        } 
        currentOp=opc;}  
    else if (opc==1)   {currentOp=-1,function=-1;  }  
    else if (opc==2)   {currentOp=-1,function=-1;}  
    else if (opc==3)   {currentOp=-1,function=-1;}  
    else if (opc==4)   {currentOp=-1,function=-1;}  
    else if (opc==5)   {currentOp=-1,function=-1;}  
    else if (opc==6)   {currentOp=-1,function=-1;}  
    else if (opc==7)   {currentOp=-1,function=-1;}  
    else if (opc==0x08) {currentOp=opc; }  
    else if (opc==0x09) {currentOp=opc; }  
    else if (opc==0x0a) {currentOp=opc; }  
    else if (opc==0x0b) {currentOp=opc; }  
    else if (opc==0x0c) {currentOp=opc; }  
    else if (opc==0x0d) {currentOp=opc; }  
    else if (opc==0x0e) {currentOp=opc; }  
    else if (opc==0x0f) {currentOp=opc; }  
    else if (opc==0x10) {

        op_10:  // INTA
        function = (ins >> 5) & 0x7f;
        switch(function) {
        case 0x40: 
        case 0x00: 
        case 0x02: 
        case 0x49: 
        case 0x09: 
        case 0x0b: 
        case 0x0f: 
        case 0x12: 
        case 0x1b: 
        case 0x1d: 
        case 0x60: 
        case 0x20: 
        case 0x22: 
        case 0x69: 
        case 0x29: 
        case 0x2b: 
        case 0x2d: 
        case 0x32: 
        case 0x3b: 
        case 0x3d: 
        case 0x4d: 
        case 0x6d: currentOp=opc;
        default:   currentOp=-1,function=-1; 
        }
    }
    else if (opc==0x11) {
        op_11:  // INTL
        function = (ins >> 5) & 0x7f;

        switch(function) {
        case 0x00: 
        case 0x08: 
        case 0x14: 
        case 0x16: 
        case 0x20: 
        case 0x24: 
        case 0x26: 
        case 0x28: 
        case 0x40: 
        case 0x44: 
        case 0x46: 
        case 0x48: 
        case 0x61: 
        case 0x64: 
        case 0x66: 
        case 0x6c:  currentOp=opc;
        default:    currentOp=-1,function=-1; 
        }}
    else if (opc==0x12) {
        op_12:  // INTS
        function = (ins >> 5) & 0x7f;

        switch(function) {
        case 0x02:  
        case 0x06:  
        case 0x0b:  
        case 0x12:  
        case 0x16:  
        case 0x1b:  
        case 0x22:  
        case 0x26:  
        case 0x2b:  
        case 0x30:  
        case 0x31:  
        case 0x32:  
        case 0x34:  
        case 0x36:  
        case 0x39:  
        case 0x3b:  
        case 0x3c:  
        case 0x52:  
        case 0x57:  
        case 0x5a:  
        case 0x62:  
        case 0x67:  
        case 0x6a:  
        case 0x72:  
        case 0x77:  
        case 0x7a:  currentOp=opc;
        default:    currentOp=-1,function=-1; 
        }}
    else if (opc==0x13) {
        op_13:  // INTM

        function = (ins >> 5) & 0x7f;
        switch(function) 
        {
        case 0x40: 
        case 0x00: 
        case 0x60: 
        case 0x20: 
        case 0x30:  currentOp=opc;
        default:   currentOp=-1,function=-1; 
        }}
    else if (opc==0x14) {
        op_14:   // ITFP
        function = (ins >> 5) & 0x7ff;

        switch(function) {
        case 0x004:
        case 0x00a:
        case 0x08a:
        case 0x10a:
        case 0x18a:
        case 0x40a:
        case 0x48a:
        case 0x50a:
        case 0x58a:
        case 0x00b:
        case 0x04b:
        case 0x08b:
        case 0x0cb:
        case 0x10b:
        case 0x14b:
        case 0x18b:
        case 0x1cb:
        case 0x50b:
        case 0x54b:
        case 0x58b:
        case 0x5cb:
        case 0x70b:
        case 0x74b:
        case 0x78b:
        case 0x7cb:
        case 0x014:
        case 0x024:
        case 0x02a:
        case 0x0aa:
        case 0x12a:
        case 0x1aa:
        case 0x42a:
        case 0x4aa:
        case 0x52a:
        case 0x5aa:
        case 0x02b:
        case 0x06b:
        case 0x0ab:
        case 0x0eb:
        case 0x12b:
        case 0x16b:
        case 0x1ab:
        case 0x1eb:
        case 0x52b:
        case 0x56b:
        case 0x5ab:
        case 0x5eb:
        case 0x72b:
        case 0x76b:
        case 0x7ab:
        case 0x7eb:
            currentOp=opc;

        default:
            currentOp=-1,function=-1; 
        }}
    else if (opc==0x15) {
        op_15:     // FLTV
        function = (ins >> 5) & 0x7ff;

        switch(function) {
        case 0x0a5:
        case 0x4a5:
        case 0x0a6:
        case 0x4a6:
        case 0x0a7:
        case 0x4a7:
        case 0x03c:
        case 0x0bc:
        case 0x03e:
        case 0x0be:
            currentOp=opc;

        default:
            if(function & 0x200)  currentOp=-1,function=-1; 

            switch(function & 0x7f) {
            case 0x000: 
            case 0x001: 
            case 0x002: 
            case 0x003: 
            case 0x01e: 
            case 0x020: 
            case 0x021: 
            case 0x022: 
            case 0x023: 
            case 0x02c: 
            case 0x02d: 
            case 0x02f: currentOp=opc;
            default:  currentOp=-1,function=-1; 
            }
            break;
        }}
    else if (opc==0x16) {
        op_16:    // FLTI
        function = (ins >> 5) & 0x7ff;  
        switch(function) {
        case 0x0a4:
        case 0x5a4:
        case 0x0a5:
        case 0x5a5:
        case 0x0a6:
        case 0x5a6:
        case 0x0a7:
        case 0x5a7:
        case 0x2ac:
        case 0x6ac:
            currentOp=opc;

        default:
            if(((function & 0x600) == 0x200) || ((function & 0x500) == 0x400)) currentOp=-1,function=-1; 

            switch(function & 0x3f) {
            case 0x00: 
            case 0x01: 
            case 0x02: 
            case 0x03: 
            case 0x20: 
            case 0x21: 
            case 0x22: 
            case 0x23: 
            case 0x2c: 
            case 0x2f:  currentOp=opc;
            case 0x3c:  if((function & 0x300) == 0x100){ currentOp=-1,function=-1;} currentOp=opc;
            case 0x3e:  if((function & 0x300) == 0x100){ currentOp=-1,function=-1;} currentOp=opc;
            default:    currentOp=-1,function=-1; 
            }
            break;
        }}
    else if (opc==0x17) {


        op_17:     // FLTL
        function = (ins >> 5) & 0x7ff;
        switch(function) {
        case 0x010:
        case 0x020:
        case 0x021:
        case 0x022:
        case 0x024:
        case 0x025:
        case 0x02a:
        case 0x02b:
        case 0x02c:
        case 0x02d:
        case 0x02e:
        case 0x02f:
        case 0x030:
        case 0x130:
        case 0x530:
            currentOp=opc;

        default:
            currentOp=-1,function=-1; 
        }}
    else if (opc==0x18) {

        op_18:    // MISC

        function = (ins & 0xffff);
        switch(function) {
        case 0x0000: 
        case 0x0400: 
        case 0x4000: 
        case 0x4400: 
        case 0x8000: 
        case 0xA000: 
        case 0xC000: 
        case 0xE000: 
        case 0xE800: 
        case 0xF000: 
        case 0xF800: 
        case 0xFC00: currentOp=opc;
        default:     currentOp=-1,function=-1; 
        }}
    else if (opc==0x19) {currentOp=opc;}
    else if (opc==0x1a) {currentOp=opc;}
    else if (opc==0x1b) {currentOp=opc;}
    else if (opc==0x1c) {
        op_1c:   // FPTI
        function = (ins >> 5) & 0x7f;

        switch(function) {
        case 0x00: 
        case 0x01: 
        case 0x30: 
        case 0x31: 
        case 0x32: 
        case 0x33: 
        case 0x34: 
        case 0x35: 
        case 0x36: 
        case 0x37: 
        case 0x38: 
        case 0x39: 
        case 0x3a: 
        case 0x3b: 
        case 0x3c: 
        case 0x3d: 
        case 0x3e: 
        case 0x3f: 
        case 0x70: 
        case 0x78:  currentOp=opc;
        default:    currentOp=-1,function=-1; 
        }}
    else if (opc>=0x1d && opc<=0x3f) currentOp=opc;
    else currentOp=-1,function=-1;

    if (currentOp!=-1 &&  opbyte==3) valid=true;
    else valid=false;

    cm.set(hash(currentOp,lastOp,function,opbyte));
    cm.set(hash(currentOp,function,opbyte,m.x.buf(8+opbyte)&0xFC));
    cm.set(hash(valid+16*opbyte,lastOp));
    cm.set(hash((0x100|B)*(opbyte>0),valid+256*opbyte));
  }
  cm.mix(m);
  return valid;
}
 virtual ~decModel1(){}
};


#include "mod_ppmd.inc"
class ppmdModel1: public Model {
  BlockData& x;
  Buf& buf;
  ppmd_Model ppmd_12_256_1;
public:
  ppmdModel1(BlockData& bd,U32 val=0):x(bd),buf(bd.buf){
    int ppmdmem=(210<<(x.clevel>8))<<(x.clevel>13);
    ppmd_12_256_1.Init(12+(x.clevel>8),ppmdmem,1,0);
 }
 int inputs() {return 1;}
int p(Mixer& m,int val1=0,int val2=0){
  m.add(stretch(4096-ppmd_12_256_1.ppmd_Predict(4096,x.y)));
  return 0;
}
  virtual ~ppmdModel1(){ }
};

//////////////////////////// Predictor /////////////////////////
// A Predictor estimates the probability that the next bit of
// uncompressed data is 1.  Methods:
// p() returns P(1) as a 12 bit number (0-4095).
// update(y) trains the predictor with the actual bit (0 or 1).

//base class
class Predictors {
public:
  BlockData x; //maintains current global data block between models
  //list of all models
  recordModel1* recordModel;
  im8bitModel1* im8bitModel;
  im24bitModel1* im24bitModel;
  recordModelx* recordModelw;
  sparseModelx* sparseModel1;
  jpegModelx* jpegModel;
  wavModel1* wavModel;
  matchModel1* matchModel;
  distanceModel1* distanceModel;
  sparseModely* sparseModel;
  wordModel1* wordModel;
  exeModel1* exeModel;
  indirectModel1* indirectModel;
  dmcModel1* dmcModel;
  nestModel1* nestModel;
  normalModel1* normalModel;
  im1bitModel1* im1bitModel;
  XMLModel1* XMLModel;
  ppmdModel1* ppmdModel;
  im4bitModel1* im4bitModel;
  TextModel *textModel;
  decModel1 *decModel;
virtual ~Predictors(){
  if (jpegModel!=0) delete jpegModel;
  if (sparseModel1!=0) delete sparseModel1;
  if (im8bitModel!=0) delete im8bitModel;
  if (im24bitModel!=0) delete im24bitModel;
  if (recordModelw!=0) delete recordModelw;
  if (exeModel!=0) delete exeModel;
  if (recordModel!=0) delete recordModel; 
  if (distanceModel!=0) delete distanceModel; 
  if (sparseModel!=0) delete sparseModel; 
  if (wordModel!=0) delete wordModel; 
  if (indirectModel!=0) delete indirectModel; 
  if (dmcModel!=0) delete dmcModel; 
  if (nestModel!=0) delete nestModel; 
  if (matchModel!=0) delete matchModel;
  if (normalModel!=0) delete normalModel;
  if (im1bitModel!=0) delete im1bitModel; 
  if (ppmdModel!=0) delete ppmdModel; 
  if (XMLModel!=0) delete XMLModel; 
  if (im4bitModel!=0) delete im4bitModel;
  if (textModel!=0) delete textModel;
  if (decModel!=0) delete decModel;
 
   };
Predictors(){
  recordModel=0;
  im8bitModel=0;
  im24bitModel=0;
  recordModelw=0;
  recordModel=0;
  sparseModel1=0;
  jpegModel=0;
  matchModel=0;
  distanceModel=0;
  sparseModel=0;
  wordModel=0;
  exeModel=0;
  indirectModel=0;
  dmcModel=0;
  nestModel=0;
  normalModel=0;
  im1bitModel=0;
  ppmdModel=0;
  XMLModel=0;
  im4bitModel=0;
  textModel=0;
  decModel=0;
}
  virtual int p() const =0;
  virtual void update()=0;
  void update0(){
    // Update global context: pos, bpos, c0, c4, buf
    x.c0+=x.c0+x.y;
    if (x.c0>=256) {
        x.buf[x.buf.pos++]=x.c0;
        x.c0=1;
        ++x.blpos;
      x.buf.pos=x.buf.pos&x.buf.poswr; //wrap
    }
    x.bpos=(x.bpos+1)&7;
  }
};


// Filter the context model with APMs
class EAPM {
  BlockData& x;
  APM1 a, a1, a2, a3, a4, a5, a6;
public:
  EAPM(BlockData& bd);
  int p1(int pr0,int pr, int r) ;
  int p2(int pr0,int pr, int r) ;
   ~EAPM(){
  }
};

EAPM::EAPM(BlockData& bd):x(bd),a(256,x), a1(0x10000,x), a2(0x10000,x),
 a3(0x10000,x), a4(0x10000,x), a5(0x10000,x), a6(0x10000,x) {
}

int EAPM::p1(int pr0,int pr, int r){
    if (x.fails&0x00000080) --x.failcount;
    x.fails=x.fails*2;
    x.failz=x.failz*2;
    if (x.y) pr^=4095;
    if (pr>=1820) ++x.fails, ++x.failcount;
    if (pr>= 848) ++x.failz;
    int pv, pu,pz,pt;
    pu=(a.p(pr0, x.c0, 3)+7*pr0+4)>>3, pz=x.failcount+1;
    pz+=tri[(x.fails>>5)&3];
    pz+=trj[(x.fails>>3)&3];
    pz+=trj[(x.fails>>1)&3];
    if (x.fails&1) pz+=8;
    pz=pz/2;      
  

    pu=a4.p(pu,   ( (x.c0*2)^hash(x.buf(1), (x.x5>>8)&255, (x.x5>>16)&0x80ff))&0xffff,r);
    pv=a2.p(pr0,  ( (x.c0*8)^hash(29,x.failz&2047))&0xffff,1+r);
    pv=a5.p(pv,           hash(x.c0,x.w5&0xfffff)&0xffff,r);
    pt=a3.p(pr0, ( (x.c0*32)^hash(19,     x.x5&0x80ffff))&0xffff,r);
    pz=a6.p(pu,   ( (x.c0*4)^hash(min(9,pz),x.x5&0x80ff))&0xffff,r);
    if (x.fails&255)  pr =(pt*6+pu  +pv*11+pz*14 +16)>>5;
    else              pr =(pt*4+pu*5+pv*12+pz*11 +16)>>5;
    return pr;
}
int EAPM::p2(int pr0,int pr8, int r){

  int pr=a.p(pr0, x.c0);

  int pr1=a1.p(pr0, x.c0+256*x.buf(1));
  int pr2=a2.p(pr0, (x.c0^hash(x.buf(1), x.buf(2)))&0xffff);
  int pr3=a3.p(pr0, (x.c0^hash(x.buf(1), x.buf(2), x.buf(3)))&0xffff);
  pr0=(pr0+pr1+pr2+pr3+2)>>2;

  pr1=a4.p(pr, x.c0+256*x.buf(1));
  pr2=a5.p(pr, (x.c0^hash(x.buf(1), x.buf(2)>>1))&0xffff);
  pr3=a6.p(pr, (x.c0^hash(x.buf(1), x.buf(2)>>1, x.buf(3)>>2))&0xffff);
  pr=(pr+pr1+pr2+pr3+2)>>2;

  pr=(pr+pr0+1)>>1;
  return pr;
}

//general predicor class
class Predictor: public Predictors {
  int pr;  // next prediction
  int pr0;
  int order;
  int ismatch;
  Mixer *m;
  EAPM a;
   bool Bypass;
  void setmixer();
public:
  Predictor();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~Predictor(){
  }
};

Predictor::Predictor(): pr(2048),pr0(pr),order(0),ismatch(0), a(x), Bypass(false) {
    if (x.clevel>=4){
        recordModel=new recordModel1(x); 
        distanceModel=new distanceModel1(x); 
        sparseModel=new sparseModely(x); 
        wordModel=new wordModel1(x); 
        indirectModel=new indirectModel1(x); 
        dmcModel=new dmcModel1(x);
        nestModel=new nestModel1(x); 
        ppmdModel=new ppmdModel1(x,1);
        XMLModel=new XMLModel1(x);
        exeModel=new exeModel1(x); 
        textModel =new TextModel(x,16) ;
   }
   matchModel=new matchModel1(x);
   normalModel=new normalModel1(x);
   const int tinput=1+(x.clevel>=4?(recordModel->inputs() + distanceModel->inputs() +
   sparseModel->inputs() +wordModel->inputs()+indirectModel->inputs() + dmcModel->inputs()+
   nestModel->inputs()+ppmdModel->inputs()+XMLModel->inputs()+exeModel->inputs()+textModel->inputs() ):0) + matchModel->inputs() + normalModel->inputs();
   m=new Mixer(tinput,  7432+256+1024+1024+8+1024+1024+512+1024*5+2048+2048+2048+1024*3+8192+8192+8192+1024,x, 21+1);
}

void Predictor::update()  {
    update0();
    if (x.bpos==0) {
        int b1=x.buf(1);
        x.c4=(x.c4<<8)+b1;
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1>>4];
        x.f4=x.f4*16+(b1>>4);
        if(x.blpos==1) {
            m->setText(false);
           if (x.filetype==DBASE || x.filetype==DBASE) m->setText(true);
         }
    }

    m->update();
    Bypass=false;
    m->add(256);
    ismatch=matchModel->p(*m);  // Length of longest matching context
    if (ismatch>0xFFF || matchModel->Bypass) {
    matchModel->Bypass = Bypass = true;
    m->reset();
    pr= matchModel->BypassPrediction;
    return;
  }
    ismatch=ilog(ismatch); // ilog of length of most recent match (0..255)
    order=normalModel->p(*m);
    order=order-2; if(order<0) order=0;if(order>8) order=7;
    int rlen=0,Valid=0,xmlstate=0;
    if (x.clevel>=4){        
            int dataRecordLen=(x.filetype==DBASE)?x.finfo:(x.filetype==IMGUNK)?x.finfo:0; //force record length 
            rlen=recordModel->p(*m,dataRecordLen);
            wordModel->p(*m,0,x.finfo>0?x.finfo:0); //col
            sparseModel->p(*m,ismatch,order);
            distanceModel->p(*m);
            indirectModel->p(*m);
            nestModel->p(*m);
            dmcModel->p(*m);
            ppmdModel->p(*m);
            xmlstate=XMLModel->p(*m);
            Valid=exeModel->p(*m);
            textModel->p(*m);
    } 
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    m->set(rlen, 1024);
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+128*Valid, 256);  
    U8 d=x.c0<<(8-x.bpos);
    m->set(((xmlstate&3)>0)*1024+(x.bpos>0)*512+(order>3)*256+(x.w4&240)+(x.b3>>4),2048);
    m->set(x.bpos*256+((x.words<<x.bpos&255)>>x.bpos|(d&255)),2048);
    m->set(ismatch, 256);
    if (x.bpos) {
      c=d; if (x.bpos==1)c+=c3/2;
      c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    pr0=m->p();
    pr=a.p1(pr0,pr,7);
}

//general predicor class
class PredictorDEC: public Predictors {
  int pr;  // next prediction
  int pr0;
  int order;
  int ismatch;
  Mixer *m;
  EAPM a;
   
  void setmixer();
public:
  PredictorDEC();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorDEC(){
  }
};

PredictorDEC::PredictorDEC(): pr(2048),pr0(pr),order(0),ismatch(0), a(x) {
    if (x.clevel>=4){
        recordModel=new recordModel1(x); 
        distanceModel=new distanceModel1(x); 
        sparseModel=new sparseModely(x); 
        wordModel=new wordModel1(x); 
        indirectModel=new indirectModel1(x); 
        dmcModel=new dmcModel1(x);
        ppmdModel=new ppmdModel1(x,1);
        decModel=new decModel1(x);
   }
   matchModel=new matchModel1(x);
   normalModel=new normalModel1(x);
   const int tinput=1+(x.clevel>=4?(recordModel->inputs() + distanceModel->inputs() +
   sparseModel->inputs() +wordModel->inputs()+indirectModel->inputs() + dmcModel->inputs()+
   ppmdModel->inputs()+decModel->inputs() ):0) + matchModel->inputs() + normalModel->inputs();
   m=new Mixer(tinput,  7432+256+1024+1024+8+1024+1024+512+1024*4+2048+2048+2048-1024,x, 7 +2+1+1+1+4+2);
}

void PredictorDEC::update()  {
    update0();
    if (x.bpos==0) {
        int b1=x.buf(1);
        x.c4=(x.c4<<8)+b1;
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1>>4];
        x.f4=x.f4*16+(b1>>4);
    }

    m->update();
    m->add(256);
    ismatch=ilog(matchModel->p(*m));  // Length of longest matching context
    order=normalModel->p(*m);
    order=order-2; if(order<0) order=0;if(order>8) order=7;
    int rlen=0,Valid=0;
    if (x.clevel>=4){        
            rlen=recordModel->p(*m,4);
            wordModel->p(*m,0,4); //col
            sparseModel->p(*m,ismatch,order);
            distanceModel->p(*m);
            indirectModel->p(*m);
            dmcModel->p(*m);
            ppmdModel->p(*m);
            Valid=decModel->p(*m);
     
    } 
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    m->set(rlen, 1024);
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+128*Valid, 256);  
    U8 d=x.c0<<(8-x.bpos);
    m->set((x.bpos>0)*512+(order>3)*256+(x.w4&240)+(x.b3>>4),1024);
    m->set(x.bpos*256+((x.words<<x.bpos&255)>>x.bpos|(d&255)),2048);
    m->set(ismatch, 256);
    if (x.bpos) {
      c=d; if (x.bpos==1)c+=c3/2;
      c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    pr0=m->p();
    pr=a.p1(pr0,pr,7);
}
//JPEG predicor class
class PredictorJPEG: public Predictors {
  int pr;  // next prediction
  Mixer *m;
  EAPM a;
public:
  PredictorJPEG();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
  ~PredictorJPEG(){  
 }
};

PredictorJPEG::PredictorJPEG(): pr(2048) ,  a(x)  {
  matchModel=new matchModel1(x); 
  jpegModel=new jpegModelx(x); 
  const int tinput=1+ matchModel->inputs() + jpegModel->inputs() ;
  m=new Mixer(tinput, 2568+1024+1025+9-256-257+1024,x, 5);
}

void PredictorJPEG::update()  {
    update0();
    
    m->update();
    m->add(256);
    int ismatch=ilog(matchModel->p(*m));  // Length of longest matching context
    if (jpegModel->p(*m)) { 
        m->set(ismatch, 256);
        pr=m->p(1,0);
    }
    else{
        U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
         m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
        m->set(x.c0, 256);
        m->set(c2, 256);
        m->set(ismatch, 256);
        U8 d=x.c0<<(8-x.bpos);
        if (x.bpos) {
            c=d; if (x.bpos==1)c+=c3/2;
            c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
        }
        else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
        m->set(c, 1536);
        pr=a.p2(m->p(),pr,7);
    }
}

//EXE predicor class
class PredictorEXE: public Predictors {
  int pr;  // next prediction
  int order;
  Mixer *m;
  EAPM a;
  void setmixer();
public:
  PredictorEXE();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
    ~PredictorEXE(){ }
};

PredictorEXE::PredictorEXE(): pr(2048),order(0),a(x) {
  if (x.clevel>=4){
    recordModel=new recordModel1(x); 
    distanceModel=new distanceModel1(x); 
    sparseModel=new sparseModely(x); 
    wordModel=new wordModel1(x); 
    exeModel=new exeModel1(x); 
    indirectModel=new indirectModel1(x);
    dmcModel=new dmcModel1(x);  
    nestModel=new nestModel1(x); // ?
  }
  matchModel=new matchModel1(x); 
  normalModel=new normalModel1(x);
  const int tinput=1+(x.clevel>=4?(recordModel->inputs()+distanceModel->inputs()+sparseModel->inputs()+
  wordModel->inputs()+  exeModel->inputs() + indirectModel->inputs() + dmcModel->inputs()+ nestModel->inputs() ):0)+
  matchModel->inputs() + normalModel->inputs() ;
  m=new Mixer(tinput, 6920+1024+512+8192+8192+8192,x, 10+1+1+2+1);
}

void PredictorEXE::update()  {
    update0();
    if (x.bpos==0) {
        int b1=x.buf(1);
        x.c4=(x.c4<<8)+b1;
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1>>4];
        x.f4=x.f4*16+(b1>>4);
    }
    m->update();
    m->add(256);
    int ismatch=ilog(matchModel->p(*m));  // Length of longest matching context
    order=normalModel->p(*m);
    order=order-2; if(order<0) order=0;if(order>8) order=7;
    int rec=0;
    if (x.clevel>=4 ){
        rec=recordModel->p(*m);
        wordModel->p(*m,order);
        nestModel->p(*m);
        sparseModel->p(*m,ismatch,order);
        distanceModel->p(*m);
        indirectModel->p(*m);
        dmcModel->p(*m);
        exeModel->p(*m,1); //1024*2
    }
    U32 c1=x.buf(1), c2=x.buf(2), c3=x.buf(3), c;
    m->set(8+ c1 + (x.bpos>5)*256 + ( ((x.c0&((1<<x.bpos)-1))==0) || (x.c0==((2<<x.bpos)-1)) )*512, 8+1024);
    m->set(x.c0, 256);
    m->set(c2, 256);
    m->set(rec, 1024);
    U8 d=x.c0<<(8-x.bpos);
    m->set(order+8*(x.c4>>6&3)+32*(x.bpos==0)+64*(c1==c2)+1*128, 256);
    m->set(c3, 256);
    m->set(ismatch, 256);
    if (x.bpos) {
        c=d; if (x.bpos==1)c+=c3/2;
        c=(min(x.bpos,5))*256+c1/32+8*(c2/32)+(c&192);
    }
    else c=c3/128+(x.c4>>31)*2+4*(c2/64)+(c1&240); 
    m->set(c, 1536);
    int pr0=m->p();
    pr=a.p2(pr0,pr,7);
}

//IMG4 predicor class
class PredictorIMG4: public Predictors {
  int pr;  // next prediction
  Mixer *m;
  EAPM a;
public:
  PredictorIMG4();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorIMG4(){
  }
};

PredictorIMG4::PredictorIMG4(): pr(2048),  a(x){
  matchModel=new matchModel1(x);   
  im4bitModel=new im4bitModel1(x); 
  const int tinput=1+ matchModel->inputs()+im4bitModel->inputs();
  m=new Mixer(tinput,2576,x, 6 );
}

void PredictorIMG4::update()  {
  update0();
  if (x.bpos==0) {
    x.c4=(x.c4<<8)+x.buf(1);
  }
  m->update();
  m->add(256);
  matchModel->p(*m);  // Length of longest matching context
  im4bitModel->p(*m,x.finfo);
  int pr0=m->p();
  pr=a.p2(pr0,pr,7);
}

//IMG8 predicor class
class PredictorIMG8: public Predictors {
  int pr;  // next prediction
  int b8g;
  Mixer *m;
  EAPM a;
public:
  PredictorIMG8();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorIMG8(){
  }
};

PredictorIMG8::PredictorIMG8(): pr(2048),b8g(0),  a(x) {
  matchModel=new matchModel1(x);   
  im8bitModel=new im8bitModel1(x); 
  const int tinput=1+ matchModel->inputs() + im8bitModel->inputs() ;
  m=new Mixer(tinput, 3877+64+128 ,x, 7+1 );
}

void PredictorIMG8::update()  {
  update0();
  if (x.bpos==0) {
    x.c4=(x.c4<<8)+x.buf(1);
  }
   
  m->update();
  m->add(256);
  int ismatch=matchModel->p(*m);  // Length of longest matching context
  m->set(ilog(ismatch), 256);
  im8bitModel->p(*m,x.finfo);
  int pr0=m->p();
  pr=a.p2(pr0,pr,7);
}

//IMG24 predicor class
class PredictorIMG24: public Predictors {
  int pr;  // next prediction
  Mixer *m;
  struct {
    APM<24> APMs[4];
    APM1 APM1s[2];
  } Image;
public:
  PredictorIMG24();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorIMG24(){
  }
};

PredictorIMG24::PredictorIMG24(): pr(2048),Image{ {0x1000, 0x10000, 0x10000, 0x10000}, {{0x10000,x}, {0x10000,x}} } {
  matchModel=new matchModel1(x); 
  im24bitModel=new im24bitModel1(x);
  const int tinput=1+matchModel->inputs() + im24bitModel->inputs() ;
  m=new Mixer(tinput, 5254+4*8192,x,9+4);
}

void PredictorIMG24::update()  {
  int pr1, pr2, pr3;
  update0();
  x.Misses+=x.Misses+((pr>>11)!=x.y);
  if (x.bpos==0) {
    int b1=x.buf(1);
    x.c4=(x.c4<<8)+b1;
        int i=WRT_mpw[b1>>4];
        x.w5=x.w5*4+i;
        x.x5=(x.x5<<8)+b1;
}
   
  m->update();
  m->add(256);
  int ismatch=matchModel->p(*m);  // Length of longest matching context
  m->set(ilog(ismatch),256);
  im24bitModel->p(*m,x.finfo);
  
  int pr0=x.filetype==IMAGE24? m->p(1,1): m->p();
  int limit=0x3FF>>((x.blpos<0xFFF)*4);
      pr  = Image.APMs[0].p(pr0, (x.c0<<4)|(x.Misses&0xF), x.y,limit);
      pr1 = Image.APMs[1].p(pr0, hash(x.c0, x.Image.pixels.W, x.Image.pixels.WW)&0xFFFF,  x.y,limit);
      pr2 = Image.APMs[2].p(pr0, hash(x.c0, x.Image.pixels.N, x.Image.pixels.NN)&0xFFFF, x.y, limit);
      pr3 = Image.APMs[3].p(pr0, (x.c0<<8)|x.Image.ctx, x.y, limit);

      pr0 = (pr0+pr1+pr2+pr3+2)>>2;

      pr1 = Image.APM1s[0].p(pr, hash(x.c0, x.Image.pixels.W, x.buf(1)-x.Image.pixels.Wp1, x.Image.plane)&0xFFFF);
      pr2 = Image.APM1s[1].p(pr, hash(x.c0, x.Image.pixels.N, x.buf(1)-x.Image.pixels.Np1, x.Image.plane)&0xFFFF);

      pr=(pr*2+pr1*3+pr2*3+4)>>3;
      pr = (pr+pr0+1)>>1;

}
//TEXT predicor class
class PredictorTXTWRT: public Predictors {
  int pr;  // next prediction
  int pr0;
  int order;
  int ismatch;
  Mixer *m;
  EAPM a;
  struct {
    APM<24> APMs[4];
    APM1 APM1s[3];
  } Text;
  bool Bypass;
  void setmixer();
public:
  PredictorTXTWRT();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorTXTWRT(){
  }
};

PredictorTXTWRT::PredictorTXTWRT(): pr(2048),pr0(pr),order(0),ismatch(0), a(x),Text{ {0x10000, 0x10000, 0x10000, 0x10000}, {{0x10000,x}, {0x10000,x}, {0x10000,x}} },Bypass(false) {
  if (x.clevel>=4){
    recordModelw=new recordModelx(x);
    sparseModel1=new sparseModelx(x);
    wordModel=new wordModel1(x);
    indirectModel=new indirectModel1(x);
    dmcModel=new dmcModel1(x);
    nestModel=new nestModel1(x);
    ppmdModel=new ppmdModel1(x);
    XMLModel=new XMLModel1(x);
    textModel =new TextModel(x,16) ;
  }
  matchModel=new matchModel1(x);
  normalModel=new normalModel1(x);
  const int tinput=1+(x.clevel>=4?(recordModelw->inputs() + sparseModel1->inputs() +
  wordModel->inputs()+ indirectModel->inputs() + dmcModel->inputs()+nestModel->inputs()+
  ppmdModel->inputs()+ XMLModel->inputs()+ textModel->inputs()):0) + matchModel->inputs() + normalModel->inputs()  ;
  m=new Mixer(tinput, 10752-512+1024*4+2048+2048+2048+ 1024*4+1024,x, 7 +3+1+2+1);
   
  
  m->setText(true);
}

void PredictorTXTWRT::update()  {
    update0();
    if (x.bpos==0) {
        int b1=x.buf(1);
        x.c4=(x.c4<<8)+b1;
        int i=WRT_mpw[b1>>4];
        x.w4=x.w4*4+i;
        if (x.b2==3) i=2;
        x.w5=x.w5*4+i;
        x.b4=x.b3;
        x.b3=x.b2;
        x.b2=b1;   
        x.x4=x.x4*256+b1,x.x5=(x.x5<<8)+b1;
        if(b1=='.' || b1=='!' || b1=='?' || b1=='/'|| b1==')'|| b1=='}') {
            x.w5=(x.w5<<8)|0x3ff,x.f4=(x.f4&0xfffffff0)+2,x.x5=(x.x5<<8)+b1,x.x4=x.x4*256+b1;
            if(b1!='!') x.w4|=12, x.tt=(x.tt&0xfffffff8)+1,x.b3='.';
        }
        if (b1==32) --b1;
        x.tt=x.tt*8+WRT_mtt[b1>>4];
        x.f4=x.f4*16+(b1>>4);
    }

    m->update();
    Bypass=false;
    m->add(256);
    ismatch= (matchModel->p(*m));  // Length of longest matching context
    if (ismatch>0xFFF || matchModel->Bypass) {
    matchModel->Bypass = Bypass = true;
    m->reset();
    pr= matchModel->BypassPrediction;
    return;
  }
    ismatch=ilog(ismatch);  // Length of longest matching context
    order=normalModel->p(*m); //ord ma
    order=order-3; 
    if(order<0) order=0;
    if(order>8) order=7;
    if (x.clevel>=4){   
        int state=XMLModel->p(*m);
        wordModel->p(*m,state&7);     
        sparseModel1->p(*m,ismatch,order);
        nestModel->p(*m);
        indirectModel->p(*m);
        dmcModel->p(*m);
        recordModelw->p(*m);
        ppmdModel->p(*m);
        textModel->p(*m,state&7);
        
    }    
        U32 c3=x.buf(3), c;
        c=(x.words>>1)&63;
        m->set(x.c0, 256);// 256 +256 +512+256*8 +256*8 +256*8 +1536+ 2048
        m->set(ismatch, 256);
        m->set((x.w4&3)*64+c+order*256, 256*8);
        m->set(256*order + (x.w4&240) + (x.b3>>4), 256*8);
        m->set((x.w4&255)+256*x.bpos, 256*8);
        if (x.bpos){
            c=x.c0<<(8-x.bpos); if (x.bpos==1)c+=x.b4/2;
            c=(min(x.bpos,5))*256+(x.tt&63)+(c&192);
        }
        else c=(x.words&12)*16+(x.tt&63);
        m->set(c, 1536);
        c=x.bpos*256+((x.c0<<(8-x.bpos))&255);
        c3 = (x.words<<x.bpos) & 255;
        m->set(c+(c3>>x.bpos), 2048);
        pr0=m->p();
        pr=a.p2(pr0,pr,7);
}

//IMG1 predicor class
class PredictorIMG1: public Predictors {
  int pr;  // next prediction
  Mixer *m;
public:
  PredictorIMG1();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorIMG1(){
  }
};

PredictorIMG1::PredictorIMG1(): pr(2048)  {
  matchModel=new matchModel1(x); 
  im1bitModel=new im1bitModel1(x); 
  const int tinput=1+ matchModel->inputs() + im1bitModel->inputs() ;
  m=new Mixer(tinput,  12801+256+1,x, 7 );
}

void PredictorIMG1::update()  {
  update0();
  m->update();
  m->add(256);
  int ismatch=matchModel->p(*m);
  m->set(ilog(ismatch),256);
  im1bitModel->p(*m, x.finfo);
  pr=m->p(); 
}

class PredictorAUDIO2: public Predictors {
  int pr;
  Mixer *m;
  EAPM a;
  void setmixer();
public:
  PredictorAUDIO2();
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  void update() ;
   ~PredictorAUDIO2(){
  }
};

PredictorAUDIO2::PredictorAUDIO2(): pr(2048),a(x) {
  wavModel=new wavModel1(x); 
  matchModel=new matchModel1(x);
  const int tinput=1+wavModel->inputs()+ matchModel->inputs() ;
  m=new Mixer(tinput,  3095+256*7+256*8+256*7+2048+256*3-264-8+4*8+512,x, 5+1);
}

void PredictorAUDIO2::update()  {
  update0();
  m->update();
  m->add(256);
  matchModel->p(*m);  
  wavModel->p(*m,x.finfo);
  pr=a.p1(m->p(),pr,7);
}

//////////////////////////// Encoder ////////////////////////////

// An Encoder does arithmetic encoding.  Methods:
// Encoder(COMPRESS, f) creates encoder for compression to archive f, which
//   must be open past any header for writing in binary mode.
// Encoder(DECOMPRESS, f) creates encoder for decompression from archive f,
//   which must be open past any header for reading in binary mode.
// code(i) in COMPRESS mode compresses bit i (0 or 1) to file f.
// code() in DECOMPRESS mode returns the next decompressed bit from file f.
//   Global y is set to the last bit coded or decoded by code().
// compress(c) in COMPRESS mode compresses one byte.
// decompress() in DECOMPRESS mode decompresses and returns one byte.
// flush() should be called exactly once after compression is done and
//   before closing f.  It does nothing in DECOMPRESS mode.
// size() returns current length of archive
// setFile(f) sets alternate source to FILE* f for decompress() in COMPRESS
//   mode (for testing transforms).
// If level (global) is 0, then data is stored without arithmetic coding.

typedef enum {COMPRESS, DECOMPRESS} Mode;
class Encoder {
private:
  const Mode mode;       // Compress or decompress?
  File* archive;         // Compressed data file
  U32 x1, x2;            // Range, initially [0, 1), scaled by 2^32
  U32 x;                 // Decompress mode: last 4 input bytes of archive
  File*alt;             // decompress() source in COMPRESS mode

  // Compress bit y or return decompressed bit
  void code(int i=0) {
    int p=predictor.p();
    p+=p==0;
    assert(p>0 && p<4096);
    U32 xmid=x1 + ((x2-x1)>>12)*p + (((x2-x1)&0xfff)*p>>12);
    //U32 xmid=x1 + ((((x2-x1)*U64( U32(p)))>>12));// is it really safe ?
    assert(xmid>=x1 && xmid<x2);
    predictor.x.y=i;
    i ? (x2=xmid) : (x1=xmid+1);
    predictor.update();
    while (((x1^x2)&0xff000000)==0) {  // pass equal leading bytes of range
      archive->putc(x2>>24);
      x1<<=8;
      x2=(x2<<8)+255;
    }
  }
  int decode() {
    int p=predictor.p();
    p+=p==0;
    assert(p>0 && p<4096);
    U32 xmid=x1 + ((x2-x1)>>12)*p + (((x2-x1)&0xfff)*p>>12);
  //  U32 xmid=x1 + ((((x2-x1)*U64( U32(p)))>>12)); // is it really safe ?
    assert(xmid>=x1 && xmid<x2);
    x<=xmid ? (x2=xmid,predictor.x.y=1) : (x1=xmid+1,predictor.x.y=0);
    predictor.update();
    while (((x1^x2)&0xff000000)==0) {  // pass equal leading bytes of range
      x1<<=8;
      x2=(x2<<8)+255;
      x=(x<<8)+(archive->getc()&255);  // EOF is OK
    }
    return predictor.x.y;
  }
 
public:
  Predictors& predictor;
  Encoder(Mode m, File* f,Predictors& predict);
  Mode getMode() const {return mode;}
  U64 size() const {return  archive->curpos();}  // length of archive so far
  void flush();  // call this when compression is finished
  void setFile(File* f) {alt=f;}

  // Compress one byte
  void compress(int c) {
    assert(mode==COMPRESS);
    if (level==0)
      archive->putc(c);
    else {
      for (int i=7; i>=0; --i)
        code((c>>i)&1);
    }
  }

  // Decompress and return one byte
  int decompress() {
    if (mode==COMPRESS) {
      assert(alt);
      return alt->getc();
    }
    else if (level==0){
     int a;
     a=archive->getc();
      return a ;}
    else {
      int c=0;
      for (int i=0; i<8; ++i)
        c+=c+decode();
      
      return c;
    }
  }
  ~Encoder(){
  
   }
};

Encoder::Encoder(Mode m, File* f,Predictors& predict):
    mode(m), archive(f), x1(0), x2(0xffffffff), x(0), alt(0),predictor(predict) {
        
  if (level>0 && mode==DECOMPRESS) {  // x = first 4 bytes of archive
    for (int i=0; i<4; ++i)
      x=(x<<8)+(archive->getc()&255);
  }
  for (int i=0; i<1024; ++i)
    dt[i]=16384/(i+i+3);

}

void Encoder::flush() {
  if (mode==COMPRESS && level>0)
    archive->put32(x1);//putc(x1>>24),archive->putc(x1>>16),archive->putc(x1>>8),archive->putc(x1);  // Flush first unequal byte of range
}
 
/////////////////////////// Filters /////////////////////////////////
//
// Before compression, data is encoded in blocks with the following format:
//
//   <type> <size> <encoded-data>
//
// Type is 1 byte (type Filetype): DEFAULT=0, JPEG, EXE, ...
// Size is 4 bytes in big-endian format.
// Encoded-data decodes to <size> bytes.  The encoded size might be
// different.  Encoded data is designed to be more compressible.
//
//   void encode(File* in, File* out, int n);
//
// Reads n bytes of in (open in "rb" mode) and encodes one or
// more blocks to temporary file out (open in "wb+" mode).
// The file pointer of in is advanced n bytes.  The file pointer of
// out is positioned after the last byte written.
//
//   en.setFile(File* out);
//   int decode(Encoder& en);
//
// Decodes and returns one byte.  Input is from en.decompress(), which
// reads from out if in COMPRESS mode.  During compression, n calls
// to decode() must exactly match n bytes of in, or else it is compressed
// as type 0 without encoding.
//
//   Filetype detect(File* in, int n, Filetype type);
//
// Reads n bytes of in, and detects when the type changes to
// something else.  If it does, then the file pointer is repositioned
// to the start of the change and the new type is returned.  If the type
// does not change, then it repositions the file pointer n bytes ahead
// and returns the old type.
//
// For each type X there are the following 2 functions:
//
//   void encode_X(File* in, File* out, int n, ...);
//
// encodes n bytes from in to out.
//
//   int decode_X(Encoder& en);
//
// decodes one byte from en and returns it.  decode() and decode_X()
// maintain state information using static variables.

#define bswap(x) \
+   ((((x) & 0xff000000) >> 24) | \
+    (((x) & 0x00ff0000) >>  8) | \
+    (((x) & 0x0000ff00) <<  8) | \
+    (((x) & 0x000000ff) << 24))

#define IMG_DET(type,start_pos,header_len,width,height) return dett=(type),\
deth=int(header_len),detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),HDR
#define IMG_DETP(type,start_pos,header_len,width,height) return dett=(type),\
deth=int(header_len),detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),TEXT
 
#define DBS_DET(type,start_pos,header_len,datalen,reclen) return dett=(type),\
deth=int(header_len),detd=int(datalen),info=int(reclen),\
 in->setpos(start+(start_pos)),HDR

#define IMG_DETX(type,start_pos,header_len,width,height) return dett=(type),\
deth=-1,detd=int((width)*(height)),info=int(width),\
 in->setpos(start+(start_pos)),DEFAULT

#define AUD_DET(type,start_pos,header_len,data_len,wmode) return dett=(type),\
deth=int(header_len),detd=(data_len),info=(wmode),\
 in->setpos(start+(start_pos)),HDR

//Return only base64 data. No HDR.
#define B64_DET(type,start_pos,header_len,base64len) return dett=(type),\
deth=(-1),detd=int(base64len),\
 in->setpos(start+start_pos),DEFAULT

#define B85_DET(type,start_pos,header_len,base85len) return dett=(type),\
deth=(-1),detd=int(base85len),\
 in->setpos(start+start_pos),DEFAULT

#define SZ_DET(type,start_pos,header_len,base64len,unsize) return dett=(type),\
deth=(-1),detd=int(base64len),info=(unsize),\
 in->setpos(start+start_pos),DEFAULT

#define MRBRLE_DET(start_pos,header_len,data_len,width,height) return dett=(MRBR),\
deth=(header_len),detd=(data_len),info=(((width+3)/4)*4),info2=(height),\
 in->setpos(start+(start_pos)),HDR

#define TIFFJPEG_DET(start_pos,header_len,data_len) return dett=(JPEG),\
deth=(header_len),detd=(data_len),info=(-1),info2=(-1),\
 in->setpos(start+(start_pos)),HDR

#define NES_DET(type,start_pos,header_len,base64len) return dett=(type),\
deth=(-1),detd=int(base64len),\
 in->setpos(start+start_pos),DEFAULT

inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c=='+') || (c=='/')|| (c==10) || (c==13));
}

inline bool is_base85(unsigned char c) {
    return (isalnum(c) || (c==13) || (c==10) || (c=='y') || (c=='z') || (c>='!' && c<='u'));
}
// Function ecc_compute(), edc_compute() and eccedc_init() taken from 
// ** UNECM - Decoder for ECM (Error Code Modeler) format.
// ** Version 1.0
// ** Copyright (C) 2002 Neill Corlett

/* LUTs used for computing ECC/EDC */
static U8 ecc_f_lut[256];
static U8 ecc_b_lut[256];
static U32 edc_lut[256];
static int luts_init=0;

void eccedc_init(void) {
  if (luts_init) return;
  U32 i, j, edc;
  for(i = 0; i < 256; i++) {
    j = (i << 1) ^ (i & 0x80 ? 0x11D : 0);
    ecc_f_lut[i] = j;
    ecc_b_lut[i ^ j] = i;
    edc = i;
    for(j = 0; j < 8; j++) edc = (edc >> 1) ^ (edc & 1 ? 0xD8018001 : 0);
    edc_lut[i] = edc;
  }
  luts_init=1;
}

void ecc_compute(U8 *src, U32 major_count, U32 minor_count, U32 major_mult, U32 minor_inc, U8 *dest) {
  U32 size = major_count * minor_count;
  U32 major, minor;
  for(major = 0; major < major_count; major++) {
    U32 index = (major >> 1) * major_mult + (major & 1);
    U8 ecc_a = 0;
    U8 ecc_b = 0;
    for(minor = 0; minor < minor_count; minor++) {
      U8 temp = src[index];
      index += minor_inc;
      if(index >= size) index -= size;
      ecc_a ^= temp;
      ecc_b ^= temp;
      ecc_a = ecc_f_lut[ecc_a];
    }
    ecc_a = ecc_b_lut[ecc_f_lut[ecc_a] ^ ecc_b];
    dest[major              ] = ecc_a;
    dest[major + major_count] = ecc_a ^ ecc_b;
  }
}

U32 edc_compute(const U8  *src, int size) {
  U32 edc = 0;
  while(size--) edc = (edc >> 8) ^ edc_lut[(edc ^ (*src++)) & 0xFF];
  return edc;
}

int expand_cd_sector(U8 *data, int a, int test) {
  U8 d2[2352];
  eccedc_init();
  d2[0]=d2[11]=0;
  for (int i=1; i<11; i++) d2[i]=255;
  int mode=(data[15]!=1?2:1);
  int form=(data[15]==3?2:1);
  if (a==-1) for (int i=12; i<15; i++) d2[i]=data[i]; else {
    int c1=(a&15)+((a>>4)&15)*10;
    int c2=((a>>8)&15)+((a>>12)&15)*10;
    int c3=((a>>16)&15)+((a>>20)&15)*10;
    c1=(c1+1)%75;
    if (c1==0) {
      c2=(c2+1)%60;
      if (c2==0) c3++;
    }
    d2[12]=(c3%10)+16*(c3/10);
    d2[13]=(c2%10)+16*(c2/10);
    d2[14]=(c1%10)+16*(c1/10);
  }
  d2[15]=mode;
  if (mode==2) for (int i=16; i<24; i++) d2[i]=data[i-4*(i>=20)];
  if (form==1) {
    if (mode==2) {
      d2[1]=d2[12],d2[2]=d2[13],d2[3]=d2[14];
      d2[12]=d2[13]=d2[14]=d2[15]=0;
    } else {
      for(int i=2068; i<2076; i++) d2[i]=0;
    }
    for (int i=16+8*(mode==2); i<2064+8*(mode==2); i++) d2[i]=data[i];
    U32 edc=edc_compute(d2+16*(mode==2), 2064-8*(mode==2));
    for (int i=0; i<4; i++) d2[2064+8*(mode==2)+i]=(edc>>(8*i))&0xff;
    ecc_compute(d2+12, 86, 24,  2, 86, d2+2076);
    ecc_compute(d2+12, 52, 43, 86, 88, d2+2248);
    if (mode==2) {
      d2[12]=d2[1],d2[13]=d2[2],d2[14]=d2[3],d2[15]=2;
      d2[1]=d2[2]=d2[3]=255;
    }
  }
  for (int i=0; i<2352; i++) if (d2[i]!=data[i] && test) form=2;
  if (form==2) {
    for (int i=24; i<2348; i++) d2[i]=data[i];
    U32 edc=edc_compute(d2+16, 2332);
    for (int i=0; i<4; i++) d2[2348+i]=(edc>>(8*i))&0xff;
  }
  for (int i=0; i<2352; i++) if (d2[i]!=data[i] && test) return 0; else data[i]=d2[i];
  return mode+form-1;
}

//LZSS compressor/decompressor class
//http://my.execpc.com/~geezer/code/lzss.c
class LZSS {
    private:
    const U32 N;                // size of ring buffer
    const U32 F;                // upper limit for g_match_len.
                                // 16 for compatibility with Microsoft COMPRESS.EXE and EXPAND.EXE
    const U32 THRESHOLD;        // encode string into position and length if match_length is greater than this
    U32  NIL;                   // index for root of binary search trees
    Array<U8> LZringbuffer;     // ring buffer of size N, with extra F-1 bytes
                                // to facilitate string comparison
    U32 matchpos;               // position and length of longest match; set by insert_node()
    U32 matchlen;
    Array<U32> LZ_lchild;       // left & right children & parent -- these constitute binary search tree
    Array<U32> LZ_rchild;
    Array<U32> LZ_parent;
    File*g_infile, *g_outfile; //input and output file to be compressed
    U32 filesizez;

    // Inserts string of length F, LZringbuffer[r..r+F-1], into one of the
    // trees (LZringbuffer[r]'th tree) and returns the longest-match position
    // and length via the global variables matchpos and matchlen.
    // If matchlen = F, then removes the old node in favour of the new
    // one, because the old one will be deleted sooner.
    // Note r plays double role, as tree node and position in buffer.
void insert_node(int r){
    U8 *key;
    U32 i, p;
    int cmp;
    cmp = 1;
    key = &LZringbuffer[r];
    p=N+1+key[0];
    LZ_rchild[r]=LZ_lchild[r]=NIL;
    matchlen = 0;
    while(1){
        if(cmp>= 0){
            if(LZ_rchild[p]!=NIL) p=LZ_rchild[p];
            else{
                LZ_rchild[p]=r;
                LZ_parent[r]=p;
                return;
            }
        }
        else{
            if(LZ_lchild[p]!=NIL) p=LZ_lchild[p];
            else{
                LZ_lchild[p]=r;
                LZ_parent[r]=p;
                return;
            }
        }
        for(i=1;i<F;i++){
            cmp=key[i]-LZringbuffer[p+i];
            if(cmp != 0) break;
        }
        if(i>matchlen){
            matchpos=p;
            matchlen=i;
            if(matchlen>=F) break;
        }
    }
    LZ_parent[r]=LZ_parent[p];
    LZ_lchild[r]=LZ_lchild[p];
    LZ_rchild[r]=LZ_rchild[p];
    LZ_parent[LZ_lchild[p]]=r;
    LZ_parent[LZ_rchild[p]]=r;
    if(LZ_rchild[LZ_parent[p]]==p) LZ_rchild[LZ_parent[p]]=r;
    else LZ_lchild[LZ_parent[p]]=r;
    LZ_parent[p]=NIL;                   // remove p
}

//deletes node p from tree
void delete_node(unsigned p){
    U32 q;
    if(LZ_parent[p]==NIL) return;       // not in tree
    if(LZ_rchild[p]==NIL) q=LZ_lchild[p];
    else if(LZ_lchild[p]==NIL) q=LZ_rchild[p];
    else{
        q=LZ_lchild[p];
        if(LZ_rchild[q]!=NIL){
            do q=LZ_rchild[q];
            while(LZ_rchild[q]!=NIL);
            LZ_rchild[LZ_parent[q]]=LZ_lchild[q];
            LZ_parent[LZ_lchild[q]]=LZ_parent[q];
            LZ_lchild[q]=LZ_lchild[p];
            LZ_parent[LZ_lchild[p]]=q;
        }
        LZ_rchild[q]=LZ_rchild[p];
        LZ_parent[LZ_rchild[p]]=q;
    }
    LZ_parent[q] = LZ_parent[p];
    if(LZ_rchild[LZ_parent[p]]==p) LZ_rchild[LZ_parent[p]]=q;
    else LZ_lchild[LZ_parent[p]] = q;
    LZ_parent[p]=NIL;
}
public:
    U32 usize;
    LZSS(File*in, File* out,U32 fsize,U32 qn);

//may fail when compressed size is larger the input (uncompressible data)
U32 compress(){
    U32 i, len, r, s, last_match_length, code_buf_ptr;
    U8 code_buf[17], mask;
    U32 ocount;
    int c;
    // code_buf[1..16] saves eight units of code, and code_buf[0] works as
    // eight flags, "1" representing that the unit is an unencoded letter (1 byte),
    // "0" a position-and-length pair (2 bytes). Thus, eight units require at most
    // 16 bytes of code.
    ocount=0;
    code_buf[0]=0;
    code_buf_ptr=mask=1;
    s=0;
    r=N-F;
    // Clear the buffer with any character that will appear often.
    memset(&LZringbuffer[0]+s,' ',r-s);
    // Read F bytes into the last F bytes of the buffer
    for(len=0;len<F;len++){
        c=g_infile->getc();
        if(c==EOF)break;
        LZringbuffer[r+len]=c;
    }
    if(len==0) return 0; //text of size zero
    // Insert the F strings, each of which begins with one or more 'space'
    // characters. Note the order in which these strings are inserted.
    // This way, degenerate trees will be less likely to occur.
    for(i=1; i<=F;i++) insert_node(r-i);
    // Finally, insert the whole string just read. The global variables
    // matchlen and matchpos are set.
    insert_node(r);
    do{
        // matchlen may be spuriously long near the end of text.
        if(matchlen>len) matchlen=len;
        if(matchlen<=THRESHOLD){            // Not long enough match. Send one byte.
            matchlen=1;
            code_buf[0]|=mask;              // 'send one byte' flag 
            code_buf[code_buf_ptr]=LZringbuffer[r];  // Send uncoded.
            code_buf_ptr++;
        }
        else{                               // Send position and length pair. Note matchlen > THRESHOLD.
            code_buf[code_buf_ptr]=(U8)matchpos;
            code_buf_ptr++;
            code_buf[code_buf_ptr]=(U8)(((matchpos>>4)&0xF0)|(matchlen-(THRESHOLD+1)));
            code_buf_ptr++;
        }
        mask<<=1;                           // Shift mask left one bit.
        if(mask==0){                        // Send at most 8 units of code together
            for(i=0;i<code_buf_ptr;i++){
                g_outfile->putc(code_buf[i]),ocount++;
                if(ocount>=filesizez) return ocount;
            }
            code_buf[0]=0;
            code_buf_ptr=mask=1;
        }
        last_match_length=matchlen;
        for(i=0;i<last_match_length;i++){
            c=g_infile->getc();
            if(c==EOF) break;
            delete_node(s);                 // Delete old strings and read new bytes
            LZringbuffer[s] = c;
            // If the position is near the end of buffer, extend the buffer
            // to make string comparison easier.
            // Since this is a ring buffer, increment the position modulo N.
            // Register the string in LZringbuffer[r..r+F-1] 
            if(s<F-1) LZringbuffer[s+N]=c;
            s=(s+1)&(N-1);
            r=(r+1)&(N-1);
            insert_node(r);
        }
        while(i++<last_match_length){       // After the end of text,
            delete_node(s);                 // no need to read, but
            s=(s+1)&(N-1);
            r=(r+1)&(N-1);
            len--;
            if(len) insert_node(r);         // buffer may not be empty.
        }
    } while(len>0);                         //until length of string to be processed is zero
    if(code_buf_ptr>1){                     // Send remaining code.
        for(i=0;i<code_buf_ptr;i++){
            g_outfile->putc(code_buf[i]),ocount++;
            if(ocount>=filesizez) return ocount;
        }
    }
    return ocount;    //return compressed size
}

U32 decompress(){
    U32 r, flags;
    int i,c, j, k;
    U32 icount,incount;
    icount=incount=0;
    memset(&LZringbuffer[0],' ',N-F);
    r = N - F;
    for(flags=0;;flags>>=1){
    // Get a byte. For each bit of this byte:
    // 1=copy one byte literally, from input to output
    // 0=get two more bytes describing length and position of previously-seen
    // data, and copy that data from the ring buffer to output
        if((flags&0x100)==0){
            c=g_infile->getc(),incount++;
            if(c==EOF||icount>=filesizez) break;
            flags=c|0xFF00;
        }
        if(flags & 1){
            c=g_infile->getc(),incount++;
            if(c==EOF||icount>=filesizez) break;
            g_outfile->putc(c),icount++;
            LZringbuffer[r]=c;
            r=(r+1)&(N-1);
        }
        // 0=get two more bytes describing length and position of previously-
        // seen data, and copy that data from the ring buffer to output
        else{
            i=g_infile->getc(),incount++;
            if(i==EOF||icount>=filesizez) break;
            j=g_infile->getc(),incount++;
            if(j==EOF ||icount>=filesizez) break;
            i|=((j&0xF0)<< 4);
            j=(j&0x0F)+THRESHOLD;
            for(k=0;k<=j;k++){
                c=LZringbuffer[(i+k)&(N-1)];
                g_outfile->putc(c),icount++;
                LZringbuffer[r]=c;
                r=(r+1)&(N-1);
            }
        }
    }
    usize=icount;       //decompressed size
    return incount-1;   //return compressed size
}
};
LZSS::LZSS(File*in, File* out,U32 fsize,U32 qn=0): N(4096),F(16+qn),THRESHOLD(2),NIL(N),
LZringbuffer(N+F-1), LZ_lchild(N+1), LZ_rchild(N+257), LZ_parent(N+1),filesizez(fsize),usize(0){
    g_infile=in, g_outfile=out;
    // initialize trees
    // For i = 0 to N - 1, LZ_rchild[i] and LZ_lchild[i] will be the right and
    // left children of node i. These nodes need not be initialized.
    // Also, LZ_parent[i] is the parent of node i. These are initialized to
    // NIL (= N), which stands for 'not used.'
    // For i = 0 to 255, LZ_rchild[N + i + 1] is the root of the tree
    // for strings that begin with character i. These are initialized
    // to NIL.  Note there are 256 trees.
    for(U32 i=N+1;i<=N+256;i++)
        LZ_rchild[i]=NIL;
    for(U32 i=0;i<N;i++)
        LZ_parent[i]=NIL;
}

//read compressed word,dword
U32 GetCDWord(File*f){
    U16 w = f->getc();
    w=w | (f->getc()<<8);
    if(w&1){
        U16 w1 = f->getc();
        w1=w1 | (f->getc()<<8);
        return ((w1<<16)|w)>>1;
    }
    return w>>1;
}
U8 GetCWord(File*f){
    U8 b=f->getc();
    if(b&1) return ((f->getc()<<8)|b)>>1;
    return b>>1;
}

int parse_zlib_header(int header) {
    switch (header) {
        case 0x2815 : return 0;  case 0x2853 : return 1;  case 0x2891 : return 2;  case 0x28cf : return 3;
        case 0x3811 : return 4;  case 0x384f : return 5;  case 0x388d : return 6;  case 0x38cb : return 7;
        case 0x480d : return 8;  case 0x484b : return 9;  case 0x4889 : return 10; case 0x48c7 : return 11;
        case 0x5809 : return 12; case 0x5847 : return 13; case 0x5885 : return 14; case 0x58c3 : return 15;
        case 0x6805 : return 16; case 0x6843 : return 17; case 0x6881 : return 18; case 0x68de : return 19;
        case 0x7801 : return 20; case 0x785e : return 21; case 0x789c : return 22; case 0x78da : return 23;
    }
    return -1;
}
int zlib_inflateInit(z_streamp strm, int zh) {
    if (zh==-1) return inflateInit2(strm, -MAX_WBITS); else return inflateInit(strm);
}


bool IsGrayscalePalette(File* in, int n = 256, int isRGBA = 0){
  U64 offset = in->curpos();
  int stride = 3+isRGBA, res = (n>0)<<8, order=1;
  for (int i = 0; (i < n*stride) && (res>>8); i++) {
    int b = in->getc();
    if (b==EOF){
      res = 0;
      break;
    }
    if (!i) {
      res = 0x100|b;
      order = 1-2*(b>0);
      continue;
    }

    //"j" is the index of the current byte in this color entry
    int j = i%stride;
    if (!j){
      // load first component of this entry
      res = (res&((b-(res&0xFF)==order)<<8));
      res|=(res)?b:0;
    }
    else if (j==3)
      res&=((!b || (b==0xFF))*0x1FF); // alpha/attribute component must be zero or 0xFF
    else
      res&=((b==(res&0xFF))<<9)-1;
  }
   in->setpos( offset);
  return (res>>8)>0;
}

#define base64max 0x8000000 //128M limit
#define base85max 0x8000000 //128M limit

struct TARheader{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char major[8];
    char minor[8];
    char pad[167];
};
int getoct(const char *p, int n){
    int i = 0;
    while (*p<'0' || *p>'7') ++p, --n;
    while (*p>='0' && *p<='7' && n>0) {
        i*=8;
        i+=*p-'0';
        ++p,--n;
    }
    return i;
}
int tarchecksum(char *p){
    int u=0;
    for (int n = 0; n < 512; ++n) {
        if (n<148 || n>155) u+=((U8 *)p)[n];
        else u += 0x20;
    }
    return (u==getoct(p+148,8));
}
bool tarend(const char *p){
    for (int n=511; n>=0; --n) if (p[n] != '\0') return false;
    return true;
}
struct dBASE {
  U8 Version;
  U32 nRecords;
  U16 RecordLength, HeaderLength;
  int Start, End;
};

struct dTIFF {
  U32 size;
  U32 offset;
  U8 compression;
  U32 width;
  U32 height;
  U8 bits;
  U8 bits1;
};

// Detect EXE or JPEG data
Filetype detect(File* in, U64 n, Filetype type, int &info, int &info2, int it=0,int s1=0) {
  U32 buf3=0, buf2=0, buf1=0, buf0=0;  // last 8 bytes
  U64 start= in->curpos();

  // For EXE detection
  Array<U64> abspos(256),  // CALL/JMP abs. addr. low byte -> last offset
    relpos(256);    // CALL/JMP relative addr. low byte -> last offset
  int e8e9count=0;  // number of consecutive CALL/JMPs
  U64 e8e9pos=0;    // offset of first CALL or JMP instruction
  U64 e8e9last=0;   // offset of most recent CALL or JMP
  // For EXE detection
  Array<U64> absposDEC(0xff+1),  // CALL/JMP abs. addr. low byte -> last offset
    relposDEC(0xff+1);    // CALL/JMP relative addr. low byte -> last offset
  int DECcount=0;  // number of consecutive CALL/JMPs
  U64 DECpos=0;    // offset of first CALL or JMP instruction
  U64 DEClast=0;   // offset of most recent CALL or JMP
  
  Array<U64> absposARM(0xff+1),  // CALL/JMP abs. addr. low byte -> last offset
    relposARM(0xff+1);    // CALL/JMP relative addr. low byte -> last offset
  int ARMcount=0;  // number of consecutive CALL/JMPs
  U64 ARMpos=0;    // offset of first CALL or JMP instruction
  U64 ARMlast=0;   // offset of most recent CALL or JMP

  U64 soi=0, sof=0, sos=0, app=0,eoi=0;  // For JPEG detection - position where found
  U64 wavi=0,wavlist=0;
  int wavsize=0,wavch=0,wavbps=0,wavm=0,wavsr=0,wavt=0,wavtype=0,wavlen=0;  // For WAVE detection
  U64 aiff=0;
  int aiffm=0,aiffs=0;  // For AIFF detection
  U64 s3mi=0;
  int s3mno=0,s3mni=0;  // For S3M detection
  U64 bmp=0;
  int imgbpp=0,bmpx=0,bmpy=0,bmpof=0,bmps=0,hdrless=0;;  // For BMP detection
  U64 rgbi=0;
  int rgbx=0,rgby=0;  // For RGB detection
  U64 tga=0;
  U64 tgax=0;
  int tgay=0,tgaz=0,tgat=0;  // For TGA detection
  U64 pgm=0;
  int pgmcomment=0,pgmw=0,pgmh=0,pgm_ptr=0,pgmc=0,pgmn=0,pamatr=0,pamd=0;  // For PBM, PGM, PPM, PAM detection
  char pgm_buf[32];
  U64 cdi=0;
  U64 mdfa=0;
  int cda=0,cdm=0;  // For CD sectors detection
  U32 cdf=0;
 // For TEXT
  U64 txtStart=0,txtLen=0,txtOff=0,txtbinc=0,txtbinp=0,txta=0,txt0=0;
  int utfc=0,utfb=0,txtIsUTF8=0; //utf count 2-6, current byte
  const int txtMinLen=1024*64;
  U64 txtNL=0, txtNLC=0,txtNLT=0;
  bool doBT=false;
  int BTcount=0;
  //base64
  U64 b64s=0,b64s1=0,b64p=0,b64slen=0,b64h=0;//,b64i=0;
  U64 base64start=0,base64end=0,b64line=0,b64nl=0,b64lcount=0;
  //base85
  U64 b85s=0,b85s1=0,b85p=0,b85slen=0,b85h=0;
  U64 base85start=0,base85end=0,b85line=0;//,b85lcount=0;//b85nl=0,
  //int b64f=0,b64fstart=0,b64flen=0,b64send=0,b64fline=0; //force base64 detection
  U64 gif=0,gifa=0,gifi=0,gifw=0,gifc=0,gifb=0,gifplt=0,gifgray=0; // For GIF detection
  U64 png=0, lastchunk=0, nextchunk=0;               // For PNG detection
  int pngw=0, pngh=0, pngbps=0, pngtype=0,pnggray=0; 
  //MSZip
  U64 MSZip=0, MSZ=0, MSZipz=0;
  int yu=0;
  int zlen=0;
  U64 fSZDD=0; //
  LZSS* lz77;
  U8 zbuf[256+32], zin[1<<16], zout[1<<16]; // For ZLIB stream detection
  int zbufpos=0, histogram[256]={0};;
  U64 zzippos=-1;
  bool brute = true;
  int pdfim=0,pdfimw=0,pdfimh=0,pdfimb=0,pdfgray=0;
  U64 pdfimp=0;
  U64 mrb=0,mrbsize=0,mrbcsize=0,mrbPictureType=0,mrbPackingMethod=0,mrbTell=0,mrbTell1=0,mrbw=0,mrbh=0; // For MRB detection
  //
  U64 tar=0,tarn=0,tarl=0;
  TARheader tarh;
  U32 op=0;//DEC A
  U64 nesh=0,nesp=0,nesc=0;
  int textbin=0; //if 1/3 is text
  dBASE dbase;
  U64 dbasei=0;
  memset(&dbase, 0, sizeof(dBASE));
  // pdf image
  U64 pdfi1=0,pdfiw=0,pdfih=0,pdfic=0;
  char pdfi_buf[32];
  int pdfi_ptr=0,pdfin=0;
  
   // For image detection
  static Array<U32> tfidf;
  tfidf.resize(0);
  static int tiffImages=-1;
  static Array<dTIFF> tiffFiles(10);
  static U64 tiffImageStart=0;
  static U64 tiffImageEnd=0;
  bool tiffMM=false;
  static int deth=0,detd=0;  // detected header/data size in bytes
  static Filetype dett;      // detected block type
  if (deth >1) return  in->setpos(start+deth),deth=0,dett;
  else if (deth ==-1) return  in->setpos(start),deth=0,dett;
  else if (detd) return  in->setpos( start+detd),detd=0,DEFAULT;
 

  for (U64 i=0; i<n; ++i) {
    int c=in->getc();
    if (c==EOF) return (Filetype)(-1);
    buf3=buf3<<8|buf2>>24;
    buf2=buf2<<8|buf1>>24;
    buf1=buf1<<8|buf0>>24;
    buf0=buf0<<8|c;
    
    if  ((c<128 && c>=32) || c==10 || c==13 || c==0x12 || c==9 || c==4 ) textbin++,info=textbin;
    
    if(tiffImages>=0){
        brute=false;
        textbin=0;
    for (int o=0;o<tiffImages; o++) { 
       if(  in->curpos()== tiffFiles[o].offset+1 ) {
           if (tiffFiles[o].compression==6 || tiffFiles[o].compression==7  ) {
               tiffImageEnd++;
                 if (type!=JPEG)return  in->setpos( start+i),JPEG;
                 else  return  in->setpos(start+tiffFiles[o].size),DEFAULT;
               }else if ( tiffFiles[o].compression==2) {
               tiffImageEnd++;
                 if (type!=DEFAULT)return  in->setpos( start+i),DEFAULT;
                 else  return  in->setpos(start+tiffFiles[o].size),DEFAULT;
           } else if (tiffFiles[o].compression==1 ||tiffFiles[o].compression==255) {
               tiffImageEnd++;
                 if (tiffFiles[o].bits==1  &&type!=IMAGE8 && tiffFiles[o].bits1!=14 ) return info=tiffFiles[o].width, in->setpos(start+i),IMAGE8;
                 if (tiffFiles[o].bits==1  &&type!=IMGUNK && tiffFiles[o].bits1==14  ) return info=0, in->setpos(start+i),IMGUNK;
                 else if (tiffFiles[o].bits==3 &&type!=IMAGE24 ) return info=tiffFiles[o].width, in->setpos( start+i),IMAGE24;
                 else if (tiffFiles[o].bits==1 && type==IMAGE8 ) return info=tiffFiles[o].width, in->setpos(start+tiffFiles[o].size),DEFAULT;
                 else if (tiffFiles[o].bits1==14 && type==IMGUNK ) return info=tiffFiles[o].width, in->setpos( start+tiffFiles[o].size),DEFAULT;
                 else if (tiffFiles[o].bits==3 &&type==IMAGE24 ) return info=tiffFiles[o].width, in->setpos(start+tiffFiles[o].size),DEFAULT;
            } 
       }
       if(tiffImageEnd>>1==tiffImages) tiffImages=-1,tiffImageEnd=0;
       }
       //continue;
      }  
     // detect PNG images
    if (!png && buf3==0x89504E47 && buf2==0x0D0A1A0A && buf1==0x0000000D && buf0==0x49484452) png=i, pngtype=-1, lastchunk=buf3;//%PNG
    if (png){
      const int p=i-png;
      if (p==12){
        pngw = buf2;
        pngh = buf1;
        pngbps = buf0>>24;
        pngtype = (U8)(buf0>>16);
        pnggray = 0;
        png*=((buf0&0xFFFF)==0 && pngw && pngh && pngbps==8 && (!pngtype || pngtype==2 || pngtype==3 || pngtype==4 || pngtype==6));
      }
      else if (p>12 && pngtype<0)
        png = 0;
      else if (p==17){
        png*=((buf1&0xFF)==0);
        nextchunk =(png)?i+8:0;
      }
      else if (p>17 && i==nextchunk){
        nextchunk+=buf1+4+8;//CRC, Chunk length+Id
        lastchunk = buf0;
        png*=(lastchunk!=0x49454E44);//IEND
        if (lastchunk==0x504C5445){//PLTE
          png*=(buf1%3==0);
          pnggray = (png && IsGrayscalePalette(in, buf1/3));
        }
      }
    }
    
    // tar
    if (((buf0)==0x61722020 || (buf0&0xffffff00)==0x61720000) && (buf1&0xffffff)==0x757374 && tar==0&&tarl==0) tar=i,tarn=0,tarl=1;
    if (tarl) {
        const int p=int(i-tar+263);        
        if (p==512 && tarn==0 && tarl==1) {
            U64 savedpos= in->curpos();
             in->setpos( savedpos-513);
             in->blockread((U8*) &tarh,  sizeof(tarh) );
            if (!tarchecksum((char*)&tarh)) tar=0,tarn=0,tarl=0;
            else{
                tarl=2;
                tar=in->curpos();
               int a=getoct(tarh.size,12);
               int b=a-(a/512)*512;
               if (b) tarn=tarn+512*2+(a/512)*512;
               else if (a==0) tarn=tarn+512;
               else tarn=tarn+512+(a/512)*512;
               tarn=tarn+p;
            }
        }
        if (tarn && tarl==2 && tarn==(start+i-tar+512)) {
            U64 savedpos= in->curpos();
             in->setpos(savedpos-512);
            in->blockread((U8*) &tarh,  sizeof(tarh)  );
            if (!tarchecksum((char*)&tarh))  tarn=tar-512-start,tar=0,tarl=0;
            if (tarend((char*)&tarh)==true) {
                if (type==TAR) return  in->setpos(start+i),DEFAULT;
                return  in->setpos(start+tarn),TAR;
            } else{
                int a=getoct(tarh.size,12);
                int b=a-(a/512)*512;
                if (b) tarn=tarn+512*2+(a/512)*512;
                else if (a==0) tarn=tarn+512;
                else tarn=tarn+512+(a/512)*512;
            }
        }
        else continue;
    }

    if ((buf0)==0x0080434b && MSZip==0  && !cdi  && type!=MDF) {
       MSZ=i;
       MSZip=i-4,MSZipz=(buf1&0xffff);
       MSZipz=((MSZipz&0xff)<<8) +(MSZipz >>8);
       zlen=MSZipz;
       yu=1;
    }
    if ( MSZip) {
        const int p=int(i-MSZip-12);        
        if (p==zlen) {
            MSZip=i-4;
            zlen=(buf1&0xffff);
            zlen=((zlen&0xff)<<8) +(zlen >>8);
            if( buf0==0x0080434b ) {    //32768 CK
                MSZipz+=zlen;           //12?
                yu++;
            }else if( (buf0&0xffff)==0x434b && zlen<32768) {                      //if final part <32768 CK
                yu++;
                MSZipz+=zlen+yu*8; //4 2 2
                if (type==MSZIP ) return  in->setpos(start+MSZipz),DEFAULT;
                return  in->setpos(start+MSZ-3),MSZIP;
            }else  {   
                MSZip=MSZipz=zlen=0;
            }
       }
    }
    
    // ZLIB stream detection
        histogram[c]++;
    if (i>=256)
      histogram[zbuf[zbufpos]]--;
    zbuf[zbufpos] = c;
    if (zbufpos<32)
      zbuf[zbufpos+256] = c;
    zbufpos=(zbufpos+1)&0xFF;
    if(!cdi && !mdfa && type!=MDF)  {
      int zh=parse_zlib_header(((int)zbuf[(zbufpos-32)&0xFF])*256+(int)zbuf[(zbufpos-32+1)&0xFF]);
    bool valid = (i>=31 && zh!=-1);
    if (!valid && brute && i>=255){
      U8 BTYPE = (zbuf[zbufpos]&7)>>1;
      if ((valid=(BTYPE==1 || BTYPE==2))){
        int maximum=0, used=0, offset=zbufpos;
        for (int i=0;i<4;i++,offset+=64){
          for (int j=0;j<64;j++){
            int freq = histogram[zbuf[(offset+j)&0xFF]];
            used+=(freq>0);
            maximum+=(freq>maximum);
          }
          if (maximum>=((12+i)<<i) || used*(6-i)<(i+1)*64){
            valid = false;
            break;
          }
        }
      }
    }
    if (valid || zzippos==i) {
      int streamLength=0, ret=0, brute=(zh==-1 && zzippos!=i);
      // Quick check possible stream by decompressing first 32 bytes
      z_stream strm;
      strm.zalloc=Z_NULL; strm.zfree=Z_NULL; strm.opaque=Z_NULL;
      strm.next_in=Z_NULL; strm.avail_in=0;
      if (zlib_inflateInit(&strm,zh)==Z_OK) {
        strm.next_in=&zbuf[(zbufpos-(brute?0:32))&0xFF]; strm.avail_in=32;
        strm.next_out=zout; strm.avail_out=1<<16;
        ret=inflate(&strm, Z_FINISH);
        ret=(inflateEnd(&strm)==Z_OK && (ret==Z_STREAM_END || ret==Z_BUF_ERROR) && strm.total_in>=16);
      }
      if (ret) {
        // Verify valid stream and determine stream length
        U64 savedpos= in->curpos();
        strm.zalloc=Z_NULL; strm.zfree=Z_NULL; strm.opaque=Z_NULL;
        strm.next_in=Z_NULL; strm.avail_in=0; strm.total_in=strm.total_out=0;
        if (zlib_inflateInit(&strm,zh)==Z_OK) {
          for (U64 j=i-(brute?255:31); j<n; j+=1<<16) {
            unsigned int blsize=min(n-j,1<<16);
             in->setpos( start+j);
            if (in->blockread(zin,   blsize  )!=blsize) break;
            strm.next_in=zin; strm.avail_in=blsize;
            do {
              strm.next_out=zout; strm.avail_out=1<<16;
              ret=inflate(&strm, Z_FINISH);
            } while (strm.avail_out==0 && ret==Z_BUF_ERROR);
            if (ret==Z_STREAM_END) streamLength=strm.total_in;
            if (ret!=Z_BUF_ERROR) break;
          }
          if (inflateEnd(&strm)!=Z_OK) streamLength=0;
        }
         in->setpos( savedpos);
      }
      if (streamLength>(brute<<7)) {
        info=0;
        if (pdfimw>0 && pdfimw<0x1000000 && pdfimh>0) {
          if (pdfimb==8 && (int)strm.total_out==pdfimw*pdfimh) info=((pdfgray?IMAGE8GRAY:IMAGE8)<<24)|pdfimw;
          if (pdfimb==8 && (int)strm.total_out==pdfimw*pdfimh*3) info=(IMAGE24<<24)|pdfimw*3;
          if (pdfimb==4 && (int)strm.total_out==((pdfimw+1)/2)*pdfimh) info=(IMAGE4<<24)|((pdfimw+1)/2);
          if (pdfimb==1 && (int)strm.total_out==((pdfimw+7)/8)*pdfimh) info=(IMAGE1<<24)|((pdfimw+7)/8);
          pdfgray=0;
        }
        else if (png && pngw<0x1000000 && lastchunk==0x49444154){//IDAT
          if (pngbps==8 && pngtype==2 && (int)strm.total_out==(pngw*3+1)*pngh) info=(PNG24<<24)|(pngw*3), png=0;
          else if (pngbps==8 && pngtype==6 && (int)strm.total_out==(pngw*4+1)*pngh) info=(PNG32<<24)|(pngw*4), png=0;
          else if (pngbps==8 && (!pngtype || pngtype==3) && (int)strm.total_out==(pngw+1)*pngh) info=(((!pngtype || pnggray)?PNG8GRAY:PNG8)<<24)|(pngw), png=0;
        }
       return in->setpos( start+i-(brute?255:31)),detd=streamLength,ZLIB;
      }
    }
    if (zh==-1 && zbuf[(zbufpos-32)&0xFF]=='P' && zbuf[(zbufpos-32+1)&0xFF]=='K' && zbuf[(zbufpos-32+2)&0xFF]=='\x3'
      && zbuf[(zbufpos-32+3)&0xFF]=='\x4' && zbuf[(zbufpos-32+8)&0xFF]=='\x8' && zbuf[(zbufpos-32+9)&0xFF]=='\0') {
        int nlen=(int)zbuf[(zbufpos-32+26)&0xFF]+((int)zbuf[(zbufpos-32+27)&0xFF])*256
                +(int)zbuf[(zbufpos-32+28)&0xFF]+((int)zbuf[(zbufpos-32+29)&0xFF])*256;
        if (nlen<256 && i+30+nlen<n) zzippos=i+30+nlen;
    }
    if (i-pdfimp>1024) pdfim=pdfimw=pdfimh=pdfimb=pdfgray=0;
    if (pdfim>1 && !(isspace(c) || isdigit(c))) pdfim=1;
    if (pdfim==2 && isdigit(c)) pdfimw=pdfimw*10+(c-'0');
    if (pdfim==3 && isdigit(c)) pdfimh=pdfimh*10+(c-'0');
    if (pdfim==4 && isdigit(c)) pdfimb=pdfimb*10+(c-'0');
    if ((buf0&0xffff)==0x3c3c) pdfimp=i,pdfim=1; // <<
    if (pdfim && (buf1&0xffff)==0x2f57 && buf0==0x69647468) pdfim=2,pdfimw=0; // /Width
    if (pdfim && (buf1&0xffffff)==0x2f4865 && buf0==0x69676874) pdfim=3,pdfimh=0; // /Height
    if (pdfim && buf3==0x42697473 && buf2==0x50657243 && buf1==0x6f6d706f
       && buf0==0x6e656e74 && zbuf[(zbufpos-32+15)&0xFF]=='/') pdfim=4,pdfimb=0; // /BitsPerComponent
    if (pdfim && (buf2&0xFFFFFF)==0x2F4465 && buf1==0x76696365 && buf0==0x47726179) pdfgray=1; // /DeviceGray
}
    // NES rom 
    //The format of the header is as follows:
    //0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
    //4: Size of PRG ROM in 16 KB units
    //5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
    //6: Flags 6
    //7: Flags 7
    //8: Size of PRG RAM in 8 KB units (Value 0 infers 8 KB for compatibility; see PRG RAM circuit)
    //9: Flags 9
    //10: Flags 10 (unofficial)
    //11-15: Zero filled
    if (buf0==0x4E45531A && type!=MDF &&  !cdi) nesh=i,nesp=0;
    if (nesh) {
      const int p=int(i-nesh);
      if (p==1) nesp=buf0&0xff; //count of pages*0x3FFF
      else if (p==2) nesc=buf0&0xff; //count of CHR*0x1FFF
      else if (p==6 && ((buf0&0xfe)!=0) )nesh=0; // flags 9
      else if (p==11 && (buf0!=0) )nesh=0;
      else if (p==12) {
        if (nesp>0 && nesp<129) NES_DET(NESROM,nesh-3,0,nesp*0x3FFF+nesc*0x1FFF+15);
        nesh=0;
      }
    }
    
    /* dBASE VERSIONS
      '02' > FoxBase
      '03' > dBase III without memo file
      '04' > dBase IV without memo file
      '05' > dBase V without memo file
      '07' > Visual Objects 1.x
      '30' > Visual FoxPro
      '31' > Visual FoxPro with AutoIncrement field
      '43' > dBASE IV SQL table files, no memo
      '63' > dBASE IV SQL system files, no memo
      '7b' > dBase IV with memo file
      '83' > dBase III with memo file
      '87' > Visual Objects 1.x with memo file
      '8b' > dBase IV with memo file
      '8e' > dBase IV with SQL table
      'cb' > dBASE IV SQL table files, with memo
      'f5' > FoxPro with memo file - tested
      'fb' > FoxPro without memo file
    */
    if (dbasei==0 && ((c&7)==3 || (c&7)==4 || (c>>4)==3|| c==0xf5 || c==0x30) && tiffImages==-1) {
        dbasei=i+1,dbase.Version = ((c>>4)==3)?3:c&7;
        dbase.HeaderLength=dbase.Start=dbase.nRecords=dbase.RecordLength=0;
    }
    if (dbasei) {
      const int p=int(i-dbasei+1);
      if (p==2 && !(c>0 && c<13)) dbasei=0;      //month
      else if (p==3 && !(c>0 && c<32)) dbasei=0; //day
      else if (p==7 && !((dbase.nRecords = bswap(buf0)) > 0 && dbase.nRecords<0xFFFFF)) dbasei=0;
      else if (p==9 && !((dbase.HeaderLength = ((buf0>>8)&0xff)|(c<<8)) > 32 && ( ((dbase.HeaderLength-32-1)%32)==0 || (dbase.HeaderLength>255+8 && (((dbase.HeaderLength-=255+8)-32-1)%32)==0) )) ) dbasei=0;
      else if (p==11 && !(((dbase.RecordLength = ((buf0>>8)&0xff)|(c<<8))) > 8) ) dbasei=0;
      else if (p==15 && ((buf0&0xfffffefe)!=0 && ((buf0>>8)&0xfe)>1 && ((buf0)&0xfe)>1  )) dbasei=0;
      else if (p==16 && dbase.RecordLength >4000)dbasei=0;
      else if (p==(dbase.HeaderLength-1) && c==0xd) { //Field Descriptor terminator
          dbase.Start = dbase.HeaderLength;
          dbase.End =  dbase.Start + dbase.nRecords * dbase.RecordLength;
          U64 savedpos = in->curpos();
          U32 seekpos = dbase.End+savedpos-p-1;
           in->setpos(seekpos);
          // test file end marker, fail if not present
          if (in->getc()!=0x1a) dbasei=0, in->setpos(savedpos);
          else{
                in->setpos(savedpos);
               DBS_DET(DBASE,dbasei- 1,dbase.HeaderLength, dbase.nRecords * dbase.RecordLength+1,dbase.RecordLength); 
          }
     }
     else if (p>dbase.HeaderLength && p>68) dbasei=0; // Fail if past Field Descriptor terminator
    }
    
    //detect LZSS compressed data in compress.exe generated archives
    if ((buf0==0x88F02733 && buf1==0x535A4444 && !cdi  && type!=MDF) ||(buf1==0x535A2088 && buf0==0xF02733D1)) fSZDD=i;
    if (fSZDD  && type!=MDF && (((i-fSZDD ==6) && (buf1&0xff00)==0x4100 && ((buf1&0xff)==0 ||(buf1&0xff)>'a')&&(buf1&0xff)<'z') || (buf1!=0x88F02733 && !cdi  && (i-fSZDD)==4))){
       int lz2=0;
        if (buf1!=0x88F02733 && (i-fSZDD)==4) lz2=2;  //+2 treshold
        U32 fsizez=bswap(buf0); //uncompressed file size
        if (fsizez<0x1ffffff){
            FileTmp outf;//=tmpfile2();          // try to decompress file
            lz77=new LZSS(in,&outf,fsizez,lz2);
            U64 savedpos= in->curpos();
            U32 u=lz77->decompress(); //compressed size
            int uf= lz77->usize; //uncompressed size
            delete lz77;
            U32 csize= in->curpos()-savedpos-(!in->eof()?1:0); //? overflow
            if (u!=csize || u>fsizez) fSZDD=0;          // reset if not same size or compressed size > uncompressed size
            else{
                 outf.setpos(0);  // try to compress decompressed file
                FileTmp out2;//=tmpfile2();
                lz77=new LZSS(&outf,&out2,u,lz2);
                U32 r=lz77->compress();
                delete lz77;
                out2.close();
                outf.close();
                if (r!=(csize)) fSZDD=0;    // reset if not same size
                else{
                     in->setpos( savedpos); //all good
                    //flag for +2 treshold, set bit 25
                    SZ_DET(SZDD,fSZDD+7-lz2,14-lz2,r,uf+(lz2?(1<<25):0)); 
                }
            }
            outf.close();
        }
        else fSZDD=0;
    } 
    
     // MDF (Alcohol 120%) CD (mode 1 and mode 2 form 1+2 - 2352 bytes+96 channel data)
    if ( !cdi && mdfa && type!=MDF)  return   in->setpos( start+mdfa-7), MDF;
    if (buf1==0x00ffffff && buf0==0xffffffff   && !mdfa  && type==MDF) mdfa=i;
    if (mdfa && i>mdfa) {
        const int p=(i-mdfa)%2448;
        if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) {
          mdfa=0;
          }
        if (!mdfa && type==MDF)  return  in->setpos( start+i-p-7), DEFAULT;
    }
    if (type==MDF) continue;
    
    // CD sectors detection (mode 1 and mode 2 form 1+2 - 2352 bytes)
    if (buf1==0x00ffffff && buf0==0xffffffff && !cdi && !mdfa) cdi=i,cda=-1,cdm=0;
    if (cdi && i>cdi) {
      const int p=(i-cdi)%2352;
      if (p==8 && (buf1!=0xffffff00 || ((buf0&0xff)!=1 && (buf0&0xff)!=2))) cdi=0; // FIX it ?
      else if (p==16 && i+2336<n) {
        U8 data[2352];
        U64 savedpos= in->curpos();
         in->setpos( start+i-23);
        in->blockread(data,   2352  );
        int t=expand_cd_sector(data, cda, 1);
        if (t!=cdm) cdm=t*(i-cdi<2352);
        if (cdm && cda!=10 && (cdm==1 || buf0==buf1) && type!=CD) {
            //skip possible 96 byte channel data and test if another frame
             in->setpos(  in->curpos()+96);
            U32 mdf= (in->getc()<<24)+(in->getc()<<16)+(in->getc()<<8)+in->getc();
            U32 mdf1=(in->getc()<<24)+(in->getc()<<16)+(in->getc()<<8)+in->getc();
            if (mdf==0x00ffffff && mdf1==0xffffffff ) mdfa=cdi,cdi=cdm=0; //drop to mdf mode?
        }
         in->setpos( savedpos); // seek back if no mdf
        if (cdm && cda!=10 && (cdm==1 || buf0==buf1)) {
          if (type!=CD) return info=cdm, in->setpos( start+cdi-7), CD;
          cda=(data[12]<<16)+(data[13]<<8)+data[14];
          if (cdm!=1 && i-cdi>2352 && buf0!=cdf) cda=10;
          if (cdm!=1) cdf=buf0;
        } else cdi=0;
      }
      if (!cdi && type==CD) return  in->setpos( start+i-p-7), DEFAULT;
    }
    if (type==CD) continue;
 
    // Detect JPEG by code SOI APPx (FF D8 FF Ex) followed by
    // SOF0 (FF C0 xx xx 08) and SOS (FF DA) within a reasonable distance.
    // Detect end by any code other than RST0-RST7 (FF D9-D7) or
    // a byte stuff (FF 00).

    //if (!soi && i>=3 && ((buf0&0xfffffff0)==0xffd8ffe0 )) soi=i, app=i+2, sos=sof=0;//|| buf0==0xffd8ffdb
 // if(tiffImages==-1 ){
    if (!soi && i>=3 && ((
    ((buf0&0xffffff00)==0xffd8ff00 && ((U8)buf0==0xC0 || (U8)buf0==0xC4 || ((U8)buf0>=0xDB && (U8)buf0<=0xFE)))
    ||(buf0&0xfffffff0)==0xffd8ffe0  ) )    
    ) soi=i, app=i+2, sos=sof=0;
    if (soi) {
      if (app==i && (buf0>>24)==0xff &&
         ((buf0>>16)&0xff)>0xc0 && ((buf0>>16)&0xff)<0xff) app=i+(buf0&0xffff)+2;
      if (app<i && (buf1&0xff)==0xff && (buf0&0xff0000ff)==0xc0000008) sof=i,brute=false;
      
      if (sof && sof>soi && i-sof<0x1000 && (buf0&0xffff)==0xffda) {
        sos=i;
        if (type!=JPEG) return  in->setpos(start+soi-3), JPEG;
      }
      if (i-soi>0x40000 && !sos) soi=0;
    }
    if (type==JPEG && soi && (buf0&0xffff)==0xffd9) eoi=i;
    if (type==JPEG &&  soi  && sos && eoi && (buf0&0xffff)==0xffd8) {
        return  in->setpos( start+i-1), DEFAULT;
    }
    if (type==JPEG && sos && i>sos && (buf0&0xff00)==0xff00
        && (buf0&0xff)!=0 && ((buf0&0xf8)!=0xd0 )) {
        return DEFAULT;
    }
  //  }
    // Detect .wav file header
    if (buf0==0x52494646) wavi=i,wavm=0;
    if (wavi) {
            int p=i-wavi;
            if (p==4) wavsize=bswap(buf0);
            else if (p==8){
                wavtype=(buf0==0x57415645)?1:(buf0==0x7366626B)?2:0;
                if (!wavtype) wavi=0;
            }
            else if (wavtype){
                if (wavtype==1) {
                    if (p==16 && (buf1!=0x666d7420 || bswap(buf0)!=16)) wavi=0;
                    else if (p==20) wavt=bswap(buf0)&0xffff;
                    else if (p==22) wavch=bswap(buf0)&0xffff;
                    else if (p==24) wavsr=bswap(buf0) ;
                    else if (p==34) wavbps=bswap(buf0)&0xffff;
                    else if (p==40+wavm && buf1!=0x64617461) wavm+=bswap(buf0)+8,wavi=(wavm>0xfffff?0:wavi);
                    else if (p==40+wavm) {
                        int wavd=bswap(buf0);
                        info2=wavsr;
                        if ((wavch==1 || wavch==2) && (wavbps==8 || wavbps==16) && wavt==1 && wavd>0 && wavsize>=wavd+36
                             && wavd%((wavbps/8)*wavch)==0 && wavsr>=0) AUD_DET(AUDIO,wavi-3,44+wavm,wavd,wavch+wavbps/4-3);
                        wavi=0;
                    }
                }
                else{
                    if ((p==16 && buf1!=0x4C495354) || (p==20 && buf0!=0x494E464F))
                        wavi=0;
                    else if (p>20 && buf1==0x4C495354 && (wavi*=(buf0!=0))){
                        wavlen = bswap(buf0);
                        wavlist = i;
                    }
                    else if (wavlist){
                        p=i-wavlist;
                        if (p==8 && (buf1!=0x73647461 || buf0!=0x736D706C))
                            wavi=0;
                        else if (p==12){
                            int wavd = bswap(buf0);
                            info2=44100;
                            if (wavd && (wavd+12)==wavlen)
                                AUD_DET(AUDIO,wavi-3,(12+wavlist-(wavi-3)+1)&~1,wavd,1+16/4-3);
                            wavi=0;
                        }
                    }
                }
            }
    }

    // Detect .aiff file header
    if (buf0==0x464f524d) aiff=i,aiffs=0; // FORM
    if (aiff) {
      const int p=int(i-aiff);
      if (p==12 && (buf1!=0x41494646 || buf0!=0x434f4d4d)) aiff=0; // AIFF COMM
      else if (p==24) {
        const int bits=buf0&0xffff, chn=buf1>>16;
        if ((bits==8 || bits==16) && (chn==1 || chn==2)) aiffm=chn+bits/4+1; else aiff=0;
      } else if (p==42+aiffs && buf1!=0x53534e44) aiffs+=(buf0+8)+(buf0&1),aiff=(aiffs>0x400?0:aiff);
      else if (p==42+aiffs) AUD_DET(AUDIO,aiff-3,54+aiffs,buf0-8,aiffm);
    }

    // Detect .mod file header 
    if ((buf0==0x4d2e4b2e || buf0==0x3643484e || buf0==0x3843484e  // M.K. 6CHN 8CHN
       || buf0==0x464c5434 || buf0==0x464c5438) && (buf1&0xc0c0c0c0)==0 && i>=1083) {
      int64_t savedpos= in->curpos();
      const int chn=((buf0>>24)==0x36?6:(((buf0>>24)==0x38 || (buf0&0xff)==0x38)?8:4));
      int len=0; // total length of samples
      int numpat=1; // number of patterns
      for (int j=0; j<31; j++) {
         in->setpos(start+i-1083+42+j*30);
        const int i1=in->getc();
        const int i2=in->getc(); 
        len+=i1*512+i2*2;
      }
       in->setpos(start+i-131);
      for (int j=0; j<128; j++) {
        int x=in->getc();
        if (x+1>numpat) numpat=x+1;
      }
      if (numpat<65) AUD_DET(AUDIO,i-1083,1084+numpat*256*chn,len,4);
       in->setpos(savedpos);
    }
    
    // Detect .s3m file header 
    if (buf0==0x1a100000) s3mi=i,s3mno=s3mni=0;
    if (s3mi) {
      const int p=int(i-s3mi);
      if (p==4) s3mno=bswap(buf0)&0xffff,s3mni=(bswap(buf0)>>16);
      else if (p==16 && (((buf1>>16)&0xff)!=0x13 || buf0!=0x5343524d)) s3mi=0;
      else if (p==16) {
        int64_t savedpos= in->curpos();
        int b[31],sam_start=(1<<16),sam_end=0,ok=1;
        for (int j=0;j<s3mni;j++) {
           in->setpos( start+s3mi-31+0x60+s3mno+j*2);
          int i1=in->getc();
          i1+=in->getc()*256;
           in->setpos( start+s3mi-31+i1*16);
          i1=in->getc();
          if (i1==1) { // type: sample
            for (int k=0;k<31;k++) b[k]=in->getc();
            int len=b[15]+(b[16]<<8);
            int ofs=b[13]+(b[14]<<8);
            if (b[30]>1) ok=0;
            if (ofs*16<sam_start) sam_start=ofs*16;
            if (ofs*16+len>sam_end) sam_end=ofs*16+len;
          }
        }
        if (ok && sam_start<(1<<16)) AUD_DET(AUDIO,s3mi-31,sam_start,sam_end-sam_start,0);
        s3mi=0;
         in->setpos(savedpos);
      }
    }
   
    //detect rle encoded mrb files inside windows hlp files
    if (!mrb && ((buf0&0xFFFF)==0x6c70 || (buf0&0xFFFF)==0x6C50) && !b64s1 && !b64s && !b85s1 && !b85s && type!=MDF &&  !cdi)
        mrb=i,mrbsize=0,mrbPictureType=0; 
    if (mrb){
        const int p=int(i-mrb);
        if (p==1 && !c==1)    mrb=0;    // if not 1 image per/file            
        if (p==7 ){  // 5=DDB   6=DIB   8=metafile
            if ((c==5 || c==6 )) mrbPictureType=c;
            else mrb=0;
         }
        if (p==8) {         // 0=uncomp 1=RunLen 2=LZ77 3=both
           if(c==1) mrbPackingMethod=c;
           else mrb=0;
        }
        if (p==10){
          if (mrbPictureType==6 && (mrbPackingMethod==1 || mrbPackingMethod==2)){
        //save ftell
        mrbTell= in->curpos()-2;
         in->setpos(mrbTell);
        U32 Xdpi=GetCDWord(in);
        U32 Ydpi=GetCDWord(in);
        U32 Planes=GetCWord(in);
        U32 BitCount=GetCWord(in);
        mrbw=GetCDWord(in);
        mrbh=GetCDWord(in);
        U32 ColorsUsed=GetCDWord(in);
        U32 ColorsImportant=GetCDWord(in);
        mrbcsize=GetCDWord(in);
        U32 HotspotSize=GetCDWord(in);
        int CompressedOffset=(in->getc()<<24)|(in->getc()<<16)|(in->getc()<<8)|in->getc();
        int HotspotOffset=(in->getc()<<24)|(in->getc()<<16)|(in->getc()<<8)|in->getc();
        CompressedOffset=bswap(CompressedOffset);
        HotspotOffset=bswap(HotspotOffset);
        mrbsize=mrbcsize+ in->curpos()-mrbTell+10+(1<<BitCount)*4; // ignore HotspotSize
        if (BitCount!=8 || mrbw<4 || mrbw>1024) {
            mrbPictureType=mrb=mrbsize=0;
            mrbTell=mrbTell+2;
             in->setpos(mrbTell);
        }
       } else mrbPictureType=mrb=mrbsize=0;;
       }
       //if (p>10 && (mrbPictureType==0 || mrbPackingMethod!=1)) mrb=0;
       if (type==MRBR &&   (mrbPictureType==6 || mrbPictureType==8) && mrbsize){
        return  in->setpos(start+mrbsize),DEFAULT;
       }
       if ( (mrbPictureType==6) && mrbsize && mrbw>4 && mrbh>4){
        MRBRLE_DET(mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
       }
    }
    // Detect .bmp image
    if ( !(bmp || hdrless) && (((buf0&0xffff)==16973) || (!(buf0&0xFFFFFF) && ((buf0>>24)==0x28))) ) //possible 'BM' or headerless DIB
      imgbpp=bmpx=bmpy=0,hdrless=!(U8)buf0,bmpof=hdrless*54,bmp=i-hdrless*16;
    if (bmp || hdrless) {
      const int p=i-bmp;
      if (p==12) bmpof=bswap(buf0);
      else if (p==16 && buf0!=0x28000000) bmp=hdrless=0; //BITMAPINFOHEADER (0x28)
      else if (p==20) bmpx=bswap(buf0),bmp=((bmpx==0||bmpx>0x30000)?(hdrless=0):bmp); //width
      else if (p==24) bmpy=abs((int)bswap(buf0)),bmp=((bmpy==0||bmpy>0x10000)?(hdrless=0):bmp); //height
      else if (p==27) imgbpp=c,bmp=((imgbpp!=1 && imgbpp!=4 && imgbpp!=8 && imgbpp!=24 && imgbpp!=32)?(hdrless=0):bmp);
      else if ((p==31) && buf0) bmp=hdrless=0;
      else if (p==36) bmps=bswap(buf0);
      // check number of colors in palette (4 bytes), must be 0 (default) or <= 1<<bpp.
      // also check if image is too small, since it might not be worth it to use the image models
      else if (p==48){
        if ( (!buf0 || ((bswap(buf0)<=(U32)(1<<imgbpp)) && (imgbpp<=8))) && (((bmpx*bmpy*imgbpp)>>3)>64) ) {
          // possible icon/cursor?
          if (hdrless && (bmpx*2==bmpy) && imgbpp>1 &&
             (
              (bmps>0 && bmps==( (bmpx*bmpy*(imgbpp+1))>>4 )) ||
              ((!bmps || bmps<((bmpx*bmpy*imgbpp)>>3)) && (
               (bmpx==8)   || (bmpx==10) || (bmpx==14) || (bmpx==16) || (bmpx==20) ||
               (bmpx==22)  || (bmpx==24) || (bmpx==32) || (bmpx==40) || (bmpx==48) ||
               (bmpx==60)  || (bmpx==64) || (bmpx==72) || (bmpx==80) || (bmpx==96) ||
               (bmpx==128) || (bmpx==256)
              ))
             )
          )
            bmpy=bmpx;

          // if DIB and not 24bpp, we must calculate the data offset based on BPP or num. of entries in color palette
          if (hdrless && (imgbpp<24))
            bmpof+=((buf0)?bswap(buf0)*4:4<<imgbpp);
          bmpof+=(bmp-1)*(bmp<1);

          if (hdrless && bmps && bmps<((bmpx*bmpy*imgbpp)>>3)) { }//Guard against erroneous DIB detections
          else if (imgbpp==1) IMG_DET(IMAGE1,max(0,bmp-1),bmpof,(((bmpx-1)>>5)+1)*4,bmpy);
          else if (imgbpp==4) IMG_DET(IMAGE4,max(0,bmp-1),bmpof,((bmpx*4+31)>>5)*4,bmpy);
          else if (imgbpp==8){
             in->setpos(start+bmp+53);
            IMG_DET( (IsGrayscalePalette(in, (buf0)?bswap(buf0):1<<imgbpp, 1))?IMAGE8GRAY:IMAGE8,max(0,bmp-1),bmpof,(bmpx+3)&-4,bmpy);
          }
          else if (imgbpp==24) IMG_DET(IMAGE24,max(0,bmp-1),bmpof,((bmpx*3)+3)&-4,bmpy);
          else if (imgbpp==32) IMG_DET(IMAGE32,max(0,bmp-1),bmpof,bmpx*4,bmpy);
        }
        bmp=hdrless=0;
      }
    }
    // Detect .pbm .pgm .ppm .pam image
    if ((buf0&0xfff0ff)==0x50300a && txtLen<txtMinLen ) { //see if text, only single files
      pgmn=(buf0&0xf00)>>8;
     if ((pgmn>=4 && pgmn<=6) || pgmn==7) pgm=i,pgm_ptr=pgmw=pgmh=pgmc=pgmcomment=pamatr=pamd=0;
    }
    if (pgm) {
      if (i-pgm==1 && c==0x23) pgmcomment=1; //pgm comment
      if (!pgmcomment && pgm_ptr) {
        int s=0;
        if (pgmn==7) {
           if ((buf1&0xffff)==0x5749 && buf0==0x44544820) pgm_ptr=0, pamatr=1; // WIDTH
           if ((buf1&0xffffff)==0x484549 && buf0==0x47485420) pgm_ptr=0, pamatr=2; // HEIGHT
           if ((buf1&0xffffff)==0x4d4158 && buf0==0x56414c20) pgm_ptr=0, pamatr=3; // MAXVAL
           if ((buf1&0xffff)==0x4445 && buf0==0x50544820) pgm_ptr=0, pamatr=4; // DEPTH
           if ((buf2&0xff)==0x54 && buf1==0x55504c54 && buf0==0x59504520) pgm_ptr=0, pamatr=5; // TUPLTYPE
           if ((buf1&0xffffff)==0x454e44 && buf0==0x4844520a) pgm_ptr=0, pamatr=6; // ENDHDR
           if (c==0x0a) {
             if (pamatr==0) pgm=0;
             else if (pamatr<5) s=pamatr;
             if (pamatr!=6) pamatr=0;
           }
        }
        else if ((c==0x20|| c==0x0a) && !pgmw) s=1;
        else if (c==0x0a && !pgmh) s=2;
        else if (c==0x0a && !pgmc && pgmn!=4) s=3;
        if (s) {
          if (pgm_ptr>=32) pgm_ptr=31;
          pgm_buf[pgm_ptr++]=0;
          int v=atoi(&pgm_buf[0]);
          if (v<0 || v>20000) v=0;
          if (s==1) pgmw=v; else if (s==2) pgmh=v; else if (s==3) pgmc=v; else if (s==4) pamd=v;
          if (v==0 || (s==3 && v>255)) pgm=0; else pgm_ptr=0;
        }
      }
      if (!pgmcomment) pgm_buf[pgm_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
      if (pgm_ptr>=32) pgm=pgm_ptr=0;
      if (i-pgm>255) pgm=pgm_ptr=0;
      if (pgmcomment && c==0x0a) pgmcomment=0;
      if (pgmw && pgmh && !pgmc && pgmn==4) IMG_DET(IMAGE1,pgm-2,i-pgm+3,(pgmw+7)/8,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==5 || (pgmn==7 && pamd==1 && pamatr==6))) IMG_DET(IMAGE8GRAY,pgm-2,i-pgm+3,pgmw,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==6 || (pgmn==7 && pamd==3 && pamatr==6))) IMG_DET(IMAGE24,pgm-2,i-pgm+3,pgmw*3,pgmh);
      if (pgmw && pgmh && pgmc && (pgmn==7 && pamd==4 && pamatr==6)) IMG_DET(IMAGE32,pgm-2,i-pgm+3,pgmw*4,pgmh);
    }
    
   /* image in pdf
     'BI
      /W 86
      /H 85
      /BPC 1 
      /IM true
      ID '
    */ 
    if ((buf0)==0x42490D0A  && pdfi1==0 ) { 
        pdfi1=i,pdfi_ptr=pdfiw=pdfih=pdfic=pdfi_ptr=0;
    }
    if (pdfi1) {
      if (pdfi_ptr) {
        int s=0;
        if ((buf0&0xffffff)==0x2F5720) pdfi_ptr=0, pdfin=1; // /W 
        if ((buf0&0xffffff)==0x2F4820 ) pdfi_ptr=0, pdfin=2; // /H
        if ((buf1&0xff)==0x2F && buf0==0x42504320) pdfi_ptr=0, pdfin=3; // /BPC
        if (buf1==0x2F494D20 && buf0==0x74727565) pdfi_ptr=0, pdfin=4; // /IM
        if ((buf0&0xffffff)==0x435320) pdfi_ptr=0, pdfin=-1; // CS
        if ((buf0&0xffffff)==0x494420) pdfi_ptr=0, pdfin=5; // ID
        if (c==0x0a) {
           if (pdfin==0) pdfi1=0;
           else if (pdfin>0 && pdfin<4) s=pdfin;
           if (pdfin==-1) pdfi_ptr=0;
           if (pdfin!=5) pdfin=0;
           
        }
        if (s) {
          if (pdfi_ptr>=16) pdfi_ptr=16;
          pdfi_buf[pdfi_ptr++]=0;
          int v=atoi(&pdfi_buf[0]);
          if (v<0 || v>1000) v=0;
          if (s==1) pdfiw=v; else if (s==2) pdfih=v; else if (s==3) pdfic=v; else if (s==4) { };
          if (v==0 || (s==3 && v>255)) pdfi1=0; else pdfi_ptr=0;
        }
      }
      pdfi_buf[pdfi_ptr++]=((c>='0' && c<='9') || ' ')?c:0;
      if (pdfi_ptr>=16) pdfi1=pdfi_ptr=0;
      if (i-pdfi1>63) pdfi1=pdfi_ptr=0;
      if (pdfiw && pdfih && pdfic==1 && pdfin==5) IMG_DETP(IMAGE1,pdfi1-3,i-pdfi1+4,(pdfiw+7)/8,pdfih);
      if (pdfiw && pdfih && pdfic==8 && pdfin==5) IMG_DETP(IMAGE8,pdfi1-3,i-pdfi1+4,pdfiw,pdfih);
    }
    
    // Detect .rgb image
    if ((buf0&0xffff)==0x01da) rgbi=i,rgbx=rgby=0;
    if (rgbi) {
      const int p=int(i-rgbi);
      if (p==1 && c!=0) rgbi=0;
      else if (p==2 && c!=1) rgbi=0;
      else if (p==4 && (buf0&0xffff)!=1 && (buf0&0xffff)!=2 && (buf0&0xffff)!=3) rgbi=0;
      else if (p==6) rgbx=buf0&0xffff,rgbi=(rgbx==0?0:rgbi);
      else if (p==8) rgby=buf0&0xffff,rgbi=(rgby==0?0:rgbi);
      else if (p==10) {
        int z=buf0&0xffff;
        if (rgbx && rgby && (z==1 || z==3 || z==4)) IMG_DET(IMAGE8,rgbi-1,512,rgbx,rgby*z);
        rgbi=0;
      }
    }
      
    // Detect .tiff file header (2/8/24 bit color, not compressed).
   if (((buf1==0x49492a00 ||buf1==0x4949524f ) && n>i+(int)bswap(buf0) && tiffImages==-1)|| 
       ((buf1==0x4d4d002a  ) && n>i+(int)(buf0) && tiffImages==-1)){
      if (buf1==0x4d4d002a) tiffMM=true;
       tiffImageStart=0,tiffImages=-1;
       U64 savedpos=in->curpos();
       int dirEntry=tiffMM==true?(int)buf0:(int)bswap(buf0);
       in->setpos(start+i+dirEntry-7);

      // read directory
      int dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
      int tifx=0,tify=0,tifz=0,tifzb=0,tifc=0,tifofs=0,tifofval=0,b[12],tifsiz=0;
      for (;;){
        if (dirsize>0 && dirsize<256) {            
        tiffImages++;
        for (int i=0; i<dirsize; i++) {
          for (int j=0; j<12; j++) b[j]=in->getc();
          if (b[11]==EOF) break;
          int tag=tiffMM==false? b[0]+(b[1]<<8):(int)bswap(b[0]+(b[1]<<8))>>16;;
          int tagfmt=tiffMM==false? b[2]+(b[3]<<8):(int)bswap(b[2]+(b[3]<<8))>>16;;
          int taglen=tiffMM==false?b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24):(int)bswap(b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24));;
          int tagval=tiffMM==false?b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24):(int)bswap(b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24));;
          //printf("Tag %d  val %d\n",tag, tagval);
          if (tagfmt==3||tagfmt==4) {
              
            if (tag==256) tifx=tagval,tiffFiles[tiffImages].width=tifx;
            else if (tag==257) tify=tagval,tiffFiles[tiffImages].height=tify;
            else if (tag==258) tifzb=(tagval==12||tagval==14||tagval==16)?14:taglen==1?tagval:8,tiffFiles[tiffImages].bits1=tifzb; // bits per component
            else if (tag==259) tifc=tagval, tiffFiles[tiffImages].compression=tifc ; // 1 = no compression, 6 jpeg
            else if (tag==273 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs;
            else if (tag==513 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs; //jpeg
            else if (tag==514 && tagfmt==4) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz,tiffFiles[tiffImages].compression=6; //jpeg
            else if (tag==277) tifz=tagval,tiffFiles[tiffImages].bits=tifz; // components per pixel
            else if (tag==279) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz;
             else if (tag==50752 || tag==50649) tiffFiles[tiffImages].size=0; //to skip cr2 jpg
            else if (tag==330 ||  tag==34665) {
                int a=tfidf.size();
                if (a==0) tfidf.resize(a+taglen);
                U64 savedpos1= in->curpos();
                 in->setpos( start+i+tagval-5);
                if (a==0 && taglen==1) tfidf[a]=tagval;
                else if (taglen==1) tfidf[a+1]=tagval;
                else{
                
                for (int i2=0;i2<taglen; i2++) {
                     int g;
                     if (tiffMM==false) 
                   g=(in->getc()|(in->getc()<<8)|(in->getc()<<16)|(in->getc()<<24));
                    else
                    g=(in->getc()<<24|(in->getc()<<16)|(in->getc()<<8)|(in->getc()));
                    tfidf[i2]=g;
                }
                }
                 in->setpos(savedpos1);               
            }
          }
        }
         if(tiffFiles[tiffImages].size==0)tiffImages--;
        }
        int gg=in->getc()|(in->getc()<<8)|(in->getc()<<24)|(in->getc()<<16);
          gg=tiffMM==false?(int)gg:(int)bswap(gg);
        if (gg>0) {
         in->setpos( start+i+(gg)-7);
        dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));//in->getc();
        }
        else break;
        
      }
       //
       if(int a=tfidf.size()>0){
            a++;
            for (int i2=0;i2<a; i2++) { 
               in->setpos( start+i+tfidf[i2]-7);
          // read directory
      int dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
      int tifx=0,tify=0,tifz=0,tifzb=0,tifc=0,tifofs=0,tifofval=0,b[12],tifsiz=0;
      if (dirsize>0 && dirsize<256) {
           tiffImages++;
        for (int i1=0; i1<dirsize; i1++) {
          for (int j=0; j<12; j++) b[j]=in->getc();
          if (b[11]==EOF) break;
          int tag=tiffMM==false? b[0]+(b[1]<<8):(int)bswap(b[0]+(b[1]<<8))>>16;;
          int tagfmt=tiffMM==false? b[2]+(b[3]<<8):(int)bswap(b[2]+(b[3]<<8))>>16;;
          int taglen=tiffMM==false?b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24):(int)bswap(b[4]+(b[5]<<8)+(b[6]<<16)+(b[7]<<24));;
          int tagval=tiffMM==false?b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24):(int)bswap(b[8]+(b[9]<<8)+(b[10]<<16)+(b[11]<<24));;
          // printf("Tag %d  val %d\n",tag, tagval);
          if (tagfmt==3||tagfmt==4) {
            if (tag==256) tifx=tagval,tiffFiles[tiffImages].width=tifx;
            else if (tag==257) tify=tagval,tiffFiles[tiffImages].height=tify;
            else if (tag==258) tifzb=(tagval==12||tagval==14||tagval==16)?14:taglen==1?tagval:8,tiffFiles[tiffImages].bits1=tifzb; // bits per component
            else if (tag==259) tifc=tagval, tiffFiles[tiffImages].compression=tifc ; // 1 = no compression, 6 jpeg
            else if (tag==273 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs;
            else if (tag==513 && tagfmt==4) tifofs=tagval,tifofval=(taglen<=1),tiffFiles[tiffImages].offset=tifofs; //jpeg
            else if (tag==514 && tagfmt==4) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz,tiffFiles[tiffImages].compression=6; //jpeg
            else if (tag==277) tifz=tagval,tiffFiles[tiffImages].bits=tifz; // components per pixel
            else if (tag==279) tifsiz=tagval,tiffFiles[tiffImages].size=tifsiz;
            else if (tag==50752) tiffFiles[tiffImages].size=0;
           
          }
        }
        if(tiffFiles[tiffImages].size==0)tiffImages--;
      }
      }
        
      }
      tiffImageStart= start+i-7;
      tiffImages++;
      for (int o=0;o<tiffImages; o++) { 
      if (tiffFiles[o].height && tiffFiles[o].bits1==14)tiffFiles[o].width=tiffFiles[o].size/tiffFiles[o].height;
       //printf("Offset %d Size %d Compression %d Width %d Height %d Bits %d Bits1 %d \n",tiffFiles[o].offset,tiffFiles[o].size,(U32)tiffFiles[o].compression,tiffFiles[o].width,tiffFiles[o].height,tiffFiles[o].bits,tiffFiles[o].bits1 );
       tiffFiles[o].offset+=tiffImageStart;
       }
               //
      if (tifx>1 && tify && tifzb && (tifz==1 || tifz==3) && (tifc==1) && (tifofs && tifofs+i<n)) {
        if (!tifofval) {
           in->setpos( start+i+tifofs-7);
          for (int j=0; j<4; j++) b[j]=in->getc();
          tifofs=b[0]+(b[1]<<8)+(b[2]<<16)+(b[3]<<24);
        }
        if (tifofs && tifofs<(1<<18) && tifofs+i<n && tifx>1) {
          if (tifz==1 && tifzb==1) IMG_DET(IMAGE1,i-7,tifofs,((tifx-1)>>3)+1,tify);
          else if (tifz==1 && tifzb==8 && tifx<30000) IMG_DET(IMAGE8,i-7,tifofs,tifx,tify);
          else if (tifz==3 && tifzb==8 && tifx<30000) IMG_DET(IMAGE24,i-7,tifofs,tifx*3,tify);
        }
      }
      in->setpos( savedpos);
    }
    

       
    // Detect .tga image (8-bit 256 colors or 24-bit uncompressed)
    if (buf1==0x00010100 && buf0==0x00000118) tga=i,tgax=tgay,tgaz=8,tgat=1;
    else if (buf1==0x00000200 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=24,tgat=2;
    else if (buf1==0x00000300 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=8,tgat=3;
    if (tga) {
      if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
      else if (i-tga==10) {
          if ((buf0&0xffff)>>8==32 && (buf0&0xfF)==0) tgaz=32;
        if (tgaz==(int)((buf0&0xffff)>>8) && tgax<30000 && tgax>1 && tgay>1 && tgay<30000){
          if (tgat==1) {
             in->setpos( start+tga+11);
            IMG_DET( (IsGrayscalePalette(in))?IMAGE8GRAY:IMAGE8,tga-7,18+256*3,tgax,tgay);
          }
          else if (tgat==2 && tgaz==24) IMG_DET(IMAGE24,tga-7,18,tgax*3,tgay);
          else if (tgat==2 && tgaz==32) IMG_DET(IMAGE32,tga-7,18,tgax*4,tgay);
          else if (tgat==3 ) IMG_DET(IMAGE8,tga-7,18,tgax,tgay);
        }
        tga=0;
      }
    }
    // Detect .gif
    if (type==DEFAULT && dett==GIF && i==0) {
      dett=DEFAULT;
      if (c==0x2c || c==0x21) gif=2,gifi=2;
    }
    if (!gif && (buf1&0xffff)==0x4749 && (buf0==0x46383961 || buf0==0x46383761)) gif=1,gifi=i+5;
    if (gif) {
      if (gif==1 && i==gifi) gif=2,gifi = i+5+(gifplt=(c&128)?(3*(2<<(c&7))):0),brute=false;
      if (gif==2 && gifplt && i==gifi-gifplt-2) gifgray = IsGrayscalePalette(in, gifplt/3), gifplt = 0;
      if (gif==2 && i==gifi) {
        if ((buf0&0xff0000)==0x210000) gif=5,gifi=i;
        else if ((buf0&0xff0000)==0x2c0000) gif=3,gifi=i;
        else gif=0;
      }
      if (gif==3 && i==gifi+6) gifw=(bswap(buf0)&0xffff),gifw=gifw>3?gifw:0;
      if (gif==3 && i==gifi+7) gif=4,gifc=gifb=0,gifa=gifi=i+2+((c&128)?(3*(2<<(c&7))):0); //bswap(buf0)&0xffff) &3+1 bpp
      if (gif==4 && i==gifi) {
        if (c>0 && gifb && gifc!=gifb) gifw=0;
        if (c>0) gifb=gifc,gifc=c,gifi+=c+1;
        else if (!gifw) gif=2,gifi=i+3;
        else if (i-gifa+2<10) gif=0;
        else return  in->setpos( start+gifa-1),detd=i-gifa+2,info=((gifgray?IMAGE8GRAY:IMAGE8)<<24)|gifw,dett=GIF;
      }
      if (gif==5 && i==gifi) {
        if (c>0) gifi+=c+1; else gif=2,gifi=i+3;
      }
    }
    
    // Detect EXE if the low order byte (little-endian) XX is more
    // recently seen (and within 4K) if a relative to absolute address
    // conversion is done in the context CALL/JMP (E8/E9) XX xx xx 00/FF
    // 4 times in a row.  Detect end of EXE at the last
    // place this happens when it does not happen for 64KB.

    if (((buf1&0xfe)==0xe8 || (buf1&0xfff0)==0x0f80) && ((buf0+1)&0xfe)==0) {
      int r=buf0>>24;  // relative address low 8 bits
      int a=((buf0>>24)+i)&0xff;  // absolute address low 8 bits
      U64 rdist=(i-relpos[r]);
      U64 adist=(i-abspos[a]);
      if (adist<rdist && adist<0x800 && abspos[a]>5) {
        e8e9last=i;
        ++e8e9count;
        if (e8e9pos==0 || e8e9pos>abspos[a]) e8e9pos=abspos[a];
      }
      else e8e9count=0;
      if (type==DEFAULT && e8e9count>=4 && e8e9pos>5)
        return  in->setpos( start+e8e9pos-5), EXE;
      abspos[a]=i;
      relpos[r]=i;
    }
    if (i-e8e9last>0x4000) {
      if (type==EXE) return  in->setpos( start+e8e9last), DEFAULT;
      e8e9count=0,e8e9pos=0;
    }

    // DEC Alpha
    op=bswap(buf0)>>21; 
    //test if bsr opcode and if last 3 opcodes are valid
    if ((op==0x34*32+26) && CAlpha2(bswap(buf1))==true && CAlpha2(bswap(buf2))==true && CAlpha2(bswap(buf3))==true && e8e9count==0 &&
     !tar && !soi && !pgm && !rgbi && !bmp && !wavi && !tga) {
      int a=op&0xff;// absolute address low 8 bits
      int r=op&0x1fffff;
      r+=(i)/4;  // relative address low 8 bits
      r=r&0xff;
      int rdist=int(i-relposDEC[r]);
      int adist=int(i-absposDEC[a]);
      if (adist<rdist && adist<0x1fffff && absposDEC[a]>16 &&  adist>16 && adist%4==0) {
        DEClast=i;
        ++DECcount;
        if (DECpos==0 || DECpos>absposDEC[a]) DECpos=absposDEC[a];
      }
      else DECcount=0;
      if (type==DEFAULT && DECcount>=16 && DECpos>16 ) 
          return  in->setpos(start+DECpos-DECpos%4), DECA;
      absposDEC[a]=i;
      relposDEC[r]=i;
    }
    if (i-DEClast>0x4000) {
      if (type==DECA)       
      return  in->setpos( start+DEClast-DEClast%4), DEFAULT;
      DECcount=0,DECpos=0,DEClast=0;
      memset(&relposDEC[0], 0, sizeof(relposDEC));
      memset(&absposDEC[0], 0, sizeof(absposDEC));
    }
 
    // ARM
    op=(buf0)>>26; 
    //test if bl opcode and if last 3 opcodes are valid 
    // BL(4) and (ADD(1) or MOV(4)) as previous, 64 bit
    // ARMv8-A_Architecture_Reference_Manual_(Issue_A.a).pdf
    if (op==0x25 && 
    (((buf1)>>26==0x25 ||(buf2)>>26==0x25||(buf3)>>26==0x25 ) ||
    (( ((buf1)>>24)&0x7F==0x11 || ((buf1)>>23)&0x7F==0x25  || ((buf1)>>23)&0x7F==0xa5 || ((buf1)>>23)&0x7F==0x64 || ((buf1)>>24)&0x7F==0x2A) && (buf1)>>31==1)
    )&& e8e9count==0 &&  !tar && !soi && !pgm && !rgbi && !bmp && !wavi && !tga){ 
      int a=(buf0)&0xff;// absolute address low 8 bits
      int r=(buf0)&0x3FFFFFF;
      r+=(i)/4;  // relative address low 8 bits
      r=r&0xff;
      int rdist=int(i-relposARM[r]);
      int adist=int(i-absposARM[a]);
      if (adist<rdist && adist<0x3FFFFFF && absposARM[a]>16 &&  adist>16 && adist%4==0) {
        ARMlast=i;
        ++ARMcount;
        if (ARMpos==0 || ARMpos>absposARM[a]) ARMpos=absposARM[a];
      }
      else ARMcount=0;
      if (type==DEFAULT && ARMcount>=16 && ARMpos>16 ) 
          return  in->setpos(start+ARMpos-ARMpos%4), ARM;
      absposARM[a]=i;
      relposARM[r]=i;
    }
    if (i-ARMlast>0x4000) {
      if (type==ARM)
      return  in->setpos( start+ARMlast-ARMlast%4), DEFAULT;
      ARMcount=0,ARMpos=0,ARMlast=0;
      memset(&relposARM[0], 0, sizeof(relposARM));
      memset(&absposARM[0], 0, sizeof(absposARM));
    }
    // base64 encoded data detection
    // detect base64 in html/xml container, single stream
    // ';base64,' or '![CDATA[' :image> 3a696d6167653e
    if (b64s1==0 &&   ((buf1==0x3b626173 && buf0==0x6536342c)||(buf1==0x215b4344 && buf0==0x4154415b) )) b64s1=1,b64h=i+1,base64start=i+1; //' base64' ||((buf1&0xffffff)==0x3a696d && buf0==0x6167653e)
    else if (b64s1==1 && (isalnum(c) || (c == '+') || (c == '/')||(c == '=')) ) {
        continue;
        }  
    else if (b64s1==1) {
         base64end=i,b64s1=0;
         if (base64end -base64start>128) B64_DET(BASE64,b64h, 8,base64end -base64start);
    }
   
   // detect base64 in eml, etc. multiline
   if (b64s==0 && buf0==0x73653634 && ((buf1&0xffffff)==0x206261 || (buf1&0xffffff)==0x204261)) b64s=1,b64p=i-6,b64h=0,b64slen=0,b64lcount=0; //' base64' ' Base64'
    else if (b64s==1 && buf0==0x0D0A0D0A ) {
        b64s=2,b64h=i+1,b64slen=b64h-b64p;
        base64start=i+1;
        if (b64slen>192) b64s=0; //drop if header is larger 
        }
    else if (b64s==2 && (buf0&0xffff)==0x0D0A && b64line==0) {
         b64line=i-base64start,b64nl=i+2;//capture line lenght
         if (b64line<=4 || b64line>255) b64s=0;
         //else continue;
    }
    else if (b64s==2 && (buf0&0xffff)==0x0D0A  && b64line!=0 && (buf0&0xffffff)!=0x3D0D0A && buf0!=0x3D3D0D0A ){
         if (i-b64nl+1<b64line && buf0!=0x0D0A0D0A) { // if smaller and not padding
            base64end=i-1;
            if (((base64end-base64start)>512) && ((base64end-base64start)<base64max)){
             b64s=0;
             B64_DET(BASE64,b64h,b64slen,base64end -base64start);
            }
         }
         else if (buf0==0x0D0A0D0A) { // if smaller and not padding
           base64end=i-1-2;
           if (((base64end-base64start)>512) && ((base64end-base64start)<base64max))
               B64_DET(BASE64,b64h,b64slen,base64end -base64start);
           b64s=0;
         }
         b64nl=i+2; //update 0x0D0A pos
         b64lcount++;
         continue;
         }
    else if (b64s==2 && ((buf0&0xffffff)==0x3D0D0A ||buf0==0x3D3D0D0A)) { //if padding '=' or '=='
        base64end=i-1;
        b64s=0;
        if (((base64end-base64start)>512) && ((base64end-base64start)<base64max))
            B64_DET(BASE64,b64h,b64slen,base64end -base64start);
    }
    else if (b64s==2 && (is_base64(c) || c=='='))   ;//continue;
    else if (b64s==2)   b64s=0;
    
    //detect ascii85 encoded data
    //headers: stream\n stream\r\n oNimage\n utimage\n \nimage\n
    if (b85s==0 && ((buf0==0x65616D0A && (buf1&0xffffff)==0x737472)|| (buf0==0x616D0D0A && buf1==0x73747265)|| (buf0==0x6167650A && buf1==0x6F4E696D) || (buf0==0x6167650A && buf1==0x7574696D) || (buf0==0x6167650A && (buf1&0xffffff)==0x0A696D))){
        b85s=1,b85p=i-6,b85h=0,b85slen=0;//,b85lcount=0; // 
        b85s=2,b85h=i+1,b85slen=b85h-b85p;
        base85start=i+1;
        if (b85slen>128) b85s=0; //drop if header is larger 
        }
    else if (b85s==2){
        if  ((buf0&0xff)==0x0d && b85line==0) {
            b85line=i-base85start;//,b85nl=i+2;//capture line lenght
            if (b85line<=4 || b85line>255) b85s=0;
        }
        
        else if ( ((buf0&0xff)==0x7E|| (buf0&0xffff)==0x0a7E)) { //if padding '~' or '=='
            base85end=i-((buf0&0xff00)==0x0a00);
            b85s=0;
            if (((base85end-base85start)>60) && ((base85end-base85start)<base85max))
            B85_DET(BASE85,b85h,b85slen,base85end -base85start);
        }
        else if ( (is_base85(c)))          ;
        else if  ((buf0&0xff)==0x0d && b85line!=0) {
            if (b85line!=i-base85start) b85s=0;
        }
        else     b85s=0;   
    }
  if (buf0==0x0A42540A) doBT=true,BTcount++;//nl BT nl
    if (buf0==0x0A45540A) doBT=false;//nl ET nl
     //Detect text and utf-8   teolc=0,teoll=0;
    if (txtStart==0  && !tar && !soi && !pgm && !rgbi && !bmp && !wavi && !tga && !b64s1 && !b64s && !b85s1 && !b85s &&
        ((c<128 && c>=32) || c==10 || c==13 || c==0x12 || c==9 )) txtStart=1,txtOff=i,txt0=0,txta=0,txtNL=0,txtNLC=0,txtNLT=0;
    if (txtStart   ) {
        // report as text if over minumum lenght
        
        if ((c<128 && c>=32) || c==10 || c==13 || c==0x12 || c==9|| c==5 || doBT==true) {
            ++txtLen;
            if ( txtLen>txtMinLen) brute=false; //disable zlib brute if text lenght is over minimum.
            if ((c>='a' && c<='z') ||  (c>='A' && c<='Z')) txta++;
            if (c>='0' && c<='9') txt0++;
            if (i-txtbinp>512) txtbinc=txtbinc>>2;
            if (txtNL==0 && c==10 ) txtNL=i;
            else if (txtNL>0 && c==10 ) {
                int tNL=i-txtNL;
                if (tNL<90 && tNL>45) 
                txtNLC++;          //Count if in range   
                else 
                txtNLT+=tNL>3?1:0; //Total new line count
                
                txtNL=i;
            }
       }
       else if ((c&0xE0)==0xc0 && utfc==0){ //if possible UTF8 2 byte
            utfc=2,utfb=1;
            }
       else if ((c&0xF0)==0xE0 && utfc==0){//if possible UTF8 3 byte
            utfc=3,utfb=1;
            }
       else if ((c&0xF8)==0xF0 && utfc==0){//if possible UTF8 4 byte
            utfc=4,utfb=1;
            }
       else if ((c&0xFC)==0xF8 && utfc==0){ //if possible UTF8 5 byte
            utfc=5,utfb=1;
            }
       else if (utfc>=2 && utfb>=1 && utfb<=5 && (c&0xC0)==0x80){
            utfb=utfb+1; //inc byte count
            //overlong UTF-8 sequence, UTF-16 surrogates as well as U+FFFE and U+FFFF test needed!!!
            switch (utfc){ //switch by detected type
            case 2:
                 if (utfb==2) txtLen=txtLen+2,utfc=0,utfb=0,txtIsUTF8=1,txta=txta+2;
                  break;
            case 3:
                 if (utfb==3) txtLen=txtLen+3,utfc=0,utfb=0,txtIsUTF8=1,txta=txta+3;
                  break;
            case 4:
                 if (utfb==4) txtLen=txtLen+4,utfc=0,utfb=0,txtIsUTF8=1,txta=txta+4;
                 break;
            case 5:
                 if (utfb==5) txtLen=txtLen+5,utfc=0,utfb=0,txtIsUTF8=1,txta=txta+5;
                 break;
            }
       }
       else if (utfc>=2 && utfb>=1){ // wrong utf
             txtbinc=txtbinc+1,     txtbinp=i;
             txtLen=txtLen+utfb;
             ++txtLen;
            utfc=utfb=txtIsUTF8=0;
            if (txtbinc>=128){
            if ( txtLen<txtMinLen){
               txtStart=txtOff=txtLen=txtbinc=0;
                   txtIsUTF8=utfc=utfb=0;
            }
            else{
                if (txta<txt0) info=1+(BTcount>255); else info=0; //use num0-9
                if (type==TEXT|| type==TEXT0 ||type== TXTUTF8|| type==EOLTEXT) return  in->setpos( start+txtLen),DEFAULT;
                if (info==1)  return  in->setpos( start+txtOff),TEXT0;
                if (txtNLC>txtNLT && it==0)  return  in->setpos( start+txtOff),EOLTEXT;
                if (txtIsUTF8==1)
                    return  in->setpos( start+txtOff),TXTUTF8;
                else
                    return  in->setpos( start+txtOff),TEXT;
            }
          
        }
       }
       else if (txtLen<txtMinLen){
            txtbinc=txtbinc+1,     txtbinp=i;
            ++txtLen;
            if (txtbinc>=128)
            txtStart=txtLen=txtOff=txtbinc=0;
            utfc=utfb=0;
       }
       else if (txtLen>=txtMinLen){
            txtbinc=txtbinc+1,     txtbinp=i;
            ++txtLen;
            if (txtbinc>=128){
                if (txta<txt0)   info=1+(BTcount>255); else info=0 ;
                if (type==TEXT|| type==TEXT0 ||type== TXTUTF8|| type==EOLTEXT) return  in->setpos( start+txtLen),DEFAULT;
                if (info==1)  return  in->setpos( start+txtOff),TEXT0;
                if (txtNLC>txtNLT && it==0)  return  in->setpos(start+txtOff),EOLTEXT;
                if (txtIsUTF8==1)
                    return  in->setpos(start+txtOff),TXTUTF8;
                 else
                    return  in->setpos( start+txtOff),TEXT;
            }
       } 
       if ( txtLen>=(txtMinLen) && type==DEFAULT || (s1==0 && (txtLen)==n && type==DEFAULT)) { 
            if (txta<txt0)               return in->setpos( start+txtOff),info=1+(BTcount>255),TEXT0;
            if (txtNLC>txtNLT && it==0)  return in->setpos( start+txtOff),EOLTEXT;
            if (txtIsUTF8==1)            return in->setpos( start+txtOff),TXTUTF8;
            else                         return in->setpos( start+txtOff),TEXT;
        }
    }
  }
     
  return type;


}

typedef enum {FDECOMPRESS, FCOMPARE, FDISCARD} FMode;

// Print progress: n is the number of bytes compressed or decompressed
void printStatus(U64 n, U64 size,int tid=-1) {
if (level>0 && tid>=0)  fprintf(stderr,"%2d %6.2f%%\b\b\b\b\b\b\b\b\b\b",tid, float(100)*n/(size+1)), fflush(stdout);
else if (level>0)  fprintf(stderr,"%6.2f%%\b\b\b\b\b\b\b", float(100)*n/(size+1)), fflush(stdout);
}

void encode_cd(File* in, File* out, int len, int info) {
  const int BLOCK=2352;
  U8 blk[BLOCK];
  out->putc((len%BLOCK)>>8);
  out->putc(len%BLOCK);
  for (int offset=0; offset<len; offset+=BLOCK) {
    if (offset+BLOCK > len) {
       in->blockread(&blk[0],   len-offset );
      out->blockwrite(&blk[0],  len-offset  );
    } else {
      in->blockread(&blk[0],   BLOCK  );
      if (info==3) blk[15]=3;
      if (offset==0) out->blockwrite(&blk[12],   4+4*(blk[15]!=1)  );
      out->blockwrite(&blk[16+8*(blk[15]!=1)],   2048+276*(info==3)  );
      if (offset+BLOCK*2 > len && blk[15]!=1) out->blockwrite(&blk[16],  4  );
    }
  }
}

int decode_cd(File*in, int size, File*out, FMode mode, U64 &diffFound) {
  const int BLOCK=2352;
  U8 blk[BLOCK];
  long i=0, i2=0;
  int a=-1, bsize=0, q=in->getc();
  q=(q<<8)+in->getc();
  size-=2;
  while (i<size) {
    if (size-i==q) {
      in->blockread(blk, q  );
      out->blockwrite(blk, q  );
      i+=q;
      i2+=q;
    } else if (i==0) {
      in->blockread(blk+12, 4  );
      if (blk[15]!=1) in->blockread(blk+16, 4  );
      bsize=2048+(blk[15]==3)*276;
      i+=4*(blk[15]!=1)+4;
    } else {
      a=(blk[12]<<16)+(blk[13]<<8)+blk[14];
    }
    in->blockread(blk+16+(blk[15]!=1)*8, bsize   );
    i+=bsize;
    if (bsize>2048) blk[15]=3;
    if (blk[15]!=1 && size-q-i==4) {
      in->blockread(blk+16, 4   );
      i+=4;
    }
    expand_cd_sector(blk, a, 0);
    if (mode==FDECOMPRESS) out->blockwrite(blk, BLOCK  );
    else if (mode==FCOMPARE) for (int j=0; j<BLOCK; ++j) if (blk[j]!=out->getc() && !diffFound) diffFound=i2+j+1;
    i2+=BLOCK;
  }
  return i2;
}


// 24-bit image data transform:
// simple color transform (b, g, r) -> (g, g-r, g-b)

void encode_bmp(File* in, File* out, int len, int width) {
  int r,g,b;
  for (int i=0; i<len/width; i++) {
    for (int j=0; j<width/3; j++) {
      b=in->getc(), g=in->getc(), r=in->getc();
      out->putc(g);
      out->putc(g-r);
      out->putc(g-b);
    }
    for (int j=0; j<width%3; j++) out->putc(in->getc());
  }
}

int decode_bmp(Encoder& en, int size, int width, File*out, FMode mode, U64 &diffFound) {
  int r,g,b,p;
  for (int i=0; i<size/width; i++) {
    p=i*width;
    for (int j=0; j<width/3; j++) {
      b=en.decompress(), g=en.decompress(), r=en.decompress();
      if (mode==FDECOMPRESS) {
        out->putc(b-r);
        out->putc(b);
        out->putc(b-g);
      }
      else if (mode==FCOMPARE) {
        if (((b-r)&255)!=out->getc() && !diffFound) diffFound=p+1;
        if (b!=out->getc() && !diffFound) diffFound=p+2;
        if (((b-g)&255)!=out->getc() && !diffFound) diffFound=p+3;
        p+=3;
      }
    }
    for (int j=0; j<width%3; j++) {
      if (mode==FDECOMPRESS) {
        out->putc(en.decompress());
      }
      else if (mode==FCOMPARE) {
        if (en.decompress()!=out->getc() && !diffFound) diffFound=p+j+1;
      }
    }
  }
  return size;
}
// 32-bit image
void encode_im32(File* in, File* out, int len, int width) {
  int r,g,b,a;
  for (int i=0; i<len/width; i++) {
    for (int j=0; j<width/4; j++) {
      b=in->getc(), g=in->getc(), r=in->getc(); a=in->getc();
      out->putc(g);
      out->putc(g-r);
      out->putc(g-b);
      out->putc(a);
    }
    for (int j=0; j<width%4; j++) out->putc(in->getc());
  }
}

int decode_im32(Encoder& en, int size, int width, File*out, FMode mode, U64 &diffFound) {
  int r,g,b,a,p;
  bool rgb = (width&(1<<31))>0;
  if (rgb) width^=(1<<31);
  for (int i=0; i<size/width; i++) {
    p=i*width;
    for (int j=0; j<width/4; j++) {
      b=en.decompress(), g=en.decompress(), r=en.decompress(), a=en.decompress();
      if (mode==FDECOMPRESS) {
        out->putc(b-r); out->putc(b); out->putc(b-g); out->putc(a);
      }
      else if (mode==FCOMPARE) {
        if (((b-r)&255)!=out->getc() && !diffFound) diffFound=p+1;
        if (b!=out->getc() && !diffFound) diffFound=p+2;
        if (((b-g)&255)!=out->getc() && !diffFound) diffFound=p+3;
        if (((a)&255)!=out->getc() && !diffFound) diffFound=p+4;
        p+=4;
      }
    }
    for (int j=0; j<width%4; j++) {
      if (mode==FDECOMPRESS) {
        out->putc(en.decompress());
      }
      else if (mode==FCOMPARE) {
        if (en.decompress()!=out->getc() && !diffFound) diffFound=p+j+1;
      }
    }
  }
  return size;
}
// EXE transform: <encoded-size> <begin> <block>...
// Encoded-size is 4 bytes, MSB first.
// begin is the offset of the start of the input file, 4 bytes, MSB first.
// Each block applies the e8e9 transform to strings falling entirely
// within the block starting from the end and working backwards.
// The 5 byte pattern is E8/E9 xx xx xx 00/FF (x86 CALL/JMP xxxxxxxx)
// where xxxxxxxx is a relative address LSB first.  The address is
// converted to an absolute address by adding the offset mod 2^25
// (in range +-2^24).

void encode_exe(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  out->put32((U32)begin);

  // Transform
  for (int offset=0; offset<len; offset+=BLOCK) {
    int size=min(int(len-offset), BLOCK);
    int bytesRead= in->blockread(&blk[0],   size );
    if (bytesRead!=size) quit("encode_exe read error");
    for (int i=bytesRead-1; i>=5; --i) {
      if ((blk[i-4]==0xe8 || blk[i-4]==0xe9 || (blk[i-5]==0x0f && (blk[i-4]&0xf0)==0x80))
         && (blk[i]==0||blk[i]==0xff)) {
        int a=(blk[i-3]|blk[i-2]<<8|blk[i-1]<<16|blk[i]<<24)+offset+begin+i+1;
        a<<=7;
        a>>=7;
        blk[i]=a>>24;
        blk[i-1]=a^176;
        blk[i-2]=(a>>8)^176;
        blk[i-3]=(a>>16)^176;
      }
    }
    out->blockwrite(&blk[0],   bytesRead  );
  }
}

U64 decode_exe(Encoder& en, int size, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
  const int BLOCK=0x10000;  // block size
  int begin, offset=6, a, showstatus=(s2!=0);
  U8 c[6];
  begin=en.decompress()<<24;
  begin|=en.decompress()<<16;
  begin|=en.decompress()<<8;
  begin|=en.decompress();
  size-=4;
  for (int i=4; i>=0; i--) c[i]=en.decompress();  // Fill queue

  while (offset<size+6) {
    memmove(c+1, c, 5);
    if (offset<=size) c[0]=en.decompress();
    // E8E9 transform: E8/E9 xx xx xx 00/FF -> subtract location from x
    if ((c[0]==0x00 || c[0]==0xFF) && (c[4]==0xE8 || c[4]==0xE9 || (c[5]==0x0F && (c[4]&0xF0)==0x80))
     && (((offset-1)^(offset-6))&-BLOCK)==0 && offset<=size) { // not crossing block boundary
      a=((c[1]^176)|(c[2]^176)<<8|(c[3]^176)<<16|c[0]<<24)-offset-begin;
      a<<=7;
      a>>=7;
      c[3]=a;
      c[2]=a>>8;
      c[1]=a>>16;
      c[0]=a>>24;
    }
    if (mode==FDECOMPRESS) out->putc(c[5]);
    else if (mode==FCOMPARE && c[5]!=out->getc() && !diffFound) diffFound=offset-6+1;
    if (showstatus && !(offset&0xfff)) printStatus(s1+offset-6, s2);
    offset++;
  }
  return size;
}

class zLibMTF{
private:
  int Root, Index;
  Array<int, 16> Previous;
  Array<int, 16> Next;
public:
  zLibMTF(): Root(0), Index(0), Previous(81), Next(81) {
    for (int i=0;i<81;i++) {
      Previous[i] = i-1;
      Next[i] = i+1;
    }
    Next[80] = -1;
  }
  inline int GetFirst(){
    return Index=Root;
  }
  inline int GetNext(){
    if(Index>=0){Index=Next[Index];return Index;}
    return Index; //-1
  }
  inline void MoveToFront(int i){
    assert(i>=0 && i<=80);
    if ((Index=i)==Root) return;
    int p=Previous[Index];
    int n=Next[Index];
    if(p>=0)Next[p] = Next[Index];
    if(n>=0)Previous[n] = Previous[Index];
    Previous[Root] = Index;
    Next[Index] = Root;
    Root=Index;
    Previous[Root]=-1;
  }
};

#define ZLIB_NUM_COMBINATIONS 81
zLibMTF MTF;
int encode_zlib(File* in, File* out, int len) {
  const int BLOCK=1<<16, LIMIT=128;
  U8 zin[BLOCK*2],zout[BLOCK],zrec[BLOCK*2];//, diffByte[81*LIMIT];
  Array<U8>  diffByte(ZLIB_NUM_COMBINATIONS*LIMIT);
  //int diffPos[81*LIMIT];
  Array<int>  diffPos(ZLIB_NUM_COMBINATIONS*LIMIT);
  // Step 1 - parse offset type form zlib stream header
  U64 pos= in->curpos();
  unsigned int h1=in->getc(), h2=in->getc();
   in->setpos( pos);
  int zh=parse_zlib_header(h1*256+h2);
  int memlevel,clevel,window=zh==-1?0:MAX_WBITS+10+zh/4,ctype=zh%4;
  int minclevel=window==0?1:ctype==3?7:ctype==2?6:ctype==1?2:1;
  int maxclevel=window==0?9:ctype==3?9:ctype==2?6:ctype==1?5:1;
  int index=-1;
  bool found=false;
  // Step 2 - check recompressiblitiy, determine parameters and save differences
  z_stream main_strm, rec_strm[ZLIB_NUM_COMBINATIONS];
  int diffCount[ZLIB_NUM_COMBINATIONS], recpos[ZLIB_NUM_COMBINATIONS], main_ret=Z_STREAM_END;
  main_strm.zalloc=Z_NULL; main_strm.zfree=Z_NULL; main_strm.opaque=Z_NULL;
  main_strm.next_in=Z_NULL; main_strm.avail_in=0;
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) return false;
  for (int i=0; i<ZLIB_NUM_COMBINATIONS; i++) {
      clevel=(i/9)+1;
    // Early skip if invalid parameter
    if (clevel<minclevel || clevel>maxclevel){
      diffCount[i]=LIMIT;
      continue;
    }
    memlevel=(i%9)+1;
    rec_strm[i].zalloc=Z_NULL; rec_strm[i].zfree=Z_NULL; rec_strm[i].opaque=Z_NULL;
    rec_strm[i].next_in=Z_NULL; rec_strm[i].avail_in=0;
    int ret=deflateInit2(&rec_strm[i], clevel, Z_DEFLATED, window-MAX_WBITS, memlevel, Z_DEFAULT_STRATEGY);
    diffCount[i]=(  ret==Z_OK)?0:LIMIT;
    recpos[i]=BLOCK*2;
    diffPos[i*LIMIT]=-1;
    diffByte[i*LIMIT]=0;
  }
  for (int i=0; i<len; i+=BLOCK) {
    unsigned int blsize=min(len-i,BLOCK);
    for (int j=0; j<ZLIB_NUM_COMBINATIONS; j++) {
        if (diffCount[j]==LIMIT) continue;
        memmove(&zrec[0], &zrec[BLOCK], BLOCK);
        if (recpos[j]>=BLOCK) //<-- FIX
           recpos[j]-=BLOCK;
        }
    memmove(&zin[0], &zin[BLOCK], BLOCK);
    in->blockread(&zin[BLOCK],   blsize  ); // Read block from input file
    
    // Decompress/inflate block
    main_strm.next_in=&zin[BLOCK]; main_strm.avail_in=blsize;
    do {
      main_strm.next_out=&zout[0]; main_strm.avail_out=BLOCK;
      main_ret=inflate(&main_strm, Z_FINISH);

      // Recompress/deflate block with all possible parameters
      for (int j=MTF.GetFirst(); j>=0; j=MTF.GetNext()){
        if (diffCount[j]==LIMIT) continue;
        rec_strm[j].next_in=&zout[0];  rec_strm[j].avail_in=BLOCK-main_strm.avail_out;
        rec_strm[j].next_out=&zrec[recpos[j]]; rec_strm[j].avail_out=BLOCK*2-recpos[j];
        int ret=deflate(&rec_strm[j], (int)main_strm.total_in == len ? Z_FINISH : Z_NO_FLUSH);
        if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END && ret!=Z_OK) { diffCount[j]=LIMIT; continue; }

        // Compare
        int end=2*BLOCK-(int)rec_strm[j].avail_out;
        int tail=max(main_ret==Z_STREAM_END ? len-(int)rec_strm[j].total_out : 0,0);
        for (int k=recpos[j]; k<end+tail; k++) {
          if ((k<end && i+k-BLOCK<len && zrec[k]!=zin[k]) || k>=end) {
            if (++diffCount[j]<LIMIT) {
              const int p=j*LIMIT+diffCount[j];
              diffPos[p]=i+k-BLOCK;
              assert(k < sizeof(zin)/sizeof(*zin));
              diffByte[p]=zin[k];
            }
          }
        }
        // Early break on perfect match
        if (main_ret==Z_STREAM_END && diffCount[j]==0){
          index=j;
          found=true;
          break;
        }
        recpos[j]=2*BLOCK-rec_strm[j].avail_out;
      }
    } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR);
    if (main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) break;
  }
  int minCount=(found)?0:LIMIT;
  for (int i=ZLIB_NUM_COMBINATIONS-1; i>=0; i--) {
     clevel=(i/9)+1;
    if (clevel>=minclevel && clevel<=maxclevel)
      deflateEnd(&rec_strm[i]);
    if (!found && diffCount[i]<minCount)
      minCount=diffCount[index=i];
  }
  inflateEnd(&main_strm);
  if (minCount==LIMIT) return false;
  MTF.MoveToFront(index);
  // Step 3 - write parameters, differences and precompressed (inflated) data
  out->putc(diffCount[index]);
  out->putc(window);
  out->putc(index);
  for (int i=0; i<=diffCount[index]; i++) {
    const int v=i==diffCount[index] ? len-diffPos[index*LIMIT+i]
                                    : diffPos[index*LIMIT+i+1]-diffPos[index*LIMIT+i]-1;
    out->put32(v);
  }
  for (int i=0; i<diffCount[index]; i++) out->putc(diffByte[index*LIMIT+i+1]);
  
   in->setpos( pos);
  main_strm.zalloc=Z_NULL; main_strm.zfree=Z_NULL; main_strm.opaque=Z_NULL;
  main_strm.next_in=Z_NULL; main_strm.avail_in=0;
  if (zlib_inflateInit(&main_strm,zh)!=Z_OK) return false;
  for (int i=0; i<len; i+=BLOCK) {
    unsigned int blsize=min(len-i,BLOCK);
    in->blockread(&zin[0],  blsize  );
    main_strm.next_in=&zin[0]; main_strm.avail_in=blsize;
    do {
      main_strm.next_out=&zout[0]; main_strm.avail_out=BLOCK;
      main_ret=inflate(&main_strm, Z_FINISH);
      out->blockwrite(&zout[0],   BLOCK-main_strm.avail_out  );
    } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR);
    if (main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) break;
  }
  inflateEnd(&main_strm);
  return main_ret==Z_STREAM_END;
}

int decode_zlib(File* in, int size, File*out, FMode mode, U64 &diffFound) {
  const int BLOCK=1<<16, LIMIT=128;
  U8 zin[BLOCK],zout[BLOCK];
  int diffCount=min(in->getc(),LIMIT-1);
  int window=in->getc()-MAX_WBITS;
  int index=in->getc();
  int memlevel=(index%9)+1;
  int clevel=(index/9)+1;  
  int len=0;
   
  Array<int>  diffPos(LIMIT);
  diffPos[0]=-1;
  for (int i=0; i<=diffCount; i++) {
    int v=in->get32();
    if (i==diffCount) len=v+diffPos[i]; else diffPos[i+1]=v+diffPos[i]+1;
  }
  Array<U8>  diffByte(LIMIT);
  diffByte[0]=0;
  for (int i=0; i<diffCount; i++) diffByte[i+1]=in->getc();
  size-=7+5*diffCount;
  
  z_stream rec_strm;
  int diffIndex=1,recpos=0;
  rec_strm.zalloc=Z_NULL; rec_strm.zfree=Z_NULL; rec_strm.opaque=Z_NULL;
  rec_strm.next_in=Z_NULL; rec_strm.avail_in=0;
  int ret=deflateInit2(&rec_strm, clevel, Z_DEFLATED, window, memlevel, Z_DEFAULT_STRATEGY);
  if (ret!=Z_OK) return 0;
  for (int i=0; i<size; i+=BLOCK) {
    int blsize=min(size-i,BLOCK);
    in->blockread(&zin[0],  blsize  );
    rec_strm.next_in=&zin[0];  rec_strm.avail_in=blsize;
    do {
      rec_strm.next_out=&zout[0]; rec_strm.avail_out=BLOCK;
      ret=deflate(&rec_strm, i+blsize==size ? Z_FINISH : Z_NO_FLUSH);
      if (ret!=Z_BUF_ERROR && ret!=Z_STREAM_END && ret!=Z_OK) break;
      const int have=min(BLOCK-rec_strm.avail_out,len-recpos);
      while (diffIndex<=diffCount && diffPos[diffIndex]>=recpos && diffPos[diffIndex]<recpos+have) {
        zout[diffPos[diffIndex]-recpos]=diffByte[diffIndex];
        diffIndex++;
      }
      if (mode==FDECOMPRESS) out->blockwrite(&zout[0],   have  );
      else if (mode==FCOMPARE) for (int j=0; j<have; j++) if (zout[j]!=out->getc() && !diffFound) diffFound=recpos+j+1;
      recpos+=have;
      
    } while (rec_strm.avail_out==0);
  }
  while (diffIndex<=diffCount) {
    if (mode==FDECOMPRESS) out->putc(diffByte[diffIndex]);
    else if (mode==FCOMPARE) if (diffByte[diffIndex]!=out->getc() && !diffFound) diffFound=recpos+1;
    diffIndex++;
    recpos++;
  }  
  deflateEnd(&rec_strm);
  return recpos==len ? len : 0;
}

 // Transform DEC Alpha code
void encode_dec(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
 int count=0;
 //U32 op1=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_dec read error");
    // int g=begin-(begin/4)*4;
        for (int i=0; i<bytesRead-3; i+=4) {
        unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
        if ((op>>21)==0x34*32+26) { // bsr r26,offset
        int offset=op&0x1fffff;
        offset+=(i)/4;
        op&=~0x1fffff;
        op|=offset&0x1fffff;
        
        count++;
      }
      op=bswap(op);
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
    //  op1=op;
    }
    
    out->blockwrite(&blk[0],  bytesRead  );
  }
  //printf("DEC replacement Count %d\n",count);
}

U64 decode_dec(Encoder& en, int size1, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
    const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    U8 c;
    int b=0;
    FileTmp dtmp;
    FileTmp dtmp1;
    U32 count=0;
    //decompress file
    for (int i=0; i<size1; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     
     dtmp.setpos(0);
    for (int j=0; j<size1; j+=BLOCK) {
        int size=min(int(size1-j), BLOCK);
        int bytesRead=dtmp.blockread(&blk[0],   size  );
        if (bytesRead!=size) quit("encode_dec read error");
        //int g=begin-(begin/4)*4;
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
            op=bswap(op);
       
                if ((op>>21)==0x34*32+26  ) { // bsr r26,offset
                   int offset=op&0x1fffff;
                   offset-=(i)/4;
                   op&=~0x1fffff;
                   op|=offset&0x1fffff;
                   //blk[i]=op;
                  // blk[i+1]=op>>8;
                   //blk[i+2]=op>>16;
                   count++;
                }
                   blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
        }
        dtmp1.blockwrite(&blk[0],   bytesRead  );
    }
    dtmp1.setpos(0);
    dtmp.close();
    for ( int i=0; i<size1; i++) {
        b=dtmp1.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    if(count<16) diffFound=1; //fail if replaced below threshold
    dtmp1.close();
    return size1; 
}

// Transform DEC Alpha code
void encode_arm(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
 int count=0;
 //U32 op1=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_arm read error");
    // int g=begin-(begin/4)*4;
        for (int i=0; i<bytesRead-3; i+=4) {
        unsigned op=blk[i+3]|(blk[i+2]<<8)|(blk[i+1]<<16)|(blk[i]<<24);
        if ((op>>26)==0x25) { //  
        int offset=op&0x3FFFFFF;
        offset+=(i)/4;
        op&=~0x3FFFFFF;
        op|=offset&0x3FFFFFF;
        
        count++;
      }
      //op=bswap(op);
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
    //  op1=op;
    }
    
    out->blockwrite(&blk[0],  bytesRead  );
  }
  //printf("DEC replacement Count %d\n",count);
}

U64 decode_arm(Encoder& en, int size1, File*out, FMode mode, U64 &diffFound, int s1=0, int s2=0) {
    const int BLOCK=0x10000;  // block size
    Array<U8> blk(BLOCK);
    U8 c;
    int b=0;
    FileTmp dtmp;
    FileTmp dtmp1;
    U32 count=0;
    //decompress file
    for (int i=0; i<size1; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     
     dtmp.setpos(0);
    for (int j=0; j<size1; j+=BLOCK) {
        int size=min(int(size1-j), BLOCK);
        int bytesRead=dtmp.blockread(&blk[0],   size  );
        if (bytesRead!=size) quit("encode_arm read error");
        //int g=begin-(begin/4)*4;
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
                if ((op>>26)==0x25  ) { // bsr r26,offset
                   int offset=op&0x3FFFFFF;
                   offset-=(i)/4;
                   op&=~0x3FFFFFF;
                   op|=offset&0x3FFFFFF;
                  
                   count++;
                }
                   blk[i+3]=op;
        blk[i+2]=op>>8;
        blk[i+1]=op>>16;
        blk[i]=op>>24;
        }
        dtmp1.blockwrite(&blk[0],   bytesRead  );
    }
    dtmp1.setpos(0);
    dtmp.close();
    for ( int i=0; i<size1; i++) {
        b=dtmp1.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    if(count<16) diffFound=1; //fail if replaced below threshold
    dtmp1.close();
    return size1; 
}
//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com
#include "wrtpre.cpp"

void encode_txt(File* in, File* out, int len,int wrtn) {
    assert(wrtn<2);
   XWRT_Encoder* wrt;
   wrt=new XWRT_Encoder();
   wrt->defaultSettings(wrtn);
   wrt->WRT_start_encoding(in,out,len,false);
   delete wrt;
}

//called only when encode_txt output was smaller then input
int decode_txt(Encoder& en, int size, File*out, FMode mode, U64 &diffFound) {
    XWRT_Decoder* wrt;
    wrt=new XWRT_Decoder();
    char c;
    int b=0;
    int bb=0;
    FileTmp dtmp;
    //decompress file
    for (int i=0; i<size; i++) {
        c=en.decompress(); 
        dtmp.putc(c);    
    }
     dtmp.setpos(0);
    wrt->defaultSettings(0);
    bb=wrt->WRT_start_decoding(&dtmp);
    for ( int i=0; i<bb; i++) {
        b=wrt->WRT_decode();    
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    dtmp.close();
    delete wrt;
    return bb; 
}

// decode/encode base64 
static const char  table1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
bool isbase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/')|| (c == 10) || (c == 13));
}

int decode_base64(File*in, int size, File*out, FMode mode, U64 &diffFound){
    U8 inn[3];
    int i, len=0, blocksout = 0;
    int fle=0;
    int linesize=0; 
    int outlen=0;
    int tlf=0,g=0;
    linesize=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> ptr((outlen>>2)*4+10);
    tlf=(tlf&192);
    if (tlf==128)       tlf=10;        // LF: 10
    else if (tlf==64)   tlf=13;        // LF: 13
    else                tlf=0;
 
    while(fle<outlen){
        len=0;
        for(i=0;i<3;i++){
            int c=in->getc();
            if(c!=EOF) {
                inn[i]=c;
                len++;
            }
            else {
                inn[i] = 0,g=1;
            }
        }
        if(len){
            U8 in0,in1,in2;
            in0=inn[0],in1=inn[1],in2=inn[2];
            ptr[fle++]=(table1[in0>>2]);
            ptr[fle++]=(table1[((in0&0x03)<<4)|((in1&0xf0)>>4)]);
            ptr[fle++]=((len>1?table1[((in1&0x0f)<<2)|((in2&0xc0)>>6)]:'='));
            ptr[fle++]=((len>2?table1[in2&0x3f]:'='));
            blocksout++;
        }
        if(blocksout>=(linesize/4) && linesize!=0){ //no lf if linesize==0
            if( blocksout &&  !in->eof() && fle<=outlen) { //no lf if eof
                if (tlf) ptr[fle++]=(tlf);
                else ptr[fle++]=13,ptr[fle++]=10;
            }
            blocksout = 0;
        }
        if (g) break; //if past eof, break
    }
    //Write out or compare
    if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
        }
    else if (mode==FCOMPARE){
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}
   
inline char valueb(char c){
       const char *p = strchr(table1, c);
       if(p) {
          return p-table1;
       } else {
          return 0;
       }
}

void encode_base64(File* in, File* out, int len) {
  int in_len = 0;
  int i = 0;
  int j = 0;
  int b=0;
  int lfp=0;
  int tlf=0;
  char src[4];
  int b64mem=(len>>2)*3+10;
  Array<U8,1> ptr(b64mem);
  int olen=5;

  while (b=in->getc(),in_len++ , ( b != '=') && is_base64(b) && in_len<=len) {
    if (b==13 || b==10) {
       if (lfp==0) lfp=in_len ,tlf=b;
       if (tlf!=b) tlf=0;
       continue;
    }
    src[i++] = b; 
    if (i ==4){
          for (j = 0; j <4; j++) src[j] = valueb(src[j]);
          src[0] = (src[0] << 2) + ((src[1] & 0x30) >> 4);
          src[1] = ((src[1] & 0xf) << 4) + ((src[2] & 0x3c) >> 2);
          src[2] = ((src[2] & 0x3) << 6) + src[3];
    
          ptr[olen++]=src[0];
          ptr[olen++]=src[1];
          ptr[olen++]=src[2];
      i = 0;
    }
  }

  if (i){
    for (j=i;j<4;j++)
      src[j] = 0;

    for (j=0;j<4;j++)
      src[j] = valueb(src[j]);

    src[0] = (src[0] << 2) + ((src[1] & 0x30) >> 4);
    src[1] = ((src[1] & 0xf) << 4) + ((src[2] & 0x3c) >> 2);
    src[2] = ((src[2] & 0x3) << 6) + src[3];

    for (j=0;(j<i-1);j++) {
        ptr[olen++]=src[j];
    }
  }
  ptr[0]=lfp&255; //nl lenght
  ptr[1]=len&255;
  ptr[2]=len>>8&255;
  ptr[3]=len>>16&255;
  if (tlf!=0) {
    if (tlf==10) ptr[4]=128;
    else ptr[4]=64;
  }
  else
      ptr[4]=len>>24&63; //1100 0000
  out->blockwrite(&ptr[0],   olen  );
}

//base85
int powers[5] = {85*85*85*85, 85*85*85, 85*85, 85, 1};

int decode_ascii85(File*in, int size, File*out, FMode mode, U64 &diffFound){
    int i;
    int fle=0;
    int nlsize=0; 
    int outlen=0;
    int tlf=0;
    nlsize=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> ptr((outlen>>2)*5+10);
    tlf=(tlf&192);
    if (tlf==128)      tlf=10;        // LF: 10
    else if (tlf==64)  tlf=13;        // LF: 13
    else               tlf=0;
    int c, count = 0, lenlf = 0;
    uint32_t tuple = 0;

    while(fle<outlen){ 
        c = in->getc();
        if (c != EOF) {
            tuple |= ((U32)c) << ((3 - count++) * 8);
            if (count < 4) continue;
        }
        else if (count == 0) break;
        int i, lim;
        char out[5];
        if (tuple == 0 && count == 4) { // for 0x00000000
            if (nlsize && lenlf >= nlsize) {
                if (tlf) ptr[fle++]=(tlf);
                else ptr[fle++]=13,ptr[fle++]=10;
                lenlf = 0;
            }
            ptr[fle++]='z';
        }
        /*    else if (tuple == 0x20202020 && count == 4 ) {
            if (nlsize && lenlf >= nlsize) {
                if (tlf) fptr[fle++]=(tlf);
                else fptr[fle++]=13,fptr[fle++]=10;
                lenlf = 0;
            }
            fptr[fle++]='y',lenlf++;
        }*/
        else {
            for (i = 0; i < 5; i++) {
                out[i] = tuple % 85 + '!';
                tuple /= 85;
            }
            lim = 4 - count;
            for (i = 4; i >= lim; i--) {
                if (nlsize && lenlf >= nlsize && ((outlen-fle)>=5)) {//    skip nl if only 5 bytes left
                    if (tlf) ptr[fle++]=(tlf);
                    else ptr[fle++]=13,ptr[fle++]=10;
                    lenlf = 0;}
                ptr[fle++]=out[i],lenlf++;
            }
        }
        if (c == EOF) break;
        tuple = 0;
        count = 0;
    }
    if (mode==FDECOMPRESS){
        out->blockwrite(&ptr[0],   outlen  );
    }
    else if (mode==FCOMPARE){
        for(i=0;i<outlen;i++){
            U8 b=ptr[i];
            if (b!=out->getc() && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}

void encode_ascii85(File* in, File* out, int len) {
    int lfp=0;
    int tlf=0;
    int b64mem=(len>>2)*5+100;
    Array<U8,1> ptr(b64mem);
    int olen=5;
    int c, count = 0;
    uint32_t tuple = 0;
    for (int f=0;f<len;f++) {
        c = in->getc();
        if (olen+10>b64mem) {count = 0; break;} //!!
        if (c==13 || c==10) {
            if (lfp==0) lfp=f ,tlf=c;
            if (tlf!=c) tlf=0;
            continue;
        }
        if (c == 'z' && count == 0) {
            if (olen+10>b64mem) {count = 0; break;} //!!
            for (int i = 1; i < 5; i++) ptr[olen++]=0;
            continue;
        }
        /*    if (c == 'y' && count == 0) {
            for (int i = 1; i < 5; i++) fptr[olen++]=0x20;
            continue;
        }*/
        if (c == EOF) {  
        if (olen+10>b64mem) {count = 0; break;} //!!      
            if (count > 0) {
                
                tuple += powers[count-1];
                for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            }
            break;
        }
        tuple += (c - '!') * powers[count++];
        if (count == 5) {
           if (olen>b64mem+10) {count = 0; break;} //!!
            for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
            tuple = 0;
            count = 0;
        }
    }
    if (count > 0) {
        
        tuple += powers[count-1];
        for (int i = 1; i < count; i++) ptr[olen++]=tuple >> ((4 - i) * 8);
    }
    ptr[0]=lfp&255; //nl lenght
    ptr[1]=len&255;
    ptr[2]=len>>8&255;
    ptr[3]=len>>16&255;
    if (tlf!=0) {
        if (tlf==10) ptr[4]=128;
        else ptr[4]=64;
    }
    else
    ptr[4]=len>>24&63; //1100 0000
    out->blockwrite(&ptr[0],   olen  );
}

//SZDD
int decode_szdd(File*in, int size, int info, File*out, FMode mode, U64 &diffFound){
    LZSS* lz77;
    int r=0;
    //Write out or compare
    if (mode==FDECOMPRESS){
            lz77=new LZSS(in,out,size,(info>>25)*2);
             r=lz77->compress();
            delete lz77;
        }
    else if (mode==FCOMPARE){
        FileTmp out1;
        lz77=new LZSS(in,&out1,size,(info>>25)*2);
        r=lz77->compress();
        delete lz77;
        out1.setpos(0);
        for(int i=0;i<r;i++){
            U8 b=out1.getc();
            if (b!=out->getc() && !diffFound) diffFound= out->curpos();
        }
        out1.close();
    }
    return r;
}

void encode_szdd(File* in, File* out, int len) {
    LZSS* lz77;
    lz77=new LZSS(in,out,len&0x1ffffff,(len>>25)*2);
    lz77->decompress();
    delete lz77;
}

//mdf 
int decode_mdf(File*in, int size,  File*out, FMode mode, U64 &diffFound){
    int q=in->getc();   // count of channels
    q=(q<<8)+in->getc();
    q=(q<<8)+in->getc();
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    Array<U8,1> ptr(CHAN*q);
    //Write out or compare
    if (mode==FDECOMPRESS){
        in->blockread(&ptr[0], CHAN*q);
        for (int offset=0; offset<q; offset++) { 
            in->blockread(&blk[0], BLOCK);
            out->blockwrite(&blk[0], BLOCK);
            out->blockwrite(&ptr[offset*CHAN], CHAN);
        }
    }
    else if (mode==FCOMPARE){
        in->blockread(&ptr[0], CHAN*q);
        int offset=0;
        for( int i=3;i<size;){
           in->blockread(&blk[0], BLOCK);
            for(int j=0;j<BLOCK;j++,i++){
                U8 b=blk[j];
                if (b!=out->getc() && !diffFound) diffFound= out->curpos();
            } 
            for(int j=0;j<CHAN;j++,i++){
                U8 b=ptr[offset*CHAN+j];
                if (b!=out->getc() && !diffFound) diffFound= out->curpos();
            }
            offset++;
        }
    }
    return size;
}

void encode_mdf(File* in, File* out, int len) {
    const int BLOCK=2352;
    const int CHAN=96;
    U8 blk[BLOCK];
    U8 blk1[CHAN];
    int ql=len/(BLOCK+CHAN);
    out->putc(ql>>16); 
    out->putc(ql>>8);
    out->putc(ql);
    U64 beginin= in->curpos();
    //channel out
    for (int offset=0; offset<ql; offset++) { 
        in->setpos(in->curpos()+  BLOCK);
        in->blockread(&blk1[0],   CHAN);
        out->blockwrite(&blk1[0], CHAN);
    }
    in->setpos( beginin);
    for (int offset=0; offset<ql; offset++) { 
        in->blockread(&blk[0],   BLOCK);
        in->setpos(in->curpos()+ CHAN) ;
        out->blockwrite(&blk[0], BLOCK);
  }
}

int encode_gif(File* in, File* out, int len) {
  int codesize=in->getc(),hdrsize=6,clearpos=0,bsize=0;
  U64 diffpos=0,beginin= in->curpos(),beginout= out->curpos();
  Array<U8,1> output(4096);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(bsize);
  out->putc(clearpos>>8);
  out->putc(clearpos&255);
  out->putc(codesize);
  for (int phase=0; phase<2; phase++) {
    in->setpos( beginin);
    int bits=codesize+1,shift=0,buf=0;
    int blocksize=0,maxcode=(1<<codesize)+1,last=-1;//,dict[4096];
    Array<int> dict(4096);
    bool end=false;
    while ((blocksize=in->getc())>0 &&  in->curpos()-beginin<len && !end) {
      for (int i=0; i<blocksize; i++) {
        buf|=in->getc()<<shift;
        shift+=8;
        while (shift>=bits && !end) {
          int code=buf&((1<<bits)-1);
          buf>>=bits;
          shift-=bits;
          if (!bsize && code!=(1<<codesize)) {
            hdrsize+=4; out->put32(0);
          }
          if (!bsize) bsize=blocksize;
          if (code==(1<<codesize)) {
            if (maxcode>(1<<codesize)+1) {
              if (clearpos && clearpos!=69631-maxcode) return 0;
              clearpos=69631-maxcode;
            }
            bits=codesize+1, maxcode=(1<<codesize)+1, last=-1;
          }
          else if (code==(1<<codesize)+1) end=true;
          else if (code>maxcode+1) return 0;
          else {
            int j=(code<=maxcode?code:last),size=1;
            while (j>=(1<<codesize)) {
              output[4096-(size++)]=dict[j]&255;
              j=dict[j]>>8;
            }
            output[4096-size]=j;
            if (phase==1) out->blockwrite(&output[4096-size],  size  ); else diffpos+=size;
            if (code==maxcode+1) { if (phase==1) out->putc(j); else diffpos++; }
            if (last!=-1) {
              if (++maxcode>=8191) return 0;
              if (maxcode<=4095)
              {
                dict[maxcode]=(last<<8)+j;
                if (phase==0) {
                  bool diff=false;
                  for (int m=(1<<codesize)+2;m<min(maxcode,4095);m++) if (dict[maxcode]==dict[m]) { diff=true; break; }
                  if (diff) {
                    hdrsize+=4;
                    j=diffpos-size-(code==maxcode);
                    out->put32(j);
                    diffpos=size+(code==maxcode);
                  }
                }
              }
              if (maxcode>=((1<<bits)-1) && bits<12) bits++;
            }
            last=code;
          }
        }
      }
    }
  }
  diffpos= out->curpos();
  out->setpos(beginout);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(255-bsize);
  out->putc((clearpos>>8)&255);
  out->putc(clearpos&255);
  out->setpos(diffpos);
  return in->curpos()-beginin==len-1;
}

#define gif_write_block(count) { output[0]=(count);\
if (mode==FDECOMPRESS) out->blockwrite(&output[0],  (count)+1  );\
else if (mode==FCOMPARE) for (int j=0; j<(count)+1; j++) if (output[j]!=out->getc() && !diffFound) diffFound=outsize+j+1;\
outsize+=(count)+1; blocksize=0; }

#define gif_write_code(c) { buf+=(c)<<shift; shift+=bits;\
while (shift>=8) { output[++blocksize]=buf&255; buf>>=8;shift-=8;\
if (blocksize==bsize) gif_write_block(bsize); }}

int decode_gif(File* in, int size, File*out, FMode mode, U64 &diffFound) {
  int diffcount=in->getc(), curdiff=0;
    Array<int> diffpos(4096);//, diffpos[4096];
  diffcount=((diffcount<<8)+in->getc()-6)/4;
  int bsize=255-in->getc();
  int clearpos=in->getc(); clearpos=(clearpos<<8)+in->getc();
  clearpos=(69631-clearpos)&0xffff;
  int codesize=in->getc(),bits=codesize+1,shift=0,buf=0,blocksize=0;
  if (diffcount>4096 || clearpos<=(1<<codesize)+2) return 1;
  int maxcode=(1<<codesize)+1, input;
    Array<int> dict(4096);
  for (int i=0; i<diffcount; i++) {
    diffpos[i]=in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    diffpos[i]=(diffpos[i]<<8)+in->getc();
    if (i>0) diffpos[i]+=diffpos[i-1];
  }
  Array<U8,1> output(256);
  size-=6+diffcount*4;
  int last=in->getc(),total=size+1,outsize=1;
  if (mode==FDECOMPRESS) out->putc(codesize);
  else if (mode==FCOMPARE) if (codesize!=out->getc() && !diffFound) diffFound=1;
  if (diffcount==0 || diffpos[0]!=0) gif_write_code(1<<codesize) else curdiff++;
  while (size!=0 && (input=in->getc())!=EOF) {
    size--;
    int code=-1, key=(last<<8)+input;
    for (int i=(1<<codesize)+2; i<=min(maxcode,4095); i++) if (dict[i]==key) code=i;
    if (curdiff<diffcount && total-size>diffpos[curdiff]) curdiff++,code=-1;
    if (code==-1) {
      gif_write_code(last);
      if (maxcode==clearpos) { gif_write_code(1<<codesize); bits=codesize+1, maxcode=(1<<codesize)+1; }
      else
      {
        ++maxcode;
        if (maxcode<=4095) dict[maxcode]=key;
        if (maxcode>=(1<<bits) && bits<12) bits++;
      }
      code=input;
    }
    last=code;
  }
  gif_write_code(last);
  gif_write_code((1<<codesize)+1);
  if (shift>0) {
    output[++blocksize]=buf&255;
    if (blocksize==bsize) gif_write_block(bsize);
  }
  if (blocksize>0) gif_write_block(blocksize);
  if (mode==FDECOMPRESS) out->putc(0);
  else if (mode==FCOMPARE) if (0!=out->getc() && !diffFound) diffFound=outsize+1;
  return outsize+1;
}

int encodeRLE(U8 *dst, U8 *ptr, int src_end){
    int i=0;
    int ind=0;
    for(ind=0;ind<src_end; ){
        if (ptr[ind+0]!=ptr[ind+1] || ptr[ind+1]!=ptr[ind+2]) {
            // Guess how many non repeating bytes we have
            int j=0;
            for( j=ind+1;j<(src_end);j++)
            if ((ptr[j+0]==ptr[j+1] && ptr[j+2]==ptr[j+0]) || ((j-ind)>=127)) break;
            int pixels=j-ind;
            if (j+1==src_end && pixels<8)pixels++;
            dst[i++]=0x80 |pixels;
            for(int cnt=0;cnt<pixels;cnt++) { 
                dst[i++]=ptr[ind+cnt];               
            }
            ind=ind+pixels;
        }
        else {
            // Get the number of repeating bytes
            int j=0;
            for(  j=ind+1;j<(src_end);j++)
            if (ptr[j+0]!=ptr[j+1]) break;
            int pixels=j-ind+1;          
            if (j==src_end && pixels<4){
                pixels--;              
                dst[i]=U8(0x80 |pixels);
                i++ ;
                for(int cnt=0;cnt<pixels;cnt++) { 
                    dst[i]=ptr[ind+cnt]; 
                    i++;
                }
                ind=ind+pixels;
            }
            else{ 
                j=pixels;  
                while (pixels>127) {
                    dst[i++]=127;                
                    dst[i++]=ptr[ind];                       
                    pixels=pixels-127;
                }
                if (pixels>0) { 
                    if (j==src_end) pixels--;
                    dst[i++]=pixels;         
                    dst[i++]=ptr[ind];
                }
                ind=ind+j;
            }
        }
    }
    return i;
}
//mrb
void encode_mrb(File* in, File* out, int len, int width, int height) {
    U64 savepos= in->curpos();
    int totalSize=(width)*height;
    Array<U8,1> ptrin(totalSize+4);
    Array<U8,1> ptr(len+4);
    U32 count=0;
    U8 value=0;
    // decode RLE
    for(int i=0;i<totalSize; ++i){
        if((count&0x7F)==0)    {
            count=in->getc();
            value=in->getc();
        }
        else if(count&0x80)    {
            value=in->getc();
        }
        count--;
        ptrin[i] =value; 
    }
    // encode RLE
    encodeRLE(&ptr[0],&ptrin[0],totalSize);
    // compare to original and output diff data
    int diffcount=0, diffpos[4096];
    in->setpos(savepos);
    for(int i=0;i<len;i++){
        U8 b=ptr[i],c=in->getc();
        if (b!=c ) {
            if (diffcount<4096)diffpos[diffcount++]=c+(i<<8);
        }
    }
    out->putc((diffcount>>8)&255); out->putc(diffcount&255);
    if (diffcount>0)
    out->blockwrite((U8*)&diffpos[0], diffcount*4);
    out->put32(len);
    out->blockwrite(&ptrin[0], totalSize);
}

int decode_mrb(File* in, int size, int width, File*out1, FMode mode, uint64_t &diffFound) {
    int diffcount=0, diffpos[4096];
    diffcount=(in->getc()<<8)+in->getc();
    if (diffcount>0) in->blockread((U8*)&diffpos[0], diffcount*4);
    int len=in->get32();
    Array<U8,1> fptr(size+4);
    Array<U8,1> ptr(size+4);
    in->blockread(&fptr[0], size );
    encodeRLE(&ptr[0],&fptr[0],size-2-4-diffcount*4); //size - header
    //Write out or compare
    if (mode==FDECOMPRESS){
        int diffo=diffpos[0]>>8;
        int diffp=0;
        for(int i=0;i<len;i++){
            if (i==diffo && diffcount) {             
                ptr[i]=diffpos[diffp]&255,diffp++,diffo=diffpos[diffp]>>8 ;
            }
        }    
        out1->blockwrite(&ptr[0], len);
    }
    else if (mode==FCOMPARE){
        int diffo=diffpos[0]>>8;
        int diffp=0;
        for(int i=0;i<len;i++){
            if (i==diffo && diffcount) {
                ptr[i]=diffpos[diffp]&255,diffp++,diffo=diffpos[diffp]>>8 ;
            }
            U8 b=ptr[i];
            if (b!=out1->getc() && !diffFound) diffFound= out1->curpos();
        }
    }
    assert(len<size);
    return len;
}

//EOL


//  char*dxx[]={"a","an","the","this","that","these","and","or","if","then","else","not","as","because","but","like","be","been","being","am","is","are","was","were","do","don","did","does","will","ll","can","could","would","should","must","may","might","have","has","had","here","there","where","when","what","who","how","which","he","she","me","we","you","it","its","they","their","him","them","em","his","her","my","your","of","for","with","without","in","on","to","from","into","at","by","over","out","about","before","after","above","between","up","down","upon","through","never","now","no","yes","some","any","one","two","each","such","much","even","again","more","too","rather","other","another","just","only","so","very","all","many","most","than","make","made","say","said","think","thought","use","used","using","know","come","go","see","look","begin","end","new","little","well","good","different","same","long","high","however","troy","bathsheba","boldwood","oak","gabriel","man","woman","first","quite","get","set","time","date","text","speech","message","subject","figure","program","comp","computer","system","systems","word","words","file","data","input","output","bit","bits","gmt","uucp","mcvax","alberta","mnetor","uunet","formant","rnews","university","newsgroups","information","organization","field","sound","code","table","example","zone","pitch","lines","path","way","int","point","frequency","signal","filter","model","nothing","case","sup","sub","pp","id","eq","re","lo","hi","en","uk","pm","sp","cs","ds","ac","dec","ul","rn","lb","le","nr","nv","ha","hn","hp","hz"
//};
//U32 dxh[224];


enum EEOLType {UNDEFINED, CRLF, LF};

#define MAX_FREQ_ORDER1 255 //
#define ORDER1_STEP    4

class RangeCoder{
    U32 code, range, FFNum, Cache;
    U64 low;
    int mZero[MAX_FREQ_ORDER1];
    int mOne[MAX_FREQ_ORDER1];
    File*outeol; 
public:
    inline void ShiftLow(){                                             
        if ((low^0xFF000000)>0xFFFFFF){            
            outeol->putc( Cache + (low>>32));       
            int c = 0xFF+(low>>32);                       
            while( FFNum ) outeol->putc(c), FFNum--; 
            Cache = U32(low)>>24;                        
        } else FFNum++;                               
        low = U32(low)<<8;                           
    }
    
    void StartEncode(File*out ){
        low=FFNum=Cache=0;  
        range=0xffffffff; 
        outeol=out; 
    }
    
    void StartDecode(File*out){ 
        outeol=out; 
        code=0; 
        range=0xffffffff;
        for (int i=0; i<5; i++) code=(code<<8) | outeol->getc();
    }
    
    void FinishEncode(){ 
        for (int i=0; i<5; i++) ShiftLow();
    }
    
    void Encode(U32 cumFreq, U32 freq, U32 totFreq){
        low += cumFreq * (range/= totFreq);
        range*= freq;
        while( range<(1<<24) ) { ShiftLow(); range<<=8; }
    }
    
    inline U32 GetFreq (U32 totFreq) {
        return code / (range/= totFreq);
    }
    void Decode (U32 cumFreq, U32 freq, U32 totFreq){
        code -= cumFreq*range;
        range *= freq;
        while (range<(1<<24)) code=(code<<8)|outeol->getc(), range<<=8;
    }
    
    inline void UpdateOrder1(int prev,int c, int step){
        if (c==0) mZero[prev]+=step;
        else      mOne[prev]+=step;

        if (mZero[prev]+mOne[prev] >= 1<<15){
            mZero[prev]=(mZero[prev]+1)/2;
            mOne[prev]=(mOne[prev]+1)/2;
        }    
    }

    inline void EncodeOrder1(int prev, int c){
        if (c==0)  Encode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else       Encode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
    }

    inline int DecodeOrder1(int prev){
        int c=GetFreq(mZero[prev]+mOne[prev]);

        if (c<mZero[prev]) c=0;
        else c=1;

        if (c==0) Decode(0,mZero[prev],mZero[prev]+mOne[prev]);
        else      Decode(mZero[prev],mOne[prev],mZero[prev]+mOne[prev]);
        return c;
    }

    U32 DecodeOrder(U32 prev){
        U32 result=DecodeOrder1(prev); 
        UpdateOrder1(prev,result,ORDER1_STEP); 
        return result;
    }
    void EncodeOrder(U32 prev, U32 result){
        EncodeOrder1(prev,result); 
        UpdateOrder1(prev,result,ORDER1_STEP); 
    }

    RangeCoder(){
        for (int i=0; i<MAX_FREQ_ORDER1; i++){
            mZero[i]=1;
            mOne[i]=1;
        }
    }
};


#define TOLOWER(c)    ((c>='A' && c<='Z')?(c+32):c)
//#define TOUPPER(c)    ((c>='a' && c<='z')?(c-32):c)
class EOLEncoderCoder{
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int lastEOL,lastChar;
public:
    EOLEncoderCoder (File*out ){
        coder.StartEncode(out);
    }
    inline int ContextEncode(int leftChar,int c,int rightChar,int distance){
        U32 prev,result;

        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z' )
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\'') || rightChar<'a' || rightChar>'z' )
        return c;
        
        if (c==32)
        result=0;
        else
        result=1;
        /*
        if (leftChar==',')
        leftChar='z'+1;
        if (leftChar=='.' || leftChar=='\'')
        leftChar='z'+3;

        leftChar-='a'; // leftChar (0-25+3)
        
        if (rightChar>127)
        rightChar=127;
        else
        if (rightChar==10)
        rightChar=32;
        else
        if (rightChar<32)
        rightChar=128;
        
        rightChar-=32;  // c (0-96)
        
        if (distance>80)
        distance=80;
        
       prev=5*16*(distance/5+1)+5*(leftChar/2+1)+(rightChar/32);*/
        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        coder.EncodeOrder(prev,result); 
        //coder.EncodeOrder(prev,0); 
        return 32;
    }
    void EncodeEOLformat(EEOLType EOLType){
        if(EOLType==CRLF)    coder.Encode(0,1,2);
        else     coder.Encode(1,1,2);
    }

    void EOLencode(File* file,File* fileout,int fileLen){
        //unsigned char s[110];
        //int s_size=0;
        //U32 dhash=0;
        //int d=0,w=0,wt=0;
        int xc=0;
        int last_c,c,next_c;
        last_c=0;
        lastEOL=0;
        EOLType=UNDEFINED;
        lastEOL=0;
        c=file->getc(),fpos++;
        fpos=0;
        while ( fpos<fileLen)    {
            next_c=file->getc(),fpos++;
            if (c==32 || c==10 || (c==13 && next_c==10)){
                if (c==13){
                    if (EOLType==CRLF || EOLType==UNDEFINED){
                        c=next_c;
                        if (fpos<fileLen){
                           next_c=file->getc(),fpos++;
                        }
                        else{
                             next_c=0,fpos++;
                        }
                        lastEOL++;
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c){
                            EOLType=CRLF;
                            EncodeEOLformat(EOLType);
                        }
                        lastEOL=fpos;
                        if (last_c==10)  xc=5;//LF marker
                        else xc=last_c;
                    }
                    else
                    xc=c;
                }
                else{
                    if (c==10 && EOLType==CRLF){ 
                        xc=c;
                    }
                    else{
                        last_c=ContextEncode(TOLOWER(last_c),TOLOWER(c),TOLOWER(next_c),fpos-lastEOL+(next_c<0?1:0));
                        if (EOLType==UNDEFINED && last_c!=c){
                            EOLType=LF;
                            EncodeEOLformat(EOLType);
                        }
                        xc=last_c;
                    }
                    if (c==10) lastEOL=fpos;
                }
            }
            else{
               xc=c;
            }
            last_c=c;
            c=xc;  
          /*  if ((c>='a' && c<='z') || (c>='A' && c<='Z')) {
               if (c>='A' && c<='Z') w=1;    
               if (s_size==0 && w==1) wt=1; //frist uper
               if (s_size>0 && w==1 && wt!=3) wt=2; //upper
               if (s_size>0 && ((wt==2 && w==0 )|| (wt==1 && w==0))) wt=3; //var

               s[s_size++]=c;
               if (c>='A' && c<='Z') c+='a'-'A';
              dhash *= 27;
              dhash += c;
               w=0;
            }
            else {
                 if (s_size>0 && wt!=3) {
                     for (int j=0; j<224;j++){
                             if (dxh[j]==dhash){
                                              if (wt>0)  fputc(wt, fileout);
                                              wt=0,w=0;
                                            // printf("%x %d\n",dhash,j);
                                             if (j<112) {
                                                        d=j+128,
                                                        fputc(d, fileout);
                                                        s_size=0; 
                                                        }
                                             else{
                                                  d=240;
                                                  fputc(d, fileout);
                                                  d=j-112+128   ;
                                                  fputc(d, fileout);
                                                  s_size=0; 
                                             } 
                         }  
                         }  
                 }
                 if (s_size>0) {
                    for(int k=0;k<s_size;k++)
                     fputc(s[k], fileout);
                 }
                 wt=0,w=0;
                 dhash=0;
                 s_size=0; 
                 if (c>127 || c==1 || c==2 || c==3)  fputc(3, fileout);//escape*/
                 fileout->putc(c );
            //}
            
           // last_c=c;
            c=next_c;
        }
        coder.FinishEncode();
    }
};

class EOLDecoderCoder{
    RangeCoder coder;
    EEOLType EOLType;
    int fpos;
    int bufChar,lastEOL,lastChar;
public:
     
    inline int ContextDecode(int leftChar,int rightChar,int distance){
        U32 prev,result;

        if (leftChar<'a' || leftChar>'z' || rightChar<'a' || rightChar>'z'  )
        if ((leftChar!=',' && leftChar!='.' && leftChar!='\'' ) || rightChar<'a' || rightChar>'z' )
        return 32;

        /*if (leftChar==',')
        leftChar='z'+1;
        if (leftChar=='.' || leftChar=='\'')
        leftChar='z'+3;
        leftChar-='a'; // leftChar (0-26)

        if (rightChar>127)
        rightChar=127;
        else
        if (rightChar==10)
        rightChar=32;
        else
        if (rightChar<32)
        rightChar=128;
        rightChar-=32;  // c (0-96)
        
        if (distance>80)
        distance=80;*/
        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
       /// prev=5*16*(distance/5+1)+5*(leftChar/2+1)+(rightChar/32);
        result=coder.DecodeOrder(prev); 
        //coder.DecodeOrder(prev); 
        if (result==0)   return 32;
        else     return 10;
    }

    EEOLType DecodeEOLformat(){
        int c=coder.GetFreq(2);
        if (c<1){
            coder.Decode(0,1,2);        
            return CRLF;
        }
        else{     
            coder.Decode(1,1,2);
            return LF;
        }
    }

    void hook_putc(int c,File* out){
        if (bufChar<0 && c==' '){
            bufChar=c;
            return;
        }
        if (bufChar>=0){            
            bufChar=ContextDecode(TOLOWER(lastChar),TOLOWER(c),fpos-lastEOL);
            if (bufChar==10){
                if (EOLType==UNDEFINED)
                EOLType=DecodeEOLformat();
                if (EOLType==CRLF){
                    lastChar=13;
                    out->putc(lastChar),fpos++;
                }
                lastEOL=fpos;
            }
            out->putc(bufChar),fpos++;
            if (c==' '){
                lastChar=bufChar;
                bufChar=c;
                return;
            }
            bufChar=-1;
        }
        if (c==10)
        lastEOL=fpos;
        lastChar=c;
        if (c==EOF) return;
        out->putc(c),fpos++;
    }

    void EOLdecode(File* in,File* out,int size,File*outeol,File*wd){
        int c=0;
        bufChar=-1;
        lastEOL=-1;
        EOLType=UNDEFINED;
        fpos=0;
        //char*p,b11[256],*t=b11;
        //File* dtmp;
        //int i;
        //int b=0,z,v;
        //int bb=0;
        coder.StartDecode(outeol);
        
        for ( int i=0; i<size; i++) {//while (!feof(in)){
        c=wd->getc();
       /*         if(c==3){
                 c=getc(wd);
                 i++;
                 hook_putc(c,out);
                 }
        else*/ if (c==5)hook_putc(13,out),hook_putc(10,out);
       /* else if(c>127){
                        if (c>239){
                              c=getc(wd);
                            c+=112;
                            i++;
                        }
                        memcpy(t,dxx[c-128],z=strlen(dxx[c-128]));
                        if(v==1)--v,t[0]=toupper(t[0]);
                        if(v==2)
                        for(i=0;i<z;i++)
                        t[i]=toupper(t[i]);
                        for(i=0;i<z;i++)
                        hook_putc(t[i],out);
                        v=0;
                    }
        else if (c==1 || c==2) v=c;*/
        else 
        hook_putc(c,out);
           // c=en.decompress();
        }
    }
};
//Based on XWRT 3.2 (29.10.2007) - XML compressor by P.Skibinski, inikep@gmail.com

int encode_txtd(File* in, File* out, int len,int wrtn) {
    U64 eolz=0;
    U64 wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
    
    EOLEncoderCoder* eolc;
    eolc=new EOLEncoderCoder(&wrtfi);
    eolc->EOLencode(in,&tmpout,len); 
    
    eolz= wrtfi.curpos();
    out->put32(eolz);
    
    wrtz= tmpout.curpos();
    out->put32(wrtz);
    wrtfi.setpos(0);
    for (U64 offset=0; offset<eolz; offset++) { 
        out->putc(wrtfi.getc()); 
   }
    wrtz= tmpout.curpos();
    tmpout.setpos(0);
    for (U64 offset=0; offset<wrtz; offset++) { 
        out->putc(tmpout.getc()); 
    }
   delete eolc;
    wrtfi.close();
    tmpout.close();
    return eolz<35;
}

int decode_txtd(File* in, int size, File*out, FMode mode, U64 &diffFound) {
    int b=0;
    U64 bb=0;
    U64 eolz=0,wrtz=0;
    FileTmp wrtfi;
    FileTmp tmpout;
     
    eolz=in->get32();
    wrtz=in->get32();
    
    for (U64 offset=0; offset<eolz; offset++) wrtfi.putc(in->getc()); 
    wrtfi.setpos(0);
    EOLDecoderCoder* eold;
    eold=new EOLDecoderCoder(); 
    eold->EOLdecode(in,&tmpout,wrtz,&wrtfi,in);

    bb= tmpout.curpos();
     tmpout.setpos(0);
    for ( U64 i=0; i<bb; i++) {
        b=tmpout.getc();
        if (mode==FDECOMPRESS) {
            out->putc(b);
        }
        else if (mode==FCOMPARE) {
            if (b!=out->getc() && !diffFound) diffFound=i;
        }
    }
    delete eold;
    tmpout.close();
    wrtfi.close();
    return bb; 
}

//////////////////// Compress, Decompress ////////////////////////////

//for block statistics, levels 0-5
U64 typenamess[datatypecount][5]={0}; //total type size for levels 0-5
U32 typenamesc[datatypecount][5]={0}; //total type count for levels 0-5
int itcount=0;               //level count

int getstreamid(Filetype type){
    if (type<TYPELAST)return typet[type][STREAM];
    return -1;
}

bool isstreamtype(Filetype type,int streamid){
    assert(streamid<streamc);
    assert(type<TYPELAST);
    if (type<TYPELAST && typet[type][STREAM]==streamid) return true;
    return false;
}

void direct_encode_blockstream(Filetype type, File*in, U64 len, Encoder &en, U64 s1, U64 s2, int info=0) {
  assert(s1<(s1+len));
  segment[segment.pos++]=type&0xff;
  segment.put8(len);
  //if (info!=-1) {
    segment.put4(info);
  //}
  int srid=getstreamid(type);
  for (U64 j=s1; j<s1+len; ++j) filestreams[srid]->putc(in->getc());
}

void DetectRecursive(File*in, U64 n, Encoder &en, char *blstr, int it, U64 s1, U64 s2);

void transform_encode_block(Filetype type, File*in, U64 len, Encoder &en, int info, int info2, char *blstr, int it, U64 s1, U64 s2, U64 begin) {
    if (type==EXE || type==DECA || type==ARM || type==CD|| type==MDF || type==IMAGE24 || type==IMAGE32  ||type==MRBR ||type==EOLTEXT||
     (type==TEXT || type==TXTUTF8|| type==TEXT0 || type==TEXT0PDF) || type==BASE64 || type==BASE85 ||type==SZDD|| type==ZLIB|| type==GIF) {
        U64 diffFound=0;
        FileTmp* tmp;
        tmp=new FileTmp;
        if (type==IMAGE24) encode_bmp(in, tmp, int(len), info);
        else if (type==IMAGE32) encode_im32(in, tmp, int(len), info);
        else if (type==MRBR) encode_mrb(in, tmp, int(len), info,info2);
        else if (type==EXE) encode_exe(in, tmp, int(len), int(begin));
        else if (type==DECA) encode_dec(in, tmp, int(len), int(begin));
        else if (type==ARM) encode_arm(in, tmp, int(len), int(begin));
        //else if (type==TEXT0PDF) encode_pdf(in, tmp, int(len), int(begin));
        else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0) ) {
            if ( type!=TXTUTF8 ){
            encode_txt(in, tmp, int(len),1);
            U64 txt0Size= tmp->curpos();
            //reset to text mode
             in->setpos(begin);
            tmp->close();
            tmp=new FileTmp;
            encode_txt(in, tmp, int(len),0);
            U64 txtSize= tmp->curpos();
            tmp->close();
            in->setpos( begin);
            tmp=new FileTmp;
            if (txt0Size<txtSize && (((txt0Size*100)/txtSize)<95)) {
                in->setpos( begin);
                encode_txt(in, tmp, int(len),1);
                type=TEXT0,info=1;
            }else{
                encode_txt(in, tmp, int(len),0);
                type=TEXT,info=0;
            }
            }
            else encode_txt(in, tmp, int(len),info&1); 
        }
        else if (type==EOLTEXT ) diffFound=encode_txtd(in, tmp, int(len),info&1);
        else if (type==BASE64) encode_base64(in, tmp, int(len));
        else if (type==BASE85) encode_ascii85(in, tmp, int(len));
        else if (type==SZDD) encode_szdd(in, tmp, info);
        else if (type==ZLIB) diffFound=encode_zlib(in, tmp, int(len))?0:1;
        else if (type==CD) encode_cd(in, tmp, int(len), info);
        else if (type==MDF) encode_mdf(in, tmp, int(len));
        else if (type==GIF) diffFound=encode_gif(in, tmp, int(len))?0:1;
        if (type==EOLTEXT && diffFound) {
            // if EOL size is below 25 then drop EOL transform and try TEXT type
            diffFound=0, in->setpos(begin),type=TEXT,tmp->close(),tmp=new FileTmp(),encode_txt(in, tmp, int(len),info&1); 
        }
        const U64 tmpsize= tmp->curpos();
        
        int tfail=0;
        tmp->setpos(0);
        en.setFile(tmp);
        
        if (type==ZLIB || type==GIF || type==MRBR|| type==BASE85 ||type==BASE64 || type==DECA || type==ARM|| (type==TEXT || type==TXTUTF8 ||type==TEXT0||type==TEXT0PDF)||type==EOLTEXT ){
        int ts=0;
         in->setpos(begin);
        if (type==BASE64 ) decode_base64(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==BASE85 ) decode_ascii85(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==ZLIB && !diffFound) decode_zlib(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==GIF && !diffFound) decode_gif(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==MRBR) decode_mrb(tmp, int(tmpsize), info, in, FCOMPARE, diffFound);
        else if (type==DECA) decode_dec(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==ARM) decode_arm(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0) ) decode_txt(en, int(tmpsize), in, FCOMPARE, diffFound);
        //else if (type==TEXT0PDF)  decode_pdf(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==EOLTEXT ) ts=decode_txtd(tmp, int(tmpsize), in, FCOMPARE, diffFound)!=len?1:0;  
        if (type==EOLTEXT && (diffFound || ts)) {
            // if fail fall back to text
            diffFound=0,ts=0,info=-1, in->setpos(begin),type=TEXT,tmp->close(),tmp=new FileTmp(),encode_txt(in, tmp, int(len),0); 
        }
        tfail=(diffFound || tmp->getc()!=EOF || ts ); 
        }
        // Test fails, compress without transform
        if (tfail) {
            printf(" Transform fails at %0lu, skipping...\n", diffFound-1);
             in->setpos(begin);
            direct_encode_blockstream(DEFAULT, in, len, en, s1, s2);
            typenamess[type][it]-=len,  typenamesc[type][it]--;       // if type fails set
            typenamess[DEFAULT][it]+=len,  typenamesc[DEFAULT][it]++; // default info
        } else {
            tmp->setpos(0);
            if (type==EXE) {
               direct_encode_blockstream(type, tmp, tmpsize, en, s1, s2);
            } else if (type==DECA || type==ARM) {
                direct_encode_blockstream(type, tmp, tmpsize, en, s1, s2);
            } else if (type==IMAGE24 || type==IMAGE32) {
                direct_encode_blockstream(type, tmp, tmpsize, en, s1, s2, info);
            } else if (type==MRBR) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=( tmp->getc()<<8)+(tmp->getc());
                hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                if (it==itcount)    itcount=it+1;
                typenamess[IMAGE8][it+1]+=tmpsize,  typenamesc[IMAGE8][it+1]++;
                direct_encode_blockstream(IMAGE8, tmp, tmpsize-hdrsize, en, s1, s2, info);
            }else if (type==GIF) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=tmp->getc();
                hdrsize=(hdrsize<<8)+tmp->getc();
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                typenamess[info>>24][it+1]+=tmpsize-hdrsize,  typenamesc[IMAGE8][it+1]++;
                direct_encode_blockstream((Filetype)(info>>24), tmp, tmpsize-hdrsize, en, s1, s2,info&0xffffff);
             }else if (type==TEXT0PDF) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                 
                direct_encode_blockstream(TEXT, tmp, tmpsize , en, s1, s2,-1);   
            } else if (type==AUDIO) {
                segment.put1(type);
                segment.put8(len); //original lenght
                segment.put4(info2); 
                direct_encode_blockstream(type, tmp, tmpsize, en, s1, s2, info);
            } else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0)  ) {
                   if ( len>0xA00000){ //if WRT is smaller then original block 
                      if (tmpsize>(len-256) ) {
                         in->setpos( begin);
                         direct_encode_blockstream(NOWRT, in, len, en, s1, s2); }
                      else
                        direct_encode_blockstream(BIGTEXT, tmp, tmpsize, en, s1, s2);}
                   else if (tmpsize< (len*2-len/2)||len) {
                        // encode as text without wrt transoform, 
                        // this will be done when stream is compressed
                        in->setpos( begin);
                        direct_encode_blockstream(type, in, len, en, s1, s2);
                   }
                   else {
                        // wrt size was bigger, encode as NOWRT and put in bigtext stream.
                        in->setpos(begin);
                        direct_encode_blockstream(NOWRT, in, len, en, s1, s2);
                   }
            }else if (type==EOLTEXT) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=tmp->get32();
                hdrsize=hdrsize+8;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                typenamess[TEXT][it+1]+=tmpsize-hdrsize,  typenamesc[TEXT][it+1]++;
                transform_encode_block(TEXT,  tmp, tmpsize-hdrsize, en, -1,-1, blstr, it, s1, s2, hdrsize); 
            } else if (typet[type][RECURSIVE]) {
                segment.put1(type);
                segment.put8(tmpsize);
                if (type==SZDD ||  type==ZLIB) segment.put4(info);else segment.put4(0);
                if (type==ZLIB) {// PDF or PNG image && info
                    Filetype type2 =(Filetype)(info>>24);
                    if (it==itcount)    itcount=it+1;
                    int hdrsize=7+5*tmp->getc();
                    tmp->setpos(0);
                    typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                    direct_encode_blockstream(HDR,  tmp, hdrsize, en,0,0);
                    if (info){
                    
                    typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                    transform_encode_block(type2,  tmp, tmpsize-hdrsize, en, info&0xffffff,-1, blstr, it, s1, s2, hdrsize); }
                    else{
                         DetectRecursive( tmp, tmpsize-hdrsize, en, blstr,it+1, 0, tmpsize-hdrsize);//it+1
                    }
                } else {     
                    DetectRecursive( tmp, tmpsize, en, blstr,it+1, 0, tmpsize);//it+1
                    tmp->close();
                    return;
                }    
            }
        }
        tmp->close();  // deletes
    } else {
        
#define tarpad  //remove for filesize padding \0 and add to default stream as hdr        
        //do tar recursion, no transform
        if (type==TAR){
        //printf(  "\n");
        int tarl=int(len),tarn=0,blnum=0,pad=0;;
        TARheader tarh;
        char b2[32];
        strcpy(b2, blstr);
        if (b2[0]) strcat(b2, "-");
        while (tarl>0){
            tarl=tarl-pad;
            U64 savedpos= in->curpos(); 
             in->setpos(savedpos+pad);
            in->blockread( (U8*)&tarh,  sizeof(tarh)  );
             in->setpos(savedpos);
            if (tarend((char*)&tarh)) {
                tarn=512+pad;
                printf(" %-11s | %-9s |%10.0I64i [%0lu - %0lu]",blstr,typenames[HDR],tarn,savedpos,savedpos+tarn-1);
                typenamess[HDR][it+1]+=tarn,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, in, tarn, en,0,0);
               }
            else if (!tarchecksum((char*)&tarh))  
                quit("tar checksum error\n");
            else{
                int a=getoct(tarh.size,12);
                int b=a-(a/512)*512;
                if (b) tarn=512+(a/512)*512;
                else if (a==0) tarn=512;
                else tarn= a;
                sprintf(blstr,"%s%d",b2,blnum++);
                int tarover=512+pad;
                //if (a && a<=512) tarover=tarover+tarn,a=0,tarn+=512;
                printf(" %-11s | %-9s |%10.0I64i [%0lu - %0lu]\n",blstr,typenames[HDR],tarover,savedpos,savedpos+tarover-1);
                typenamess[HDR][it+1]+=tarover,  typenamesc[HDR][it+1]++; 
                if (it==itcount)    itcount=it+1;
                direct_encode_blockstream(HDR, in, tarover, en,0,0);
                pad=0;
                if (a!=0){
                    #ifdef tarpad
                        DetectRecursive(in, a, en, blstr, 0, 0, a);
                        pad=tarn-a; 
                        tarn=a+512;
                    #else
                        DetectRecursive(in, tarn, en, blstr, 0, 0, a);
                        pad=0;
                        tarn+=512;
                    #endif
               }
             }
             tarl-=tarn;
             }
             printf("\n");
        }else

        {
        const int i1=(typet[type][INFO]/*type==IMAGE1 || type==IMAGE8 || type==IMAGE4  ||type==PNG8|| type==PNG8GRAY|| type==PNG24 || type==PNG32|| type==IMAGE8GRAY || type==AUDIO || type==DBASE ||type==IMGUNK*/)?info:-1;
        direct_encode_blockstream(type, in, len, en, s1, s2, i1);}
    }
    
}

void DetectRecursive(File*in, U64 n, Encoder &en, char *blstr, int it=0, U64 s1=0, U64 s2=0) {
  static const char* audiotypes[6]={"8b mono","8b stereo","16b mono","16b stereo","32b mono","32b stereo"};
  Filetype type=DEFAULT;
  int blnum=0, info,info2;  // image width or audio type
  U64 begin= in->curpos(), end0=begin+n;
  char b2[32];
  strcpy(b2, blstr);
  if (b2[0]) strcat(b2, "-");
  if (it==5) {
    direct_encode_blockstream(DEFAULT, in, n, en, s1, s2);
    return;
  }
  s2+=n;

  // Transform and test in blocks
  while (n>0) {
    Filetype nextType=detect(in, n, type, info,info2,it,s1);
    U64 end= in->curpos();
     in->setpos( begin);
    if (end>end0) {  // if some detection reports longer then actual size file is
      end=begin+1;
      type=DEFAULT;
    }
    U64 len=U64(end-begin);
    if (begin>end) len=0;
    if (len>=2147483646) {  // fix me, len is int, must be U32  or do not allow larger then +int block size
      len=2147483646;
      type=DEFAULT;
    }
    if (len>0) {
    if (it>itcount)    itcount=it;
    //if(info==2 && type==TEXT0) type=TEXT0PDF;
    if((len>>1)<(info) && type==DEFAULT && info<len) type=BINTEXT;
    typenamess[type][it]+=len,  typenamesc[type][it]++; 
      //s2-=len;
      sprintf(blstr,"%s%d",b2,blnum++);
      
      printf(" %-11s | %-9s |%10.0I64i [%0lu - %0lu]",blstr,typenames[type],len,begin,end-1);
      if (type==AUDIO) printf(" (%s)", audiotypes[(info&31)%4+(info>>7)*2]);
      else if (type==IMAGE1 || type==IMAGE4 || type==IMAGE8 || type==IMAGE24 || type==MRBR|| type==IMAGE8GRAY || type==IMAGE32 ||type==GIF) printf(" (width: %d)", info&0xFFFFFF);
      else if (type==CD) printf(" (m%d/f%d)", info==1?1:2, info!=3?1:2);
      else if (type==ZLIB && (info>>24) > 0) printf(" (%s)",typenames[info>>24]);
      printf("\n");
      transform_encode_block(type, in, len, en, info,info2, blstr, it, s1, s2, begin);
      
      s1+=len;
      n-=len;
    }
    
    type=nextType;
    begin=end;
  }
}

// Compress a file. Split filesize bytes into blocks by type.
// For each block, output
// <type> <size> and call encode_X to convert to type X.
// Test transform and compress.
void DetectStreams(const char* filename, U64 filesize) {
  FileTmp tmp;
  Predictors *t;
  t=0;
  Encoder en(COMPRESS, &tmp,*t);
  assert(en.getMode()==COMPRESS);
  assert(filename && filename[0]);
  FileDisk in;
  in.open(filename,true);
  printf("Block segmentation:\n");
  char blstr[32]="";
  DetectRecursive(&in, filesize, en, blstr);
  in.close();
  tmp.close();
}

U64 decompressStreamRecursive(File*out, U64 size, Encoder& en, FMode mode, int it=0, U64 s1=0, U64 s2=0) {
    Filetype type;
    U64 len=0L, i=0L;
    U64 diffFound=0L;
    int info=-1;
    s2+=size;
    while (i<size) {
        type=(Filetype)segment(segment.pos++);
        //int smr=0;
        for (int k=0; k<8; k++) len=len<<8,len+=segment(segment.pos++);
        for (int k=info=0; k<4; ++k) info=(info<<8)+segment(segment.pos++);
        int srid=getstreamid(type);
        if (srid>=0) en.setFile(filestreams[srid]);
        #ifndef NDEBUG 
         printf(" %d  %-9s |%0lu [%0lu]\n",it, typenames[type],len,i );
        #endif
        if (type==IMAGE24 && !(info&PNGFlag))      len=decode_bmp(en, int(len), info, out, mode, diffFound);
        else if (type==IMAGE32 && !(info&PNGFlag)) decode_im32(en, int(len), info, out, mode, diffFound);
        else if (type==EXE)     len=decode_exe(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==DECA)    len=decode_dec(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==ARM)    len=decode_arm(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==BIGTEXT) len=decode_txt(en, int(len), out, mode, diffFound);
        //else if (type==EOLTEXT) len=decode_txtd(en, int(len), out, mode, diffFound);
        else if (type==BASE85 ||type==BASE64 || type==SZDD || type==ZLIB || type==CD || type==MDF  || type==GIF || type==MRBR|| type==EOLTEXT) {
            FileTmp tmp;
            decompressStreamRecursive(&tmp, len, en, FDECOMPRESS, it+1, s1+i, s2-len);
            if (mode!=FDISCARD) {
                tmp.setpos(0);
                if (type==BASE64) len=decode_base64(&tmp, int(len), out, mode, diffFound);
                else if (type==BASE85) len=decode_ascii85(&tmp, int(len), out, mode, diffFound);
                else if (type==SZDD)   len=decode_szdd(&tmp,info,info ,out, mode, diffFound);
                else if (type==ZLIB)   len=decode_zlib(&tmp,int(len),out, mode, diffFound);
                else if (type==CD)     len=decode_cd(&tmp, int(len), out, mode, diffFound);
                else if (type==MDF)    len=decode_mdf(&tmp, int(len), out, mode, diffFound);
                else if (type==GIF)    len=decode_gif(&tmp, int(len), out, mode, diffFound);
                else if (type==MRBR)   len=decode_mrb(&tmp, int(len), info, out, mode, diffFound);
                else if (type==EOLTEXT)len=decode_txtd(&tmp, int(len), out, mode, diffFound);
            }
            tmp.close();
        }
        else {
            for (U64 j=i+s1; j<i+s1+len; ++j) {
                if (!(j&0x1fff)) printStatus(j, s2);
                if (mode==FDECOMPRESS) out->putc(en.decompress());
                else if (mode==FCOMPARE) {
                    int a=out->getc();
                    int b=en.decompress();
                    if (a!=b && !diffFound) {
                        mode=FDISCARD;
                        diffFound=j+1;
                    }
                } else en.decompress();
            }
        }
        i+=len;
    }
    return diffFound;
}

// Decompress a file from datastream
void DecodeStreams(const char* filename, U64 filesize) {
  FMode mode=FDECOMPRESS;
  assert(filename && filename[0]);
  FileTmp  tmp;
  Predictors *t; //dummy
  t=0;
  Encoder en(COMPRESS, &tmp,*t);
  // Test if output file exists.  If so, then compare.
  FileDisk f;
  bool success=f.open(filename,true);
  if (success) mode=FCOMPARE,printf("Comparing");
  else {
    // Create file
    f.create(filename);
    mode=FDECOMPRESS, printf("Extracting");
  }
  printf(" %s %0lu -> \n", filename, filesize);

  // Decompress/Compare
  U64 r=decompressStreamRecursive(&f, filesize, en, mode);
  if (mode==FCOMPARE && !r && f.getc()!=EOF) printf("file is longer\n");
  else if (mode==FCOMPARE && r) printf("differ at %0lu\n",r-1);
  else if (mode==FCOMPARE) printf("identical\n");
  else printf("done   \n");
  f.close();
  tmp.close();
}

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
int putsize(String& archive, String& s, const char* fname, int base) {
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

int expand(String& archive, String& s, const char* fname, int base) {
  int result=0;
  DWORD attr=GetFileAttributes(fname);
  if ((attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
    WIN32_FIND_DATA ffd;
    String fdir(fname);
    fdir+="/*";
    HANDLE h=FindFirstFile(fdir.c_str(), &ffd);
    while (h!=INVALID_HANDLE_VALUE) {
      if (!equals(ffd.cFileName, ".") && !equals(ffd.cFileName, "..")) {
        String d(fname);
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

int expand(String& archive, String& s, const char* fname, int base) {
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
        String d(fname);
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

int expand(String& archive, String& s, const char* fname, int base) {
  return putsize(archive, s, fname, base);
}

#endif
#endif


U64 filestreamsize[streamc];

void compressStream(int streamid,U64 size, File* in, File* out) {
    int i; //stream
    i=streamid;
    Encoder* threadencode;
    Predictors* threadpredict;
    U64 datasegmentsize;
    U64 datasegmentlen;
    int datasegmentpos;
    int datasegmentinfo;
    Filetype datasegmenttype;
    U64 scompsize=0;
                datasegmentsize=size;
                    U64 total=size;
                    datasegmentpos=0;
                    datasegmentinfo=0;
                    datasegmentlen=0;
                    // datastreams
                    switch(i) {
                        default:
                        case 0: { threadpredict=new Predictor(); break;}
                        case 1: { threadpredict=new PredictorJPEG(); break;}
                        case 2: { threadpredict=new PredictorIMG1(); break;}
                        case 3: { threadpredict=new PredictorIMG4(); break;}
                        case 4: { threadpredict=new PredictorIMG8(); break;}
                        case 5: { threadpredict=new PredictorIMG24(); break;}
                        case 6: { threadpredict=new PredictorAUDIO2(); break;}
                        case 7: { threadpredict=new PredictorEXE(); break;}
                        case 8: { threadpredict=new PredictorTXTWRT(); break;}
                        case 9: 
                        case 10: { threadpredict=new PredictorTXTWRT(); break;}
                        case 11: { threadpredict=new PredictorDEC(); break;}
                    }
                    threadencode=new Encoder (COMPRESS, out,*threadpredict); 
                     if ((i>=0 && i<=7) || i==10|| i==11){
                        while (datasegmentsize>0) {
                            while (datasegmentlen==0){
                                datasegmenttype=(Filetype)segment(datasegmentpos++);
                                for (int ii=0; ii<8; ii++) datasegmentlen<<=8,datasegmentlen+=segment(datasegmentpos++);
                                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                                if (!(isstreamtype(datasegmenttype,i) ))datasegmentlen=0;
                                threadencode->predictor.x.filetype=datasegmenttype;
                                threadencode->predictor.x.blpos=0;
                                threadencode->predictor.x.finfo=datasegmentinfo;
                            }
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                //#ifndef MT
                                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                                //#endif
                                threadencode->compress(in->getc());
                                datasegmentsize--;
                            }
                            #ifndef NDEBUG 
                            printf("Stream(%d) block from %0lu to %0lu bytes\n",i,datasegmentlen, out->curpos()-scompsize);
                            scompsize= out->curpos();
                            #endif
                            datasegmentlen=0;
                        }
                        threadencode->flush();
                    }
                    if (i==8 || i==9 ){
                            FileTmp tm;
                            XWRT_Encoder* wrt;
                            wrt=new XWRT_Encoder();
                            wrt->defaultSettings(i==8);
                            wrt->WRT_start_encoding(in,&tm,datasegmentsize,false);
                            delete wrt;
                            datasegmentlen= tm.curpos();
                            filestreamsize[i]=datasegmentlen;
                            printf(" Total %0lu wrt: %0lu\n",datasegmentsize,datasegmentlen); 
                            tm.setpos(0);
                            threadencode->predictor.x.filetype=DICTTXT;
                            threadencode->predictor.x.blpos=0;
                            threadencode->predictor.x.finfo=-1;
                            
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                //#ifndef MT
                                if (!(k&0x1fff)) printStatus(k, datasegmentlen,i);
                                #ifndef NDEBUG 
                                if (!(k&0xffff) && k) {
                                    printf("Stream(%d) block pos %0lu compressed to %0lu bytes\n",i,k, out->curpos()-scompsize);
                                    scompsize= out->curpos();
                                }
                                #endif
                                //#endif
                                threadencode->compress( tm.getc());
                            }
                            
                            tm.close();
                            threadencode->flush();
                            //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,datasegmentlen+0.0,ftello(out)-scompsize+0.0);
                            datasegmentlen=datasegmentsize=0;   
                    }
                    
                    
            delete threadpredict;
            delete threadencode;
           printf("Stream(%d) compressed from %0lu to %0lu bytes\n",i,size, out->curpos());
}

#ifdef MT
//multithreading code from pzpaq.cpp v0.05
#ifdef PTHREAD
pthread_cond_t cv=PTHREAD_COND_INITIALIZER;  // to signal FINISHED
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; // protects cv
typedef pthread_t pthread_tx;
#else
HANDLE mutex;  // protects Job::state
typedef HANDLE pthread_tx;
#endif


File* filesmt[streamc];
typedef enum {READY, RUNNING, FINISHED_ERR, FINISHED, ERR, OK} State;
// Instructions to thread to compress or decompress one block.
struct Job {
  State state;        // job state, protected by mutex
  int id;             
  int streamid;
  U64 datasegmentsize;
  int command;
  File*in;
  File*out;
  pthread_tx tid;      // thread ID (for scheduler)
  Job();
  void print(int i) const;
};

// Initialize
Job::Job(): state(READY),id(0),streamid(-1),datasegmentsize(0),command(-1) {
  // tid is not initialized until state==RUNNING
}

// Print contents
void Job::print(int i=0) const {
  fprintf(stderr,
      "Job %d: state=%d stream=%d\n", i, state,streamid);
}
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

void decompress(const Job& job) {
}        

#define check(f) { \
  int rc=f; \
  if (rc) fprintf(stderr, "Line %d: %s: error %d\n", __LINE__, #f, rc); \
}
// Worker thread
#ifdef PTHREAD
void*
#else
DWORD
#endif
thread(void *arg) {

  // Do the work and receive status in msg
  Job* job=(Job*)arg;
  const char* result=0;  // error message unless OK
  try {
    if (job->command==0) 
      compressStream(job->streamid,job->datasegmentsize,job->in,job->out);
    else if (job->command==1)
      decompress(*job); 
  }
  catch (const char* msg) {
    result=msg;
  }
// Call f and check that the return code is 0

  // Let controlling thread know we're done and the result
#ifdef PTHREAD
  check(pthread_mutex_lock(&mutex));
  job->state=result?FINISHED_ERR:FINISHED;
  check(pthread_cond_signal(&cv));
  check(pthread_mutex_unlock(&mutex));
#else
  WaitForSingleObject(mutex, INFINITE);
  job->state=result?FINISHED_ERR:FINISHED;
  ReleaseMutex(mutex);
#endif
  return 0;
}
#endif

// To compress to file1.paq8pxd: paq8pxd [-n] file1 [file2...]
// To decompress: paq8pxd file1.paq8pxd [output_dir]
int main(int argc, char** argv) {
    bool pause=argc<=2;  // Pause when done?
    try {

        // Get option
        bool doExtract=false;  // -d option
        bool doList=false;  // -l option
        char* aopt;
        aopt=&argv[1][0];
        
#ifdef MT 
        int topt=1;
        if (argc>1 && aopt[0]=='-' && aopt[1]  && strlen(aopt)<=6) {
#else
        if (argc>1 && aopt[0]=='-' && aopt[1]  && strlen(aopt)<=4) {    
#endif
            if (aopt[1]=='d' && !aopt[2])
                doExtract=true;
            else if (aopt[1]=='l' && !aopt[2])
                doList=true;
            else if (aopt[2]>='0' && aopt[2]<='9' && strlen(aopt)==3 && aopt[1]=='s'){
                level=aopt[2]-'0';
            }
            else if (aopt[2]=='1' && aopt[3]>='0' && aopt[3]<='5' && strlen(aopt)==4 && aopt[1]=='s'){
                aopt[1]='-', aopt[0]=' ';
                level=((~atol(aopt))+1);                 
            }
#ifdef MT 
            else if (aopt[2]>='0' && aopt[2]<='9'&& (aopt[4]<='9' && aopt[4]>'0') && strlen(aopt)==5 && 
            (aopt[1]=='s')){
                topt=aopt[4]-'0';
                level=aopt[2]-'0';}
            else if (aopt[2]=='1' && aopt[3]>='0' && aopt[3]<='5' && 
            (aopt[5]<='9' && aopt[5]>'0')&& strlen(aopt)==6 && aopt[1]=='s'){
                topt=aopt[5]-'0';
                aopt[4]=0;
                aopt[1]='-';
                aopt[0]=' ';
                level=((~atol(aopt))+1); 
            }
#endif
            else
                quit("Valid options are -M0 through -M15, -d, -l where M is mode\n");
            --argc;
            ++argv;
            pause=false;
        }



        // Print help message quick 
        if (argc<2) {
            printf(PROGNAME " archiver (C) 2018, Matt Mahoney et al.\n"
            "Free under GPL, http://www.gnu.org/licenses/gpl.txt\n");
#ifdef __GNUC__     
            printf("Compiled %s, compiler gcc version %d.%d.%d\n\n",__DATE__, __GNUC__, __GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#endif
#ifdef __clang_major__
            printf("Compiled %s, compiler clang version %d.%d\n\n",__DATE__, __clang_major__, __clang_minor__);
#endif
            printf(
#ifdef WINDOWS
            "To compress or extract, drop a file or folder on the "
            PROGNAME " icon.\n"
            "The output will be put in the same folder as the input.\n"
            "\n"
            "Or from a command window: "
#endif
            "To compress:\n"
            "  " PROGNAME " -Mlevel file               (compresses to file." PROGNAME ")\n"
            "  " PROGNAME " -Mlevel archive files...   (creates archive." PROGNAME ")\n"
            "  " PROGNAME " file                       (level -%d slow mode, pause when done)\n"
            "M: select mode s                          (s - slow)\n"
            "level: -[s]0 = store,\n"
            "  -[s]1...-[s]3 = faster (uses 35, 48, 59 MB)\n"
            "  -s4...-s8  = smaller       (uses 133, 233, 435, 837, 1643 MB)\n"
            "  -s9...-s15 = experimental  (uses 3.1, 6.0, 7.8, 9.3, 12.0, 17.8, 25.3 GB)\n"
#ifdef MT 
            "  to use multithreading -Mlevel:threads (1-9, compression only)\n"
            "  " PROGNAME " -s4:2 file (use slow mode level 4 threads 2)\n\n"
#endif            
#if defined(WINDOWS) || defined (UNIX)
            "You may also compress directories.\n"
#endif
            "\n"
            "To extract or compare:\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME "      (extract to dir1)\n"
            "  " PROGNAME " -d dir1/archive." PROGNAME " dir2 (extract to dir2)\n"
            "  " PROGNAME " archive." PROGNAME "              (extract, pause when done)\n"
            "\n"
            "To view contents: " PROGNAME " -l archive." PROGNAME "\n"
            "\n",
            DEFAULT_OPTION);
            quit();
        }
    
        File* archive=0;               // compressed file
        int files=0;                   // number of files to compress/decompress
        Array<const char*> fname(1);   // file names (resized to files)
        Array<U64> fsize(1);           // file lengths (resized to files)
        U16 streambit=0;               //bit is set if stream has size, 11-0
        // Compress or decompress?  Get archive name
        Mode mode=COMPRESS;
        String archiveName(argv[1]);
        {
            const int prognamesize=strlen(PROGNAME);
            const int arg1size=strlen(argv[1]);
            if (arg1size>prognamesize+1 && argv[1][arg1size-prognamesize-1]=='.'
                    && equals(PROGNAME, argv[1]+arg1size-prognamesize)) {
                mode=DECOMPRESS;
            }
            else if (doExtract || doList)
            mode=DECOMPRESS;
            else {
                archiveName+=".";
                archiveName+=PROGNAME;
            }
        }

        // Compress: write archive header, get file names and sizes
        String header_string;
        String filenames;
        
        if (mode==COMPRESS) {
            segment.setsize(48); //inital segment buffer size (about 277 blocks)
            // Expand filenames to read later.  Write their base names and sizes
            // to archive.
            int i;
            for (i=1; i<argc; ++i) {
                String name(argv[i]);
                int len=name.size()-1;
                for (int j=0; j<=len; ++j)  // change \ to /
                if (name[j]=='\\') name[j]='/';
                while (len>0 && name[len-1]=='/')  // remove trailing /
                name[--len]=0;
                int base=len-1;
                while (base>=0 && name[base]!='/') --base;  // find last /
                ++base;
                if (base==0 && len>=2 && name[1]==':') base=2;  // chop "C:"
                int expanded=expand(header_string, filenames, name.c_str(), base);
                if (!expanded && (i>1||argc==2))
                printf("%s: not found, skipping...\n", name.c_str());
                files+=expanded;
            }

            // If there is at least one file to compress
            // then create the archive header.
            if (files<1) quit("Nothing to compress\n");
            archive=new FileDisk();
            archive->create(archiveName.c_str());
            archive->append(PROGNAME);
            archive->putc(0);
            archive->putc(level);
            segment.hpos= archive->curpos();
            
            for (int i=0; i<12+4+2; i++) archive->putc(0); //space for segment size in header +streams info
            
            printf("Creating archive %s with %d file(s)...\n",
            archiveName.c_str(), files);
        }

        // Decompress: open archive for reading and store file names and sizes
        if (mode==DECOMPRESS) {
            archive= new FileDisk();
            archive->open(archiveName.c_str(),true);
            // Check for proper format and get option
            String header;
            int len=strlen(PROGNAME)+1, c, i=0;
            header.resize(len+1);
            while (i<len && (c=archive->getc())!=EOF) {
                header[i]=c;
                i++;
            }
            header[i]=0;
            if (strncmp(header.c_str(), PROGNAME "\0", strlen(PROGNAME)+1))
            printf("%s: not a %s file\n", archiveName.c_str(), PROGNAME), quit();
            level=archive->getc();
            level=level&0xf;
            //if (level<0||level>15) level=DEFAULT_OPTION;
            
            // Read segment data from archive end
            U64 currentpos,datapos=0L;
            for (int i=0; i<8; i++) datapos=datapos<<8,datapos+=archive->getc();
            segment.hpos=datapos;
            U32 segpos=archive->get32();  //read segment data size
            segment.pos=archive->get32(); //read segment data size
            streambit=archive->getc()<<8; //get bitinfo of streams present
            streambit+=archive->getc();
            if (segment.hpos==0 || segment.pos==0) quit("Segment data not found.");
            segment.setsize(segment.pos);
            currentpos= archive->curpos();
             archive->setpos( segment.hpos); 
            if (archive->blockread( &segment[0],   segment.pos  )<segment.pos) quit("Segment data corrupted.");
            // Decompress segment data 
            Encoder* segencode;
            Predictors* segpredict;
            FileTmp  tmp;
            tmp.blockwrite(&segment[0],   segment.pos  ); 
            tmp.setpos(0); 
            segpredict=new Predictor();
            segencode=new Encoder (DECOMPRESS, &tmp ,*segpredict); 
            segment.pos=0;
            for (U32 k=0; k<segpos; ++k) {
                 segment.put1( segencode->decompress());
            }
            delete segpredict;
            delete segencode;
            tmp.close();
            //read stream sizes if stream bit is set
            for (int i=0;i<streamc;i++){
                if ((streambit>>(streamc-i))&1){
                   for (int j=0; j<8; j++) filestreamsize[i]<<=8,filestreamsize[i]+=archive->getc();
                }
            }
            archive->setpos(currentpos); 
            segment.pos=0; //reset to offset 0
        }
        Encoder* en;
        Predictors* predictord;
        predictord=new Predictor();
        en=new Encoder(mode, archive,*predictord);
        
        // Compress header
        if (mode==COMPRESS) {
            int len=header_string.size();
            printf("\nFile list (%d bytes)\n", len);
            assert(en->getMode()==COMPRESS);
            U64 start=en->size();
            en->compress(0); // block type 0
            en->compress(len>>24); en->compress(len>>16); en->compress(len>>8); en->compress(len); // block length
            for (int i=0; i<len; i++) en->compress(header_string[i]);
            printf("Compressed from %d to %0lu bytes.\n",len,en->size()-start);
        }

        // Deompress header
        if (mode==DECOMPRESS) {
            if (en->decompress()!=0) printf("%s: header corrupted\n", archiveName.c_str()), quit();
            int len=0;
            len+=en->decompress()<<24;
            len+=en->decompress()<<16;
            len+=en->decompress()<<8;
            len+=en->decompress();
            header_string.resize(len);
            for (int i=0; i<len; i++) {
                header_string[i]=en->decompress();
                if (header_string[i]=='\n') files++;
            }
            if (doList) printf("File list of %s archive:\n%s", archiveName.c_str(), header_string.c_str());
        }
        
        // Fill fname[files], fsize[files] with input filenames and sizes
        fname.resize(files);
        fsize.resize(files);
        char *p=&header_string[0];
        char* q=&filenames[0];
        for (int i=0; i<files; ++i) {
            assert(p);
            fsize[i]=atoll(p);
            assert(fsize[i]>=0);
            while (*p!='\t') ++p; *(p++)='\0';
            fname[i]=mode==COMPRESS?q:p;
            while (*p!='\n') ++p; *(p++)='\0';
            if (mode==COMPRESS) { while (*q!='\n') ++q; *(q++)='\0'; }
        }
        // Compress or decompress files
        assert(fname.size()==files);
        assert(fsize.size()==files);
        U64 total_size=0;  // sum of file sizes
        for (int i=0; i<files; ++i) total_size+=fsize[i];
        if (mode==COMPRESS) {
            en->flush();
            delete en;
            delete predictord;
            for (int i=0; i<streamc; ++i) {
                filestreams[i]=new FileTmp();
            }
            for (int i=0; i<files; ++i) {
                printf("\n%d/%d  Filename: %s (%0lu bytes)\n", i+1, files, fname[i], fsize[i]);
                DetectStreams(fname[i], fsize[i]);
            }
            segment.put1(0xff); //end marker
            printf("\n Segment data size: %d bytes\n",segment.pos);
            
            //Display Level statistics
            U32 ttc;
            U64 tts;
            for (int j=0; j<=itcount; ++j) {
                printf("\n %-2s |%-9s |%-10s |%-10s\n","TN","Type name", "Count","Total size");
                printf("-----------------------------------------\n");
                ttc=0,tts=0;
                for (int i=0; i<datatypecount; ++i)   if (typenamess[i][j]) printf(" %2d |%-9s |%10d |%10.0I64i\n",i,typenames[i], typenamesc[i][j],typenamess[i][j]),ttc+=typenamesc[i][j],tts+=typenamess[i][j];
                printf("-----------------------------------------\n");
                printf("%-13s%1d |%10d |%10.0I64i\n\n","Total level",j, ttc,tts);
            }
            
#ifdef MT
            std::vector<Job> jobs;
#endif
            for (int i=0; i<streamc; ++i) {
                U64 datasegmentsize;
                datasegmentsize= filestreams[i]->curpos();    //get segment data offset
                filestreamsize[i]=datasegmentsize;
                 filestreams[i]->setpos( 0);
                streambit=(streambit+(datasegmentsize>0))<<1; //set stream bit if streamsize >0
                if (datasegmentsize>0){                       //if segment contains data
                    switch(i) {
                        case 0: {
                            printf("default   stream(0).  Total %0lu\n",datasegmentsize); break;}
                        case 1: {
                            printf("jpeg      stream(1).  Total %0lu\n",datasegmentsize); break;}        
                        case 2: {
                            printf("image1    stream(2).  Total %0lu\n",datasegmentsize); break;}
                        case 3: {
                            printf("image4    stream(3).  Total %0lu\n",datasegmentsize); break;}    
                        case 4: {
                            printf("image8    stream(4).  Total %0lu\n",datasegmentsize); break;}
                        case 5: {
                            printf("image24   stream(5).  Total %0lu\n",datasegmentsize); break;}        
                        case 6: {
                            printf("audio     stream(6).  Total %0lu\n",datasegmentsize); break;}
                        case 7: {
                            printf("exe       stream(7).  Total %0lu\n",datasegmentsize); break;}
                        case 8: {
                            printf("text0 wrt stream(8).  Total %0lu\n",datasegmentsize); break;}
                        case 9: 
                        case 10: {
                            printf("%stext wrt stream(%d). Total %0lu\n",i==10?"big":"",i,datasegmentsize); break;}   
                        case 11: {
                            printf("dec       stream(11). Total %0lu\n",datasegmentsize); break;}
                    }
#ifdef MT
                                                              // add streams to job list
                    filesmt[i]=new FileTmp();                 //open tmp file for stream output
                    Job job;
                    job.out=filesmt[i];
                    job.in=filestreams[i];
                    job.streamid=i;
                    job.command=0; //0 compress
                    job.datasegmentsize=datasegmentsize;
                    jobs.push_back(job);
#else
                    compressStream(i,datasegmentsize,filestreams[i],archive);
#endif
                }
            }

#ifdef MT
  // Loop until all jobs return OK or ERR: start a job whenever one
  // is eligible. If none is eligible then wait for one to finish and
  // try again. If none are eligible and none are running then it is
  // an error.
  int thread_count=0;  // number RUNNING, not to exceed topt
  U32 job_count=0;     // number of jobs with state OK or ERR

  // Aquire lock on jobs[i].state.
  // Threads can access only while waiting on a FINISHED signal.
#ifdef PTHREAD
  pthread_attr_t attr; // thread joinable attribute
  check(pthread_attr_init(&attr));
  check(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
  check(pthread_mutex_lock(&mutex));  // locked
#else
  mutex=CreateMutex(NULL, FALSE, NULL);  // not locked
#endif

  while(job_count<jobs.size()) {

    // If there is more than 1 thread then run the biggest jobs first
    // that satisfies the memory bound. If 1 then take the next ready job
    // that satisfies the bound. If no threads are running, then ignore
    // the memory bound.
    int bi=-1;  // find a job to start
    if (thread_count<topt) {
      for (U32 i=0; i<jobs.size(); ++i) {
        if (jobs[i].state==READY  && bi<0 ) {
          bi=i;
          if (topt==1) break;
        }
      }
    }

    // If found then run it
    if (bi>=0) {
      jobs[bi].state=RUNNING;
      ++thread_count;
#ifdef PTHREAD
      check(pthread_create(&jobs[bi].tid, &attr, thread, &jobs[bi]));
#else
      jobs[bi].tid=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread,
          &jobs[bi], 0, NULL);
#endif
    }

    // If no jobs can start then wait for one to finish
    else {
#ifdef PTHREAD
      check(pthread_cond_wait(&cv, &mutex));  // wait on cv

      // Join any finished threads. Usually that is the one
      // that signaled it, but there may be others.
      for (U32 i=0; i<jobs.size(); ++i) {
        if (jobs[i].state==FINISHED || jobs[i].state==FINISHED_ERR) {
          void* status=0;
          check(pthread_join(jobs[i].tid, &status));
          if (jobs[i].state==FINISHED) jobs[i].state=OK;
          if (jobs[i].state==FINISHED_ERR) quit(); //exit program on thread error 
          ++job_count;
          --thread_count;
        }
      }
#else
      // Make a list of running jobs and wait on one to finish
      HANDLE joblist[MAXIMUM_WAIT_OBJECTS];
      int jobptr[MAXIMUM_WAIT_OBJECTS];
      DWORD njobs=0;
      WaitForSingleObject(mutex, INFINITE);
      for (U32 i=0; i<jobs.size() && njobs<MAXIMUM_WAIT_OBJECTS; ++i) {
        if (jobs[i].state==RUNNING || jobs[i].state==FINISHED
            || jobs[i].state==FINISHED_ERR) {
          jobptr[njobs]=i;
          joblist[njobs++]=jobs[i].tid;
        }
      }
      ReleaseMutex(mutex);
      DWORD id=WaitForMultipleObjects(njobs, joblist, FALSE, INFINITE);
      if (id>=WAIT_OBJECT_0 && id<WAIT_OBJECT_0+njobs) {
        id-=WAIT_OBJECT_0;
        id=jobptr[id];
        if (jobs[id].state==FINISHED) jobs[id].state=OK;
        if (jobs[id].state==FINISHED_ERR) quit(); //exit program on thread error 
        ++job_count;
        --thread_count;
      }
#endif
    }
  }
#ifdef PTHREAD
  check(pthread_mutex_unlock(&mutex));
#endif

    // Append temporary files to archive if OK.
    for (U32 i=0; i<jobs.size(); ++i) {
        if (jobs[i].state==OK) {
            filesmt[jobs[i].streamid]->setpos( 0);
            //append streams to archive
            const int BLOCK=4096;
            U8 blk[BLOCK];
            bool readdone=false; 
            for (;;) { 
                if (readdone) break;
                int bytesread=filesmt[jobs[i].streamid]->blockread(&blk[0], BLOCK);
                if (bytesread!=BLOCK) {
                    readdone=true;                   
                    archive->blockwrite(&blk[0],  bytesread  );
                } else      
                    archive->blockwrite(&blk[0],  BLOCK  );
            }
            filesmt[jobs[i].streamid]->close();
        }
    }

             #endif
            for (int i=0; i<streamc; ++i) {
                filestreams[i]->close();
            }
            
            // Write out segment data
            U64 segmentpos;
            segmentpos= archive->curpos();  //get segment data offset
            archive->setpos( segment.hpos);
            archive->put64(segmentpos);     //write segment data offset
            //compress segment data
            Encoder* segencode;
            Predictors* segpredict;
            FileTmp tmp;                    // temporary encoded file
            segpredict=new Predictor();
            segencode=new Encoder (COMPRESS, &tmp ,*segpredict); 
            for (U64 k=0; k<segment.pos; ++k) {
                segencode->compress(segment[k]);
            }
            segencode->flush();
            delete segpredict;
            delete segencode;
            archive->put32(segment.pos);     // write segment data size
            printf(" Segment data compressed from %d",segment.pos);
            segment.pos=tmp.curpos();
            segment.setsize(segment.pos);
            printf(" to %d bytes\n ",segment.pos);
            tmp.setpos( 0); 
            if (tmp.blockread(&segment[0], segment.pos)<segment.pos) quit("Segment data corrupted.");
            tmp.close();
            archive->put32(segment.pos);      // write  compressed segment data size
            archive->putc(streambit>>8&0xff); // write stream bit info
            archive->putc(streambit&0xff); 
            archive->setpos(segmentpos); 
            archive->blockwrite(&segment[0], segment.pos); //write out segment data
            //write stream size if present
            for (int i=0;i<streamc;i++){
                if (filestreamsize[i]>0) archive->put64(filestreamsize[i]);
            }
            printf("Total %0lu bytes compressed to %0lu bytes.\n", total_size,  archive->curpos()); 
            
        }
        // Decompress files to dir2: paq8pxd -d dir1/archive.paq8pxd dir2
        // If there is no dir2, then extract to dir1
        // If there is no dir1, then extract to .
        else if (!doList) {
            assert(argc>=2);
            String dir(argc>2?argv[2]:argv[1]);
            if (argc==2) {  // chop "/archive.paq8pxd"
                int i;
                for (i=dir.size()-2; i>=0; --i) {
                    if (dir[i]=='/' || dir[i]=='\\') {
                        dir[i]=0;
                        break;
                    }
                    if (i==1 && dir[i]==':') {  // leave "C:"
                        dir[i+1]=0;
                        break;
                    }
                }
                if (i==-1) dir=".";  // "/" not found
            }
            dir=dir.c_str();
            if (dir[0] && (dir.size()!=3 || dir[1]!=':')) dir+="/";
            /////
            
            delete en;
            delete predictord;
            for (int i=0; i<streamc; ++i) {
                filestreams[i]=new FileTmp;
            }            
            U64 datasegmentsize;
            U64 datasegmentlen;
            int datasegmentpos;
            int datasegmentinfo;
            Filetype datasegmenttype;
           predictord=0;
           Encoder *defaultencoder;
           defaultencoder=0;
            for (int i=0; i<streamc; ++i) {
                datasegmentsize=(filestreamsize[i]); // get segment data offset
                if (datasegmentsize>0){              // if segment contains data
                    filestreams[i]->setpos( 0);
                    U64 total=datasegmentsize;
                    datasegmentpos=0;
                    datasegmentinfo=0;
                    datasegmentlen=0;
                    if (predictord) delete predictord,predictord=0;
                    if (defaultencoder) delete defaultencoder,defaultencoder=0;
                    printf("DeCompressing ");
                    switch(i) {
                        case 0: {
                            printf("default stream.\n"); predictord=new Predictor();     break;}
                        case 1: {
                            printf("jpeg stream.\n");    predictord=new PredictorJPEG(); break;}
                        case 2: {
                            printf("image1 stream.\n");  predictord=new PredictorIMG1(); break;}
                        case 3: {
                            printf("image4 stream.\n");  predictord=new PredictorIMG4(); break;}
                        case 4: {
                            printf("image8 stream.\n");  predictord=new PredictorIMG8(); break;}
                        case 5: {
                            printf("image24 stream.\n"); predictord=new PredictorIMG24(); break;}
                        case 6: {
                            printf("audio stream.\n");   predictord=new PredictorAUDIO2(); break;}
                        case 7: {
                            printf("exe stream.\n");     predictord=new PredictorEXE();    break;}
                        case 8: {
                            printf("text0 wrt stream.\n"); predictord=new PredictorTXTWRT(); break;}
                        case 9:
                        case 10: {
                            printf("%stext wrt stream.\n",i==10?"big":"");  predictord=new PredictorTXTWRT(); break;}
                        case 11: {
                            printf("dec stream.\n"); predictord=new PredictorDEC(); break;}
                    }
                     defaultencoder=new Encoder (mode, archive,*predictord); 
                     if ((i>=0 && i<=7)||i==10||i==11){
                        while (datasegmentsize>0) {
                            while (datasegmentlen==0){
                                datasegmenttype=(Filetype)segment(datasegmentpos++);
                                for (int ii=0; ii<8; ii++) datasegmentlen=datasegmentlen<<8,datasegmentlen+=segment(datasegmentpos++);
                                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                                if (!(isstreamtype(datasegmenttype,i) ))datasegmentlen=0;
                                defaultencoder->predictor.x.filetype=datasegmenttype;
                                defaultencoder->predictor.x.blpos=0;
                                defaultencoder->predictor.x.finfo=datasegmentinfo;
                            }
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                                filestreams[i]->putc(defaultencoder->decompress());
                                datasegmentsize--;
                            }
                            datasegmentlen=0;
                        }
                    }
                    if (i==8 || i==9 ){
                        while (datasegmentsize>0) {
                        FileTmp tm;
                            datasegmentlen=datasegmentsize;
                            defaultencoder->predictor.x.filetype=DICTTXT;
                            defaultencoder->predictor.x.blpos=0;
                            defaultencoder->predictor.x.finfo=-1;
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                                tm.putc(defaultencoder->decompress());
                                datasegmentsize--;
                            }
                            
                            XWRT_Decoder* wrt;
                            wrt=new XWRT_Decoder();
                            int b=0;
                            wrt->defaultSettings(0);
                             tm.setpos( 0);
                            int bb=wrt->WRT_start_decoding(&tm);
                            for ( int ii=0; ii<bb; ii++) {
                                b=wrt->WRT_decode();    
                                filestreams[i]->putc(b);
                            }
                            tm.close();                             
                            delete wrt;
                            datasegmentlen=datasegmentsize=0;
                        }
                    }
                }
            } 
            // set datastream file pointers to beginning
            for (int i=0; i<streamc; ++i)         
            filestreams[i]->setpos( 0);
            /////
            segment.pos=0;
            for (int i=0; i<files; ++i) {
                String out(dir.c_str());
                out+=fname[i];
                DecodeStreams(out.c_str(), fsize[i]);
            } 
            int d=segment(segment.pos++);
            if (d!=0xff) printf("Segmend end marker not found\n");
            for (int i=0; i<streamc; ++i) {
                filestreams[i]->close();
            }
        }
        archive->close();
        if (!doList) programChecker.print();
    }
    catch(const char* s) {
        if (s) printf("%s\n", s);
    }
    if (pause) {
        printf("\nClose this window or press ENTER to continue...\n");
        getchar();
    }
    return 0;
}
