# ucstrcase

A C library for fast case-insensitive comparison of UTF-8 strings using perfect hashing.
Adapted from [strcase](https://github.com/charlievieth/strcase) by Charlie Vieth.


## Performance

5x-10x faster than `icu::UnicodeString::caseCompare`.
Often faster than `strcasecmp` (which does not handle unicode strings), and worst-case only 50% slower than `strcasecmp` for UTF-8 strings.

```
**** Generated UTF-8 strings *****
ucstrcasecmp time : 225ms
strcasecmp time : 368ms
icu::UnicodeString::caseCompare time : 3604ms
**** Generated ASCII strings *****
ucstrcasecmp time : 194ms
strcasecmp time : 360ms
icu::UnicodeString::caseCompare time : 4486ms
**** Generated worst-case matching UTF-8 strings *****
ucstrcasecmp time : 3069ms
strcasecmp time : 1988ms
icu::UnicodeString::caseCompare time : 12730ms
```

## Usage

You can use the files in the `ports` directory to install this package with vcpkg.
