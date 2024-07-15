#include "lookup.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "normtables.h"
#include "data.h"
#include "decode.h"

#ifndef NDEBUG
#include <assert.h>
#else
#define assert(x)
#endif
#if defined(__GNUC__)
#define likely(expr)    (__builtin_expect(!!(expr), 1))
#define unlikely(expr)  (__builtin_expect(!!(expr), 0))
#define noinline        __attribute__((noinline))
#elif defined(_MSC_VER)
#define likely(expr)    (expr)
#define unlikely(expr)  (expr)
#define noinline        __declspec(noinline)
#else
#define likely(expr)    (expr)
#define unlikely(expr)  (expr)
#define noinline
#endif

// Function prototypes
size_t my_hash(uint32_t key, uint32_t salt, size_t n);

size_t fully_decomposed(uint32_t c, const uint32_t **ret, const uint16_t *salt,
                        const DecomposedKV *kv, const uint32_t *chars,
                        size_t salt_len, size_t kv_len);

u16Pair pair_mph_lookup(uint32_t x, const uint16_t *salt,
                        const DecomposedKV *kv, size_t salt_len, size_t kv_len);
bool bool_mph_lookup(uint32_t x, const uint16_t *salt, const uint32_t *kv,
                     size_t salt_len, size_t kv_len);
uint8_t u8_mph_lookup(uint32_t x, const uint16_t *salt, const uint32_t *kv,
                      size_t salt_len, size_t kv_len);

inline size_t my_hash(uint32_t key, uint32_t salt, size_t n) {
  uint32_t y = key + salt;
  y *= 2654435769;
  y = y ^ (key * 0x31415926);
  return ((uint64_t)y * (uint64_t)n) >> 32;
}

inline bool bool_mph_lookup(uint32_t x, const uint16_t *salt,
                            const uint32_t *kv, size_t salt_len,
                            size_t kv_len) {
  uint32_t s = salt[my_hash(x, 0, salt_len)];
  uint32_t key_val = kv[my_hash(x, s, kv_len)];
  return key_val == x;
}

inline uint8_t u8_mph_lookup(uint32_t x, const uint16_t *salt,
                             const uint32_t *kv, size_t salt_len,
                             size_t kv_len) {
  uint32_t s = salt[my_hash(x, 0, salt_len)];
  uint32_t key_val = kv[my_hash(x, s, kv_len)];
  if (x == key_val >> 8) {
    return key_val & 0xff;
  }
  return 0;
}

const u16Pair DEFAULT_PAIR = {0, 0};

inline u16Pair pair_mph_lookup(uint32_t x, const uint16_t *salt,
                               const DecomposedKV *kv, size_t salt_len,
                               size_t kv_len) {
  uint32_t s = salt[my_hash(x, 0, salt_len)];
  DecomposedKV key_val = kv[my_hash(x, s, kv_len)];
  if (x == key_val.code) {
    return key_val.decomp;
  } else {
    return DEFAULT_PAIR;
  }
}

inline size_t fully_decomposed(uint32_t c, const uint32_t **ret,
                               const uint16_t *salt, const DecomposedKV *kv,
                               const uint32_t *chars, size_t salt_len,
                               size_t kv_len) {

  u16Pair result = pair_mph_lookup(c, salt, kv, salt_len, kv_len);
  if (result.b != 0) {
    *ret = &chars[result.a];
    return result.b;
  }
  *ret = NULL;
  return 1;
}

// Function definitions
uint8_t canonical_combining_class(uint32_t c) {
  return u8_mph_lookup(
      c, CANONICAL_COMBINING_CLASS_SALT, CANONICAL_COMBINING_CLASS_KV,
      CANONICAL_COMBINING_CLASS_SALT_SIZE, CANONICAL_COMBINING_CLASS_KV_SIZE);
}

// Function definitions
uint32_t composition_table(uint32_t c1, uint32_t c2) {
  if (c1 < 0x10000 && c2 < 0x10000) {
    uint32_t x = ((uint32_t)c1 << 16) | (uint32_t)c2;
    uint32_t s =
        COMPOSITION_TABLE_SALT[my_hash(x, 0, COMPOSITION_TABLE_SALT_SIZE)];
    CompositionTableKV key_val =
        COMPOSITION_TABLE_KV[my_hash(x, s, COMPOSITION_TABLE_KV_SIZE)];
    if (x == key_val.key) {
      return key_val.value;
    } else {
      return 0;
    }
  } else {
    return composition_table_astral(c1, c2);
  }
}

size_t canonical_fully_decomposed(uint32_t c, const uint32_t **ret) {
  return fully_decomposed(c, ret, CANONICAL_DECOMPOSED_SALT,
                          CANONICAL_DECOMPOSED_KV, CANONICAL_DECOMPOSED_CHARS,
                          CANONICAL_DECOMPOSED_SALT_SIZE,
                          CANONICAL_DECOMPOSED_KV_SIZE);
}
size_t compatibility_fully_decomposed(uint32_t c, const uint32_t **ret) {
  return fully_decomposed(
      c, ret, COMPATIBILITY_DECOMPOSED_SALT, COMPATIBILITY_DECOMPOSED_KV,
      COMPATIBILITY_DECOMPOSED_CHARS, COMPATIBILITY_DECOMPOSED_SALT_SIZE,
      COMPATIBILITY_DECOMPOSED_KV_SIZE);
}

