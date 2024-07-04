

#pragma once
#include <stdint.h>
#include <stdbool.h>


// A foldPair stores Unicode case folding pairs
typedef struct foldPair_t{
    uint32_t From;
    uint32_t To;
} foldPair;

typedef struct {
    uint16_t r;
    uint16_t a[2];
} _FoldMapExcludingUpperLowerItem;


// Built from Unicode version "15.0.0".
// Built from CLDR version "32".

static const uint32_t _CaseFoldsSeed = 0xFFE00C86;
static const uint32_t _CaseFoldsShift = 19;


static const uint32_t _UpperLowerSeed = 0x6AE7FD95;
static const uint32_t _UpperLowerShift = 19;


static const uint32_t _FoldMapSeed = 0x96480001;
static const uint32_t _FoldMapShift = 24;


static const uint32_t _FoldMapExcludingUpperLowerSeed = 0x96480001;
static const uint32_t _FoldMapExcludingUpperLowerShift = 24;


extern const foldPair _CaseFolds[8192];
extern const uint32_t _UpperLower[8192][2];
extern const uint16_t _FoldMap[256][4];
extern const _FoldMapExcludingUpperLowerItem _FoldMapExcludingUpperLower[256];
#ifdef __cplusplus
extern "C" {
#endif

foldPair getCaseFold(int i);
const uint16_t * getFoldMap(int i);

#ifdef __cplusplus
}
#endif
