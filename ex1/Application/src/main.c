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
bool rumbler = false;

void setRumblerCallback(const uint8_t wii, const error_t status) {}
void rcvButton(const uint8_t wii, const uint16_t buttonStates) {
  // 1 = 2
  // 2 = 1
  // 4 = B
  // 8 = A
  // 16 = -
  if (buttonStates == 4) {
    rumbler = !rumbler;
    wiiUserSetRumbler(wii, rumbler, &setRumblerCallback);
  }
}
void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {}
void conCallback(const uint8_t wii, const connection_status_t status) {
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

  glcdInit();

  /*
    rand_feed(true);
    rand_feed(true);
    rand_feed(false);
    rand_feed(true);
    rand_feed(true);
    rand_feed(false);
    rand_feed(true);
    */

  xy_point a = {0, 0};
  xy_point b = {50, rand16() % 50U};

  glcdDrawLine(a, b, &glcdInvertPixel);
}

void background() {

  /*
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
  */

  /*
  infinite_loop :
  ; goto sleep
  cli
  ldi temp , (1<<SE )
  out SMCR, temp
  sei
  sleep
  rjmp infinite_loop
  */

  xy_point a = {0, 0};
  xy_point b = {50, rand16() % 50U};

  glcdDrawLine(a, b, &glcdInvertPixel);
}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}