// cjk_compat_variants_fully_decomposed
size_t cjk_compat_variants_fully_decomposed(uint32_t c, const uint32_t **ret) {
  return fully_decomposed(c, ret, CJK_COMPAT_VARIANTS_DECOMPOSED_SALT,
                          CJK_COMPAT_VARIANTS_DECOMPOSED_KV,
                          CJK_COMPAT_VARIANTS_DECOMPOSED_CHARS,
                          CJK_COMPAT_VARIANTS_DECOMPOSED_SALT_SIZE,
                          CJK_COMPAT_VARIANTS_DECOMPOSED_KV_SIZE);
}

bool is_combining_mark(uint32_t c) {
  return bool_mph_lookup(c, COMBINING_MARK_SALT, COMBINING_MARK_KV,
                         COMBINING_MARK_SALT_SIZE, COMBINING_MARK_KV_SIZE);
}

IsNormalized is_qc_nfc(uint32_t c) {
	return (IsNormalized)u8_mph_lookup(c, NFC_QC_SALT, NFC_QC_KV, NFC_QC_SALT_SIZE, NFC_QC_KV_SIZE);
}
IsNormalized is_qc_nfd(uint32_t c) {
	return (IsNormalized)u8_mph_lookup(c, NFD_QC_SALT, NFD_QC_KV, NFD_QC_SALT_SIZE, NFD_QC_KV_SIZE);
}
IsNormalized is_qc_nfkc(uint32_t c) {
	return (IsNormalized)u8_mph_lookup(c, NFKC_QC_SALT, NFKC_QC_KV, NFKC_QC_SALT_SIZE, NFKC_QC_KV_SIZE);
}
IsNormalized is_qc_nfkd(uint32_t c) {
	return (IsNormalized)u8_mph_lookup(c, NFKD_QC_SALT, NFKD_QC_KV, NFKD_QC_SALT_SIZE, NFKD_QC_KV_SIZE);
}

// typedef for a function pointer like is_qfc_nfc
typedef IsNormalized (*IsNormalizedFunc)(uint32_t);

IsNormalized quick_check(const char* s, IsNormalizedFunc f);

inline
IsNormalized quick_check(const char* s, IsNormalizedFunc f){
	IsNormalized result = Yes;
	size_t len = strlen(s);
	RuneResult runeResult = DecodeRuneInString(s, len);
	while(runeResult.rune != 0 && len > 0){
		IsNormalized res = f(runeResult.rune);
		switch(res){
			case No:
				return No;
			case Maybe:
				result = Maybe;
				break;
			case Yes:
				break;
		}
		s += runeResult.size;
		len -= runeResult.size;
		runeResult = DecodeRuneInString(s, len);
	}
	return result;
}

IsNormalized quick_check_nfc(const char* s){
	return quick_check(s, is_qc_nfc);
}

IsNormalized quick_check_nfd(const char* s){
	return quick_check(s, is_qc_nfd);
}

IsNormalized quick_check_nfkc(const char* s){
	return quick_check(s, is_qc_nfkc);
}

IsNormalized quick_check_nfkd(const char* s){
	return quick_check(s, is_qc_nfkd);
}

size_t stream_safe_trailing_nonstarters(uint32_t c) {
  return u8_mph_lookup(c, TRAILING_NONSTARTERS_SALT, TRAILING_NONSTARTERS_KV,
                       TRAILING_NONSTARTERS_SALT_SIZE,
                       TRAILING_NONSTARTERS_KV_SIZE);
}


// TODO: rename to "foldCase"
//
// caseFold returns the Unicode simple case-fold for r, if one exists, or r
// unmodified, if one does not exist.
uint32_t caseFold(uint32_t r) {
  // TODO: check if r is ASCII here?
  uint32_t h = (r * _CaseFoldsSeed) >> _CaseFoldsShift;
  foldPair p = _CaseFolds[h];
  if (p.From == r) {
    return p.To;
  }
  return r;
}

// TODO: rename
uint16_t *foldMap(uint32_t r) {
  uint32_t h = (r * _FoldMapSeed) >> _FoldMapShift;
  uint16_t *p = (uint16_t *)&_FoldMap[h];
  if (p[0] == r) {
    return p;
  }
  return NULL;
}

// foldMapExcludingUpperLower returns a static array, which is not idiomatic in
// C for returning multiple values. Instead, we could pass the output array as
// an argument to the function.
void foldMapExcludingUpperLower(uint32_t r, uint32_t result[2]) {
  uint32_t u = r;
  uint32_t h = (u * _FoldMapSeed) >> _FoldMapShift;
  if (_FoldMapExcludingUpperLower[h].r == u) {
    result[0] = _FoldMapExcludingUpperLower[h].a[0];
    result[1] = _FoldMapExcludingUpperLower[h].a[1];
  } else {
    result[0] = 0;
    result[1] = 0;
  }
}

const uint16_t * getFullCaseFold(uint32_t r){
	uint32_t h = (r * _FullCaseFoldsSeed) >> _FullCaseFoldsShift;
	const fullFoldPair * p = &_FullCaseFolds[h];
	if (unlikely(p->From == r)) {
		return p->To;
	}
	return NULL;
}