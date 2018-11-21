
#include "adc.h"
#include "timer_utils.h"
#include <avr/io.h>

/**
 * 26. ADC, page 275
 * The device also supports 16/32 differential voltage input combinations. Four of the differential
inputs (ADC1 & ADC0, ADC3 & ADC2, ADC9 & ADC8 and ADC11 & ADC10) are equipped with
a programmable gain stage, providing amplification steps of 0 dB (1×), 20 dB (10×) or 46 dB
(200×) on the differential input voltage before the ADC conversion. The 16 channels are split in
two sections of 8 channels where in each section seven differential analog input channels share
a common negative terminal (ADC1/ADC9), while any other ADC input in that section can be
selected as the positive input terminal. If 1× or 10× gain is used, 8 bit resolution can be
expected. If 200× gain is used, 7 bit resolution can be expected.
*/

void adcInit(void) { void setup16BitTimer(void); }
