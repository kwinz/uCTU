#include "adc.h"
#include "debug.h"
#include "font.h"
#include "game.h"
#include "glcd.h"
#include "lcd.h"
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
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include <math.h>

extern const font Standard5x7;
extern const uint8_t _mac[1][6];

bool rumbler = false;

volatile GameState_t gamestate = CONNECT_PAINT;

volatile bool wantTick = false;

void newTick(void) { wantTick = true; }

void setRumblerCallback(const uint8_t wii, const error_t status) {
  if (status != SUCCESS) {
    PORTA = 0xAA;
    fail();
  }
  // wiiUserSetRumbler(wii, false, &setRumblerCallback);
}

#define BUTTON_1_BV 2
#define BUTTON_2_BV 1
#define BUTTON_A_BV 8
#define BUTTON_B_BV 4
#define BUTTON_MIN_BV 16

volatile uint16_t buttonStatesLast;
volatile uint16_t buttonsPressed;

void rcvButton(const uint8_t wii, const uint16_t buttonStates) {

  const uint16_t newButtonPresses = (buttonStates ^ buttonStatesLast) & buttonStates;
  buttonsPressed |= newButtonPresses;
  buttonStatesLast = buttonStates;
}

static int16_t rumblingTicks = -1;

static void setAccelCallback(uint8_t wii, error_t status) {}

void conCallback(const uint8_t wii, const connection_status_t status) {
  if (status == CONNECTED) {
    gamestate = MENU_PAINT;
    wiiUserSetAccel(wii, true, &setAccelCallback);

  } else {
    wiiUserConnect(wii, _mac[0], &conCallback);
  }
}

void setup() {
  // those are used as debugging leds
  // initialize as output and set to off
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

  // initLcd();
  // dispString("Booted7", 0, 0);
  glcdInit();
  // testGlcd();

  adcInit();

  songInit();

  sei();

  // main tick
  start16BitTimer(TIMER3, 4500U, &newTick);
}

static void songOver(Song_t song) {
  if (SONG_IMPRESSIVE == song) {
    songPlay(SONG_NOSONG, &songOver);
  } else {
    // repeat
    songPlay(song, &songOver);
  }
}

void background() {
  PORTL = getCurrentSong();

  if (HAVE_MP3_BOARD) {
    songTick();
  }
  syncScreen();

  if (unlikely(rumblingTicks > 0)) {
    --rumblingTicks;
    if (unlikely(0 == rumblingTicks)) {
      wiiUserSetRumbler(0, false, &setRumblerCallback);
      rumblingTicks = -1;
    }
  }

  cli();
  if (buttonsPressed & 1) {
    buttonsPressed = 0;
    sei();
    Song_t currentSong = getCurrentSong();
    if (currentSong != SONG_NOSONG) {
      songPlay(++currentSong, &songOver);
    }
  } else if (buttonsPressed & 2) {
    buttonsPressed = 0;
    sei();
    Song_t currentSong = getCurrentSong();
    if (currentSong != SONG_BATMAN) {
      songPlay(--currentSong, &songOver);
    }
  }
}

const char macFormat_p[] PROGMEM = "%02x:%02x:%02x:%02x:%02x:%02x";
const char presSync_p[] PROGMEM = "Press sync (or any)\nkey to connect!\0";
const char scoreFormat_p[] PROGMEM = "# %" PRIu8 ": %" PRIu16;

#define NUM_HIGHSCORES 5
uint16_t highscores[NUM_HIGHSCORES] = {0};

