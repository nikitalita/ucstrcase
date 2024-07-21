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
typedef uint32_t char32_t;

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






uint32_t compose_hangul(uint32_t a, uint32_t b) {
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

uint32_t compose(uint32_t c1, uint32_t c2) {
	char32_t ret = compose_hangul(c1, c2);
	if (ret == 0) {
		ret = composition_table(c1, c2);
	}
	return ret;
}


bool is_hangul_syllable(uint32_t c) {
  return c >= S_BASE && c < (S_BASE + S_COUNT);
}


inline void decompose_hangul(uint32_t s, PushBackFunc push_back, void *iter) {
  uint32_t s_index = s - S_BASE;
  uint32_t l_index = s_index / N_COUNT;
  push_back(iter, L_BASE + l_index);
  uint32_t v_index = (s_index % N_COUNT) / T_COUNT;
  push_back(iter, V_BASE + v_index);
  uint32_t t_index = s_index % T_COUNT;
  if (t_index > 0) {
    push_back(iter, T_BASE + t_index);
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

inline void decompose(uint32_t c, PushBackFunc push_back, void *self, DecompositionType decomptype){
	if (c <= 0x7f) {
		push_back(self, c);
		return;
	}

	if (is_hangul_syllable(c)) {
		decompose_hangul(c, push_back, self);
		return;
	}
	const uint32_t *ret;
	size_t len;
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
		case NFKC_CF:
			len = NFKC_CF_decomposed(c, &ret);
			if (len == 0){
				// ignorable character, return
				return;
			}
			break;

		default:
			assert(0 && "Invalid decomposition type");
			return;
	}
	if (ret == NULL) {
		push_back(self, c);
		return;
	}

	for (size_t i = 0; i < len; i++) {
		push_back(self, ret[i]);
	}

}

