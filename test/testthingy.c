
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "simdutf-wrapper.h"

#include "ucstrcase.h"
#include "compose.h"
const char* test1 = "This is a test string.";
const char* test2 = "this IS a TEST string.";
const char* test3 = "ﷺ";
const char* test4 = "صلى الله عليه وسلم";
const char* test5 = "ﷻ";
const char* test6 = "جل جلاله";

typedef uint32_t char32_t;


size_t decompose_str(const char* str, size_t len, char32_t* output) {
	DecompositionIter_t iter;
	decomposition_iter_init(&iter, str, len, Compatible);
	uint32_t ch = decomposition_iter_next(&iter);
	size_t count = 0;
	while (ch != 0) {
		count++;
		*output = ch;
		output++;
		ch = decomposition_iter_next(&iter);
	}
//	*output = 0;
	decomposition_iter_destroy(&iter);
	return count;
}

size_t recompose_str(const char* str, size_t len, char32_t* output) {
	RecompositionIter_t iter;
	recomp_init(&iter, str, len, Compatible);
	uint32_t ch = recomp_next(&iter);
	size_t count = 0;
	while (ch != 0) {
		count++;
		*output = ch;
		output++;
		ch = recomp_next(&iter);
	}
	*output = 0;
	recomp_destroy(&iter);
	return count;
}

#define MAX_DECOMPOSED_SIZE 18
int main(){
	const char* test_to_use = test3;
	const char* compare_test = test4;
	size_t len = strlen(test_to_use);
	char32_t *decomposed_u32 = (char32_t*)malloc((MAX_DECOMPOSED_SIZE * len + 1) * sizeof(char32_t));
	size_t decomp_u32_len = decompose_str(test_to_use, len, decomposed_u32);
	// convert it back
	char * decomposed_u8 = (char *)malloc((MAX_DECOMPOSED_SIZE * len + 1) * sizeof(char));
	size_t decomp_u8_len = simdutf_convert_utf32_to_utf8(decomposed_u32, decomp_u32_len, decomposed_u8);
	char32_t * recomposed_u32 = (char32_t *)malloc((decomp_u32_len + 1) * sizeof(char32_t));
	recompose_str(decomposed_u8, decomp_u8_len, recomposed_u32);
	char * recomposed_u8 = (char *)malloc((MAX_DECOMPOSED_SIZE * len + 1) * sizeof(char));
	size_t recomposed_u8_len = simdutf_convert_utf32_to_utf8(recomposed_u32, decomp_u32_len, recomposed_u8);
//	char* recomposed_u8 = (char*)malloc((MAX_DECOMPOSED_SIZE * len + 1) * sizeof(char));
//	char * recomposed = (char *)malloc(MAX_DECOMPOSED_SIZE * 4 * sizeof(char));
	// print it out
	printf("composed:   %s\n", test_to_use);
	printf("decomposed: %s\n", decomposed_u8);
	printf("reference:  %s\n", compare_test);
	printf("recomposed: %s\n", recomposed_u8);
	// print out the individual bytes as \x
	for (size_t i = 0; i < strlen(decomposed_u8); i++){
		printf("\\x%02x", (uint8_t)decomposed_u8[i]);
	}
	printf("\n");
	if (strcmp(compare_test, decomposed_u8) != 0){
		printf("Decomposition failed\n");
	} else{
		printf("Decomposition passed\n");
	}
	// free the memory
	free(decomposed_u32);
	free(decomposed_u8);
	if (ucstrcasecmp(test1, test2) != 0){
		printf("ucstrcasecmp failed\n");
		return 1;
	}
	printf("ucstrcasecmp passed\n");
	return 0;
}