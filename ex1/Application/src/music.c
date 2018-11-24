#include "music.h"
#include "adc.h"
#include "mp3.h"
#include "sdcard.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// clang-format off
// clang-format on

const PROGMEM uint32_t songstarts[] = {
    3200000, 3590240, 3639392, 4030368, 4360416, 4385760, 4675648, 5004992, 5052096, 5184448,
    5460224, 5692928, 5765088, 6109984, 6778144, 7059968, 7096832, 7148256, 7270240};

const PROGMEM uint32_t songdurations[] = {390240, 49152,  390960, 330048, 25344, 289872, 329328,
                                          47104,  132336, 275760, 232704, 72144, 344880, 668160,
                                          281808, 36864,  51424,  121968, 860256};

static int8_t oldVolume = 0;

sdcard_block_t buffer;
static uint8_t currentSong = SONG_NOSONG;
uint32_t byteAddress = 5460224LU;
uint32_t songEnd;
static void (*songOverCallback)(Song_t song);

/**
 * Be sure to call
 * sdcardInit(); mp3Init(&dataRequestCallback); and adcInit(); first
 */
void songPlay(Song_t song, void (*songOver)(Song_t song)) {
  if (song != SONG_NOSONG) {
    const uint32_t songStart = pgm_read_dword(songstarts + song);
    songEnd = songStart + pgm_read_dword(songdurations + song);
    byteAddress = songStart;
  }
  currentSong = song;
  songOverCallback = songOver;
  // mp3SetVolume(0xff);
}

void songTick(void) {
  if (currentSong == SONG_NOSONG) {
    return;
  }

  if (byteAddress >= songEnd) {
    currentSong = SONG_NOSONG;
    (*songOverCallback)(currentSong);
    return;
  }

  while (!mp3Busy()) {
    // PORTK++;
    cli();
    if (haveNewVolume) {
      haveNewVolume = false;
      if (oldVolume != volumeFromADC) {
        oldVolume = volumeFromADC;
        sei();
        // mp3SetVolume(volumeFromADC);
      }
    }
    sei();

    // we disable interrupts during mp3 send because
    // we get audio glitches if someone else accesses the SPI
    sdcardReadBlock(byteAddress, buffer);
    mp3SendMusic(buffer);
    //
    byteAddress += 32;
  }
}