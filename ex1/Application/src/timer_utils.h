#pragma once
#include <stdint.h>

// typedef enum { TIMER0 = 0, TIMER2 = 2 } Timer8Bit_t;
typedef enum { TIMER1 = 1, TIMER3 = 3, TIMER4 = 4, TIMER5 = 5 } Timer16Bit_t;

void start16BitTimer(Timer16Bit_t timer, uint32_t usec, void (*periodicCallback)(void));