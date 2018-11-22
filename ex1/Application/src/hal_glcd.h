#pragma once

#include <stdbool.h>
#include <stdint.h>

uint8_t halGlcdInit(void);

uint8_t halGlcdSetAddress(const uint8_t xCol, const uint8_t yPage);

uint8_t halGlcdWriteData(const uint8_t data);