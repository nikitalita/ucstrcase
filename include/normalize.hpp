#include <cuchar>
#include <vector>
#include <string>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <new>
#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>
#include "decode.h"
#include "compose.h"
#include "lookup.h"
#include "normalize.h"
#include <stdint.h>			// for mingw

#include <string_view>

namespace ucstrcase {
struct DecompV {
    uint8_t cclass;
    char32_t ch;
};


template<typename From, DecompositionType kind>
void _decomp_iter_push_back(void* decompiter, uint32_t c);

//forward declare Recompositions
template<typename From, DecompositionType kind>
class Recompositions;

template<typename From, DecompositionType kind>
class Decompositions
{
    //declare _decomp_iter_push_back as friend
    friend void _decomp_iter_push_back<From, kind>(void* decompiter, uint32_t c);

		//declare Recompositions of the same template type as a friend
		friend class Recompositions<From, kind>;
    std::basic_string_view<From> str;
    size_t pos;
    std::vector<DecompV> buffer;
    uint32_t ready_start;
    uint32_t ready_end;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = char32_t;
    using difference_type   = ptrdiff_t;
    using pointer           = From*;
    using reference         = From&;

    Decompositions(const From* str, size_t len)
        : str(str, len), pos(0), ready_start(0), ready_end(0)
    {
        buffer.reserve(4);
    }
    Decompositions(const std::basic_string_view<From>& str)
        : str(str), pos(0), ready_start(0), ready_end(0)
    {
        buffer.reserve(4);
    }

    inline DecompV next_cc();
    inline void reset();
    inline void sort_pending();

    inline char32_t next()
    {
        return next_cc().ch;
    }

		inline size_t next_utf8(std::string &buf){
			char b[4];
			size_t sz = next_utf8(b);
			buf.append(b, sz);
			return sz;
		}
    template<typename F = From>
    inline typename std::enable_if<std::is_same<F, char>::value, char32_t>::type str_next()
    {
        auto runeResult = DecodeRuneInString(str.data() + pos, str.size() - pos);
        pos += runeResult.size;
        return runeResult.rune;
    }
		template<typename F = From>
		inline typename std::enable_if<std::is_same<F, char16_t>::value, char32_t>::type str_next()
		{
			auto runeResult = DecodeRuneInUTF16String((str.data() + pos), (str.size() - pos));
			pos += runeResult.size;
			return runeResult.rune;
		}

    template<typename F = From>
		inline typename std::enable_if<std::is_same<F, char32_t>::value, char32_t>::type str_next()
    {
        return str[pos++];
    }

		inline std::string to_string();
		inline const char * to_cstring();
		inline std::u32string to_utf32_string();

private:
		inline size_t next_utf8(char* buf)
		{
			return char_utf32_to_utf8(next(), buf);
		}
    inline size_t len(){
        return str.size();
    }
		inline size_t est_remaining() {
			return str.size() - pos + buffer.size();
		}
    inline void case_fold(char32_t ch);

