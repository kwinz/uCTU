#include "debug.h"
#include "tools.h"

#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define RTS_PIN 2
#define CTS_PIN 3

static void uart_1M(void) {

  UCSR0A = 0;
  UCSR0B = 0;
  UCSR0C = 0;
  UBRR0 = 0;

  // 8 bits/frame (011)
  UCSR0C |= (1 << UCSZ11 | 1 << UCSZ10);

  // enable transmitter
  UCSR0B |= (1 << TXEN1);

  UBRR0 = 103;
}

static bool hwTXBuferEmpty() {
  // Data Register Empty - The UDREn Flag indicates if the transmit buffer (UDRn) is ready to
  // receive new data.
  return UCSR0A & (1 << UDRE1);
}

void debug_init(void) { uart_1M(); }

static void myPutCh(char character) {
  while (!hwTXBuferEmpty())
    ;
  UDR1 = character;
}

void debug_puts(char *string) {
  DDRA = 0xff;
  PINA = 0xF0;
  for (;;) {
    if (*string == '\0') {
      myPutCh('\r');
      ++PINA;
      myPutCh('\n');
      ++PINA;
      return;
    } else {
      myPutCh(*string);
      ++PINA;
      string++;
    }
  }
}