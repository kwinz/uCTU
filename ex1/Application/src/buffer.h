#pragma once

#include "buffer.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h> /* memset */

// needs to be <=255
#define BUF_SIZE 128

typedef struct {
  uint8_t data[BUF_SIZE];
  bool full;
  uint8_t reader;
  uint8_t writer;
} ringbuffer;

void inline __attribute__((always_inline)) buffer_init(volatile ringbuffer *buffer);
uint8_t inline __attribute__((always_inline)) buffer_count(volatile ringbuffer *buffer);
bool inline __attribute__((always_inline))
buffer_put(volatile ringbuffer *buffer, const uint8_t in);
bool inline __attribute__((always_inline))
buffer_take(volatile ringbuffer *buffer, uint8_t *const out);
static inline __attribute__((always_inline)) uint8_t next(uint8_t current);
static inline __attribute__((always_inline)) bool isFull(volatile ringbuffer *buffer);
static inline __attribute__((always_inline)) bool isEmpty(volatile ringbuffer *buffer);

// public methods

uint8_t buffer_count(volatile ringbuffer *buffer) {
  if (isFull(buffer)) {
    return BUF_SIZE;
  }

  if (isEmpty(buffer)) {
    return 0;
  }

  return (buffer->reader > buffer->writer) ? (buffer->reader - buffer->writer)
                                           : (buffer->writer - buffer->reader);
}

void buffer_init(volatile ringbuffer *buffer) { memset((void *)buffer, 0, sizeof(ringbuffer)); }

/**
 *
 * @return true if successful, retry on on false
 */
bool buffer_put(volatile ringbuffer *buffer, const uint8_t in) {
  const bool full = isFull(buffer);
  if (full) {
    return false;
  }

  buffer->data[buffer->writer] = in;

  buffer->writer = next(buffer->writer);
  if (buffer->writer == buffer->reader) {
    buffer->full = true;
  }

  return true;
}

bool buffer_take(volatile ringbuffer *buffer, uint8_t *const out) {
  const bool empty = isEmpty(buffer);

  if (empty) {
    return false;
  }

  *out = buffer->data[buffer->reader];

  buffer->reader = next(buffer->reader);
  if (buffer->writer == buffer->reader) {
    buffer->full = false;
  }

  return true;
}

// (private) static methods

uint8_t next(uint8_t current) {
  if (current == BUF_SIZE - 1) {
    return 0;
  } else {
    return ++current;
  }
}

bool isFull(volatile ringbuffer *buffer) {
  return (buffer->reader == buffer->writer) && buffer->full;
}

bool isEmpty(volatile ringbuffer *buffer) {
  return (buffer->reader == buffer->writer) && !(buffer->full);
}