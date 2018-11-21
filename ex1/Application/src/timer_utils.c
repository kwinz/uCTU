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

/*
        fclk_I/O
f  = ----------------------
       N ⋅ ( 1 + OCRnA )

wherby N is the prescaler

*/

// we have to be able to make 5ms
// 16.6ms

void setup8BitTimer(void) {
  cli();

  // setup register A
  {
    // clear all bits except for the reserved ones
    // we don't want to use any compare match outputs
    uint8_t tccr2A_target = TCCR2A & ((1 < 2) | (1 < 3));
    // use CTC mode; see page 131 of manual
    tccr2A_target |= (1 << WGM01);
    TCCR2A = tccr2A_target;
  }

  // setup register B
  {
    uint8_t tccr2B_target = TCCR2B;
    // clear WGM02 for CTC
    tccr2B_target &= ~(1 << WGM02);
    /*
    CS02 CS01 CS00
    0 0 1         clk I/O /(No prescaling)
    0 1 0         clk I/O /8 (From prescaler)
    0 1 1         clk I/O /64 (From prescaler)
    1 0 0         clk I/O /256 (From prescaler)
    1 0 1         clk I/O /1024 (From prescaler)
    */
    // no prescaling
    tccr2B_target |= (1 << CS00);
    TCCR2B = tccr2B_target;
  }

  { OCR0A = 100; }

  TCNT0 = 0;

  // FIXME: unfinshed

  sei();
}

typedef enum { Prescaler8, Prescaler1024 } Prescaler_t;

static void (*periodicCallback1)(void);
static void (*periodicCallback3)(void);
static void (*periodicCallback4)(void);
static void (*periodicCallback5)(void);

// can sleep up to 4194304 micro-seconds (4.19s)
void start16BitTimer(Timer16Bit_t timer, uint32_t usec, void (*periodicCallback)(void)) {
  cli();

  /*
  use CTC mode; see page 148 of manual
  WGMn3 | WGMn2 (CTCn) | WGMn1 (PWMn1) | WGMn0 (PWMn0)
  CTC = 0 | 1 |  0 | 0
  */

  // setup register A
  {
    // clear all bits (including WGMn1 and WGMn0 for CTC)
    TCCR1A = 0;
    TCCR3A = 0;
    TCCR4A = 0;
    TCCR5A = 0;
  }

  uint16_t oCRnA;
  Prescaler_t prescaler;
  if (usec < 32768) {
    prescaler = Prescaler8;
    oCRnA = (F_CPU / 1000000UL / 8) * usec;
  } else {
    prescaler = Prescaler1024;
    oCRnA = (F_CPU / 1000000UL) * (usec / 1024);
  }

  // setup register B
  uint8_t tCCRnB;
  {
    // CTC
    tCCRnB = (1 << WGM32);

    /*
    CS02 CS01 CS00
    0 0 1         clk I/O /(No prescaling)
    0 1 0         clk I/O /8 (From prescaler)
    0 1 1         clk I/O /64 (From prescaler)
    1 0 0         clk I/O /256 (From prescaler)
    1 0 1         clk I/O /1024 (From prescaler)
    */
    // no prescaling
    if (prescaler == Prescaler8) {
      tCCRnB |= (1 << CS31);
    } else if (prescaler == Prescaler1024) {
      tCCRnB |= (1 << CS30);
      tCCRnB |= (1 << CS32);
    } else {
      PORTA = 0xAA;
      fail();
    }
  }

  switch (timer) {
  case (TIMER1): {
    OCR1A = oCRnA;
    periodicCallback1 = periodicCallback;
    // clear all bits except for reserved bit 5 and then add the bits from tCCRnB
    TCCR1B = (TCCR1B & (1 << 5)) | tCCRnB;
    TIMSK1 |= _BV(OCIE1A);
  } break;
  case (TIMER3): {
    OCR3A = oCRnA;
    periodicCallback3 = periodicCallback;
    TCCR3B = (TCCR3B & (1 << 5)) | tCCRnB;
    TIMSK3 |= _BV(OCIE3A);
  } break;
  case (TIMER4): {
    OCR4A = oCRnA;
    periodicCallback4 = periodicCallback;
    TCCR4B = (TCCR4B & (1 << 5)) | tCCRnB;
    TIMSK4 |= _BV(OCIE4A);
  } break;
  case (TIMER5): {
    OCR5A = oCRnA;
    periodicCallback5 = periodicCallback;
    TCCR5B = (TCCR5B & (1 << 5)) | tCCRnB;
    TIMSK5 |= _BV(OCIE5A);
  } break;
  default:
    PORTA = 0xAA;
    fail();
  }

  sei();
}

ISR(TIMER1_COMPA_vect) {
  periodicCallback1();
  // TIMSK1 &= ~_BV(OCIE1A);
  sei();
}

ISR(TIMER3_COMPA_vect) {
  periodicCallback3();
  sei();
}

ISR(TIMER4_COMPA_vect) {
  periodicCallback4();
  sei();
}

ISR(TIMER5_COMPA_vect) {
  periodicCallback5();
  sei();
}