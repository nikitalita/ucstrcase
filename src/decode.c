#include "decode.h"

#define utf8RuneSelf 0x80
#define RuneError 0xFFFD
#define mask2 0x1F
#define mask3 0x0F
#define mask4 0x07
#define maskx 0x3F
#define locb 0x80
#define hicb 0xBF
#define UTFMax 4
#define u_xx 0xF1 // invalid: size 1
#define u_as 0xF0 // ASCII: size 1
#define u_s1 0x02 // accept 0, size 2
#define u_s2 0x13 // accept 1, size 3
#define u_s3 0x03 // accept 0, size 3
#define u_s4 0x23 // accept 2, size 3
#define u_s5 0x34 // accept 3, size 4
#define u_s6 0x04 // accept 0, size 4
#define u_s7 0x44 // accept 4, size 4

#define rune1Max ((1 << 7) - 1)
#define rune2Max ((1 << 11) - 1)
#define rune3Max ((1 << 16) - 1)
#define surrogateMin 0xD800
#define surrogateMax 0xDFFF
#define MaxRune 0x10FFFF

// clang-format off

// Static lookup table for the first byte that follows the initial byte of a UTF-8 sequence.
static const uint8_t first[256] = {
	//   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x00-0x0F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x10-0x1F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x20-0x2F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x30-0x3F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x40-0x4F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x50-0x5F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x60-0x6F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as, // 0x70-0x7F
	//   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx, // 0x80-0x8F
	u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx, // 0x90-0x9F
	u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx, // 0xA0-0xAF
	u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx, // 0xB0-0xBF
	u_xx,u_xx,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1, // 0xC0-0xCF
	u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1,u_s1, // 0xD0-0xDF
	u_s2,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s3,u_s4,u_s3,u_s3, // 0xE0-0xEF
	u_s5,u_s6,u_s6,u_s6,u_s7,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx,u_xx, // 0xF0-0xFF
};
// clang-format on

typedef struct {
  unsigned char lo; // lowest value for second byte.
  unsigned char hi; // highest value for second byte.
} acceptRange;

