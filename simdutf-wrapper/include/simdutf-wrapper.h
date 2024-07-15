#pragma once
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// // check if <uchar.h> is available via __has_include
#if defined(__cplusplus) && __cplusplus >= 201103L && defined(__has_include)
#if __has_include(<uchar.h>)
#include <uchar.h>
typedef char16_t simdutfc16_t;
typedef char32_t simdutfc32_t;
#else
#define _SIMDUTF_WARPPER_NO_UCHAR_ 1
#endif
#else
#define _SIMDUTF_WARPPER_NO_UCHAR_ 1
#endif
#ifdef _SIMDUTF_WARPPER_NO_UCHAR_
typedef uint16_t simdutfc16_t;
typedef uint32_t simdutfc32_t;
#undef _SIMDUTF_WARPPER_NO_UCHAR_
#endif

#ifdef __cplusplus
extern "C" {
#endif

// hand-coded bindings
typedef struct simdutfrs_result {
  uint32_t error;
  size_t count;
} simdutfrs_result_t;

void simdutf_change_endianness_utf16(const simdutfc16_t *src, size_t len,
                                     simdutfc16_t *dst);

uint32_t simdutf_autodetect_encoding(const char *src, size_t len);

uint32_t simdutf_detect_encodings(const char *src, size_t len);

// end hand-coded bindings

bool simdutf_validate_ascii(const char *buf, size_t len);

bool simdutf_validate_utf8(const char *buf, size_t len);

bool simdutf_validate_utf16(const simdutfc16_t *buf, size_t len);

bool simdutf_validate_utf16be(const simdutfc16_t *buf, size_t len);

bool simdutf_validate_utf16le(const simdutfc16_t *buf, size_t len);

bool simdutf_validate_utf32(const simdutfc32_t *buf, size_t len);

simdutfrs_result_t simdutf_validate_ascii_with_errors(const char *buf,
                                                      size_t len);

simdutfrs_result_t simdutf_validate_utf8_with_errors(const char *buf,
                                                     size_t len);

simdutfrs_result_t simdutf_validate_utf16_with_errors(const simdutfc16_t *buf,
                                                      size_t len);

simdutfrs_result_t simdutf_validate_utf16be_with_errors(const simdutfc16_t *buf,
                                                        size_t len);

simdutfrs_result_t simdutf_validate_utf16le_with_errors(const simdutfc16_t *buf,
                                                        size_t len);

simdutfrs_result_t simdutf_validate_utf32_with_errors(const simdutfc32_t *buf,
                                                      size_t len);

size_t simdutf_count_utf8(const char *buf, size_t len);

size_t simdutf_count_utf16(const simdutfc16_t *buf, size_t len);

size_t simdutf_count_utf16be(const simdutfc16_t *buf, size_t len);

size_t simdutf_count_utf16le(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf8_length_from_utf16(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf8_length_from_utf16be(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf8_length_from_utf16le(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf8_length_from_utf32(const simdutfc32_t *buf, size_t len);

size_t simdutf_utf16_length_from_utf8(const char *buf, size_t len);

size_t simdutf_utf16_length_from_utf32(const simdutfc32_t *buf, size_t len);

size_t simdutf_utf32_length_from_utf8(const char *buf, size_t len);

size_t simdutf_utf32_length_from_utf16(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf32_length_from_utf16be(const simdutfc16_t *buf, size_t len);

size_t simdutf_utf32_length_from_utf16le(const simdutfc16_t *buf, size_t len);

size_t simdutf_convert_utf8_to_utf16(const char *src, size_t len,
                                     simdutfc16_t *dst);

size_t simdutf_convert_utf8_to_utf16be(const char *src, size_t len,
                                       simdutfc16_t *dst);

size_t simdutf_convert_utf8_to_utf16le(const char *src, size_t len,
                                       simdutfc16_t *dst);

size_t simdutf_convert_utf8_to_utf32(const char *src, size_t len,
                                     simdutfc32_t *dst);

size_t simdutf_convert_utf16_to_utf8(const simdutfc16_t *src, size_t len,
                                     char *dst);

size_t simdutf_convert_utf16_to_utf32(const simdutfc16_t *src, size_t len,
                                      simdutfc32_t *dst);

size_t simdutf_convert_utf16be_to_utf8(const simdutfc16_t *src, size_t len,
                                       char *dst);

size_t simdutf_convert_utf16be_to_utf32(const simdutfc16_t *src, size_t len,
                                        simdutfc32_t *dst);

size_t simdutf_convert_utf16le_to_utf8(const simdutfc16_t *src, size_t len,
                                       char *dst);

size_t simdutf_convert_utf16le_to_utf32(const simdutfc16_t *src, size_t len,
                                        simdutfc32_t *dst);

size_t simdutf_convert_utf32_to_utf8(const simdutfc32_t *src, size_t len,
                                     char *dst);

size_t simdutf_convert_utf32_to_utf16(const simdutfc32_t *src, size_t len,
                                      simdutfc16_t *dst);

size_t simdutf_convert_utf32_to_utf16be(const simdutfc32_t *src, size_t len,
                                        simdutfc16_t *dst);

size_t simdutf_convert_utf32_to_utf16le(const simdutfc32_t *src, size_t len,
                                        simdutfc16_t *dst);

simdutfrs_result_t simdutf_convert_utf8_to_utf16_with_errors(const char *src,
                                                             simdutfc16_t *dst);

simdutfrs_result_t
simdutf_convert_utf8_to_utf16be_with_errors(const char *src, simdutfc16_t *dst);

simdutfrs_result_t
simdutf_convert_utf8_to_utf16le_with_errors(const char *src, simdutfc16_t *dst);

simdutfrs_result_t simdutf_convert_utf8_to_utf32_with_errors(const char *src,
                                                             simdutfc32_t *dst);

simdutfrs_result_t
simdutf_convert_utf16_to_utf8_with_errors(const simdutfc16_t *src, size_t len,
                                          char *dst);

simdutfrs_result_t
simdutf_convert_utf16_to_utf32_with_errors(const simdutfc16_t *src, size_t len,
                                           simdutfc32_t *dst);

simdutfrs_result_t
simdutf_convert_utf16be_to_utf8_with_errors(const simdutfc16_t *src, size_t len,
                                            char *dst);

simdutfrs_result_t
simdutf_convert_utf16be_to_utf32_with_errors(const simdutfc16_t *src,
                                             size_t len, simdutfc32_t *dst);

simdutfrs_result_t
simdutf_convert_utf16le_to_utf8_with_errors(const simdutfc16_t *src, size_t len,
                                            char *dst);

simdutfrs_result_t
simdutf_convert_utf16le_to_utf32_with_errors(const simdutfc16_t *src,
                                             size_t len, simdutfc32_t *dst);

simdutfrs_result_t
simdutf_convert_utf32_to_utf8_with_errors(const simdutfc32_t *src, size_t len,
                                          char *dst);

simdutfrs_result_t
simdutf_convert_utf32_to_utf16_with_errors(const simdutfc32_t *src, size_t len,
                                           simdutfc16_t *dst);

simdutfrs_result_t
simdutf_convert_utf32_to_utf16be_with_errors(const simdutfc32_t *src, size_t len,
                                             simdutfc16_t *dst);

simdutfrs_result_t
simdutf_convert_utf32_to_utf16le_with_errors(const simdutfc32_t *src, size_t len,
                                             simdutfc16_t *dst);

size_t simdutf_convert_valid_utf8_to_utf16(const char *src, size_t len,
                                           simdutfc16_t *dst);

size_t simdutf_convert_valid_utf8_to_utf16be(const char *src, size_t len,
                                             simdutfc16_t *dst);

size_t simdutf_convert_valid_utf8_to_utf16le(const char *src, size_t len,
                                             simdutfc16_t *dst);

size_t simdutf_convert_valid_utf8_to_utf32(const char *src, size_t len,
                                           simdutfc32_t *dst);

size_t simdutf_convert_valid_utf16_to_utf8(const simdutfc16_t *src, size_t len,
                                           char *dst);

size_t simdutf_convert_valid_utf16_to_utf32(const simdutfc16_t *src, size_t len,
                                            simdutfc32_t *dst);

size_t simdutf_convert_valid_utf16be_to_utf8(const simdutfc16_t *src,
                                             size_t len, char *dst);

size_t simdutf_convert_valid_utf16be_to_utf32(const simdutfc16_t *src,
                                              size_t len, simdutfc32_t *dst);

size_t simdutf_convert_valid_utf16le_to_utf8(const simdutfc16_t *src,
                                             size_t len, char *dst);

size_t simdutf_convert_valid_utf16le_to_utf32(const simdutfc16_t *src,
                                              size_t len, simdutfc32_t *dst);

size_t simdutf_convert_valid_utf32_to_utf8(const simdutfc32_t *src, size_t len,
                                           char *dst);

size_t simdutf_convert_valid_utf32_to_utf16(const simdutfc32_t *src, size_t len,
                                            simdutfc16_t *dst);

size_t simdutf_convert_valid_utf32_to_utf16be(const simdutfc32_t *src, size_t len,
                                              simdutfc16_t *dst);

size_t simdutf_convert_valid_utf32_to_utf16le(const simdutfc32_t *src, size_t len,
                                              simdutfc16_t *dst);

#ifdef __cplusplus
}
#endif