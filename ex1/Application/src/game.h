#pragma once

#include <stdbool.h>
#include <stdint.h>

uint16_t score;

void gameStart(void);
void gameTick(void);
void rcvAccel(const uint8_t wii, const uint16_t x, const uint16_t y, const uint16_t z);