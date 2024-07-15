#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  unsigned int rune;
  int size;
} RuneResult;

RuneResult DecodeRuneInString(const char *s, size_t length);
RuneResult DecodeLastRuneInString(const char *s, size_t length);
#ifdef __cplusplus
} // extern "C"
#endif