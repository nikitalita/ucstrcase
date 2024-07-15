#include "normalize.h"
#include "compose.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "lookup.h"
#include "normtables.h"
#include "data.h"
#include "simdutf-wrapper.h"
#include "pragmas.h"
typedef uint32_t char32_t;

/** TINYBUF */
void tinybuf_move_to_heap(TinyBuf *buf, size_t size);
void _expand_buf(TinyBuf *buf);


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define branchlessMIN(a, b) (b ^ ((a ^ b) & -(a < b)))


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
inline TINYBUF_ELEMENT tinybuf_pop(TinyBuf *buf) {
  return buf->buf_ptr[--buf->end];
}

inline
void tinybuf_put(TinyBuf *buf, size_t index, TINYBUF_ELEMENT value) {
  buf->buf_ptr[index] = value;
}

inline uint32_t tinybuf_size(TinyBuf *buf) { return buf->end; }

void tinybuf_free_heap(TinyBuf *buf) {
  if (buf->heap_buf_capacity && buf->buf_ptr && buf->buf_ptr != buf->_buf) {
    free(buf->buf_ptr);
    buf->buf_ptr = buf->_buf;
    buf->heap_buf_capacity = 0;
  }
	buf->end = 0;
}



/** COMPOSITION */


void decomposition_iter_push_back(DecompositionIter_t *iter, uint32_t ch);


void _void_iter_push_back(void* self, uint32_t ch);
inline void _void_iter_push_back(void* self, uint32_t ch){
	decomposition_iter_push_back((DecompositionIter_t *)self, ch);
}

void decomposition_iter_decompose(DecompositionIter_t *decompiter, uint32_t c) {
  	decompose(c, _void_iter_push_back, decompiter, decompiter->kind);
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

void decomposition_iter_case_fold(DecompositionIter_t *iter, uint32_t ch);

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


uint32_t recomp_next(RecompositionIter_t *recomp) {
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