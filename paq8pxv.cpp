    /* paq8pxd file compressor/archiver.  Release by Kaido Orav

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
  -DDEFAULT_OPTION=N  (to change the default compression level from 8 to N).

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

- DEC Alpha: Swap byte order, do call transform

- ARM: do call transform

- JPEG: detected by SOI and SOF and ending with EOI or any nondecodable
  data.  No transform is applied.  The purpose is to separate images
  embedded in execuables to block the EXE transform, and for a future
  place to insert a transform.
  
- BASE64: Decodes BASE64 encoded data and recursively transformed
  up to level 5. Input can be full stream or end-of-line coded.
  
- BASE85: Decodes Ascii85 encoded data and recursively transformed
  up to level 5. Input can be full stream or end-of-line coded.
  Supports: https://en.wikipedia.org/wiki/Ascii85#Adobe_version

- UUENCODE: Decodes UUENCODE encoded data.

- 24-bit images: 24-bit image data uses simple color transform
  (b, g, r) -> (g, g-r, g-b)
  
- ZLIB:  Decodes zlib encoded data and recursively transformed
  up to level 5. Supports zlib compressed images (4/8/24 bit) in pdf

- GIF:  Gif (8 bit) image  recompression. 
  
- CD: mode 1 and mode 2 form 1+2 - 2352 bytes

- MDF: wraped around CD, re-arranges subchannel and CD data

- TEXT: All detected text blocks are transformed using dynamic dictionary
  preprocessing (based on XWRT). If transformed block is larger from original
  then transform is skipped.
  
- LZSS: (Haruhiko Okumura's LZSS): Decompresses Microsoft compress.exe archives.

- MRB: 8, 4 bit images with RLE compression in hlp files

- RLE: TGA, TIFF images

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


DIFFERENCES FROM PAQ8PXD_V17V2
- update all base classes from PAQ8PXD_V62
- disable zlib recomrpession and detection
- some fixes in vm
- change cfg files
*/

#define PROGNAME "paq8pxdv1"  // Please change this if you change the program.
#define SIMD_GET_SSE  //uncomment to use SSE2 in ContexMap
//#define MT            //uncomment for multithreading, compression only
//#define VMJIT  // uncomment to compile with x86 JIT
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
#define ispowerof2(x) ((x&(x-1))==0)
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

// Array<T,Align> a(n); allocates memory for n elements of T.
// The base address is aligned if the "alignment" parameter is given.
// Constructors for T are not called, the allocated memory is initialized to 0s.
// It's the caller's responsibility to populate the array with elements.
// Parameters are checked and indexing is bounds checked if assertions are on.
// Use of copy and assignment constructors are not supported.
//
// a.size(): returns the number of T elements currently in the array.
// a.resize(newsize): grows or shrinks the array.
// a.append(x): appends x to the end of the array and reserving space for more elements if needed.
// a.pop_back(): removes the last element by reducing the size by one (but does not free memory).
#ifndef NDEBUG
static void chkindex(U64 index, U64 upper_bound) {
  if (index>=upper_bound) {
    fprintf(stderr, "out of upper bound\n");
    quit();
  }
}
#endif

template <class T, const int Align=16> class Array {
private:
  U64 used_size;
  U64 reserved_size;
  char *ptr; // Address of allocated memory (may not be aligned)
  T* data;   // Aligned base address of the elements, (ptr <= T)
  void create(U64 requested_size);
  inline U64 padding() const {return Align-1;}
  inline U64 allocated_bytes() const {return (reserved_size==0)?0:reserved_size*sizeof(T)+padding();}
public:
  explicit Array(U64 requested_size) {create(requested_size);}
  ~Array();
  T& operator[](U64 i) {
    #ifndef NDEBUG
    chkindex(i,used_size);
    #endif
    return data[i];
  }
  const T& operator[](U64 i) const {
    #ifndef NDEBUG
    chkindex(i,used_size);
    #endif
    return data[i];
  }
  U64 size() const {return used_size;}
  void resize(U64 new_size);
  void pop_back() {assert(used_size>0); --used_size; }  // decrement size
  void push_back(const T& x);  // increment size, append x
  Array(const Array&) { assert(false); } //prevent copying - this method must be public (gcc must see it but actually won't use it)
private:
  Array& operator=(const Array&); //prevent assignment
};

template<class T, const int Align> void Array<T,Align>::create(U64 requested_size) {
  assert((Align&(Align-1))==0);
  used_size=reserved_size=requested_size;
  if (requested_size==0) {
    data=0;ptr=0;
    return;
  }
  U64 bytes_to_allocate=allocated_bytes();
  ptr=(char*)calloc(bytes_to_allocate,1);
  if(!ptr){
      printf("Requested size %0lu MB\n",((bytes_to_allocate)/1024)/1024);
      quit("Out of memory.");
  }
  U64 pad=padding();
  data=(T*)(((uintptr_t)ptr+pad) & ~(uintptr_t)pad);
  assert(ptr<=(char*)data && (char*)data<=ptr+Align);
  assert(((uintptr_t)data & (Align-1))==0); //aligned as expected?
  programChecker.alloc(bytes_to_allocate);
}

template<class T, const int Align> void Array<T,Align>::resize(U64 new_size) {
  if (new_size<=reserved_size) {
    used_size=new_size;
    return;
  }
  char *old_ptr=ptr;
  T *old_data=data;
  U64 old_size=used_size;
  programChecker.free(allocated_bytes());
  create(new_size);
  if(old_size>0) {
    assert(old_ptr && old_data);
    memcpy(data, old_data, sizeof(T)*old_size);
  }
  if(old_ptr){free(old_ptr);old_ptr=0;}
}

template<class T, const int Align> void Array<T,Align>::push_back(const T& x) {
  if(used_size==reserved_size) {
    U64 old_size=used_size;
    U64 new_size=used_size*2+16;
    resize(new_size);
    used_size=old_size;
  }
  data[used_size++]=x;
}

template<class T, const int Align> Array<T, Align>::~Array() {
  programChecker.free(allocated_bytes());
  free(ptr);
  used_size=reserved_size=0;
  data=0;ptr=0;
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
  FileTmp(): MAX_RAM_FOR_TMP_CONTENT( 16 * 1024 * 1024){content_in_ram=new Array<U8>(0); filepos=0; filesize=0; file_on_disk = 0;}
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
      if(newpos>filesize)ram_to_disk(); //panic: we don't support seeking past end of file - let's switch to disk
      else {filepos = newpos; return;}
    }  
     (*file_on_disk).setpos(newpos);
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

// Finalizers
// - Keep the necessary number of MSBs after a (combination of) 
//   multiplicative hash(es)

U32 finalize32(const U32 hash, const int hashbits) {
  assert(0<hashbits && hashbits<=32);
  return hash>>(32-hashbits);
}
U32 finalize64(const U64 hash, const int hashbits) {
  assert(0<hashbits && hashbits<=32);
  return U32(hash>>(64-hashbits));
}

// Get the next MSBs (8 or 6 bits) following "hasbits" for checksum
// Remark: the result must be cast/masked to the proper checksum size (U8, U16) by the caller
/*U64 checksum64(const U64 hash, const int hashbits, const int checksumbits) {
  return hash>>(64-hashbits-checksumbits); 
}*/
/////////////////////// Global context /////////////////////////
bool modeQuick=false;
U8 level=DEFAULT_OPTION;  // Compression level 0 to 15
U64 MEM(){
     return 0x10000UL<<level;
}

Segment segment; //for file segments type size info(if not -1)
const int streamc=12;
File * filestreams[streamc];
const int datatypecount=39+1+1+1;
typedef enum {DEFAULT=0,BINTEXT,DBASE, JPEG, HDR,IMGUNK, IMAGE1,IMAGE4, IMAGE8,IMAGE8GRAY, IMAGE24,IMAGE32, AUDIO, EXE,DECA,ARM,
              CD, TEXT,TEXT0, TXTUTF8,NESROM, BASE64, BASE85,UUENC, GIF ,SZDD,MRBR,MRBR4,RLE,LZW,
              ZLIB,MDF,MSZIP,EOLTEXT,DICTTXT,BIGTEXT,NOWRT,TAR,PNG8, PNG8GRAY,PNG24, PNG32,TYPELAST} Filetype;
typedef enum {INFO=0, STREAM,RECURSIVE} Filetypes;
const char* typenames[datatypecount]={"default","bintext","dBase", "jpeg", "hdr", "imgunk","1b-image", "4b-image", "8b-image","8b-gimage", "24b-image","32b-image", "audio",
                                "exe","DECa","ARM", "cd", "text","text0","utf-8","nes","base64","base85","uuenc","gif","SZDD","mrb","mrb4","rle","lzw","zlib","mdf","mszip","eoltxt",
                                "","","","tar","PNG8","PNG8G","PNG24","PNG32"};
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
  { 0, 0,  0},// ARM, 
  { 0,-1, 1},//  CD,
  { 0, 9, 0},// TEXT,  
  { 0, 8, 0},// TEXT0,
  { 0, 9, 0},// TXTUTF8,  
  { 0, 0, 0},// NESROM,  
  { 0,-1, 1},// BASE64, 
  { 0,-1, 1},// BASE85,   
  { 0,-1, 1},// UUENC, 
  { 0,-1, 0},// GIF,    
  { 1,-1, 1},// SZDD,  
  { 0,-1, 0},// MRBR, 
  { 0,-1, 0},// MRBR4,
  { 0,-1, 0},// RLE,
  { 0,-1, 0},// LZW,
  { 0,0, 0},// ZLIB, 
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
  { 1, 5, 0}// PNG32,
  };

//#define PNGFlag (1<<31)
//#define GrayFlag (1<<30)
// Contain all global data usable between models
class BlockData {
public: 
    Segment segment; //for file segments type size info(if not -1)
    int y; // Last bit, 0 or 1, set by encoder
    int c0; // Last 0-7 bits of the partial byte with a leading 1 bit (1-255)
    U32 c4;//,c8; // Last 4,4 whole bytes, packed.  Last byte is bits 0-7.
    int bpos; // bits in c0 (0 to 7)
    Buf buf;  // Rotating input queue set by Predictor
    Buf bufn;  // Rotating input queue set by Predictor
    int blpos; // Relative position in block
    int rm1;
    Filetype filetype;
    int finfo;
BlockData():y(0), c0(1), c4(0),bpos(0),blpos(0),rm1(1),filetype(DEFAULT)
   {
        // Set globals according to option
        assert(level<=15);
        bufn.setsize(0x10000);
        if (level>=9) buf.setsize(0x10000000); //limit 256mb
        else buf.setsize(MEM()*8);
        #ifndef NDEBUG 
        printf("\n Buf size %d bytes\n", buf.poswr);
        #endif
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
#include <immintrin.h>
#define OPTIMIZE "AVX2-"
#elif defined(__SSE4_1__)   
#include<smmintrin.h>
#elif   defined(__SSSE3__)
#include<tmmintrin.h>
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
  int mask;
public:
  APM1(int n,BlockData& x);
  int p(int pr=2048, int cxt=0, int rate=7) {
    assert(pr>=0 && pr<4096 && rate>0 && rate<32);
    
    pr=stretch(pr);
    int g=(x.y<<16)+(x.y<<rate)-x.y-x.y;
    t[index] += (g-t[index]) >> rate;
    t[index+1] += (g-t[index+1]) >> rate;
    const int w=pr&127;  // interpolation weight (33 points)
    index=((pr+2048)>>7)+(cxt&mask)*33;
    return (t[index]*(128-w)+t[index+1]*w) >> 11;
  }
};

// maps p, cxt -> p initially
APM1::APM1(int n,BlockData& bd): index(0), N(n), t(n*33),x(bd),mask(n-1) {
    assert(ispowerof2(n));
  for (int i=0; i<N; ++i)
    for (int j=0; j<33; ++j)
      t[i*33+j] = i==0 ? squash((j-16)*128)*16 : t[j];
}

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
  //bool doText; 
  Array<ErrorInfo> info; 
  Array<int> rates; // learning rates
public:  
  BlockData& x;
  Mixer(int n, int m,BlockData& bd, int s=1, int w=0);
  
#if defined(__AVX2__)
 int dot_product (const short* const t, const short* const w, int n) {
  assert(n == ((n + 15) & -16));
  __m256i sum = _mm256_setzero_si256 ();
  while ((n -= 16) >= 0) { // Each loop sums 16 products
    __m256i tmp = _mm256_madd_epi16 (*(__m256i *) &t[n], *(__m256i *) &w[n]); // t[n] * w[n] + t[n+1] * w[n+1]
    tmp = _mm256_srai_epi32 (tmp, 8); //                                        (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
    sum = _mm256_add_epi32 (sum, tmp); //                                sum += (t[n] * w[n] + t[n+1] * w[n+1]) >> 8
  } 
   sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());       //add [1]=[1]+[2], [2]=[3]+[4], [3]=0, [4]=0, [5]=[5]+[6], [6]=[7]+[8], [7]=0, [8]=0
   sum =_mm256_hadd_epi32(sum,_mm256_setzero_si256 ());       //add [1]=[1]+[2], [2]=0,       [3]=0, [4]=0, [5]=[5]+[6], [6]=0,       [7]=0, [8]=0
   __m128i lo = _mm256_extractf128_si256(sum, 0);
   __m128i hi = _mm256_extractf128_si256(sum, 1);
   __m128i newsum = _mm_add_epi32(lo, hi);                    //sum last two
   return _mm_cvtsi128_si32(newsum);
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
  sum = _mm_add_epi32(sum, _mm_srli_si128 (sum, 8));
  sum = _mm_add_epi32(sum, _mm_srli_si128 (sum, 4));
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
    else     if(x.filetype==DICTTXT) train(&tx[0], &wx[0], nx, ((x.y<<12)-base)*3/2), reset();
    else             update();
  }
  // Input x (call up to N times)
  void add(int x) {
    assert(nx<N);
    tx[nx++]=x;
  }
  void add32(U32 a){
      assert(nx+2<N);
     ((U32 *) &tx[nx],a);
    nx=nx+2;
  }
  void add64(U64 a){
      assert(nx+4<N);
     ((U64 *) &tx[nx],a);
    nx=nx+4;
  }
  void addXMM(XMM a){
    assert(nx+8<N);
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
          if(x.filetype==DICTTXT) dp=(dp*9)>>9;  
          else             dp=dp>>(5+shift0);
          pr[i]=squash(dp);
          mp->add(dp);
      }
     if(x.filetype!=DICTTXT) mp->set(0, 1);
      return mp->p(shift0, shift1);
    }
    else {  // S=1 context
    if(x.filetype!=DICTTXT)  
    return pr[0]=squash(dot_product(&tx[0], &wx[0], nx)>>(8+shift1));
      int z=dot_product(&tx[0], &wx[0], nx);
    base=squash( (z*16) >>13);
    return squash(z>>9);
    }
  }
  ~Mixer();
};

Mixer::~Mixer() {
  delete mp;
}

Mixer::Mixer(int n, int m, BlockData& bd, int s, int w):
    N((n+15)&-16), M(m), S(s), wx(N*M),
    cxt(S), ncxt(0), base(0), pr(S), mp(0),tx(N),nx(0),x(bd),/*doText(false),*/ info(S), rates(S) {
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

 
class StateMapContext {
protected:
  const int N;  // Number of contexts
  int cxt;      // Context of last prediction
  Array<U32> t;       // cxt -> prediction in high 22 bits, count in low 10 bits
  int pr;
  int mask;
  BlockData& x;
  inline void update( int limit) {
  //	printf("cx=%d  \n",cxt   );
    assert(cxt>=0 && cxt<N);
    assert(x.y==0 || x.y==1);
    U32 &p=t[cxt];
    int n=p&1023, pr=p>>10;  // count, prediction
    p+=(n<limit);
    p+=((((x.y<<22)-pr)>>3)*dt[n]&0xfffffc00);
}
public:
  StateMapContext(int n,  BlockData& x);//
  // update bit y (0..1), predict next bit in context cx
  int p(int cx,int limit=1023) {
    //assert(cx>=0 && cx<N);
    assert(limit>0 && limit<1024);
    update(limit);
    return pr=t[cxt=(cx&mask)]>>20;
  }
  void mix(Mixer& m) {   
    m.add(stretch(pr));
  }
};

StateMapContext::StateMapContext(int n, BlockData& bd): N(n), cxt(0), t(n), mask(n-1), x(bd) {
    assert(ispowerof2(n));
  for (int i=0; i<N; ++i)
    t[i]=1<<31; 
} 
// An APM maps a probability and a context to a new probability.  Methods:
//
// APM a(n) creates with n contexts using 96*n bytes memory.
// a.pp(y, pr, cx, limit) updates and returns a new probability (0..4095)
//     like with StateMap.  pr (0..4095) is considered part of the context.
//     The output is computed by interpolating pr into 24 ranges nonlinearly
//     with smaller ranges near the ends.  The initial output is pr.
//     y=(0..1) is the last bit.  cx=(0..n-1) is the other context.
//     limit=(0..1023) defaults to 255.

class APM: public StateMap {
public:
  APM(int n);
  int p(int pr, int cx, int y,int limit=255) {
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/24);
    assert(y==0 || y==1);
    assert(limit>0 && limit<1024);
    update(y,limit);
    pr=(stretch(pr)+2048)*23;
    int wt=pr&0xfff;  // interpolation weight of next element
    cx=cx*24+(pr>>12);
    assert(cx>=0 && cx<N-1);
    cxt=cx+(wt>>11);
    pr=((t[cx]>>13)*(0x1000-wt)+(t[cx+1]>>13)*wt)>>19;
    return pr;
  }
};

