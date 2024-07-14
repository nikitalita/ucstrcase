#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  unsigned int rune;
  int size;
} RuneResult;

RuneResult DecodeRuneInString(const char *s, size_t length);
RuneResult _unsafe_utf8_decode(const char *buf, size_t _slen);

int ucstrncasecmp(const char *s, const char *t, size_t len);
int ucstrcasecmp(const char *s, const char *t);
bool EqualFold(const char *s, const char *t);
uint32_t caseFold(uint32_t c);
#ifdef __cplusplus
}
#endif