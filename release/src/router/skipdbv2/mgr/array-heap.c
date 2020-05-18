#include <stdlib.h>
#include <stdio.h>

#include "array-heap.h"

int array_init(array* arr, int size) {
  arr->data = realloc(NULL, sizeof(void*) * size);
  if (0 == (size_t)arr->data) {
    return -1;
  }
  arr->length = size;
  arr->index = 0;
  return 0;
}

int array_push(array* arr, void* data) {
  ((size_t*)arr->data)[arr->index] = (size_t)data;
  arr->index += 1;
  if (arr->index >= arr->length) {
    if (-1 == array_grow(arr, arr->length * 2))
    {
      return -1;
    }
  }
  return arr->index - 1;
}

int array_grow(array* arr, int size) {
  if (size <= arr->length) {
    return -1;
  }
  arr->data = realloc(arr->data, sizeof(void*) * size);
  if (-1 == (size_t)arr->data) {
    return -1;
  }
  arr->length = size;
  return 0;
}

void array_free(array* arr, void (*free_element)(void*)) {
  int i;
  for (i = 0; i < arr->index; i += 1) {
    free_element((void*)((size_t*)arr->data)[i]);
  }
  free(arr->data);
  arr->index = -1;
  arr->length = 0;
  arr->data = NULL;
}
