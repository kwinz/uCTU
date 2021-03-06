#include "hal_wt41_fc_uart.h"
#include "tools.h"

#include <stdbool.h>
#include <stdint.h>

#include "timer_utils.h"
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

#define BUF_SIZE 128
static volatile uint8_t buffer[BUF_SIZE];
static volatile uint8_t reader = 0;
static volatile uint8_t writer = 0;
static volatile bool calling = false;
static volatile uint8_t sendbuffer;
static volatile bool resetting = true;
static volatile bool bufferedSendBytePresent = false;

static bool hwTXBuferEmpty() {
  // Data Register Empty - The UDREn Flag indicates if the transmit buffer (UDRn) is ready to
  // receive new data.
  return UCSR3A & (1 << UDRE3);
}

static bool isRTS() { return PINJ & (1 << RTS_PIN); }

static void trySendFromISR() {
  if (!resetting && bufferedSendBytePresent && hwTXBuferEmpty() && !isRTS()) {
    UDR3 = sendbuffer;
    bufferedSendBytePresent = false;
    sei();
    sndCallbackGlobal();
  }
}

void stopReset() {
  TIMSK1 &= ~_BV(OCIE1A);
  PORTJ = PORTJ | (1 << 5);
  resetting = false;
  trySendFromISR();
  sei();
}

error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) {
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
    // reset for 5ms = 5000us
    start16BitTimer(TIMER1, 5000U, &stopReset);
  }
  return SUCCESS;
}

error_t halWT41FcUartSend(uint8_t byte) {
  cli();
  if (!resetting && hwTXBuferEmpty() && !isRTS()) {
    UDR3 = byte;
    sei();
    sndCallbackGlobal();
  } else {
    sendbuffer = byte;
    bufferedSendBytePresent = true;
  }
  sei();
  return SUCCESS;
}

ISR(USART3_RX_vect) {
  buffer[writer] = UDR3;
  writer = (writer + 1) % BUF_SIZE;

  uint8_t elements = writer > reader ? writer - reader : BUF_SIZE - reader + writer;
  if (elements > BUF_SIZE - 5) {
    PORTJ |= (1 << CTS_PIN);
  } else if (elements < BUF_SIZE / 2) {
    PORTJ &= ~(1 << CTS_PIN);
  }

  if (calling) {
    sei();
    return;
  } else
    calling = true;
  do {
    sei();
    rcvCallbackGlobal(buffer[reader]);
    cli();
    reader = (reader + 1) % BUF_SIZE;
  } while (writer != reader);
  calling = false;
  sei();
}

ISR(USART3_TX_vect) {
  trySendFromISR();
  sei();
}

ISR(PCINT1_vect) {
  trySendFromISR();
  sei();
}