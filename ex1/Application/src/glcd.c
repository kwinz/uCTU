#include "glcd.h"
#include "hal_glcd.h"
#include "string.h"
#include "tools.h"

static uint8_t yshiftStatic = 0;

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

static inline uint8_t min(uint8_t a, uint8_t b) {
  if (a > b)
    return b;
  return a;
}

static inline uint8_t max(uint8_t a, uint8_t b) {
  if (a > b)
    return a;
  return b;
}

/** \brief          Draws a line from p1 to p2 using a given drawing function.
    \param p1       Start point.
    \param p2       End point.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawLine(const xy_point p1, const xy_point p2,
                  void (*drawPx)(const uint8_t, const uint8_t)) {

  uint8_t x = p1.x, y = p1.y, x2 = p2.x, y2 = p2.y;

  uint8_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

  uint8_t w = x2 - x;
  if (x2 > x) {
    dx1 = 1;
    dx2 = 1;
  } else if (x2 < x) {
    w = x - x2;
    dx1 = -1;
    dx2 = -1;
  }

  uint8_t h = y2 - y;
  if (y2 > y) {
    dy1 = 1;
  } else if (y2 < y) {
    h = y - y2;
    dy1 = -1;
  }

  uint8_t longest = w;
  uint8_t shortest = h;
  if (!(longest > shortest)) {
    longest = h;
    shortest = w;

    if (y2 > y) {
      dy2 = 1;
    } else if (y2 < y) {
      dy2 = -1;
    }
    dx2 = 0;
  }
  int numerator = longest >> 1;
  for (int i = 0; i <= longest; i++) {
    drawPx(x, y);
    numerator += shortest;
    if (!(numerator < longest)) {
      numerator -= longest;
      x += dx1;
      y += dy1;
    } else {
      x += dx2;
      y += dy2;
    }
  }
}

/** \brief          Draws a rectangle from p1 to p2 using a given drawing function.
    \param p1       First corner.
    \param p2       Second corner.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawRect(const xy_point p1, const xy_point p2,
                  void (*drawPx)(const uint8_t, const uint8_t)) {}

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

  for (uint8_t x = left; x < right; ++x) {
    for (uint8_t y = top; y < bottom; ++y) {
      drawPx(x, y);
    }
  }
}

/** \brief          Draws a vertical line at a given x-coordinate using a given drawing function.
    \param x        x-position of the line.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawVertical(const uint8_t x, void (*drawPx)(const uint8_t, const uint8_t)) {}

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
                  void (*drawPx)(const uint8_t, const uint8_t)) {}

/** \brief          Draws a character a given point using a given drawing function and a given
   font. \param text     Text to display. \param p        Position where to display the text (the
   anchor is bottom left). \param f        Font to use. \param drawPx   Drawing function. Should
   be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawText(const char *text, const xy_point p, const font *f,
                  void (*drawPx)(const uint8_t, const uint8_t)) {

  uint8_t charIndex = 0;
  while (*text != '\0') {
    uint8_t base_x = p.x + charIndex * f->charSpacing;
    for (uint8_t char_x = 0; char_x < f->width; ++char_x) {
      uint8_t x = char_x + base_x;
      const uint8_t colByte = pgm_read_byte(f->font + ((*text - f->startChar) * f->width + char_x));
      for (uint8_t char_y = 0; char_y < f->height; ++char_y) {
        const uint8_t y = p.y - f->height + char_y;
        if (colByte & (1 << char_y)) {
          drawPx(x, y);
        }
      }
    }
    charIndex++;
    text++;
  }
}

/** \brief          Draws a character a given point using a given drawing function and a given
   font. \param text     Text to display. The text is stored on the program memory. \param p
   Position where to display the text (the anchor is bottom left). \param f        Font to use.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawTextPgm(const char *text, const xy_point p, const font *f,
                     void (*drawPx)(const uint8_t, const uint8_t)) {}

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
