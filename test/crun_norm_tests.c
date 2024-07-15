#include "normalization_tests.h"
#include "simdutf-wrapper.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "normalize.h"


char * convert_utf32_to_utf8(_UCSTEST_C32PTR_T src, size_t len){
    char *dst = (char *)malloc((len * 4 + 1) * sizeof(char));
    size_t result = simdutf_convert_utf32_to_utf8(src, len, dst);
    return dst;
}

size_t strlen_utf32(_UCSTEST_C32PTR_T src){
    size_t len = 0;
    while (src[len] != 0){
        len++;
    }
    return len;
}


_UCSTEST_C32PTR_T DecomposeString(const char *s, DecompositionType kind){
    DecompositionIter_t iter;
    size_t s_len = strlen(s);
    decomposition_iter_init(&iter, s, s_len, kind);
    _UCSTEST_C32_T rune = 0xFFFD;
    _UCSTEST_C32_T *runes = (_UCSTEST_C32_T *)malloc(s_len * 18 * sizeof(_UCSTEST_C32_T));
    size_t i = 0;
    rune = decomposition_iter_next(&iter);
    while (rune != 0) {
        runes[i] = rune;
        i++;
        rune = decomposition_iter_next(&iter);
    }
    runes[i] = 0;
    runes = (_UCSTEST_C32_T *)realloc(runes, (i + 1) * sizeof(_UCSTEST_C32_T));
    return runes;
}

_UCSTEST_C32PTR_T RecomposeString(const char *s, DecompositionType kind){
    RecompositionIter_t iter;
    size_t s_len = strlen(s);
    recomp_init(&iter, s, s_len, kind);
    _UCSTEST_C32_T rune = 0xFFFD;
    _UCSTEST_C32_T *runes = (_UCSTEST_C32_T *)malloc(s_len * 18 * sizeof(_UCSTEST_C32_T));
    size_t i = 0;
    rune = recomp_next(&iter);
    while (rune != 0) {
        runes[i] = rune;
        i++;
        rune = recomp_next(&iter);
    }
    runes[i] = 0;
    runes = (_UCSTEST_C32_T *)realloc(runes, (i + 1) * sizeof(_UCSTEST_C32_T));
    return runes;
}

bool compare_utf32(_UCSTEST_C32PTR_T a, _UCSTEST_C32PTR_T b){
    size_t i = 0;
    while (a[i] != 0 && b[i] != 0){
        if (a[i] != b[i]){
            return false;
        }
        i++;
    }
    return a[i] == b[i];
}
#define REQUIRE(x) if (!(x)) { printf("Test failed at line %d\n", name, __LINE__); return 1; }
#define test_REQUIRE(name, x) if (!(x)) { printf("%s_test failed at line %d\n", name, __LINE__); return 1; }

