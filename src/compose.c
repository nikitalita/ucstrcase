#include "compose.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ucstrcase.h"
#include "normtables.h"
#include "data.h"
#include "simdutf-wrapper.h"

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

#define S_BASE 0xAC00UL
#define L_BASE 0x1100UL
#define V_BASE 0x1161UL
#define T_BASE 0x11A7UL
#define L_COUNT 19UL
#define V_COUNT 21UL
#define T_COUNT 28UL
#define N_COUNT (V_COUNT * T_COUNT)
#define S_COUNT (L_COUNT * N_COUNT)

#define S_LAST (S_BASE + S_COUNT - 1)
#define L_LAST (L_BASE + L_COUNT - 1)
#define V_LAST (V_BASE + V_COUNT - 1)
#define T_LAST (T_BASE + T_COUNT - 1)



void tinybuf_init(TinyBuf *buf);
TINYBUF_ELEMENT tinybuf_get(TinyBuf *buf, size_t index);
void decomposition_iter_push_back(DecompositionIter_t *iter, uint32_t ch);

bool is_hangul_syllable(uint32_t c) {
  return c >= S_BASE && c < (S_BASE + S_COUNT);
}
void decompose_hangul(DecompositionIter_t *iter, uint32_t s);
uint32_t hangul_decomposition_length(uint32_t s);
void decomposition_iter_decompose(DecompositionIter_t *decompiter, uint32_t c);

inline void decompose_hangul(DecompositionIter_t *iter, uint32_t s) {
  uint32_t s_index = s - S_BASE;
  uint32_t l_index = s_index / N_COUNT;
  decomposition_iter_push_back(iter, L_BASE + l_index);
  uint32_t v_index = (s_index % N_COUNT) / T_COUNT;
  decomposition_iter_push_back(iter, V_BASE + v_index);
  uint32_t t_index = s_index % T_COUNT;
  if (t_index > 0) {
    decomposition_iter_push_back(iter, T_BASE + t_index);
  }
}

inline uint32_t hangul_decomposition_length(uint32_t s) {
  uint32_t si = s - S_BASE;
  uint32_t ti = si % T_COUNT;
  if (ti > 0) {
    return 3;
  } else {
    return 2;
  }
}

void decomposition_iter_decompose(DecompositionIter_t *decompiter, uint32_t c) {
  if (c <= 0x7f) {
    decomposition_iter_push_back(decompiter, c);
    return;
  }

  if (is_hangul_syllable(c)) {
    decompose_hangul(decompiter, c);
    return;
  }
  const uint32_t *ret;
  size_t len;
  DecompositionType decomptype = decompiter->kind;
  switch (decomptype) {
	case Compatible:
	case CompatibleCaseFold:
		len = compatibility_fully_decomposed(c, &ret);
		if (ret != NULL){
			break;
		}
		// else fallthrough
  case Canonical:
  case CanonicalCaseFold:
    len = canonical_fully_decomposed(c, &ret);
    break;

  default:
    assert(0 && "Invalid decomposition type");
    return;
  }
  if (ret == NULL) {
    decomposition_iter_push_back(decompiter, c);
    return;
  }

	for (size_t i = 0; i < len; i++) {
		decomposition_iter_push_back(decompiter, ret[i]);
	}

}


void tinybuf_move_to_heap(TinyBuf *buf, size_t size);

inline
void tinybuf_move_to_heap(TinyBuf *buf, size_t size) {
	buf->heap_buf_capacity = size;
	buf->buf_ptr =
					(TINYBUF_ELEMENT *) malloc(TINYBUF_EL_SIZE * buf->heap_buf_capacity);
	memcpy(buf->buf_ptr, buf->_buf,
				 TINYBUF_EL_SIZE * (buf->heap_buf_capacity));
}


void tinybuf_init(TinyBuf *buf) {
  memset(buf->_buf, 0, TINYBUF_EL_SIZE * TINY_BUF_SIZE);
	buf->buf_ptr = buf->_buf;
	buf->heap_buf_capacity = 0;
//	tinybuf_move_to_heap(buf, TINY_BUF_SIZE * 2);
  buf->end = 0;
}


void noinline _expand_buf(TinyBuf *buf) {
	buf->heap_buf_capacity += 1;
	TINYBUF_ELEMENT *tmp_ptr = (TINYBUF_ELEMENT *) realloc(
					buf->buf_ptr, TINYBUF_EL_SIZE * buf->heap_buf_capacity);
	if (tmp_ptr == NULL) {
		tmp_ptr = (TINYBUF_ELEMENT *) malloc(TINYBUF_EL_SIZE * buf->heap_buf_capacity);
		memcpy(tmp_ptr, buf->buf_ptr, TINYBUF_EL_SIZE * buf->end);
		free(buf->buf_ptr);
	}
	buf->buf_ptr = tmp_ptr;
}

