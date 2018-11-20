
#include "adc.h"
#include "timer_utils.h"
#include <avr/io.h>

void adcInit(void) { void setup16BitTimer(void); }

ISR(TIMER1_COMPA_vect, ISR_BLOCK) /* Blocking timer interrupt */
{
  static volatile uint8_t counter = 0;

  click = FALSE;
  callCallback(&counter);
  PORTA = counter;
}