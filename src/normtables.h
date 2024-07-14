
// NOTE: The following code was generated by "internal/unicode.py", do not edit directly

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef struct{
    uint32_t key;
    uint16_t value;
} CompositionTableKV;

typedef struct {
    uint16_t a;
    uint16_t b;
} u16Pair;

typedef struct {
    uint32_t code;
    u16Pair decomp;
} DecomposedKV;

#ifdef __cplusplus
extern "C" {
#endif
uint32_t composition_table_astral(uint32_t ch, uint32_t c2);
bool is_public_assigned(uint32_t c);

#ifdef __cplusplus
}
#endif

extern const uint16_t CANONICAL_COMBINING_CLASS_SALT[922];
extern const size_t CANONICAL_COMBINING_CLASS_SALT_SIZE;
extern const uint32_t CANONICAL_COMBINING_CLASS_KV[922];
extern const size_t CANONICAL_COMBINING_CLASS_KV_SIZE;


extern const uint16_t COMPOSITION_TABLE_SALT[928];
extern const size_t COMPOSITION_TABLE_SALT_SIZE;
extern const CompositionTableKV COMPOSITION_TABLE_KV[928];
extern const size_t COMPOSITION_TABLE_KV_SIZE;

extern const uint32_t CANONICAL_DECOMPOSED_CHARS[3406];
extern const size_t CANONICAL_DECOMPOSED_CHARS_SIZE;
extern const size_t CANONICAL_DECOMPOSED_CHARS_MAX_ITEM_LEN;
extern const size_t CANONICAL_DECOMPOSED_CHARS_MAX_ITEM_UTF8_LEN;

extern const uint16_t CANONICAL_DECOMPOSED_SALT[2061];
extern const size_t CANONICAL_DECOMPOSED_SALT_SIZE;
extern const DecomposedKV CANONICAL_DECOMPOSED_KV[2061];
extern const size_t CANONICAL_DECOMPOSED_KV_SIZE;

extern const uint32_t COMPATIBILITY_DECOMPOSED_CHARS[5735];
extern const size_t COMPATIBILITY_DECOMPOSED_CHARS_SIZE;
extern const size_t COMPATIBILITY_DECOMPOSED_CHARS_MAX_ITEM_LEN;
extern const size_t COMPATIBILITY_DECOMPOSED_CHARS_MAX_ITEM_UTF8_LEN;

extern const uint16_t COMPATIBILITY_DECOMPOSED_SALT[3812];
extern const size_t COMPATIBILITY_DECOMPOSED_SALT_SIZE;
extern const DecomposedKV COMPATIBILITY_DECOMPOSED_KV[3812];
extern const size_t COMPATIBILITY_DECOMPOSED_KV_SIZE;

extern const uint32_t CJK_COMPAT_VARIANTS_DECOMPOSED_CHARS[2004];
extern const size_t CJK_COMPAT_VARIANTS_DECOMPOSED_CHARS_SIZE;
extern const size_t CJK_COMPAT_VARIANTS_DECOMPOSED_CHARS_MAX_ITEM_LEN;
extern const size_t CJK_COMPAT_VARIANTS_DECOMPOSED_CHARS_MAX_ITEM_UTF8_LEN;

extern const uint16_t CJK_COMPAT_VARIANTS_DECOMPOSED_SALT[1002];
extern const size_t CJK_COMPAT_VARIANTS_DECOMPOSED_SALT_SIZE;
extern const DecomposedKV CJK_COMPAT_VARIANTS_DECOMPOSED_KV[1002];
extern const size_t CJK_COMPAT_VARIANTS_DECOMPOSED_KV_SIZE;


extern const uint16_t COMBINING_MARK_SALT[2450];
extern const size_t COMBINING_MARK_SALT_SIZE;
extern const uint32_t COMBINING_MARK_KV[2450];
extern const size_t COMBINING_MARK_KV_SIZE;



extern const uint16_t TRAILING_NONSTARTERS_SALT[1090];
extern const size_t TRAILING_NONSTARTERS_SALT_SIZE;
extern const uint32_t TRAILING_NONSTARTERS_KV[1090];
extern const size_t TRAILING_NONSTARTERS_KV_SIZE;
