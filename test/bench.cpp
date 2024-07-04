
#include <unicode/stringoptions.h>
#include <stdio.h>
#include <string.h>
#include <ucstrcase.h>
#include <string>
#include <iostream>
#include <vector>
#define U_SHOW_CPLUSPLUS_API 1
#include <unicode/unistr.h>
#include <unicode/uchar.h>
#include "data.h"
#define utf8_probability 3 // 3-percent chance of generating a UTF-8 string
#define utf8_char_probability 2 // 2-percent chance of generating a UTF-8 character
#define utf8_invalid_probability 20 // 1-percent chance of generating an invalid UTF-8 character
#define TABLE_SIZE 1000000
#define TABLE_TRAVERSALS 10
#define STRING_SIZE 30
#define locb 0x80
#define hicb 0xBF

typedef struct {
    unsigned char lo; // lowest value for second byte.
    unsigned char hi; // highest value for second byte.
} acceptRange;

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
#define u_xx 0xF1 // invalid: size 1
#define u_as 0xF0 // ASCII: size 1
#define u_s1 0x02 // accept 0, size 2
#define u_s2 0x13 // accept 1, size 3
#define u_s3 0x03 // accept 0, size 3
#define u_s4 0x23 // accept 2, size 3
#define u_s5 0x34 // accept 3, size 4
#define u_s6 0x04 // accept 0, size 4
#define u_s7 0x44 // accept 4, size 4

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
#include <chrono>

int64_t getCurrentMillis()
{
    using std::chrono::system_clock;
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Convert duration to milliseconds
    auto milliseconds
        = std::chrono::duration_cast<std::chrono::milliseconds>(
              duration)
              .count();
    
    return milliseconds;
}
acceptRange getAcceptRange(uint8_t firstByte) {
    return bacceptRanges[first[firstByte]];
}

