#include "im1bit.hpp"

// This model is from paq8px

//////////////////////////// im1bitModel /////////////////////////////////
// Model for 1-bit image data

im1bitModel1::im1bitModel1( BlockData& bd,U32 val): x(bd),buf(bd.buf),
r0(0),r1(0),r2(0),r3(0),r4(0),r5(0),r6(0),r7(0),r8(0),
t(C),N(18+1+1), cxt(N),counts(C),mapL({{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2},{ 16, 128*2}} ),
row(0),col(0),im( {21, 1, 128, 127}) {

    sm=new StateMap[N];

    // Set model mixer contexts and parameters
    mxp.push_back( { 256,64,0,28,&mxcxt[0],0,false} );
    mxp.push_back( { 256,64,0,28,&mxcxt[1],0,false} );
    mxp.push_back( { 256,64,0,28,&mxcxt[2],0,false} );
    mxp.push_back( { 256,64,0,28,&mxcxt[3],0,false} );
    mxp.push_back( {  16,64,0,28,&mxcxt[4],0,false} );
    mxp.push_back( {  64,64,0,28,&mxcxt[5],0,false} );
    mxp.push_back( {  41,64,0,28,&mxcxt[6],0,false} );
    mxp.push_back( {   8*4,64,0,28,&mxcxt[7],0,false} );
    mxp.push_back( {4096,64,0,28,&mxcxt[8],0,false} );
}
void im1bitModel1::add(Mixers& m, uint32_t n0, uint32_t n1) {
    int p1 = ((n1 + 1) << 12) / ((n0 + n1) + 2);
    m.add(stretch(p1) >> 0);
}