int nfc_test(const NormalizationTest *test, const char *source, const char *test_nfc, const char *test_nfd,const char *test_nfkc, const char *test_nfkd){
	{
		_UCSTEST_C32PTR_T nfc_from_source = RecomposeString(source, Canonical);
		_UCSTEST_C32PTR_T nfc_from_nfc = RecomposeString(test_nfc, Canonical);
		_UCSTEST_C32PTR_T nfc_from_nfd = RecomposeString(test_nfd, Canonical);
		_UCSTEST_C32PTR_T nfc_from_nfkc = RecomposeString(test_nfkc, Canonical);
		_UCSTEST_C32PTR_T nfc_from_nfkd = RecomposeString(test_nfkd, Canonical);
		test_REQUIRE("nfc", compare_utf32(nfc_from_source, test->nfc));
		test_REQUIRE("nfc", compare_utf32(nfc_from_nfc, test->nfc));
        test_REQUIRE("nfc", compare_utf32(nfc_from_nfd, test->nfc));
		test_REQUIRE("nfc", compare_utf32(test->nfkc, nfc_from_nfkc));
        test_REQUIRE("nfc", compare_utf32(test->nfkc, nfc_from_nfkd));
	}
    return 0;
}
int nfd_test(const NormalizationTest *test, const char *source, const char *test_nfc, const char *test_nfd,const char *test_nfkc, const char *test_nfkd){
    {
        _UCSTEST_C32PTR_T nfd_from_source = DecomposeString(source, Canonical);
        _UCSTEST_C32PTR_T nfd_from_nfc = DecomposeString(test_nfc, Canonical);
        _UCSTEST_C32PTR_T nfd_from_nfd = DecomposeString(test_nfd, Canonical);
        _UCSTEST_C32PTR_T nfd_from_nfkc = DecomposeString(test_nfkc, Canonical);
        _UCSTEST_C32PTR_T nfd_from_nfkd = DecomposeString(test_nfkd, Canonical);
        test_REQUIRE("nfd", compare_utf32(nfd_from_source, test->nfd));
        test_REQUIRE("nfd", compare_utf32(nfd_from_nfc, test->nfd));
        test_REQUIRE("nfd", compare_utf32(nfd_from_nfd, test->nfd));
        test_REQUIRE("nfd", compare_utf32(nfd_from_nfkc, test->nfkd));
        test_REQUIRE("nfd", compare_utf32(nfd_from_nfkd, test->nfkd));
    }
    return 0;
}
int nfkd_test(const NormalizationTest *test, const char *source, const char *test_nfc, const char *test_nfd,const char *test_nfkc, const char *test_nfkd){
    {
        _UCSTEST_C32PTR_T nfkd_from_source = DecomposeString(source, Compatible);
        _UCSTEST_C32PTR_T nfkd_from_nfc = DecomposeString(test_nfc, Compatible);
        _UCSTEST_C32PTR_T nfkd_from_nfd = DecomposeString(test_nfd, Compatible);
        _UCSTEST_C32PTR_T nfkd_from_nfkc = DecomposeString(test_nfkc, Compatible);
        _UCSTEST_C32PTR_T nfkd_from_nfkd = DecomposeString(test_nfkd, Compatible);
        test_REQUIRE("nfkd", compare_utf32(nfkd_from_source, test->nfkd));
        test_REQUIRE("nfkd", compare_utf32(nfkd_from_nfc, test->nfkd));
        test_REQUIRE("nfkd", compare_utf32(nfkd_from_nfd, test->nfkd));
        test_REQUIRE("nfkd", compare_utf32(nfkd_from_nfkc, test->nfkd));
        test_REQUIRE("nfkd", compare_utf32(nfkd_from_nfkd, test->nfkd));
    }
    return 0;
}
int nfkc_test(const NormalizationTest *test, const char *source, const char *test_nfc, const char *test_nfd,const char *test_nfkc, const char *test_nfkd){
    {
        _UCSTEST_C32PTR_T nfkc_from_source = RecomposeString(source, Compatible);
        _UCSTEST_C32PTR_T nfkc_from_nfc = RecomposeString(test_nfc, Compatible);
        _UCSTEST_C32PTR_T nfkc_from_nfd = RecomposeString(test_nfd, Compatible);
        _UCSTEST_C32PTR_T nfkc_from_nfkc = RecomposeString(test_nfkc, Compatible);
        _UCSTEST_C32PTR_T nfkc_from_nfkd = RecomposeString(test_nfkd, Compatible);
        test_REQUIRE("nfkc", compare_utf32(nfkc_from_source, test->nfkc));
        test_REQUIRE("nfkc", compare_utf32(nfkc_from_nfc, test->nfkc));
        test_REQUIRE("nfkc", compare_utf32(nfkc_from_nfd, test->nfkc));
        test_REQUIRE("nfkc", compare_utf32(nfkc_from_nfkc, test->nfkc));
        test_REQUIRE("nfkc", compare_utf32(nfkc_from_nfkd, test->nfkc));
    }
    return 0;
}
int run_test_part(int partnum, size_t size, const NormalizationTest *tests){
    int i = 0;
    size_t total = 0;
    for (; i < size; i++){
        const NormalizationTest *test = &tests[i];
        char *source = convert_utf32_to_utf8(test->source, strlen_utf32(test->source));
        char *nfc = convert_utf32_to_utf8(test->nfc, strlen_utf32(test->nfc));
        char *nfd = convert_utf32_to_utf8(test->nfd, strlen_utf32(test->nfd));
        char *nfkc = convert_utf32_to_utf8(test->nfkc, strlen_utf32(test->nfkc));
        char *nfkd = convert_utf32_to_utf8(test->nfkd, strlen_utf32(test->nfkd));
        int result = nfc_test(test, source, nfc, nfd, nfkc, nfkd);
        result += nfd_test(test, source, nfc, nfd, nfkc, nfkd);
        result += nfkc_test(test, source, nfc, nfd, nfkc, nfkd);
        result += nfkd_test(test, source, nfc, nfd, nfkc, nfkd);
        free(source);
        free(nfc);
        free(nfd);
        free(nfkc);
        free(nfkd);
        if (result != 0){
            printf("Test %s failed\n", test->test_name);
            return -1;
        }
    }
    total += i;
    printf("Part %d: %d tests passed\n", partnum, i);
    return total;
}
int main(){
    int i = 0;
    size_t total = 0;
    int iters = run_test_part(0, NORMALIZATION_PART_0_TESTS_SIZE, NORMALIZATION_PART_0_TESTS);
    if (iters < 0){
        return 1;
    }
    total += iters;
    iters = run_test_part(1, NORMALIZATION_PART_1_TESTS_SIZE, NORMALIZATION_PART_1_TESTS);
    if (iters < 0){
        return 1;
    }
    total += iters;
    iters = run_test_part(2, NORMALIZATION_PART_2_TESTS_SIZE, NORMALIZATION_PART_2_TESTS);
    if (iters < 0){
        return 1;
    }
    total += iters;
    iters = run_test_part(3, NORMALIZATION_PART_3_TESTS_SIZE, NORMALIZATION_PART_3_TESTS);
    if (iters < 0){
        return 1;
    }
    total += iters;
    printf("All %lu tests passed\n", total);
    return 0;
}