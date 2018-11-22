#pragma once

#include <stdbool.h>
#include <stdint.h>

void start(void);
void gameTick(void);
void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z);