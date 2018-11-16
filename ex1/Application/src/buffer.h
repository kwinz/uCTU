#pragma once

#include <stdbool.h>
#include <stdint.h>

// needs to be <=255
#define BUF_SIZE 32

typedef struct {
  uint8_t data[BUF_SIZE];
  bool full;
  uint8_t reader;
  uint8_t writer;
} ringbuffer;

void buffer_init(ringbuffer *const buffer);
uint8_t buffer_count(ringbuffer *const buffer);
bool buffer_put(ringbuffer *const buffer, const uint8_t in);
bool buffer_take(ringbuffer *const buffer, uint8_t *const out);
