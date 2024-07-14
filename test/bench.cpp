#include "normtables.h"


#include <unicode/stringoptions.h>
#include <stdio.h>
#include <string.h>
#include <ucstrcase.h>
#include <string>
#include <iostream>
#include <vector>
#include <cuchar>
#define U_SHOW_CPLUSPLUS_API 1
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include "thirdparty/utf8.h"
#include <thirdparty/utf8proc.h>
#include <fmt/format.h>
#include "data.h"
#define utf8_probability 3 // 3-percent chance of generating a UTF-8 string
#define utf8_char_probability 5 // 2-percent chance of generating a UTF-8 character
#define utf8_invalid_probability 20 // 1-percent chance of generating an invalid UTF-8 character
#define TABLE_SIZE 1000000
#define TABLE_TRAVERSALS 1
#define STRING_SIZE 200
#define locb 0x80
#define hicb 0xBF

#include <chrono>
int new_normalizing_compare(const char *s, const char *t);
int new_normalizing_compare_n(const char *s, const char *t, size_t len);
int64_t getCurrentNanos()
{
    using std::chrono::system_clock;
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Convert duration to milliseconds
    auto milliseconds
        = std::chrono::duration_cast<std::chrono::nanoseconds>(
              duration)
              .count();
    
    return milliseconds;
}

