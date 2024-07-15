#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
int ucstrncasecmp(const char *s, const char *t, size_t len);
int ucstrcasecmp(const char *s, const char *t);
bool EqualFold(const char *s, const char *t);
#ifdef __cplusplus
}
#endif