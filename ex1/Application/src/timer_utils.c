#include "timer_utils.h"

/*
        ; timer counter control register A
        ; start clock with mode CTC
        ; WGM1{3:0} = 0100	(CTC)
        ; CS1{2:0} = 100	(prescaler)
        lds	16, TCCR1A
        andi temp, ~((1<<WGM10)|(1<<WGM11))
        sts TCCR1A, temp
        lds temp, TCCR1B
        ori temp, (1<<WGM12) | (1<<CS12)
        andi temp, ~((1<<WGM13) | (1<<CS11) | (1<<CS10))
        sts TCCR1B, temp

        ; output compare register A
        ldi temp, hi8(1250)
        ldi r17, lo8(1250)
        sts OCR1AH, temp
        sts OCR1AL, r17

        ; timer counter
        ldi temp, 0x00
        sts TCNT1H, temp
        sts TCNT1L, temp

        ; timer mask
        lds temp, TIMSK1
        ori temp, (1<<OCIE1A)
        sts TIMSK1, temp

        sei

*/

void setupTimer(void) {}