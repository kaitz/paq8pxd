#pragma once
#include "types.hpp"
#include "log.hpp"
#include <assert.h>
#include <cinttypes>

// Adaptive Error Info

struct ErrorInfo {
    uint32_t data[2], sum, mask, collected;
    int32_t rates=14;
    int32_t direction=0;
    void reset() {
        sum=mask=collected=0;
        data[0]=data[1]=0;
    }
    inline U32 SQR(U32 x) {
        return x*x;
    }
    bool stat(int pr, int y,int sh=0) {
        bool change=false;
        int err=((y<<12)-pr)>>sh;

        U32 logErr=min(0xF,ilog2(abs(err)));
        sum-=SQR(data[1]>>28);
        data[1]<<=4; data[1]|=data[0]>>28;
        data[0]<<=4; data[0]|=logErr;
        sum+=SQR(logErr);
        collected+=collected<4096;
        mask<<=1; 
        mask|=(logErr<=((data[0]>>4)&0xF));
        U32 count=BitCount(mask);
        if (collected>=64 && (sum>1500+uint32_t(rates)*32 || count<9 || (mask&0xFF)==0)) {
            reset();
            rates=min(14,rates*2-(rates*3/4));// rates*5/4=rates*2-(rates*3/4);
            direction--;
            change=true;
        }
        else if (collected==4096 && sum>=56 && sum<=144 && count>28-uint32_t(rates) && ((mask&0xFF)==0xFF)) {
            rates-=rates>2;
            direction++;
            reset();
            change=true;
        }
        return change;
    }
};
