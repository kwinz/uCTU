#include "game.h"
#include "font.h"
#include "glcd.h"
#include "hal_glcd.h"
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
uint16_t score = 0;

typedef struct lry_point_t {
  uint8_t l, r, y;
  bool active;
} lry_point;

#define OBSTACLE_COUNT 3

static lry_point obstacles[OBSTACLE_COUNT];

// static functions

void gameStart(void) {
  glcdFillScreen(GLCD_CLEAR);

  ticks = 0;
  yshift = 0;
  ball = (xy_point){.x = 64, .y = 50};
  level = 0;
  score = 0;

  memset(obstacles, 0, 3 * sizeof(lry_point));
  obstacles[0].y = 10;
  obstacles[1].y = 30;
  obstacles[2].y = 50;
}

void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {
  // https://wiibrew.org/wiki/Wiimote#Accelerometer
  // Note that X has 10 bits of precision, while Y and Z only have 9.

  cli();
  if (hi8(y) == 1) {
    moveRight = false;
    speed = ((y)&0x0ff);
  } else {
    moveRight = true;
    speed = ~((y)&0x0ff);
  }
  sei();
}

static inline void drawBallOptimized(xy_point ball, void (*drawPx)(const uint8_t, const uint8_t)) {
  uint8_t x = ball.x, y = ball.y;
  drawPx(x + 1, y);
  drawPx(x + 2, y);

  y++;
  y %= GLC_HEIGHT;
  drawPx(x, y);
  drawPx(x + 1, y);
  drawPx(x + 2, y);
  drawPx(x + 3, y);

  y++;
  y %= GLC_HEIGHT;
  drawPx(x, y);
  drawPx(x + 1, y);
  drawPx(x + 2, y);
  drawPx(x + 3, y);

  y++;
  y %= GLC_HEIGHT;
  drawPx(x + 1, y);
  drawPx(x + 2, y);
}

static inline void drawHorizontalLineOptimized(uint8_t left, const uint8_t right, const uint8_t y,
                                               void (*drawPx)(const uint8_t, const uint8_t)) {
  for (; left <= right; left++) {
    drawPx(left, y);
  }
}

void gameTick(void) {
  ticks++;

  // delete old ball
  drawBallOptimized(ball, &glcdClearPixel);

  const uint8_t top = yshift;
  const uint8_t bottom = (top + 63) % GLC_HEIGHT;

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
      if (likely(obstacle.active) && (ball.y + 6) % GLC_HEIGHT == obstacle.y &&
          obstacle.l < ball.x && ball.x < obstacle.r) {
        colided = true;
        continue;
      }
    }

    if ((ball.y + 6) % GLC_HEIGHT == bottom) {
      colided = true;
    }

    if (!colided) {
      ball.y++;
      ball.y %= GLC_HEIGHT;
    }

    // are we dead?
    if (ball.y == top) {
      gamestate = DEAD_ENTER;
      return;
    }
  }

  // shit vertically and update obstacles
  if (ticks % (10 - min(level, 7)) == 0) {
    glcdDrawHorizontal(yshift, &glcdClearPixel);
    yshift++;
    yshift %= GLC_HEIGHT;
    glcdSetYShift(yshift);
    const uint8_t top = yshift;
    const uint8_t bottom = (top + 63) % GLC_HEIGHT;

    for (uint8_t i = 0; i < OBSTACLE_COUNT; ++i) {
      if (obstacles[i].y == top) {
        const uint8_t left = rand16() % GLC_HEIGHT;
        const uint8_t right = rand16() % GLC_HEIGHT + 63;
        obstacles[i].l = left;
        obstacles[i].r = right;
        obstacles[i].y = bottom;
        drawHorizontalLineOptimized(left, right, bottom, &glcdSetPixel);
        obstacles[i].active = true;
      }
    }
  }

  // draw new ball
  drawBallOptimized(ball, &glcdSetPixel);

  if (ticks % 10 == 0) {
    score++;
  }

  if (ticks % 1000 == 0) {
    level++;
  }
}