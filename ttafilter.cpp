
// based on Alexander Djourik ttaenc v2.0 b20040108
#include <stdio.h>
#include <memory.h>
#define FRAME_TIME        1.04489795918367346939

#define MAX_BPS            32
#define MIN_LEVEL        1
#define MAX_LEVEL        3
#define DEF_LEVEL        2

#define SWAP16(x) (\
(((x)&(1<< 0))?(1<<15):0) | \
(((x)&(1<< 1))?(1<<14):0) | \
(((x)&(1<< 2))?(1<<13):0) | \
(((x)&(1<< 3))?(1<<12):0) | \
(((x)&(1<< 4))?(1<<11):0) | \
(((x)&(1<< 5))?(1<<10):0) | \
(((x)&(1<< 6))?(1<< 9):0) | \
(((x)&(1<< 7))?(1<< 8):0) | \
(((x)&(1<< 8))?(1<< 7):0) | \
(((x)&(1<< 9))?(1<< 6):0) | \
(((x)&(1<<10))?(1<< 5):0) | \
(((x)&(1<<11))?(1<< 4):0) | \
(((x)&(1<<12))?(1<< 3):0) | \
(((x)&(1<<13))?(1<< 2):0) | \
(((x)&(1<<14))?(1<< 1):0) | \
(((x)&(1<<15))?(1<< 0):0))

#define MAX_ORDER    32
#define BUF_SIZE    4096

#ifdef _WIN32
    typedef unsigned __int64 uint64;
#else
    typedef unsigned long long uint64;
#endif

#define PREDICTOR1(x, k)    ((int)((((uint64)x << k) - x) >> k))

typedef struct {
    int order;
    int mode;
    int shift;
    int round;
    int qm[MAX_ORDER];
    int dx[BUF_SIZE];
    int dl[BUF_SIZE];
    int *px;
    int *pl;
} fltst;

static fltst fst1, fst2, fst3;
///////// Filters Settings /////////
static int flt_set [3][3][4][3] = {
    8,10,0,  8,9,0,   8,10,0,  8,12,1,
    8,10,0,  8,11,0,  8,10,0,  16,12,1,
    8,10,0,  8,11,0,  8,10,0,  32,12,1,
    
    0,0,0,   0,0,0,   0,0,0,   0,0,0,
    16,10,1, 16,9,1,  16,10,1, 0,0,0,
    32,11,1, 32,11,1, 32,11,1, 0,0,0,
    
    0,0,0,   0,0,0,   0,0,0,   0,0,0,
    0,0,0,   0,0,0,   0,0,0,   0,0,0,
    0,0,0,   16,9,1,  16,10,1, 0,0,0
};

__inline void filter_compress (fltst *fs, int *in) {
    int *pA, *pB, *pE;
    int out, sum;
    pA = fs->pl;
    pB = fs->qm;
    pE = pA + fs->order;
    for (sum = fs->round; pA < pE;
    pA += 8, pB += 8) {
        sum += *(pA+0) * *(pB+0);
        sum += *(pA+1) * *(pB+1);
        sum += *(pA+2) * *(pB+2);
        sum += *(pA+3) * *(pB+3);
        sum += *(pA+4) * *(pB+4);
        sum += *(pA+5) * *(pB+5);
        sum += *(pA+6) * *(pB+6);
        sum += *(pA+7) * *(pB+7);
    }
    out = *in - (sum >> fs->shift);
    
    pA = fs->pl + fs->order;
    *pA = *in;
    if (!fs->mode) {
        pB = fs->pl + fs->order - 1;
        // adaptive polynomial predictors
        *(pB-0) = *(pA-0) - *(pB-0);
        *(pB-1) = *(pA-1) - *(pB-1);
        *(pB-2) = *(pA-2) - *(pB-2);
    }
    pA = fs->qm;
    pB = fs->px;
    pE = pA + fs->order;
    if (out < 0)
    for (; pA < pE;
    pA += 8, pB += 8) {
        *(pA+0) += *(pB+0);
        *(pA+1) += *(pB+1);
        *(pA+2) += *(pB+2);
        *(pA+3) += *(pB+3);
        *(pA+4) += *(pB+4);
        *(pA+5) += *(pB+5);
        *(pA+6) += *(pB+6);
        *(pA+7) += *(pB+7);
    } else if (out > 0)
    for (; pA < pE;
    pA += 8, pB += 8) {
        *(pA+0) -= *(pB+0);
        *(pA+1) -= *(pB+1);
        *(pA+2) -= *(pB+2);
        *(pA+3) -= *(pB+3);
        *(pA+4) -= *(pB+4);
        *(pA+5) -= *(pB+5);
        *(pA+6) -= *(pB+6);
        *(pA+7) -= *(pB+7);
    }
    pA = fs->px + fs->order;
    pB = fs->pl + fs->order;
    *(pA-0) = ((*(pB-0) >> 28) & 8) - 4;
    *(pA-1) = ((*(pB-1) >> 29) & 4) - 2;
    *(pA-2) = ((*(pB-2) >> 29) & 4) - 2;
    *(pA-3) = ((*(pB-3) >> 30) & 2) - 1;
    if (fs->px + fs->order == fs->dx + (BUF_SIZE-1)) {
        memcpy(fs->dx, fs->px + 1, fs->order *
        sizeof(int)); fs->px = fs->dx;
    } else     fs->px++;
    if (fs->pl + fs->order ==  fs->dl + (BUF_SIZE-1)) {
        memcpy(fs->dl, fs->pl + 1, fs->order *
        sizeof(int)); fs->pl = fs->dl;
    } else fs->pl++;
    
    *in = out;
}

