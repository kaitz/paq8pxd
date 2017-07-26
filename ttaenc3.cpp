/*
 * ttaenc.c
 *
 * Description: TTAv1 lossless audio encoder/decoder.
 * Copyright (c) 2007, Aleksander Djuric (ald@true-audio.com)
 * Distributed under the GNU General Public License (GPL).
 * The complete text of the license can be found in the
 * COPYING file included in the distribution.
 *
 */

/* 
 ttaenc  v3.4.1 b20070727    
 http://tta.sourceforge.net
*/
#define FRAME_TIME        1.04489795918367346939
#define MAX_ORDER        16
#define BIT_BUFFER_SIZE (1024*1024)

#define MEMORY_ERROR    7
#define WRITE_ERROR    8
#define READ_ERROR    9

#ifdef _WIN32
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
typedef __uint32_t uint32;
typedef __uint64_t uint64;
#endif

#define PREDICTOR1(x, k)    ((int)((((uint64)x << k) - x) >> k))
#define ENC(x)  (((x)>0)?((x)<<1)-1:(-(x)<<1))
#define DEC(x)  (((x)&1)?(++(x)>>1):(-(x)>>1))

typedef struct {
    int shift;
    int round;
    int error;
    int qm[MAX_ORDER];
    int dx[MAX_ORDER];
    int dl[MAX_ORDER];
} fltst;

typedef struct {
    fltst fst;
    int last;
} encoder;


/******************* static variables and structures *******************/
static unsigned char *WAVE_BUFFER;

void tta_error(int error, wchar_t *name){
    switch (error) { 
    case MEMORY_ERROR:
    quit("Error:\tinsufficient memory availablen\n"); 
    case WRITE_ERROR:
    quit("Error:\tcan't write to output filen\n"); 
    case READ_ERROR:
    quit("Error:\tcan't read from input file\n\n");
   }
}

void *tta_malloc(size_t num, size_t size){
    void *array;

    if ((array = calloc(num, size)) == NULL)
    tta_error(MEMORY_ERROR, NULL);

    return (array);
}

int read_wave(int *data, int byte_size, unsigned int len, FILE *fdin) {
    unsigned int res;
    unsigned char *src, *end;
    int *dst = data;

    src = WAVE_BUFFER;
    if (!(res = fread(WAVE_BUFFER, byte_size, len, fdin)))
        tta_error(READ_ERROR, NULL);
    end = WAVE_BUFFER + res * byte_size;

    switch (byte_size) {
    case 1: for (; src < end; dst++)
        *dst = (signed int) *src++ - 0x80;
        break;
    case 2: for (; src < end; dst++) {
        *dst = (unsigned char) *src++;
        *dst |= (signed char) *src++ << 8;
        }
        break;
    case 3: for (; src < end; dst++) {
        *dst = (unsigned char) *src++;
        *dst |= (unsigned char) *src++ << 8;
        *dst |= (signed char) *src++ << 16;
        }
        break;
    }

    return res;
}

int write_wave(int *data, int byte_size, int num_chan, unsigned int len, FILE *fdout) {
    unsigned int res;
    int *src = data, *end;
    unsigned char *dst;

    dst = WAVE_BUFFER;
    end = data + len;

    switch (byte_size) {
    case 1: for (; src < end; src++)
        *dst++ = (unsigned char) (*src + 0x80);
        break;
    case 2: for (; src < end; src++) {
        *dst++ = (unsigned char) *src;
        *dst++ = (unsigned char) (*src >> 8);
        }
        break;
    case 3: for (; src < end; src++) {
        *dst++ = (unsigned char) *src;
        *dst++ = (unsigned char) (*src >> 8);
        *dst++ = (unsigned char) (*src >> 16);
        }
        break;
    }

    if (!(res = fwrite(WAVE_BUFFER, byte_size, len, fdout)))
        tta_error(WRITE_ERROR, NULL);

    return res;
}