int im1bitModel1::p(Mixers& m,int w, int val2) {
    // update the model
    int i;
    for (i=0; i<N; i++) {
        t[cxt[i]]=nex(t[cxt[i]],x.y);
        
        uint32_t c = counts[cxt[i]];
        int cnt0 = (c >> 16) & 65535;
        int cnt1 = (c & 65535);

        if (x.y == 0) cnt0++; else cnt1++;
        if (cnt0 + cnt1 >= 65536) { cnt0 >>= 1; cnt1 >>= 1; } //prevent overflow in add()
        
        c = cnt0 << 16 | cnt1;
        counts[cxt[i]] = c;
    }
    
    // update the contexts (pixels surrounding the predicted one)
    r0+=r0+x.y;
    r1+=r1+((x.buf(w*1-1)>>x.bposshift)&1);
    r2+=r2+((x.buf(w*2-1)>>x.bposshift)&1);
    r3+=r3+((x.buf(w*3-1)>>x.bposshift)&1);
    r4+=r4+((x.buf(w*4-1)>>x.bposshift)&1);
    r5+=r5+((x.buf(w*5-1)>>x.bposshift)&1);
    r6+=r6+((x.buf(w*6-1)>>x.bposshift)&1);
    r7+=r7+((x.buf(w*7-1)>>x.bposshift)&1);
    r8+=r8+((x.buf(w*8-1)>>x.bposshift)&1);
    col++;
    if (col/8==w) row++,col=0;
    int c = 0; //base for each context
    const int y=x.y;
    i = 0; //context index

    //   x
    //  x?
    cxt[i++] = c + (y | (r1 >> 8 & 1) << 1);
    c += 1 << 2;

    //  xxx
    //  x?
    uint32_t surrounding4 = y | (r1 >> 7 & 7) << 1;
    cxt[i++] = c + surrounding4;
    c += 1 << 4;

    //    x
    //    x
    //  xx?
    cxt[i++] = c + ((r0 & 3) | (r1 >> 8 & 1) << 2 | ((r2 >> 8) & 1) << 3);
    c += 1 << 4;

    //    x
    //    x
    //    x
    // xxx?
    uint32_t mCtx6 = ((r0 & 7) | (r1 >> 8 & 1) << 3 | (r2 >> 8 & 1) << 4 | (r3 >> 8 & 1) << 5);
    cxt[i++] = c + mCtx6;
    c += 1 << 6;

    // +
    //     
    //     x x
    //     xxxx
    // x  ?
    cxt[i++] = c +( (r0>>2)&1)|(((r1>>1)&0xF0)>>3)|(((r2>>3)&0xA)>>1);
    c += 1 << 7;

    //  xx 
    //   xxx
    // xxx?
    cxt[i++] = c + (r0 & 7) | (r1 >> 7 & 7) << 3 | (r2 >> 9 & 3) << 6;
    c += 1 << 8; 

    //   x
    //   x
    //  xxxxx
    //  x?
    cxt[i++] = c + (y | (r1 >> 5 & 0x1f) << 1 | (r2 >> 8 & 1) << 6 | (r3 >> 8 & 1) << 7);
    c += 1 << 8;

    //   xx
    //   xx
    //   xxx
    //  x?
    cxt[i++] = c + (y | (r1 >> 6 & 7) << 1 | (r2 >> 7 & 3) << 4 | (r3 >> 7 & 3) << 6);
    c += 1 << 8;

    //  x
    //  x
    //  x
    //  x
    //  x
    //  x
    //  x
    //  x
    //  ?
    cxt[i++] = c + ((r1 >> 8 & 1) << 7 | (r2 >> 8 & 1) << 6 | (r3 >> 8 & 1) << 5 | (r4 >> 8 & 1) << 4 | (r5 >> 8 & 1) << 3 | (r6 >> 8 & 1) << 2 | (r7 >> 8 & 1) << 1 | (r8 >> 8 & 1));
    c += 1 << 8;
    // +
    //    x
    //    x
    //    x
    //    x
    //    x
    //    x
    //    x
    //    x
    //  ?
    cxt[i++] = c + (((r1 >> 6 & 1) << 7 | (r2 >> 6 & 1) << 6 | (r3 >> 6 & 1) << 5 | (r4 >> 6 & 1) << 4 | (r5 >> 6 & 1) << 3 | (r6 >> 6 & 1) << 2 | (r7 >> 6 & 1) << 1 | (r8 >> 6 & 1)));
    c += 1 << 8;
    //+
    //    x   x
    // xxx xxx 
    // ?
    cxt[i++] = c + (r1 & 0xee)+(r2  & 0x11);
    c += 1 << 8;

    // xxxxxxxx?
    cxt[i++] = c + (r0 & 0xff);
    c += 1 << 8;

    //  xx
    //  xx
    //  xx
    //  xx
    //  x?
    cxt[i++] = c + (y | (r1 >> 8 & 3) << 1 | (r2 >> 8 & 3) << 3 | (r3 >> 8 & 3) << 5 | (r4 >> 8 & 3) << 7);
    c += 1 << 9;

    //  xxxxxx
    //   xxxx?
    cxt[i++] = c + ((r0 & 0x0f) | (r1 >> 8 & 0x3f) << 4);
    c += 1 << 10;

    //       xx
    //   xxxxxxx
    //  xxx?
    cxt[i++] = c + ((r0 & 7) | (r1 >> 4 & 0x7f) << 3 | ((r2 >> 5) & 3) << 10);
    c += 1 << 12;

    // xxxxx   
    // xxxxx
    // xx?
    uint32_t surrounding12 = (r0 & 3) | (r1 >> 6 & 0x1f) << 2 | (r2 >> 6 & 0x1f) << 7;
    cxt[i++] = c + surrounding12;
    c += 1 << 12;

    // xxxxxxx   
    // xxxxxxx   
    // xxxxxxx
    // xxx?
    uint32_t surrounding24 = (r0 & 7) | (r1 >> 5 & 0x7f) << 3 | (r2 >> 5 & 0x7f) << 10 | (r3 >> 5 & 0x7f) << 17;

    // xxxxxxxxx
    // x.......x
    // x.......x
    // x.......x
    // x...?
    uint32_t frame16 = (r0 >> 3 & 1) | (r1 >> 12 & 1) << 1 | (r2 >> 12 & 1) << 2 | (r3 >> 12 & 1) << 3 | (r4 >> 4 & 0x1ff) << 4 | (r3 >> 4 & 1) << 13 | (r2 >> 4 & 1) << 14 | (r1 >> 4 & 1) << 15;

    //contexts: surrounding pixel counts (most useful for dithered images)
    int bitcount04 = BitCount(surrounding4); //bitcount for the surrounding 4 pixels
    int bitcount12 = BitCount(surrounding12); //... 12 pixels
    int bitcount24 = BitCount(surrounding24); //... 24 pixels
    int bitcount40 = BitCount(frame16) + bitcount24; //...40 pixels

    cxt[i++] = c + bitcount04; 
    c += 5;  

    cxt[i++] = c + bitcount12; 
    c += 13;

    cxt[i++] = c + bitcount24; 
    c += 25; 

    cxt[i++] = c + bitcount40; 
    c += 41; 

    assert(i == N);
    assert(c == C);

    //     xxxxx
    //    xxxxxxx
    // xxxxxxxxxxxxxx
    // xxxxxx?
    mapL[0].set(hash((r0 & 0x3f), (r1 >> 1 & 0x1fff), (r2 >> 5 & 0x7f), (r3 >> 6 & 0x1f))); // 6+13+7+5=31 bits

    //         xx
    //         xx
    //   xxxxxxxx
    // xxxxxxxx?
    mapL[1].set(hash((r0 & 0xff), (r1 >> 7 & 0xff), (r2 >> 7 & 3)<<2 | (r3 >> 7 & 3))); // 8+8+2+2=20 bits

    //    xx
    //    xx
    //    xx
    //    xx
    //    xx
    //    xx
    //    xx
    //    xx
    //   x?
    mapL[2].set(hash(y | (r1 >> 7 & 3) << 1, (r2 >> 7 & 3) | (r3 >> 7 & 3) << 2, (r4 >> 7 & 3) | (r5 >> 7 & 3) << 2, (r6 >> 7 & 3) | (r7 >> 7 & 3) << 2, (r8 >> 7 & 3)));  // 8*2+1=17 bits hashed to 16 bits

    //  xxxxxxx
    //  xxxxxxx
    //  xxxxxxx
    //  xxx?
    mapL[3].set(hash(surrounding24)); // 24 bits

    // xxxxxxxxx
    // xxxxxxxxx
    // xxxxxxxxx
    // xxxxxxxxx
    // xxxx?
    mapL[4].set(hash(surrounding24, frame16,((r0 & 7) | (r1 >> 4 & 0x7f) << 3 | ((r2 >> 5) & 3) << 10))); // 40 bits
    mapL[5].set(hash(surrounding12,  bitcount24,r2>> 9)); // 40 bits

    mapL[0].mix(m);
    mapL[1].mix(m);
    mapL[2].mix(m);
    mapL[3].mix(m);
    mapL[4].mix(m);
    mapL[5].mix(m);
    im.set(hash(surrounding12,  bitcount40));
    im.mix2(m);
    // predict
    for (i=0; i<N; i++) {
        const int state=t[cxt[i]];
        if (state) {
            m.add(stretch(sm[i].p3(state,x.y,3))>>0);
            uint32_t n0 = (counts[cxt[i]] >> 16) & 65535;
            uint32_t n1 = (counts[cxt[i]]) & 65535;
            add(m, n0, n1);
        }
        else m.add(0), m.add(0);
    }

    //for dithered images
    add(m, bitcount04, 04 - bitcount04);
    add(m, bitcount12, 12 - bitcount12);
    add(m, bitcount24, 24 - bitcount24);
    add(m, bitcount40, 40 - bitcount40);
    mxcxt[0]=(r0&0x7)|(r1>>4&0x38)|(r2>>3&0xc0);
    mxcxt[1]=((r1&0x30)^(r3&0x0c))|(r0&3);
    mxcxt[2]=(r0&1)|(r1>>4&0x3e)|(r2>>2&0x40)|(r3>>1&0x80);
    mxcxt[3]=(r0&0x3e)^((r1>>8)&0x0c)^((r2>>8)&0xc8);
    mxcxt[4]=surrounding4-1; // disabled if =0
    mxcxt[5]=mCtx6;
    mxcxt[6]=bitcount40-1; // disabled if =0
    mxcxt[7]=x.bpos+((r0 & 0xff)==0)*8+((r1 & 0xff)==0)*16;
    mxcxt[8]=surrounding12;
    return 0;
}
