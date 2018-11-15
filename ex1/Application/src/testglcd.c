#include "glcd.h"
#include "rand.h"
#include "wii_user.h"
#include <stdbool.h>

void rcvButton(uint8_t wii, uint16_t buttonStates) {}
void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z) {}
void conCallback(uint8_t wii, connection_status_t status) {}

void setup() {
  DDRA = 0xFF;
  PORTA = 0xFF;

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