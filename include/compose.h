#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// check if the char32_t type exists
#if defined(__cplusplus) && __cplusplus >= 201103L
#include <uchar.h>
#else
typedef uint32_t char32_t;

#endif
// check if _Static_assert is available (C11 feature)
#if !defined(__cplusplus) && !defined(static_assert) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L 
#define static_assert _Static_assert
#endif

typedef enum {
  Canonical,
  Compatible,
  CanonicalCaseFold,
  CompatibleCaseFold,
} DecompositionType;


typedef struct {
  uint8_t cclass;
//	char32_t ch : 24;
	char32_t ch;
} DecompV;

#define TINY_BUF_SIZE 4
#define TINYBUF_ELEMENT DecompV
#define TINYBUF_EL_SIZE sizeof(TINYBUF_ELEMENT)

// This is a tiny buffer that can be used to store a small number of uint32_t
// values, but automatically allocates a heap buffer if the number of values
// exceeds TINY_BUF_SIZE.
typedef struct {
  TINYBUF_ELEMENT _buf[TINY_BUF_SIZE];
  TINYBUF_ELEMENT *buf_ptr;
  uint32_t end;
  uint32_t heap_buf_capacity;
} TinyBuf;

typedef struct DecompositionIter{
  const char *str;
  size_t len;
  size_t pos;
  TinyBuf buffer;
  uint32_t ready_start;
  uint32_t ready_end;
  DecompositionType kind;
#ifndef NDEBUG
  uint8_t not_initialized;
#endif
} DecompositionIter_t;

typedef enum {
	Composing,
	Purging,
	Finished,
} RecompositionState;

typedef struct RecompositionIter{
	DecompositionIter_t iter;
	RecompositionState state;

	TinyBuf buffer;
	char32_t composee;
	uint32_t state_next;
	DecompositionType kind;
	uint8_t last_ccc;

} RecompositionIter_t;

typedef enum {
	Yes,
	No,
	Maybe,
} IsNormalized;

#ifdef __cplusplus
extern "C" {
#endif
uint8_t canonical_combining_class(uint32_t c);
uint32_t composition_table(uint32_t c1, uint32_t c2);
size_t canonical_fully_decomposed(uint32_t c, const uint32_t **ret);
size_t compatibility_fully_decomposed(uint32_t c, const uint32_t **ret);
size_t cjk_compat_variants_fully_decomposed(uint32_t c, const uint32_t **ret);
bool is_combining_mark(uint32_t c);
size_t stream_safe_trailing_nonstarters(uint32_t c);

void decomposition_iter_init(DecompositionIter_t *iter, const char *str,
                             size_t len, DecompositionType kind);
uint32_t decomposition_iter_next(DecompositionIter_t *iter);
void decomposition_iter_destroy(DecompositionIter_t *iter);
void decomposition_iter_reset(DecompositionIter_t *iter);

void recomp_destroy(RecompositionIter_t *recomp);
void recomp_init(RecompositionIter_t *recomp, const char *str,
								 size_t len, DecompositionType kind);
char32_t recomp_next(RecompositionIter_t *recomp);
IsNormalized quick_check_nfc(const char* s);
IsNormalized quick_check_nfd(const char* s);
IsNormalized quick_check_nfkc(const char* s);
IsNormalized quick_check_nfkd(const char* s);
IsNormalized is_qc_nfc(uint32_t c);
IsNormalized is_qc_nfd(uint32_t c);
IsNormalized is_qc_nfkc(uint32_t c);
IsNormalized is_qc_nfkd(uint32_t c);

#ifdef __cplusplus
}
#endif