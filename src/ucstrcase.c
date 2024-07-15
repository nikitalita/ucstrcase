#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "ucstrcase.h"
#include "decode.h"
#include "lookup.h"
// UnicodeVersion is the Unicode version from which the tables in this file are
// derived. const char * utf8_UnicodeVersion = "15.0.0";
// TODO: make these static
#define utf8RuneSelf 0x80
#define utf8MaxRune 0x10FFFF
#define RuneError 0xFFFD

int IndexByteNonASCII(const unsigned char *b, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (b[i] & utf8RuneSelf) {
      return (int)i;
    }
  }
  return -1;
}

int IndexNonASCII(const char *s) {
  for (size_t i = 0; s[i] != '\0'; i++) {
    if ((unsigned char)s[i] & utf8RuneSelf) {
      return (int)i;
    }
  }
  return -1;
}

int isAlpha(unsigned char c) {
  return (c | 20) >= 'a' && (c | 20) <= 'z';
}

int Count(const unsigned char *b, size_t len, unsigned char c) {
  int count = 0;
  if (isAlpha(c)) {
    for (size_t i = 0; i < len; i++) {
      if (b[i] == c) {
        count++;
      }
    }
    return count;
  }
  for (size_t i = 0; i < len; i++) {
    if (b[i] == c || b[i] == (c ^ ' ')) {
      count++;
    }
  }
  return count;
}

int CountString(const char *s, unsigned char c) {
  size_t len = strlen(s);
  return Count((const unsigned char *)s, len, c);
}

int IndexByte(const unsigned char *s, size_t len, unsigned char c) {
  if (!isAlpha(c)) {
    for (size_t i = 0; i < len; i++) {
      if (s[i] == c) {
        return (int)i;
      }
    }
    return -1;
  }
  c |= ' ';
  for (size_t i = 0; i < len; i++) {
    if ((s[i] | ' ') == c) {
      return (int)i;
    }
  }
  return -1;
}

int IndexByteString(const char *s, unsigned char c) {
  return IndexByte((const unsigned char *)s, strlen(s), c);
}

int clamp(int n);
int ucstrcasecmp(const char *s, const char *t);
bool EqualFold(const char *s, const char *t);


int clamp(int n) {
  if (n < 0) {
    return -1;
  }
  if (n > 0) {
    return 1;
  }
  return 0;
}

// ASCII lowercase map
static const unsigned char _lower[128] = {
    0,   1,   2,    3,   4,   5,   6,   7,   8,   9,    10,  11,  12,  13,  14,
    15,  16,  17,   18,  19,  20,  21,  22,  23,  24,   25,  26,  27,  28,  29,
    30,  31,  ' ',  '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',',
    '-', '.', '/',  '0', '1', '2', '3', '4', '5', '6',  '7', '8', '9', ':', ';',
    '<', '=', '>',  '?', '@', 'a', 'b', 'c', 'd', 'e',  'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm',  'n', 'o', 'p', 'q', 'r', 's', 't',  'u', 'v', 'w', 'x', 'y',
    'z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c',  'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k',  'l', 'm', 'n', 'o', 'p', 'q', 'r',  's', 't', 'u', 'v', 'w',
    'x', 'y', 'z',  '{', '|', '}', '~', 127,
};

int ucstrcasecmp(const char *s, const char *t) {
  size_t slen = strlen(s);
  size_t tlen = strlen(t);
  return ucstrncasecmp(s, t, slen > tlen ? slen : tlen);
}


#define branchlessMIN(a, b) (b ^ ((a ^ b) & -(a < b)))

/* 
 * Decode the next character from BUF, returning a RuneResult structure.
 * The size field will be set to the number of bytes read from the buffer.
 * The rune field will be set to the decoded Unicode code point.
 * If the input is invalid, the rune field will be set to RuneError.
 *
 * Since this is a branchless decoder, four bytes will be read from the
 * buffer regardless of the actual length of the next character. This
 * means the buffer _must_ have at least three bytes of zero padding
 * following the end of the data stream.
 */
