#include "game.h"
#include "font.h"
#include "glcd.h"
#include "lcd.h"
#include "mp3.h"
#include "rand.h"
#include "tools.h"
#include <avr/interrupt.h>

static volatile bool moveRight = false;
static volatile uint8_t speed = 0;
extern volatile GameState_t gamestate;

// game state
static uint16_t ticks = 0;
static uint8_t yshift = 0;
static xy_point ball = {5, 50};
static xy_point old_ball = {5, 50};

typedef struct lry_point_t {
  uint8_t l, r, y;
  bool active;
} lry_point;

static lry_point obstacle0 = {0, 0, 10, false}, obstacle1 = {0, 0, 30, false},
                 obstacle2 = {0, 0, 50, false};

// static functions

// callbacks, ISRs and/or public interfaces

/**
 * This function expects p1 to be the upper left corner
 * and p2 to be the lower right corner
 *
 */
void glcdFillRectWrappingSafe(xy_point p1, xy_point p2,
                              void (*drawPx)(const uint8_t, const uint8_t)) {

  const uint8_t orig_p1y = p1.y, orig_p2y = p2.y;
  /*
  if (p1.y > 80) {

    glcdFillRect(p1, p2, drawPx);

    glcdFillRect(p1, p2, drawPx);
  } else
  */
  if (p2.y > 63) {
    p2.y = 63;
    glcdFillRect(p1, p2, drawPx);
    p1.y = 0;
    p2.y = orig_p2y % 64;
    glcdFillRect(p1, p2, drawPx);
  } else {
    glcdFillRect(p1, p2, drawPx);
  }
}

void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {
  // https://wiibrew.org/wiki/Wiimote#Accelerometer
  // Note that X has 10 bits of precision, while Y and Z only have 9.

  // PORTL = hi8(y);

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

  // PORTK = speed;

  sei();
}

void gameStart(void) {
  glcdFillScreen(GLCD_CLEAR);
  yshift = 0;
}

void gameTick(void) {
  ticks++;

  // delete old ball
  {
    xy_point left_rect = ball;
    xy_point right_rect = ball;
    right_rect.x += 4;
    right_rect.y += 4;
    glcdFillRectWrappingSafe(left_rect, right_rect, &glcdClearPixel);
    // glcdFillRect(, 1, &glcdClearPixel);
  }

  const uint8_t top = yshift;
  const uint8_t bottom = (top + 63) % 64;

  // move ball horizontally
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
    // ball.x %= 128;

    if (ball.x < 1 || ball.x > 150) {
      ball.x = 1;
    } else if (ball.x > 120) {
      ball.x = 120;
    }
  }

  // move ball down
  if (ticks % 6 == 0) {
    PORTK = ball.y;
    PORTL = bottom;

    if ((ball.y + 6) % 64 == obstacle0.y && obstacle0.l < ball.x && ball.x < obstacle0.r) {
    } else if ((ball.y + 6) % 64 == bottom) {
    } else {
      ball.y++;
      ball.y %= 64;

      // if (bottom < ball.y + 6) {
      //  ball.y = bottom - 6;
      //} else
    }

    // are we dead?
    if (ball.y == top) {
      gamestate = DEAD_ENTER;
      return;
    }
  }

  // draw new ball
  {
    xy_point left_rect = ball;
    xy_point right_rect = ball;
    right_rect.x += 4;
    right_rect.y += 4;
    glcdFillRectWrappingSafe(left_rect, right_rect, &glcdSetPixel);
  }

  if (ticks % 10 == 0) {
    glcdDrawHorizontal(yshift, &glcdClearPixel);
    yshift++;
    yshift %= 64;
    glcdSetYShift(yshift);
    const uint8_t top = yshift;
    const uint8_t bottom = (top + 63) % 64;

    if (obstacle0.y == top) {
      const uint8_t left = rand16() % 64;
      const uint8_t right = rand16() % 64 + 63;
      obstacle0.l = left;
      obstacle0.r = right;
      obstacle0.y = bottom;
      const xy_point left_rect = {left, bottom};
      const xy_point right_rect = {right, bottom};
      // glcdFillRect(left_rect, right_rect, &glcdSetPixel);
      glcdDrawLine(left_rect, right_rect, &glcdSetPixel);
      obstacle0.active = true;
    }
  }
}