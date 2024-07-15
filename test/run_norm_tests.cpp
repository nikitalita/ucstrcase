#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <stdint.h>
#include "normalization_tests.h"
#include "compose.h"
#include "test_stuff.h"


using namespace test_funcs;

void nfd_test(const NormalizationTest &test, const std::string &source, const std::string &test_nfc, const std::string &test_nfd,const std::string &test_nfkc, const std::string &test_nfkd){
	{
		std::u32string nfd_from_source = ucstrcase::Decompositions<char, DecompositionType::Canonical>(source.c_str(), source.size()).to_utf32_string();
		std::u32string nfd_from_nfc = ucstrcase::Decompositions<char, DecompositionType::Canonical>(test_nfc.c_str(), test_nfc.size()).to_utf32_string();
		std::u32string nfd_from_nfd = ucstrcase::Decompositions<char, DecompositionType::Canonical>(test_nfd.c_str(), test_nfd.size()).to_utf32_string();
		std::u32string nfd_from_nfkc = ucstrcase::Decompositions<char, DecompositionType::Canonical>(test_nfkc.c_str(), test_nfkc.size()).to_utf32_string();
		std::u32string nfd_from_nfkd = ucstrcase::Decompositions<char, DecompositionType::Canonical>(test_nfkd.c_str(), test_nfkd.size()).to_utf32_string();
//		std::u32string nfd_from_nfc = DecomposeString(test_nfc.c_str(), test_nfc.size(), DecompositionType::Canonical);
//		std::u32string nfd_from_nfd = DecomposeString(test_nfd.c_str(), test_nfd.size(), DecompositionType::Canonical);
//		std::u32string nfd_from_nfkc = DecomposeString(test_nfkc.c_str(), test_nfkc.size(),
//																								 DecompositionType::Canonical);
//		std::u32string nfd_from_nfkd = DecomposeString(test_nfkd.c_str(), test_nfkd.size(),
//																								 DecompositionType::Canonical);

		std::u32string test_nfd_u32{test.nfd};
		REQUIRE(test_nfd_u32 == nfd_from_source);
		REQUIRE(test.nfd == nfd_from_nfc);
		REQUIRE(test.nfd == nfd_from_nfd);
		REQUIRE(test.nfkd == nfd_from_nfkc);
		REQUIRE(test.nfkd == nfd_from_nfkd);
	}
}

void nfc_test(const NormalizationTest &test, const std::string &source, const std::string &test_nfc, const std::string &test_nfd,const std::string &test_nfkc, const std::string &test_nfkd){
	{
//		std::u32string nfc_from_source = RecomposeString(source.c_str(), source.size(), DecompositionType::Canonical);
//		std::u32string nfc_from_nfc = RecomposeString(test_nfc.c_str(), test_nfc.size(), DecompositionType::Canonical);
//		std::u32string nfc_from_nfd = RecomposeString(test_nfd.c_str(), test_nfd.size(), DecompositionType::Canonical);
//		std::u32string nfc_from_nfkc = RecomposeString(test_nfkc.c_str(), test_nfkc.size(), DecompositionType::Canonical);
//		std::u32string nfc_from_nfkd = RecomposeString(test_nfkd.c_str(), test_nfkd.size(), DecompositionType::Canonical);
		std::u32string nfc_from_source = ucstrcase::Recompositions<char, DecompositionType::Canonical>(source.c_str(), source.size()).to_utf32_string();
		std::u32string nfc_from_nfc = ucstrcase::Recompositions<char, DecompositionType::Canonical>(test_nfc.c_str(), test_nfc.size()).to_utf32_string();
		std::u32string nfc_from_nfd = ucstrcase::Recompositions<char, DecompositionType::Canonical>(test_nfd.c_str(), test_nfd.size()).to_utf32_string();
		std::u32string nfc_from_nfkc = ucstrcase::Recompositions<char, DecompositionType::Canonical>(test_nfkc.c_str(), test_nfkc.size()).to_utf32_string();
		std::u32string nfc_from_nfkd = ucstrcase::Recompositions<char, DecompositionType::Canonical>(test_nfkd.c_str(), test_nfkd.size()).to_utf32_string();
		std::u32string test_nfc_u32{test.nfc};
		std::u32string test_nfkc_u32{test.nfkc};




		REQUIRE(test_nfc_u32 == nfc_from_source);
		REQUIRE(test_nfc_u32 == nfc_from_nfc);
		REQUIRE(test_nfc_u32 == nfc_from_nfd);
		REQUIRE(test_nfkc_u32 == nfc_from_nfkc);
		REQUIRE(test_nfkc_u32 == nfc_from_nfkd);
	}
}

