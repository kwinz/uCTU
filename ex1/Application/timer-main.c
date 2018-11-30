
//#include "/usr/lib/avr/include/avr/iom128.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

void main(void){
    PORTA=0xAA;
    DDRA = DDRB = 0xFF;
    DDRC =0;

    // mode
    TCCR0A |= _BV(WGM01);
    TCCR0A &= ~(_BV(WGM00));

    // mode + prescaler
    TCCR0B |= _BV(CS02)| _BV(CS00);
    TCCR0B &= ~(_BV(WGM02)| _BV(CS01));

    // interrupts
    TIMSK0 |= _BV(OCIE0A);
    TIMSK0 &= ~(_BV(OCIE0B)| _BV(TOIE0));

    //CTR match
    OCR0A = 250;
    TCNT0 = 0;

    sei();

    PORTA++;
    PORTB = TCNT0;

    while(true){
        if(PINC){
            PORTA=0;
            TCNT0 = 0;
        }
    }
}

ISR(TIMER0_COMPA_vect){
    PORTA++;
}