char* generate_random_utf8_string(int length) {
    if (length <= 0) return NULL;

    // Allocate memory for UTF-8 string including null terminator
    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (!str) return NULL;
    
    for (int i = 0; i < length; ++i) {
        int rnd = rand() % 100; // Determine the length of the next character (1 to 4 bytes)
        if (rnd < 100 - utf8_char_probability || i +3  >= length) { // 1-byte character
            str[i] = (char)((rand() % 0x7F) + 1); // 0x01-0x7F
        } else{
          // now randomly generate a 2, 3, or 4 byte character
          rnd = rand() % 3 + 2;
          uint8_t firstByteHi = 0;
          uint8_t firstByteLo = 0;
           if (rnd == 2) { // 2-byte character
              if (i + 1 >= length) break; // Ensure there's room for 2 bytes
              firstByteHi = 0xC2;
              firstByteLo = 0xDF;
          } else if (rnd == 3) { // 3-byte character
              if (i + 2 >= length) break; // Ensure there's room for 3 bytes
              firstByteHi = 0xE0;
              firstByteLo = 0xEF;
          } else { // 4-byte character
              if (i + 3 >= length) break; // Ensure there's room for 4 bytes
              firstByteHi = 0xF0;
              firstByteLo = 0xF4;
          }
          uint8_t firstByte = (char)((rand() % (firstByteLo - firstByteHi)) + firstByteHi);

          while (getAcceptRange(firstByte).hi == 0) {
              firstByte = (char)((rand() % (firstByteLo - firstByteHi)) + firstByteHi);
          }
          auto range = getAcceptRange(firstByte);
          str[i++] = firstByte;
          str[i++] = (char)(((rand() % (range.hi - range.lo)) + range.lo) & range.hi); // 0x80-0xBF
          for (int j = 0; j < rnd - 2; ++j) {
             auto chr = (char)((rand() % (hicb - locb)) + locb);
              str[i++] = chr; // 0x80-0xBF
          }

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

enum testType{
	UCSTRCASECMP = 0,
	UCSTRCASECMP_N,
	STRCASECMP,
  STRNCASECMP,
  ICU_CASE_CMP,
  ICU_CASE_CMP_N,
  MAX
};

const std::string funcNames[testType::MAX]= {
	{"ucstrcasecmp"},
	{"ucstrncasecmp"},
	{"strcasecmp"},
  {"strncasecmp"},
  {"icu::UnicodeString::caseCompare"},
  {"icu::UnicodeString::caseCompareBetween"},
};


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
  const size_t table_mod = table_len;
	const auto inc = i_plus_1_should_match ? 2 : 1;
	const size_t iters = TABLE_SIZE * TABLE_TRAVERSALS * inc;

	// if string_length == 0, do the non-ntypes (even indexes)
	if (filters.empty())
		filters = {UCSTRCASECMP, UCSTRCASECMP_N, STRCASECMP, STRNCASECMP, ICU_CASE_CMP, ICU_CASE_CMP_N};
  for (int i = 0; i < filters.size(); i++){
    testType type = (testType)filters[i];
    if (string_length != 0 && (!isNType(type))){
      continue;
    } else if (string_length == 0 && isNType(type)){
      continue;
    }
    start = getCurrentMillis();
    int errors = 0;
    for (size_t i = 0; i < iters; i+=inc) {
      // worst case; string that matches
      const char* a = s[i%table_mod];
      auto alen = string_length ? string_length : strlen(a);
      auto result = run_test((testType)type, a, a, string_length, i_plus_1_should_match);
      if (!no_check_match && result != 0) {
        std::cout << "Error: " << funcNames[type] << ": no match for matching strings!!" << std::endl;
        errors++;
      }
      const char* b = s[(i+1)%table_mod];
      auto blen = string_length ? string_length : strlen(b);
      result = run_test((testType)type, a, b, string_length, i_plus_1_should_match);
			if (!no_check_match && !i_plus_1_should_match && result == 0) {
        std::cout << "Error: " << funcNames[type] << ": match for different strings!!" << std::endl;
        errors++;
      }
      if (errors > MAX_ERRORS){
        std::cout << "Too many errors, stopping test" << std::endl;
        break;
      }
    }
    stop = getCurrentMillis();
    printf("%s time : %lldms\n", funcNames[type].c_str(), stop - start);
  }
}

void test_icu(){
  const char * atest = "b";
  const char * btest = "B";
  // get the rune
  auto a_rune = DecodeRuneInString(atest, strlen(atest));
  auto b_rune = DecodeRuneInString(btest, strlen(btest));


  const char * test1 = "Ḩ";
  const char * test2 = "ḩ";
  const char * cyrillic1 = "ƂⰩⅯƂⰨⅮƁⰧⅭɃⰦⅬSⰥⅫŽⰤⅪŽⰣⅩŻⰢⅨŻⰡⅧŹⰠⅦŹⰟⅥŸⰞⅤŶⰝⅣŶⰜⅢŴⰛⅡŴⰚⅠŲⰙⅯŲⰘⅮŰⰗⅭŰⰖⅬŮⰕⅫŮⰔⅪŬⰓⅩŬⰒⅨŪⰑⅧŪⰐⅦŨⰏⅥŨⰎⅤŦⰍⅣŦⰌⅢŤⰋⅡŤⰊⅠŢⰉ";
  // the above, but with a different case
  const char * cyrillic2 = "ƃⱙⅿƃⱘⅾɓⱗⅽƀⱖⅼſⱕⅻžⱔⅺžⱓⅹżⱒⅸżⱑⅷźⱐⅶźⱏⅵÿⱎⅴŷⱍⅳŷⱌⅲŵⱋⅱŵⱊⅰųⱉⅿųⱈⅾűⱇⅽűⱆⅼůⱅⅻůⱄⅺŭⱃⅹŭⱂⅸūⱁⅷūⱀⅶũⰿⅵũⰾⅴŧⰽⅳŧⰼⅲťⰻⅱťⰺⅰţⰹ";

  icu::UnicodeString aicustr(cyrillic1);
  icu::UnicodeString bicustr(cyrillic2);
  auto result = aicustr.caseCompare(bicustr, U_FOLD_CASE_DEFAULT);
  if (result != 0){
    std::cout << "Error: icu::UnicodeString: no match for matching strings!!" << std::endl;
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
int main() {
//  test_icu();
  // return 0;
  



  // generate a random utf-8 string

	 generateRandomMatchingStrings(s, TABLE_SIZE, STRING_SIZE);
	 printf("**** Generated Matching UTF-8 strings *****\n");
	 run_tests(s, true, TABLE_SIZE);

  generateRandomMatchingStrings2(s, TABLE_SIZE, STRING_SIZE);
  printf("**** Generated Tricky matching UTF-8 strings *****\n");
//  run_tests(s, true, TABLE_SIZE, 0, false, { UCSTRCASECMP});
	run_tests(s, true, TABLE_SIZE);

   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_utf8_string(STRING_SIZE);
   }
   printf("**** Generated UTF-8 strings *****\n");
   run_tests(s);

   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_ascii_string(STRING_SIZE);
   }
   printf("**** Generated ASCII strings *****\n");
   run_tests(s);

   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_invalid_utf8_string(STRING_SIZE);
   }

   printf("**** Generated INVALID UTF-8 strings *****\n");
   run_tests(s, false, TABLE_SIZE, STRING_SIZE, true);

   for (int i = 0; i < TABLE_SIZE; i++) {
     s[i] = generate_random_garbage(STRING_SIZE);
   }

   printf("**** Generated GARBAGE strings *****\n");
   run_tests(s, false, TABLE_SIZE, STRING_SIZE, true);


  return 0;
}
