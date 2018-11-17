#include "glcd.h"
#include "rand.h"
#include "tools.h"
#include "wii_user.h"

#include "hal_wt41_fc_uart.h"

#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>

void rcvButton(uint8_t wii, uint16_t buttonStates) {}
void rcvAccel(uint8_t wii, uint16_t x, uint16_t y, uint16_t z) {}
void conCallback(uint8_t wii, connection_status_t status) {}

static void mySndCallback() { halWT41FcUartSend(4U); }
static void myRcvCallback(uint8_t in) {}

void setup() {
  DDRA = 0xFF;
  PORTA = 0x00;

  halWT41FcUartInit(&mySndCallback, &myRcvCallback);
  sei();
  // while (true) {
  halWT41FcUartSend(4U);
  //}
  // uint8_t wii = 1;
  // const uint8_t mac = 1;
  // error_t ret = wiiUserInit(&rcvButton, &rcvAccel);
  // ret = wiiUserConnect(wii, &mac, &conCallback);
}

void background() {}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}