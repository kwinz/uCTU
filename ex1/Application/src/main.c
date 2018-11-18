#include "debug.h"
#include "glcd.h"
#include "rand.h"
#include "tools.h"
#include "wii_user.h"

#include "hal_wt41_fc_uart.h"

#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

extern const uint8_t _mac[1][6];

void setRumblerCallback(uint8_t wii, error_t status) {}
void rcvButton(uint8_t wii, uint16_t buttonStates) {
  wiiUserSetRumbler(wii, false, &setRumblerCallback);
}
void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z) {}
void conCallback(uint8_t wii, connection_status_t status) {
  if (status == CONNECTED) {
    PORTA = 0x0F;
    // fail();
  } else {
    PORTA++;
    wiiUserConnect(wii, _mac[0], &conCallback);
  }
}

void setup() {
  DDRA = 0xFF;
  PORTA = 0x00;
  DDRD = 0xFF;
  PORTD = 0x00;

  uint8_t wii = 0;
  error_t ret = wiiUserInit(&rcvButton, &rcvAccel);
  if (ret != SUCCESS) {
    PORTA = 0xAA;
    fail();
  }
  ret = wiiUserConnect(wii, _mac[0], &conCallback);
  if (ret != SUCCESS) {
    PORTA = 0xAA;
    fail();
  }
}

void background() {
  // sleep_enable();
  // sei();
  // sleep_cpu();
  // sleep_disable();
}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}