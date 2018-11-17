#include "glcd.h"
#include "rand.h"
#include "tools.h"
#include "wii_user.h"

#include "hal_wt41_fc_uart.h"

#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>

extern const uint8_t *_mac;

void setRumblerCallback(uint8_t wii, error_t status) {}
void rcvButton(uint8_t wii, uint16_t buttonStates) {
  wiiUserSetRumbler(wii, true, &setRumblerCallback);
}
void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z) {}
void conCallback(uint8_t wii, connection_status_t status) {}

void setup() {
  // DDRA = 0xFF;
  // PORTA = 0x00;

  uint8_t wii = 0;
  error_t ret = wiiUserInit(&rcvButton, &rcvAccel);
  ret = wiiUserConnect(wii, _mac, &conCallback);
}

void background() {}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}