__inline void filter_decompress (fltst *fs, int *in) {
    int *pA, *pB, *pE;
    int out, sum;
    pA = fs->pl;
    pB = fs->qm;
    pE = pA + fs->order;
    
    for (sum = fs->round;
    pA < pE; pA += 8, pB += 8) {
        sum += *(pA+0) * *(pB+0);
        sum += *(pA+1) * *(pB+1);
        sum += *(pA+2) * *(pB+2);
        sum += *(pA+3) * *(pB+3);
        sum += *(pA+4) * *(pB+4);
        sum += *(pA+5) * *(pB+5);
        sum += *(pA+6) * *(pB+6);
        sum += *(pA+7) * *(pB+7);
    }
    out = *in + (sum >> fs->shift);
    pA = fs->pl + fs->order;
    *pA = out;
    if (!fs->mode) {
        pB = fs->pl + fs->order - 1;
        // adaptive polynomial predictors
        *(pB-0) = *(pA-0) - *(pB-0);
        *(pB-1) = *(pA-1) - *(pB-1);
        *(pB-2) = *(pA-2) - *(pB-2);
    }
    pA = fs->qm;
    pB = fs->px;
    pE = pA + fs->order;
    if (*in < 0)
    for (; pA < pE;
    pA += 8, pB += 8) {
        *(pA+0) += *(pB+0);
        *(pA+1) += *(pB+1);
        *(pA+2) += *(pB+2);
        *(pA+3) += *(pB+3);
        *(pA+4) += *(pB+4);
        *(pA+5) += *(pB+5);
        *(pA+6) += *(pB+6);
        *(pA+7) += *(pB+7);
    } else if (*in > 0)
    for (; pA < pE;
    pA += 8, pB += 8) {
        *(pA+0) -= *(pB+0);
        *(pA+1) -= *(pB+1);
        *(pA+2) -= *(pB+2);
        *(pA+3) -= *(pB+3);
        *(pA+4) -= *(pB+4);
        *(pA+5) -= *(pB+5);
        *(pA+6) -= *(pB+6);
        *(pA+7) -= *(pB+7);
    }
    pA = fs->px + fs->order;
    pB = fs->pl + fs->order;
    *(pA-0) = ((*(pB-0) >> 28) & 8) - 4;
    *(pA-1) = ((*(pB-1) >> 29) & 4) - 2;
    *(pA-2) = ((*(pB-2) >> 29) & 4) - 2;
    *(pA-3) = ((*(pB-3) >> 30) & 2) - 1;
    if (fs->px + fs->order == fs->dx + (BUF_SIZE - 1)) {
        memcpy(fs->dx, fs->px + 1, fs->order *
        sizeof(int)); fs->px = fs->dx;
    } else     fs->px++;
    if (fs->pl + fs->order ==  fs->dl + (BUF_SIZE - 1)) {
        memcpy(fs->dl, fs->pl + 1, fs->order *
        sizeof(int)); fs->pl = fs->dl;
    } else fs->pl++;
    *in = out;
}