RuneResult
_unsafe_utf8_decode(const char *buf, size_t _slen)
{
    static const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const uint32_t mins[] = {4194304, 0, 128, 2048, 65536};
    static const int shiftc[] = {0, 18, 12, 6, 0};
    static const int shifte[] = {0, 6, 4, 2, 0};

    const uint8_t *s = (const uint8_t *)buf;
    int len = lengths[s[0] >> 3];
    RuneResult result = (RuneResult){0, 0};
    result.size = len + !len;


    /* Assume a four-byte character and load four bytes. Unused bits are
     * shifted out.
     */
    result.rune  = (uint32_t)(s[0] & masks[len]) << 18;
    result.rune |= (uint32_t)(s[1] & 0x3f) << 12;
    result.rune |= (uint32_t)(s[2] & 0x3f) <<  6;
    result.rune |= (uint32_t)(s[3] & 0x3f) <<  0;
    result.rune >>= shiftc[len];
    
    /* Accumulate the various error conditions. */
    int e = (result.rune < mins[len]) << 6; // non-canonical encoding
    e |= ((result.rune >> 11) == 0x1b) << 7;  // surrogate half?
    e |= (result.rune > 0x10FFFF) << 8;  // out of range?
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]       ) >> 6;
    e ^= 0x2a; // top two bits of each tail byte correct?
    e >>= shifte[len];
    e = (e != 0);
    int mask = ((e != 0)) << 31 >> 31;
    result.rune = (result.rune & ~mask) | (RuneError & mask);
    return result;
}


// #define _UCSTRCASE_BAIL_IF_INVALID 1

int ucstrncasecmp(const char *s, const char *t, size_t len) {
  int i = 0;
  int s_len = strnlen(s, len);
  int t_len = strnlen(t, len);
  if (s == t) {
    return 0;
  }

  for (; i < s_len && i < t_len; i++) {
    unsigned char sr = s[i];
    unsigned char tr = t[i];
    if ((sr | tr) &
        0x80) { // Check if either character is a non-ASCII character
      goto hasUnicode;
    }
    if (sr == tr || _lower[sr & 0x7F] == _lower[tr & 0x7F]) {
      continue;
    }
    if (_lower[sr & 0x7F] < _lower[tr & 0x7F]) {
      return -1;
    }
    return 1;
  }
  return clamp(s_len - t_len);

hasUnicode:
  s += i;
  t += i;
  s_len -= i;
  t_len -= i;
  while (*s != '\0' && s_len > 4 && t_len > 4) {
    if (*t == '\0') {
      return 1;
    }

    uint8_t si, ti;
    uint32_t sr, tr;
    RuneResult s_result = _unsafe_utf8_decode(s, s_len);
    RuneResult t_result = _unsafe_utf8_decode(t, t_len);
    sr = caseFold(s_result.rune);
    s_len -= s_result.size;
    s += s_result.size;

    tr = caseFold(t_result.rune);
    t_len -= t_result.size;
    t += t_result.size;
#ifdef _UCSTRCASE_BAIL_IF_INVALID
    if (sr == RuneError || tr == RuneError) {
      return clamp((int)sr - (int)tr);
    }
#endif
    // no size checking here because unicode upper and lower case characters can
    // have different lengths
    if (sr == tr) {
      continue;
    }
    return clamp((int)sr - (int)tr);
  }
  // do the rest with safe utf8 decode
  // TODO: Don't copy all this code around
  while (*s != '\0' && s_len > 0 && t_len > 0) {
    if (*t == '\0') {
      return 1;
    }

    uint8_t si, ti;
    uint32_t sr, tr;
    RuneResult s_rune = DecodeRuneInString((void*)s, s_len);
    RuneResult t_rune = DecodeRuneInString((void*)t, t_len);
    sr = caseFold(s_rune.rune);
    s_len -= s_rune.size;
    s += s_rune.size;

    tr = caseFold(t_rune.rune);
    t_len -= t_rune.size;
    t += t_rune.size;
    if (sr == tr) {
      continue;
    }
    return clamp((int)sr - (int)tr);
  }
  if (*t == '\0') {
    return 0;
  }
  return clamp(s_len - t_len);
}

bool EqualFold(const char *s, const char *t) {
  return s == t || ucstrcasecmp(s, t) == 0;
}