// Initialize acceptRanges based on the Go code snippet provided.
acceptRange acceptRanges[16] = {
    {locb, hicb}, // 0 (ASCII)
    {0xA0, hicb}, // 1
    {locb, 0x9F}, // 2
    {0x90, hicb}, // 3
    {locb, 0x8F}, // 4
    {0, 0},       {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {0, 0},       {0, 0}, {0, 0}, {0, 0}, {0, 0},
};

bool is_surrogate(uint16_t uc);
bool is_high_surrogate(uint16_t uc);
bool is_low_surrogate(uint16_t uc);
uint32_t surrogate_to_utf32(uint16_t high, uint16_t low);



RuneResult DecodeRuneInString(const char *s, size_t length) {
  RuneResult result;
  int n = length;
  if (n < 1) {
    result.rune = RuneError;
    result.size = 0;
    return result;
  }
  unsigned char s0 = s[0];
  int x = first[s0];
  if (x >= u_as) {
    // The following code simulates an additional check for x == xx and
    // handling the ASCII and invalid cases accordingly. This mask-and-or
    // approach prevents an additional branch.
    int mask = (int)(x) << 31 >> 31; // Create 0x0000 or 0xFFFF.
    result.rune = (s[0] & ~mask) | (RuneError & mask);
    result.size = 1;
    return result;
  }
  int sz = x & 7;
  acceptRange accept = acceptRanges[x >> 4];
  if (n < sz) {
    result.rune = RuneError;
    result.size = 1;
    return result;
  }
  unsigned char s1 = s[1];
  if (s1 < accept.lo || accept.hi < s1) {
    result.rune = RuneError;
    result.size = 1;
    return result;
  }
  if (sz == 2) {
    result.rune = ((s0 & mask2) << 6) | (s1 & maskx);
    result.size = 2;
    return result;
  }
  unsigned char s2 = s[2];
  if (s2 < locb || hicb < s2) {
    result.rune = RuneError;
    result.size = 1;
    return result;
  }
  if (sz == 3) {
    result.rune = ((s0 & mask3) << 12) | ((s1 & maskx) << 6) | (s2 & maskx);
    result.size = 3;
    return result;
  }
  unsigned char s3 = s[3];
  if (s3 < locb || hicb < s3) {
    result.rune = RuneError;
    result.size = 1;
    return result;
  }
  result.rune = ((s0 & mask4) << 18) | ((s1 & maskx) << 12) |
                ((s2 & maskx) << 6) | (s3 & maskx);
  result.size = 4;
  return result;
}

// ValidRune reports whether r can be legally encoded as UTF-8.
// Code points that are out of range or a surrogate half are illegal.
bool ValidRune(uint32_t r) {
  if ((0 <= r && r < surrogateMin) || (surrogateMax < r && r <= MaxRune)) {
    return true;
  }
  return false;
}
bool RuneStart(unsigned char b) { return (b & 0xC0) != 0x80; }

RuneResult DecodeLastRuneInString(const char *s, size_t length) {
  RuneResult result;
  int end = length;
  if (end == 0) {
    result.rune = RuneError;
    result.size = 0;
    return result;
  }
  int start = end - 1;
  unsigned char r = s[start];
  if (r < utf8RuneSelf) {
    result.rune = r;
    result.size = 1;
    return result;
  }
  int lim = end - UTFMax;
  if (lim < 0) {
    lim = 0;
  }
  for (start--; start >= lim; start--) {
    if (RuneStart(s[start])) { // Assuming RuneStart checks if the byte is a
                               // start of a UTF-8 sequence
      break;
    }
  }
  if (start < 0) {
    start = 0;
  }
  // Assuming DecodeRuneInString function is defined to decode a rune from a
  // substring
  result = DecodeRuneInString(&s[start], length);
  if (start + result.size != end) {
    result.rune = RuneError;
    result.size = 1;
  }
  return result;
}

int RuneLen(unsigned int r) {
  if (r < 0) {
    return -1;
  } else if (r <= rune1Max) {
    return 1;
  } else if (r <= rune2Max) {
    return 2;
  } else if (surrogateMin <= r && r <= surrogateMax) {
    return -1;
  } else if (r <= rune3Max) {
    return 3;
  } else if (r <= MaxRune) {
    return 4;
  }
  return -1;
}



inline size_t char_utf32_to_utf8(uint32_t utf32, const char *buffer)
// Encodes the UTF-32 encoded char into a UTF-8 string.
// Stores the result in the buffer and returns the position
// of the end of the buffer
// (unchecked access, be sure to provide a buffer that is big enough)
{
	char *end = (char *)(buffer);
	if (utf32 < 0x7F){ *(end++) = (char)(utf32); return 1;}
	else if (utf32 < 0x7FF) {
		*(end++) = 0b11000000 + (char)(utf32 >> 6);
		*(end++) = 0b10000000 + (char)(utf32 & 0b00111111);
		return 2;
	} else if (utf32 < 0x10000) {
		*(end++) = 0b11100000 + (char)(utf32 >> 12);
		*(end++) = 0b10000000 + (char)((utf32 >> 6) & 0b00111111);
		*(end++) = 0b10000000 + (char)(utf32 & 0b00111111);
		return 3;
	} else if (utf32 < 0x110000) {
		*(end++) = 0b11110000 + (char)(utf32 >> 18);
		*(end++) = 0b10000000 + (char)((utf32 >> 12) & 0b00111111);
		*(end++) = 0b10000000 + (char)((utf32 >> 6) & 0b00111111);
		*(end++) = 0b10000000 + (char)(utf32 & 0b00111111);
		return 4;
	}
	return 0;
//	*end = '\0';
}

inline bool is_surrogate(uint16_t uc) { return (uc - 0xd800u) < 2048u; }
inline bool is_high_surrogate(uint16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
inline bool is_low_surrogate(uint16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

inline uint32_t surrogate_to_utf32(uint16_t high, uint16_t low) {
	return (high << 10) + low - 0x35fdc00;
}

inline RuneResult DecodeRuneInUTF16String(const uint16_t *input,
																					size_t input_size)
{
	const uint16_t uc = *input;
	if (!is_surrogate(uc)) {
		return (RuneResult){uc, 1};
	} else {
		if (is_high_surrogate(uc) && input_size > 1 && is_low_surrogate(*input+1)){
			return (RuneResult){surrogate_to_utf32(uc, *input+1), 2};
		}
	}
	return (RuneResult){RuneError, 2};
}