void insertHighScore(const uint16_t highscore) {
  uint8_t i = 0;
  uint16_t temp;
  for (; i < NUM_HIGHSCORES; i++) {
    if (highscore > highscores[i]) {
      temp = highscores[i];
      highscores[i] = highscore;
      i++;
      for (; i < NUM_HIGHSCORES; i++) {
        // swap
        highscores[i] ^= temp;
        temp ^= highscores[i];
        highscores[i] ^= temp;
      }
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

      if (HAVE_MP3_BOARD) {
        songPlay(SONG_BUBBLE, &songOver);
      }

      glcdFillScreen(GLCD_CLEAR);
      glcdSetYShift(00);
      xy_point c = {5, 10};
      glcdDrawTextPgm(presSync_p, c, &Standard5x7, &glcdSetPixel);
      c.y = 30;
      {
        // buffer of 6 characters + '/0' byte, see mac_format format string
        char macString[6 * 3 + 1];
        sprintf_P(macString, macFormat_p, _mac[0][0], _mac[0][1], _mac[0][2], _mac[0][3],
                  _mac[0][4], _mac[0][5]);
        glcdDrawText(macString, c, &Standard5x7, &glcdSetPixel);
      }

      gamestate = CONNECT;
    } break;
    case CONNECT: {
      sei();
    } break;
    case MENU_PAINT: {
      sei();
      if (HAVE_MP3_BOARD) {
        songPlay(SONG_HIMALAYAS, &songOver);
      }
      glcdSetYShift(0);
      glcdFillScreen(GLCD_CLEAR);
      xy_point c = {20, 20};
      glcdDrawText("Falling Ball!", c, &Standard5x7, &glcdSetPixel);
      c.y += Standard5x7.lineSpacing;
      glcdDrawText("Continue/Game: A", c, &Standard5x7, &glcdSetPixel);
      c.y += Standard5x7.lineSpacing;
      glcdDrawText("Highscore: -", c, &Standard5x7, &glcdSetPixel);
      c.y += Standard5x7.lineSpacing;
      glcdDrawText("Previous Track: 1", c, &Standard5x7, &glcdSetPixel);
      c.y += Standard5x7.lineSpacing;
      glcdDrawText("Next Track: 2", c, &Standard5x7, &glcdSetPixel);
      gamestate = MENU;
      wiiUserSetRumbler(0, true, &setRumblerCallback);
      rumblingTicks = 400;
    } break;
    case MENU: {
      if (buttonsPressed & BUTTON_A_BV) {
        buttonsPressed = 0;
        sei();
        gamestate = PLAYING_ENTER;
      }
      if (buttonsPressed & BUTTON_MIN_BV) {
        buttonsPressed = 0;
        sei();
        gamestate = HIGHSCORE_PAINT;
      }
    } break;
    case PLAYING_ENTER: {
      if (HAVE_MP3_BOARD) {
        songPlay(SONG_ZGAGA, &songOver);
      }
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
      if (HAVE_MP3_BOARD) {
        songPlay(SONG_IMPRESSIVE, &songOver);
      }
      glcdSetYShift(0);
      glcdFillScreen(GLCD_CLEAR);
      xy_point c = {20, 20};
      glcdDrawText("Your score was:", c, &Standard5x7, &glcdSetPixel);
      c.x = 40;
      c.y = 30;
      insertHighScore(score);
      glcdDrawText(int2string(score), c, &Standard5x7, &glcdSetPixel);
      gamestate = DEAD;
      wiiUserSetRumbler(0, true, &setRumblerCallback);
      rumblingTicks = 400;

    } break;
    case DEAD: {
      if (buttonsPressed & BUTTON_A_BV) {
        buttonsPressed = 0;
        sei();
        gamestate = MENU_PAINT;
      }
    } break;
    case HIGHSCORE_PAINT: {
      sei();
      glcdSetYShift(0);
      glcdFillScreen(GLCD_CLEAR);
      xy_point c = {10, 10};
      glcdDrawText("HIGHSCORES", c, &Standard5x7, &glcdSetPixel);
      gamestate = HIGHSCORE;
      {
        char scoreString[30];
        for (uint8_t i = 1; i <= NUM_HIGHSCORES; i++) {
          c.y += Standard5x7.lineSpacing;
          sprintf_P(scoreString, scoreFormat_p, i, highscores[i - 1]);
          glcdDrawText(scoreString, c, &Standard5x7, &glcdSetPixel);
        }
      }
    } break;
    case HIGHSCORE: {
      if (buttonsPressed & BUTTON_A_BV) {
        buttonsPressed = 0;
        sei();
        gamestate = MENU_PAINT;
      }
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