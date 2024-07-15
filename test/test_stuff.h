
#ifndef UCSTRCASE_TEST_STUFF_H
#define UCSTRCASE_TEST_STUFF_H
#include <string>
#include <vector>
#include "normalize.h"
#include "normalize.hpp"
#include <simdutf.h>

//#include "normalization_tests.h"
namespace test_funcs{

char* char_utf32_to_utf8(char32_t utf32, const char* buffer);
std::u32string DecomposeString(const char *source, size_t source_sz, DecompositionType kind);
std::u32string DecomposeString(const std::u32string &s, DecompositionType kind);
std::u32string RecomposeString(const char *source, size_t source_sz, DecompositionType kind);
std::u32string RecomposeString(const std::u32string &s, DecompositionType kind);
std::u32string convert_utf8_to_utf32(const std::string &src);
std::string convert_utf32_to_utf8(const std::u32string &src);
template <DecompositionType kind>
std::u32string DecomposeString2(const char *s, size_t len) {
	return ucstrcase::Decompositions<char, kind>(s, len).to_utf32_string();
}
template <DecompositionType kind>
std::u32string DecomposeString2(const std::u32string &s) {
	return ucstrcase::Decompositions<char32_t, kind>(s).to_utf32_string();
}
bool quick_check_nfc(const std::u32string &s);
bool quick_check_nfd(const std::u32string &s);
bool quick_check_nfkc(const std::u32string &s);
bool quick_check_nfkd(const std::u32string &s);
//void single_test(const NormalizationTest &test);
};

#endif //UCSTRCASE_TEST_STUFF_H
