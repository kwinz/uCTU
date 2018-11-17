#include "hal_wt41_fc_uart.h"
#include "buffer.h"
#include "tools.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

/*
Port B SPI
Port D USART1
Port H USART2
Port J USART3
every USART can operate also in SPI mode
*/

//#define MYUBRR (FOSC / 16 / BAUD - 1)

// https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/avr/iomxx0_1.h

/*
void USART_Init(unsigned int ubrr) {
  // Set baud rate
  UBRR3H = (unsigned char)(ubrr >> 8);
  UBRR3L = (unsigned char)ubrr;
  // Enable receiver and transmitter
  UCSR3B = (1 << RXEN3) | (1 << TXEN3);
  // Set frame format: 8data, 2stop bit
  // FIXME: UCSZ03??
  UCSR3C = (1 << USBS3) | (3 << UCSZ02);
}
*/

#define RTS_PIN 2
#define CTS_PIN 3

static ringbuffer buf;
static volatile uint8_t sendbuffer;
static volatile bool resetting = false;
static volatile bool sending = false;
static volatile bool rcvCallbacksRunning = false;

// output CTS high (set) indicates that we currently cannot handle anymore data.
static void setCTS() { PORTJ |= (1 << CTS_PIN); }
static void clearCTS() { PORTJ &= ~(1 << CTS_PIN); }

// When the WT 41 Bluetooth module sets RTS to high, no more data must be sent to the module.
static bool isRTS() { return (PORTJ & (1 << RTS_PIN)) ? true : false; }

static void uart_1M(void) {

  {
    UCSR3A = 0;
    UCSR3B = 0;
    UCSR3C = 0;
  }

  // set UART3 with a baudrate of 1 Mbit/s
  // page 231 of ATmega1280 manual
  {
    const unsigned int ubrr = 0;
    UBRR3H = (unsigned char)(ubrr >> 8);
    UBRR3L = (unsigned char)ubrr;
    // clear U2X3
    UCSR3A &= ~(1 << U2X3);
  }

  /*
  no parity, 1 stop bit (i.e., 8N1), and hardware flow control (RTS/CTS) in both
  directions
  */
  {

    UCSR3B |= (1 << TXCIE3); //|(1 << RXCIE3); //|
    UCSR3B |= (1 << RXCIE3);

    // 8 bits/frame (011)
    UCSR3B &= ~(1 << UCSZ32);
    UCSR3C |= (1 << UCSZ31 | 1 << UCSZ30);

    // Asynchronous USART
    UCSR3C &= ~(1 << UMSEL31 | 1 << UMSEL30);

    // Partiy disabled
    UCSR3C &= ~(1 << UPM31 | 1 << UPM30);

    // 1 stop bit
    UCSR3C &= ~(1 << USBS3);

    // enable all interrupts
    // UCSR3B |= (1 << RXCIE3) | (1 << TXCIE3) | (1 << UDRIE3);

    // enable receiver
    UCSR3B |= (1 << RXEN3);

    // enable transmitter
    UCSR3B |= (1 << TXEN3);
  }
}

// /usr/share/doc/avr-libc/examples/stdiodemo/

static void (*sndCallbackGlobal)();

static void (*rcvCallbackGlobal)(uint8_t);

error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) {

  DDRE = 0xFF;
  PORTE = 0x00;

  sndCallbackGlobal = sndCallback;
  rcvCallbackGlobal = rcvCallback;

  buffer_init(&buf);

  // RTS is an input
  DDRJ &= ~(1 << RTS_PIN);
  // CTS is an output
  DDRJ |= (1 << CTS_PIN);

  uart_1M();

  // reset bluetooth module
  {
    resetting = true;
    // DDRJ = 0xFF;
    DDRJ = DDRJ | (1 << 5);
    PORTJ = PORTJ & ~(1 << 5);
    busyWaitMS(5);
    PORTJ = PORTJ | (1 << 5);
    resetting = false;
  }

  // setup RTS interrupt
  {
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT11);
  }

  clearCTS();

  // UCSR3A |= (1 << RXC3);
  // PORTB = UCSR3A;

  sei();

  PORTE |= (1 << 0);
  return SUCCESS;
}

void static send(const uint8_t out) {
  /* Put data into buffer, sends the data */
  UDR3 = out;
  PORTE |= (1 << 1);
}

static bool hwTXBuferEmpty() {
  // Data Register Empty - The UDREn Flag indicates if the transmit buffer (UDRn) is ready to
  // receive new data.
  return UCSR3A & (1 << UDRE3);
}

static void trySendingBufferedValue() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // should we feed the hw a new byte immediately?
    if (sending && !isRTS()) {
      send(sendbuffer);
      sending = false;
    }
  }
}

error_t halWT41FcUartSend(uint8_t byte) {
  // while (!(UCSR3A & (1 << UDRE3)));

  PORTE |= (1 << 2);

  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  if (sending) {
    // assert(false);
  }
  if (hwTXBuferEmpty() && !isRTS()) {
    send(byte);
  } else {
    sendbuffer = byte;
    sending = true;
  }
  //}

  return SUCCESS;
}

ISR(USART3_RX_vect, ISR_BLOCK) {
  PORTE |= (1 << 5);
  // Rx Complete
  // PORTE = ~PORTE;
  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  const uint8_t in = UDR3;
  const bool successful = buffer_put(&buf, in);

  const uint8_t count = buffer_count(&buf);
  if (BUF_SIZE - count <= 5) {
    setCTS();
  } else if (count < BUF_SIZE / 2) {
    clearCTS();
  }
  //}

  // ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
  if (rcvCallbacksRunning) {
    return;
  } else {
    rcvCallbacksRunning = true;
  }
  //}

  while (true) {
    uint8_t out;
    bool successful;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { successful = buffer_take(&buf, &out); }
    if (!successful) {
      rcvCallbacksRunning = false;
      return;
    }
    PORTE |= (1 << 4);
    sei();
    rcvCallbackGlobal(out);
    cli();
  }
}

// ISR(USART3_UDRE_vect, ISR_NOBLOCK) {
// Data register Empty
//}

ISR(USART3_TX_vect, ISR_BLOCK) {
  // previous byte was written

  PORTE |= (1 << 6);

  // trySendingBufferedValue();

  // send callback for the previous written byte
  // sei();
  sndCallbackGlobal();
  // cli();
}

// RTS PJ2 (XCK3/PCINT11)
// CTS PJ3 (PCINT12)
// Any change on any enabled PCINT15:8 pin will cause an inter-rupt.
// But we have only enabled RTS PJ2 (XCK3/PCINT11)
ISR(PCINT1_vect, ISR_BLOCK) {
  PORTE |= (1 << 3);
  trySendingBufferedValue();
}