
#include "adc.h"
#include "mp3.h"
#include "rand.h"
#include "timer_utils.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <tools.h>

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


In Single Conversion mode, always select the channel before starting the conversion. The chan-
nel selection may be changed one ADC clock cycle after writing one to ADSC. However, the
simplest method is to wait for the conversion to complete before changing the channel selection.
In Free Running mode, always select the channel before starting the first conversion. The chan-
nel selection may be changed one ADC clock cycle after writing one to ADSC. However, the
simplest method is to wait for the first conversion to complete, and then change the channel
selection. Since the next conversion has already started automatically, the next result will reflect
the previous channel selection. Subsequent conversions will reflect the new channel selection.
When switching to a differential gain channel, the first conversion result may have a poor accu-
racy due to the required settling time for the automatic offset cancellation circuitry. The user
should preferably disregard the first conversion result.

ADMUX can be safely updated in the following ways:
1! When ADATE (Bit 5 – ADATE: ADC Auto Trigger Enable) or ADEN is cleared. (Markus: they mean
both?!)
2. During conversion, minimum one ADC clock cycle after the trigger event.
3! After a conversion, before the Interrupt Flag used as trigger source is cleared.
*/

/*
PF0 = ADC0
PF7 = ACD7
PK0 = ADC8
PK7 = ADC15
*/

volatile bool haveNewVolume = false;
static volatile bool volumeMode = false;

static void sampleRand(void) {
  PORTH++;
  // for rand we use
  // MUX5:0 001111  ADC3,ADC2 200× gain
  // write the 5 bits to ADMUX. ADCSRB is already set
  ADMUX = (ADMUX & 0xE0) | 0x0F;
}

static void sampleVolume() {
  // the potentiometer can be connected to PF0-PF3 via jumpers
  // on this board it is PF0
  // BIG AVR 6 Development System, page 16

  // therefore MUX5:0 is 000000
  // write the 5 bits to ADMUX. ADCSRB is already set
  ADMUX = (ADMUX & 0xE0) | 0;
}

static void randfeed() {

  if (volumeMode) {
    sampleVolume();
  } else {
    sampleRand();
  }

  // ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
  ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADSC);
}

void adcInit(void) {

  // DIDR0 – Digital Input Disable Register 0
  // turn off digital io on port F to save power (those are adc0-7 or PINF)
  DIDR0 = 0xFF;

  // REFS1 REFS0 ADLAR MUX4 MUX3 MUX2 MUX1 MUX0
  // Write one to ADLAR to left adjust the result. Otherwise, the result is right adjusted. Changing
  // the ADLAR bit will affect the ADC Data Register immediately, regardless of any ongoing.
  // conversions. (Markus: I decided not to use this)
  ADMUX = (1 << REFS0);

  //– ACME – – MUX5 ADTS2 ADTS1 ADTS0
  // using Free Running mode for now, but "Timer/Counter1 Compare Match B" sounds interresting
  // set all bits to 0 except for reserved ones
  ADCSRB &= (1 << 7) | (1 << 5) | (1 << 4);

  // 50000U
  start16BitTimer(TIMER5, 50000U, &randfeed);
}

ISR(ADC_vect) {

  if (!HAVE_MP3_BOARD) {
    rand_feed(ADC);
  } else if (!volumeMode) {
    rand_feed(ADC);
    volumeMode = true;
  } else {
    // result in ADC (16bit)
    uint16_t volume = ADC >> 2;

    volume =
        255 - ((255 - volume) * (255 - volume) / 255 * (255 - volume) / 255 * (255 - volume) / 255);

    if (volume > 255) {
      // PORTA = 0xAA;
      fail();
    }

    haveNewVolume = true;
    volumeFromADC = volume;
    volumeMode = false;
  }

  // turn timer off;
  ADCSRA = 0;
}
