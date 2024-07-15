#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  Canonical,
  Compatible,
  CanonicalCaseFold,
  CompatibleCaseFold,
} DecompositionType;

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*PushBackFunc) (void *self, uint32_t ch);
void decompose_hangul(uint32_t s, PushBackFunc push_back, void *self);
uint32_t hangul_decomposition_length(uint32_t s);
void decompose(uint32_t c, PushBackFunc push_back, void *self, DecompositionType decomptype);
uint32_t compose_hangul(uint32_t a, uint32_t b);
uint32_t compose(uint32_t c1, uint32_t c2);
bool is_hangul_syllable(uint32_t c);
#ifdef __cplusplus
}
#endif