    inline void decompose(char32_t ch);
    inline void increment_next_ready();
    inline void push_back(char32_t ch);
    inline void reset_buffer();
};

template <typename From, DecompositionType kind>
inline void Decompositions<From, kind>::reset()
{
		pos = 0;
		ready_start = 0;
		ready_end = 0;
		buffer.clear();
}
template <typename From, DecompositionType kind>
inline std::u32string Decompositions<From, kind>::to_utf32_string() {
	std::u32string result;
	result.reserve(est_remaining());
	size_t sz = 0;
	char32_t rune = next();
	while (rune != 0) {
		result += rune;
		rune = next();
		sz++;
	}
	result.resize(result.size());
	return result;
}

template <typename From, DecompositionType kind>
inline std::string Decompositions<From, kind>::to_string()
{
	std::string result;
	size_t reserved = est_remaining() * (sizeof(char32_t) / sizeof(From)) + 1;
	result.resize(reserved, 0);
	size_t sz = next_utf8(result.data());
	while (result[sz-1] != 0) {
		if (sz +4 >= result.size()) {
			reserved += reserved/2;
			result.resize(reserved, 0);
		}
		sz += next_utf8(result.data() + sz);
	}
	result.resize(sz);
	return result;
}
template <typename From, DecompositionType kind>
inline const char * Decompositions<From, kind>::to_cstring()
{
	// TODO: optimize
	auto res = to_string();
	return strdup(res.c_str());
}

template<typename From, DecompositionType kind>
void Decompositions<From, kind>::case_fold(char32_t ch){
    // NOT IMPLEMENTED!
    const uint16_t *fold = getFullCaseFold(ch);
	if (!fold){
			decompose(ch);
			return;
	}
	int i = 0;
	uint16_t fold_ch = fold[i];
	while (fold_ch != 0 && i < 3) {
		decompose(fold_ch);
		fold_ch = fold[++i];
	}

}

template<typename From, DecompositionType kind>
DecompV Decompositions<From, kind>::next_cc()
{
	while (ready_end == 0) {
		if (pos < len()) {
			char32_t result = str_next();
			// end of string
			if (result == 0){
				sort_pending();
				ready_end = buffer.size();
				break;
			}
			if (kind == CanonicalCaseFold || kind == CompatibleCaseFold) {
				case_fold(result);
			} else {
				decompose(result);
			}
		} else {
			if (buffer.size() == 0) {
				return (DecompV){0,0};
			} else {
				sort_pending();
				ready_end = buffer.size();
				break;
			}
		}
	}
	DecompV ch = buffer[ready_start];
	increment_next_ready();
	return ch;
};


template<typename From, DecompositionType kind>
void Decompositions<From, kind>::sort_pending(){
    // sort the buffer
		if (ready_end == buffer.size()) {
			return;
		}
		auto start = buffer.begin() + ready_end;
		auto end = buffer.end();
    std::sort(start, end, [](const DecompV& a, const DecompV& b){
        return a.cclass < b.cclass;
    });
}

template<typename From, DecompositionType kind>
inline
void _decomp_iter_push_back(void* decompiter, uint32_t c){
    ((Decompositions<From, kind>*)decompiter)->push_back(c);
}

template<typename From, DecompositionType kind>
void Decompositions<From, kind>::decompose(char32_t ch){
    ::decompose(ch, _decomp_iter_push_back<From, kind>, this, kind);
}

template<typename From, DecompositionType kind>
void Decompositions<From, kind>::push_back(char32_t ch){
  uint8_t cls = canonical_combining_class(ch);
  if (cls == 0) {
    sort_pending();
    buffer.push_back((DecompV){cls, ch});
    ready_end = buffer.size();
  } else {
    buffer.push_back((DecompV){cls, ch});
  }
}

// reset_buffer
template<typename From, DecompositionType kind>
void Decompositions<From, kind>::reset_buffer(){
    buffer.erase(buffer.begin(), buffer.begin() + ready_start);
    ready_start = ready_end = 0;
}

// increment_next_ready

template<typename From, DecompositionType kind>
void Decompositions<From, kind>::increment_next_ready(){
    ready_start++;
    if (ready_start == ready_end) {
        reset_buffer();
    }
}



template<typename From, DecompositionType kind>
class Recompositions
{
	enum RecompositionState : uint8_t {
		Composing,
		Purging,
		Finished,
	};

	//declare _decomp_iter_push_back as friend
	Decompositions<From, kind> iter;
	std::vector<DecompV> buffer;
	char32_t composee;
	uint32_t state_next;
	RecompositionState state;
	uint8_t last_cc;

public:
	using iterator_category = std::forward_iterator_tag;
	using value_type        = char32_t;
	using difference_type   = ptrdiff_t;
	using pointer           = From*;
	using reference         = From&;
	constexpr static uint8_t NO_CCC = 0xff;

	Recompositions(const From* str, size_t len)
					: iter(str, len), state(Composing), state_next(0), last_cc(NO_CCC), composee(0)
	{
		buffer.reserve(4);
	}
	Recompositions(const std::basic_string_view<From>& str)
					: iter(str), state(Composing), state_next(0), last_cc(NO_CCC), composee(0)
	{
		buffer.reserve(4);
	}

//	inline DecompV next_cc();
	inline void reset();

	inline char32_t next();

	inline size_t next_utf8(std::string &buf){
		char b[4]{0,0,0,0};
		size_t sz = next_utf8(b);
		buf.append(b, sz);
		return sz;
	}

