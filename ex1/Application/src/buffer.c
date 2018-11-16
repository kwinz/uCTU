#include "buffer.h"
#include <string.h> /* memset */

static uint8_t next(uint8_t current);
static bool isFull(ringbuffer *const buffer);
static bool isEmpty(ringbuffer *const buffer);

// public methods

uint8_t buffer_count(ringbuffer *const buffer) {
  if (isFull(buffer)) {
    return BUF_SIZE;
  }

    if (isEmpty(buffer)) {
    return 0;
  }

  return (buffer->reader > buffer->writer) ? (buffer->reader - buffer->writer)
                                           : (buffer->writer - buffer->reader);
}

bool buffer_init(ringbuffer *const buffer) { memset(buffer, 0, sizeof(ringbuffer)); }

/**
 *
 * @return true if successful, retry on on false
 */
bool buffer_put(ringbuffer *const buffer, const uint8_t in) {
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

bool buffer_take(ringbuffer *const buffer, uint8_t *const out) {
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

bool isFull(ringbuffer *const buffer) { return (buffer->reader == buffer->writer) && buffer->full; }

bool isEmpty(ringbuffer *const buffer) {
  return (buffer->reader == buffer->writer) && !(buffer->full);
}