#include "hal_glcd.h"

static void halGlcdCtrlWriteData(const uint8_t controller, const uint8_t data) {}
static uint8_t halGlcdCtrlReadData(const uint8_t controller) { return 0; }
static void halGlcdCtrlWriteCmd(const uint8_t controller, const uint8_t data) {}
static void halGlcdCtrlSetAddress(const uint8_t controller, const uint8_t x, const uint8_t y) {}
static void halGlcdCtrlBusyWait(const uint8_t controller) {}
static void halGlcdCtrlSelect(const uint8_t controller) {}

uint8_t halGlcdInit(void) { return 0; }

uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage) { return 0; }

uint8_t halGlcdWriteData(const uint8_t data) { return 0; }