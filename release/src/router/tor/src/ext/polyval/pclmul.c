/*
 * Copyright (c) 2017 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * This is the GHASH implementation that leverages the pclmulqdq opcode
 * (from the AES-NI instructions).
 */

#include <wmmintrin.h>

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define BR_TARGET(x) __attribute__((target(x)))

#if defined(__GNUC__) && !defined(__clang__)
        _Pragma("GCC target(\"sse2,ssse3,sse4.1,aes,pclmul\")")
#endif

#if 0
/*
 * Test CPU support for PCLMULQDQ.
 */
static inline int
pclmul_supported(void)
{
	/*
	 * Bit mask for features in ECX:
	 *    1   PCLMULQDQ support
	 */
	return br_cpuid(0, 0, 0x00000002, 0);
}

/* see bearssl_hash.h */
br_ghash
br_ghash_pclmul_get(void)
{
	return pclmul_supported() ? &br_ghash_pclmul : 0;
}

BR_TARGETS_X86_UP
#endif
/*
 * Call pclmulqdq. Clang appears to have trouble with the intrinsic, so,
 * for that compiler, we use inline assembly. Inline assembly is
 * potentially a bit slower because the compiler does not understand
 * what the opcode does, and thus cannot optimize instruction
 * scheduling.
 *
 * We use a target of "sse2" only, so that Clang may still handle the
 * '__m128i' type and allocate SSE2 registers.
 */
#ifdef __clang__AND_NOT_WORKING
 BR_TARGET("sse2")
static inline __m128i
pclmulqdq00(__m128i x, __m128i y)
{
	__asm__ ("pclmulqdq $0x00, %1, %0" : "+x" (x) : "x" (y));
	return x;
}
BR_TARGET("sse2")
static inline __m128i
pclmulqdq11(__m128i x, __m128i y)
{
	__asm__ ("pclmulqdq $0x11, %1, %0" : "+x" (x) : "x" (y));
	return x;
}
#else
#define pclmulqdq00(x, y)   _mm_clmulepi64_si128(x, y, 0x00)
#define pclmulqdq11(x, y)   _mm_clmulepi64_si128(x, y, 0x11)
#endif

/*
 * From a 128-bit value kw, compute kx as the XOR of the two 64-bit
 * halves of kw (into the right half of kx; left half is unspecified).
 */
#define BK(kw, kx)   do { \
		kx = _mm_xor_si128(kw, _mm_shuffle_epi32(kw, 0x0E)); \
	} while (0)

/*
 * Combine two 64-bit values (k0:k1) into a 128-bit (kw) value and
 * the XOR of the two values (kx).
 */
#define PBK(k0, k1, kw, kx)   do { \
		kw = _mm_unpacklo_epi64(k1, k0); \
		kx = _mm_xor_si128(k0, k1); \
	} while (0)

/*
 * Perform reduction in GF(2^128). The 256-bit value is in x0..x3;
 * result is written in x0..x1.
 */
#define REDUCE_F128(x0, x1, x2, x3)   do { \
		x1 = _mm_xor_si128( \
			x1, \
			_mm_xor_si128( \
				_mm_xor_si128( \
					x3, \
					_mm_srli_epi64(x3, 1)), \
				_mm_xor_si128( \
					_mm_srli_epi64(x3, 2), \
					_mm_srli_epi64(x3, 7)))); \
		x2 = _mm_xor_si128( \
			_mm_xor_si128( \
				x2, \
				_mm_slli_epi64(x3, 63)), \
			_mm_xor_si128( \
				_mm_slli_epi64(x3, 62), \
				_mm_slli_epi64(x3, 57))); \
		x0 = _mm_xor_si128( \
			x0, \
			_mm_xor_si128( \
				_mm_xor_si128( \
					x2, \
					_mm_srli_epi64(x2, 1)), \
				_mm_xor_si128( \
					_mm_srli_epi64(x2, 2), \
					_mm_srli_epi64(x2, 7)))); \
		x1 = _mm_xor_si128( \
			_mm_xor_si128( \
				x1, \
				_mm_slli_epi64(x2, 63)), \
			_mm_xor_si128( \
				_mm_slli_epi64(x2, 62), \
				_mm_slli_epi64(x2, 57))); \
	} while (0)