static void filter_init (fltst *fs, int order, int shift, int mode) {
    memset (fs, 0, sizeof(fltst));
    fs->px = fs->dx;
    fs->pl = fs->dl;
    fs->order = order;
    fs->shift = shift;
    fs->round = 1 << (shift - 1);
    fs->mode = mode;
}

void filters_compress (int *data, unsigned int len, int level, int byte_size) {
    int *p = data;
    int tmp, last;
    int *f1 = (int *)flt_set[0][level-1][byte_size-1];
    int *f2 = (int *)flt_set[1][level-1][byte_size-1];
    int *f3 = (int *)flt_set[2][level-1][byte_size-1];
    filter_init (&fst1, f1[0], f1[1], f1[2]);
    filter_init (&fst2, f2[0], f2[1], f2[2]);
    filter_init (&fst3, f3[0], f3[1], f3[2]);
    for (last = 0; p < data + len; p++) {
        // compress stage 1: fixed order 1 prediction
        tmp = *p; switch (byte_size) {
        case 1: *p -= PREDICTOR1(last, 4); break;    // bps 8
        case 2: *p -= PREDICTOR1(last, 5); break;    // bps 16
        case 3: *p -= PREDICTOR1(last, 5); break;    // bps 24
        case 4: *p -= last; break;                    // bps 32
        } last = tmp;
        // compress stage 2: adaptive hybrid filters
        if (fst1.order) filter_compress (&fst1, p);
        if (fst2.order) filter_compress (&fst2, p);
        if (fst3.order) filter_compress (&fst3, p);
    }
}

void filters_decompress (int *data, unsigned int len, int level, int byte_size) {
    int *p = data;
    int last;
    int *f1 = (int *)flt_set[0][level-1][byte_size-1];
    int *f2 = (int *)flt_set[1][level-1][byte_size-1];
    int *f3 = (int *)flt_set[2][level-1][byte_size-1];
    filter_init (&fst1, f1[0], f1[1], f1[2]);
    filter_init (&fst2, f2[0], f2[1], f2[2]);
    filter_init (&fst3, f3[0], f3[1], f3[2]);
    for (last = 0; p < data + len; p++) {
        // decompress stage 1: adaptive hybrid filters
        if (fst3.order) filter_decompress (&fst3, p);
        if (fst2.order) filter_decompress (&fst2, p);
        if (fst1.order) filter_decompress (&fst1, p);
        // decompress stage 2: fixed order 1 prediction
        switch (byte_size) {
        case 1: *p += PREDICTOR1(last, 4); break;    // bps 8
        case 2: *p += PREDICTOR1(last, 5); break;    // bps 16
        case 3: *p += PREDICTOR1(last, 5); break;    // bps 24
        case 4: *p += last; break;                    // bps 32
        } last = *p;
    }
}



void * malloc1d (size_t num, size_t size) {
    void    *array1;

    if ((array1 = calloc (num, size)) == NULL)
        quit("Error (tta): insufficient memory available\n");

    return (array1);
}

int ** malloc2d (int num, unsigned int len) {
    int    i, **array1, *tmp;

    array1 = (int **) calloc (num, sizeof( long*) + len * sizeof(int));
    if (array1 == NULL)     quit("Error (tta): insufficient memory available\n");

    for(i = 0, tmp = (int *) (array1 + num); i < num; i++)
        array1[i] = tmp + i * len;
    
    return (array1);
}

