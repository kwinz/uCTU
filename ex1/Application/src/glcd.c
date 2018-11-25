#include "glcd.h"
#include "hal_glcd.h"
#include "tools.h"

#include <avr/pgmspace.h>
#include <string.h>

static uint8_t yshiftStatic = 0;

/* The frambuffer is relatively big with 1KiB,
but it makes the GLCD that much faster.
We can only write/read byte-sized chunks.
By having them in RAM, we can just straight write
one byte to glcd instead of having to
1. read back, 2. modify, 3. reset address, 4. write back
the whole byte with the associated delays for each #drawPx().
*/
static uint8_t framebuffer[GLC_WIDTH][GLC_HEIGHT / GLC_PAGEH] = {{0}};

/** \brief      Initializes port and clears the content of the GLCD. */
void glcdInit(void) { halGlcdInit(); }

/* drawing functions */

/** \brief      Fills the entire lcd with the pattern fill.
                If you want to clear the screen use fill = GLCD_CLEAR.
                If you want to blacken the screen use fill = GLCD_FILL.

    \param fill Pattern to fill screen with.
*/
void glcdFillScreen(const uint8_t fill) {
  memset(framebuffer, fill, GLC_BYTES);
  halGlcdSetAddress(0, 0);
  for (uint16_t i = 0; i < GLC_BYTES; ++i) {
    halGlcdWriteData(fill);
  }
}

/** \brief      Sets one single pixel
    \param x    x-coordinate of pixel to set.
    \param y    y-coordinate of pixel to set.
*/
void glcdSetPixel(const uint8_t x, const uint8_t y) {
  const uint8_t pageNumber = y / GLC_PAGEH;
  framebuffer[x][pageNumber] |= (1 << (y % GLC_PAGEH));
  halGlcdSetAddress(x, pageNumber);
  halGlcdWriteData(framebuffer[x][pageNumber]);
}

/** \brief      Clears one single pixel
    \param x    x-coordinate of pixel to clear.
    \param y    y-coordinate of pixel to clear.
*/
void glcdClearPixel(const uint8_t x, const uint8_t y) {
  const uint8_t pageNumber = y / GLC_PAGEH;
  framebuffer[x][pageNumber] &= ~(1 << (y % GLC_PAGEH));
  halGlcdSetAddress(x, pageNumber);
  halGlcdWriteData(framebuffer[x][pageNumber]);
}

/** \brief      Inverts one single pixel
    \param x    x-coordinate of pixel to invert.
    \param y    y-coordinate of pixel to invert.
*/
void glcdInvertPixel(const uint8_t x, const uint8_t y) {
  const uint8_t pageNumber = y / GLC_PAGEH;
  framebuffer[x][pageNumber] ^= (1 << (y % GLC_PAGEH));
  halGlcdSetAddress(x, pageNumber);
  halGlcdWriteData(framebuffer[x][pageNumber]);
}

static void glcdDrawBresenhamX(int16_t p1x, int16_t p1y, int16_t p2x, int16_t p2y,
                               void (*drawPx)(const uint8_t, const uint8_t)) {

  int16_t ydiff;
  int8_t yIncrease;
  if (p2y > p1y) {
    ydiff = p2y - p1y;
    yIncrease = 1;
  } else {
    ydiff = p1y - p2y;
    yIncrease = -1;
  }

  const int16_t xdiff = p2x - p1x;
  int16_t diff = 2 * ydiff - xdiff;

  for (int16_t y = p1y, x = p1x; x <= p2x; ++x) {
    drawPx(x, y);
    if (diff > 0) {
      y += yIncrease;
      diff -= 2 * xdiff;
    }
    diff += 2 * ydiff;
  }
}

static void glcdDrawBresenhamY(int16_t p1x, int16_t p1y, int16_t p2x, int16_t p2y,
                               void (*drawPx)(const uint8_t, const uint8_t)) {
  int16_t xdiff;
  int8_t xIncrease;
  if (p2x > p1x) {
    xdiff = p2x - p1x;
    xIncrease = 1;
  } else {
    xdiff = p1x - p2x;
    xIncrease = -1;
  }

  const int16_t ydiff = p2y - p1y;
  int16_t diff = 2 * xdiff - ydiff;

  for (int16_t x = p1x, y = p1y; y <= p2y; ++y) {
    drawPx(x, y);
    if (diff > 0) {
      x += xIncrease;
      diff -= 2 * ydiff;
    }
    diff += 2 * xdiff;
  }
}

