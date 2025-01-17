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
int ucstrncasecmp(const char *s, const char *t, size_t len);
int ucstrcasecmp(const char *s, const char *t);
bool EqualFold(const char *s, const char *t);
#ifdef __cplusplus
}
#endif