BR_TARGET("ssse3,pclmul")
static inline void
expand_key_pclmul(const polyval_t *pv, pv_expanded_key_t *out)
{
	__m128i h1w, h1x;
	__m128i lastw, lastx;
	__m128i t0, t1, t2, t3;

	h1w = PCLMUL_MEMBER(pv->key.h);
        BK(h1w, h1x);
        lastw = h1w;

	for (int i = PV_BLOCK_STRIDE - 2; i >= 0; --i) {
		BK(lastw, lastx);

		t1 = pclmulqdq11(lastw, h1w);
		t3 = pclmulqdq00(lastw, h1w);
		t2 = pclmulqdq00(lastx, h1x);
		t2 = _mm_xor_si128(t2, _mm_xor_si128(t1, t3));
		t0 = _mm_shuffle_epi32(t1, 0x0E);
		t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
		t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));
		REDUCE_F128(t0, t1, t2, t3);
		out->k[i] = lastw = _mm_unpacklo_epi64(t1, t0);
	}
}

// Add PCLMUL_BLOCK_STRIDE * 16 bytes from input.
BR_TARGET("ssse3,pclmul")
static inline void
pv_add_multiple_pclmul(polyval_t *pv,
		       const uint8_t *input,
		       const pv_expanded_key_t *expanded)
{
	__m128i t0, t1, t2, t3;

	t1 = _mm_setzero_si128();
	t2 = _mm_setzero_si128();
	t3 = _mm_setzero_si128();

        for (int i = 0; i < PV_BLOCK_STRIDE; ++i, input += 16) {
		__m128i aw = _mm_loadu_si128((void *)(input));
		__m128i ax;
		__m128i hx, hw;
		if (i == 0) {
			aw = _mm_xor_si128(aw, PCLMUL_MEMBER(pv->y));
		}
		if (i == PV_BLOCK_STRIDE - 1) {
			hw = PCLMUL_MEMBER(pv->key.h);
		} else {
			hw = expanded->k[i];
		}
		BK(aw, ax);
		BK(hw, hx);
		t1 = _mm_xor_si128(t1, pclmulqdq11(aw, hw));
		t3 = _mm_xor_si128(t3, pclmulqdq00(aw, hw));
		t2 = _mm_xor_si128(t2, pclmulqdq00(ax, hx));
	}

	t2 = _mm_xor_si128(t2, _mm_xor_si128(t1, t3));
	t0 = _mm_shuffle_epi32(t1, 0x0E);
	t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
	t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));

	REDUCE_F128(t0, t1, t2, t3);
	PCLMUL_MEMBER(pv->y) = _mm_unpacklo_epi64(t1, t0);
}


/* see bearssl_hash.h */
BR_TARGET("ssse3,pclmul")
static inline void
pv_mul_y_h_pclmul(polyval_t *pv)
{
	__m128i yw, h1w, h1x;

        h1w = PCLMUL_MEMBER(pv->key.h);
        BK(h1w, h1x);

        {
		__m128i aw, ax;
		__m128i t0, t1, t2, t3;

                aw = PCLMUL_MEMBER(pv->y);
		BK(aw, ax);

		t1 = pclmulqdq11(aw, h1w);
		t3 = pclmulqdq00(aw, h1w);
		t2 = pclmulqdq00(ax, h1x);
		t2 = _mm_xor_si128(t2, _mm_xor_si128(t1, t3));
		t0 = _mm_shuffle_epi32(t1, 0x0E);
		t1 = _mm_xor_si128(t1, _mm_shuffle_epi32(t2, 0x0E));
		t2 = _mm_xor_si128(t2, _mm_shuffle_epi32(t3, 0x0E));
#if 0 // This step is GHASH-only.
		SL_256(t0, t1, t2, t3);
#endif
		REDUCE_F128(t0, t1, t2, t3);
		yw = _mm_unpacklo_epi64(t1, t0);
	}

	PCLMUL_MEMBER(pv->y) = yw;
}
