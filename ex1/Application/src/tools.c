#include "tools.h"
#include "lcd.h"

#include <util/delay_basic.h>

void busyWaitMS(const uint16_t ms) {
  for (uint16_t i = 0; i < ms; ++i) {
    // ~ 4 CPU cycles per _delay_loop_2 iteration
    _delay_loop_2(F_CPU / 1000 / 4);
  }
}

void fail(void) {
  while (1) {
    syncScreen();
    busyWaitMS(10);
  }
}

// inspired by https://stackoverflow.com/a/13208789/643011