APM::APM(int n): StateMap(n*24) {
  for (int i=0; i<N; ++i) {
    int p=((i%24*2+1)*4096)/48-2048;
    t[i]=(U32(squash(p))<<20)+6;
  }
}


class APM2: public StateMapContext {
public:
  APM2(int n, BlockData& bd);
  int p(int pr, int cx, int limit=255) {
    assert(pr>=0 && pr<4096);
    assert(cx>=0 && cx<N/24);
    //assert(y==0 || y==1);
    assert(limit>0 && limit<1024);
    update(limit);
    pr=(stretch(pr)+2048)*23;
    int wt=pr&0xfff;  // interpolation weight of next element
    cx=cx*24+(pr>>12);
    assert(cx>=0 && cx<N-1);
    cxt=cx+(wt>>11);
    pr=((t[cx]>>13)*(0x1000-wt)+(t[cx+1]>>13)*wt)>>19;
    return pr;
  }
};

APM2::APM2(int n, BlockData& bd): StateMapContext(n*24,bd)  {
  for (int i=0; i<N; ++i) {
    int p=((i%24*2+1)*4096)/48-2048;
    t[i]=(U32(squash(p))<<20)+6;
  }
}

//////////////////////////// hash //////////////////////////////

// Hash 2-5 ints.
inline U32 hash(U32 a, U32 b, U32 c=0xffffffff, U32 d=0xffffffff,
    U32 e=0xffffffff) {
  U32 h=a*200002979u+b*30005491u+c*50004239u+d*70004807u+e*110002499u;
  return h^h>>9^a>>2^b>>3^c>>4^d>>5^e>>6;
}
inline U32 hash0(U32 a, U32 b, U32 c=0xffffffff, U32 d=0xffffffff,
    U32 e=0xffffffff) {
  U32 h=a*200002981u+b*30005491u+c*50004239u+d*70004807u+e*110002499u;
  return h^(h)>>9^a>>2^b>>3^c>>4^d>>5^e>>6;
}
// Magic number 2654435761 is the prime number closest to the 
// golden ratio of 2^32 (2654435769)
#define PHI 0x9E3779B1 //2654435761

// A hash function to diffuse a 32-bit input
inline U32 hash(U32 x) {
  x++; // zeroes are common and mapped to zero
  x = ((x >> 16) ^ x) * 0x85ebca6b;
  x = ((x >> 13) ^ x) * 0xc2b2ae35;
  x = (x >> 16) ^ x;
  return x;
}

