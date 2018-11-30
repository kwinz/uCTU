
//#include "/usr/lib/avr/include/avr/iom128.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

void setupTimer(){
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
    OCR0A = 254;
    TCNT0 = 0;
}

void setupUART(){
    //UCSR0A |= _BV();
    UCSR0A &= ~(_BV(U2X0)|_BV(MPCM0));

    UCSR0B |= _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0);
    UCSR0B &= ~(_BV(UDRIE0) | _BV(UCSZ02));

    UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0C &= ~(_BV(UMSEL01)|_BV(UMSEL00)  |_BV(UPM01) |_BV(UPM00) |_BV(USBS0) | _BV (UCPOL0));

    //9600 BAUD
    UBRR0  = 103;
}

void main(void){
    PORTA=0xAA;
    DDRA = DDRB = 0xFF;
    DDRC =0;

    
    ADMUX |= _BV(REFS0)|_BV(MUX0);
    ADMUX &= ~(_BV(REFS1)|_BV(ADLAR)|_BV(MUX4)|_BV(MUX3)|_BV(MUX2)|_BV(MUX1));

    //ADTS = auto trigger
    //ADCSRB |= _BV();
    ADCSRB &= ~(_BV(MUX5)|_BV(ADTS2)|_BV(ADTS1)|_BV(ADTS0));

    ADCSRA &= ~(_BV(ADATE));
    ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0)|_BV(ADIE)|_BV(ADEN)|_BV(ADSC);

    setupTimer();
    setupUART();

    sei();
    while(true);
}

ISR(TIMER0_COMPA_vect){
    PORTA++;
}

volatile uint8_t data=0;
volatile bool sendData=false;

ISR(USART0_RX_vect){
    data = UDR0 +1;
    if( !(UCSR0A & _BV(UDRE0)) ){
        sendData=true;
        //PORTB++;
        return; 
    }
        
    UDR0 = data;
}

ISR(USART0_TX_vect){
    if( (UCSR0A & _BV(UDRE0)) && sendData ){
        sendData=false;
        UDR0 = data;
    }
        
    return; 
}

ISR(ADC_vect){
    PORTB=ADC>>2;
    ADCSRA |= _BV(ADSC);
    //while(true);
}