void tinybuf_push_back(TinyBuf *buf, DecompV value) {
  if (!buf->heap_buf_capacity && buf->end >= TINY_BUF_SIZE) {
			tinybuf_move_to_heap(buf, TINY_BUF_SIZE*2);
  } else if (buf->heap_buf_capacity) {
		if ((buf->end >= buf->heap_buf_capacity)) {
			_expand_buf(buf);
		}
  }

  buf->buf_ptr[buf->end] = value;
  buf->end++;
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define branchlessMIN(a, b) (b ^ ((a ^ b) & -(a < b)))

void tinybuf_drain(TinyBuf *buf, uint32_t start, uint32_t end) {
  uint32_t aend = MIN(end, buf->end);
  memcpy(buf->buf_ptr + start, buf->buf_ptr + aend,
          TINYBUF_EL_SIZE * (buf->end - aend));
  buf->end -= aend - start;
}

void tinybuf_truncate(TinyBuf *buf, uint32_t new_end) {
  if (new_end >= buf->end) {
    return;
  }
  buf->end = new_end;
}

inline TINYBUF_ELEMENT tinybuf_get(TinyBuf *buf, size_t index) {
  return buf->buf_ptr[index];
}
TINYBUF_ELEMENT tinybuf_pop(TinyBuf *buf);
inline TINYBUF_ELEMENT tinybuf_pop(TinyBuf *buf) {
  return buf->buf_ptr[--buf->end];
}
void tinybuf_put(TinyBuf *buf, size_t index, TINYBUF_ELEMENT value);
inline
void tinybuf_put(TinyBuf *buf, size_t index, TINYBUF_ELEMENT value) {
  buf->buf_ptr[index] = value;
}

uint32_t tinybuf_size(TinyBuf *buf);
inline uint32_t tinybuf_size(TinyBuf *buf) { return buf->end; }

void tinybuf_free_heap(TinyBuf *buf) {
  if (buf->heap_buf_capacity && buf->buf_ptr && buf->buf_ptr != buf->_buf) {
    free(buf->buf_ptr);
    buf->buf_ptr = buf->_buf;
    buf->heap_buf_capacity = 0;
  }
	buf->end = 0;
}

/// You MUST call this function before using the iterator
/// This function initializes the iterator with the given string and length
/// and the kind of decomposition to use.
/// `decomposition_iter_destroy` MUST be called after the iterator is no longer
/// needed.
void decomposition_iter_init(DecompositionIter_t *iter, const char *str,
                             size_t len, DecompositionType kind) {
  iter->str = str;
  iter->len = len;
  iter->ready_start = 0;
  iter->ready_end = 0;
  iter->pos = 0;
  iter->kind = kind;
  tinybuf_init(&iter->buffer);
#ifndef NDEBUG
  iter->not_initialized = 0;
#endif
}

void decomposition_iter_destroy(DecompositionIter_t *iter) {
  tinybuf_free_heap(&iter->buffer);
}
#define MAX(a, b) ((a) > (b) ? (a) : (b))
// TODO: something better than insertion sort
void decomposition_sort_pending(DecompositionIter_t *iter) {
  int64_t size = tinybuf_size(&iter->buffer);
	int64_t i = iter->ready_end;
  while (i < size - 1) {
    uint32_t j = i + 1;
		DecompV elem_i = tinybuf_get(&iter->buffer, i);
		DecompV elem_j = tinybuf_get(&iter->buffer, j);
    if (elem_j.cclass < elem_i.cclass) {
      tinybuf_put(&iter->buffer, i, elem_j);
      tinybuf_put(&iter->buffer, j, elem_i);
      i = MAX(iter->ready_end, i - 2);
    } else{
			i++;
		}
  }
}

void decomposition_iter_push_back(DecompositionIter_t *iter, uint32_t ch) {
  uint8_t class = canonical_combining_class(ch);
  if (class == 0) {
    decomposition_sort_pending(iter);
    tinybuf_push_back(&iter->buffer, (DecompV){class, ch});
    iter->ready_end = tinybuf_size(&iter->buffer);
  } else {
    tinybuf_push_back(&iter->buffer, (DecompV){class, ch});
  }
}
const uint16_t * getFullCaseFold(uint32_t r);
void decomposition_iter_case_fold(DecompositionIter_t *iter, uint32_t ch);

inline
const uint16_t * getFullCaseFold(uint32_t r){
	uint32_t h = (r * _FullCaseFoldsSeed) >> _FullCaseFoldsShift;
	const fullFoldPair * p = &_FullCaseFolds[h];
	if (unlikely(p->From == r)) {
		return p->To;
	}
	return NULL;
}
inline
void decomposition_iter_case_fold(DecompositionIter_t *iter, uint32_t ch) {
	const uint16_t *fold = getFullCaseFold(ch);
	if (likely(!fold)){
			decomposition_iter_decompose(iter, caseFold(ch));
			return;
	}
	int i = 0;
	uint16_t fold_ch = fold[i];
	while (fold_ch != 0 && i < 3) {
		decomposition_iter_decompose(iter, fold_ch);
		fold_ch = fold[++i];
	}
}


void decomposition_iter_reset_buffer(DecompositionIter_t *iter) {
  uint32_t pending = tinybuf_size(&iter->buffer) - iter->ready_end;
  tinybuf_drain(&iter->buffer, 0, iter->ready_end);
  tinybuf_truncate(&iter->buffer, pending);
  iter->ready_start = 0;
  iter->ready_end = 0;
}

void decomposition_iter_increment_next_ready(DecompositionIter_t *iter) {
  uint32_t next = iter->ready_start + 1;
  if (next == iter->ready_end) {
    decomposition_iter_reset_buffer(iter);
  } else {
    iter->ready_start = next;
  }
}

void decomposition_iter_reset(DecompositionIter_t *iter) {
	iter->pos = 0;
	iter->ready_start = 0;
	iter->ready_end = 0;
	tinybuf_truncate(&iter->buffer, 0);
}


DecompV decomposition_iter_next_ccc(DecompositionIter_t *iter) {
#ifndef NDEBUG
	assert(!iter->not_initialized && "decomposition_iter_init must be called before use");
#endif
	while (iter->ready_end == 0) {
		if (iter->pos < iter->len) {
			RuneResult result =
							DecodeRuneInString(iter->str + iter->pos, iter->len - iter->pos);
			// end of string
			if (result.rune == 0){
				iter->pos += result.size;
				decomposition_sort_pending(iter);
				iter->ready_end = tinybuf_size(&iter->buffer);
				break;
			}
			if (iter->kind == CanonicalCaseFold || iter->kind == CompatibleCaseFold) {
				decomposition_iter_case_fold(iter, result.rune);
			} else {
				decomposition_iter_decompose(iter, result.rune);
			}
			iter->pos += result.size;
		} else {
			if (tinybuf_size(&iter->buffer) == 0) {
				tinybuf_free_heap(&iter->buffer);
				return (DecompV){0,0};
			} else {
				decomposition_sort_pending(iter);
				iter->ready_end = tinybuf_size(&iter->buffer);
				break;
			}
		}
	}
	DecompV ch = tinybuf_get(&iter->buffer, iter->ready_start);
	decomposition_iter_increment_next_ready(iter);
	return ch;
}
uint32_t decomposition_iter_next(DecompositionIter_t *iter) {
	return decomposition_iter_next_ccc(iter).ch;
}


char32_t compose_hangul(char32_t a, char32_t b) {
	if (a >= L_BASE && a <= L_LAST && b >= V_BASE && b <= V_LAST) {
		uint32_t l_index = a - L_BASE;
		uint32_t v_index = b - V_BASE;
		uint32_t lv_index = l_index * N_COUNT + v_index * T_COUNT;
		return S_BASE + lv_index;
	} else if (a >= S_BASE && a <= S_LAST && b >= T_BASE && b <= T_LAST &&
						 (a - S_BASE) % T_COUNT == 0) {
		return a + (b - T_BASE);
	}
	return 0;
}

char32_t compose(char32_t c1, char32_t c2) {
	char32_t ret = compose_hangul(c1, c2);
	if (ret == 0) {
		ret = composition_table(c1, c2);
	}
	return ret;
}

#define NO_CCC 0xff

void recomp_init(RecompositionIter_t *recomp, const char *str,
								 size_t len, DecompositionType kind) {
	DecompositionType decomp_kind = kind;
//	if (decomp_kind == CanonicalCaseFold) {
//		decomp_kind = Canonical;
//	} else if (decomp_kind == CompatibleCaseFold) {
//		decomp_kind = Compatible;
//	}
	decomposition_iter_init(&recomp->iter, str, len, decomp_kind);
	recomp->kind = kind;
	recomp->state = Composing;
	recomp->state_next = 0;
	tinybuf_init(&recomp->buffer);
	recomp->composee = 0;
	recomp->last_ccc = NO_CCC;
}

void recomp_destroy(RecompositionIter_t *recomp) {
	decomposition_iter_destroy(&recomp->iter);
	tinybuf_free_heap(&recomp->buffer);
}
char32_t _recomp_ret_char(RecompositionIter_t *recomp, char32_t ch);
char32_t recomp_take_composee(RecompositionIter_t *recomp);


inline
char32_t _recomp_ret_char(RecompositionIter_t *recomp, char32_t ch) {
//	if (recomp->kind == CanonicalCaseFold || recomp->kind == CompatibleCaseFold) {
//		return caseFold(ch);
//	}
	return ch;
}

inline
char32_t recomp_take_composee(RecompositionIter_t *recomp) {
	char32_t composee = recomp->composee;
	recomp->composee = 0;
	return _recomp_ret_char(recomp, composee);
}


void recomp_set_state(RecompositionIter_t *recomp, RecompositionState state, uint32_t state_next) {
	recomp->state = state;
	if (state != Composing) {
		recomp->state_next = state_next;
	} else {
		recomp->state_next = 0;
	}
}


char32_t recomp_next(RecompositionIter_t *recomp) {
	while (1) {
		switch (recomp->state) {
			case Composing: {
				for (DecompV chc = decomposition_iter_next_ccc(&recomp->iter); chc.ch != 0; chc = decomposition_iter_next_ccc(&recomp->iter)) {
					char32_t ch = chc.ch;
					uint8_t ch_class = chc.cclass;
					char32_t k = recomp->composee;
					if (k == 0) {
						if (ch_class != 0) {
							return _recomp_ret_char(recomp, ch);
						}
						recomp->composee = ch;
						continue;
					}
					int16_t l_class = recomp->last_ccc;
					if (l_class == NO_CCC) {
						char32_t r = compose(k, ch);
//						if (!r && recomp->kind == CanonicalCaseFold || recomp->kind == CompatibleCaseFold) {
//							r = compose(caseFold(k), caseFold(ch));
//						}
						if (r != 0) {
							recomp->composee = r;
							continue;
						}
						if (ch_class == 0) {
							recomp->composee = ch;
							return _recomp_ret_char(recomp, k);
						}
						tinybuf_push_back(&recomp->buffer, (DecompV) {ch_class, ch});
						recomp->last_ccc = ch_class;
					} else {
						if (l_class >= ch_class
								|| (ch_class == 0 && recomp->composee != 0)) {
							if (ch_class == 0) {
								recomp->composee = ch;
								recomp->last_ccc = NO_CCC;
								recomp_set_state(recomp, Purging, 0);
								return _recomp_ret_char(recomp, k);
							}
							tinybuf_push_back(&recomp->buffer, (DecompV) {ch_class, ch});
							recomp->last_ccc = ch_class;
							continue;
						}
						char32_t r = compose(k, ch);
						if (r != 0) {
							recomp->composee = r;
							continue;
						}
						tinybuf_push_back(&recomp->buffer, (DecompV) {ch_class, ch});
						recomp->last_ccc = ch_class;
					}
				}
				// end of decomposition
				recomp->state = Finished;
				if (recomp->composee != 0) {
					return recomp_take_composee(recomp);
				}
			} break;
			case Purging: {
				char32_t next_char = tinybuf_size(&recomp->buffer) > recomp->state_next ? tinybuf_get(&recomp->buffer, recomp->state_next).ch : 0;
				if (next_char == 0) {
					tinybuf_truncate(&recomp->buffer, 0);
					recomp_set_state(recomp, Composing, 0);
				} else {
					recomp->state_next++;
					return _recomp_ret_char(recomp, next_char);
				}
			} break;
			case Finished: {
				char32_t next_char = tinybuf_size(&recomp->buffer) > recomp->state_next ? tinybuf_get(&recomp->buffer, recomp->state_next).ch : 0;
				if (next_char == 0) {
					tinybuf_truncate(&recomp->buffer, 0);
					tinybuf_free_heap(&recomp->buffer);
					return recomp_take_composee(recomp);
				} else {
					recomp->state_next++;
					return _recomp_ret_char(recomp, next_char);
				}
			} break;
			default:
				assert(0 && "Invalid state");
				break;
		}
	}
	assert(0 && "Unreachable!");
}