	inline std::string to_string();
	inline const char * to_cstring();
	inline std::u32string to_utf32_string();

private:
	inline size_t next_utf8(char* buf)
	{
		return char_utf32_to_utf8(next(), buf);
	}
	inline char32_t ret_char(char32_t ch) {
		// TODO: Something?
		return ch;
	}
//	inline DecompV ret_dv(DecompV chc) {
//		// TODO: Something?
//		return chc;
//	}

	inline char32_t take_composee() {
		char32_t ch = composee;
		composee = 0;
		return ch;
	}

	inline size_t len(){
		return iter.len();
	}
	inline size_t est_remaining(){
		return iter.est_remaining() + buffer.size() + (composee != 0 ? 1 : 0);
	}
	inline void set_state(RecompositionState s, uint32_t next){
		state = s;
		state_next = next;
	}
};

//std::string to_string();
template<typename From, DecompositionType kind>
inline std::string Recompositions<From, kind>::to_string()
{
	std::string result;
	size_t reserved = est_remaining() * (sizeof(char32_t) / sizeof(From)) + 1;
	result.resize(reserved, 0);
//	result.reserve(reserved);
	size_t sz = next_utf8(result.data());
	while (result[sz-1] != 0) {
		if (sz +4 >= result.size()) {
			reserved += reserved/2;
			result.resize(reserved, 0);
		}
		sz += next_utf8(result.data() + sz);
	}
	result.resize(sz-1);
	return result;
}

template<typename From, DecompositionType kind>
inline const char * Recompositions<From, kind>::to_cstring()
{
	auto res = to_string();
	return strdup(res.c_str());
}

template<typename From, DecompositionType kind>
inline std::u32string Recompositions<From, kind>::to_utf32_string() {
	std::u32string result;
	result.reserve(est_remaining());
	char32_t rune = next();
	while (rune != 0) {
		result += rune;
		rune = next();
	}
	result.resize(result.size());
	return result;
}

template<typename From, DecompositionType kind>
void Recompositions<From, kind>::reset()
{
	iter.reset();
	state = Composing;
	state_next = 0;
	last_cc = 0;
	composee = 0;
	buffer.clear();
}
//next_cc
template<typename From, DecompositionType kind>
char32_t Recompositions<From, kind>::next() {
	while (1) {
		switch (state) {
			case Composing: {
				for (DecompV chc = iter.next_cc(); chc.ch != 0; chc = iter.next_cc()) {
					char32_t ch = chc.ch;
					uint8_t ch_class = chc.cclass;
					char32_t k = composee;
					if (k == 0) {
						if (ch_class != 0) {
							return ret_char(ch);
						}
						composee = ch;
						continue;
					}
					uint8_t l_class = last_cc;
					if (l_class == NO_CCC) {
						char32_t r = compose(k, ch);
						if (r != 0) {
							composee = r;
							continue;
						}
						if (ch_class == 0) {
							composee = ch;
							return ret_char(k);
						}
						buffer.push_back((DecompV) {ch_class, ch});
						last_cc = ch_class;
					} else {
						if (l_class >= ch_class
								|| (ch_class == 0 && composee != 0)) {
							if (ch_class == 0) {
								composee = ch;
								last_cc = NO_CCC;
								set_state(Purging, 0);
								return ret_char(k);
							}
							buffer.push_back((DecompV) {ch_class, ch});
							last_cc = ch_class;
							continue;
						}
						char32_t r = compose(k, ch);
						if (r != 0) {
							composee = r;
							continue;
						}
						buffer.push_back((DecompV) {ch_class, ch});
						last_cc = ch_class;
					}
				}
				// end of decomposition
				set_state(Finished, 0);
				if (composee != 0) {
					return take_composee();
				}
			}
				break;
			case Purging: {
				char32_t next_char = buffer.size() > state_next ? buffer[state_next].ch : 0;
				if (next_char == 0) {
					buffer.clear();
					set_state(Composing, 0);
				} else {
					state_next++;
					return ret_char(next_char);
				}
			}
				break;
			case Finished: {
				char32_t next_char = buffer.size() > state_next ? buffer[state_next].ch : 0;
				if (next_char == 0) {
					buffer.clear();
					return take_composee();
				} else {
					state_next++;
					return ret_char(next_char);
				}
			} break;
			default:
				assert(0 && "Invalid state");
				break;
		}
	}
	assert(0 && "Unreachable!");
}


} // namespace ucstrcase
