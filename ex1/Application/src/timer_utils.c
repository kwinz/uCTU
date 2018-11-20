#include "timer_utils.h"
#include "tools.h"
#include <avr/interrupt.h>
#include <avr/io.h>
//#include <util/atomic.h>

/*
17.3 Accessing 16-bit Registers
page 138

Accessing the low byte triggers the 16-bit read or write operation. When the low byte of
a 16-bit register is written by the CPU, the high byte stored in the Temporary Register, and the
low byte written are both copied into the 16-bit register in the same clock cycle. When the low
byte of a 16-bit register is read by the CPU, the high byte of the 16-bit register is copied into
the Temporary Register in the same clock cycle as the low byte is read. Not all 16-bit accesses uses
the Temporary Register for the high byte. Reading the OCRnA/B/C 16-bit registers does not involve
using the Temporary Register. To do a 16-bit write, the high byte must be written before the low
byte. For a 16-bit read, the low byte must be read before the high byte

Note by Markus: atomic TCNTn, OCRnA/B/C or ICRn reads have to be done with interrupts disabled.
*/

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

// timer0,2: 8 bit
// timer1,3,4,5: 16 bit

// Timer/Counter0 is a general purpose 8-bit Timer/Counter module, with two independent Output
// Compare Units, and with PWM support.

// Timer/Counter2 is a general purpose, single channel, 8-bit Timer/Counter module.

// The 16-bit Timer/Counter unit allows accurate program execution timing
// (event management), wave generation, and signal timing measurement.

// Timer/Counter 0, 1, 3, 4, and 5 share the same prescaler module, but the Timer/Counters can have
// different prescaler settings. The description below applies to all Timer/Counters. Tn is used as
// a general name, n = 0, 1, 3, 4, or 5.

// In Clear Timer on Compare or CTC mode (WGM22:0 = 2), the OCR2A Register is used to manipulate the
// counter resolution. In CTC mode the counter is cleared to zero when the counter value (TCNT2)
// matches the OCR2A.

// we have to be able to make 5ms
// 16.6ms

void setup16BitTimer(void) {
  cli();
  /* Set up timer interrupt */
  TCCR1A &= ~(_BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0) | _BV(COM1C1) | _BV(COM1C0) |
              _BV(WGM11) | _BV(WGM10));
  TCCR1B &= ~(_BV(ICNC1) | _BV(WGM13) | _BV(ICES1) | _BV(CS12) | _BV(CS11) | _BV(CS10));
  OCR1A = ((250 * F_CPU) / (64 * 1000U)) - 1;
  TIMSK1 |= _BV(OCIE1A);
  TCCR1B |= _BV(CS11) | _BV(CS10) | _BV(WGM12);
  sei();
}