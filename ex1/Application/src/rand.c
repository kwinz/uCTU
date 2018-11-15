#include "rand.h"
#include <util/atomic.h>
#include <avr/io.h>

uint16_t lfsr = 1;
const uint16_t poly = 0x80e3;

uint8_t rand_shift(uint8_t in){
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t out=0;
        /*
        asm volatile("LDI %OUT, 0" "\n\t"
            "ROR %IN" "\n\t"
            "ROR %lfsr" "\n\t"
            "BRCC L_1" "\n\t"
            "INC %OUT" "\n\t"
            "EOR %lfsr %poly" "\n\t"
            "L_1=: " "\n\t"
            : "=&d" (out), "=&d" (in), "+&x" (lfsr)
            : "y" (poly)
        );
        */

        asm volatile(
            "ROR %3" "\n\t"
            "ROR 27" "\n\t"
            "ROR 26" "\n\t"
            "BRCC LN%=" "\n\t"
            "INC %0" "\n\t"
            "EOR 27, 31" "\n\t"
            "EOR 26, 30" "\n\t"
            "LN%=:" " \n\t"
            : "=&d" (out), "+x" (lfsr)
            : "z" (poly), "d" (in)
        );
        return out;
    }
     __builtin_unreachable();
}

uint8_t rand_shift_c(uint8_t in){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t out = lfsr & 1;
        lfsr >>= 1;
        //lfsr &= 0x8F;
        lfsr |= (in<<15);
        if (out){
            lfsr ^= poly;
        }
        return out;
    }
    __builtin_unreachable();
}

void rand_feed(uint8_t in){
    rand_shift(in&1);
}

uint8_t rand1(){
    return rand_shift(0);
}

uint16_t rand16(){
    uint16_t randomWord=0;

    for(int i = 0; i< 15; ++i){
        randomWord|=rand1();
        randomWord<<=1;
    }
    randomWord|=rand1();

    return randomWord;
}