static int read_wave (int *data, int byte_size, unsigned int len, FILE *fdin) {
    unsigned int i, bytes_read = 0;
    void    *buffer;

    buffer = malloc1d (len + 2, byte_size);

    switch (byte_size) {
    case 1: {
                unsigned char *sbuffer =(unsigned char *) buffer;
                if ((bytes_read = fread (sbuffer, byte_size, len, fdin)) == 0) 
                        quit("Error (tta): can't read from input file\n");
                for (i = 0; i < bytes_read; i++) data[i] = (int) sbuffer[i] - 0x80;
                break;
            }
    case 2:    {
                short *sbuffer = (short *)buffer;
                if ((bytes_read = fread (sbuffer, byte_size, len, fdin)) == 0) 
                    quit("Error (tta): can't read from input file\n");
                for (i = 0; i < bytes_read; i++) data[i] = (int) sbuffer[i];
                break;    
            }
    case 3:    {
                unsigned char *sbuffer =(unsigned char *)buffer;
                if ((bytes_read = fread (sbuffer, byte_size, len, fdin)) == 0) 
                    quit("Error (tta): can't read from input file\n");
                for (i = 0; i < bytes_read; i++) {
                    unsigned int t = *((int *)(sbuffer + i * byte_size));
                    data[i] = (int) (t << 8) >> 8;
                }
                break;
            }
    case 4:    { 
                if ((bytes_read = fread (data, byte_size, len, fdin)) == 0) 
                    quit("Error (tta): can't read from input file\n");
                break;
            }
    }

    free (buffer);
    return (bytes_read);
}

static int write_wave (int **data, int byte_size, int num_chan, unsigned int len, FILE *fdout) {
    int    n;
    unsigned int    i, bytes_wrote = 0;
    void    *buffer;

    buffer = malloc1d (len * num_chan + 2, byte_size);

    switch (byte_size) {
    case 1: {
                unsigned char  *sbuffer = (unsigned char  *)buffer;
                for (i = 0; i < (len * num_chan); i+= num_chan)
                for (n = 0; n < num_chan; n++) sbuffer[i+n] = (unsigned char) (data[n][i/num_chan] + 0x80);
                bytes_wrote = fwrite (sbuffer, byte_size, len * num_chan, fdout);
                if (bytes_wrote == 0) quit("Error (tta): can't write to output file\n");
                break;
            }
    case 2: {
                short *sbuffer = (short *)buffer;
                for (i = 0; i < (len * num_chan); i+= num_chan)
                for (n = 0; n < num_chan; n++) sbuffer[i+n] = (short) data[n][i/num_chan];
                bytes_wrote = fwrite (sbuffer, byte_size, len * num_chan, fdout);
                if (bytes_wrote == 0) quit("Error (tta): can't write to output file\n");
                break;
            }
    case 3: {
                unsigned char *sbuffer = (unsigned char *)buffer;
                for (i = 0; i < (len * num_chan); i+= num_chan)
                for (n = 0; n < num_chan; n++) 
                    *((int *)(sbuffer + (i+n) * byte_size)) = data[n][i/num_chan];
                bytes_wrote = fwrite (sbuffer, byte_size, len * num_chan, fdout);
                if (bytes_wrote == 0) quit("Error (tta): can't write to output file\n");
                break;
            }
    case 4: {
                int *sbuffer =(int *)buffer;
                for (i = 0; i < (len * num_chan); i+= num_chan)
                for (n = 0; n < num_chan; n++) sbuffer[i+n] = data[n][i/num_chan];
                bytes_wrote = fwrite (sbuffer, byte_size, len * num_chan, fdout);
                if (bytes_wrote == 0) quit("Error (tta): can't write to output file\n");
                break;
            }
    }
    free (buffer);
    return (bytes_wrote);
}

void split_int (int *data, int frame_len, int num_chan, int **buffer) {
    int    i, j, n;

    for (i = 0; i < frame_len; i++)
    for (j = 0; j < num_chan; j++) {
        buffer[j][i] = data[i * num_chan + j];
    }

    if (num_chan > 1)
    for (i = 0, n = (num_chan - 1); i < frame_len; i++) {
        for (j = 0; j < n; j++)
            buffer[j][i] = buffer[j+1][i] - buffer[j][i];
        buffer[n][i] = buffer[n][i] - (buffer[n-1][i] / 2);
    }
}

