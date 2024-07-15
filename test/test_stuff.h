
#ifndef UCSTRCASE_TEST_STUFF_H
#define UCSTRCASE_TEST_STUFF_H
#include <string>
#include <vector>
#include "compose.h"
//#include "normalization_tests.h"
namespace test_funcs{

char* char_utf32_to_utf8(char32_t utf32, const char* buffer);
std::u32string DecomposeString(const char *source, size_t source_sz, DecompositionType kind);
std::u32string DecomposeString(const std::u32string &s, DecompositionType kind);
std::u32string RecomposeString(const char *source, size_t source_sz, DecompositionType kind);
std::u32string RecomposeString(const std::u32string &s, DecompositionType kind);
std::u32string convert_utf8_to_utf32(const std::string &src);
std::string convert_utf32_to_utf8(const std::u32string &src);
//void single_test(const NormalizationTest &test);
};

#endif //UCSTRCASE_TEST_STUFF_H
