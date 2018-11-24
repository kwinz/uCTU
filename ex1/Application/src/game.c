#include "game.h"
#include "font.h"
#include "glcd.h"
#include "lcd.h"
#include "rand.h"
#include "tools.h"

#include <avr/interrupt.h>
#include <string.h>

static volatile bool moveRight = false;
static volatile uint8_t speed = 0;
extern volatile GameState_t gamestate;

// game state
static uint16_t ticks = 0;
static uint8_t yshift = 0;
static xy_point ball = {5, 50};
static uint8_t level = 0;

typedef struct lry_point_t {
  uint8_t l, r, y;
  bool active;
} lry_point;

#define OBSTACLE_COUNT 3

static lry_point obstacles[OBSTACLE_COUNT];
// obstacle1 = {0, 0, 30, false}, obstacle2 = {0, 0, 50, false};

// static functions

// callbacks, ISRs and/or public interfaces

/**
 * This function expects p1 to be the upper left corner
 * and p2 to be the lower right corner
 *
 */
void glcdFillRectWrappingSafe(xy_point p1, xy_point p2,
                              void (*drawPx)(const uint8_t, const uint8_t)) {

  // const uint8_t orig_p1y = p1.y;
  const uint8_t orig_p2y = p2.y;
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

static inline void drawBallOptimized(xy_point ball, void (*drawPx)(const uint8_t, const uint8_t)) {
  uint8_t x = ball.x, y = ball.y;
  drawPx(x + 1, y);
  drawPx(x + 2, y);

  y++;
  y %= 64;
  drawPx(x, y);
  drawPx(x + 1, y);
  drawPx(x + 2, y);
  drawPx(x + 3, y);

  y++;
  y %= 64;
  drawPx(x, y);
  drawPx(x + 1, y);
  drawPx(x + 2, y);
  drawPx(x + 3, y);

  y++;
  y %= 64;
  drawPx(x + 1, y);
  drawPx(x + 2, y);
}

static inline void drawHorizontalLineOptimized(uint8_t left, const uint8_t right, const uint8_t y,
                                               void (*drawPx)(const uint8_t, const uint8_t)) {
  for (; left <= right; left++) {
    drawPx(left, y);
  }
}

void gameStart(void) {
  glcdFillScreen(GLCD_CLEAR);

  ticks = 0;
  yshift = 0;
  ball = (xy_point){.x = 5, .y = 50};
  level = 0;

  memset(obstacles, 0, 3 * sizeof(lry_point));
  obstacles[0].y = 10;
  obstacles[1].y = 30;
  obstacles[2].y = 50;
}

void gameTick(void) {
  ticks++;

  // delete old ball
  drawBallOptimized(ball, &glcdClearPixel);

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
  if (ticks % (6 - min(level, 4)) == 0) {
    // PORTK = ball.y;
    // PORTL = bottom;

    bool colided = false;
    for (uint8_t i = 0; i < OBSTACLE_COUNT; ++i) {
      const lry_point obstacle = obstacles[i];
      if (likely(obstacle.active) && (ball.y + 6) % 64 == obstacle.y && obstacle.l < ball.x &&
          ball.x < obstacle.r) {
        colided = true;
        continue;
      }
    }

    if ((ball.y + 6) % 64 == bottom) {
      colided = true;
    }

    if (!colided) {
      ball.y++;
      ball.y %= 64;
    }

    // are we dead?
    if (ball.y == top) {
      gamestate = DEAD_ENTER;
      return;
    }
  }

  // draw new ball
  drawBallOptimized(ball, &glcdSetPixel);

  if (ticks % (10 - min(level, 7)) == 0) {
    glcdDrawHorizontal(yshift, &glcdClearPixel);
    yshift++;
    yshift %= 64;
    glcdSetYShift(yshift);
    const uint8_t top = yshift;
    const uint8_t bottom = (top + 63) % 64;

    for (uint8_t i = 0; i < OBSTACLE_COUNT; ++i) {
      if (obstacles[i].y == top) {
        const uint8_t left = rand16() % 64;
        const uint8_t right = rand16() % 64 + 63;
        obstacles[i].l = left;
        obstacles[i].r = right;
        obstacles[i].y = bottom;
        drawHorizontalLineOptimized(left, right, bottom, &glcdSetPixel);
        obstacles[i].active = true;
      }
    }
  }

  if (ticks % 1000 == 0) {
    level++;
  }
}