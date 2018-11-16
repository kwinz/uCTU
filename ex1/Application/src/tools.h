#pragma once

#include <inttypes.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define FOSC F_CPU // Clock Speed

void fail(void);
void busyWaitMS(const uint16_t ms);