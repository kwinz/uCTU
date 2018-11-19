#include "debug.h"
#include "font.h"
#include "glcd.h"
#include "mp3.h"
#include "rand.h"
#include "sdcard.h"
#include "spi.h"
#include "tools.h"
#include "wii_user.h"

#include "hal_wt41_fc_uart.h"

#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <math.h>

extern const font Standard5x7;
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
    // PORTA = 0x0F;
    // fail();
  } else {
    // PORTA++;
    wiiUserConnect(wii, _mac[0], &conCallback);
  }
}

sdcard_block_t buffer;
volatile uint32_t byteAddress = 5460224LU;
volatile uint8_t count = 0;

static void dataRequestCallback(void) {
  //++PORTH;
}

void setup() {
  // DDRA = 0xFF;
  // PORTA = 0x00;
  DDRH = 0xFF;
  PORTH = 0x00;

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

  sei();

  spiInit();

  sdcardInit();

  mp3Init(&dataRequestCallback);
  // mp3SetVolume(0xA0);

  // PORTD++;

  mp3SetVolume(0xC0);

  sdcardReadBlock(byteAddress, buffer);
  while (mp3Busy())
    ;
  mp3SendMusic(buffer);
  byteAddress += 32;

  // mp3StartSineTest();
}

void background() {
  if (!mp3Busy()) {
    sdcardReadBlock(byteAddress, buffer);
    mp3SendMusic(buffer);
    byteAddress += 32;
  }

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

  // xy_point a = {0, 0};
  // y_point b = {50, rand16() % 50U};
  xy_point c = {5, 50};

  // glcdDrawLine(a, b, &glcdInvertPixel);

  glcdDrawText("olol", c, &Standard5x7, &glcdInvertPixel);

  if (rand16() < 200) {
    glcdSetYShift(glcdGetYShift() + 1);
  }
}

int main(void) {
  setup();

  while (true) {
    background();
  }
  return 0;
}