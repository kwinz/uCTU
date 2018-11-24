#include "adc.h"
#include "debug.h"
#include "font.h"
#include "game.h"
#include "glcd.h"
#include "lcd.h"
#include "mp3.h"
#include "music.h"
#include "rand.h"
#include "sdcard.h"
#include "spi.h"
#include "timer_utils.h"
#include "tools.h"
#include "util.h"
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

volatile GameState_t gamestate = CONNECT_PAINT;

volatile bool wantTick = false;

void newTick(void) { wantTick = true; }

void setRumblerCallback(const uint8_t wii, const error_t status) {}

void rcvButton(const uint8_t wii, const uint16_t buttonStates) {

  if (buttonStates & 8) {
    gamestate = PLAYING_ENTER;
  }
  // 1 = 2
  // 2 = 1
  // 4 = B
  // 8 = A
  // 16 = -
  // if (buttonStates & 4) {
  //  rumbler = !rumbler;
  //  wiiUserSetRumbler(wii, rumbler, &setRumblerCallback);
  //}
}

static void setAccelCallback(uint8_t wii, error_t status) {}

void conCallback(const uint8_t wii, const connection_status_t status) {
  if (status == CONNECTED) {
    gamestate = MENU_PAINT;
    wiiUserSetAccel(wii, true, &setAccelCallback);

  } else {
    wiiUserConnect(wii, _mac[0], &conCallback);
  }
}

static void dataRequestCallback(void) {
  // we are woken up from sleep due to the callback interrupt
  // don't do anything else here
}

void testGlcd() {

  glcdFillScreen(GLCD_FILL);
  glcdFillScreen(GLCD_CLEAR);

  glcdSetPixel(60, 20);
  glcdSetPixel(61, 21);
  glcdSetPixel(62, 22);
  glcdSetPixel(63, 23);
  glcdSetPixel(64, 24);
  glcdSetPixel(65, 25);
  glcdSetPixel(66, 26);
  glcdSetPixel(67, 27);
  glcdSetPixel(68, 28);
  glcdSetPixel(69, 29);

  {
    xy_point c = {20, 10};
    glcdDrawText("Hi Fraenk <3", c, &Standard5x7, &glcdSetPixel);
  }
  {
    xy_point p2 = {10, 10};
    xy_point p1 = {120, 60};
    glcdDrawLine(p1, p2, &glcdSetPixel);
  }

  {
    xy_point p1 = {10, 30};
    xy_point p2 = {120, 30};
    glcdDrawLine(p1, p2, &glcdSetPixel);
  }

  {
    xy_point p1 = {40, 10};
    xy_point p2 = {40, 60};
    glcdDrawLine(p1, p2, &glcdSetPixel);
  }

  {
    xy_point p2 = {120, 20};
    xy_point p1 = {30, 60};
    glcdDrawLine(p1, p2, &glcdSetPixel);
  }
}

void mySyncScreen() {
  sei();
  syncScreen();
}

void setup() {
  DDRH = 0xFF;
  PORTH = 0x00;
  DDRK = 0xFF;
  PORTK = 0x00;
  DDRL = 0xFF;
  PORTL = 0x00;

  gamestate = CONNECT_PAINT;

  if (HAVE_BLUETOOTH_BOARD) {
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

  initLcd();
  clearScreen();

  glcdInit();

  sei();

  // dispString("Booted7", 0, 0);

  // start16BitTimer(TIMER4, 10000U, &mySyncScreen);

  // testGlcd();
  // fail();
  //===========================

  if (HAVE_MP3_BOARD) {
    spiInit();

    const error_t sdcarderror = sdcardInit();
    if (SUCCESS != sdcarderror) {
      PORTA = 0xAA;

      fail();
    }

    PORTK++;
  }

  adcInit();

  if (HAVE_MP3_BOARD) {
    mp3Init(&dataRequestCallback);
    // mp3SetVolume(0xA0);
  }

  // mp3StartSineTest();

  start16BitTimer(TIMER3, 4500U, &newTick);

  //
}

void background() {

  if (HAVE_MP3_BOARD) {
    songTick();
  }
  syncScreen();
}

void songOver(Song_t song) {}

int main(void) {
  setup();

  cli();
  while (true) {
    switch (gamestate) {
    case CONNECT_PAINT: {
      sei();

      if (HAVE_MP3_BOARD) {
        songPlay(SONG_BATMAN, &songOver);
      }

      glcdFillScreen(GLCD_CLEAR);
      glcdSetYShift(00);
      xy_point c = {20, 10};
      glcdDrawText("press any key", c, &Standard5x7, &glcdSetPixel);
      c.y = 40;
      glcdDrawText("to connect", c, &Standard5x7, &glcdSetPixel);

      gamestate = CONNECT;
    } break;
    case CONNECT: {
      sei();
    } break;
    case MENU_PAINT: {
      sei();
      glcdSetYShift(0);
      glcdFillScreen(GLCD_CLEAR);
      xy_point c = {20, 20};
      glcdDrawText("New game: A", c, &Standard5x7, &glcdSetPixel);
      gamestate = MENU;
    } break;
    case MENU: {
      sei();
    } break;
    case PLAYING_ENTER: {
      sei();
      gameStart();
      gamestate = PLAYING;
    } break;
    case PLAYING: {
      if (wantTick) {
        wantTick = false;
        sei();
        gameTick();
      }
    } break;
    case DEAD_ENTER: {
      sei();
      glcdSetYShift(0);
      glcdFillScreen(GLCD_CLEAR);
      xy_point c = {20, 20};
      glcdDrawText("u ded", c, &Standard5x7, &glcdSetPixel);
      gamestate = DEAD;

    } break;
    case DEAD: {
      sei();
    } break;
    case HIGHSCORE_PAINT: {
      sei();
    } break;
    case HIGHSCORE: {
      sei();
    } break;
    default:
      sei();
      fail();
    }
    sei();

    background();

    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
  }
  return 0;
}