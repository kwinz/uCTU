#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define FOSC F_CPU // Clock Speed

#define HAVE_MP3_BOARD true
#define HAVE_BLUETOOTH_BOARD true

void fail(void);
void busyWaitMS(const uint16_t ms);

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

static volatile bool usingSPI = false;