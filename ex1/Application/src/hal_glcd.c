#include "hal_glcd.h"

// d/i - ?
// cs1 - PE2
// cs2 - PE3
// rst - PE4
// r/w - PE5
// E - PE6
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

static void halGlcdCtrlBusyWait(const uint8_t controller) {
  for (uint8_t i = 0; i < 255; ++i) {
    asm volatile("nop" :::);
  }
}

static void halGlcdCtrlWriteCmd(const uint8_t controller, const uint8_t data) { int y = 2; }

static void halGlcdCtrlSetAddress(const uint8_t controller, const uint8_t x, const uint8_t y) {}

static void halGlcdCtrlSelect(const uint8_t controller) {}

static uint8_t halGlcdCtrlReadData(const uint8_t controller) { return 0; }

static void halGlcdCtrlWriteData(const uint8_t controller, const uint8_t data) {}

uint8_t halGlcdInit(void) { return 0; }

uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage) { return 0; }

uint8_t halGlcdWriteData(const uint8_t data) { return 0; }