/************************* filter functions ****************************/
__inline void memshl (register int *pA, register int *pB) {
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA++ = *pB++;
    *pA   = *pB;
}

__inline void hybrid_filter (fltst *fs, int *in, int mode) {
    register int *pA = fs->dl;
    register int *pB = fs->qm;
    register int *pM = fs->dx;
    register int sum = fs->round;

    if (!fs->error) {
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++;
        sum += *pA++ * *pB, pB++; pM += 8;
    } else if (fs->error < 0) {
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
        sum += *pA++ * (*pB -= *pM++), pB++;
    } else {
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
        sum += *pA++ * (*pB += *pM++), pB++;
    }

    *(pM-0) = ((*(pA-1) >> 30) | 1) << 2;
    *(pM-1) = ((*(pA-2) >> 30) | 1) << 1;
    *(pM-2) = ((*(pA-3) >> 30) | 1) << 1;
    *(pM-3) = ((*(pA-4) >> 30) | 1);

    if (mode) {
        *pA = *in;
        *in -= (sum >> fs->shift);
        fs->error = *in;
    } else {
        fs->error = *in;
        *in += (sum >> fs->shift);
        *pA = *in;
    }

    *(pA-1) = *(pA-0) - *(pA-1);
    *(pA-2) = *(pA-1) - *(pA-2);
    *(pA-3) = *(pA-2) - *(pA-3);

    memshl (fs->dl, fs->dl + 1);
    memshl (fs->dx, fs->dx + 1);
}

void filter_init (fltst *fs, int shift) {
    memset (fs, 0, sizeof(fltst));
    fs->shift = shift;
    fs->round = 1 << (shift - 1);
}


void encoder_init(encoder *tta, int nch, int byte_size) {
    int flt_set [3] = { 10, 9, 10 };
    int i;

    for (i = 0; i < nch; i++) {
        filter_init(&tta[i].fst, flt_set[byte_size - 1]);
     
        tta[i].last = 0;
    }
}

int compress(FILE *fdin, FILE *fdout,int file_size,int info,int info2) 
{
    int *p, *data, tmp, prev;
    unsigned int num_chan, data_size, byte_size, data_len;
    unsigned int buffer_len, framelen, lastlen, fframes;
    unsigned int value, k, unary, binary;
    unsigned int st_size, *st, offset = 0;
    unsigned int def_subchunk_size = 16;
    encoder *tta, *enc;
  
    int wavsr=0;
    switch (info>>5) {
      case 3: wavsr=44100; break;
      case 2: wavsr=22050; break;
      case 1: wavsr=11025; break;
      case 0: wavsr=8000; break;
      default: wavsr = -1;
    }
    if (wavsr==-1) quit("Error (tta): encode wav wsr");

    int bits=(((info&31)%4)/2)*8+8;
    framelen = (int) (FRAME_TIME * wavsr);
    num_chan = (info&31)%2+1;
    data_size = file_size; 
    byte_size = (bits + 7)/8;
    data_len = data_size / (byte_size * num_chan);

    lastlen = data_len % framelen;
    fframes = data_len / framelen + (lastlen ? 1 : 0);
    st_size = (fframes + 1);
    buffer_len = num_chan * framelen;
  // grab some space for an encoder buffers
    data = (int *) tta_malloc(buffer_len, sizeof(int));
   
    enc = tta =(encoder*) tta_malloc(num_chan, sizeof(encoder));
    WAVE_BUFFER = (unsigned char *) tta_malloc(buffer_len, byte_size);

    while (fframes--) {
        if (!fframes && lastlen)
            buffer_len = num_chan * (framelen = lastlen);

        read_wave(data, byte_size, buffer_len, fdin);
        encoder_init(tta, num_chan, byte_size);

        for (p = data, prev = 0; p < data + buffer_len; p++) {
            fltst *fst = &enc->fst;
            int *last = &enc->last;

            // transform data
            if (enc < tta + num_chan - 1)
                *p = prev = *(p + 1) - *p;
            else *p -= prev / 2;

            // compress stage 1: fixed order 1 prediction
            tmp = *p;
            switch (byte_size) {
            case 1:    *p -= PREDICTOR1(*last, 4); break;    // bps 8
            case 2:    *p -= PREDICTOR1(*last, 5); break;    // bps 16
            case 3:    *p -= PREDICTOR1(*last, 5); break;    // bps 24
            } *last = tmp;

            // compress stage 2: adaptive hybrid filter
            hybrid_filter(fst, p, 1);

            value =ENC( *p);
            putc(value&0xff,fdout);
            putc(value>>8&0xff,fdout);
            putc(value>>16&0xff,fdout);
            putc(value>>24&0xff,fdout);
         
            if (enc < tta + num_chan - 1) enc++;
            else enc = tta;
        }     
    }
    free(WAVE_BUFFER);
    free(data);
    free(tta);
    return 0;
}

