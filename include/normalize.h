#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "compose.h"
typedef struct {
  uint8_t cclass;
//	uint32_t ch : 24;
	uint32_t ch;
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
	uint32_t composee;
	uint32_t state_next;
	DecompositionType kind;
	uint8_t last_ccc;

} RecompositionIter_t;


#ifdef __cplusplus
extern "C" {
#endif
void decomposition_iter_init(DecompositionIter_t *iter, const char *str,
                             size_t len, DecompositionType kind);
uint32_t decomposition_iter_next(DecompositionIter_t *iter);
void decomposition_iter_destroy(DecompositionIter_t *iter);
void decomposition_iter_reset(DecompositionIter_t *iter);
void decomposition_iter_decompose(DecompositionIter_t *decompiter, uint32_t c);

void recomp_destroy(RecompositionIter_t *recomp);
void recomp_init(RecompositionIter_t *recomp, const char *str,
								 size_t len, DecompositionType kind);
uint32_t recomp_next(RecompositionIter_t *recomp);


#ifdef __cplusplus
}
#endif