/** \brief          Draws a line from p1 to p2 using a given drawing function.
    \param p1       Start point.
    \param p2       End point.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawLine(xy_point p1, xy_point p2, void (*drawPx)(const uint8_t, const uint8_t)) {

  // inspired by https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  // and https://tuwel.tuwien.ac.at/mod/forum/discuss.php?d=126640

  uint8_t ydiff = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
  uint8_t xdiff = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;

  xy_point temp;

  if (ydiff < xdiff) {
    // xdiff is longer than ydiff
    if (p1.x > p2.x) {
      temp = p1;
      p1 = p2;
      p2 = temp;
    }
    glcdDrawBresenhamX(p1.x, p1.y, p2.x, p2.y, drawPx);

  } else {
    // ydiff is longer than xdiff
    if (p1.y > p2.y) {
      temp = p1;
      p1 = p2;
      p2 = temp;
    }
    glcdDrawBresenhamY(p1.x, p1.y, p2.x, p2.y, drawPx);
  }
}

/** \brief          Draws a rectangle from p1 to p2 using a given drawing function.
    \param p1       First corner.
    \param p2       Second corner.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawRect(const xy_point p1, const xy_point p2,
                  void (*drawPx)(const uint8_t, const uint8_t)) {
  const uint8_t left = min(p1.x, p2.x);
  const uint8_t right = max(p1.x, p2.x);
  const uint8_t top = min(p1.y, p2.y);
  const uint8_t bottom = max(p1.y, p2.y);

  for (uint8_t y = top; y <= bottom; ++y) {
    drawPx(left, y);
    drawPx(right, y);
  }

  // avoid to overdraw the already set edges
  if (left + 1 <= right - 1) {
    for (uint8_t x = left + 1; x <= right - 1; ++x) {
      drawPx(x, top);
      drawPx(x, bottom);
    }
  }
}

/** \brief          Fills a rectangle from p1 to p2 using a given drawing function.
    \param p1       First corner.
    \param p2       Second corner.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdFillRect(const xy_point p1, const xy_point p2,
                  void (*drawPx)(const uint8_t, const uint8_t)) {
  const uint8_t left = min(p1.x, p2.x);
  const uint8_t right = max(p1.x, p2.x);
  const uint8_t top = min(p1.y, p2.y);
  const uint8_t bottom = max(p1.y, p2.y);

  for (uint8_t x = left; x <= right; ++x) {
    for (uint8_t y = top; y <= bottom; ++y) {
      drawPx(x, y);
    }
  }
}

/** \brief          Draws a vertical line at a given x-coordinate using a given drawing function.
    \param x        x-position of the line.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawVertical(const uint8_t x, void (*drawPx)(const uint8_t, const uint8_t)) {
  for (uint8_t y = 0; y < GLC_HEIGHT; ++y) {
    drawPx(x, y);
  }
}

/** \brief          Draws a horizontal line at a given y-coordinate using a given drawing
   function. \param y        y-position of the line. \param drawPx   Drawing function. Should be
   setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawHorizontal(const uint8_t y, void (*drawPx)(const uint8_t, const uint8_t)) {
  for (uint8_t x = 0; x < GLC_WIDTH; ++x) {
    drawPx(x, y);
  }
}

/** \brief          Draws a character a given point using a given drawing function and a given
   font. \param c        Character to display. \param p        Position where to display the
   character (the anchor is bottom left). \param f        Font to use. \param drawPx   Drawing
   function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawChar(const char c, const xy_point p, const font *f,
                  void (*drawPx)(const uint8_t, const uint8_t)) {
  const uint8_t base_x = p.x;
  for (uint8_t char_x = 0; char_x < f->width; ++char_x) {
    uint8_t x = char_x + base_x;
    const uint8_t colByte = pgm_read_byte(f->font + ((c - f->startChar) * f->width + char_x));
    for (uint8_t char_y = 0; char_y < f->height; ++char_y) {
      const uint8_t y = p.y - f->height + char_y;
      if (colByte & (1 << char_y)) {
        drawPx(x, y);
      }
    }
  }
}

/** \brief          Draws a character a given point using a given drawing function and a given
   font. \param text     Text to display. \param p        Position where to display the text (the
   anchor is bottom left). \param f        Font to use. \param drawPx   Drawing function. Should
   be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawText(const char *text, const xy_point p, const font *f,
                  void (*drawPx)(const uint8_t, const uint8_t)) {

  uint8_t countNewlines = 0;
  uint8_t charInLine = 0;
  while (*text != '\0') {
    if (*text == '\r') {
      text++;
      continue;
    }
    if (*text == '\n') {
      countNewlines++;
      charInLine = 0;
      text++;
      continue;
    }
    const uint8_t base_x = p.x + charInLine * f->charSpacing;
    const uint8_t base_y = p.y + countNewlines * f->lineSpacing;
    xy_point char_point = {base_x, base_y};
    glcdDrawChar(*text, char_point, f, drawPx);
    charInLine++;
    text++;
  }
}

#define GLCD_STRBUFF_RAM_SIZE 40

/**
 * \brief  Draws a character a given point using a given drawing function and a given font.

 * Supports strings with at most GLCD_STRBUFF_RAM_SIZE length.

   \param text     Text to display. The text is stored on the program memory.
   \param p        Position where to display the text (the anchor is bottom left).
   \param f        Font to use.
   \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawTextPgm(const char *text, const xy_point p, const font *f,
                     void (*drawPx)(const uint8_t, const uint8_t)) {
  char strbuff_ram[GLCD_STRBUFF_RAM_SIZE];
  strncpy_PF(strbuff_ram, (intptr_t)text, GLCD_STRBUFF_RAM_SIZE - 1);
  strbuff_ram[GLCD_STRBUFF_RAM_SIZE - 1] = '\0';
  glcdDrawText(strbuff_ram, p, f, drawPx);
}

/** \brief          Set the Y shift for the GLCD, allowing you to implement vertical scrolling.
    \param yshift   Y position in RAM that becomes the top line of the display

*/

void glcdSetYShift(uint8_t yshift) {
  yshiftStatic = yshift;
  halGlcdSetYShift(yshift);
}

/** \brief          Get the Y shift for the GLCD, allowing you to implement vertical scrolling.
    \param return   Y position in RAM that becomes the top line of the display

*/
uint8_t glcdGetYShift() { return yshiftStatic; }