void split_float (int *data, int frame_len, int num_chan, int **buffer) {
    int    i, j;

    for (i = 0; i < frame_len; i++) 
    for (j = 0; j < num_chan; j++) {
        unsigned int t = data[i * num_chan + j];
        unsigned int negative = (t & 0x80000000)? -1:1;
        unsigned int data_hi = (t & 0x7FFF0000) >> 16;
        unsigned int data_lo = (t & 0x0000FFFF);

        buffer[j][i] = data_hi - 0x3F80;
        buffer[j+num_chan][i] = (SWAP16(data_lo) + 1) * negative;
    }
}

int compress (FILE *fdin, FILE *fdout,int file_size,int info,int info2) {
    int    *data, **buffer;
    unsigned int    i, num_chan,  byte_size;
    unsigned int    len, frame_size, frame_len,data_size ;
    unsigned int    is_float;

    int wavsr=info2;
    
    frame_size    = (int) (FRAME_TIME * wavsr);
    int bits=(((info&31)%4)/2)*8+8;
    num_chan    = (info&31)%2+1;
    data_size    = file_size;
    byte_size    = (bits + 7) / 8;
    len            = data_size/byte_size;
    is_float = (info>>7); //no detection set 0 
    
    // grab some space for a buffers
    data = (int *) malloc1d (num_chan * frame_size, sizeof (int));
    buffer = malloc2d (num_chan << is_float, frame_size);

    while (len > 0) {
        frame_len = num_chan * frame_size;
        frame_len = (frame_len <= len)? frame_len:len;
        frame_len = read_wave (data, byte_size, frame_len, fdin);
        len -= frame_len;
        frame_len /= num_chan;

        if (is_float)
            split_float (data, frame_len, num_chan, buffer);
        else split_int (data, frame_len, num_chan, buffer);
        
        // compress block
        for (i = 0; i < (num_chan << is_float); i++) {
            filters_compress (buffer[i], frame_len, 3 /*level*/, byte_size);
            fwrite (buffer[i],1, frame_len*sizeof(int), fdout);
        }
    }
    free (buffer);
    free (data);
    return 0;
}

void combine_int (int frame_len, int num_chan, int **buffer) {
    int    i, j, n;
    if (num_chan > 1)
    for (i = 0, n = (num_chan - 1); i < frame_len; i++) {
        buffer[n][i] = buffer[n][i] + (buffer[n-1][i] / 2);
        for (j = n; j > 0; j--)
            buffer[j-1][i] = buffer[j][i] - buffer[j-1][i];
    }
}

void combine_float (int frame_len, int num_chan, int **buffer) {
    int    i, j;
    for (i = 0; i < frame_len; i++) 
    for (j = 0; j < num_chan; j++) {
        unsigned int negative = buffer[j+num_chan][i] & 0x80000000;
        unsigned int data_hi = buffer[j][i];
        unsigned int data_lo = abs(buffer[j+num_chan][i]) - 1;
        
        data_hi += 0x3F80;
        buffer[j][i] = (data_hi << 16) | SWAP16(data_lo) | negative;
    }
}

int decompress (FILE *fdin, FILE *fdout,int dlen, int info, int info2,int smr) {
    int    **buffer;
    unsigned int    i, num_chan, byte_size;
    unsigned int    is_float, frame_size, frame_len;
    unsigned int  len;
    
    int bits=(((info&31)%4)/2)*8+8;
    int tta_level        = 3;
    byte_size        = (bits + 7) / 8;
    //get SampleRate
    int wavsr=smr;
    
    frame_size        = (int) (FRAME_TIME * wavsr);
    num_chan        = (info&31)%2+1;
    len                = (info2/byte_size)/num_chan;
    is_float        = (info>>7);
    
    // grab some space for a buffer
    buffer = malloc2d (num_chan << is_float, frame_size);
    while (len > 0) {
        frame_len = (frame_size <= len)? frame_size:len;

        for (i = 0; i < (num_chan << is_float); i++) {
            fread (buffer[i], 1, frame_len*sizeof(int), fdin);
            filters_decompress (buffer[i], frame_len, tta_level, byte_size);
        }

        if (is_float) combine_float(frame_len, num_chan, buffer);
        else combine_int(frame_len, num_chan, buffer);

       write_wave(buffer, byte_size, num_chan, frame_len, fdout);

        len -= frame_len;
    }

    free (buffer);
    return 0;
}
