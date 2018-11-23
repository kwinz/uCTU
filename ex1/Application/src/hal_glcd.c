#include "hal_glcd.h"

#include <avr/io.h>

// d/i - ?
// cs1 - PE2 (inverted?)
#define GL_CS1 _BV(2)
// cs2 - PE3 (inverted?)
#define GL_CS2 _BV(3)
// RS - PE4
#define GL_RS _BV(4)
// r/w - PE5
#define GL_RW _BV(5)
// E - PE6
#define GL_E _BV(6)
// RST PE7
#define GL_RESET _BV(7)
// db0-db7 - PA
// source: bigavr6_schematic_v100.pdf

/*

for writing:
-----------
first E low
then data, CS, RS (and RW low)
then E high

for reading:
------------
first E low
then cs, rs (and RW high)
then E high
then read off data

source: glcd_128x64_spec.pdf

*/

volatile static uint8_t count = 0;

static void halGlcdCtrlBusyWait() {
  // for (uint8_t j = 0; j < 255; ++j) {
  for (uint8_t i = 0; i < 30; ++i) {
    asm volatile("nop" :::);
  }
  //}
}

/*
Cmd means RS=L
write means RW=L
*/
static void halGlcdCtrlWriteCmd(const bool controller2, const uint8_t data) {
  PORTE = GL_RESET;
  PORTE &= ~GL_E;
  halGlcdCtrlBusyWait();
  DDRA = 0xFF;
  PORTA = data;
  PORTE = (controller2 ? GL_CS1 : GL_CS2) | GL_RESET;
  halGlcdCtrlBusyWait();
  PORTE |= GL_E;
  halGlcdCtrlBusyWait();
  PORTE &= ~GL_E;
}

/*
DATA means RS=H
write means RW=L
*/
static void halGlcdCtrlWriteData(const bool controller2, const uint8_t data) {
  PORTE = GL_RESET;
  PORTE &= ~GL_E;
  halGlcdCtrlBusyWait();
  DDRA = 0xFF;
  PORTA = data;
  PORTE = (controller2 ? GL_CS1 : GL_CS2) | GL_RESET;
  PORTE |= GL_RS;
  halGlcdCtrlBusyWait();
  PORTE |= GL_E;
  halGlcdCtrlBusyWait();
  PORTE &= ~GL_E;
}

/*
DATA means RS=H
read means RW=H
*/
static uint8_t halGlcdCtrlReadData(const bool controller2) {
  PORTE = GL_RESET;
  PORTE &= ~GL_E;
  halGlcdCtrlBusyWait();
  DDRA = 0x00;
  PORTE = (controller2 ? GL_CS1 : GL_CS2) | GL_RESET;
  PORTE |= GL_RS;
  PORTE |= GL_RW;
  halGlcdCtrlBusyWait();
  PORTE |= GL_E;
  halGlcdCtrlBusyWait();
  return PINA;
  PORTE &= ~GL_E;
}

/**
 *
 * WARNING: the parameters of this function X Y are swapped in comparison with halGlcdSetAddress
 *
 *
 */
static void halGlcdCtrlSetAddress(const bool controller2, const uint8_t x, const uint8_t y) {
  // write y
  {
    const uint8_t data = (1 << 6) | (y & 0x3F);
    halGlcdCtrlWriteCmd(controller2, data);
  }
  // write x
  {
    const uint8_t data = (0xb8) | (x & 0x07);
    halGlcdCtrlWriteCmd(controller2, data);
  }
}

static void halGlcdTurnOn(const bool on) {
  const uint8_t data = 0x3E | on;
  halGlcdCtrlWriteCmd(false, data);
  halGlcdCtrlWriteCmd(true, data);
}

/*
Initializes the microcontroller’s interface to the GLCD, reset and initializes of the display
controllers. After calling this function the GLCD should be empty and ready for use.
*/
uint8_t halGlcdInit(void) {
  DDRE = 0xFF;
  halGlcdTurnOn(true);
  halGlcdSetAddress(0, 0);
  return 0;
}

static uint8_t xColStatic, yPageStatic;

/*
This function sets the internal RAM address to match the x and y addresses.
While 0 6 xCol 6 127 is the horizontal coordinate from left to right,
0 6 yPage 6 7 denotes the 8-bit pages oriented vertically from top to bottom.
Note that this convention differs from the chip datasheet where the
orientation of x and y is different.
*/
uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage) {
  xColStatic = xCol;
  yPageStatic = yPage;

  bool controller2 = false;
  if (xColStatic > 63) {
    controller2 = true;
  }

  halGlcdCtrlSetAddress(controller2, yPage, xCol % 64);
  return 0;
}

/*This function writes data to the display RAM of the GLCD controller at the currently set address.
The post increment of the writedisplaydata operation, which is provided by the GLCD controller,
should be transparently extended over both display controllers.
*/
uint8_t halGlcdWriteData(const uint8_t data) {
  bool controller2 = false;
  if (xColStatic > 63) {
    controller2 = true;
  }

  halGlcdCtrlWriteData(controller2, data);

  xColStatic++;
  if (xColStatic == 64) {
    halGlcdCtrlSetAddress(true, yPageStatic, xColStatic % 64);
  }

  if (xColStatic == 128) {
    xColStatic = 0;
    yPageStatic++;
    yPageStatic %= (GLC_HEIGHT / GLC_PAGEH);
    halGlcdCtrlSetAddress(false, yPageStatic, xColStatic % 64);
  }

  PORTH++;
  PORTK = xColStatic;
  PORTL = yPageStatic;

  return 0;
}

uint8_t halGlcdReadData() {

  // FIXME: not implemented

  return halGlcdCtrlReadData(false);
}

uint8_t halGlcdSetYShift(const uint8_t yshift) {
  const uint8_t data = 0xC0 | (0x3f & yshift);
  halGlcdCtrlWriteCmd(false, data);
  halGlcdCtrlWriteCmd(true, data);
  return 0;
}