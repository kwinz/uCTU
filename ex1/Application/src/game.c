#include "game.h"
#include "font.h"
#include "glcd.h"
#include "mp3.h"
#include "rand.h"
#include "tools.h"
#include <avr/interrupt.h>

static uint8_t ticks = 0;

static volatile xy_point ball = {5, 50};
static xy_point old_ball = {5, 50};

void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {
  // https://wiibrew.org/wiki/Wiimote#Accelerometer

  // PORTK = lo8(x);
  // PORTL = hi8(x);

  // fail();

  if (x | 0x0200) {
    ball.x -= ((x & 0xFF) / 128);
  } else {
    ball.x += ((x & 0xFF) / 128);
  }
}

void gameTick(void) {
  cli();
  xy_point new_ball = ball;
  sei();

  new_ball.x = new_ball.x / 2;

  ticks++;

  // game fills the whole 128x64pix screen
  // ball must start at the bottom, in the center

  // player must have the possiblity to start a new game and also access the high score table

  // xy_point a = {0, 0};
  // y_point b = {50, rand16() % 50U};

  glcdDrawCircle(old_ball, 1, &glcdClearPixel);
  glcdDrawCircle(old_ball, 2, &glcdClearPixel);

  glcdDrawCircle(new_ball, 1, &glcdSetPixel);
  glcdDrawCircle(new_ball, 2, &glcdSetPixel);

  cli();
  old_ball = new_ball;
  sei();

  // glcdDrawLine(a, b, &glcdInvertPixel);

  // glcdDrawText("olol", c, &Standard5x7, &glcdInvertPixel);

  // if (rand16() < 200) {
  if (ticks % 10 == 0) {
    glcdSetYShift(glcdGetYShift() + 1);
  }
}