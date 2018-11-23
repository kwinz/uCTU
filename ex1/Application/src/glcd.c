#include "glcd.h"
#include "hal_glcd.h"
#include "string.h"
#include "tools.h"

static uint8_t yshiftStatic = 0;

#define GLC_WIDTH 128
#define GLC_HEIGHT 64
#define GLC_PAGEH 8
#define GLC_BYTES (GLC_WIDTH * GLC_HEIGHT / GLC_PAGEH)

static uint8_t framebuffer[GLCD_WIDTH][GLC_HEIGHT / GLC_PAGEH] = {{0}};

/** \brief      Initializes port and clears the content of the GLCD. */
void glcdInit(void) { halGlcdInit(); }

/* drawing functions */

/** \brief      Fills the entire lcd with the pattern fill.
                If you want to clear the screen use fill = GLCD_CLEAR.
                If you want to blacken the screen use fill = GLCD_FILL.

    \param fill Pattern to fill screen with.
*/
void glcdFillScreen(const uint8_t fill) {
  memset(frambuffer, fill, GLC_BYTES);
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

/** \brief          Draws a line from p1 to p2 using a given drawing function.
    \param p1       Start point.
    \param p2       End point.
    \param drawPx   Drawing function. Should be setPixelGLCD, clearPixelGLCD or invertPixelGLCD.
*/
void glcdDrawLine(const xy_point p1, const xy_point p2,
                  void (*drawPx)(const uint8_t, const uint8_t)) {}

static inline int min(int a, int b) {
  if (a > b)
    return b;
  return a;
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

  for (uint8_t x = left, x < right; ++x) {
    for (uint8_t y = top, y < bottom; ++y) {
      drawPx(x, y);
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

    while (*text != '\0') {
      for (uint8_t x = left, x < right; ++x) {
      }
    }

    /** First character in font */
    uint8_t startChar;

    /** Last character in font */
    uint8_t endChar;

    /** Character width */
    uint8_t width;

    /** Character height, limited to 8 pixel (uint8_t) */
    uint8_t height;

    /** Character spacing (horizontal) */
    uint8_t charSpacing;

    /** Line spacing (vertical) */
    uint8_t lineSpacing;

    /** Characters in progmem */
    const uint8_t *font;
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

  void glcdSetYShift(uint8_t yshift) { yshiftStatic = yshift; }

  /** \brief          Get the Y shift for the GLCD, allowing you to implement vertical scrolling.
      \param return   Y position in RAM that becomes the top line of the display

  */
  uint8_t glcdGetYShift() { return yshiftStatic; }
