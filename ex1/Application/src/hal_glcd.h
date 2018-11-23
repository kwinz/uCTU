#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GLC_WIDTH 128
#define GLC_HEIGHT 64
#define GLC_PAGEH 8
#define GLC_BYTES (GLC_WIDTH * GLC_HEIGHT / GLC_PAGEH)

uint8_t halGlcdInit(void);

uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage);

uint8_t halGlcdWriteData(const uint8_t data);

uint8_t halGlcdReadData();