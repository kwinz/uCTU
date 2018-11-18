#include "spi.h"

#include <avr/io.h>

void spiInit(void) {
  /*The SPI module implements a driver for the hardware SPI module of the ATmega1280. Use Mode 0
(CPOL=0, CPHA=0) and send the MSB first.*/

  // DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK);
  DDRB = (1 << PB1) | (1 << PB2);
  SPCR = (1 << SPE) | (1 << MSTR);
  SPSR &= ~(1 << SPI2X);
  spiSetPrescaler(SPI_PRESCALER_4);
}
void spiSend(uint8_t data) {
  /*Sends one byte via the SPI module. As the SPI data rate is very high, the overhead for setting
up an interrupt is often larger than the actual time required to send the data. Therefore, you are
allowed to use busy waiting to wait for a transfer to complete.*/
  SPDR = data;
  // When a serial transfer is complete, the SPIF Flag is set.
  while (!(SPSR & (1 << SPIF)))
    ;
}
uint8_t spiReceive(void) {
  /*
  Receives one byte via the SPI module. As with the send function you may use busy waiting here.
Note that the SD Card needs the pattern 0xFF to be sent as dummy value during read.*/
  SPDR = 0xFF;
  // When a serial transfer is complete, the SPIF Flag is set.
  while (!(SPSR & (1 << SPIF)))
    ;
  return SPDR;
}

void spiSetPrescaler(spi_prescaler_t prescaler) {
  switch (prescaler) {
  case (SPI_PRESCALER_128): {
    SPCR |= (1 << SPR1) | (1 << SPR0);
  } break;
  case (SPI_PRESCALER_4): {
    SPCR &= ~((1 << SPR1) | (1 << SPR0));
  } break;
  case (SPI_PRESCALER_16): {
    SPCR |= (1 << SPR0);
    SPCR &= ~(1 << SPR1);
  }
  }
}