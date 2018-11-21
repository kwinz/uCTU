#pragma once

#include <stdbool.h>
#include <stdint.h>

volatile bool haveNewVolume;
volatile uint8_t volumeFromADC;

void adcInit(void);