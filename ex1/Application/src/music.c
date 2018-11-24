#include "music.h"
#include "adc.h"
#include "mp3.h"
#include "sdcard.h"
#include "tools.h"

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

static uint8_t oldVolume = 0;

static sdcard_block_t buffer;
static uint8_t currentSong = SONG_NOSONG;
static uint32_t byteAddress = 5460224LU;
static uint32_t songEnd;
static void (*songOverCallback)(const Song_t song);

static void dataRequestCallback(void) {
  // we are woken up from sleep due to the callback interrupt
  // don't do anything else here, data feed is handled in background()
  sei();
}

/**
 * Handles
 * sdcardInit(); mp3Init(&dataRequestCallback); and adcInit();
 */
void songInit(void) {
  if (HAVE_MP3_BOARD) {
    PORTK++;
    spiInit();
    PORTK++;
    const error_t sdcarderror = sdcardInit();
    if (SUCCESS != sdcarderror) {
      PORTA = 0xAA;
      fail();
    }

    mp3Init(&dataRequestCallback);
  }
}

/**
 * Plays a song and then calls songOver() callback with the song that just finished
 * Be sure to call songInit() exactly once first.
 */
void songPlay(const Song_t song, void (*songOver)(const Song_t song)) {
  if (song != SONG_NOSONG) {
    const uint32_t songStart = pgm_read_dword(songstarts + song);
    songEnd = songStart + pgm_read_dword(songdurations + song);
    byteAddress = songStart;
  }
  currentSong = song;
  songOverCallback = songOver;
  // mp3SetVolume(0xff);
}

/**
 * Needs to be called regularly (5ms?) to keep playing a song set with songPlay.
 *
 * Be sure to call songInit() exactly once first. Also it is recommended to call #adcInit() once
 * before.
 */
void songTick(void) {
  if (currentSong == SONG_NOSONG) {
    return;
  }

  if (byteAddress >= songEnd) {
    Song_t currentSongCopy = currentSong;
    currentSong = SONG_NOSONG;
    (*songOverCallback)(currentSongCopy);

    return;
  }

  if (!mp3Busy()) {
    cli();
    if (haveNewVolume) {
      haveNewVolume = false;
      if (oldVolume != volumeFromADC) {
        oldVolume = volumeFromADC;
        sei();
        mp3SetVolume(volumeFromADC);
      }
    }
    sei();
  }

  for (uint8_t i = 0; i < 5 && !mp3Busy(); i++) {
    sdcardReadBlock(byteAddress, buffer);
    mp3SendMusic(buffer);
    byteAddress += 32;
  }
}