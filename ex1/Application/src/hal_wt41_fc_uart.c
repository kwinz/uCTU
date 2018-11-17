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

static ringbuffer a;
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
    UCSR3B |= (1 << TXCIE3); //|(1 << RXCIE3); //|
    UCSR3B |= (1 << RXCIE3);

    // enable receiver
    UCSR3B |= (1 << RXEN3);

    // enable transmitter
    UCSR3B |= (1 << TXEN3);
  }
}

// /usr/share/doc/avr-libc/examples/stdiodemo/

static void (*sndCallbackGlobal)();

static void (*rcvCallbackGlobal)(uint8_t);

/*
 This functions initializes UART3 as listed above, and prepares the ringbuffer of the receiving
part. It also resets the Bluetooth module by pulling the reset pin PJ5 low for 5 ms. Whenever
the module receives a character from the Bluetooth module it has to be put it into a ringbuffer.
The buffer should be able to hold at least 32 bytes. For every character put in the ringbuffer,
the rcvCallback callback function must be called. Re-enable the interrupts before calling the
callback function. Make sure you do not call rcvCallback again before the previous call has
returned. If there are less than 5 bytes free in the buffer, the HAL module should trigger the
flow control, by setting CTS to high, to indicate that it currently cannot handle anymore data.
If the buffer gets at least half empty (less than 16 bytes stored in the case of a 32 byte
buffer) the module should release flow control by setting CTS to low.*/
error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) {

  DDRA = 0xFF;
  DDRB = 0xFF;
  PORTA = 0x00;

  sndCallbackGlobal = sndCallback;
  rcvCallbackGlobal = rcvCallback;

  buffer_init(&a);
  // buffer_put(&a, 5);
  // uint8_t out;
  // const bool successful = buffer_take(&a, &out);

  // RTS is an input
  DDRJ &= ~(1 << RTS_PIN);
  // CTS is an output
  DDRJ |= (1 << CTS_PIN);

  DDRJ |= (1 << 1);
  DDRJ &= ~(1 << 0);

  UCSR3A = 0;
  UCSR3B = 0;
  UCSR3C = 0;
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

  PORTA |= (1 << 0);
  return SUCCESS;
}

void static send(const uint8_t out) {
  /* Put data into buffer, sends the data */
  UDR3 = out;
  PORTA |= (1 << 1);
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

/*
error_t halWT41FcUartSend(uint8_t byte);
This function should send the byte given as a parameter to the Bluetooth module. The corresponding
sndCallback callback function will be called when the byte has been copied into the shift register
of the UART, i.e., the byte is currently being sent to the Bluetooth module. You have to ensure that
the callback is not called if there was no preceding send, e.g., after a reset.
When the WT 41 Bluetooth module sets RTS to high, no more data must be sent to the module.6 When the
Bluetooth module clears RTS, transmission of data to the module can resume. While the check for the
low RTS pin can be done right before a character is sent (copied into the UART data buffer), the
recognition of the change on the RTS pin from high to low has to be done interrupt-driven using a
pin change interrupt. In case halWT41FcUartSend is called while the reset of the WT 41 Bluetooth
module is still performed, the data bytes has to be buffered. After the reset has concluded, the
buffered bytes has to be sent and sndCallback called. This approach must also be taken when the
hardware flow control of the WT 41 (RTS) is active. The Bluetooth stack will send data byte-wise and
continue with the next byte only when sndCallback is called, so no extra buffering needs to be done.
*/
error_t halWT41FcUartSend(uint8_t byte) {
  // while (!(UCSR3A & (1 << UDRE3)));

  PORTA |= (1 << 2);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (sending) {
      // assert(false);
    }
    if (hwTXBuferEmpty() && !isRTS()) {
      send(byte);
    } else {
      sendbuffer = byte;
      sending = true;
    }
  }

  return SUCCESS;
}

// RTS PJ2 (XCK3/PCINT11)
// CTS PJ3 (PCINT12)
// Any change on any enabled PCINT15:8 pin will cause an inter-rupt.
// But we have only enabled RTS PJ2 (XCK3/PCINT11)
ISR(PCINT1_vect, ISR_NOBLOCK) {
  PORTA |= (1 << 3);
  if (isRTS()) {
    PORTA |= (1 << 7);
  } else {
    PORTA &= ~(1 << 7);
  }
  trySendingBufferedValue();
}

ISR(USART3_RX_vect, ISR_NOBLOCK) {
  PORTA |= (1 << 5);
  // Rx Complete
  // PORTA = ~PORTA;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    const uint8_t in = UDR3;
    const bool successful = buffer_put(&a, in);
    if (!successful) {
      // fail();
    }
    const uint8_t count = buffer_count(&a);
    if (BUF_SIZE - count <= 5) {
      setCTS();
    } else if (count < BUF_SIZE / 2) {
      clearCTS();
    }
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (rcvCallbacksRunning) {
      return;
    } else {
      rcvCallbacksRunning = true;
    }
  }

  while (true) {
    uint8_t out;
    bool successful;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { successful = buffer_take(&a, &out); }
    if (!successful) {
      rcvCallbacksRunning = false;
      return;
    }
    PORTA |= (1 << 4);
    rcvCallbackGlobal(out);
  }
}

// ISR(USART3_UDRE_vect, ISR_NOBLOCK) {
// Data register Empty
//}

ISR(USART3_TX_vect, ISR_NOBLOCK) {
  // previous byte was written

  PORTB = UCSR3A;

  PORTA |= (1 << 6);
  trySendingBufferedValue();

  // send callback for the previous written byte
  sndCallbackGlobal();
}