// Combine a hash value (seed) with another (non-hash) value.
// The result is a combined hash. 
//
// Use this function repeatedly to combine all input values 
// to be hashed to a final hash value.
inline U32 combine(U32 seed, const U32 x) {
  seed+=(x+1)*PHI;
  seed+=seed<<10;
  seed^=seed>>6;
  return seed;
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
template <U32 B> class BH {
  enum {M=8};  // search limit
  Array<U8, 64> t; // elements
  //Array<U8> tmp;
  U8 tmp[B];
  U32 n; // size-1
public:
  BH(U32 i): t(i*B), n(i-1) {
    //printf("BH %0.0f, i %d B %d power %d\n",(i*B)+0.0,i,B,(i&(i-1))==0);
    assert(B>=2 && i>0 && (i&(i-1))==0); // size a power of 2?
    
  }
  U8* operator[](U32 i);
};

template <U32 B>
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
- InputBits: How many bits [1..8] of input are to be modelled for each context.
New contexts must be set at those intervals.

Uses (2^(BitsOfContext+1))*((2^InputBits)-1) bytes of memory.
*/

class SmallStationaryContextMap {
  Array<U16> Data;
  int Context, Mask, Stride, bCount, bTotal, B;
  U16 *cp;
public:
  SmallStationaryContextMap(int BitsOfContext, int InputBits = 8) : Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), Context(0), Mask((1<<BitsOfContext)-1), Stride((1<<InputBits)-1), bCount(0), bTotal(InputBits), B(0) {
    if (BitsOfContext>16){
        printf("dff");
    }
    assert(BitsOfContext<=16);
    assert(InputBits>0 && InputBits<=8);
    Reset();
    cp=&Data[0];
  }
  void set(U32 ctx) {
    Context = (ctx&Mask)*Stride;
    bCount=B=0;
  }
  void Reset() {
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=0x7FFF;
  }
  void mix(Mixer& m, const int rate = 7, const int Multiplier = 1, const int Divisor = 4) {
    *cp+=((m.x.y<<16)-(*cp)+(1<<(rate-1)))>>rate;
    B+=(m.x.y && B>0);
    cp = &Data[Context+B];
    int Prediction = (*cp)>>4;
    m.add((stretch(Prediction)*Multiplier)/Divisor);
    m.add(((Prediction-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }
};

/*
  Map for modelling contexts of (nearly-)stationary data.
  The context is looked up directly. For each bit modelled, a 32bit element stores
  a 22 bit prediction and a 10 bit adaptation rate offset.

  - BitsOfContext: How many bits to use for each context. Higher bits are discarded.
  - InputBits: How many bits [1..8] of input are to be modelled for each context.
    New contexts must be set at those intervals.
  - Rate: Initial adaptation rate offset [0..1023]. Lower offsets mean faster adaptation.
    Will be increased on every occurrence until the higher bound is reached.

    Uses (2^(BitsOfContext+2))*((2^InputBits)-1) bytes of memory.
*/

class StationaryMap {
  Array<U32> Data;
  int Context, Mask, Stride, bCount, bTotal, B;
  U32 *cp;
public:
  StationaryMap(int BitsOfContext, int InputBits = 8, int Rate = 0): Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), Context(0), Mask((1<<BitsOfContext)-1), Stride((1<<InputBits)-1), bCount(0), bTotal(InputBits), B(0) {
    assert(InputBits>0 && InputBits<=8);
    assert(BitsOfContext+InputBits<=24);
    Reset(Rate);
    cp=&Data[0];
  }
  void set(U32 ctx) {
    Context = (ctx&Mask)*Stride;
    bCount=B=0;
  }
  void Reset( int Rate = 0 ){
    for (U32 i=0; i<Data.size(); ++i)
      Data[i]=(0x7FF<<20)|min(1023,Rate);
  }
  void mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 1023) {
    // update
    U32 Count = min(min(Limit,0x3FF), ((*cp)&0x3FF)+1);
    int Prediction = (*cp)>>10, Error = (m.x.y<<22)-Prediction;
    Error = ((Error/8)*dt[Count])/1024;
    Prediction = min(0x3FFFFF,max(0,Prediction+Error));
    *cp = (Prediction<<10)|Count;
    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    Prediction = (*cp)>>20;
    m.add((stretch(Prediction)*Multiplier)/Divisor);
    m.add(((Prediction-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
  }
};


class IndirectMap {
  Array<U8> Data;
  StateMap Map;
  const int mask, maskbits, stride;
  int Context, bCount, bTotal, B;
  U8 *cp;
public:
  IndirectMap(int BitsOfContext, int InputBits = 8): Data((1ull<<BitsOfContext)*((1ull<<InputBits)-1)), mask((1<<BitsOfContext)-1), maskbits(BitsOfContext), stride((1<<InputBits)-1), Context(0), bCount(0), bTotal(InputBits), B(0) {
    assert(InputBits>0 && InputBits<=8);
    assert(BitsOfContext+InputBits<=24);
    cp=&Data[0];
  }
  void set_direct(const U32 ctx) {
    Context = (ctx&mask)*stride;
    bCount=B=0;
  }
  void set(const U64 ctx) {
    Context = (finalize64(ctx,maskbits)&mask)*stride;
    bCount=B=0;
  }
  void mix(Mixer& m, const int Multiplier = 1, const int Divisor = 4, const U16 Limit = 1023) {
    // update
    *cp = nex(*cp, m.x.y);
    // predict
    B+=(m.x.y && B>0);
    cp=&Data[Context+B];
    const U8 state = *cp;
    const int p1 = Map.p(state,m.x.y, Limit);
    m.add((stretch(p1)*Multiplier)/Divisor);
    m.add(((p1-2048)*Multiplier)/(Divisor*2));
    bCount++; B+=B+1;
    if (bCount==bTotal)
      bCount=B=0;
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
    //if (size>(0x100000000UL)) return (0x100000000UL); //limit to 4GB, using this will consume lots of memory above level 11
    if (size>(0x80000000UL)) return (0x80000000UL); //limit to 2GB
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
  int result;
  int mix1(Mixer& m, int cc, int bp, int c1, int y1);
    // mix() with global context passed as arguments to improve speed.
    
public:
  ContextMap(U64 m, int c=1);  // m = memory in bytes, a power of 2, C = c
  ~ContextMap();
  void set(U32 cx, int next=-1);   // set next whole byte context to cx
    // if next is 0 then set order does not matter
  int mix(Mixer& m) {return mix1(m, m.x.c0, m.x.bpos, m.x.buf(1), m.x.y);}
  int get() {return result;}
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
    cxt(C), runp(C), r0(C),r1(C),r0i(C),rmask(C),cn(0),result(0) {
  assert(m>=64 && (m&m-1)==0);  // power of 2?
  assert(sizeof(E)==64);
  sm=new StateMap[C];
  for (int i=0; i<C; ++i) {
    cp0[i]=cp[i]=&t[0].bh[0][0];
    runp[i]=cp[i]+3;
  }
  #ifndef NDEBUG 
  printf("ContextMap t %0.2f mbytes\n",(((t.size()*sizeof(E)) +0.0)/1024)/1024);
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
  m.add(st*(n1-n0));
  m.add((p1&n0)-(p0&n1));
  m.add((p1&n1)-(p0&n0));
  return s>0;
}


// Update the model with bit y1, and predict next bit to mixer m.
// Context: cc=c0, bp=bpos, c1=buf(1), y1=y.
int ContextMap::mix1(Mixer& m, int cc, int bp, int c1, int y1) {
  // Update model with y
   result=0;

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
      case 1: case 3: case 6: cp[i]=cp0[i]+1+(cc&1); break;
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
    for (int i=0; i<(inputs()-1); i++)
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
    for (int i=0; i<(cnc); i=i+8) {
        XMM  x0=_mm_setzero_si128();
        XMM  x1=_mm_set1_epi16(1);
        const int bsh=(8-bp);
        XMM   b=_mm_load_si128 ((XMM  *) &r1[i]);
        XMM xcc=_mm_set1_epi16(cc);
        //(r1[i  ]+256)>>(8-bp)==cc
        XMM   lasth=_mm_set1_epi16(256);
        XMM  xr1=_mm_add_epi16 (b, lasth);
        xr1=_mm_srli_epi16 (xr1, bsh);
        xr1=_mm_cmpeq_epi16(xr1,xcc); //(a == b) ? 0xffff : 0x0
        //b                           //((r1[i  ]>>(7-bp)&1)*2-1) 
        const int bsh1=(7-bp);
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

/*
Context map for large contexts (32bits).
Maps to a bit history state, a 3 MRU byte history, and 1 byte RunStats.

Bit and byte histories are stored in a hash table with 64 byte buckets.
The buckets are indexed by a context ending after 0, 2 or 5 bits of the
current byte. Thus, each byte modeled results in 3 main memory accesses
per context, with all other accesses to cache.

On a byte boundary (bit 0), only 3 of the 7 bit history states are used.
Of the remaining 4 bytes, 3 are then used to store the last bytes seen
in this context, 7 bits to store the length of consecutive occurrences of
the previously seen byte, and 1 bit to signal if more than 1 byte as been
seen in this context. The byte history is then combined with the bit history
states to provide additional states that are then mapped to predictions.
*/

class ContextMap2 {
  const U32 C; // max number of contexts
  class Bucket { // hash bucket, 64 bytes
    U16 Checksums[7]; // byte context checksums
    U8 MRU; // last 2 accesses (0-6) in low, high nibble
  public:
    U8 BitState[7][7]; // byte context, 3-bit context -> bit history state
                       // BitState[][0] = 1st bit, BitState[][1,2] = 2nd bit, BitState[][3..6] = 3rd bit
                       // BitState[][0] is also a replacement priority, 0 = empty
    inline U8* Find(U16 Checksum) { // Find or create hash element matching checksum.
                                    // If not found, insert or replace lowest priority (skipping 2 most recent).
      if (Checksums[MRU&15]==Checksum)
        return &BitState[MRU&15][0];
      int worst=0xFFFF, idx=0;
      for (int i=0; i<7; ++i) {
        if (Checksums[i]==Checksum)
          return MRU=MRU<<4|i, (U8*)&BitState[i][0];
        if (BitState[i][0]<worst && (MRU&15)!=i && MRU>>4!=i) {
          worst = BitState[i][0];
          idx=i;
        }
      }
      MRU = 0xF0|idx;
      Checksums[idx] = Checksum;
      return (U8*)memset(&BitState[idx][0], 0, 7);
    }
  };
  Array<Bucket, 64> Table; // bit histories for bits 0-1, 2-4, 5-7
                           // For 0-1, also contains run stats in BitState[][3] and byte history in BitState[][4..6]
  Array<U8*> BitState; // C pointers to current bit history states
  Array<U8*> BitState0; // First element of 7 element array containing BitState[i]
  Array<U8*> ByteHistory; // C pointers to run stats plus byte history, 4 bytes, [RunStats,1..3]
  Array<U32> Contexts; // C whole byte contexts (hashes)
  Array<bool> HasHistory; // True if context has a full valid byte history (i.e., seen at least 3 times)
  StateMap **Maps6b, **Maps8b, **Maps12b;
  U32 index; // Next context to set by set()
  U32 bits;
  U8 lastByte, lastBit, bitPos;
  int result ;
  inline void Update() {
    U64 mask = Table.size()-1;
    for (U32 i=0; i<index; i++) {
      if (BitState[i])
        *BitState[i] = nex(*BitState[i], lastBit);

      if (bitPos>1 && ByteHistory[i][0]==0)
        BitState[i] = nullptr;
      else {
        U16 chksum = Contexts[i]>>16;
        switch (bitPos) {
          case 0: {
            BitState[i] = BitState0[i] = Table[(Contexts[i]+bits)&mask].Find(chksum);
            // Update pending bit histories for bits 2-7
            if (BitState0[i][3]==2) {
              const int c = BitState0[i][4]+256;
              U8 *p = Table[(Contexts[i]+(c>>6))&mask].Find(chksum);
              p[0] = 1+((c>>5)&1);
              p[1+((c>>5)&1)] = 1+((c>>4)&1);
              p[3+((c>>4)&3)] = 1+((c>>3)&1);
              p = Table[(Contexts[i]+(c>>3))&mask].Find(chksum);
              p[0] = 1+((c>>2)&1);
              p[1+((c>>2)&1)] = 1+((c>>1)&1);
              p[3+((c>>1)&3)] = 1+(c&1);
              BitState0[i][6] = 0;
            }
            // Update byte history of previous context
            ByteHistory[i][3] = ByteHistory[i][2];
            ByteHistory[i][2] = ByteHistory[i][1];
            if (ByteHistory[i][0]==0)  // new context
              ByteHistory[i][0]=2, ByteHistory[i][1]=lastByte;
            else if (ByteHistory[i][1]!=lastByte)  // different byte in context
              ByteHistory[i][0]=1, ByteHistory[i][1]=lastByte;
            else if (ByteHistory[i][0]<254)  // same byte in context
              ByteHistory[i][0]+=2;
            else if (ByteHistory[i][0]==255) // more than one byte seen, but long run of current byte, reset to single byte seen
              ByteHistory[i][0] = 128;

            ByteHistory[i] = BitState0[i]+3;
            HasHistory[i] = *BitState0[i]>15;
            break;
          }
          case 2: case 5: {
            BitState[i] = BitState0[i] = Table[(Contexts[i]+bits)&mask].Find(chksum);
            break;
          }
          case 1: case 3: case 6: BitState[i] = BitState0[i]+1+lastBit; break;
          case 4: case 7: BitState[i] = BitState0[i]+3+(bits&3); break;
        }
      }
    }
  }
public:
  // Construct using Size bytes of memory for Count contexts
  ContextMap2(const U64 Size, const U32 Count) : C(Count), Table(Size>>6), BitState(Count), BitState0(Count), ByteHistory(Count), Contexts(Count), HasHistory(Count){
   // assert(Size>=64 && ispowerof2(Size));
    assert(sizeof(Bucket)==64);
    Maps6b = new StateMap*[C];
    Maps8b = new StateMap*[C];
    Maps12b = new StateMap*[C];
    for (U32 i=0; i<C; i++) {
      Maps6b[i] = new StateMap((1<<6)+8);
      Maps8b[i] = new StateMap(1<<8);
      Maps12b[i] = new StateMap((1<<12)+(1<<9));
      BitState[i] = BitState0[i] = &Table[i].BitState[0][0];
      ByteHistory[i] = BitState[i]+3;
    }
    index = 0;
    lastByte = lastBit = 0;
    bits = 1;  bitPos = 0;
    #ifndef NDEBUG 
  printf("ContextMap2 t %0.2f mbytes\n",(((Table.size()*sizeof(Bucket)) +0.0)/1024)/1024);
  #endif
  result=0;
  }
  ~ContextMap2() {
    for (U32 i=0; i<C; i++) {
      delete Maps6b[i];
      delete Maps8b[i];
      delete Maps12b[i];
    }
    delete[] Maps6b;
    delete[] Maps8b;
    delete[] Maps12b;
  }
  int get() {return result;}
  inline void set(U32 ctx) { // set next whole byte context to ctx
    ctx = ctx*987654323+index; // permute (don't hash) ctx to spread the distribution
    ctx = ctx<<16|ctx>>16;
    Contexts[index] = ctx*123456791+index;
    index++;
    assert(index>0 && index<=C);
  }
  int mix(Mixer& m) {
    result = 0;
    lastBit = m.x.y;
    bitPos = m.x.bpos;
    bits+=bits+lastBit;
    lastByte = bits&0xFF;
    if (bitPos==0)
      bits = 1;
    Update();

    for (U32 i=0; i<index; i++) {
      // predict from bit context
      int state = (BitState[i])?*BitState[i]:0;
      result+=(state>0);
      int p1 = Maps8b[i]->p(state,m.x.y);
      int n0=nex(state, 2), n1=nex(state, 3), k=-~n1;
      k = (k*64)/(k-~n0);
      n0=-!n0, n1=-!n1;
      // predict from last byte in context
      if ((U32)((ByteHistory[i][1]+256)>>(8-bitPos))==bits){
        int RunStats = ByteHistory[i][0]; // count*2, +1 if 2 different bytes seen
        int sign=(ByteHistory[i][1]>>(7-bitPos)&1)*2-1;  // predicted bit + for 1, - for 0
        int value = ilog(RunStats+1)<<(3-(RunStats&1));
        m.add(sign*value);
      }
      else if (bitPos>0 && (ByteHistory[i][0]&1)>0) {
        if ((U32)((ByteHistory[i][2]+256)>>(8-bitPos))==bits)
          m.add((((ByteHistory[i][2]>>(7-bitPos))&1)*2-1)*128);
        else if (HasHistory[i] && (U32)((ByteHistory[i][3]+256)>>(8-bitPos))==bits)
          m.add((((ByteHistory[i][3]>>(7-bitPos))&1)*2-1)*128);
        else
          m.add(0);
      }
      else
        m.add(0);

      int st=(stretch(p1)+(1<<1))>>2;
      m.add(st);
      m.add((p1-2047)>>3);
      if (state == 0) {
        m.add(0);
        m.add(0);
      } else {
        m.add(st*abs(n1-n0));
        const int p0=4095-p1;
        m.add(((p1&n0)-(p0&n1)+(1<<3))>>4);
      }

      if (HasHistory[i]) {
        state  = (ByteHistory[i][1]>>(7-bitPos))&1;
        state |= ((ByteHistory[i][2]>>(7-bitPos))&1)*2;
        state |= ((ByteHistory[i][3]>>(7-bitPos))&1)*4;
      }
      else
        state = 8;

      m.add(stretch(Maps12b[i]->p((state<<9)|(bitPos<<6)|k,m.x.y) )>>2);
      m.add(stretch(Maps6b[i]->p((state<<3)|bitPos,m.x.y))>>2);
    }
    if (bitPos==7) index = 0;
   // return result;
  }
  int inputs(){return 7; }
};

////////////////////////////// Indirect Context //////////////////////////////

template <typename T>
class IndirectContext {
private:
  Array<T> data;
  T* ctx;
  U32 ctxMask, inputMask, inputBits;
public:
  IndirectContext(const int BitsPerContext, const int InputBits = 8) :
    data(1ull<<BitsPerContext),
    ctx(&data[0]),
    ctxMask((1ul<<BitsPerContext)-1),
    inputMask((1ul<<InputBits)-1),
    inputBits(InputBits)
  {
    assert(BitsPerContext>0 && BitsPerContext<=20);
    assert(InputBits>0 && InputBits<=8);
  }
  void operator+=(const U32 i) {
    assert(i<=inputMask);
    (*ctx)<<=inputBits;
    (*ctx)|=i;
  }
  void operator=(const U32 i) {
    ctx = &data[i&ctxMask];
  }
  T& operator()(void) {
    return *ctx;
  }
};

class MTFList{
private:
  int Root, Index;
  Array<int, 16> Previous;
  Array<int, 16> Next;
public:
  MTFList(const U16 n): Root(0), Index(0), Previous(n), Next(n) {
    assert(n>0);
    for (int i=0;i<n;i++) {
      Previous[i] = i-1;
      Next[i] = i+1;
    }
    Next[n-1] = -1;
  }
  inline int GetFirst(){
    return Index=Root;
  }
  inline int GetNext(){
    if(Index>=0){Index=Next[Index];return Index;}
    return Index; //-1
  }
  inline void MoveToFront(int i){
    assert(i>=0 && i<Previous.size());
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
/*
  
const U8 AsciiGroupC0[254] ={
  0, 10,
  0, 1, 10, 10,
  0, 4, 2, 3, 10, 10, 10, 10,
  0, 0, 5, 4, 2, 2, 3, 3, 10, 10, 10, 10, 10, 10, 10, 10,
  0, 0, 0, 0, 5, 5, 9, 4, 2, 2, 2, 2, 3, 3, 3, 3, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
  0, 0, 0, 0, 0, 0, 0, 0, 5, 8, 8, 5, 9, 9, 6, 5, 2, 2, 2, 2, 2, 2, 2, 8, 3, 3, 3, 3, 3, 3, 3, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 8, 8, 8, 8, 5, 5, 9, 9, 9, 9, 9, 7, 8, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 8, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 8, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
};
const U8 AsciiGroup[128] = {
  0,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  4,  5,  5,  4,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,
  6,  7,  8, 17, 17,  9, 17, 10,
  11, 12, 17, 17, 13, 14, 15, 16,
  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1, 18, 19, 20, 23, 21, 22,
  23,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2, 24, 27, 25, 27, 26,
  27,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3, 28, 30, 29, 30, 30
};*/
class Model {
public:
  virtual  int p(Mixer& m,int val1,int val2)=0;
  virtual  int inputs()=0;
};
/*
class SparseMatchModel {
private:
    BlockData& x;
    Buf& buffer;
  enum Parameters : U32 {
    MaxLen    = 0xFFFF, // longest allowed match
    MinLen    = 3,      // default minimum required match length
    NumHashes = 4,      // number of hashes used
  };
  struct sparseConfig {
    U32 offset;//    = 0;      // number of last input bytes to ignore when searching for a match
    U32 stride ;//    = 1;      // look for a match only every stride bytes after the offset
    U32 deletions;//  = 0;      // when a match is found, ignore these many initial post-match bytes, to model deletions
    U32 minLen ;//    = MinLen;
    U32 bitMask ;//   = 0xFF;   // match every byte according to this bit mask
  };
  const sparseConfig sparse[NumHashes] = { {0,1,0,5,0xDF},{1,1,0,4,0xFF}, {0,2,0,4,0xDF}, {0,1,0,5,0x0F}};
  Array<U32> Table;
  StationaryMap Maps[4];
  IndirectContext<U8> iCtx8;
  IndirectContext<U16> iCtx16;
  MTFList list;
  U32 hashes[NumHashes];
  U32 hashIndex;   // index of hash used to find current match
  U32 length;      // rebased length of match (length=1 represents the smallest accepted match length), or 0 if no match
  U32 index;       // points to next byte of match in buffer, 0 when there is no match
  U32 mask;
  U8 expectedByte; // prediction is based on this byte (buffer[index]), valid only when length>0
  bool valid;
  void Update() {
    // update sparse hashes
    for (U32 i=0; i<NumHashes; i++) {
      hashes[i] = (i+1)*PHI;
      for (U32 j=0, k=sparse[i].offset+1; j<sparse[i].minLen; j++, k+=sparse[i].stride)
        hashes[i] = combine(hashes[i], (buffer(k)&sparse[i].bitMask)<<i);
      hashes[i]&=mask;
    }
    // extend current match, if available
    if (length) {
      index++;
      if (length<MaxLen)
        length++;
    }
    // or find a new match
    else {     
      for (int i=list.GetFirst(); i>=0; i=list.GetNext()) {
        index = Table[hashes[i]];
        if (index>0) {
          U32 offset = sparse[i].offset+1;
          while (length<sparse[i].minLen && ((buffer(offset)^buffer[index-offset])&sparse[i].bitMask)==0) {
            length++;
            offset+=sparse[i].stride;
          }
          if (length>=sparse[i].minLen) {
            length-=(sparse[i].minLen-1);
            index+=sparse[i].deletions;
            hashIndex = i;
            list.MoveToFront(i);
            break;
          }
        }
        length = index = 0;
      }
    }
    // update position information in hashtable
    for (U32 i=0; i<NumHashes; i++)
      Table[hashes[i]] = buffer.pos;
    
    expectedByte = buffer[index];
    if (valid)
      iCtx8+=x.y, iCtx16+=buffer(1);
    valid = length>1; // only predict after at least one byte following the match
    if (valid) {
      Maps[0].set(hash(expectedByte, x.c0, buffer(1), buffer(2), ilog2(length+1)*NumHashes+hashIndex));
      Maps[1].set((expectedByte<<8)|buffer(1));
      iCtx8=(buffer(1)<<8)|expectedByte, iCtx16=(buffer(1)<<8)|expectedByte;
      Maps[2].set(iCtx8());
      Maps[3].set(iCtx16());
    }
  }
public:
    SparseMatchModel(BlockData& bd, U32 val1=0) :
    x(bd),buffer(bd.buf),
    Table(CMlimit(MEM()/2)),//?
    Maps{ {22, 1}, {17, 4}, {8, 1}, {19,1} },
    iCtx8{19,1},
    iCtx16{16},
    list(NumHashes),
    hashes{ 0 },
    hashIndex(0),
    length(0),
    mask(CMlimit(MEM()/2)-1),
    expectedByte(0),
    valid(false)
  {
    //assert(ispowerof2(Size));
  }
  int inputs() {return  4*2+3;}
  int p(Mixer& m,int val1=0,int val2=0) {
    const U8 B = x.c0<<(8-x.bpos);
    if (x.bpos==0)
      Update();
    else if (valid) {
      U8 B = x.c0<<(8-x.bpos);
      Maps[0].set(hash(expectedByte, x.c0, buffer(1), buffer(2), ilog2(length+1)*NumHashes+hashIndex));
      if (x.bpos==4)
        Maps[1].set(0x10000|((expectedByte^U8(x.c0<<4))<<8)|buffer(1));
      iCtx8+=x.y, iCtx8=(x.bpos<<16)|(buffer(1)<<8)|(expectedByte^B);
      Maps[2].set(iCtx8());
      Maps[3].set((x.bpos<<16)|(iCtx16()^U32(B|(B<<8))));
    }

    // check if next bit matches the prediction, accounting for the required bitmask
    if (length>0 && (((expectedByte^B)&sparse[hashIndex].bitMask)>>(8-x.bpos))!=0)
      length = 0;

    if (valid) {
      if (length>1 && ((sparse[hashIndex].bitMask>>(7-x.bpos))&1)>0) {
        const int expectedBit = (expectedByte>>(7-x.bpos))&1;
        const int sign = 2*expectedBit-1;
        m.add(sign*(min(length-1, 64)<<4)); // +/- 16..1024
        m.add(sign*(1<<min(length-2, 3))*min(length-1, 8)<<4); // +/- 16..1024
        m.add(sign*512);
      }
      else {
        m.add(0); m.add(0); m.add(0);
      }

      for (int i=0;i<4;i++)
        Maps[i].mix(m, 1, 2);

    }
    else
      for (int i=0; i<11; i++, m.add(0));
    m.set((hashIndex<<6)|(x.bpos<<3)|min(7, length), NumHashes*64); //256
    //m.set((hashIndex<<11)|(min(7, ilog2(length+1))<<8)|(x.c0^(expectedByte>>(8-x.bpos))), NumHashes*2048); //8192
    return length;
  }
};*/
//////////////////////////// matchModel ///////////////////////////
class MatchContext   {
private:
    BlockData& x;
    Buf& buffer;
    const U64 Size;
  enum Parameters : U32{
      MaxLen = 0xFFFF, // longest allowed match
    MaxExtend = 0,   // longest match expansion allowed // warning: larger value -> slowdown
    MinLen = 5,      // minimum required match length
    StepSize = 2,    // additional minimum length increase per higher order hash
    DeltaLen = 5,    // minimum length to switch to delta mode
    NumCtxs = 3,     // number of contexts used
    NumHashes = 3    // number of hashes used

  };
  Array<U32> Table;
  StateMap StateMaps[NumCtxs];
  SmallStationaryContextMap SCM[3];
  StationaryMap Maps[3];
  IndirectContext<U8> iCtx;
  U32 hashes[NumHashes];
  U32 ctx[NumCtxs];
  U32 length;    // rebased length of match (length=1 represents the smallest accepted match length), or 0 if no match
  U32 index;     // points to next byte of match in buffer, 0 when there is no match
  U32 mask;
  U8 expectedByte; // prediction is based on this byte (buffer[index]), valid only when length>0
  bool delta;
  int result;
  void Update() {
        delta = false;
    if (length==0 && Bypass)
      Bypass = false; // can quit bypass mode on byte boundary only
    // update hashes
    for (U32 i=0, minLen=MinLen+(NumHashes-1)*StepSize; i<NumHashes; i++, minLen-=StepSize) {
      hashes[i] = hash(minLen);
      for (U32 j=1; j<=minLen; j++)
        hashes[i] = combine(hashes[i], buffer(j)<<i);
      hashes[i]&=mask;
    }
    // extend current match, if available
    if (length) {
      index++;
      if (length<MaxLen)
        length++;
    }
    // or find a new match, starting with the highest order hash and falling back to lower ones
    else {
      U32 minLen = MinLen+(NumHashes-1)*StepSize, bestLen = 0, bestIndex = 0;
      for (U32 i=0; i<NumHashes && length<minLen; i++, minLen-=StepSize) {
        index = Table[hashes[i]];
        if (index>0) {
          length = 0;
          while (length<(minLen+MaxExtend) && buffer(length+1)==buffer[index-length-1])
            length++;
          if (length>bestLen) {
            bestLen = length;
            bestIndex = index;
          }
        }
      }
      if (bestLen>=minLen) {
        length = bestLen-(MinLen-1); // rebase, a length of 1 actually means a length of MinLen
        index = bestIndex;
      }
      else
        length = index = 0;
    }
    // update position information in hashtable
    for (U32 i=0; i<NumHashes; i++)
      Table[hashes[i]] = x.buf.pos;
      expectedByte = buffer[index];
    iCtx+=x.y, iCtx=(buffer(1)<<8)|expectedByte;
    SCM[0].set(expectedByte);
    SCM[1].set(expectedByte);
    SCM[2].set(x.buf.pos);
    Maps[0].set((expectedByte<<8)|buffer(1));
    Maps[1].set(hash(expectedByte, x.c0, buffer(1), buffer(2), min(3,(int)ilog2(length+1))));
    Maps[2].set(iCtx());
    //x.Match.byte = (length)?expectedByte:0;
  }
public:
  bool canBypass;
  bool Bypass;
  U16 BypassPrediction; // prediction for bypass mode
  virtual ~MatchContext(){ }
  int inputs() {return  2+NumCtxs+3*2+3*2+2;}
  MatchContext(BlockData& bd, int m, U32 val1=0) :
    x(bd),buffer(bd.buf),Size( m),
    Table(Size/sizeof(U32)),
    StateMaps{56*256, 8*256*256+1, 256*256 },
    SCM{ {8,8},{11,1},{8,8} },
    Maps{ {16}, {22,1}, {4,1} },
    iCtx{19,1},
    hashes{ 0 },
    ctx{ 0 },
    length(0),
    mask(Size/sizeof(U32)-1),
    expectedByte(0),
    delta(false),
    canBypass(true),
    Bypass(false),
    BypassPrediction(2048),
    result(0)
  {
    assert((Size&(Size-1))==0);
  }
  int get(){return result;}
  void mix(Mixer& m) {
    if (x.bpos==0)
      Update();
   else {
      U8 B = x.c0<<(8-x.bpos);
      SCM[1].set((x.bpos<<8)|(expectedByte^B));
      Maps[1].set(hash(expectedByte, x.c0, buffer(1), buffer(2), min(3,(int)ilog2(length+1))));
      iCtx+=x.y, iCtx=(x.bpos<<16)|(buffer(1)<<8)|(expectedByte^B);
      Maps[2].set(iCtx());
    }
    const int expectedBit = (expectedByte>>(7-x.bpos))&1;

    if(length>0) {
      const bool isMatch = x.bpos==0 ? buffer(1)==buffer[index-1] : ((expectedByte+256)>>(8-x.bpos))==x.c0; // next bit matches the prediction?
      if(!isMatch) {
        delta = (length+MinLen)>DeltaLen;
        length = 0;
      }
    }

    if (!(canBypass && Bypass)) {
      for (U32 i=0; i<NumCtxs; i++)
        ctx[i] = 0;
      if (length>0) {
          if (length<=16)
            ctx[0] = (length-1)*2 + expectedBit; // 0..31
          else
            ctx[0] = 24 + (min(length-1, 63)>>2)*2 + expectedBit; // 32..55
          ctx[0] = ((ctx[0]<<8) | x.c0);
          ctx[1] = ((expectedByte<<11) | (x.bpos<<8) | buffer(1)) + 1;
          const int sign = 2*expectedBit-1;
          m.add(sign*(min(length,32)<<5)); // +/- 32..1024
          m.add(sign*(ilog(length)<<2));   // +/-  0..1024
        }
        else { // no match at all or delta mode
          m.add(0);
          m.add(0);
        }

      if (delta)
        ctx[2] = (expectedByte<<8) | x.c0;

      for (U32 i=0; i<NumCtxs; i++) {
        const U32 c = ctx[i];
        const U32 p = StateMaps[i].p(c,x.y);
        if (c!=0)
          m.add((stretch(p)+1)>>1);
        else
          m.add(0);
      }

      SCM[0].mix(m);
      SCM[1].mix(m, 6);
      SCM[2].mix(m, 5);
      Maps[0].mix(m, 1, 4, 255);
      Maps[1].mix(m);
      Maps[2].mix(m);
    }
    BypassPrediction = length==0 ? 2048 : (expectedBit==0 ? 1 : 4095);
   // x.Match.length = length;
    result=ilog(length);
  }
  };
  
  

//////////////////////////// wordModel /////////////////////////
#define bswap(x) \
+   ((((x) & 0xff000000) >> 24) | \
+    (((x) & 0x00ff0000) >>  8) | \
+    (((x) & 0x0000ff00) <<  8) | \
+    (((x) & 0x000000ff) << 24))
//#define SPACE 0x20

//////////////////////////// Predictor /////////////////////////
// A Predictor estimates the probability that the next bit of
// uncompressed data is 1.  Methods:
// p() returns P(1) as a 12 bit number (0-4095).
// update(y) trains the predictor with the actual bit (0 or 1).
#include "vm.cpp"
//base class
class Predictors {
public:
  BlockData x; //maintains current global data block between models
  //list of all models
 
virtual ~Predictors(){ };
Predictors(){
 
}
  virtual int p() const =0;
  virtual void update()=0;
  virtual void set()=0;
  virtual void setdebug(int a)=0;
  void update0(){
    // Update global context: pos, bpos, c0, c4, buf
    x.c0+=x.c0+x.y;
    if (x.c0>=256) {
        x.c4=(x.c4<<8)+(x.c0&0xff);
        x.buf[x.buf.pos++]=x.c0;
        x.c0=1;
        ++x.blpos;
        x.buf.pos=x.buf.pos&x.buf.poswr; //wrap
    }
    x.bpos=(x.bpos+1)&7;
  }
};

//general predicor class
class Predictor: public Predictors {
  int pr;
  VM vm;
public:  
  Predictor(char *m): pr(2048),vm(m,x) {setdebug(0);}
  int p()  const {assert(pr>=0 && pr<4096); return pr;} 
  ~Predictor(){ vm.killvm( );}
  void set() {  vm.block(x.finfo,0);  }
  void setdebug(int a){      vm.debug=a;  }
  void update()  {
    update0();
    pr=vm.doupdate(x.y,x.c0,x.bpos,x.c4,x.buf.pos);
  }
};
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
    archive->put32(x1);  // Flush first unequal byte of range
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
#define UUU_DET(type,start_pos,header_len,base64len,is96) return dett=(type),\
deth=(-1),detd=int(base64len),info=(is96),\
 in->setpos(start+start_pos),DEFAULT
 
#define B85_DET(type,start_pos,header_len,base85len) return dett=(type),\
deth=(-1),detd=int(base85len),\
 in->setpos(start+start_pos),DEFAULT

#define SZ_DET(type,start_pos,header_len,base64len,unsize) return dett=(type),\
deth=(-1),detd=int(base64len),info=(unsize),\
 in->setpos(start+start_pos),DEFAULT

#define MRBRLE_DET(type,start_pos,header_len,data_len,width,height) return dett=(type),\
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
/*
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
}*/


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
      order = 1-2*(b>int(ilog2(n)/4));
      continue;
    }

    //"j" is the index of the current byte in this color entry
    int j = i%stride;
    if (!j){
      // load first component of this entry
      int k = (b-(res&0xFF))*order;
      res = res&((k>=0 && k<=8)<<8);
      res|=(res)?b:0;
    }
    else if (j==3)
      res&=((!b || (b==0xFF))*0x1FF); // alpha/attribute component must be zero or 0xFF
    else
      res&=((b==(res&0xFF))*0x1FF);
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

#define MIN_TEXT_SIZE 0x400 //1KB
#define MAX_TEXT_MISSES 3 //number of misses in last 32 bytes before resetting
struct TextInfo {
  U64 start;
  U32 lastSpace;
  U32 lastNL;
  U64 lastNLpos;
  U32 wordLength;
  U32 misses;
  U32 missCount;
  U32 zeroRun;
  U32 spaceRun;
  U32 countNL;
  U32 totalNL;
  U32 countLetters;
  U32 countNumbers;
  U32 countUTF8;
  bool isLetter, isUTF8, needsEolTransform, seenNL, isNumbertext;
};
struct bmpInfo {
U64 bmp;
  int bpp,
  x,
  y,
  of,
  size,
  hdrless; 
};  
struct gifInfo {
U64 gif,
    a,
  i,
  w,
  c,
  b,
  plt,
  gray  ; 
};  

Filetype detect(File* in, U64 n, Filetype type, int &info, int &info2, int it=0,int s1=0) {
  U32 buf4=0,buf3=0, buf2=0, buf1=0, buf0=0;  // last 8 bytes
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
  bmpInfo bmp = {0};    // For BMP detection
  U64 rgbi=0;
  int rgbx=0,rgby=0;  // For RGB detection
  U64 tga=0;
  U64 tgax=0;
  int tgay=0,tgaz=0,tgat=0,tgaid=0,tgamap=0;  // For TGA detection
  U64 pgm=0;
  int pgmcomment=0,pgmw=0,pgmh=0,pgm_ptr=0,pgmc=0,pgmn=0,pamatr=0,pamd=0;  // For PBM, PGM, PPM, PAM detection
  char pgm_buf[32];
  U64 cdi=0;
  U64 mdfa=0;
  int cda=0,cdm=0;   // For CD sectors detection
  U32 cdf=0;
  TextInfo text = {0}; // For TEXT
 ///
   U64 uuds=0,uuds1=0,uudp=0,uudslen=0,uudh=0;//,b64i=0;
  U64 uudstart=0,uudend=0,uudline=0,uudnl=0,uudlcount=0,uuc=0;
  //base64
  U64 b64s=0,b64s1=0,b64p=0,b64slen=0,b64h=0;//,b64i=0;
  U64 base64start=0,base64end=0,b64line=0,b64nl=0,b64lcount=0;
  //base85
  U64 b85s=0,b85s1=0,b85p=0,b85slen=0,b85h=0;
  U64 base85start=0,base85end=0,b85line=0;
  //U64 gif=0,gifa=0,gifi=0,gifw=0,gifc=0,gifb=0,gifplt=0,gifgray=0; // For GIF detection
  gifInfo gif = {0};
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
  U32 mrbmulti=0;
  //
  U64 tar=0,tarn=0,tarl=0,utar=0;
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
  static Array<U32> tfidf(0);
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
    buf4=buf4<<8|buf3>>24;
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
    // ustar header detection
    if (((buf0)==0x61722020 || (buf0&0xffffff00)==0x61720000) && (buf1&0xffffff)==0x757374 && tar==0&&tarl==0) tar=i,tarn=0,tarl=1,utar=263;
    // brute force detection on recursion level 0, at the block start only
    if(tarl==0 && it==0 && i==512 && start==0){
        U64 tarsave= in->curpos();
        in->setpos( tarsave-513);
        int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
            if (tarchecksum((char*)&tarh)){
                tar=i,tarn=512,tarl=2,utar=0;
                tar=in->curpos();
               int a=getoct(tarh.size,12);
               int b=a-(a/512)*512;
               if (b) tarn=tarn+512*2+(a/512)*512;
               else if (a==0) tarn=tarn+512;
               else tarn=tarn+512+(a/512)*512;
               tarn=tarn+int(i-tar+utar);
            } else  in->setpos(tarsave);
    }
    if (tarl) {
        const int p=int(i-tar+utar);        
        if (p==512 && tarn==0 && tarl==1) {
            U64 savedpos= in->curpos();
             in->setpos( savedpos-513);
             int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
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
            int bin=in->blockread((U8*) &tarh,  sizeof(tarh) );
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
        continue;
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
 /*  histogram[c]++;
    if (i>=256)
      histogram[zbuf[zbufpos]]--;
    zbuf[zbufpos] = c;
    if (zbufpos<32)
      zbuf[zbufpos+256] = c;
    zbufpos=(zbufpos+1)&0xFF;
    if(!cdi && !mdfa && type!=MDF && b85s==0)  {
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
}*/
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
      else if (p==17) {
          //Field Descriptor terminator 
          U64 savedpos = in->curpos();
          in->setpos(savedpos+dbase.HeaderLength-19);
          U8 marker=in->getc();
          if (marker!=0xd) dbasei=0,in->setpos(savedpos); 
          else{
            dbase.Start = 0;//dbase.HeaderLength;
            dbase.End =  dbase.Start + dbase.nRecords * dbase.RecordLength;
            U32 seekpos = dbase.End+in->curpos();
            in->setpos(seekpos);
            // get file end marker, fail if not present
            marker=in->getc();
            if (marker!=0x1a) dbasei=0, in->setpos(savedpos);
            else{
               in->setpos(savedpos);
               DBS_DET(DBASE,dbasei- 1,dbase.HeaderLength, dbase.nRecords * dbase.RecordLength+1,dbase.RecordLength); 
            }
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
    if (type==JPEG) continue;

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
   
    //detect rle encoded mrb files inside windows hlp files 506C
    if (!mrb && ((buf0&0xFFFF)==0x6c70 || (buf0&0xFFFF)==0x6C50) && !b64s1 && !b64s && !b85s1 && !b85s && type!=MDF &&  !cdi)
        mrb=i,mrbsize=0,mrbPictureType=mrbmulti=0; 
    if (mrb){
        U32 BitCount=0;
        const int p=int(i-mrb)-mrbmulti*4; //select only first image from multiple
        if (p==1 && c>1 && c<4&& mrbmulti==0)    mrbmulti=c-1;
        if (p==1 && c==0) mrb=0;
        if (p==7 ){  // 5=DDB   6=DIB   8=metafile
            if ((c==5 || c==6 )) mrbPictureType=c;
            else mrb=0;
         }
        if (p==8) {         // 0=uncomp 1=RunLen 2=LZ77 3=both
           if(c==1||c==2||c==0) mrbPackingMethod=c;
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
         BitCount=GetCWord(in);
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
        if (!(BitCount==8 || BitCount==4)|| mrbw<4 || mrbw>1024 || mrbPackingMethod==2|| mrbPackingMethod==0) {
            mrbPictureType=mrb=mrbsize=mrbmulti=0;
            mrbTell=mrbTell+2;
             in->setpos(mrbTell);
        }
       } else mrbPictureType=mrb=mrbsize=0;;
       }
       if ((type==MRBR || type==MRBR4 ) &&   (mrbPictureType==6 || mrbPictureType==8) && mrbsize){
        return  in->setpos(start+mrbsize),DEFAULT;
       }
       if ( (mrbPictureType==6 && BitCount==8) && mrbsize && mrbw>4 && mrbh>4){
        MRBRLE_DET(MRBR,mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
       }
       else if ( (mrbPictureType==6 && BitCount==4) && mrbsize && mrbw>4 && mrbh>4){
        MRBRLE_DET(MRBR4,mrb-1, mrbsize-mrbcsize, mrbcsize, mrbw, mrbh);
       }
    }
    // Detect .bmp image
    if ( !(bmp.bmp || bmp.hdrless) && (((buf0&0xffff)==16973) || (!(buf0&0xFFFFFF) && ((buf0>>24)==0x28))) ) //possible 'BM' or headerless DIB
      bmp = {0},bmp.hdrless=!(U8)buf0,bmp.of=bmp.hdrless*54,bmp.bmp=i-bmp.hdrless*16;
    if (bmp.bmp || bmp.hdrless) {
      const int p=i-bmp.bmp;
      if (p==12) bmp.of=bswap(buf0);
      else if (p==16 && buf0!=0x28000000) bmp = {0}; //BITMAPINFOHEADER (0x28)
      else if (p==20) bmp.x=bswap(buf0),bmp.bmp=((bmp.x==0||bmp.x>0x30000)?(bmp.hdrless=0):bmp.bmp); //width
      else if (p==24) bmp.y=abs((int)bswap(buf0)),bmp.bmp=((bmp.y==0||bmp.y>0x10000)?(bmp.hdrless=0):bmp.bmp); //height
      else if (p==27) bmp.bpp=c,bmp.bmp=((bmp.bpp!=1 && bmp.bpp!=4 && bmp.bpp!=8 && bmp.bpp!=24 && bmp.bpp!=32)?(bmp.hdrless=0):bmp.bmp);
      else if ((p==31) && buf0) bmp = {0};
      else if (p==36) bmp.size=bswap(buf0);
      // check number of colors in palette (4 bytes), must be 0 (default) or <= 1<<bpp.
      // also check if image is too small, since it might not be worth it to use the image models
      else if (p==48){
        if ( (!buf0 || ((bswap(buf0)<=(U32)(1<<bmp.bpp)) && (bmp.bpp<=8))) && (((bmp.x*bmp.y*bmp.bpp)>>3)>64) ) {
          // possible icon/cursor?
          if (bmp.hdrless && (bmp.x*2==bmp.y) && bmp.bpp>1 &&
             (
              (bmp.size>0 && bmp.size==( (bmp.x*bmp.y*(bmp.bpp+1))>>4 )) ||
              ((!bmp.size || bmp.size<((bmp.x*bmp.y*bmp.bpp)>>3)) && (
               (bmp.x==8)   || (bmp.x==10) || (bmp.x==14) || (bmp.x==16) || (bmp.x==20) ||
               (bmp.x==22)  || (bmp.x==24) || (bmp.x==32) || (bmp.x==40) || (bmp.x==48) ||
               (bmp.x==60)  || (bmp.x==64) || (bmp.x==72) || (bmp.x==80) || (bmp.x==96) ||
               (bmp.x==128) || (bmp.x==256)
              ))
             )
          )
            bmp.y=bmp.x;

          // if DIB and not 24bpp, we must calculate the data offset based on BPP or num. of entries in color palette
          if (bmp.hdrless && (bmp.bpp<24))
            bmp.of+=((buf0)?bswap(buf0)*4:4<<bmp.bpp);
          bmp.of+=(bmp.bmp-1)*(bmp.bmp<1);

          if (bmp.hdrless && bmp.size && bmp.size<((bmp.x*bmp.y*bmp.bpp)>>3)) { }//Guard against erroneous DIB detections
          else if (bmp.bpp==1) IMG_DET(IMAGE1,max(0,bmp.bmp-1),bmp.of,(((bmp.x-1)>>5)+1)*4,bmp.y);
          else if (bmp.bpp==4) IMG_DET(IMAGE4,max(0,bmp.bmp-1),bmp.of,((bmp.x*4+31)>>5)*4,bmp.y);
          else if (bmp.bpp==8){
             in->setpos(start+bmp.bmp+53);
            IMG_DET( (IsGrayscalePalette(in, (buf0)?bswap(buf0):1<<bmp.bpp, 1))?IMAGE8GRAY:IMAGE8,max(0,bmp.bmp-1),bmp.of,(bmp.x+3)&-4,bmp.y);
          }
          else if (bmp.bpp==24) IMG_DET(IMAGE24,max(0,bmp.bmp-1),bmp.of,((bmp.x*3)+3)&-4,bmp.y);
          else if (bmp.bpp==32) IMG_DET(IMAGE32,max(0,bmp.bmp-1),bmp.of,bmp.x*4,bmp.y);
        }
        bmp = {0};
      }
    }
    // Detect .pbm .pgm .ppm .pam image
    if ((buf0&0xfff0ff)==0x50300a && (i-text.start-1)<MIN_TEXT_SIZE ) { 
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
   if ( (((buf1==0x49492a00 ||buf1==0x4949524f ) && n>i+(int)bswap(buf0) && tiffImages==-1)|| 
       ((buf1==0x4d4d002a  ) && n>i+(int)(buf0) && tiffImages==-1)) && !soi){
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
        dirsize=tiffMM==true?(in->getc()<<8|in->getc()):(in->getc()|(in->getc()<<8));
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
       tiffFiles[o].offset+=tiffImageStart;
       }
       //
      if (tifx>1 && tify && tifzb && (tifz==1 || tifz==3) && ((tifc==1) || (tifc==5/*LZW*/ && tifsiz>0)) && (tifofs && tifofs+i<n)) {
        if (!tifofval) {
           in->setpos( start+i+tifofs-7);
          for (int j=0; j<4; j++) b[j]=in->getc();
          tifofs=b[0]+(b[1]<<8)+(b[2]<<16)+(b[3]<<24);
        }
        if (tifofs && tifofs<(1<<18) && tifofs+i<n && tifx>1) {
            if (tifc==1) {
          if (tifz==1 && tifzb==1) IMG_DET(IMAGE1,i-7,tifofs,((tifx-1)>>3)+1,tify);
          else if (tifz==1 && tifzb==8 && tifx<30000) IMG_DET(IMAGE8,i-7,tifofs,tifx,tify);
          else if (tifz==3 && tifzb==8 && tifx<30000) IMG_DET(IMAGE24,i-7,tifofs,tifx*3,tify);
        }
        else if (tifc==5 && tifsiz>0) {
            tifx=((tifx+8-tifzb)/(9-tifzb))*tifz;
            info=tifz*tifzb;
            info=(((info==1)?IMAGE1:((info==8)?IMAGE8:IMAGE24))<<24)|tifx;
            detd=tifsiz;
            in->setpos(start+i-7+tifofs);
            return dett=LZW;
          }
        }
      }
      in->setpos( savedpos);
    }
       
    // Detect .tga image (8-bit 256 colors or 24-bit uncompressed)
    if ((buf1&0xFFF7FF)==0x00010100 && (buf0&0xFFFFFFC7)==0x00000100 && (c==16 || c==24 || c==32)) tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF,tgaid=buf1>>24,tgamap=c/8;
    else if ((buf1&0xFFFFFF)==0x00000200 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=24,tgat=2;
    else if ((buf1&0xFFF7FF)==0x00000300 && buf0==0x00000000) tga=i,tgax=tgay,tgaz=8,tgat=(buf1>>8)&0xF;
    if (tga) {
      if (i-tga==8) tga=(buf1==0?tga:0),tgax=(bswap(buf0)&0xffff),tgay=(bswap(buf0)>>16);
      else if (i-tga==10) {
          if ((buf0&0xFFF7)==32<<8)
          tgaz=32;
        if ((tgaz<<8)==(int)(buf0&0xFFD7) && tgax && tgay && U32(tgax*tgay)<0xFFFFFFF) {
          if (tgat==1){
            in->setpos(start+tga+11+tgaid);
            IMG_DET( (IsGrayscalePalette(in))?IMAGE8GRAY:IMAGE8,tga-7,18+tgaid+256*tgamap,tgax,tgay);
          }
          else if (tgat==2) IMG_DET((tgaz==24)?IMAGE24:IMAGE32,tga-7,18+tgaid,tgax*(tgaz>>3),tgay);
          else if (tgat==3) IMG_DET(IMAGE8GRAY,tga-7,18+tgaid,tgax,tgay);
          else if (tgat==9 || tgat==11) {
              const U64 savedpos=in->curpos();
            in->setpos(start+tga+11+tgaid);
            if (tgat==9) {
              info=(IsGrayscalePalette(in)?IMAGE8GRAY:IMAGE8)<<24;
              in->setpos(start+tga+11+tgaid+256*tgamap);
            }
            else
              info=IMAGE8GRAY<<24;
            info|=tgax;
            // now detect compressed image data size
            detd=0;
            int c=in->getc(), b=0, total=tgax*tgay, line=0;
            while (total>0 && c>=0 && (++detd, b=in->getc())>=0){
              if (c==0x80) { c=b; continue; }
              else if (c>0x7F) {
                total-=(c=(c&0x7F)+1); line+=c;
                c=in->getc();
                detd++;
              }
              else {
                in->setpos(in->curpos()+c); 
                detd+=++c; total-=c; line+=c;
                c=in->getc();
              }
              if (line>tgax) break;
              else if (line==tgax) line=0;
            }
            if (total==0) {
              in->setpos(start+tga+11+tgaid+256*tgamap);
              return dett=RLE;
            }
            else
              in->setpos(savedpos);
          }
        }
        tga=0;
      }
    }
    // Detect .gif
    if (type==DEFAULT && dett==GIF && i==0) {
      dett=DEFAULT;
      if (c==0x2c || c==0x21) gif.gif=2,gif.i=2;
      else gif.gray=0;
    }
    if (!gif.gif && (buf1&0xffff)==0x4749 && (buf0==0x46383961 || buf0==0x46383761)) gif.gif=1,gif.i=i+5;
    if (gif.gif) {
      if (gif.gif==1 && i==gif.i) gif.gif=2,gif.i = i+5+(gif.plt=(c&128)?(3*(2<<(c&7))):0),brute=false;
      if (gif.gif==2 && gif.plt && i==gif.i-gif.plt-3) gif.gray = IsGrayscalePalette(in, gif.plt/3), gif.plt = 0;
      if (gif.gif==2 && i==gif.i) {
        if ((buf0&0xff0000)==0x210000) gif.gif=5,gif.i=i;
        else if ((buf0&0xff0000)==0x2c0000) gif.gif=3,gif.i=i;
        else gif.gif=0;
      }
      if (gif.gif==3 && i==gif.i+6) gif.w=(bswap(buf0)&0xffff);
      if (gif.gif==3 && i==gif.i+7) gif.gif=4,gif.c=gif.b=0,gif.a=gif.i=i+2+(gif.plt=((c&128)?(3*(2<<(c&7))):0));
      if (gif.gif==4 && gif.plt) gif.gray = IsGrayscalePalette(in, gif.plt/3), gif.plt = 0;
      if (gif.gif==4 && i==gif.i) {
        if (c>0 && gif.b && gif.c!=gif.b) gif.w=0;
        if (c>0) gif.b=gif.c,gif.c=c,gif.i+=c+1;
        else if (!gif.w) gif.gif=2,gif.i=i+3;
        else return  in->setpos( start+gif.a-1),detd=i-gif.a+2,info=((gif.gray?IMAGE8GRAY:IMAGE8)<<24)|gif.w,dett=GIF;
      }
      if (gif.gif==5 && i==gif.i) {
        if (c>0) gif.i+=c+1; else gif.gif=2,gif.i=i+3;
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
    if (type==EXE) continue;

    // DEC Alpha
    op=bswap(buf0)>>21; 
    //test if bsr opcode and if last 3 opcodes are valid
    if ((op==0x34*32+26) && CAlpha2(bswap(buf1))==true && CAlpha2(bswap(buf2))==true && CAlpha2(bswap(buf3))==true && CAlpha2(bswap(buf4))==true && e8e9count==0 &&
     !tar && !soi && !pgm && !rgbi && !bmp.bmp && !wavi && !tga) {
      int a=op&0xff;// absolute address low 8 bits
      int r=op&0x1fffff;
      r+=(i)/4;  // relative address low 8 bits
      r=r&0xff;
      int rdist=int(i-relposDEC[r]);
      int adist=int(i-absposDEC[a]);
      if (adist<rdist && adist<0x8000 && absposDEC[a]>16 &&  adist>16 && adist%4==0) {
        DEClast=i;
        ++DECcount;
        if (DECpos==0 || DECpos>absposDEC[a]) DECpos=absposDEC[a];
      }
      else DECcount=0;
      if (type==DEFAULT && DECcount>=16 && DECpos>8 ){
          return in->setpos(start+DECpos-(start+DECpos)%4), DECA;
           }
      absposDEC[a]=i;
      relposDEC[r]=i;
    }
    if (i-DEClast>0x4000) {
      if (type==DECA)       
      return  in->setpos( start+DEClast-(start+DEClast)%4), DEFAULT;
      DECcount=0,DECpos=0,DEClast=0;
      memset(&relposDEC[0], 0, sizeof(relposDEC));
      memset(&absposDEC[0], 0, sizeof(absposDEC));
    }
    if (type==DECA) continue;
    
    // ARM
    op=(buf0)>>26; 
    //test if bl opcode and if last 3 opcodes are valid 
    // BL(4) and (ADD(1) or MOV(4)) as previous, 64 bit
    // ARMv8-A_Architecture_Reference_Manual_(Issue_A.a).pdf
    if (op==0x25 && /*DECcount==0 &&*/
    (((buf1)>>26==0x25 ||(buf2)>>26==0x25/*||(buf3)>>26==0x25*/ ) ||
    (( ((buf1)>>24)&0x7F==0x11 || ((buf1)>>23)&0x7F==0x25  || ((buf1)>>23)&0x7F==0xa5 || ((buf1)>>23)&0x7F==0x64 || ((buf1)>>24)&0x7F==0x2A) )
    )&& e8e9count==0 &&  !tar && !soi && !pgm && !rgbi && !bmp.bmp && !wavi && !tga && (buf1)>>31==1&& (buf2)>>31==1&& (buf3)>>31==1&& (buf4)>>31==1){ 
      int a=(buf0)&0xff;// absolute address low 8 bits
      int r=(buf0)&0x3FFFFFF;
      r+=(i)/4;  // relative address low 8 bits
      r=r&0xff;
      int rdist=int(i-relposARM[r]);
      int adist=int(i-absposARM[a]);
      if (adist<rdist && adist<0x3FFFFF && absposARM[a]>16 &&  adist>16 && adist%4==0) {
        ARMlast=i;
        ++ARMcount;
        if (ARMpos==0 || ARMpos>absposARM[a]) ARMpos=absposARM[a];
      }
      else ARMcount=0;
      if (type==DEFAULT && ARMcount>=18 && ARMpos>16 ) 
          return in->setpos(start+ARMpos-ARMpos%4), ARM;
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
    
    // detect uuencoode in eml 
    // only 61 byte linesize and, ignore with trailin 1 byte lines.
    // 0A424547 494E2D2D 63757420
   if (uuds==0 && ((buf0==0x67696E20 && (buf1&0xffffff)==0x0A6265) ||
      ( buf2==0x0A424547&& buf1==0x494E2D2D&& buf0==0x63757420) )) uuds=1,uudp=i-8,uudh=0,uudslen=0,uudlcount=0; //'\n begin ' '\nBEGIN--cut '
    else if (uuds==1 && (buf0&0xffff)==0x0A4D ) {
        uuds=2,uudh=i,uudslen=uudh-uudp;
        uudstart=i;
        if (uudslen>40) uuds=0; //drop if header is larger 
        }
    else if (uuds==1 && (buf0&0xffff)==0x0A62 ) uuds=0; //reset for begin
    else if (uuds==2 && (buf0&0xff)==0x0A && uudline==0) {
         uudline=i-uudstart,uudnl=i;      //capture line lenght
         if (uudline!=61) uuds=uudline=0; //drop if not
    }
    else if (uuds==2 &&( (buf0&0xff)==0x0A || (buf0==0x454E442D && (buf1&0xff)==0x0A))  && uudline!=0){// lf or END-
         if ( (i-uudnl+1<uudline && (buf0&0xffff)!=0x0A0A) ||  ((buf0&0xffff)==0x0A0A) ) { // if smaller and not padding
            uudend=i-1;
            if ( (((uudend-uudstart)>128) && ((uudend-uudstart)<512*1024))  ){
             uuds=0;
             UUU_DET(UUENC,uudh,uudslen,uudend -uudstart,uuc);
            }
         }
         else if(buf0==0x454E442D){ // 'END-'
             uudend=i-5;
              UUU_DET(UUENC,uudh,uudslen,uudend -uudstart,uuc);
         }
         uudnl=i+2; //update 0x0D0A pos
         uudlcount++;
         }
    else if (uuds==2 && (c>=32 && c<=96)) {if (uuc==0 && c==96) uuc=1;} // some files use char 96, set for info;
    else if (uuds==2)   uuds=0;
    
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
         //continue;
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
        base85start=i;//+1;
        if (b85slen>128) b85s=0; //drop if header is larger 
        //txtStart=0;
        }
    else if (b85s==2){
        if  ((buf0&0xff)==0x0d && b85line==0) {
            b85line=i-base85start;//,b85nl=i+2;//capture line lenght
            if (b85line<=4 || b85line>255) b85s=0;
        }
        
        else if ( (buf0&0xff)==0x7E)  { //if padding '~' or '=='
            base85end=i-1;
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
    if (b85s==2)continue;
    
    // Detect text, utf-8, eoltext and text0
    text.isLetter = tolower(c)!=toupper(c);
    text.countLetters+=(text.isLetter)?1:0;
    text.countNumbers+=(c>='0' && c<='9') ?1:0;
    text.isNumbertext=text.countLetters< text.countNumbers;
    text.isUTF8 = ((c!=0xC0 && c!=0xC1 && c<0xF5) && (
        (c<0x80) ||
        // possible 1st byte of UTF8 sequence
        ((buf0&0xC000)!=0xC000 && ((c&0xE0)==0xC0 || (c&0xF0)==0xE0 || (c&0xF8)==0xF0)) ||
        // possible 2nd byte of UTF8 sequence
        ((buf0&0xE0C0)==0xC080 && (buf0&0xFE00)!=0xC000) || (buf0&0xF0C0)==0xE080 || ((buf0&0xF8C0)==0xF080 && ((buf0>>8)&0xFF)<0xF5) ||
        // possible 3rd byte of UTF8 sequence
        (buf0&0xF0C0C0)==0xE08080 || ((buf0&0xF8C0C0)==0xF08080 && ((buf0>>16)&0xFF)<0xF5) ||
        // possible 4th byte of UTF8 sequence
        ((buf0&0xF8C0C0C0)==0xF0808080 && (buf0>>24)<0xF5)
    ));
     text.countUTF8+=((text.isUTF8 && !text.isLetter && (c>=0x80))?1:0);
    if (text.lastNLpos==0 && c==NEW_LINE ) text.lastNLpos=i;
    else if (text.lastNLpos>0 && c==NEW_LINE ) {
        int tNL=i-text.lastNLpos;
        if (tNL<90 && tNL>45) 
            text.countNL++;          //Count if in range   
        else 
            text.totalNL+=tNL>3?1:0; //Total new line count
        text.lastNLpos=i;
    }
    text.lastNL = (c==NEW_LINE || c==CARRIAGE_RETURN ||c==10|| c==5)?0:text.lastNL+1;
    if (c==SPACE || c==TAB ||c==0x12 ){
      text.lastSpace = 0;
      text.spaceRun++;
    }
    else{
      text.lastSpace++;
      text.spaceRun = 0;
    }
    text.wordLength = (text.isLetter)?text.wordLength+1:0;
    text.missCount-=text.misses>>31;
    text.misses<<=1;
    text.zeroRun=(!c && text.zeroRun<32)?text.zeroRun+1:0;
    //if (c==NEW_LINE || c==5){
      //if (!text.seenNL)
       // text.needsEolTransform = true;
    //  text.seenNL = true;
      //text.needsEolTransform&=(text.countNL>text.totalNL);//U8(buf0>>8)==CARRIAGE_RETURN;
    //}
    bool tspace=(c<SPACE && c!=TAB && (text.zeroRun<2 || text.zeroRun>8) && text.lastNL!=0);
    //bool tcr=((buf0&0xFF00)==(CARRIAGE_RETURN<<8) || (buf0&0xFF00)==(10<<8));
    bool tscr= (text.spaceRun>8 && text.lastNL>256 && !text.isUTF8); // utf8 line lenght can be more then 4 times longer
    bool tword=(!text.isLetter && !text.isUTF8 &&  ( text.lastNL>256 || text.lastSpace > max( text.lastNL, text.wordLength*8) || text.wordLength>32) );
    if (tspace || 
       // tcr||
        tscr||
        tword
     ) {
        text.misses|=1;
        text.missCount++;
        int length = i-text.start-1; 
        bool dtype=(png || pdfimw || cdi || soi || pgm || rgbi || tga || gif.gif || b64s||tar || bmp.bmp ||wavi ||b64s1 ||b85s1||b85s||DECcount||mrb||uuds );
        if (((length<MIN_TEXT_SIZE && text.missCount>MAX_TEXT_MISSES) || dtype)){
          text = {0};
          text.start = i+1;
        }
        else if (text.missCount>MAX_TEXT_MISSES ) {
          text.needsEolTransform=(text.countNL>text.totalNL);
          if (text.isNumbertext)     info=1;
          in->setpos(start + text.start);
          detd = length;
          return (text.needsEolTransform)?EOLTEXT:(( text.isNumbertext)?TEXT0:(text.countUTF8>MIN_TEXT_SIZE?TXTUTF8:TEXT));
        }
    }
    //disable zlib brute if text lenght is over minimum.
    if ( (i-text.start)>MIN_TEXT_SIZE) brute=false;
  }
    if (n-text.start>=MIN_TEXT_SIZE && ! (png || pdfimw || cdi || soi || pgm || rgbi || tga || gif.gif || b64s||tar || bmp.bmp ||wavi ||b64s1 ||b85s1||b85s||DECcount||mrb ||uuds) ||
       (s1==0 && (n-text.start)==n && type==DEFAULT) // ignore minimum text lenght
       ){
        text.needsEolTransform=(text.countNL>text.totalNL);
        in->setpos(start + text.start);
        detd = n-text.start;
        if ( text.isNumbertext)     info=1;
    return (text.needsEolTransform)?EOLTEXT:(( text.isNumbertext)?TEXT0:(text.countUTF8>MIN_TEXT_SIZE?TXTUTF8:TEXT));
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

void encode_rle(File *in, File *out, U64 size, int info, int &hdrsize) {
  U8 b, c = in->getc();
  int i = 1, maxBlockSize = info&0xFFFFFF;
  out->put32(maxBlockSize);
  hdrsize=(4);
  while (i<(int)size) {
    b = in->getc(), i++;
    if (c==0x80) { c = b; continue; }
    else if (c>0x7F) {
      for (int j=0; j<=(c&0x7F); j++) out->putc(b);
      c = in->getc(), i++;
    }
    else {
      for (int j=0; j<=c; j++, i++) { out->putc(b), b = in->getc(); }
      c = b;
    }
  }
}

#define rleOutputRun { \
  while (run > 128) { \
    *outPtr++ = 0xFF, *outPtr++ = byte; \
    run-=128; \
  } \
  *outPtr++ = (U8)(0x80|(run-1)), *outPtr++ = byte; \
}

U64 decode_rle(File *in, U64 size, File *out, FMode mode, U64 &diffFound) {
  U8 inBuffer[0x10000]={0};
  U8 outBuffer[0x10200]={0};
  U64 pos = 0;
  int maxBlockSize = (int)in->get32();
  enum { BASE, LITERAL, RUN, LITERAL_RUN } state;
  do {
    U64 remaining = in->blockread(&inBuffer[0], maxBlockSize);
    U8 *inPtr = (U8*)inBuffer;
    U8 *outPtr= (U8*)outBuffer;
    U8 *lastLiteral = nullptr;
    state = BASE;
    while (remaining>0){
      U8 byte = *inPtr++, loop = 0;
      int run = 1;
      for (remaining--; remaining>0 && byte==*inPtr; remaining--, run++, inPtr++);
      do {
        loop = 0;
        switch (state) {
          case BASE: case RUN: {
            if (run>1) {
              state = RUN;
              rleOutputRun;
            }
            else {
              lastLiteral = outPtr;
              *outPtr++ = 0, *outPtr++ = byte;
              state = LITERAL;
            }
            break;
          }
          case LITERAL: {
            if (run>1) {
              state = LITERAL_RUN;
              rleOutputRun;
            }
            else {
              if (++(*lastLiteral)==127)
                state = BASE;
              *outPtr++ = byte;
            }
            break;
          }
          case LITERAL_RUN: {
            if (outPtr[-2]==0x81 && *lastLiteral<(125)) {
              state = (((*lastLiteral)+=2)==127)?BASE:LITERAL;
              outPtr[-2] = outPtr[-1];
            }
            else
              state = RUN;
            loop = 1;
          }
        }
      } while (loop);
    }

    U64 length = outPtr-(U8*)(&outBuffer[0]);
    if (mode==FDECOMPRESS)
      out->blockwrite(&outBuffer[0], length);
    else if (mode==FCOMPARE) {
      for (int j=0; j<(int)length; ++j) {
        if (outBuffer[j]!=out->getc() && !diffFound) {
          diffFound = pos+j+1;
          break; 
        }
      }
    }
    pos+=length;
  } while (!in->eof() && !diffFound);
  return pos;
}


struct LZWentry{
  int16_t prefix;
  int16_t suffix;
};

#define LZW_RESET_CODE 256
#define LZW_EOF_CODE   257

class LZWDictionary{
private:
  const static int32_t HashSize = 9221;
  LZWentry dictionary[4096];
  int16_t table[HashSize];
  uint8_t buffer[4096];
public:
  int32_t index;
  LZWDictionary(): index(0){ reset(); }
  void reset(){
    memset(&dictionary, 0xFF, sizeof(dictionary));
    memset(&table, 0xFF, sizeof(table));
    for (int32_t i=0; i<256; i++){
      table[-findEntry(-1, i)-1] = (int16_t)i;
      dictionary[i].suffix = i;
    }
    index = 258; //2 extra codes, one for resetting the dictionary and one for signaling EOF
  }
  int32_t findEntry(const int32_t prefix, const int32_t suffix){
    int32_t i = finalize32(hash(prefix, suffix), 13);
    int32_t offset = (i>0)?HashSize-i:1;
    while (true){
      if (table[i]<0) //free slot?
        return -i-1;
      else if (dictionary[table[i]].prefix==prefix && dictionary[table[i]].suffix==suffix) //is it the entry we want?
        return table[i];
      i-=offset;
      if (i<0)
        i+=HashSize;
    }
  }
  void addEntry(const int32_t prefix, const int32_t suffix, const int32_t offset = -1){
    if (prefix==-1 || prefix>=index || index>4095 || offset>=0)
      return;
    dictionary[index].prefix = prefix;
    dictionary[index].suffix = suffix;
    table[-offset-1] = index;
    index+=(index<4096);
  }
  int32_t dumpEntry(File *f, int32_t code){
    int32_t n = 4095;
    while (code>256 && n>=0){
      buffer[n] = uint8_t(dictionary[code].suffix);
      n--;
      code = dictionary[code].prefix;
    }
    buffer[n] = uint8_t(code);
    f->blockwrite(&buffer[n], 4096-n);
    return code;
  }
};

int encode_lzw(File *in, File *out, U64 size, int &hdrsize) {
  LZWDictionary dic;
  int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
  bool done = false;
  while (!done) {
    buffer = in->getc();
    if (buffer<0) { return 0; }
    for (int32_t j=0; j<8; j++ ) {
      code+=code+((buffer>>(7-j))&1), bitsUsed++;
      if (bitsUsed>=bitsPerCode) {
        if (code==LZW_EOF_CODE){ done=true; break; }
        else if (code==LZW_RESET_CODE){
          dic.reset();
          parent=-1; bitsPerCode=9;
        }
        else{
          if (code<dic.index){
            if (parent!=-1)
              dic.addEntry(parent, dic.dumpEntry(out, code));
            else
              out->putc(code);
          }
          else if (code==dic.index){
            int32_t a = dic.dumpEntry(out, parent);
            out->putc(a);
            dic.addEntry(parent,a);
          }
          else return 0;
          parent = code;
        }
        bitsUsed=0; code=0;
        if ((1<<bitsPerCode)==dic.index+1 && dic.index<4096)
          bitsPerCode++;
      }
    }
  }
  return 1;
}

inline void writeCode(File *f, const FMode mode, int32_t *buffer, U64 *pos, int32_t *bitsUsed, const int32_t bitsPerCode, const int32_t code, U64 *diffFound){
  *buffer<<=bitsPerCode; *buffer|=code;
  (*bitsUsed)+=bitsPerCode;
  while ((*bitsUsed)>7) {
    const uint8_t B = *buffer>>(*bitsUsed-=8);
    (*pos)++;
    if (mode==FDECOMPRESS) f->putc(B);
    else if (mode==FCOMPARE && B!=f->getc()) *diffFound=*pos;
  }
}

U64 decode_lzw(File *in, U64 size, File *out, FMode mode, U64 &diffFound) {
  LZWDictionary dic;
  U64 pos=0;
  int32_t parent=-1, code=0, buffer=0, bitsPerCode=9, bitsUsed=0;
  writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE, &diffFound);
  while ((code=in->getc())>=0 && diffFound==0) {
    int32_t index = dic.findEntry(parent, code);
    if (index<0){ // entry not found
      writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, parent, &diffFound);
      if (dic.index>4092){
        writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_RESET_CODE, &diffFound);
        dic.reset();
        bitsPerCode = 9;
      }
      else{
        dic.addEntry(parent, code, index);
        if (dic.index>=(1<<bitsPerCode))
          bitsPerCode++;
      }
      parent = code;
    }
    else
      parent = index;
  }
  if (parent>=0)
    writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, parent, &diffFound);
  writeCode(out, mode, &buffer, &pos, &bitsUsed, bitsPerCode, LZW_EOF_CODE, &diffFound);
  if (bitsUsed>0) { // flush buffer
    pos++;
    if (mode==FDECOMPRESS) out->putc(uint8_t(buffer));
    else if (mode==FCOMPARE && uint8_t(buffer)!=out->getc()) diffFound=pos;
  }
  return pos;
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
/*
MTFList  MTF(81);

#define ZLIB_NUM_COMBINATIONS 81

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
  int index=-1, nTrials=0;
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
  for (U64 i=0; i<len; i+=BLOCK) {
    U32 blsize=min(U32(len-i),BLOCK);
    nTrials=0;
    for (int j=0; j<ZLIB_NUM_COMBINATIONS; j++) {
      if (diffCount[j]==LIMIT) continue;
      nTrials++;
      if (recpos[j]>=BLOCK)
        recpos[j]-=BLOCK;
    }
    // early break if nothing left to test
    if (nTrials==0)
      break;
    memmove(&zrec[0], &zrec[BLOCK], BLOCK);
    memmove(&zin[0], &zin[BLOCK], BLOCK);
    in->blockread(&zin[BLOCK],   blsize  ); // Read block from input file
    
    // Decompress/inflate block
    main_strm.next_in=&zin[BLOCK]; main_strm.avail_in=blsize;
    do {
      main_strm.next_out=&zout[0]; main_strm.avail_out=BLOCK;
      main_ret=inflate(&main_strm, Z_FINISH);
      nTrials=0;
      // Recompress/deflate block with all possible parameters
      for (int j=MTF.GetFirst(); j>=0; j=MTF.GetNext()){
        if (diffCount[j]==LIMIT) continue;
        nTrials++;
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
     } while (main_strm.avail_out==0 && main_ret==Z_BUF_ERROR && nTrials>0);
    if ((main_ret!=Z_BUF_ERROR && main_ret!=Z_STREAM_END) || nTrials==0) break;
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
}*/

 // Transform DEC Alpha code
void encode_dec(File* in, File* out, int len, int begin) {
  const int BLOCK=0x10000;
  Array<U8> blk(BLOCK);
  int count=0;
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_dec read error");
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
    }
    out->blockwrite(&blk[0],  bytesRead  );
  }
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
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
            op=bswap(op);
                if ((op>>21)==0x34*32+26  ) { // bsr r26,offset
                   int offset=op&0x1fffff;
                   offset-=(i)/4;
                   op&=~0x1fffff;
                   op|=offset&0x1fffff;
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
  for (int j=0; j<len; j+=BLOCK) {
    int size=min(int(len-j), BLOCK);
    int bytesRead=in->blockread(&blk[0], size  );
    if (bytesRead!=size) quit("encode_arm read error");
        for (int i=0; i<bytesRead-3; i+=4) {
        unsigned op=blk[i+3]|(blk[i+2]<<8)|(blk[i+1]<<16)|(blk[i]<<24);
        if ((op>>26)==0x25) {
        int offset=op&0x3FFFFFF;
        offset+=(i)/4;
        op&=~0x3FFFFFF;
        op|=offset&0x3FFFFFF;
        count++;
      }
        blk[i]=op;
        blk[i+1]=op>>8;
        blk[i+2]=op>>16;
        blk[i+3]=op>>24;
    }
    out->blockwrite(&blk[0],  bytesRead  );
  }
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
        for (int i=0; i<bytesRead-3; i+=4) {
            unsigned op=blk[i]|(blk[i+1]<<8)|(blk[i+2]<<16)|(blk[i+3]<<24);
                if ((op>>26)==0x25) { 
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
//it's not standard so some files use 'space' some use '`'
#define UUENCODE(c,b) ((c) ? ((c) & 077) + ' ': (b) ? '`':((c) & 077) + ' ')
int decode_uud(File*in, int size, File*out, FMode mode, U64 &diffFound){
    //U8 inn[3];
    int i;//, len=0, blocksout = 0;
    int fle=0;
    int flag=0; 
    int outlen=0,n;
    int tlf=0;//,g=0;
    flag=in->getc();
    outlen=in->getc();
    outlen+=(in->getc()<<8);
    outlen+=(in->getc()<<16);
    tlf=(in->getc());
    outlen+=((tlf&63)<<24);
    Array<U8,1> p(45+4);
    Array<U8,1> ptr((outlen>>1)*4+10);
    tlf=(tlf&192);                     //ignored
    if (tlf==128)       tlf=10;        // LF: 10 
    else if (tlf==64)   tlf=13;        // LF: 13
    else                tlf=0;
    int c1, c2, c3, c4;
    while(fle<outlen){
        memset(&p[0], 0, 45);
        n=in->blockread(&p[0], 45);
        ptr[fle++]=UUENCODE(n,flag);
        for(i = 0; i < n; i += 3){
            c1 = p[0+i] >> 2;
            c2 = (p[0+i] << 4) & 060 | (p[1+i] >> 4) & 017;
            c3 = (p[1+i] << 2) & 074 | (p[2+i] >> 6) & 03;
            c4 = p[2+i] & 077;

            ptr[fle++]=(UUENCODE(c1,flag));
            ptr[fle++]=(UUENCODE(c2,flag));
            ptr[fle++]=(UUENCODE(c3,flag));
            ptr[fle++]=(UUENCODE(c4,flag));
       }
       ptr[fle++]=10; //lf
    }

    //Write out or compare
    if (mode==FDECOMPRESS){
            out->blockwrite(&ptr[0],   outlen  );
        }
    else if (mode==FCOMPARE){
       // out->setpos(0);
    for(i=0;i<outlen;i++){
        U8 b=ptr[i];
        U8 c=out->getc();
            if (b!=c && !diffFound) diffFound= out->curpos();
        }
    }
    return outlen;
}
    
#define UUDECODE(c) (((c) - ' ') & 077)
void encode_uud(File* in, File* out, int len,int info) {
  int in_len = 0;
  int i = 0;
  int j = 0;
  int b=0,n=0;
  int lfp=0;
  int tlf=0;
  char src[4];
  int uumem=(len>>1)*3+10;
  Array<U8,1> ptr(uumem);
  Array<U8,1> p(62);
  int olen=5,inbytes=0;
  int c1, c2, c3,lp=0;
  lfp=in->getc();
  inbytes++;
  b=lfp;

  while (inbytes<len){
    n=UUDECODE(b);
    memset(&p[0], 0, 61);
    in->blockread(&p[0], 61  );
    inbytes=inbytes+61;
    lp=0;
    for(; n > 0; lp += 4, n -= 3){
      c1 = UUDECODE(p[0+lp]) << 2 | UUDECODE(p[1+lp]) >> 4;
      c2 = UUDECODE(p[1+lp]) << 4 | UUDECODE(p[2+lp]) >> 2;
      c3 = UUDECODE(p[2+lp]) << 6 | UUDECODE(p[3+lp]);
      if(n >= 1)
        ptr[olen++]=c1;
      if(n >= 2)
        ptr[olen++]=c2;
      if(n >= 3)
        ptr[olen++]=c3;
   }
   b=in->getc(); //read lf
   inbytes++;
  }

  ptr[0]=info&255; //special flag for space or '`'
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

#define LZW_TABLE_SIZE 9221

#define lzw_find(k) {\
  offset = ((k)*PHI)>>19; \
  int stride = (offset>0)?LZW_TABLE_SIZE-offset:1; \
  while (true){ \
    if ((index=table[offset])<0){ index=-offset-1; break; } \
    else if (dict[index]==int(k)){ break; } \
    offset-=stride; \
    if (offset<0) \
      offset+=LZW_TABLE_SIZE; \
  } \
}

#define lzw_reset { for (int i=0; i<LZW_TABLE_SIZE; table[i]=-1, i++); }

int encode_gif(File* in, File* out, int len) {
  int codesize=in->getc(),hdrsize=6,clearpos=0,bsize=0,code,offset=0;
  U64 diffpos=0,beginin= in->curpos(),beginout= out->curpos();
  Array<U8,1> output(4096);
  out->putc(hdrsize>>8);
  out->putc(hdrsize&255);
  out->putc(bsize);
  out->putc(clearpos>>8);
  out->putc(clearpos&255);
  out->putc(codesize);
  Array<int> table(LZW_TABLE_SIZE);  
  for (int phase=0; phase<2; phase++) {
    in->setpos( beginin);
    int bits=codesize+1,shift=0,buf=0;
    int blocksize=0,maxcode=(1<<codesize)+1,last=-1;//,dict[4096];
    Array<int> dict(4096);
    lzw_reset;
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
            lzw_reset;
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
                int key=(last<<8)+j, index=-1;
                lzw_find(key);
                dict[maxcode]=key;
                table[(index<0)?-index-1:offset]=maxcode;
                if (phase==0 && index>0) {
                    hdrsize+=4;
                    j=diffpos-size-(code==maxcode);
                    out->put32(j);
                    diffpos=size+(code==maxcode);
                  }
                }
              //}
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
  int maxcode=(1<<codesize)+1, input,code,offset=0;
    Array<int> dict(4096);
      Array<int> table(LZW_TABLE_SIZE);
  lzw_reset;
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
    int key=(last<<8)+input, index=(code=-1);
    if (last<0) index=input; else lzw_find(key);
    code = index;
    if (curdiff<diffcount && total-(int)size>diffpos[curdiff]) curdiff++, code=-1;
    if (code<0) {
      gif_write_code(last);
      if (maxcode==clearpos) { gif_write_code(1<<codesize); bits=codesize+1, maxcode=(1<<codesize)+1; lzw_reset }
      else
      {
        ++maxcode;
        if (maxcode<=4095) { dict[maxcode]=key; table[(index<0)?-index-1:offset]=maxcode; }
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

int encodeRLE(U8 *dst, U8 *ptr, int src_end, int maxlen){
    int i=0;
    int ind=0;
    for(ind=0;ind<src_end; ){
        if (i>maxlen) return i;
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
                if (i>maxlen) return i;              
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
                if (i>maxlen) return i;
                for(int cnt=0;cnt<pixels;cnt++) { 
                    dst[i]=ptr[ind+cnt]; 
                    i++;
                    if (i>maxlen) return i;
                }
                ind=ind+pixels;
            }
            else{ 
                j=pixels;  
                while (pixels>127) {
                    dst[i++]=127;                
                    dst[i++]=ptr[ind];  
                    if (i>maxlen) return i;                     
                    pixels=pixels-127;
                }
                if (pixels>0) { 
                    if (j==src_end) pixels--;
                    dst[i++]=pixels;         
                    dst[i++]=ptr[ind];
                    if (i>maxlen) return i;
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
    Array<U32> diffpos(4096);
    U32 count=0;
    U8 value=0; 
    int diffcount=0;
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
    int a=encodeRLE(&ptr[0],&ptrin[0],totalSize,len);
    assert(a<(len+4));
    // compare to original and output diff data
    in->setpos(savepos);
    for(int i=0;i<len;i++){
        U8 b=ptr[i],c=in->getc();
        if (diffcount==4095 ||  diffcount>(len/2)||i>0xFFFFFF) return; // fail
        if (b!=c ) {
            if (diffcount<4095)diffpos[diffcount++]=c+(i<<8);
        }
    }
    out->putc((diffcount>>8)&255); out->putc(diffcount&255);
    if (diffcount>0)
    out->blockwrite((U8*)&diffpos[0], diffcount*4);
    out->put32(len);
    out->blockwrite(&ptrin[0], totalSize);
}

int decode_mrb(File* in, int size, int width, File*out1, FMode mode, uint64_t &diffFound) {
    if (size==0) {
        diffFound=1;
        return 0;
    }
    Array<U32> diffpos(4096);
    int diffcount=0;
    diffcount=(in->getc()<<8)+in->getc();
    if (diffcount>0) in->blockread((U8*)&diffpos[0], diffcount*4);
    int len=in->get32();
    Array<U8,1> fptr(size+4);
    Array<U8,1> ptr(size+4);
    in->blockread(&fptr[0], size );
    encodeRLE(&ptr[0],&fptr[0],size-2-4-diffcount*4,len); //size - header
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

enum EEOLType {UNDEFINED, CRLF, LF};

#define MAX_FREQ_ORDER1 255
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

        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        coder.EncodeOrder(prev,result); 
        return 32;
    }
    void EncodeEOLformat(EEOLType EOLType){
        if(EOLType==CRLF)    coder.Encode(0,1,2);
        else     coder.Encode(1,1,2);
    }

    void EOLencode(File* file,File* fileout,int fileLen){
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
            fileout->putc(c );
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

        if(leftChar>96||leftChar==',')leftChar=122;
        if(leftChar<96)leftChar=125;
        prev=min(distance,90)/5*12+(leftChar-'a')/3;
        result=coder.DecodeOrder(prev); 
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

    void hook_putc(int c,File* out,int maxlen){
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
                    if (fpos==maxlen) return;
                    out->putc(lastChar),fpos++;
                }
                lastEOL=fpos;
            }
            if (fpos==maxlen) return;
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
        if (fpos==maxlen) return;
        out->putc(c),fpos++;
    }

    void EOLdecode(File* in,File* out,int size,File*outeol,File*wd,int len){
        int c=0;
        bufChar=-1;
        lastEOL=-1;
        EOLType=UNDEFINED;
        fpos=0;
        coder.StartDecode(outeol);
        
        for ( int i=0; i<size; i++) {
            c=wd->getc();
            if (c==5){
                hook_putc(13,out,len);
                hook_putc(10,out,len);
            }
            else {    
                hook_putc(c,out,len);
            }
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
    out->put32(len);
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
    int len=in->get32();
    eolz=in->get32();
    wrtz=in->get32();
    
    for (U64 offset=0; offset<eolz; offset++) wrtfi.putc(in->getc()); 
    wrtfi.setpos(0);
    EOLDecoderCoder* eold;
    eold=new EOLDecoderCoder(); 
    eold->EOLdecode(in,&tmpout,wrtz,&wrtfi,in,len);

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
  segment.put4(info);
  int srid=getstreamid(type);
  for (U64 j=s1; j<s1+len; ++j) filestreams[srid]->putc(in->getc());
}

void DetectRecursive(File*in, U64 n, Encoder &en, char *blstr, int it, U64 s1, U64 s2);

void transform_encode_block(Filetype type, File*in, U64 len, Encoder &en, int info, int info2, char *blstr, int it, U64 s1, U64 s2, U64 begin) {
    if (type==EXE || type==DECA || type==ARM || type==CD|| type==MDF || type==IMAGE24 || type==IMAGE32  ||type==MRBR ||type==MRBR4||type==RLE || type==LZW||type==EOLTEXT||
     (type==TEXT || type==TXTUTF8|| type==TEXT0 ) || type==BASE64 || type==BASE85 || type==UUENC||type==SZDD|| /*type==ZLIB||*/ type==GIF) {
        U64 diffFound=0;
        FileTmp* tmp;
        tmp=new FileTmp;
        if (type==IMAGE24) encode_bmp(in, tmp, int(len), info);
        else if (type==IMAGE32) encode_im32(in, tmp, int(len), info);
        else if (type==MRBR) encode_mrb(in, tmp, int(len), info,info2);
        else if (type==MRBR4) encode_mrb(in, tmp, int(len),     ((info*4+15)/16)*2,info2);
        else if (type==RLE) encode_rle(in, tmp, len, info, info2);
        else if (type==LZW) encode_lzw(in, tmp, len, info2);
        else if (type==EXE) encode_exe(in, tmp, int(len), int(begin));
        else if (type==DECA) encode_dec(in, tmp, int(len), int(begin));
        else if (type==ARM) encode_arm(in, tmp, int(len), int(begin));
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
        else if (type==UUENC) encode_uud(in, tmp, int(len),info);
        else if (type==BASE85) encode_ascii85(in, tmp, int(len));
        else if (type==SZDD) encode_szdd(in, tmp, info);
      //  else if (type==ZLIB) diffFound=encode_zlib(in, tmp, int(len))?0:1;
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
        
        if (/*type==ZLIB ||*/ type==GIF || type==MRBR|| type==MRBR4|| type==RLE|| type==LZW||type==BASE85 ||type==BASE64 || type==UUENC|| type==DECA|| type==ARM || (type==TEXT || type==TXTUTF8 ||type==TEXT0)||type==EOLTEXT ){
        int ts=0;
         in->setpos(begin);
        if (type==BASE64 ) decode_base64(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==UUENC ) decode_uud(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==BASE85 ) decode_ascii85(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        //else if (type==ZLIB && !diffFound) decode_zlib(tmp, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==GIF && !diffFound) decode_gif(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==MRBR || type==MRBR4) decode_mrb(tmp, int(tmpsize), info, in, FCOMPARE, diffFound);
        else if (type==RLE)                 decode_rle(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==LZW)                 decode_lzw(tmp, tmpsize, in, FCOMPARE, diffFound);
        else if (type==DECA) decode_dec(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if (type==ARM) decode_arm(en, int(tmpsize), in, FCOMPARE, diffFound);
        else if ((type==TEXT || type==TXTUTF8 ||type==TEXT0) ) decode_txt(en, int(tmpsize), in, FCOMPARE, diffFound);
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
            } else if (type==RLE) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=( 4);
                Filetype type2 =(Filetype)(info>>24);
                //hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, en, s1, s2, info);
            } else if (type==LZW) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=( 0);
                Filetype type2 =(Filetype)(info>>24);
                //hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
               // typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
               // direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                if (it==itcount)    itcount=it+1;
                typenamess[type2][it+1]+=tmpsize-hdrsize,  typenamesc[type2][it+1]++;
                direct_encode_blockstream(type2, tmp, tmpsize-hdrsize, en, s1, s2, info&0xffffff);
            } else if (type==MRBR4) {
                segment.put1(type);
                segment.put8(tmpsize);
                segment.put4(0);
                int hdrsize=( tmp->getc()<<8)+(tmp->getc());
                hdrsize=4+hdrsize*4+4;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                if (it==itcount)    itcount=it+1;
                typenamess[IMAGE4][it+1]+=tmpsize,  typenamesc[IMAGE4][it+1]++;
                direct_encode_blockstream(IMAGE4, tmp, tmpsize-hdrsize, en, s1, s2, info);
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
                hdrsize=tmp->get32();
                hdrsize=hdrsize+12;
                tmp->setpos(0);
                typenamess[HDR][it+1]+=hdrsize,  typenamesc[HDR][it+1]++; 
                direct_encode_blockstream(HDR, tmp, hdrsize, en,0, s2);
                typenamess[TEXT][it+1]+=tmpsize-hdrsize,  typenamesc[TEXT][it+1]++;
                transform_encode_block(TEXT,  tmp, tmpsize-hdrsize, en, -1,-1, blstr, it, s1, s2, hdrsize); 
            } else if (typet[type][RECURSIVE]) {
                segment.put1(type);
                segment.put8(tmpsize);
                if (type==SZDD /*||  type==ZLIB*/) segment.put4(info);else segment.put4(0);
              /*  if (type==ZLIB) {// PDF or PNG image && info
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
                } else {   */  
                    DetectRecursive( tmp, tmpsize, en, blstr,it+1, 0, tmpsize);//it+1
                    tmp->close();
                    return;
              //  }    
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
        }else {
            const int i1=(typet[type][INFO])?info:-1;/*type==IMAGE1 || type==IMAGE8 || type==IMAGE4  ||type==PNG8|| type==PNG8GRAY|| type==PNG24 || type==PNG32|| type==IMAGE8GRAY || type==AUDIO || type==DBASE ||type==IMGUNK*/
            direct_encode_blockstream(type, in, len, en, s1, s2, i1);
        }
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
    if((len>>1)<(info) && type==DEFAULT && info<len) type=BINTEXT;
    typenamess[type][it]+=len,  typenamesc[type][it]++; 
      //s2-=len;
      sprintf(blstr,"%s%d",b2,blnum++);
      
      printf(" %-11s | %-9s |%10.0d [%d - %d]",blstr,typenames[type],(U32)len,(U32)begin,(U32)end-1);
      if (type==AUDIO) printf(" (%s)", audiotypes[(info&31)%4+(info>>7)*2]);
      else if (type==IMAGE1 || type==IMAGE4 || type==IMAGE8 || type==IMAGE24 || type==MRBR|| type==MRBR4|| type==IMAGE8GRAY || type==IMAGE32 ||type==GIF) printf(" (width: %d)", info&0xFFFFFF);
      else if (type==CD) printf(" (m%d/f%d)", info==1?1:2, info!=3?1:2);
     // else if (type==ZLIB && (info>>24) > 0) printf(" (%s)",typenames[info>>24]);
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
        for (int k=0; k<8; k++) len=len<<8,len+=segment(segment.pos++);
        for (int k=info=0; k<4; ++k) info=(info<<8)+segment(segment.pos++);
        int srid=getstreamid(type);
        if (srid>=0) en.setFile(filestreams[srid]);
        #ifndef NDEBUG 
         printf(" %d  %-9s |%0lu [%0lu]\n",it, typenames[type],len,i );
        #endif
        if (type==IMAGE24 /*&& !(info&PNGFlag)*/)      len=decode_bmp(en, int(len), info, out, mode, diffFound);
        else if (type==IMAGE32 /*&& !(info&PNGFlag)*/) decode_im32(en, int(len), info, out, mode, diffFound);
        else if (type==EXE)     len=decode_exe(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==DECA)    len=decode_dec(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==ARM)     len=decode_arm(en, int(len), out, mode, diffFound, int(s1), int(s2));
        else if (type==BIGTEXT) len=decode_txt(en, int(len), out, mode, diffFound);
        //else if (type==EOLTEXT) len=decode_txtd(en, int(len), out, mode, diffFound);
        else if (type==BASE85 || type==BASE64 || type==UUENC || type==SZDD || /*type==ZLIB ||*/ type==CD || type==MDF  || type==GIF || type==MRBR|| type==MRBR4 || type==RLE ||type==EOLTEXT) {
            FileTmp tmp;
            decompressStreamRecursive(&tmp, len, en, FDECOMPRESS, it+1, s1+i, s2-len);
            if (mode!=FDISCARD) {
                tmp.setpos(0);
                if (type==BASE64) len=decode_base64(&tmp, int(len), out, mode, diffFound);
                else if (type==UUENC)  len=decode_uud(&tmp, int(len), out, mode, diffFound);
                else if (type==BASE85) len=decode_ascii85(&tmp, int(len), out, mode, diffFound);
                else if (type==SZDD)   len=decode_szdd(&tmp,info,info ,out, mode, diffFound);
               // else if (type==ZLIB)   len=decode_zlib(&tmp,int(len),out, mode, diffFound);
                else if (type==CD)     len=decode_cd(&tmp, int(len), out, mode, diffFound);
                else if (type==MDF)    len=decode_mdf(&tmp, int(len), out, mode, diffFound);
                else if (type==GIF)    len=decode_gif(&tmp, int(len), out, mode, diffFound);
                else if (type==MRBR|| type==MRBR4)   len=decode_mrb(&tmp, int(len), info, out, mode, diffFound);
                else if (type==EOLTEXT)len=decode_txtd(&tmp, int(len), out, mode, diffFound);
                else if (type==RLE)    len=decode_rle(&tmp, len, out, mode, diffFound);
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
char *pp ="int c,c1,*t; \n void update(int y,int c0,int b,int c4,int p){ \n"
"int cc1; \n cc1=c+c1; \n if (y) t[cc1]=t[cc1]+((65536-t[cc1])>>5); \n"
"else t[cc1]=t[cc1]-(t[cc1]>>5); \n"
"if ((c=c*2+y)>=512) c1=(c1+(c&255)<<9)&0x1ffffff,c=1; \n"
"return apm(0,(t[c+c1]>>4),c,7);} \n void block(int a,int b){} \n"
"int main() { \n int i; \n if (!(t=malloc(0x2000000*sizeof(int)))) exit(-1); \n"
"vms(0,1,0,0,0,0,0,0,0); \n vmi(2,0,256,0,-1);\nc1=0,c=1; \n"
"for (i=0; i<0x2000000; i++) t[i]=32768;}";
/* "int c,c1,*t; \n void update(int y,int c0,int b,int c4,int p){ \n"
                                    "if (y) t[c+c1]=t[c+c1]+((65536-t[c+c1])>>5); \n"
                                    "else t[c+c1]=t[c+c1]-(t[c+c1]>>5); \n"
                                    "if ((c=c*2+y)>=512) c1=(c1+(c&255)<<9)&0x1ffffff,c=1; \n"
                                    "return (t[c+c1]>>4);} \n void block(int a,int b){} \n"
                                    "int main() { \n int i; \n if (!(t=malloc(0x2000000*sizeof(int)))) exit(-1); \n"
                                    "c1=0,c=1; \n for (i=0; i<0x2000000; i++) t[i]=32768;}";*/
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
    U8 *p;
                datasegmentsize=size;
                    U64 total=size;
                    datasegmentpos=0;
                    datasegmentinfo=0;
                    datasegmentlen=0;
                    // datastreams
                    FILE *moin;
                    moin=0; 
                    if (level>0){
                    
                    switch(i) {
                        case 0:
                        case 1: 
                        case 3: 
                        case 6: 
                        case 7:  
                        case 8:  
                        case 9: 
                        case 10:
                        case 11:{
                            //if (modeQuick) 
                             // moin=fopen("test3s.cfg", "rb");
                        //    else 
                            moin=fopen("test3d.cfg", "rb");
                        break;}  
                        case 2: {
                            moin=fopen("test3img.cfg", "rb");
                            break;}
                        case 4: {
                              moin=fopen("test3i8.cfg", "rb");  
                            break;}
                        case 5: {
                              moin=fopen("test3i24.cfg", "rb");   
                            break;}
                    }
                    }
                                         if(moin){
                        File *modelo;//open tmp file for compressed config file
                       modelo= new FileTmp();
                        Encoder* enm;
                        Predictors* prm;
                         
                        prm=new Predictor(pp);
                        enm=new Encoder(COMPRESS, modelo,*prm);
                        prm->set();
        
                           fseek ( moin , 0 , SEEK_END );
                           int fsz=ftello(moin); 
                           assert(fsz>0);
                           fseek ( moin , 0 , SEEK_SET );
                           //compress model file
                           enm->compress(fsz>>24); enm->compress(fsz>>16); enm->compress(fsz>>8); enm->compress(fsz); // config file length
                           for (int k=0;k<fsz;++k) enm->compress(getc(moin));
                           enm->flush();
                        delete enm;
                        delete prm;
           printf("Compressed model from %d",fsz);
                          fsz=modelo->curpos(); //ftello(modelo); 
                          modelo->setpos(0);// fseek ( modelo , 0 , SEEK_SET );
                          p = (U8 *)calloc(fsz+1,1); 
                         modelo->blockread(p,fsz) ;//fread( p, 1,fsz,modelo); 
                          p[fsz] = 0;
                          printf(" to %d bytes\n",fsz);
                           // write compressed model to archive
                        //fwrite(&p[0], 1, fsz, out);
                        out->blockwrite(&p[0],fsz);
                        //read again model file
                        fseek ( moin , 0 , SEEK_END );
                        fsz=ftello(moin); 
                           fseek ( moin , 0 , SEEK_SET );
                           free(p);
                           //read config file for compression
                          p = (U8 *)calloc(fsz+1,1); 
                         fread( p, 1,fsz,moin); 
                          p[fsz] = 0;
                          //close compressed and uncomressed model files
                        fclose(moin); 
                        modelo->close();// fclose();
                    }
                    else quit("Config file not found.");
                      
                    threadpredict=new Predictor((char *)p);
                    
                    switch(i) {
                        case 0: {
                              printf("Compressing default stream(0).  Total %0.0f  \n",datasegmentsize +0.0);
                            break;}
                        case 1: {
                              printf("Compressing jpeg    stream(1).  Total %0.0f  \n",datasegmentsize +0.0); 
                            break;}        
                        case 2: {
                              printf("Compressing image1  stream(2).  Total %0.0f  \n",datasegmentsize +0.0);
                            break;}
                        case 3: {
                              printf("Compressing image4  stream(3).  Total %0.0f  \n",datasegmentsize +0.0); 
                            break;}    
                        case 4: {
                              printf("Compressing image8  stream(4).  Total %0.0f  \n",datasegmentsize +0.0); 
                            break;}
                        case 5: {
                              printf("Compressing image24 stream(5).  Total %0.0f  \n",datasegmentsize +0.0); 
                            break;}        
                        case 6: {
                              printf("Compressing audio   stream(6).  Total %0.0f  \n",datasegmentsize +0.0); 
                            break;}
                        case 7: {
                              printf("Compressing exe     stream(7).  Total %0.0f  \n",datasegmentsize +0.0);
                            break;}
                        case 8: {
                              printf("Compressing text0 wrt stream(8). Total %0.0f  \n",datasegmentsize +0.0);
                            //wrtn=1;
                            break;}
                            case 11: {
                              printf("Compressing dec wrt stream(8). Total %0.0f  \n",datasegmentsize +0.0);
                            //wrtn=1;
                            break;}
                        case 9: 
                        case 10:{
                              printf("Compressing %stext wrt stream(%d). Total %0.0f  \n",i==10?"big":"",i,datasegmentsize +0.0);
                            //wrtn=0;
                            break;}   
                    }
                    threadencode=new Encoder (COMPRESS, out,*threadpredict); 
                    
                     if ((i>=0 && i<=7) || i==10|| i==11){
                        while (datasegmentsize>0) {
                            while (datasegmentlen==0){
                                datasegmenttype=(Filetype)segment(datasegmentpos++);
                                for (int ii=0; ii<8; ii++) datasegmentlen<<=8,datasegmentlen+=segment(datasegmentpos++);
                                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                                if (!(isstreamtype(datasegmenttype,i) ))datasegmentlen=0;
                                if (level>0){
                                threadencode->predictor.x.filetype=datasegmenttype;
                                threadencode->predictor.x.blpos=0;
                                threadencode->predictor.x.finfo=datasegmentinfo;
                                threadencode->predictor.set();
                                threadencode->predictor.setdebug(0);
                                }
                            }
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                //#ifndef MT
                                if (!(datasegmentsize&0x1fff)) printStatus(total-datasegmentsize, total,i);
                                //#endif
                                threadencode->compress(in->getc());
                                datasegmentsize--;
                            }
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
                            printf(" Total %d wrt: %d\n",(U32)datasegmentsize,(U32)datasegmentlen); 
                            tm.setpos(0);
                            if (level>0){
                            threadencode->predictor.x.filetype=DICTTXT;
                            threadencode->predictor.x.blpos=0;
                            threadencode->predictor.x.finfo=-1;
                            threadencode->predictor.set();
                            threadencode->predictor.setdebug(0);
                            }
                            threadpredict->set();
                            for (U64 k=0; k<datasegmentlen; ++k) {
                                //#ifndef MT
                                if (!(k&0x1fff)) printStatus(k, datasegmentlen,i);
                                //#endif
                                threadencode->compress( tm.getc());
                            }
                            
                            tm.close();
                            threadencode->flush();
                            //printf("Stream(%d) block pos %11.0f compressed to %11.0f bytes\n",i,datasegmentlen+0.0,ftello(out)-scompsize+0.0);
                            datasegmentlen=datasegmentsize=0;   
                    }
                    
                    
            if (level>0) delete threadpredict;
            delete threadencode;
           printf("Stream(%d) compressed from %d to %d bytes\n",i,(U32)size, (U32)out->curpos());
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
                quit("Valid options are -s0 through -s15, -d, -l\n");
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
#ifdef            _MSC_VER 
            printf("Compiled %s, compiler Visual Studio version %d\n\n",__DATE__, _MSC_VER);
#endif
#ifdef VMJIT
            printf("Compiled with VM x86 JIT.\n"); 
#else
            printf("Compiled with VM emulation.\n");
#endif
#ifdef MT
printf("Multithreading enabled with %s.\n",
#ifdef PTHREAD
"PTHREAD"
#else
"windows native threads"
#endif
);

#if defined(__AVX2__)
printf("Compiled with AVX2\n");
#elif defined(__SSE4_1__)   
printf("Compiled with SSE41\n");
#elif  defined(__SSSE3__)
printf("Compiled with SSSE3\n");
#elif defined(__SSE2__) 
printf("Compiled with SSE2\n");
#elif defined(__SSE__)
printf("Compiled with SSE\n");
#else
printf("No vector instrucionts\n");
#endif
#endif
printf("\n");

            printf(
#ifdef WINDOWS
            "To compress or extract, drop a file or folder on the "
            PROGNAME " icon.\n"
            "The output will be put in the same folder as the input.\n"
            "\n"
            "Or from a command window: "
#endif
            "To compress:\n"
            "  " PROGNAME " -slevel file               (compresses to file." PROGNAME ")\n"
            "  " PROGNAME " -slevel archive files...   (creates archive." PROGNAME ")\n"
            "  " PROGNAME " file                       (level -%d pause when done)\n"
            "level: -s0          store\n"
            "  -s1...-s3         (uses 393, 398, 409 MB)\n"
            "  -s4...-s9         (uses 1.2  1.3  1.5  1.9 2.7 4.9 GB)\n"
            "  -s10...-s15       (uses 7.0  9.0 11.1 27.0   x.x x.x GB)\n"
#ifdef MT 
            "  to use multithreading -level:threads (1-9, compression only)\n"
            "  " PROGNAME " -s4:2 file (use level 4 threads 2)\n\n"
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
   /* char *pp = "int c,c1,*t; \n void update(int y,int c0,int b,int c4,int p){ \n"
                                    "if (y) t[c+c1]=t[c+c1]+((65536-t[c+c1])>>5); \n"
                                    "else t[c+c1]=t[c+c1]-(t[c+c1]>>5); \n"
                                    "if ((c=c*2+y)>=512) c1=(c1+(c&255)<<9)&0x1ffffff,c=1; \n"
                                    "return (t[c+c1]>>4);} \n void block(int a,int b){} \n"
                                    "int main() { \n int i; \n if (!(t=malloc(0x2000000*sizeof(int)))) exit(-1); \n"
                                    "c1=0,c=1; \n for (i=0; i<0x2000000; i++) t[i]=32768;}";*/
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
            segpredict=new Predictor(pp);
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
        predictord=new Predictor(pp);
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
          if (jobs[i].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
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
        if (jobs[id].state==FINISHED_ERR) quit("thread"); //exit program on thread error 
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
            segpredict=new Predictor(pp);
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
           char *app;
            for (int i=0; i<streamc; ++i) {
                datasegmentsize=(filestreamsize[i]); // get segment data offset
                if (datasegmentsize>0){              // if segment contains data
                    filestreams[i]->setpos( 0);
                    U64 total=datasegmentsize;
                    datasegmentpos=0;
                    datasegmentinfo=0;
                    datasegmentlen=0;
                    if (predictord) delete predictord,predictord=0,free(app);
                    if (defaultencoder) delete defaultencoder,defaultencoder=0;
                    //load config file from archive stream
                    //read compressed file header and data
                    int fsz=0;  
                        Encoder* enm;
                    Predictors* prm;
                   /* char *ppp = "int c,c1,*t; \n void update(int y,int c0,int b,int c4,int p){ \n"
                                "if (y) t[c+c1]=t[c+c1]+((65536-t[c+c1])>>5); \n"
                                "else t[c+c1]=t[c+c1]-(t[c+c1]>>5); \n"
                                "if ((c=c*2+y)>=512) c1=(c1+(c&255)<<9)&0x1ffffff,c=1; \n"
                                "return (t[c+c1]>>4);} \n void block(int a,int b){} \n"
                                "int main() { \n int i; \n if (!(t=malloc(0x2000000*sizeof(int)))) exit(-1); \n"
                                "c1=0,c=1; \n for (i=0; i<0x2000000; i++) t[i]=32768;}";*/
                    prm=new Predictor(pp);
                    enm=new Encoder(DECOMPRESS, archive,*prm);
                    prm->set();
                    int len=0;
                    len+=enm->decompress()<<24; //decompress compressed model lenght
                    len+=enm->decompress()<<16;
                    len+=enm->decompress()<<8;
                    len+=enm->decompress();
                   app = (char *)calloc(len+1,1); //alloc mem for decompressed buf
                       // decompress model into buf pp
                       for (int k=0;k<len;++k) app[k]=enm->decompress();
                        app[len] = 0;
                    delete enm; //delete encoder and predictor
                    delete prm; 
                      //init predictor with decompressed model
                      predictord=new Predictor(app);
                    printf("DeCompressing ");
                    switch(i) {
                        case 0: { printf("default   stream(0).\n"); break;}
                        case 1: { printf("jpeg      stream(1).\n"); break;}        
                        case 2: { printf("image1    stream(2).\n"); break;}
                        case 3: { printf("image4    stream(3).\n"); break;}    
                        case 4: { printf("image8    stream(4).\n"); break;}
                        case 5: { printf("image24   stream(5).\n"); break;}        
                        case 6: { printf("audio     stream(6).\n"); break;}
                        case 7: { printf("exe       stream(7).\n"); break;}
                        case 8: {  printf("text0 wrt stream(8).\n"); break;}
                        case 9: 
                        case 10: { printf("%stext wrt stream(%d).\n",i==10?"big":"",i); break;}   
                        case 11: { printf("dec       stream(11).\n"); break;}
                    }
                     /*if (level>0){
                    switch(i) {
                        case 0: {
                            predictord=new Predictor();    
                             break;}
                        case 1: {
                             predictord=new PredictorJPEG(); break;}
                        case 2: {
                            predictord=new PredictorIMG1(); break;}
                        case 3: {
                            predictord=new PredictorIMG4(); break;}
                        case 4: {
                            predictord=new PredictorIMG8(); break;}
                        case 5: {
                            predictord=new PredictorIMG24(); break;}
                        case 6: {
                            predictord=new PredictorAUDIO2(); break;}
                        case 7: {
                            predictord=new PredictorEXE();    break;}
                        case 8: {
                            predictord=new PredictorTXTWRT(); break;}
                        case 9:
                        case 10: {
                            predictord=new PredictorTXTWRT(); break;}
                        case 11: {
                            predictord=new PredictorDEC(); break;}
                    }
                    }*/
                     defaultencoder=new Encoder (mode, archive,*predictord); 
                     if ((i>=0 && i<=7)||i==10||i==11){
                        while (datasegmentsize>0) {
                            while (datasegmentlen==0){
                                datasegmenttype=(Filetype)segment(datasegmentpos++);
                                for (int ii=0; ii<8; ii++) datasegmentlen=datasegmentlen<<8,datasegmentlen+=segment(datasegmentpos++);
                                for (int ii=0; ii<4; ii++) datasegmentinfo=(datasegmentinfo<<8)+segment(datasegmentpos++);
                                if (!(isstreamtype(datasegmenttype,i) ))datasegmentlen=0;
                                if (level>0) {
                                defaultencoder->predictor.x.filetype=datasegmenttype;
                                defaultencoder->predictor.x.blpos=0;
                                defaultencoder->predictor.x.finfo=datasegmentinfo; 
                                defaultencoder->predictor.set();
                                defaultencoder->predictor.setdebug(0);
                                }
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
                            if (level>0) {
                            defaultencoder->predictor.x.filetype=DICTTXT;
                            defaultencoder->predictor.x.blpos=0;
                            defaultencoder->predictor.x.finfo=-1; 
                            defaultencoder->predictor.set();
                                defaultencoder->predictor.setdebug(0);
                            
                            }
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

