#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum Songs {
  SONG_BATMAN = 0,
  SONG_GLORY = 1,
  SONG_BUBBLE = 2,
  SONG_BOSSDRUMS = 3,
  SONG_BOSS_COMPLETE = 4,
  SONG_HIMALAYAS = 5,
  SONG_TRANSILVANIA = 6,
  SONG_IMPRESSIVE = 7,
  SONG_KUNGFU = 8,
  SONG_LEMMINGS = 9,
  SONG_SIMCITY = 10,
  SONG_SMB_STAR = 11,
  SONG_SMB_THEME = 12,
  SONG_SPACEQUEST = 13,
  SONG_TETRIS = 14,
  SONG_UNREAL = 15,
  SONG_VICTORY_FANFARE = 16,
  SONG_Z2CAVE = 17,
  SONG_ZGAGA = 18,
  SONG_NOSONG
} Song_t;

void songInit(void);
void songPlay(const Song_t song, void (*songOver)(const Song_t song));
void songTick(void);
Song_t getCurrentSong(void);