#include "test_stuff.h"
#include "simdutf-wrapper.h"
#include <simdutf.h>
//#include <catch2/catch_test_macros.hpp>

namespace test_funcs {

char *char_utf32_to_utf8(char32_t utf32, const char *buffer)
// Encodes the UTF-32 encoded char into a UTF-8 string.
// Stores the result in the buffer and returns the position
// of the end of the buffer
// (unchecked access, be sure to provide a buffer that is big enough)
{
	char *end = const_cast<char *>(buffer);
	if (utf32 < 0x7F) *(end++) = static_cast<unsigned>(utf32);
	else if (utf32 < 0x7FF) {
		*(end++) = 0b1100'0000 + static_cast<unsigned>(utf32 >> 6);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	} else if (utf32 < 0x10000) {
		*(end++) = 0b1110'0000 + static_cast<unsigned>(utf32 >> 12);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	} else if (utf32 < 0x110000) {
		*(end++) = 0b1111'0000 + static_cast<unsigned>(utf32 >> 18);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 12) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
		*(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
	} else throw std::runtime_error("Invalid UTF-32 character");
	*end = '\0';
	return end;
}

std::u32string DecomposeString(const char *s, size_t len, DecompositionType kind) {
	DecompositionIter_t iter;
	decomposition_iter_init(&iter, s, len, kind);
	char32_t rune = 0xFFFD;
	std::u32string runes;
	runes.reserve(len * 4 * 8);
	rune = decomposition_iter_next(&iter);
	while (rune != 0) {
		runes.push_back(rune);
		rune = decomposition_iter_next(&iter);
	}
	runes.resize(runes.size());

	return runes;
}

std::u32string RecomposeString(const char *s, size_t len, DecompositionType kind) {
	RecompositionIter_t iter;
	recomp_init(&iter, s, len, kind);
	char32_t rune = 0xFFFD;
	std::u32string runes;
	runes.reserve(len * 4 * 8);
	rune = recomp_next(&iter);
	while (rune != 0) {
		runes.push_back(rune);
		rune = recomp_next(&iter);
	}
	runes.resize(runes.size());

	return runes;

}
std::u32string RecomposeString(const std::u32string &s, DecompositionType kind) {
	char * str = (char *)malloc(s.size() * 4 * sizeof(char));
	size_t str_size = simdutf::convert_utf32_to_utf8(s.data(), s.size(), str);
	auto ret = RecomposeString(str, str_size, kind);
	free(str);
	return ret;
}

std::u32string DecomposeString(const std::u32string &s, DecompositionType kind) {
	char * str = (char *)malloc(s.size() * 4 * sizeof(char));
	size_t str_size = simdutf::convert_utf32_to_utf8(s.data(), s.size(), str);
	auto ret = DecomposeString(str, str_size, kind);
	free(str);
	return ret;
}


// Character by character test
// TEST_CASE("Normalization Part 1 tests: Character by character test") {
std::u32string convert_utf8_to_utf32(const std::string &src) {
	std::u32string result;
	result.resize(src.size());
	size_t sz = simdutf::convert_utf8_to_utf32(src.data(), src.size(), const_cast<char32_t *>(result.data()));
	result.resize(sz);
	return result;

}

std::string convert_utf32_to_utf8(const std::u32string &src) {
	std::string result;
	result.resize(src.size() * 4);
	size_t sz = simdutf::convert_utf32_to_utf8(src.data(), src.size(), const_cast<char *>(result.data()));
	result.resize(sz);
	return result;
}
}