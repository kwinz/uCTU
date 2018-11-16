#include "hal_wt41_fc_uart.h"
#include "tools.h"

#include <stdbool.h>

#include <avr/io.h>

/*
Port B SPI
Port D USART1
Port H USART2
Port J USART3
every USART can operate also in SPI mode
*/

#define MYUBRR (FOSC / 16 / BAUD - 1)

// https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/avr/iomxx0_1.h

void USART_Init(unsigned int ubrr) {
  /* Set baud rate */
  UBRR3H = (unsigned char)(ubrr >> 8);
  UBRR3L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR3B = (1 << RXEN3) | (1 << TXEN3);
  /* Set frame format: 8data, 2stop bit */
  // FIXME: UCSZ03??
  UCSR3C = (1 << USBS3) | (3 << UCSZ02);
}

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
    // enable receiver
    UCSR3B |= (1 << RXEN3);

    // enable transmitter
    UCSR3B |= (1 << TXEN3);

    // 8 bits/frame (011)
    UCSR3B &= ~(1 << UCSZ32);
    UCSR3C |= (1 << UCSZ31 || 1 << UCSZ30);

    // Asynchronous USART
    UCSR3C &= ~(1 << UMSEL31 || 1 << UMSEL30);

    // Partiy disabled
    UCSR3C &= ~(1 << UPM31 || 1 << UPM30);

    // 1 stop bit
    UCSR3C &= ~(1 << USBS3);
  }

  // /usr/share/doc/avr-libc/examples/stdiodemo/
}

void USART_Transmit(unsigned char data) {
  /* Wait for empty transmit buffer */
  while (!(UCSR3A & (1 << UDRE3)))
    ;
  /* Put data into buffer, sends the data */
  UDR3 = data;
}

// When the WT 41 Bluetooth module sets RTS to high, no more data must be sent to the module.
#define RTS_PIN 2

// output CTS high indicates that we currently cannot handle anymore data.
#define CTS_PIN 3

// needs to be <=255
#define BUF_SIZE 64

uint8_t receive_buffer[BUF_SIZE];
bool full = false;

uint8_t reader = 0;
uint8_t writer = BUF_SIZE - 1;

uint8_t next(uint8_t current) {
  if (current == BUF_SIZE - 1) {
    return 0;
  } else {
    return ++current;
  }
}

bool isFull() { return (reader == writer) && full }

bool isEmpty() { return (reader == writer) && !full }

bool count() {
  if (isFull()) {
    return BUF_SIZE;
  }
  if (isEmpty()) {
    return 0;
  }

  return (reader > writer) ? (reader - writer) : (writer - reader);
}

/**
 *
 * @return true if successful, retry on on false
 */
bool put_buffer(const uint8_t in) {
  const bool full = isFull();
  if (full) {
    return false;
  }

  receive_buffer[writer] = in;

  writer = next(writer);
  if (writer == reader) {
    full = true;
  }

  return true;
}

bool take_buffer(uint8_t *const out) {
  const bool empty = isEmpty();

  if (empty) {
    return false;
  }

  *out = receive_buffer[reader];

  reader = next(reader);
  if (writer == reader) {
    full = false;
  }

  return true;
}

/*
 This functions initializes UART3 as listed above, and prepares the ringbuffer of the receiving
part. It also resets the Bluetooth module by pulling the reset pin PJ5 low for 5 ms. Whenever the
module receives a character from the Bluetooth module it has to be put it into a ringbuffer. The
buffer should be able to hold at least 32 bytes. For every character put in the ringbuffer, the
rcvCallback callback function must be called. Re-enable the interrupts before calling the callback
function. Make sure you do not call rcvCallback again before the previous call has returned.
If there are less than 5 bytes free in the buffer, the HAL module should trigger the flow control,
by setting CTS to high, to indicate that it currently cannot handle anymore data. If the buffer gets
at least half empty (less than 16 bytes stored in the case of a 32 byte buffer) the module should
release flow control by setting CTS to low.*/
error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) {

  // RTS is an input
  DDRJ &= ~(1 << RTS_PIN);
  // CTS is an output
  DDRJ |= (1 << CTS_PIN);

  uart_1M();

  // reset bluetooth module
  {
    // DDRJ = 0xFF;
    DDRJ = DDRJ | (1 << 5);
    PORTJ = PORTJ & ~(1 << 5);
    busyWaitMS(1000);
    PORTJ = PORTJ | (1 << 5);
  }

  return SUCCESS;
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
error_t halWT41FcUartSend(uint8_t byte) { return SUCCESS; }
