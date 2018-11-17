#include "hal_wt41_fc_uart.h"
#include "tools.h"

#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define RTS_PIN 2
#define CTS_PIN 3

static void uart_1M(void) {

  UCSR3A = 0;
  UCSR3B = 0;
  UCSR3C = 0;

  UBRR3 = 0;

  // 8 bits/frame (011)
  UCSR3C |= (1 << UCSZ31 | 1 << UCSZ30);

  // enable all interrupts
  UCSR3B |= (1 << RXCIE3) | (1 << TXCIE3);

  // enable receiver && enable transmitter
  UCSR3B |= (1 << RXEN3) | (1 << TXEN3);
}

static void (*sndCallbackGlobal)();
static void (*rcvCallbackGlobal)(uint8_t);

#define BUF_SIZE 32
static volatile uint8_t buffer[BUF_SIZE];
static volatile uint8_t reader = 0;
static volatile uint8_t writer = 0;
static volatile bool calling = false;
static volatile uint8_t sendbuffer;
static volatile bool resetting = false;
static volatile bool bufferedSendBytePresent = false;

static volatile int count = 0;

static bool hwTXBuferEmpty() {
  // Data Register Empty - The UDREn Flag indicates if the transmit buffer (UDRn) is ready to
  // receive new data.
  return UCSR3A & (1 << UDRE3);
}

static bool isRTS() { return PINJ & (1 << RTS_PIN); }

error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) {
  DDRC = 0xFF;
  PORTC = 0x00;
  DDRD = 0xFF;
  PORTD = 0x00;
  DDRH = 0xFF;
  PORTH = 0x00;

  sndCallbackGlobal = sndCallback;
  rcvCallbackGlobal = rcvCallback;

  uart_1M();

  // setup RTS interrupt
  {
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT11);
  }

  // RTS is an input
  DDRJ &= ~(1 << RTS_PIN);
  // CTS is an output
  DDRJ |= (1 << CTS_PIN);

  // reset bluetooth module
  {
    // DDRJ = 0xFF;
    DDRJ = DDRJ | (1 << 5);
    PORTJ = PORTJ & ~(1 << 5);
    busyWaitMS(5);
    PORTJ = PORTJ | (1 << 5);
  }

  cli();
  PORTC |= (1 << 0);
  sei();
  return SUCCESS;
}

error_t halWT41FcUartSend(uint8_t byte) {
  cli();
  PORTC |= (1 << 2);
  sei();

  if (!resetting && hwTXBuferEmpty() && !isRTS()) {
    UDR3 = byte;
  } else {
    cli();
    sendbuffer = byte;
    bufferedSendBytePresent = true;
    sei();
  }
  return SUCCESS;
}

ISR(USART3_RX_vect) {
  PORTC |= (1 << 5);
  buffer[writer] = UDR3;
  writer = (writer + 1) % BUF_SIZE;
  sei();
  // world's shortest busy wait
  ++count;
  ++count;
  cli();

  uint8_t elements = writer > reader ? writer - reader : BUF_SIZE - reader + writer;
  if (elements > BUF_SIZE - 5) {
    PORTJ |= (1 << CTS_PIN);
    // PORTC |= (1 << 3);
  } else if (elements < BUF_SIZE / 2) {
    PORTJ &= ~(1 << CTS_PIN);
    // PORTC |= (1 << 4);
  }

  if (calling)
    return;
  else
    calling = true;
  do {
    sei();
    rcvCallbackGlobal(buffer[reader]);
    cli();
    reader = (reader + 1) % BUF_SIZE;
  } while (writer != reader);
  calling = false;
}

static void trySendFromISR() {
  if (!resetting && bufferedSendBytePresent && hwTXBuferEmpty() && !isRTS()) {
    UDR3 = sendbuffer;
    bufferedSendBytePresent = false;
  }
}

ISR(USART3_TX_vect) {
  PORTC |= (1 << 6);

  // try to send new byte
  trySendFromISR();

  sei();
  // send callback for the previous written byte
  sndCallbackGlobal();
}

ISR(PCINT1_vect) { trySendFromISR(); }