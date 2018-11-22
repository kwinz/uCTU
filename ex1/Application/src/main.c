#include "adc.h"
#include "debug.h"
#include "font.h"
#include "game.h"
#include "glcd.h"
#include "lcd.h"
#include "mp3.h"
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

sdcard_block_t buffer;
volatile uint32_t byteAddress = 5460224LU;
volatile uint8_t count = 0;

static void dataRequestCallback(void) {
  // we are woken up from sleep due to the callback interrupt
  // don't do anything else here
}

void setup() {
  DDRH = 0xFF;
  PORTH = 0x00;
  DDRK = 0xFF;
  PORTK = 0x00;
  clearScreen();
  DDRL = 0xFF;
  PORTL = 0x00;
  clearScreen();

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
  glcdInit();
  sei();

  if (HAVE_MP3_BOARD) {
    spiInit();
    sdcardInit();
  }

  adcInit();

  if (HAVE_MP3_BOARD) {
    mp3Init(&dataRequestCallback);
    // mp3SetVolume(0xA0);
  }

  // mp3StartSineTest();

  start16BitTimer(TIMER3, 4500U, &newTick);
}

static int8_t oldVolume = 0;

void background() {

  if (HAVE_MP3_BOARD) {
    if (!mp3Busy()) {
      // PORTK++;
      cli();
      if (haveNewVolume) {
        haveNewVolume = false;
        if (oldVolume != volumeFromADC) {
          oldVolume = volumeFromADC;
          sei();
          mp3SetVolume(volumeFromADC);
        }
      }
      sei();

      // we disable interrupts during mp3 send because
      // we get audio glitches if someone else accesses the SPI
      sdcardReadBlock(byteAddress, buffer);
      mp3SendMusic(buffer);
      //
      byteAddress += 32;
    }
  }
}

int main(void) {
  setup();

  cli();
  while (true) {
    switch (gamestate) {
    case CONNECT_PAINT: {
      sei();

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