char* char_utf32_to_utf8(char32_t utf32, const char* buffer)
// Encodes the UTF-32 encoded char into a UTF-8 string.
// Stores the result in the buffer and returns the position
// of the end of the buffer
// (unchecked access, be sure to provide a buffer that is big enough)
{
	char* end = const_cast<char*>(buffer);
	if(utf32 < 0x7F) *(end++) = static_cast<unsigned>(utf32);
	else if(utf32 < 0x7FF) {
		*(end++) = 0b1100'0000 + static_cast<unsigned>(utf32 >> 6);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	}
	else if(utf32 < 0x10000){
		*(end++) = 0b1110'0000 + static_cast<unsigned>(utf32 >> 12);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	} else if(utf32 < 0x110000) {
		*(end++) = 0b1111'0000 + static_cast<unsigned>(utf32 >> 18);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 12) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	}
	else throw std::runtime_error("Invalid UTF-32 character");
	*end = '\0';
	return end;
}


typedef struct {
    unsigned char lo; // lowest value for second byte.
    unsigned char hi; // highest value for second byte.
} acceptRange;

#define u_xx 0xF1 // invalid: size 1
#define u_as 0xF0 // ASCII: size 1
#define u_s1 0x02 // accept 0, size 2
#define u_s2 0x13 // accept 1, size 3
#define u_s3 0x03 // accept 0, size 3
#define u_s4 0x23 // accept 2, size 3
#define u_s5 0x34 // accept 3, size 4
#define u_s6 0x04 // accept 0, size 4
#define u_s7 0x44 // accept 4, size 4


acceptRange bacceptRanges[16]{
    {locb, hicb}, // 0 (ASCII)
    {0xA0, hicb}, // 1
    {locb, 0x9F}, // 2
    {0x90, hicb}, // 3
    {locb, 0x8F}, // 4
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},

};

static const uint8_t first[256] = {
	//   1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x00-0x0F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x10-0x1F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x20-0x2F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x30-0x3F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x40-0x4F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x50-0x5F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x60-0x6F
	u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,u_as,// 0x70-0x7F
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
acceptRange getAcceptRange(uint8_t firstByte) {
    return bacceptRanges[first[firstByte] >> 4];
}

uint8_t uchar32_get_len(uint32_t c) {
	if (c < 0x80) {
		return 1;
	} else if (c < 0x800) {
		return 2;
	} else if (c < 0x10000) {
		return 3;
	} else {
		return 4;
	}
}
char get_random_char(uint8_t low, uint8_t high);
inline
char get_random_char(uint8_t low, uint8_t high){
	return (char)((rand() % (high - low)) + low);
}

#define branchlessMIN(a, b) (b ^ ((a ^ b) & -(a < b)))
char* generate_random_utf8_string(int length) {
    if (length <= 0) return NULL;

    // Allocate memory for UTF-8 string including null terminator
    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (!str) return NULL;
    
    for (int i = 0; i < length; ++i) {
        int rnd = rand() % 100; // Determine the length of the next character (1 to 4 bytes)
        if (rnd < 100 - utf8_char_probability || i + 1 >= length) { // 1-byte character
					// get a random printable ascii character
            str[i] = get_random_char(0x20, 0x7E); // 0x20-0x7E
        } else{
          // now randomly generate a 2, 3, or 4 byte character
          rnd = branchlessMIN(rand() % 3 + 2, length - (i+1));

          uint8_t firstByteHi = 0;
          uint8_t firstByteLo = 0;
					int idx = i;
					if (rnd == 2) { // 2-byte character
              firstByteHi = 0xC2;
              firstByteLo = 0xDF;
          } else if (rnd == 3) { // 3-byte character
              firstByteHi = 0xE0;
              firstByteLo = 0xEF;
          } else { // 4-byte character
              firstByteHi = 0xF0;
              firstByteLo = 0xF4;
          }
					char firstByte = get_random_char(firstByteHi, firstByteLo);
					auto range = getAcceptRange(firstByte);
          while (range.hi == 0) {
              firstByte = get_random_char(firstByteHi, firstByteLo);
							range = getAcceptRange(firstByte);
          }

          str[idx++] = firstByte;
					auto secondByte = get_random_char(range.lo, range.hi);
					// dotted I, remove it
          if (rnd == 2 && firstByte == (char)0xC4 && secondByte == (char)0xB0){
						secondByte = (char)0xB1;
					}
					str[idx++] = secondByte; // 0x80-0xBF
          if (rnd > 2){
              str[idx++] = get_random_char(locb, hicb); // 0x80-0xBF
							if (rnd > 3){
								str[idx++] = get_random_char(locb, hicb); // 0x80-0xBF
							}
					}

					if (idx < i + rnd){
						printf("SOMETHIUNG FUCKY HAPPENED!!!!!");
					}
					// decode it to make sure it's valid
					auto result = DecodeRuneInString(str + i, rnd);
					if (result.rune == 0xFFFD || result.size != rnd){
						// replace all the bytes with a random ascii character
						for (int k = 0; k < rnd; k++){
							str[i+k] = get_random_char(0x20, 0x7E);
						}
					}
          i += rnd - 1;
        }
    }

    str[length] = '\0'; // Null-terminate the string
    return str;
}
char* generate_random_invalid_utf8_string(int length) {
    if (length <= 0) return NULL;

    // Allocate memory for UTF-8 string including null terminator
    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (!str) return NULL;
    for (int i = 0; i < length; ++i) {
        int rnd = rand() % 100; // Determine the length of the next character (1 to 4 bytes)
        
        if (rnd < 100 - utf8_invalid_probability || i + 3  >= length) { // 1-byte character
            str[i] = (char)((rand() % 0x7F) + 1); // 0x01-0x7F
        } else{
          // now randomly generate a 1, 2, 3, or 4 byte character
          rnd = rand() % 4;
          if (rnd == 0) { // 1-byte character
              str[i] = (char)((rand() % 0x7F) + 0x80); // 0x80-0xBF
          } else if (rnd == 1) { // 2-byte character
              if (i + 1 >= length) break; // Ensure there's room for 2 bytes
              str[i++] = (char)((rand() % (0x7F - 0xC2)) + 0x80); // 0x80-0xBF
              str[i] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
          } else if (rnd == 2) { // 3-byte character
              if (i + 2 >= length) break; // Ensure there's room for 3 bytes
              str[i++] = (char)((rand() % (0x7F - 0xE0)) + 0x80); // 0x80-0xBF
              str[i++] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
              str[i] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
          } else { // 4-byte character
              if (i + 3 >= length) break; // Ensure there's room for 4 bytes
              str[i++] = (char)((rand() % (0x7F - 0xF0)) + 0x80); // 0x80-0xBF
              str[i++] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
              str[i++] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
              str[i] = (char)((rand() % (0xBF - 0x80)) + 0x80); // 0x80-0xBF
          }
      }
    }
    str[length] = '\0'; // Null-terminate the string
    return str;
}


char* generate_random_ascii_string(int length) {
    if (length <= 0) return NULL;

    // Allocate memory for ASCII string including null terminator
    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (!str) return NULL;

    for (int i = 0; i < length; ++i) {
        str[i] = (char)((rand() % 0x5E) + 0x20); // 0x20-0x7E
    }

    str[length] = '\0'; // Null-terminate the string
    return str;
}

char* generate_random_utf8_string_weighted(int length){
    int rnd = rand() % 100; // Determine if the string will be ASCII or UTF-8

    if (rnd < utf8_probability) { // Generate a UTF-8 string
      return generate_random_utf8_string(length);
    } else { // Generate an ASCII string
      return generate_random_ascii_string(length);
    }
}

char* generate_random_garbage(int length) {
  char* str = (char*)malloc((length + 1) * sizeof(char));
  // no null terminator!
  for (int i = 0; i < length+1; i++) {
    str[i] = (rand() % 255)+1;
  }
  return str;
}

char *tolower_normalize(const char *str) 
{
	utf8proc_uint8_t *retval;
	auto result = utf8proc_map((const uint8_t*)str, 0, &retval, (utf8proc_option_t)(UTF8PROC_NULLTERM | UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD));
  if (result < 0){
    return NULL;
  }
	return (char*)retval;
}


int clamp(int n) {
	if (n < 0) {
		return -1;
	}
	if (n > 0) {
		return 1;
	}
	return 0;
}
#define BUF_MAX_SIZE 100000
//utf8proc_int32_t a_buf[BUF_MAX_SIZE];
//utf8proc_int32_t b_buf[BUF_MAX_SIZE];
int test_norm_match(const char * a, const char * b){
	auto a_size = strlen(a);
	auto b_size = strlen(b);
	auto max_size = a_size > b_size ? a_size : b_size;
	const size_t buf_max_size = max_size * 4 * 4;
 utf8proc_int32_t *a_buf = (utf8proc_int32_t*)malloc(a_size * 4 *4);
 utf8proc_int32_t *b_buf = (utf8proc_int32_t*)malloc(b_size * 4 *4);
	static const auto options = UTF8PROC_NULLTERM | UTF8PROC_STABLE |
															UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT | UTF8PROC_CASEFOLD;
	auto a_act_size = utf8proc_decompose_custom((utf8proc_uint8_t*)a, a_size, a_buf, buf_max_size, (utf8proc_option_t)options, nullptr, nullptr);
	auto b_act_size = utf8proc_decompose_custom((utf8proc_uint8_t*)b, b_size, b_buf, buf_max_size, (utf8proc_option_t)options, nullptr, nullptr);
	a_act_size = utf8proc_normalize_utf32(a_buf, a_act_size, (utf8proc_option_t)options);
	b_act_size = utf8proc_normalize_utf32(b_buf, b_act_size, (utf8proc_option_t)options);
	int match = 0;
	for (int i = 0; i < a_act_size; i++){
		if (a_buf[i] != b_buf[i]){
			match = clamp(a_buf[i] - b_buf[i]);
			break;
		}
	}
	match = match ? match : clamp(a_act_size - b_act_size);
	// free(a_buf);
	// free(b_buf);
	return match;
}

int utf8proc_casecmp(const char *a, const char *b){
  auto a_lower = tolower_normalize(a);
  auto b_lower = tolower_normalize(b);
  if (a_lower == NULL && b_lower == NULL){
    return 0;
  }
  if (a_lower == NULL || b_lower == NULL){
    return 1 ? a_lower == NULL : b_lower == NULL;
  }
  auto result = strcmp(a_lower, b_lower);
  if (a_lower)
  free(a_lower);
  if (b_lower)
  free(b_lower);
  return result;
}

int utf8proc_casecmp_n(const char *a, const char *b, size_t len){
  auto a_lower = tolower_normalize(a);
  auto b_lower = tolower_normalize(b);
  if (a_lower == NULL && b_lower == NULL){
    return 0;
  }
  if (a_lower == NULL || b_lower == NULL){
    return 1 ? a_lower == NULL : b_lower == NULL;
  }

  auto result = strncmp(a_lower, b_lower, len);
  if (a_lower)
  free(a_lower);
  if (b_lower)
  free(b_lower);
  return result;
}

enum testType{
	UCSTRCASECMP = 0,
	UCSTRCASECMP_N,
	STRCASECMP,
  STRNCASECMP,
  ICU_CASE_CMP,
  ICU_CASE_CMP_N,
  UTF8CASECMP,
  UTF8NCASECMP,
  UTF8PROC_CASECMP,
  UTF8PROC_CASECMP_N,
  NEW_NORMALIZING_COMPARE,
  NEW_NORMALIZING_COMPARE_N,
  MAX
};

const std::string funcNames[testType::MAX]= {
	{"ucstrcasecmp"},
	{"ucstrncasecmp"},
	{"strcasecmp"},
  {"strncasecmp"},
  {"icu::UnicodeString::caseCompare"},
  {"icu::UnicodeString::caseCompareBetween"},
  {"utf8casecmp"},
  {"utf8ncasecmp"},
  {"utf8proc_casecmp"},
  {"utf8proc_casecmp_n"},
  {"new_normalizing_compare"},
  {"new_normalizing_compare_n"},
};

constexpr const char composed_upper_c_cedilla_num[] = {(char)0xC3, (char)0x87, 0x00};
constexpr const char decomposed_upper_c_cedilla_num[] = {0x43, (char)0xCC, (char)0xA7, 0};
constexpr const char* composed_upper_c_cedilla = composed_upper_c_cedilla_num;
constexpr const char* decomposed_upper_c_cedilla = decomposed_upper_c_cedilla_num;
constexpr const char composed_lower_c_cedilla_num[] = {(char)0xC3, (char)0xA7, 0x00};
constexpr const char decomposed_lower_c_cedilla_num[] = {0x63,(char) 0xCC, (char)0xA7, 0};
constexpr const char* composed_lower_c_cedilla = composed_lower_c_cedilla_num;
constexpr const char* decomposed_lower_c_cedilla = decomposed_lower_c_cedilla_num;

int run_test(testType type, const char* a, const char* b,  int64_t len = -1, bool zero_is_matching = true) {
  int result = 0;
  switch (type) {
    case STRCASECMP:
      result = strcasecmp(a, b);
      break;
    case STRNCASECMP:
      result = strncasecmp(a, b, len);
      break;
    case ICU_CASE_CMP:
      {
        icu::UnicodeString aicustr(a);
        result = aicustr.caseCompare(b, U_FOLD_CASE_DEFAULT);
      }
      break;
    case ICU_CASE_CMP_N:
      {
        icu::UnicodeString aicustr = len > 0 ? icu::UnicodeString(a, len) : icu::UnicodeString(a);
        icu::UnicodeString bicustr = len > 0 ? icu::UnicodeString(b, len) : icu::UnicodeString(b);
        result = aicustr.caseCompareBetween(0, aicustr.length(), b, 0, bicustr.length(), 0);
      }
      break;
    case UCSTRCASECMP:
      result = ucstrcasecmp(a, b);
      break;
    case UCSTRCASECMP_N:
      result = ucstrncasecmp(a, b, len);
      break;
    case UTF8CASECMP:
      result = utf8casecmp((utf8_int8_t*)a, (utf8_int8_t*)b);
      break;
    case UTF8NCASECMP:
      result = utf8ncasecmp((utf8_int8_t*)a, (utf8_int8_t*)b, len);
      break;
    case UTF8PROC_CASECMP:
      result = utf8proc_casecmp(a, b);
      break;
    case UTF8PROC_CASECMP_N:
      result = utf8proc_casecmp_n(a, b, len);
      break;
    case NEW_NORMALIZING_COMPARE:
      result = new_normalizing_compare(a, b);
      break;
    case NEW_NORMALIZING_COMPARE_N:
      result = new_normalizing_compare_n(a, b, len);
      break;

    default:
      throw std::runtime_error("Invalid test type");
  }
  return result;
}



constexpr bool isNType(testType type){
	return type % 2 == 1;
}


#define MAX_ERRORS 10
void run_tests(const char **s, bool i_plus_1_should_match = false, size_t table_len = TABLE_SIZE, size_t string_length = 0, bool no_check_match = false, std::vector<testType> filters = {}){
  int64_t stop, start;
  int64_t duration = 0;
  const size_t table_mod = table_len;
	const auto inc = i_plus_1_should_match ? 2 : 1;
	const size_t iters = table_len * TABLE_TRAVERSALS * inc;

	// if string_length == 0, do the non-ntypes (even indexes)
	if (filters.empty())
		filters = {UCSTRCASECMP, UCSTRCASECMP_N, NEW_NORMALIZING_COMPARE, NEW_NORMALIZING_COMPARE_N, STRCASECMP, STRNCASECMP, ICU_CASE_CMP, ICU_CASE_CMP_N, UTF8CASECMP, UTF8NCASECMP, };
  for (int tidx = 0; tidx < filters.size(); tidx++){
    auto type = (testType)filters[tidx];
    if (string_length != 0 && (!isNType(type))){
      continue;
    } else if (string_length == 0 && isNType(type)){
      continue;
    }
    int errors = 0;
    for (size_t i = 0; i < iters; i+=inc) {
      // worst case; string that matches
      const char* a = s[i%table_mod];
      int result = 0;
      // start = getCurrentNanos();
      // result = run_test((testType)type, a, a, string_length, i_plus_1_should_match);
      // duration += getCurrentNanos() - start;
      // if (!no_check_match && result != 0) {
      //   std::cout << "Error: " << funcNames[type] << ": no match for matching strings!!" << std::endl;
      //   errors++;
      // }
      const char* b = s[(i+1)%table_mod];
      start = getCurrentNanos();
      result = run_test((testType)type, a, b, string_length, i_plus_1_should_match);
      duration += getCurrentNanos() - start;
			if (!no_check_match) {
				if (!i_plus_1_should_match && result == 0) {
#ifndef NDEBUG
					std::cout << "Error: " << funcNames[type] << ": match for different strings!!" << std::endl;
#endif
					errors++;
				} else if (i_plus_1_should_match && result != 0) {
#ifndef NDEBUG
					std::cout << "Error: " << funcNames[type] << ": no match for different strings!!" << std::endl;
					// rerunning
					result = run_test((testType)type, a, b, string_length, i_plus_1_should_match);
#endif
					errors++;
				}
      }
      if (errors > MAX_ERRORS){
        std::cout << "Too many errors, stopping test" << std::endl;
        break;
      }
    }
    stop = getCurrentNanos();
    printf("%s iters %llu, failures %lld, time : %lldms\n",funcNames[type].c_str(), iters, errors, duration / 1000000);
  }
}

void test_icu(){
  const char * atest = "b";
  const char * btest = "B";
  // get the rune
  auto a_rune = DecodeRuneInString(atest, strlen(atest));
  auto b_rune = DecodeRuneInString(btest, strlen(btest));


  const char * test1 = composed_upper_c_cedilla;
  const char * test2 = decomposed_lower_c_cedilla;
  const char * cyrillic1 = "ƂⰩⅯƂⰨⅮƁⰧⅭɃⰦⅬSⰥⅫŽⰤⅪŽⰣⅩŻⰢⅨŻⰡⅧŹⰠⅦŹⰟⅥŸⰞⅤŶⰝⅣŶⰜⅢŴⰛⅡŴⰚⅠŲⰙⅯŲⰘⅮŰⰗⅭŰⰖⅬŮⰕⅫŮⰔⅪŬⰓⅩŬⰒⅨŪⰑⅧŪⰐⅦŨⰏⅥŨⰎⅤŦⰍⅣŦⰌⅢŤⰋⅡŤⰊⅠŢⰉ";
  // the above, but with a different case
  const char * cyrillic2 = "ƃⱙⅿƃⱘⅾɓⱗⅽƀⱖⅼſⱕⅻžⱔⅺžⱓⅹżⱒⅸżⱑⅷźⱐⅶźⱏⅵÿⱎⅴŷⱍⅳŷⱌⅲŵⱋⅱŵⱊⅰųⱉⅿųⱈⅾűⱇⅽűⱆⅼůⱅⅻůⱄⅺŭⱃⅹŭⱂⅸūⱁⅷūⱀⅶũⰿⅵũⰾⅴŧⰽⅳŧⰼⅲťⰻⅱťⰺⅰţⰹ";

  icu::UnicodeString aicustr(test1);
  icu::UnicodeString bicustr(test2);
  aicustr.toLower();
  bicustr.toLower();
  auto result = aicustr.caseCompare(bicustr, U_FOLD_CASE_DEFAULT);
  if (result != 0){
    std::cout << "Error: icu::UnicodeString: no match for matching strings: " << test1 << " and " << test2 << std::endl;
  } else {
    std::cout << "icu::UnicodeString: match for matching strings: " << test1 << " and " << test2 << std::endl;
  }

  // try utf8proc
  auto result2 = utf8proc_casecmp(test1, test2);
  if (result2 != 0){
    std::cout << "Error: utf8proc_casecmp: no match for matching strings: " << test1 << " and " << test2 << std::endl;
  } else{
    std::cout << "utf8proc_casecmp: match for matching strings: " << test1 << " and " << test2 << std::endl;
  }

  //try ucstrcase
  auto result3 = ucstrcasecmp(test1, test2);
  if (result3 != 0){
    std::cout << "Error: ucstrncasecmp: no match for matching strings: " << test1 << " and " << test2 << std::endl;
  } else {
    std::cout << "ucstrncasecmp: match for matching strings: " << test1 << " and " << test2 << std::endl;
  }
}
void generateRandomMatchingStrings3(const char **s, int tableLen, int strLen){
  //
  for (int i = 0; i < tableLen; i+=2){
		if (i == 2260){
			printf("sssssss!");
		}
    char * a = generate_random_utf8_string(strLen);
    icu::UnicodeString aicustr(a, strLen);

    aicustr = aicustr.toLower();
    std::string a_str;
    icu::StringByteSink sink{&a_str};
    aicustr.toUTF8(sink);
    char * b = (char *)malloc(a_str.size() + 1);
    memcpy(b, a_str.c_str(), a_str.size() + 1);
    s[i] = a;
    s[i+1] = b;
  }
}

void generateRandomMatchingStrings(const char **s, int tableLen, int strLen){
  std::vector<foldPair> foldPairs;
  for (int i = 0; i < 8192; i ++){
    auto cf = getCaseFold(i);
    if (cf.From != 0){
      foldPairs.push_back(cf);
    }
  }

  // generate random matching strings
  if (tableLen % 2){
    throw std::runtime_error("tableLen must be even");
  }
  for (int i = 0; i < tableLen; i+=2){
    icu::UnicodeString a = icu::UnicodeString();
    icu::UnicodeString b = icu::UnicodeString();
    for (int j = 0; j < strLen; j++){
      auto rnd = rand() % foldPairs.size();
      auto pair = foldPairs[rnd];
      a.append((UChar32)pair.From);
      b.append((UChar32)pair.To);
      auto a_last_char = a.char32At(a.length()-1);
      auto b_last_char = b.char32At(b.length()-1);
      if (a_last_char != pair.From || b_last_char != pair.To){
        std::cout << "Error: " << a_last_char << " != " << pair.From << " or " << b_last_char << " != " << pair.To << std::endl;
      }
    }
    std::string a_str;
    std::string b_str;
    icu::StringByteSink sink{&a_str};
    icu::StringByteSink sink2{&b_str};
    a.toUTF8(sink);
    b.toUTF8(sink2);
    auto a_cstr = new char[a_str.size() + 1];
    auto b_cstr = new char[b_str.size() + 1];
    memcpy(a_cstr, a_str.c_str(), a_str.size());
    memcpy(b_cstr, b_str.c_str(), b_str.size());
    a_cstr[a_str.size()] = '\0';
    b_cstr[b_str.size()] = '\0';
    s[i] = a_cstr;
    s[i+1] = b_cstr;
  }

}

void generateRandomMatchingStrings2(const char **s, int tableLen, int strLen){
  auto FOLDMAPSIZE = (uint16_t) 256;
  auto FOLDMAPITEMLEN = (uint16_t) 4;
  // generate random matching strings
  if (tableLen % 2){
    throw std::runtime_error("tableLen must be even");
  }
  for (int i = 0; i < tableLen; i+=2){
    icu::UnicodeString a = icu::UnicodeString();
    icu::UnicodeString b = icu::UnicodeString();
    for (int j = 0; j < strLen; j++){
      const uint16_t* pair = NULL;
      int idx = rand() % FOLDMAPSIZE;
      while(!pair || !pair[0] || !pair[1]) {
       idx = rand() % FOLDMAPSIZE;
       pair = getFoldMap(idx);
      }
      uint16_t upper = 0;
      uint16_t lower = 0;
      int i = 0;
      while(!upper || !lower){
        auto i = rand() % FOLDMAPITEMLEN;
        auto val = pair[i];
        if (!val)
          continue;
        if (u_isUUppercase(val)){
          upper = val;
        } else if (u_isULowercase(val)){
          lower = val;
        }
        if (upper && lower){
          break;
        }
      }
      a.append((UChar32)upper);
      b.append((UChar32)lower);
      if (a.char32At(a.length()-1) != upper || b.char32At(b.length()-1) != lower){
        std::cout << "Error: " << a.char32At(a.length()-1) << " != " << upper << " or " << b.char32At(b.length()-1) << " != " << lower << std::endl;
      }
    }
    std::string a_str;
    std::string b_str;
    icu::StringByteSink sink{&a_str};
    icu::StringByteSink sink2{&b_str};
    a.toUTF8(sink);
    b.toUTF8(sink2);
    auto a_cstr = new char[a_str.size() + 1];
    auto b_cstr = new char[b_str.size() + 1];
    memcpy(a_cstr, a_str.c_str(), a_str.size());
    memcpy(b_cstr, b_str.c_str(), b_str.size());
    a_cstr[a_str.size()] = '\0';
    b_cstr[b_str.size()] = '\0';
    s[i] = a_cstr;
    s[i+1] = b_cstr;
  }

}

static const char* s[TABLE_SIZE];

void free_all_strings(){
  for (int i = 0; i < TABLE_SIZE; i++){
    free((void*)s[i]);
  }
}


void test_thing(){
  const uint8_t test1[] = {'c', 0xCC, 0x81, 0xCC, 0xA7, 0};
  const uint8_t test2[] = {'c', 0xCC, 0xA7, 0xCC, 0x81, 0};
  const char* test1_ptr = (const char*)test1;
  const char* test2_ptr = (const char*)test2;
  if (test_norm_match((const char *)test1, (const char *)test2) != 0){
    std::cout << "Error: test_norm_match: no match for matching strings: " << test1 << " and " << test2 << std::endl;
  } else {
    std::cout << "SUCCESS: test_norm_match: match for matching strings: " << test1 << " and " << test2 << std::endl;
  }
}


void test_normalization(const char **s, size_t table_len = TABLE_SIZE){
  // get start time
  int64_t start = getCurrentNanos();
	int64_t duration = 0;
  int64_t failures = 0;
  int64_t successes = 0;
	int inc = 2;
  const int64_t iters = table_len * TABLE_TRAVERSALS * inc;
  for (int i = 0; i < iters; i+=inc){
    auto a = s[i % table_len];
    auto b = s[(i+1) % table_len];
		start = getCurrentNanos();
		int result = test_norm_match(a, b);
		duration += getCurrentNanos() - start;

    if (result != 0){
      failures++;
    } else {
      successes++;
    }
  }
  printf("utf8proc_NFKC iters %llu, failures %lld, time : %lldms\n",iters, failures, (duration)/ 1000000);
}


#include "compose.h"

void test_new_normal(){
  const uint8_t test1[] = {'c', 0xCC, 0x81, 0xCC, 0xA7, 0};
  const uint8_t test2[] = {'C', 0xCC, 0xA7, 0xCC, 0x81, 0};
  const char* test1_ptr = (const char*)test1;
  const char* test2_ptr = (const char*)test2;
  DecompositionIter_t iter1;
  DecompositionIter_t iter2;
  decomposition_iter_init(&iter1, (const char*)test1, sizeof(test1), DecompositionType::CompatibleCaseFold);
  decomposition_iter_init(&iter2, (const char*)test2, sizeof(test2), DecompositionType::CompatibleCaseFold);
  uint32_t rune1 = 0xFFFD;
  uint32_t rune2 = 0xFFFD;
  std::vector<uint32_t> runes1;
  std::vector<uint32_t> runes2;
  bool result = true;
  while (rune1 != 0 && rune2 != 0){
    rune1 = decomposition_iter_next(&iter1);
    rune2 = decomposition_iter_next(&iter2);
    runes1.push_back(rune1);
    runes2.push_back(rune2);
    if (rune1 != rune2){
      result = false;
      std::cout << "Error: test_new_normal: no match for matching strings: " << test1 << " and " << test2 << std::endl;
      return;
    }
  }
  std::cout << "SUCCESS: test_new_normal: match for matching strings: " << test1 << " and " << test2 << std::endl;
}

int new_normalizing_compare_n(const char *s, const char *t, size_t len){
  DecompositionIter_t iter1;
  DecompositionIter_t iter2;
  decomposition_iter_init(&iter1, s, strnlen(s, len), DecompositionType::CompatibleCaseFold);
  decomposition_iter_init(&iter2, t, strnlen(t, len), DecompositionType::CompatibleCaseFold);
  uint32_t rune1 = 0xFFFD;
  uint32_t rune2 = 0xFFFD;
  while (rune1 != 0 && rune2 != 0){
    rune1 = decomposition_iter_next(&iter1);
    rune2 = decomposition_iter_next(&iter2);
    if (rune1 != rune2){
      return clamp((int)((int64_t)rune1 - (int64_t)rune2));
    }
  }
  return clamp((int)((int64_t)rune1 - (int64_t)rune2));
}



int get_first_bit_set(uint32_t n){
#ifdef __GNUC__
  return __builtin_clz(n);
#else
  int count = 0;
  while (n){
    n >>= 1;
    count++;
  }
  return count;
#endif
}

int IndexByteNonASCII(const unsigned char *b, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (b[i] & 0x80) {
      return (int)i;
    }
  }
  return -1;
}



int new_normalizing_compare_old(const char *s, const char *t){
  size_t s_len = strlen(s);
  size_t t_len = strlen(t);
  ssize_t s_unicode_idx = IndexByteNonASCII((const unsigned char*)s, s_len);
  ssize_t t_unicode_idx = IndexByteNonASCII((const unsigned char*)t, t_len);
  s_unicode_idx = s_unicode_idx == -1 ? s_len : s_unicode_idx;
  t_unicode_idx = t_unicode_idx == -1 ? t_len : t_unicode_idx;
  // ascii fast path
  size_t i = 0;
  for (; i < s_unicode_idx && i < t_unicode_idx; i++){
    if ((s[i] | 0x20) != (t[i] | 0x20)){
      return clamp((int)s[i] - (int)t[i]);
    }
  }
  if (i >= s_len || i >= t_len){
    return clamp((int)s_len - (int)t_len);
  }
  s += i;
  t += i;
  s_len -= i;
  t_len -= i;

  DecompositionIter_t iter1;
  DecompositionIter_t iter2;
  decomposition_iter_init(&iter1, s, s_len, DecompositionType::CompatibleCaseFold);
  decomposition_iter_init(&iter2, t, t_len, DecompositionType::CompatibleCaseFold);
	char32_t rune1 = 0xFFFD;
	char32_t rune2 = 0xFFFD;
	std::vector<char32_t> runes1;
	std::vector<char32_t> runes2;
  while (rune1 != 0 && rune2 != 0){
    rune1 = decomposition_iter_next(&iter1);
    rune2 = decomposition_iter_next(&iter2);
#ifndef NDEBUG
		runes1.push_back(rune1);
		runes2.push_back(rune2);
#endif
    if (rune1 != rune2){
      return clamp((int)((int64_t)rune1 - (int64_t)rune2));
    }
  }
  return clamp((int)((int64_t)rune1 - (int64_t)rune2));
}
#include "simdutf-wrapper.h"

#include <codecvt>
int new_normalizing_compare(const char *s, const char *t){
	size_t s_len = strlen(s);
	size_t t_len = strlen(t);

	// ascii fast path
	ssize_t s_unicode_idx = IndexByteNonASCII((const unsigned char*)s, s_len);
	ssize_t t_unicode_idx = IndexByteNonASCII((const unsigned char*)t, t_len);
	s_unicode_idx = s_unicode_idx == -1 ? s_len : s_unicode_idx;
	t_unicode_idx = t_unicode_idx == -1 ? t_len : t_unicode_idx;
	size_t i = 0;
	for (; i < s_unicode_idx && i < t_unicode_idx; i++){
		if ((s[i] | 0x20) != (t[i] | 0x20)){
			return clamp((int)s[i] - (int)t[i]);
		}
	}
	if (i >= s_len || i >= t_len){
		return clamp((int)s_len - (int)t_len);
	}
	// go back 1 because it might be part of a decomposed multi-byte character
	i -= (i > 0) ? 1 : 0;
	s += i;
	t += i;
	s_len -= i;
	t_len -= i;

//	RecompositionIter_t iter1;
//	RecompositionIter_t iter2;
//	recomp_init(&iter1, s, s_len, DecompositionType::CompatibleCaseFold);
//	recomp_init(&iter2, t, t_len, DecompositionType::CompatibleCaseFold);
// decomp
	DecompositionIter_t iter1;
	DecompositionIter_t iter2;
	decomposition_iter_init(&iter1, s, s_len, DecompositionType::CompatibleCaseFold);
	decomposition_iter_init(&iter2, t, t_len, DecompositionType::CompatibleCaseFold);
	char32_t rune1 = 0xFFFD;
	char32_t rune2 = 0xFFFD;
//	std::vector<char32_t> runes1;
//	std::vector<char32_t> runes2;
	while (rune1 != 0 && rune2 != 0){
//		rune1 = recomp_next(&iter1);
//		rune2 = recomp_next(&iter2);
		rune1 = decomposition_iter_next(&iter1);
		rune2 = decomposition_iter_next(&iter2);
#ifndef NDEBUG
//		runes1.push_back(rune1);
//		runes2.push_back(rune2);
#endif
		if (rune1 != rune2){
#ifndef NDEBUG
//			char32_t *r1 = runes1.data();
//			char32_t *r2 = runes2.data();
//      std::u32string s1 = std::u32string(r1, r1 + runes1.size());
//      std::u32string s2 = std::u32string(r2, r2 + runes2.size());
////			std::cout << "Error: new_normalizing_compare: no match for matching strings at: " << s + iter1.iter.pos << " and " << t + iter2.iter.pos << std::endl;
//      // get the locale that enables printing 32 bit characters
//      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> thing;
//      std::cout << thing.to_bytes(s1) << " and " << thing.to_bytes(s2) << std::endl;
#endif
			decomposition_iter_destroy(&iter1);
			decomposition_iter_destroy(&iter2);
			return clamp((int)((int64_t)rune1 - (int64_t)rune2));
		}
	}
	decomposition_iter_destroy(&iter1);
	decomposition_iter_destroy(&iter2);
	return clamp((int)((int64_t)rune1 - (int64_t)rune2));
}

void test_dot_i(){
	const char * test1 = "İ";
	const char * test2 = "i";
	const char * test3 = "Aῌ";
	const char * test4 = "aῃ";
	const char * test1_to_use = test3;
	const char * test2_to_use = test4;
	auto rune1 = DecodeRuneInString(test1_to_use, strlen(test1_to_use));
	auto rune2 = DecodeRuneInString(test2_to_use, strlen(test2_to_use));

	auto converted1 = caseFold(rune1.rune);
	auto converted2 = caseFold(rune2.rune);
	if (converted1 != converted2){
		std::cout << "Error: casefold: no match for matching strings: " << test1_to_use << " and " << test2_to_use << std::endl;
	} else {
		std::cout << "SUCCESS: casefold: match for matching strings: " << test1_to_use << " and " << test2_to_use << std::endl;
	}
	if (new_normalizing_compare(test1_to_use, test2_to_use) != 0){
		std::cout << "Error: new_normalizing_compare: no match for matching strings: " << test1_to_use << " and " << test2_to_use << std::endl;
	} else {
		std::cout << "SUCCESS: new_normalizing_compare: match for matching strings: " <<	test1_to_use << " and " << test2_to_use << std::endl;
	}
}
#include <simdutf.h>
std::string our_normalize_str(const char *s, size_t len){
	DecompositionIter_t iter;
	decomposition_iter_init(&iter, s, len, DecompositionType::CompatibleCaseFold);
	char32_t rune = 0xFFFD;
	std::vector<char32_t> runes;
	runes.reserve(STRING_SIZE*4*8);
	while (rune != 0) {
		rune = decomposition_iter_next(&iter);
		runes.push_back(rune);
	}
	std::string result;
//  size_t sz2 = simdutf_utf8_length_from_utf32(runes.data(), runes.size());
  size_t sz2 = runes.size() * 4;
	result.resize(sz2);

	size_t sz = simdutf::convert_utf32_to_utf8(runes.data(), runes.size(), result.data());
	result.resize(sz);
//		char * result = (char *)malloc(runes.size() * 4);
//	size_t sz = simdutf::convert_utf32_to_utf8(runes.data(), runes.size(), result);
//	result = (char *)realloc(result, sz + 1);

	return result;
}

void test_converting(const char** s, size_t table_len = TABLE_SIZE){
	// get start time
	int64_t start = getCurrentNanos();
	int64_t duration = 0;
	int64_t failures = 0;
	int64_t successes = 0;
	int inc = 2;
	const int64_t iters = table_len * TABLE_TRAVERSALS * inc;
	for (int i = 0; i < iters; i+=inc){
		auto a = s[i % table_len];
		start = getCurrentNanos();
		auto result = our_normalize_str(a, strlen(a));
		duration += getCurrentNanos() - start;

		if (result.empty()){
			failures++;
		} else {
			successes++;
		}
	}
	printf("our_normalize_str conversion iters %llu, failures %lld, time : %lldms\n",iters, failures, (duration)/ 1000000);
	for (int i = 0; i < iters; i+=inc){
		auto str = s[i % table_len];
		start = getCurrentNanos();
		const char *retval;
		auto strsize = utf8proc_map((const uint8_t*)str, 0, (uint8_t**)&retval, (utf8proc_option_t)(UTF8PROC_NULLTERM | UTF8PROC_COMPAT | UTF8PROC_STABLE | UTF8PROC_DECOMPOSE));

		duration += getCurrentNanos() - start;

		if (strsize == 0){
			failures++;
		} else {
			successes++;
		}
	}
	printf("utf8proc_map conversion iters %llu, failures %lld, time : %lldms\n",iters, failures, (duration)/ 1000000);

}


//U"ę<Z" U"ę<z"
int main() {
  //  test_icu();
  //  return 0;
  // return 0;
  test_thing();
  test_new_normal();
	test_dot_i();

  // generate a random utf-8 string

//	std::vector<testType> filters = {UCSTRCASECMP, UCSTRCASECMP_N, NEW_NORMALIZING_COMPARE, NEW_NORMALIZING_COMPARE_N, STRCASECMP, STRNCASECMP, ICU_CASE_CMP, ICU_CASE_CMP_N, UTF8CASECMP, UTF8NCASECMP, };
	std::vector<testType> filters = { NEW_NORMALIZING_COMPARE, NEW_NORMALIZING_COMPARE_N};





  generateRandomMatchingStrings3(s, TABLE_SIZE, STRING_SIZE);
	 printf("**** Generated Random Matching UTF-8 strings *****\n");
	test_converting(s, TABLE_SIZE);
	run_tests(s, true, TABLE_SIZE, 0, false, filters);
	test_normalization(s, TABLE_SIZE);
	free_all_strings();

	 generateRandomMatchingStrings(s, TABLE_SIZE, STRING_SIZE);
	 printf("**** Generated Matching UTF-8 strings *****\n");
	 run_tests(s, true, TABLE_SIZE, 0, false, filters);
   free_all_strings();

  generateRandomMatchingStrings2(s, TABLE_SIZE, STRING_SIZE);
  printf("**** Generated Tricky matching UTF-8 strings *****\n");
  //  run_tests(s, true, TABLE_SIZE, 0, false, { UCSTRCASECMP});
  run_tests(s, true, TABLE_SIZE, 0, false, filters);
  free_all_strings();

	for (int i = 0; i < TABLE_SIZE; i++) {
		s[i] = generate_random_ascii_string(STRING_SIZE);
	}
	printf("**** Generated ASCII strings *****\n");
	run_tests(s, false, TABLE_SIZE, 0, false, filters);
	free_all_strings();

   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_utf8_string(STRING_SIZE);
   }
   printf("**** Generated UTF-8 strings *****\n");
   run_tests(s, false, TABLE_SIZE, 0, false, filters);
   free_all_strings();

  // generate a random utf-8 string
   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_garbage(STRING_SIZE);
   }
   printf("**** Generated GARBAGE strings *****\n");
   run_tests(s, false, TABLE_SIZE, STRING_SIZE, true, filters);
   free_all_strings();



   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_invalid_utf8_string(STRING_SIZE);
   }

   printf("**** Generated INVALID UTF-8 strings *****\n");
   run_tests(s, false, TABLE_SIZE, STRING_SIZE, false, filters);
   free_all_strings();


  return 0;
}
