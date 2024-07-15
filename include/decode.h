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
size_t char_utf32_to_utf8(uint32_t utf32, const char *buffer);
int RuneLen(unsigned int r);
RuneResult DecodeRuneInUTF16String(const uint16_t *input,
																	 size_t input_size);
#ifdef __cplusplus
} // extern "C"
#endif