void nfkd_test(const NormalizationTest &test, const std::string &source, const std::string &test_nfc, const std::string &test_nfd,const std::string &test_nfkc, const std::string &test_nfkd){
	{
//		std::u32string nfkd_from_source = DecomposeString(source.c_str(), source.size(), DecompositionType::Compatible);
//		std::u32string nfkd_from_nfc = DecomposeString(test_nfc.c_str(), test_nfc.size(), DecompositionType::Compatible);
//		std::u32string nfkd_from_nfd = DecomposeString(test_nfd.c_str(), test_nfd.size(), DecompositionType::Compatible);
//		std::u32string nfkd_from_nfkc = DecomposeString(test_nfkc.c_str(), test_nfkc.size(), DecompositionType::Compatible);
//		std::u32string nfkd_from_nfkd = DecomposeString(test_nfkd.c_str(), test_nfkd.size(), DecompositionType::Compatible);
		std::u32string nfkd_from_source = ucstrcase::Decompositions<char, DecompositionType::Compatible>(source.c_str(), source.size()).to_utf32_string();
		std::u32string nfkd_from_nfc = ucstrcase::Decompositions<char, DecompositionType::Compatible>(test_nfc.c_str(), test_nfc.size()).to_utf32_string();
		std::u32string nfkd_from_nfd = ucstrcase::Decompositions<char, DecompositionType::Compatible>(test_nfd.c_str(), test_nfd.size()).to_utf32_string();
		std::u32string nfkd_from_nfkc = ucstrcase::Decompositions<char, DecompositionType::Compatible>(test_nfkc.c_str(), test_nfkc.size()).to_utf32_string();
		std::u32string nfkd_from_nfkd = ucstrcase::Decompositions<char, DecompositionType::Compatible>(test_nfkd.c_str(), test_nfkd.size()).to_utf32_string();
		REQUIRE(test.nfkd == nfkd_from_source);
		REQUIRE(test.nfkd == nfkd_from_nfc);
		REQUIRE(test.nfkd == nfkd_from_nfd);
		REQUIRE(test.nfkd == nfkd_from_nfkc);//s
		REQUIRE(test.nfkd == nfkd_from_nfkd);
	}
}

void nfkc_test(const NormalizationTest &test, const std::string &source, const std::string &test_nfc, const std::string &test_nfd,const std::string &test_nfkc, const std::string &test_nfkd){
	{
		std::u32string nfkc_from_source = RecomposeString(source.c_str(), source.size(), DecompositionType::Compatible);
		std::u32string nfkc_from_nfc = RecomposeString(test_nfc.c_str(), test_nfc.size(), DecompositionType::Compatible);
		std::u32string nfkc_from_nfd = RecomposeString(test_nfd.c_str(), test_nfd.size(), DecompositionType::Compatible);
		std::u32string nfkc_from_nfkc = RecomposeString(test_nfkc.c_str(), test_nfkc.size(), DecompositionType::Compatible);
		std::u32string nfkc_from_nfkd = RecomposeString(test_nfkd.c_str(), test_nfkd.size(), DecompositionType::Compatible);
		REQUIRE(test.nfkc == nfkc_from_source);
		REQUIRE(test.nfkc == nfkc_from_nfc);
		REQUIRE(test.nfkc == nfkc_from_nfd);
		REQUIRE(test.nfkc == nfkc_from_nfkc);
		REQUIRE(test.nfkc == nfkc_from_nfkd);
	}
}

void single_test(const NormalizationTest &test){
	std::string source = convert_utf32_to_utf8(test.source);
	std::string test_nfc = convert_utf32_to_utf8(test.nfc);
	std::string test_nfd = convert_utf32_to_utf8(test.nfd);
	std::string test_nfkc = convert_utf32_to_utf8(test.nfkc);
	std::string test_nfkd = convert_utf32_to_utf8(test.nfkd);
	nfkd_test(test, source, test_nfc, test_nfd, test_nfkc, test_nfkd);
	nfd_test(test, source, test_nfc, test_nfd, test_nfkc, test_nfkd);
	nfc_test(test, source, test_nfc, test_nfd, test_nfkc, test_nfkd);
	nfkc_test(test, source, test_nfc, test_nfd, test_nfkc, test_nfkd);
}

std::string get_prefix(int i, const std::string_view &test_name){
	return std::to_string(i) + ": " + test_name.data();
}

TEST_CASE("Normalization tests Part 0: Specific cases") {
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_0_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_0_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		single_test(test);
	}
}
TEST_CASE("Normalization tests Part 1: Character by character test") {
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_1_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_1_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		single_test(test);
	}
}
TEST_CASE("Normalization tests Part 2: Canonical Order Test") {
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_2_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_2_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		single_test(test);
	}
}
TEST_CASE("Normalization tests Part 3: PRI #29 Test") {
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_3_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_3_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		single_test(test);
	}
}

void run_quick_test(const NormalizationTest &test){
	REQUIRE(quick_check_nfd(test.nfd));
	REQUIRE(quick_check_nfc(test.nfc));
	REQUIRE(quick_check_nfkd(test.nfkd));
	REQUIRE(quick_check_nfkc(test.nfkc));
	if (test.nfc != test.nfd){
		REQUIRE(!quick_check_nfc(test.nfd));
		REQUIRE(!quick_check_nfd(test.nfc));
	}
	if (test.nfkc != test.nfc){
		REQUIRE(!quick_check_nfkc(test.nfc));
		REQUIRE(quick_check_nfc(test.nfkc));
	}
	if (test.nfkd != test.nfd){
		REQUIRE(!quick_check_nfkd(test.nfd));
		REQUIRE(quick_check_nfd(test.nfkd));
	}
}

TEST_CASE("Quick Check tests Part 0"){
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_0_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_0_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		run_quick_test(test);
	}
}

TEST_CASE("Quick Check tests Part 1"){
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_1_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_1_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		run_quick_test(test);
	}
}
TEST_CASE("Quick Check tests Part 2"){
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_2_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_2_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		run_quick_test(test);
	}
}
TEST_CASE("Quick Check tests Part 3"){
	auto i = GENERATE(Catch::Generators::range((size_t)0, (size_t)NORMALIZATION_PART_3_TESTS_SIZE));
	const NormalizationTest &test = NORMALIZATION_PART_3_TESTS[i];
	auto testname_suffix = get_prefix(i, test.test_name);
	DYNAMIC_SECTION("" << testname_suffix) {
		run_quick_test(test);
	}
}