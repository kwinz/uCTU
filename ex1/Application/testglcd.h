
const char helloworld[] PROGMEM = "Hallo World <3\nHello Micro!";

static void testGlcd(void) {
  glcdFillScreen(GLCD_FILL);
  glcdFillScreen(GLCD_CLEAR);

  glcdSetPixel(60, 20);
  glcdSetPixel(61, 21);
  glcdSetPixel(62, 22);
  glcdSetPixel(63, 23);
  glcdClearPixel(64, 24);
  glcdSetPixel(65, 25);
  glcdSetPixel(66, 26);
  glcdSetPixel(67, 27);
  glcdSetPixel(68, 28);
  glcdSetPixel(69, 29);

  {
    xy_point c = {20, 10};
    glcdDrawTextPgm(helloworld, c, &Standard5x7, &glcdSetPixel);
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

  {
    xy_point p2 = {45, 45};
    xy_point p1 = {50, 50};
    glcdDrawRect(p1, p2, &glcdInvertPixel);
  }

  {
    xy_point c = {10, 50};
    glcdDrawChar('%', c, &Standard5x7, &glcdSetPixel);
  }

  glcdDrawHorizontal(17, &glcdInvertPixel);

  glcdDrawVertical(75, &glcdInvertPixel);

  fail();
}
