#include "game.h"
#include "font.h"
#include "glcd.h"
#include "lcd.h"
#include "mp3.h"
#include "rand.h"
#include "tools.h"
#include <avr/interrupt.h>

static uint16_t ticks = 0;

static volatile bool moveRight = false;
static volatile uint8_t speed = 0;

// game starte
static xy_point ball = {5, 50};
static xy_point old_ball = {5, 50};

typedef struct lry_point_t {
  uint8_t l, r, y;
  bool active;
} lry_point;

static lry_point obstacle0 = {0, 0, 10, false}, obstacle1 = {0, 0, 30, false},
                 obstacle2 = {0, 0, 50, false};

static uint8_t yshift = 0;

// static functions

// callbacks, ISRs and/or public interfaces

void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {
  // https://wiibrew.org/wiki/Wiimote#Accelerometer
  // Note that X has 10 bits of precision, while Y and Z only have 9.

  PORTL = hi8(y);

  // dispUint8(hi8(x), 0, 0);
  // fail();

  cli();

  if (hi8(y) == 1) {
    // ball.x -= 1; //((x & 0xFF) / 128);
    moveRight = false;
    speed = ((y)&0x0ff);
  } else {
    // ball.x += 1; //((x & 0xFF) / 128);
    moveRight = true;
    speed = ~((y)&0x0ff);
  }

  PORTK = speed;

  sei();
}

void gameStart(void) {
  glcdFillScreen(GLCD_CLEAR);
  yshift = 0;
}

void gameTick(void) {
  ticks++;

  glcdDrawCircle(ball, 1, &glcdClearPixel);
  glcdDrawCircle(ball, 2, &glcdClearPixel);

  bool moveball = false;
  if (speed > 40) {
    moveball = ticks % 2 == 0;
  } else if (speed > 30) {
    moveball = ticks % 5 == 0;
  } else if (speed > 20) {
    moveball = ticks % 10 == 0;
  } else if (speed > 10) {
    moveball = ticks % 20 == 0;
  }

  if (moveball) {
    ball.x += moveRight ? 1 : -1;
    ball.x %= 128;
  }

  if (ball.y != obstacle0.y) {
    ball.y++;
  }

  glcdDrawCircle(ball, 1, &glcdSetPixel);
  glcdDrawCircle(ball, 2, &glcdSetPixel);

  if (ticks % 10 == 0) {
    glcdDrawHorizontal(yshift, &glcdClearPixel);
    yshift++;
    glcdSetYShift(yshift);
    const uint8_t top = yshift;
    const uint8_t bottom = top + 63;

    if (obstacle0.y == top) {
      const uint8_t left = rand16() % 64;
      const uint8_t right = rand16() % 64 + 63;
      obstacle0.l = left;
      obstacle0.r = right;
      obstacle0.y = bottom;
      const xy_point left_rect = {left, bottom};
      const xy_point right_rect = {right, bottom - 1};
      glcdDrawRect(left_rect, right_rect, &glcdSetPixel);
      obstacle0.active = true;
    }
  }
}