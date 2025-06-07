#ifndef BITBOARD_UTILS_H_INCLUDE
#define BITBOARD_UTILS_H_INCLUDE

#include "board.h"

#define AFILE 0x0101010101010101
#define HFILE 0x8080808080808080

#define nAFILE ~AFILE
#define nHFILE ~HFILE

/* Unused, for future SIMD code 
example function that used SIMD:

template <typename T>

inline north_shift(T bb) {
    return (bb << 8);
}

the function would be able to be used with both scalar and SIMD types.

BB bb = ...
BB bb2 = ...

BB scalar_result = north_shift(bb);
XMM xmm_result = north_shift(XMM(bb2, bb2));

*/

#include <nmmintrin.h>

class XMM {
public:
    __m128i x;
    XMM() : x(_mm_setzero_si128()) {}
    XMM(BB bb) : x(_mm_cvtsi64_si128(bb)) {}
    XMM(BB hi, BB lo) : x(_mm_set_epi64x(hi, lo)) {}
    XMM(__m128i a) : x(a) {}

    XMM& operator >>= (int sh) { x = _mm_srli_epi64(x, sh); return *this; }
    XMM& operator <<= (int sh) { x = _mm_slli_epi64(x, sh); return *this; }

    XMM& operator &= (const XMM& a) { x = _mm_and_si128(x, a.x); return *this; }
    XMM& operator |= (const XMM& a) { x = _mm_or_si128(x, a.x);  return *this; }
    XMM& operator ^= (const XMM& a) { x = _mm_xor_si128(x, a.x); return *this; }

    friend XMM operator >> (const XMM& a, int sh) { return XMM(_mm_srli_epi64(a.x, sh)); }
    friend XMM operator << (const XMM& a, int sh) { return XMM(_mm_slli_epi64(a.x, sh)); }
    friend XMM operator & (const XMM& a, const XMM& b) { return XMM(_mm_and_si128(a.x, b.x)); }
    friend XMM operator | (const XMM& a, const XMM& b) { return XMM(_mm_or_si128(a.x, b.x)); }
    friend XMM operator ^ (const XMM& a, const XMM& b) { return XMM(_mm_xor_si128(a.x, b.x)); }

    BB lo() const { return _mm_cvtsi128_si64(x); }
    BB hi() const { return _mm_extract_epi64(x, 1); }

    XMM swapp() const {
        return XMM(_mm_shuffle_epi32(x, _MM_SHUFFLE(1, 0, 3, 2)));
    }
};

#endif