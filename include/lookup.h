#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
	Yes,
	No,
	Maybe,
} IsNormalized;

#ifdef __cplusplus
extern "C" {
#endif
uint8_t canonical_combining_class(uint32_t c);
uint32_t composition_table(uint32_t c1, uint32_t c2);
/**
 * @brief Returns the fully canonically decomposed form of a character
 * 
 * @param c The character to decompose
 * @param ret A pointer to a pointer to the start of the character's decomposition
 * @return size_t The number of codepoints in the decomposition
 */
size_t canonical_fully_decomposed(uint32_t c, const uint32_t **ret);

/**
 * @brief Returns the fully compatibility decomposed form of a character
 * 
 * @param c The character to decompose
 * @param ret A pointer to a pointer to the start of the character's decomposition
 * @return size_t The number of codepoints in the decomposition
 */
size_t compatibility_fully_decomposed(uint32_t c, const uint32_t **ret);

/**
 * @brief Returns the fully compatibility decomposed CJK variant of a character
 * 
 * @param c The character to decompose
 * @param ret A pointer to a pointer to the start of the character's decomposition
 * @return size_t The number of codepoints in the decomposition
 */
size_t cjk_compat_variants_fully_decomposed(uint32_t c, const uint32_t **ret);

size_t NFKC_CF_decomposed(uint32_t c, const uint32_t **ret);

/**
 * @brief Returns whether a character is a combining mark
 * 
 * @param c The character to check
 * @return true If the character is a combining mark, false otherwise
 */
bool is_combining_mark(uint32_t c);

/**
 * @brief Returns the number of trailing non-starter characters in a 
 * string that are safe to stream
 * 
 * @param s The string to check
 * @return size_t The number of non-starter characters that are safe to stream
 */
size_t stream_safe_trailing_nonstarters(uint32_t c);

/**
 * @brief Reports whether the string is in a fully composed canonical form (NFC)
 * 
 * @param s The string to check
 * @return IsNormalized Yes if the string is in NFC, No if it is not, Maybe if it is ambiguous
 */
IsNormalized quick_check_nfc(const char* s);
IsNormalized quick_check_nfd(const char* s);
IsNormalized quick_check_nfkc(const char* s);
IsNormalized quick_check_nfkd(const char* s);
IsNormalized is_qc_nfc(uint32_t c);
IsNormalized is_qc_nfd(uint32_t c);
IsNormalized is_qc_nfkc(uint32_t c);
IsNormalized is_qc_nfkd(uint32_t c);

uint32_t caseFold(uint32_t c);
uint16_t *foldMap(uint32_t r);
void foldMapExcludingUpperLower(uint32_t r, uint32_t result[2]);
const uint16_t * getFullCaseFold(uint32_t r);

#ifdef __cplusplus

} // extern "C"
#endif