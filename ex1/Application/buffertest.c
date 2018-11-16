#include "src/buffer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv) {
  ringbuffer a;
  buffer_init(&a);

  assert(buffer_put(&a, 1));
  assert(buffer_put(&a, 2));
  // assert(buffer_put(&a, 3));
  printf("size: %u\n", buffer_count(&a));

  uint8_t out;

  assert(buffer_take(&a, &out));
  printf("took: %u\n", out);
  assert(buffer_take(&a, &out));
  printf("took: %u\n", out);
  // assert(buffer_take(&a, &out));
  // printf("took: %u\n", out);

  printf("size: %u\n", buffer_count(&a));
  assert(buffer_put(&a, 4));
  assert(buffer_put(&a, 5));
  printf("size: %u\n", buffer_count(&a));

  assert(buffer_take(&a, &out));
  printf("took: %u\n", out);
  printf("size: %u\n", buffer_count(&a));

  assert(buffer_take(&a, &out));
  printf("took: %u\n", out);
  printf("size: %u\n", buffer_count(&a));

  return 0;
}