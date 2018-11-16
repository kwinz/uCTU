#include "glcd.h"
#include "rand.h"
#include "tools.h"
#include "wii_user.h"
#include <stdbool.h>
#include <stdio.h>

#include <avr/io.h>
#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 1000000UL
#define USART_UBBR_VALUE ((F_CPU / (USART_BAUD << 4)) - 1)
#include <util/setbaud.h>

void rcvButton(uint8_t wii, uint16_t buttonStates) {}
void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z) {}
void conCallback(uint8_t wii, connection_status_t status) {}

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

  /*
  UART3 with a baudrate of 1 Mbit/s, 8
  data bits, no parity, 1 stop bit (i.e., 8N1), and hardware flow control (RTS/CTS) in both
  directions
  */

  UBRR3H = UBRRH_VALUE;
  UBRR3L = UBRRL_VALUE;
#if USE_2X
  UCSR3A |= (1 << U2X3);
#else
  UCSR3A &= ~(1 << U2X3);
#endif
}

void USART_Transmit(unsigned char data) {
  /* Wait for empty transmit buffer */
  while (!(UCSR3A & (1 << UDRE3)))
    ;
  /* Put data into buffer, sends the data */
  UDR3 = data;
}

void setup() {
  DDRA = 0xFF;
  PORTA = 0xAF;

  USART_Init(MYUBRR);
  uart_1M();

  uint8_t wii = 1;
  const uint8_t mac = 1;

  error_t ret = wiiUserInit(&rcvButton, &rcvAccel);
  ret = wiiUserConnect(wii, &mac, &conCallback);
}

void background() {}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}