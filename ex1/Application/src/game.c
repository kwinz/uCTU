#include "game.h"
#include "font.h"
#include "glcd.h"
#include "mp3.h"
#include "rand.h"

static uint8_t ticks = 0;
static volatile xy_point ball = {5, 50};

void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z) {
  ball.x = (ball.x + x / 255 / 10) % 128;
}

void gameTick(void) {
  ticks++;

  // game fills the whole 128x64pix screen
  // ball must start at the bottom, in the center

  // player must have the possiblity to start a new game and also access the high score table

  // xy_point a = {0, 0};
  // y_point b = {50, rand16() % 50U};

  glcdDrawCircle(ball, 1, &glcdSetPixel);
  glcdDrawCircle(ball, 2, &glcdSetPixel);

  // glcdDrawLine(a, b, &glcdInvertPixel);

  // glcdDrawText("olol", c, &Standard5x7, &glcdInvertPixel);

  // if (rand16() < 200) {
  if (ticks % 10 == 0) {
    glcdSetYShift(glcdGetYShift() + 1);
  }
}