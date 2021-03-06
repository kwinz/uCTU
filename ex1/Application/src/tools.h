#pragma once

/**
 * This is my version of / an additon to util.h
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define FOSC F_CPU // Clock Speed

#define HAVE_MP3_BOARD true
#define HAVE_BLUETOOTH_BOARD true

void fail(void);
void busyWaitMS(const uint16_t ms);

// source: common macros
// e.g.
// https://stackoverflow.com/questions/109710/how-does-the-likely-unlikely-macros-in-the-linux-kernel-works-and-what-is-their
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

// source: https://www.avrfreaks.net/forum/c-equivalent-lo8-and-hi8
#define lo8(x) ((x)&0xff)
#define hi8(x) ((x) >> 8)

typedef enum {
  CONNECT_PAINT,
  CONNECT,
  MENU_PAINT,
  MENU,
  PLAYING_ENTER,
  PLAYING,
  DEAD_ENTER,
  DEAD,
  HIGHSCORE_PAINT,
  HIGHSCORE
} GameState_t;

// defined in header file to get guaranteed inline for this helper function
inline __attribute__((always_inline)) uint8_t min(uint8_t a, uint8_t b) {
  if (a > b)
    return b;
  return a;
}

inline __attribute__((always_inline)) uint8_t max(uint8_t a, uint8_t b) {
  if (a > b)
    return a;
  return b;
}