int decompress(FILE *fdin, FILE *fdout,int dlen, int info, int info2) {
    int *p, *data, value;
    unsigned int num_chan, data_size, byte_size, checksum;
    unsigned int buffer_len, framelen, lastlen, fframes;
    unsigned int k, depth, unary, binary = 0;
    unsigned int st_size, st_state, *st;
    unsigned int def_subchunk_size = 16;
    encoder *tta, *enc;
 
    int bits=(((info&31)%4)/2)*8+8;

    //get SampleRate
    int wavsr=0;
    switch (info>>5) {
      case 3: wavsr=44100; break;
      case 2: wavsr=22050; break;
      case 1: wavsr=11025; break;
      case 0: wavsr=8000; break;
      default: wavsr = -1;
    }
    if (wavsr==-1) quit("encode wav wsr");
    num_chan = (info&31)%2+1;
    byte_size = (bits + 7) / 8;
    framelen = (int) (FRAME_TIME * wavsr);
    int dlen1=info2/ (byte_size * num_chan);
    data_size = dlen1 * byte_size * num_chan;

    lastlen = dlen1 % framelen;
    fframes = dlen1 / framelen + (lastlen ? 1 : 0);
    st_size = (fframes + 1);
    st_state = 0;
    buffer_len = num_chan * framelen;

    // grab some space for a buffer
    data = (int *) tta_malloc(buffer_len, sizeof(int));
    enc = tta =(encoder*) tta_malloc(num_chan, sizeof(encoder));
    WAVE_BUFFER = (unsigned char *) tta_malloc(buffer_len, byte_size);

    while (fframes--) {
        if (!fframes && lastlen)
            buffer_len = num_chan * (framelen = lastlen);

        encoder_init(tta, num_chan, byte_size);
        for (p = data; p < data + buffer_len; p++) {
            fltst *fst = &enc->fst;
            int *last = &enc->last;
            value=getc(fdin);
            value|=getc(fdin)<<8;
            value|=getc(fdin)<<16;
            value|=getc(fdin)<<24;
            *p =DEC( value);

            // decompress stage 1: adaptive hybrid filter
            hybrid_filter(fst, p, 0);

            // decompress stage 2: fixed order 1 prediction
            switch (byte_size) {
            case 1: *p += PREDICTOR1(*last, 4); break;    // bps 8
            case 2: *p += PREDICTOR1(*last, 5); break;    // bps 16
            case 3: *p += PREDICTOR1(*last, 5); break;    // bps 24
            } *last = *p;

            if (enc < tta + num_chan - 1) enc++;
            else {
                if (num_chan > 1) {
                    int *r = p - 1;
                    for (*p += *r/2; r > p - num_chan; r--)
                        *r = *(r + 1) - *r;
                }
                enc = tta;
            }
        }     
            write_wave(data, byte_size, num_chan, buffer_len, fdout) ;
    }
    free(WAVE_BUFFER);
    free(data);
    free(tta);
    return 0;
}

