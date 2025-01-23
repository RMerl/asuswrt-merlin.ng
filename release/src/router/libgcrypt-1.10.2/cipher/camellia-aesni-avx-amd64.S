/* camellia-avx-aesni-amd64.S  -  AES-NI/AVX implementation of Camellia cipher
 *
 * Copyright (C) 2013-2015,2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#ifdef __x86_64
#if (defined(HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS) || \
     defined(HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)) && \
    defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)

#include "asm-common-amd64.h"

#define CAMELLIA_TABLE_BYTE_LEN 272

/* struct CAMELLIA_context: */
#define key_table 0
#define key_bitlength CAMELLIA_TABLE_BYTE_LEN

/* register macros */
#define CTX %rdi

/**********************************************************************
  helper macros
 **********************************************************************/
#define filter_8bit(x, lo_t, hi_t, mask4bit, tmp0) \
	vpand x, mask4bit, tmp0; \
	vpandn x, mask4bit, x; \
	vpsrld $4, x, x; \
	\
	vpshufb tmp0, lo_t, tmp0; \
	vpshufb x, hi_t, x; \
	vpxor tmp0, x, x;

/**********************************************************************
  16-way camellia
 **********************************************************************/

/*
 * IN:
 *   x0..x7: byte-sliced AB state
 *   mem_cd: register pointer storing CD state
 *   key: index for key material
 * OUT:
 *   x0..x7: new byte-sliced CD state
 */
#define roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, t0, t1, t2, t3, t4, t5, t6, \
		  t7, mem_cd, key) \
	/* \
	 * S-function with AES subbytes \
	 */ \
	vmovdqa .Linv_shift_row rRIP, t4; \
	vbroadcastss .L0f0f0f0f rRIP, t7; \
	vmovdqa .Lpre_tf_lo_s1 rRIP, t0; \
	vmovdqa .Lpre_tf_hi_s1 rRIP, t1; \
	\
	/* AES inverse shift rows */ \
	vpshufb t4, x0, x0; \
	vpshufb t4, x7, x7; \
	vpshufb t4, x1, x1; \
	vpshufb t4, x4, x4; \
	vpshufb t4, x2, x2; \
	vpshufb t4, x5, x5; \
	vpshufb t4, x3, x3; \
	vpshufb t4, x6, x6; \
	\
	/* prefilter sboxes 1, 2 and 3 */ \
	vmovdqa .Lpre_tf_lo_s4 rRIP, t2; \
	vmovdqa .Lpre_tf_hi_s4 rRIP, t3; \
	filter_8bit(x0, t0, t1, t7, t6); \
	filter_8bit(x7, t0, t1, t7, t6); \
	filter_8bit(x1, t0, t1, t7, t6); \
	filter_8bit(x4, t0, t1, t7, t6); \
	filter_8bit(x2, t0, t1, t7, t6); \
	filter_8bit(x5, t0, t1, t7, t6); \
	\
	/* prefilter sbox 4 */ \
	vpxor t4, t4, t4; \
	filter_8bit(x3, t2, t3, t7, t6); \
	filter_8bit(x6, t2, t3, t7, t6); \
	\
	/* AES subbytes + AES shift rows */ \
	vmovdqa .Lpost_tf_lo_s1 rRIP, t0; \
	vmovdqa .Lpost_tf_hi_s1 rRIP, t1; \
	vaesenclast t4, x0, x0; \
	vaesenclast t4, x7, x7; \
	vaesenclast t4, x1, x1; \
	vaesenclast t4, x4, x4; \
	vaesenclast t4, x2, x2; \
	vaesenclast t4, x5, x5; \
	vaesenclast t4, x3, x3; \
	vaesenclast t4, x6, x6; \
	\
	/* postfilter sboxes 1 and 4 */ \
	vmovdqa .Lpost_tf_lo_s3 rRIP, t2; \
	vmovdqa .Lpost_tf_hi_s3 rRIP, t3; \
	filter_8bit(x0, t0, t1, t7, t6); \
	filter_8bit(x7, t0, t1, t7, t6); \
	filter_8bit(x3, t0, t1, t7, t6); \
	filter_8bit(x6, t0, t1, t7, t6); \
	\
	/* postfilter sbox 3 */ \
	vmovdqa .Lpost_tf_lo_s2 rRIP, t4; \
	vmovdqa .Lpost_tf_hi_s2 rRIP, t5; \
	filter_8bit(x2, t2, t3, t7, t6); \
	filter_8bit(x5, t2, t3, t7, t6); \
	\
	vpxor t6, t6, t6; \
	vmovq key, t0; \
	\
	/* postfilter sbox 2 */ \
	filter_8bit(x1, t4, t5, t7, t2); \
	filter_8bit(x4, t4, t5, t7, t2); \
	\
	vpsrldq $5, t0, t5; \
	vpsrldq $1, t0, t1; \
	vpsrldq $2, t0, t2; \
	vpsrldq $3, t0, t3; \
	vpsrldq $4, t0, t4; \
	vpshufb t6, t0, t0; \
	vpshufb t6, t1, t1; \
	vpshufb t6, t2, t2; \
	vpshufb t6, t3, t3; \
	vpshufb t6, t4, t4; \
	vpsrldq $2, t5, t7; \
	vpshufb t6, t7, t7; \
	\
	/* P-function */ \
	vpxor x5, x0, x0; \
	vpxor x6, x1, x1; \
	vpxor x7, x2, x2; \
	vpxor x4, x3, x3; \
	\
	vpxor x2, x4, x4; \
	vpxor x3, x5, x5; \
	vpxor x0, x6, x6; \
	vpxor x1, x7, x7; \
	\
	vpxor x7, x0, x0; \
	vpxor x4, x1, x1; \
	vpxor x5, x2, x2; \
	vpxor x6, x3, x3; \
	\
	vpxor x3, x4, x4; \
	vpxor x0, x5, x5; \
	vpxor x1, x6, x6; \
	vpxor x2, x7, x7; /* note: high and low parts swapped */ \
	\
	/* Add key material and result to CD (x becomes new CD) */ \
	\
	vpxor t3, x4, x4; \
	vpxor 0 * 16(mem_cd), x4, x4; \
	\
	vpxor t2, x5, x5; \
	vpxor 1 * 16(mem_cd), x5, x5; \
	\
	vpsrldq $1, t5, t3; \
	vpshufb t6, t5, t5; \
	vpshufb t6, t3, t6; \
	\
	vpxor t1, x6, x6; \
	vpxor 2 * 16(mem_cd), x6, x6; \
	\
	vpxor t0, x7, x7; \
	vpxor 3 * 16(mem_cd), x7, x7; \
	\
	vpxor t7, x0, x0; \
	vpxor 4 * 16(mem_cd), x0, x0; \
	\
	vpxor t6, x1, x1; \
	vpxor 5 * 16(mem_cd), x1, x1; \
	\
	vpxor t5, x2, x2; \
	vpxor 6 * 16(mem_cd), x2, x2; \
	\
	vpxor t4, x3, x3; \
	vpxor 7 * 16(mem_cd), x3, x3;

/*
 * IN/OUT:
 *  x0..x7: byte-sliced AB state preloaded
 *  mem_ab: byte-sliced AB state in memory
 *  mem_cb: byte-sliced CD state in memory
 */
#define two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i, dir, store_ab) \
	roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		  y6, y7, mem_cd, (key_table + (i) * 8)(CTX)); \
	\
	vmovdqu x4, 0 * 16(mem_cd); \
	vmovdqu x5, 1 * 16(mem_cd); \
	vmovdqu x6, 2 * 16(mem_cd); \
	vmovdqu x7, 3 * 16(mem_cd); \
	vmovdqu x0, 4 * 16(mem_cd); \
	vmovdqu x1, 5 * 16(mem_cd); \
	vmovdqu x2, 6 * 16(mem_cd); \
	vmovdqu x3, 7 * 16(mem_cd); \
	\
	roundsm16(x4, x5, x6, x7, x0, x1, x2, x3, y0, y1, y2, y3, y4, y5, \
		  y6, y7, mem_ab, (key_table + ((i) + (dir)) * 8)(CTX)); \
	\
	store_ab(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab);

#define dummy_store(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab) /* do nothing */

#define store_ab_state(x0, x1, x2, x3, x4, x5, x6, x7, mem_ab) \
	/* Store new AB state */ \
	vmovdqu x0, 0 * 16(mem_ab); \
	vmovdqu x1, 1 * 16(mem_ab); \
	vmovdqu x2, 2 * 16(mem_ab); \
	vmovdqu x3, 3 * 16(mem_ab); \
	vmovdqu x4, 4 * 16(mem_ab); \
	vmovdqu x5, 5 * 16(mem_ab); \
	vmovdqu x6, 6 * 16(mem_ab); \
	vmovdqu x7, 7 * 16(mem_ab);

#define enc_rounds16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i) \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 2, 1, store_ab_state); \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 4, 1, store_ab_state); \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 6, 1, dummy_store);

#define dec_rounds16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, i) \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 7, -1, store_ab_state); \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 5, -1, store_ab_state); \
	two_roundsm16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd, (i) + 3, -1, dummy_store);

/*
 * IN:
 *  v0..3: byte-sliced 32-bit integers
 * OUT:
 *  v0..3: (IN <<< 1)
 */
#define rol32_1_16(v0, v1, v2, v3, t0, t1, t2, zero) \
	vpcmpgtb v0, zero, t0; \
	vpaddb v0, v0, v0; \
	vpabsb t0, t0; \
	\
	vpcmpgtb v1, zero, t1; \
	vpaddb v1, v1, v1; \
	vpabsb t1, t1; \
	\
	vpcmpgtb v2, zero, t2; \
	vpaddb v2, v2, v2; \
	vpabsb t2, t2; \
	\
	vpor t0, v1, v1; \
	\
	vpcmpgtb v3, zero, t0; \
	vpaddb v3, v3, v3; \
	vpabsb t0, t0; \
	\
	vpor t1, v2, v2; \
	vpor t2, v3, v3; \
	vpor t0, v0, v0;

/*
 * IN:
 *   r: byte-sliced AB state in memory
 *   l: byte-sliced CD state in memory
 * OUT:
 *   x0..x7: new byte-sliced CD state
 */
#define fls16(l, l0, l1, l2, l3, l4, l5, l6, l7, r, t0, t1, t2, t3, tt0, \
	      tt1, tt2, tt3, kll, klr, krl, krr) \
	/* \
	 * t0 = kll; \
	 * t0 &= ll; \
	 * lr ^= rol32(t0, 1); \
	 */ \
	vpxor tt0, tt0, tt0; \
	vmovd kll, t0; \
	vpshufb tt0, t0, t3; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t2; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t1; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t0; \
	\
	vpand l0, t0, t0; \
	vpand l1, t1, t1; \
	vpand l2, t2, t2; \
	vpand l3, t3, t3; \
	\
	rol32_1_16(t3, t2, t1, t0, tt1, tt2, tt3, tt0); \
	\
	vpxor l4, t0, l4; \
	vmovdqu l4, 4 * 16(l); \
	vpxor l5, t1, l5; \
	vmovdqu l5, 5 * 16(l); \
	vpxor l6, t2, l6; \
	vmovdqu l6, 6 * 16(l); \
	vpxor l7, t3, l7; \
	vmovdqu l7, 7 * 16(l); \
	\
	/* \
	 * t2 = krr; \
	 * t2 |= rr; \
	 * rl ^= t2; \
	 */ \
	\
	vmovd krr, t0; \
	vpshufb tt0, t0, t3; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t2; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t1; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t0; \
	\
	vpor 4 * 16(r), t0, t0; \
	vpor 5 * 16(r), t1, t1; \
	vpor 6 * 16(r), t2, t2; \
	vpor 7 * 16(r), t3, t3; \
	\
	vpxor 0 * 16(r), t0, t0; \
	vpxor 1 * 16(r), t1, t1; \
	vpxor 2 * 16(r), t2, t2; \
	vpxor 3 * 16(r), t3, t3; \
	vmovdqu t0, 0 * 16(r); \
	vmovdqu t1, 1 * 16(r); \
	vmovdqu t2, 2 * 16(r); \
	vmovdqu t3, 3 * 16(r); \
	\
	/* \
	 * t2 = krl; \
	 * t2 &= rl; \
	 * rr ^= rol32(t2, 1); \
	 */ \
	vmovd krl, t0; \
	vpshufb tt0, t0, t3; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t2; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t1; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t0; \
	\
	vpand 0 * 16(r), t0, t0; \
	vpand 1 * 16(r), t1, t1; \
	vpand 2 * 16(r), t2, t2; \
	vpand 3 * 16(r), t3, t3; \
	\
	rol32_1_16(t3, t2, t1, t0, tt1, tt2, tt3, tt0); \
	\
	vpxor 4 * 16(r), t0, t0; \
	vpxor 5 * 16(r), t1, t1; \
	vpxor 6 * 16(r), t2, t2; \
	vpxor 7 * 16(r), t3, t3; \
	vmovdqu t0, 4 * 16(r); \
	vmovdqu t1, 5 * 16(r); \
	vmovdqu t2, 6 * 16(r); \
	vmovdqu t3, 7 * 16(r); \
	\
	/* \
	 * t0 = klr; \
	 * t0 |= lr; \
	 * ll ^= t0; \
	 */ \
	\
	vmovd klr, t0; \
	vpshufb tt0, t0, t3; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t2; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t1; \
	vpsrldq $1, t0, t0; \
	vpshufb tt0, t0, t0; \
	\
	vpor l4, t0, t0; \
	vpor l5, t1, t1; \
	vpor l6, t2, t2; \
	vpor l7, t3, t3; \
	\
	vpxor l0, t0, l0; \
	vmovdqu l0, 0 * 16(l); \
	vpxor l1, t1, l1; \
	vmovdqu l1, 1 * 16(l); \
	vpxor l2, t2, l2; \
	vmovdqu l2, 2 * 16(l); \
	vpxor l3, t3, l3; \
	vmovdqu l3, 3 * 16(l);

#define transpose_4x4(x0, x1, x2, x3, t1, t2) \
	vpunpckhdq x1, x0, t2; \
	vpunpckldq x1, x0, x0; \
	\
	vpunpckldq x3, x2, t1; \
	vpunpckhdq x3, x2, x2; \
	\
	vpunpckhqdq t1, x0, x1; \
	vpunpcklqdq t1, x0, x0; \
	\
	vpunpckhqdq x2, t2, x3; \
	vpunpcklqdq x2, t2, x2;

#define byteslice_16x16b_fast(a0, b0, c0, d0, a1, b1, c1, d1, a2, b2, c2, d2, \
			      a3, b3, c3, d3, st0, st1) \
	vmovdqu d2, st0; \
	vmovdqu d3, st1; \
	transpose_4x4(a0, a1, a2, a3, d2, d3); \
	transpose_4x4(b0, b1, b2, b3, d2, d3); \
	vmovdqu st0, d2; \
	vmovdqu st1, d3; \
	\
	vmovdqu a0, st0; \
	vmovdqu a1, st1; \
	transpose_4x4(c0, c1, c2, c3, a0, a1); \
	transpose_4x4(d0, d1, d2, d3, a0, a1); \
	\
	vmovdqu .Lshufb_16x16b rRIP, a0; \
	vmovdqu st1, a1; \
	vpshufb a0, a2, a2; \
	vpshufb a0, a3, a3; \
	vpshufb a0, b0, b0; \
	vpshufb a0, b1, b1; \
	vpshufb a0, b2, b2; \
	vpshufb a0, b3, b3; \
	vpshufb a0, a1, a1; \
	vpshufb a0, c0, c0; \
	vpshufb a0, c1, c1; \
	vpshufb a0, c2, c2; \
	vpshufb a0, c3, c3; \
	vpshufb a0, d0, d0; \
	vpshufb a0, d1, d1; \
	vpshufb a0, d2, d2; \
	vpshufb a0, d3, d3; \
	vmovdqu d3, st1; \
	vmovdqu st0, d3; \
	vpshufb a0, d3, a0; \
	vmovdqu d2, st0; \
	\
	transpose_4x4(a0, b0, c0, d0, d2, d3); \
	transpose_4x4(a1, b1, c1, d1, d2, d3); \
	vmovdqu st0, d2; \
	vmovdqu st1, d3; \
	\
	vmovdqu b0, st0; \
	vmovdqu b1, st1; \
	transpose_4x4(a2, b2, c2, d2, b0, b1); \
	transpose_4x4(a3, b3, c3, d3, b0, b1); \
	vmovdqu st0, b0; \
	vmovdqu st1, b1; \
	/* does not adjust output bytes inside vectors */

#define transpose_8x8b(a, b, c, d, e, f, g, h, t0, t1, t2, t3, t4) \
	vpunpcklbw a, b, t0; \
	vpunpckhbw a, b, b; \
	\
	vpunpcklbw c, d, t1; \
	vpunpckhbw c, d, d; \
	\
	vpunpcklbw e, f, t2; \
	vpunpckhbw e, f, f; \
	\
	vpunpcklbw g, h, t3; \
	vpunpckhbw g, h, h; \
	\
	vpunpcklwd t0, t1, g; \
	vpunpckhwd t0, t1, t0; \
	\
	vpunpcklwd b, d, t1; \
	vpunpckhwd b, d, e; \
	\
	vpunpcklwd t2, t3, c; \
	vpunpckhwd t2, t3, t2; \
	\
	vpunpcklwd f, h, t3; \
	vpunpckhwd f, h, b; \
	\
	vpunpcklwd e, b, t4; \
	vpunpckhwd e, b, b; \
	\
	vpunpcklwd t1, t3, e; \
	vpunpckhwd t1, t3, f; \
	\
	vmovdqa .Ltranspose_8x8_shuf rRIP, t3; \
	\
	vpunpcklwd g, c, d; \
	vpunpckhwd g, c, c; \
	\
	vpunpcklwd t0, t2, t1; \
	vpunpckhwd t0, t2, h; \
	\
	vpunpckhqdq b, h, a; \
	vpshufb t3, a, a; \
	vpunpcklqdq b, h, b; \
	vpshufb t3, b, b; \
	\
	vpunpckhqdq e, d, g; \
	vpshufb t3, g, g; \
	vpunpcklqdq e, d, h; \
	vpshufb t3, h, h; \
	\
	vpunpckhqdq f, c, e; \
	vpshufb t3, e, e; \
	vpunpcklqdq f, c, f; \
	vpshufb t3, f, f; \
	\
	vpunpckhqdq t4, t1, c; \
	vpshufb t3, c, c; \
	vpunpcklqdq t4, t1, d; \
	vpshufb t3, d, d;

/* load blocks to registers and apply pre-whitening */
#define inpack16_pre(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		     y6, y7, rio, key) \
	vmovq key, x0; \
	vpshufb .Lpack_bswap rRIP, x0, x0; \
	\
	vpxor 0 * 16(rio), x0, y7; \
	vpxor 1 * 16(rio), x0, y6; \
	vpxor 2 * 16(rio), x0, y5; \
	vpxor 3 * 16(rio), x0, y4; \
	vpxor 4 * 16(rio), x0, y3; \
	vpxor 5 * 16(rio), x0, y2; \
	vpxor 6 * 16(rio), x0, y1; \
	vpxor 7 * 16(rio), x0, y0; \
	vpxor 8 * 16(rio), x0, x7; \
	vpxor 9 * 16(rio), x0, x6; \
	vpxor 10 * 16(rio), x0, x5; \
	vpxor 11 * 16(rio), x0, x4; \
	vpxor 12 * 16(rio), x0, x3; \
	vpxor 13 * 16(rio), x0, x2; \
	vpxor 14 * 16(rio), x0, x1; \
	vpxor 15 * 16(rio), x0, x0;

/* byteslice pre-whitened blocks and store to temporary memory */
#define inpack16_post(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		      y6, y7, mem_ab, mem_cd) \
	byteslice_16x16b_fast(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, \
			      y4, y5, y6, y7, (mem_ab), (mem_cd)); \
	\
	vmovdqu x0, 0 * 16(mem_ab); \
	vmovdqu x1, 1 * 16(mem_ab); \
	vmovdqu x2, 2 * 16(mem_ab); \
	vmovdqu x3, 3 * 16(mem_ab); \
	vmovdqu x4, 4 * 16(mem_ab); \
	vmovdqu x5, 5 * 16(mem_ab); \
	vmovdqu x6, 6 * 16(mem_ab); \
	vmovdqu x7, 7 * 16(mem_ab); \
	vmovdqu y0, 0 * 16(mem_cd); \
	vmovdqu y1, 1 * 16(mem_cd); \
	vmovdqu y2, 2 * 16(mem_cd); \
	vmovdqu y3, 3 * 16(mem_cd); \
	vmovdqu y4, 4 * 16(mem_cd); \
	vmovdqu y5, 5 * 16(mem_cd); \
	vmovdqu y6, 6 * 16(mem_cd); \
	vmovdqu y7, 7 * 16(mem_cd);

/* de-byteslice, apply post-whitening and store blocks */
#define outunpack16(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, \
		    y5, y6, y7, key, stack_tmp0, stack_tmp1) \
	byteslice_16x16b_fast(y0, y4, x0, x4, y1, y5, x1, x5, y2, y6, x2, x6, \
			      y3, y7, x3, x7, stack_tmp0, stack_tmp1); \
	\
	vmovdqu x0, stack_tmp0; \
	\
	vmovq key, x0; \
	vpshufb .Lpack_bswap rRIP, x0, x0; \
	\
	vpxor x0, y7, y7; \
	vpxor x0, y6, y6; \
	vpxor x0, y5, y5; \
	vpxor x0, y4, y4; \
	vpxor x0, y3, y3; \
	vpxor x0, y2, y2; \
	vpxor x0, y1, y1; \
	vpxor x0, y0, y0; \
	vpxor x0, x7, x7; \
	vpxor x0, x6, x6; \
	vpxor x0, x5, x5; \
	vpxor x0, x4, x4; \
	vpxor x0, x3, x3; \
	vpxor x0, x2, x2; \
	vpxor x0, x1, x1; \
	vpxor stack_tmp0, x0, x0;

#define write_output(x0, x1, x2, x3, x4, x5, x6, x7, y0, y1, y2, y3, y4, y5, \
		     y6, y7, rio) \
	vmovdqu x0, 0 * 16(rio); \
	vmovdqu x1, 1 * 16(rio); \
	vmovdqu x2, 2 * 16(rio); \
	vmovdqu x3, 3 * 16(rio); \
	vmovdqu x4, 4 * 16(rio); \
	vmovdqu x5, 5 * 16(rio); \
	vmovdqu x6, 6 * 16(rio); \
	vmovdqu x7, 7 * 16(rio); \
	vmovdqu y0, 8 * 16(rio); \
	vmovdqu y1, 9 * 16(rio); \
	vmovdqu y2, 10 * 16(rio); \
	vmovdqu y3, 11 * 16(rio); \
	vmovdqu y4, 12 * 16(rio); \
	vmovdqu y5, 13 * 16(rio); \
	vmovdqu y6, 14 * 16(rio); \
	vmovdqu y7, 15 * 16(rio);

.text
.align 16

#define SHUFB_BYTES(idx) \
	0 + (idx), 4 + (idx), 8 + (idx), 12 + (idx)

.Lshufb_16x16b:
	.byte SHUFB_BYTES(0), SHUFB_BYTES(1), SHUFB_BYTES(2), SHUFB_BYTES(3);

.Lpack_bswap:
	.long 0x00010203
	.long 0x04050607
	.long 0x80808080
	.long 0x80808080

/* For CTR-mode IV byteswap */
.Lbswap128_mask:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

/*
 * pre-SubByte transform
 *
 * pre-lookup for sbox1, sbox2, sbox3:
 *   swap_bitendianness(
 *       isom_map_camellia_to_aes(
 *           camellia_f(
 *               swap_bitendianess(in)
 *           )
 *       )
 *   )
 *
 * (note: '⊕ 0xc5' inside camellia_f())
 */
.Lpre_tf_lo_s1:
	.byte 0x45, 0xe8, 0x40, 0xed, 0x2e, 0x83, 0x2b, 0x86
	.byte 0x4b, 0xe6, 0x4e, 0xe3, 0x20, 0x8d, 0x25, 0x88
.Lpre_tf_hi_s1:
	.byte 0x00, 0x51, 0xf1, 0xa0, 0x8a, 0xdb, 0x7b, 0x2a
	.byte 0x09, 0x58, 0xf8, 0xa9, 0x83, 0xd2, 0x72, 0x23

/*
 * pre-SubByte transform
 *
 * pre-lookup for sbox4:
 *   swap_bitendianness(
 *       isom_map_camellia_to_aes(
 *           camellia_f(
 *               swap_bitendianess(in <<< 1)
 *           )
 *       )
 *   )
 *
 * (note: '⊕ 0xc5' inside camellia_f())
 */
.Lpre_tf_lo_s4:
	.byte 0x45, 0x40, 0x2e, 0x2b, 0x4b, 0x4e, 0x20, 0x25
	.byte 0x14, 0x11, 0x7f, 0x7a, 0x1a, 0x1f, 0x71, 0x74
.Lpre_tf_hi_s4:
	.byte 0x00, 0xf1, 0x8a, 0x7b, 0x09, 0xf8, 0x83, 0x72
	.byte 0xad, 0x5c, 0x27, 0xd6, 0xa4, 0x55, 0x2e, 0xdf

/*
 * post-SubByte transform
 *
 * post-lookup for sbox1, sbox4:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  )
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s1:
	.byte 0x3c, 0xcc, 0xcf, 0x3f, 0x32, 0xc2, 0xc1, 0x31
	.byte 0xdc, 0x2c, 0x2f, 0xdf, 0xd2, 0x22, 0x21, 0xd1
.Lpost_tf_hi_s1:
	.byte 0x00, 0xf9, 0x86, 0x7f, 0xd7, 0x2e, 0x51, 0xa8
	.byte 0xa4, 0x5d, 0x22, 0xdb, 0x73, 0x8a, 0xf5, 0x0c

/*
 * post-SubByte transform
 *
 * post-lookup for sbox2:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  ) <<< 1
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s2:
	.byte 0x78, 0x99, 0x9f, 0x7e, 0x64, 0x85, 0x83, 0x62
	.byte 0xb9, 0x58, 0x5e, 0xbf, 0xa5, 0x44, 0x42, 0xa3
.Lpost_tf_hi_s2:
	.byte 0x00, 0xf3, 0x0d, 0xfe, 0xaf, 0x5c, 0xa2, 0x51
	.byte 0x49, 0xba, 0x44, 0xb7, 0xe6, 0x15, 0xeb, 0x18

/*
 * post-SubByte transform
 *
 * post-lookup for sbox3:
 *  swap_bitendianness(
 *      camellia_h(
 *          isom_map_aes_to_camellia(
 *              swap_bitendianness(
 *                  aes_inverse_affine_transform(in)
 *              )
 *          )
 *      )
 *  ) >>> 1
 *
 * (note: '⊕ 0x6e' inside camellia_h())
 */
.Lpost_tf_lo_s3:
	.byte 0x1e, 0x66, 0xe7, 0x9f, 0x19, 0x61, 0xe0, 0x98
	.byte 0x6e, 0x16, 0x97, 0xef, 0x69, 0x11, 0x90, 0xe8
.Lpost_tf_hi_s3:
	.byte 0x00, 0xfc, 0x43, 0xbf, 0xeb, 0x17, 0xa8, 0x54
	.byte 0x52, 0xae, 0x11, 0xed, 0xb9, 0x45, 0xfa, 0x06

/* For isolating SubBytes from AESENCLAST, inverse shift row */
.Linv_shift_row:
	.byte 0x00, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x0e, 0x0b
	.byte 0x08, 0x05, 0x02, 0x0f, 0x0c, 0x09, 0x06, 0x03

/* shuffle mask for 8x8 byte transpose */
.Ltranspose_8x8_shuf:
	.byte 0, 1, 4, 5, 2, 3, 6, 7, 8+0, 8+1, 8+4, 8+5, 8+2, 8+3, 8+6, 8+7

.align 4
/* 4-bit mask */
.L0f0f0f0f:
	.long 0x0f0f0f0f


.align 8
ELF(.type   __camellia_enc_blk16,@function;)

__camellia_enc_blk16:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rax: temporary storage, 256 bytes
	 *	%r8d: 24 for 16 byte key, 32 for larger
	 *	%xmm0..%xmm15: 16 plaintext blocks
	 * output:
	 *	%xmm0..%xmm15: 16 encrypted blocks, order swapped:
	 *       7, 8, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
	 */
	CFI_STARTPROC();

	leaq 8 * 16(%rax), %rcx;

	leaq (-8 * 8)(CTX, %r8, 8), %r8;

	inpack16_post(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		      %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		      %xmm15, %rax, %rcx);

.align 8
.Lenc_loop:
	enc_rounds16(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		     %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		     %xmm15, %rax, %rcx, 0);

	cmpq %r8, CTX;
	je .Lenc_done;
	leaq (8 * 8)(CTX), CTX;

	fls16(%rax, %xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
	      %rcx, %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
	      %xmm15,
	      ((key_table) + 0)(CTX),
	      ((key_table) + 4)(CTX),
	      ((key_table) + 8)(CTX),
	      ((key_table) + 12)(CTX));
	jmp .Lenc_loop;

.align 8
.Lenc_done:
	/* load CD for output */
	vmovdqu 0 * 16(%rcx), %xmm8;
	vmovdqu 1 * 16(%rcx), %xmm9;
	vmovdqu 2 * 16(%rcx), %xmm10;
	vmovdqu 3 * 16(%rcx), %xmm11;
	vmovdqu 4 * 16(%rcx), %xmm12;
	vmovdqu 5 * 16(%rcx), %xmm13;
	vmovdqu 6 * 16(%rcx), %xmm14;
	vmovdqu 7 * 16(%rcx), %xmm15;

	outunpack16(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		    %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		    %xmm15, ((key_table) + 8 * 8)(%r8), (%rax), 1 * 16(%rax));

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __camellia_enc_blk16,.-__camellia_enc_blk16;)

.align 8
ELF(.type   __camellia_dec_blk16,@function;)

__camellia_dec_blk16:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rax: temporary storage, 256 bytes
	 *	%r8d: 24 for 16 byte key, 32 for larger
	 *	%xmm0..%xmm15: 16 encrypted blocks
	 * output:
	 *	%xmm0..%xmm15: 16 plaintext blocks, order swapped:
	 *       7, 8, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
	 */
	CFI_STARTPROC();

	movq %r8, %rcx;
	movq CTX, %r8
	leaq (-8 * 8)(CTX, %rcx, 8), CTX;

	leaq 8 * 16(%rax), %rcx;

	inpack16_post(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		      %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		      %xmm15, %rax, %rcx);

.align 8
.Ldec_loop:
	dec_rounds16(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		     %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		     %xmm15, %rax, %rcx, 0);

	cmpq %r8, CTX;
	je .Ldec_done;

	fls16(%rax, %xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
	      %rcx, %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
	      %xmm15,
	      ((key_table) + 8)(CTX),
	      ((key_table) + 12)(CTX),
	      ((key_table) + 0)(CTX),
	      ((key_table) + 4)(CTX));

	leaq (-8 * 8)(CTX), CTX;
	jmp .Ldec_loop;

.align 8
.Ldec_done:
	/* load CD for output */
	vmovdqu 0 * 16(%rcx), %xmm8;
	vmovdqu 1 * 16(%rcx), %xmm9;
	vmovdqu 2 * 16(%rcx), %xmm10;
	vmovdqu 3 * 16(%rcx), %xmm11;
	vmovdqu 4 * 16(%rcx), %xmm12;
	vmovdqu 5 * 16(%rcx), %xmm13;
	vmovdqu 6 * 16(%rcx), %xmm14;
	vmovdqu 7 * 16(%rcx), %xmm15;

	outunpack16(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		    %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		    %xmm15, (key_table)(CTX), (%rax), 1 * 16(%rax));

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __camellia_dec_blk16,.-__camellia_dec_blk16;)

#define inc_le128(x, minus_one, tmp) \
	vpcmpeqq minus_one, x, tmp; \
	vpsubq minus_one, x, x; \
	vpslldq $8, tmp, tmp; \
	vpsubq tmp, x, x;

.align 8
.globl _gcry_camellia_aesni_avx_ctr_enc
ELF(.type   _gcry_camellia_aesni_avx_ctr_enc,@function;)

_gcry_camellia_aesni_avx_ctr_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv (big endian, 128bit)
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 16), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	vmovdqa .Lbswap128_mask rRIP, %xmm14;

	/* load IV and byteswap */
	vmovdqu (%rcx), %xmm15;
	vmovdqu %xmm15, 15 * 16(%rax);
	vpshufb %xmm14, %xmm15, %xmm0; /* be => le */

	vpcmpeqd %xmm15, %xmm15, %xmm15;
	vpsrldq $8, %xmm15, %xmm15; /* low: -1, high: 0 */

	/* construct IVs */
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm13;
	vmovdqu %xmm13, 14 * 16(%rax);
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm13;
	vmovdqu %xmm13, 13 * 16(%rax);
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm12;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm11;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm10;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm9;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm8;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm7;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm6;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm5;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm4;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm3;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm2;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vpshufb %xmm14, %xmm0, %xmm1;
	inc_le128(%xmm0, %xmm15, %xmm13);
	vmovdqa %xmm0, %xmm13;
	vpshufb %xmm14, %xmm0, %xmm0;
	inc_le128(%xmm13, %xmm15, %xmm14);
	vpshufb .Lbswap128_mask rRIP, %xmm13, %xmm13; /* le => be */
	vmovdqu %xmm13, (%rcx);

	/* inpack16_pre: */
	vmovq (key_table)(CTX), %xmm15;
	vpshufb .Lpack_bswap rRIP, %xmm15, %xmm15;
	vpxor %xmm0, %xmm15, %xmm0;
	vpxor %xmm1, %xmm15, %xmm1;
	vpxor %xmm2, %xmm15, %xmm2;
	vpxor %xmm3, %xmm15, %xmm3;
	vpxor %xmm4, %xmm15, %xmm4;
	vpxor %xmm5, %xmm15, %xmm5;
	vpxor %xmm6, %xmm15, %xmm6;
	vpxor %xmm7, %xmm15, %xmm7;
	vpxor %xmm8, %xmm15, %xmm8;
	vpxor %xmm9, %xmm15, %xmm9;
	vpxor %xmm10, %xmm15, %xmm10;
	vpxor %xmm11, %xmm15, %xmm11;
	vpxor %xmm12, %xmm15, %xmm12;
	vpxor 13 * 16(%rax), %xmm15, %xmm13;
	vpxor 14 * 16(%rax), %xmm15, %xmm14;
	vpxor 15 * 16(%rax), %xmm15, %xmm15;

	call __camellia_enc_blk16;

	vpxor 0 * 16(%rdx), %xmm7, %xmm7;
	vpxor 1 * 16(%rdx), %xmm6, %xmm6;
	vpxor 2 * 16(%rdx), %xmm5, %xmm5;
	vpxor 3 * 16(%rdx), %xmm4, %xmm4;
	vpxor 4 * 16(%rdx), %xmm3, %xmm3;
	vpxor 5 * 16(%rdx), %xmm2, %xmm2;
	vpxor 6 * 16(%rdx), %xmm1, %xmm1;
	vpxor 7 * 16(%rdx), %xmm0, %xmm0;
	vpxor 8 * 16(%rdx), %xmm15, %xmm15;
	vpxor 9 * 16(%rdx), %xmm14, %xmm14;
	vpxor 10 * 16(%rdx), %xmm13, %xmm13;
	vpxor 11 * 16(%rdx), %xmm12, %xmm12;
	vpxor 12 * 16(%rdx), %xmm11, %xmm11;
	vpxor 13 * 16(%rdx), %xmm10, %xmm10;
	vpxor 14 * 16(%rdx), %xmm9, %xmm9;
	vpxor 15 * 16(%rdx), %xmm8, %xmm8;

	write_output(%xmm7, %xmm6, %xmm5, %xmm4, %xmm3, %xmm2, %xmm1, %xmm0,
		     %xmm15, %xmm14, %xmm13, %xmm12, %xmm11, %xmm10, %xmm9,
		     %xmm8, %rsi);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_ctr_enc,.-_gcry_camellia_aesni_avx_ctr_enc;)

.align 8
.globl _gcry_camellia_aesni_avx_cbc_dec
ELF(.type   _gcry_camellia_aesni_avx_cbc_dec,@function;)

_gcry_camellia_aesni_avx_cbc_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	movq %rcx, %r9;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	inpack16_pre(%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7,
		     %xmm8, %xmm9, %xmm10, %xmm11, %xmm12, %xmm13, %xmm14,
		     %xmm15, %rdx, (key_table)(CTX, %r8, 8));

	subq $(16 * 16), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	call __camellia_dec_blk16;

	/* XOR output with IV */
	vpxor (%r9), %xmm7, %xmm7;
	vpxor (0 * 16)(%rdx), %xmm6, %xmm6;
	vpxor (1 * 16)(%rdx), %xmm5, %xmm5;
	vpxor (2 * 16)(%rdx), %xmm4, %xmm4;
	vpxor (3 * 16)(%rdx), %xmm3, %xmm3;
	vpxor (4 * 16)(%rdx), %xmm2, %xmm2;
	vpxor (5 * 16)(%rdx), %xmm1, %xmm1;
	vpxor (6 * 16)(%rdx), %xmm0, %xmm0;
	vpxor (7 * 16)(%rdx), %xmm15, %xmm15;
	vpxor (8 * 16)(%rdx), %xmm14, %xmm14;
	vpxor (9 * 16)(%rdx), %xmm13, %xmm13;
	vpxor (10 * 16)(%rdx), %xmm12, %xmm12;
	vpxor (11 * 16)(%rdx), %xmm11, %xmm11;
	vpxor (12 * 16)(%rdx), %xmm10, %xmm10;
	vpxor (13 * 16)(%rdx), %xmm9, %xmm9;
	vpxor (14 * 16)(%rdx), %xmm8, %xmm8;
	movq (15 * 16 + 0)(%rdx), %r10;
	movq (15 * 16 + 8)(%rdx), %r11;

	write_output(%xmm7, %xmm6, %xmm5, %xmm4, %xmm3, %xmm2, %xmm1, %xmm0,
		     %xmm15, %xmm14, %xmm13, %xmm12, %xmm11, %xmm10, %xmm9,
		     %xmm8, %rsi);

	/* store new IV */
	movq %r10, (0)(%r9);
	movq %r11, (8)(%r9);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_cbc_dec,.-_gcry_camellia_aesni_avx_cbc_dec;)

.align 8
.globl _gcry_camellia_aesni_avx_cfb_dec
ELF(.type   _gcry_camellia_aesni_avx_cfb_dec,@function;)

_gcry_camellia_aesni_avx_cfb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: iv
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %eax;
	cmovel %eax, %r8d; /* max */

	subq $(16 * 16), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	/* inpack16_pre: */
	vmovq (key_table)(CTX), %xmm0;
	vpshufb .Lpack_bswap rRIP, %xmm0, %xmm0;
	vpxor (%rcx), %xmm0, %xmm15;
	vmovdqu 15 * 16(%rdx), %xmm1;
	vmovdqu %xmm1, (%rcx); /* store new IV */
	vpxor 0 * 16(%rdx), %xmm0, %xmm14;
	vpxor 1 * 16(%rdx), %xmm0, %xmm13;
	vpxor 2 * 16(%rdx), %xmm0, %xmm12;
	vpxor 3 * 16(%rdx), %xmm0, %xmm11;
	vpxor 4 * 16(%rdx), %xmm0, %xmm10;
	vpxor 5 * 16(%rdx), %xmm0, %xmm9;
	vpxor 6 * 16(%rdx), %xmm0, %xmm8;
	vpxor 7 * 16(%rdx), %xmm0, %xmm7;
	vpxor 8 * 16(%rdx), %xmm0, %xmm6;
	vpxor 9 * 16(%rdx), %xmm0, %xmm5;
	vpxor 10 * 16(%rdx), %xmm0, %xmm4;
	vpxor 11 * 16(%rdx), %xmm0, %xmm3;
	vpxor 12 * 16(%rdx), %xmm0, %xmm2;
	vpxor 13 * 16(%rdx), %xmm0, %xmm1;
	vpxor 14 * 16(%rdx), %xmm0, %xmm0;

	call __camellia_enc_blk16;

	vpxor 0 * 16(%rdx), %xmm7, %xmm7;
	vpxor 1 * 16(%rdx), %xmm6, %xmm6;
	vpxor 2 * 16(%rdx), %xmm5, %xmm5;
	vpxor 3 * 16(%rdx), %xmm4, %xmm4;
	vpxor 4 * 16(%rdx), %xmm3, %xmm3;
	vpxor 5 * 16(%rdx), %xmm2, %xmm2;
	vpxor 6 * 16(%rdx), %xmm1, %xmm1;
	vpxor 7 * 16(%rdx), %xmm0, %xmm0;
	vpxor 8 * 16(%rdx), %xmm15, %xmm15;
	vpxor 9 * 16(%rdx), %xmm14, %xmm14;
	vpxor 10 * 16(%rdx), %xmm13, %xmm13;
	vpxor 11 * 16(%rdx), %xmm12, %xmm12;
	vpxor 12 * 16(%rdx), %xmm11, %xmm11;
	vpxor 13 * 16(%rdx), %xmm10, %xmm10;
	vpxor 14 * 16(%rdx), %xmm9, %xmm9;
	vpxor 15 * 16(%rdx), %xmm8, %xmm8;

	write_output(%xmm7, %xmm6, %xmm5, %xmm4, %xmm3, %xmm2, %xmm1, %xmm0,
		     %xmm15, %xmm14, %xmm13, %xmm12, %xmm11, %xmm10, %xmm9,
		     %xmm8, %rsi);

	vzeroall;

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_cfb_dec,.-_gcry_camellia_aesni_avx_cfb_dec;)

.align 8
.globl _gcry_camellia_aesni_avx_ocb_enc
ELF(.type   _gcry_camellia_aesni_avx_ocb_enc,@function;)

_gcry_camellia_aesni_avx_ocb_enc:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	subq $(16 * 16 + 4 * 8), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 16 + 0 * 8)(%rsp);
	movq %r11, (16 * 16 + 1 * 8)(%rsp);
	movq %r12, (16 * 16 + 2 * 8)(%rsp);
	movq %r13, (16 * 16 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 16 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 16 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 16 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 16 + 3 * 8);

	vmovdqu (%rcx), %xmm14;
	vmovdqu (%r8), %xmm15;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Checksum_i = Checksum_{i-1} xor P_i  */
	/* C_i = Offset_i xor ENCIPHER(K, P_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  vmovdqu (n * 16)(%rdx), xreg; \
	  vpxor (lreg), %xmm14, %xmm14; \
	  vpxor xreg, %xmm15, %xmm15; \
	  vpxor xreg, %xmm14, xreg; \
	  vmovdqu %xmm14, (n * 16)(%rsi);
	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %xmm0);
	vmovdqu %xmm0, (15 * 16)(%rax);
	OCB_INPUT(1, %r11, %xmm0);
	vmovdqu %xmm0, (14 * 16)(%rax);
	OCB_INPUT(2, %r12, %xmm13);
	OCB_INPUT(3, %r13, %xmm12);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %xmm11);
	OCB_INPUT(5, %r11, %xmm10);
	OCB_INPUT(6, %r12, %xmm9);
	OCB_INPUT(7, %r13, %xmm8);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(8, %r10, %xmm7);
	OCB_INPUT(9, %r11, %xmm6);
	OCB_INPUT(10, %r12, %xmm5);
	OCB_INPUT(11, %r13, %xmm4);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(12, %r10, %xmm3);
	OCB_INPUT(13, %r11, %xmm2);
	OCB_INPUT(14, %r12, %xmm1);
	OCB_INPUT(15, %r13, %xmm0);
#undef OCB_INPUT

	vmovdqu %xmm14, (%rcx);
	vmovdqu %xmm15, (%r8);

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r10d;
	cmovel %r10d, %r8d; /* max */

	/* inpack16_pre: */
	vmovq (key_table)(CTX), %xmm15;
	vpshufb .Lpack_bswap rRIP, %xmm15, %xmm15;
	vpxor %xmm0, %xmm15, %xmm0;
	vpxor %xmm1, %xmm15, %xmm1;
	vpxor %xmm2, %xmm15, %xmm2;
	vpxor %xmm3, %xmm15, %xmm3;
	vpxor %xmm4, %xmm15, %xmm4;
	vpxor %xmm5, %xmm15, %xmm5;
	vpxor %xmm6, %xmm15, %xmm6;
	vpxor %xmm7, %xmm15, %xmm7;
	vpxor %xmm8, %xmm15, %xmm8;
	vpxor %xmm9, %xmm15, %xmm9;
	vpxor %xmm10, %xmm15, %xmm10;
	vpxor %xmm11, %xmm15, %xmm11;
	vpxor %xmm12, %xmm15, %xmm12;
	vpxor %xmm13, %xmm15, %xmm13;
	vpxor 14 * 16(%rax), %xmm15, %xmm14;
	vpxor 15 * 16(%rax), %xmm15, %xmm15;

	call __camellia_enc_blk16;

	vpxor 0 * 16(%rsi), %xmm7, %xmm7;
	vpxor 1 * 16(%rsi), %xmm6, %xmm6;
	vpxor 2 * 16(%rsi), %xmm5, %xmm5;
	vpxor 3 * 16(%rsi), %xmm4, %xmm4;
	vpxor 4 * 16(%rsi), %xmm3, %xmm3;
	vpxor 5 * 16(%rsi), %xmm2, %xmm2;
	vpxor 6 * 16(%rsi), %xmm1, %xmm1;
	vpxor 7 * 16(%rsi), %xmm0, %xmm0;
	vpxor 8 * 16(%rsi), %xmm15, %xmm15;
	vpxor 9 * 16(%rsi), %xmm14, %xmm14;
	vpxor 10 * 16(%rsi), %xmm13, %xmm13;
	vpxor 11 * 16(%rsi), %xmm12, %xmm12;
	vpxor 12 * 16(%rsi), %xmm11, %xmm11;
	vpxor 13 * 16(%rsi), %xmm10, %xmm10;
	vpxor 14 * 16(%rsi), %xmm9, %xmm9;
	vpxor 15 * 16(%rsi), %xmm8, %xmm8;

	write_output(%xmm7, %xmm6, %xmm5, %xmm4, %xmm3, %xmm2, %xmm1, %xmm0,
		     %xmm15, %xmm14, %xmm13, %xmm12, %xmm11, %xmm10, %xmm9,
		     %xmm8, %rsi);

	vzeroall;

	movq (16 * 16 + 0 * 8)(%rsp), %r10;
	movq (16 * 16 + 1 * 8)(%rsp), %r11;
	movq (16 * 16 + 2 * 8)(%rsp), %r12;
	movq (16 * 16 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_ocb_enc,.-_gcry_camellia_aesni_avx_ocb_enc;)

.align 8
.globl _gcry_camellia_aesni_avx_ocb_dec
ELF(.type   _gcry_camellia_aesni_avx_ocb_dec,@function;)

_gcry_camellia_aesni_avx_ocb_dec:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: dst (16 blocks)
	 *	%rdx: src (16 blocks)
	 *	%rcx: offset
	 *	%r8 : checksum
	 *	%r9 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	subq $(16 * 16 + 4 * 8), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 16 + 0 * 8)(%rsp);
	movq %r11, (16 * 16 + 1 * 8)(%rsp);
	movq %r12, (16 * 16 + 2 * 8)(%rsp);
	movq %r13, (16 * 16 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 16 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 16 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 16 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 16 + 3 * 8);

	vmovdqu (%rcx), %xmm15;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* P_i = Offset_i xor DECIPHER(K, C_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  vmovdqu (n * 16)(%rdx), xreg; \
	  vpxor (lreg), %xmm15, %xmm15; \
	  vpxor xreg, %xmm15, xreg; \
	  vmovdqu %xmm15, (n * 16)(%rsi);
	movq (0 * 8)(%r9), %r10;
	movq (1 * 8)(%r9), %r11;
	movq (2 * 8)(%r9), %r12;
	movq (3 * 8)(%r9), %r13;
	OCB_INPUT(0, %r10, %xmm0);
	vmovdqu %xmm0, (15 * 16)(%rax);
	OCB_INPUT(1, %r11, %xmm14);
	OCB_INPUT(2, %r12, %xmm13);
	OCB_INPUT(3, %r13, %xmm12);
	movq (4 * 8)(%r9), %r10;
	movq (5 * 8)(%r9), %r11;
	movq (6 * 8)(%r9), %r12;
	movq (7 * 8)(%r9), %r13;
	OCB_INPUT(4, %r10, %xmm11);
	OCB_INPUT(5, %r11, %xmm10);
	OCB_INPUT(6, %r12, %xmm9);
	OCB_INPUT(7, %r13, %xmm8);
	movq (8 * 8)(%r9), %r10;
	movq (9 * 8)(%r9), %r11;
	movq (10 * 8)(%r9), %r12;
	movq (11 * 8)(%r9), %r13;
	OCB_INPUT(8, %r10, %xmm7);
	OCB_INPUT(9, %r11, %xmm6);
	OCB_INPUT(10, %r12, %xmm5);
	OCB_INPUT(11, %r13, %xmm4);
	movq (12 * 8)(%r9), %r10;
	movq (13 * 8)(%r9), %r11;
	movq (14 * 8)(%r9), %r12;
	movq (15 * 8)(%r9), %r13;
	OCB_INPUT(12, %r10, %xmm3);
	OCB_INPUT(13, %r11, %xmm2);
	OCB_INPUT(14, %r12, %xmm1);
	OCB_INPUT(15, %r13, %xmm0);
#undef OCB_INPUT

	vmovdqu %xmm15, (%rcx);

	movq %r8, %r10;

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r9d;
	cmovel %r9d, %r8d; /* max */

	/* inpack16_pre: */
	vmovq (key_table)(CTX, %r8, 8), %xmm15;
	vpshufb .Lpack_bswap rRIP, %xmm15, %xmm15;
	vpxor %xmm0, %xmm15, %xmm0;
	vpxor %xmm1, %xmm15, %xmm1;
	vpxor %xmm2, %xmm15, %xmm2;
	vpxor %xmm3, %xmm15, %xmm3;
	vpxor %xmm4, %xmm15, %xmm4;
	vpxor %xmm5, %xmm15, %xmm5;
	vpxor %xmm6, %xmm15, %xmm6;
	vpxor %xmm7, %xmm15, %xmm7;
	vpxor %xmm8, %xmm15, %xmm8;
	vpxor %xmm9, %xmm15, %xmm9;
	vpxor %xmm10, %xmm15, %xmm10;
	vpxor %xmm11, %xmm15, %xmm11;
	vpxor %xmm12, %xmm15, %xmm12;
	vpxor %xmm13, %xmm15, %xmm13;
	vpxor %xmm14, %xmm15, %xmm14;
	vpxor 15 * 16(%rax), %xmm15, %xmm15;

	call __camellia_dec_blk16;

	vpxor 0 * 16(%rsi), %xmm7, %xmm7;
	vpxor 1 * 16(%rsi), %xmm6, %xmm6;
	vpxor 2 * 16(%rsi), %xmm5, %xmm5;
	vpxor 3 * 16(%rsi), %xmm4, %xmm4;
	vpxor 4 * 16(%rsi), %xmm3, %xmm3;
	vpxor 5 * 16(%rsi), %xmm2, %xmm2;
	vpxor 6 * 16(%rsi), %xmm1, %xmm1;
	vpxor 7 * 16(%rsi), %xmm0, %xmm0;
	vmovdqu %xmm7, (7 * 16)(%rax);
	vpxor 8 * 16(%rsi), %xmm15, %xmm15;
	vpxor 9 * 16(%rsi), %xmm14, %xmm14;
	vpxor 10 * 16(%rsi), %xmm13, %xmm13;
	vpxor 11 * 16(%rsi), %xmm12, %xmm12;
	vpxor 12 * 16(%rsi), %xmm11, %xmm11;
	vpxor 13 * 16(%rsi), %xmm10, %xmm10;
	vpxor 14 * 16(%rsi), %xmm9, %xmm9;
	vpxor 15 * 16(%rsi), %xmm8, %xmm8;

	/* Checksum_i = Checksum_{i-1} xor P_i  */

	vpxor (%r10), %xmm7, %xmm7;
	vpxor %xmm6, %xmm7, %xmm7;
	vpxor %xmm5, %xmm7, %xmm7;
	vpxor %xmm4, %xmm7, %xmm7;
	vpxor %xmm3, %xmm7, %xmm7;
	vpxor %xmm2, %xmm7, %xmm7;
	vpxor %xmm1, %xmm7, %xmm7;
	vpxor %xmm0, %xmm7, %xmm7;
	vpxor %xmm15, %xmm7, %xmm7;
	vpxor %xmm14, %xmm7, %xmm7;
	vpxor %xmm13, %xmm7, %xmm7;
	vpxor %xmm12, %xmm7, %xmm7;
	vpxor %xmm11, %xmm7, %xmm7;
	vpxor %xmm10, %xmm7, %xmm7;
	vpxor %xmm9, %xmm7, %xmm7;
	vpxor %xmm8, %xmm7, %xmm7;
	vmovdqu %xmm7, (%r10);
	vmovdqu (7 * 16)(%rax), %xmm7;

	write_output(%xmm7, %xmm6, %xmm5, %xmm4, %xmm3, %xmm2, %xmm1, %xmm0,
		     %xmm15, %xmm14, %xmm13, %xmm12, %xmm11, %xmm10, %xmm9,
		     %xmm8, %rsi);

	vzeroall;

	movq (16 * 16 + 0 * 8)(%rsp), %r10;
	movq (16 * 16 + 1 * 8)(%rsp), %r11;
	movq (16 * 16 + 2 * 8)(%rsp), %r12;
	movq (16 * 16 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_ocb_dec,.-_gcry_camellia_aesni_avx_ocb_dec;)

.align 8
.globl _gcry_camellia_aesni_avx_ocb_auth
ELF(.type   _gcry_camellia_aesni_avx_ocb_auth,@function;)

_gcry_camellia_aesni_avx_ocb_auth:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: abuf (16 blocks)
	 *	%rdx: offset
	 *	%rcx: checksum
	 *	%r8 : L pointers (void *L[16])
	 */
	CFI_STARTPROC();

	pushq %rbp;
	CFI_PUSH(%rbp);
	movq %rsp, %rbp;
	CFI_DEF_CFA_REGISTER(%rbp);

	vzeroupper;

	subq $(16 * 16 + 4 * 8), %rsp;
	andq $~31, %rsp;
	movq %rsp, %rax;

	movq %r10, (16 * 16 + 0 * 8)(%rsp);
	movq %r11, (16 * 16 + 1 * 8)(%rsp);
	movq %r12, (16 * 16 + 2 * 8)(%rsp);
	movq %r13, (16 * 16 + 3 * 8)(%rsp);
	CFI_REG_ON_STACK(r10, 16 * 16 + 0 * 8);
	CFI_REG_ON_STACK(r11, 16 * 16 + 1 * 8);
	CFI_REG_ON_STACK(r12, 16 * 16 + 2 * 8);
	CFI_REG_ON_STACK(r13, 16 * 16 + 3 * 8);

	vmovdqu (%rdx), %xmm15;

	/* Offset_i = Offset_{i-1} xor L_{ntz(i)} */
	/* Sum_i = Sum_{i-1} xor ENCIPHER(K, A_i xor Offset_i)  */

#define OCB_INPUT(n, lreg, xreg) \
	  vmovdqu (n * 16)(%rsi), xreg; \
	  vpxor (lreg), %xmm15, %xmm15; \
	  vpxor xreg, %xmm15, xreg;

	movq (0 * 8)(%r8), %r10;
	movq (1 * 8)(%r8), %r11;
	movq (2 * 8)(%r8), %r12;
	movq (3 * 8)(%r8), %r13;
	OCB_INPUT(0, %r10, %xmm0);
	vmovdqu %xmm0, (15 * 16)(%rax);
	OCB_INPUT(1, %r11, %xmm14);
	OCB_INPUT(2, %r12, %xmm13);
	OCB_INPUT(3, %r13, %xmm12);
	movq (4 * 8)(%r8), %r10;
	movq (5 * 8)(%r8), %r11;
	movq (6 * 8)(%r8), %r12;
	movq (7 * 8)(%r8), %r13;
	OCB_INPUT(4, %r10, %xmm11);
	OCB_INPUT(5, %r11, %xmm10);
	OCB_INPUT(6, %r12, %xmm9);
	OCB_INPUT(7, %r13, %xmm8);
	movq (8 * 8)(%r8), %r10;
	movq (9 * 8)(%r8), %r11;
	movq (10 * 8)(%r8), %r12;
	movq (11 * 8)(%r8), %r13;
	OCB_INPUT(8, %r10, %xmm7);
	OCB_INPUT(9, %r11, %xmm6);
	OCB_INPUT(10, %r12, %xmm5);
	OCB_INPUT(11, %r13, %xmm4);
	movq (12 * 8)(%r8), %r10;
	movq (13 * 8)(%r8), %r11;
	movq (14 * 8)(%r8), %r12;
	movq (15 * 8)(%r8), %r13;
	OCB_INPUT(12, %r10, %xmm3);
	OCB_INPUT(13, %r11, %xmm2);
	OCB_INPUT(14, %r12, %xmm1);
	OCB_INPUT(15, %r13, %xmm0);
#undef OCB_INPUT

	cmpl $128, key_bitlength(CTX);
	movl $32, %r8d;
	movl $24, %r10d;
	cmovel %r10d, %r8d; /* max */

	vmovdqu %xmm15, (%rdx);

	movq %rcx, %r10;

	/* inpack16_pre: */
	vmovq (key_table)(CTX), %xmm15;
	vpshufb .Lpack_bswap rRIP, %xmm15, %xmm15;
	vpxor %xmm0, %xmm15, %xmm0;
	vpxor %xmm1, %xmm15, %xmm1;
	vpxor %xmm2, %xmm15, %xmm2;
	vpxor %xmm3, %xmm15, %xmm3;
	vpxor %xmm4, %xmm15, %xmm4;
	vpxor %xmm5, %xmm15, %xmm5;
	vpxor %xmm6, %xmm15, %xmm6;
	vpxor %xmm7, %xmm15, %xmm7;
	vpxor %xmm8, %xmm15, %xmm8;
	vpxor %xmm9, %xmm15, %xmm9;
	vpxor %xmm10, %xmm15, %xmm10;
	vpxor %xmm11, %xmm15, %xmm11;
	vpxor %xmm12, %xmm15, %xmm12;
	vpxor %xmm13, %xmm15, %xmm13;
	vpxor %xmm14, %xmm15, %xmm14;
	vpxor 15 * 16(%rax), %xmm15, %xmm15;

	call __camellia_enc_blk16;

	vpxor %xmm7, %xmm6, %xmm6;
	vpxor %xmm5, %xmm4, %xmm4;
	vpxor %xmm3, %xmm2, %xmm2;
	vpxor %xmm1, %xmm0, %xmm0;
	vpxor %xmm15, %xmm14, %xmm14;
	vpxor %xmm13, %xmm12, %xmm12;
	vpxor %xmm11, %xmm10, %xmm10;
	vpxor %xmm9, %xmm8, %xmm8;

	vpxor %xmm6, %xmm4, %xmm4;
	vpxor %xmm2, %xmm0, %xmm0;
	vpxor %xmm14, %xmm12, %xmm12;
	vpxor %xmm10, %xmm8, %xmm8;

	vpxor %xmm4, %xmm0, %xmm0;
	vpxor %xmm12, %xmm8, %xmm8;

	vpxor %xmm0, %xmm8, %xmm0;
	vpxor (%r10), %xmm0, %xmm0;
	vmovdqu %xmm0, (%r10);

	vzeroall;

	movq (16 * 16 + 0 * 8)(%rsp), %r10;
	movq (16 * 16 + 1 * 8)(%rsp), %r11;
	movq (16 * 16 + 2 * 8)(%rsp), %r12;
	movq (16 * 16 + 3 * 8)(%rsp), %r13;
	CFI_RESTORE(%r10);
	CFI_RESTORE(%r11);
	CFI_RESTORE(%r12);
	CFI_RESTORE(%r13);

	leave;
	CFI_LEAVE();
	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_ocb_auth,.-_gcry_camellia_aesni_avx_ocb_auth;)

/*
 * IN:
 *  ab: 64-bit AB state
 *  cd: 64-bit CD state
 */
#define camellia_f(ab, x, t0, t1, t2, t3, t4, inv_shift_row, sbox4mask, \
		   _0f0f0f0fmask, pre_s1lo_mask, pre_s1hi_mask, key) \
	vmovq key, t0; \
	vpxor x, x, t3; \
	\
	vpxor ab, t0, x; \
	\
	/* \
	 * S-function with AES subbytes \
	 */ \
	\
	/* input rotation for sbox4 (<<< 1) */ \
	vpand x, sbox4mask, t0; \
	vpandn x, sbox4mask, x; \
	vpaddw t0, t0, t1; \
	vpsrlw $7, t0, t0; \
	vpor t0, t1, t0; \
	vpand sbox4mask, t0, t0; \
	vpor t0, x, x; \
	\
	vmovdqa .Lpost_tf_lo_s1 rRIP, t0; \
	vmovdqa .Lpost_tf_hi_s1 rRIP, t1; \
	\
	/* prefilter sboxes */ \
	filter_8bit(x, pre_s1lo_mask, pre_s1hi_mask, _0f0f0f0fmask, t2); \
	\
	/* AES subbytes + AES shift rows + AES inv shift rows */ \
	vaesenclast t3, x, x; \
	\
	/* postfilter sboxes */ \
	filter_8bit(x, t0, t1, _0f0f0f0fmask, t2); \
	\
	/* output rotation for sbox2 (<<< 1) */ \
	/* output rotation for sbox3 (>>> 1) */ \
	vpshufb inv_shift_row, x, t1; \
	vpshufb .Lsp0044440444044404mask rRIP, x, t4; \
	vpshufb .Lsp1110111010011110mask rRIP, x, x; \
	vpaddb t1, t1, t2; \
	vpsrlw $7, t1, t0; \
	vpsllw $7, t1, t3; \
	vpor t0, t2, t0; \
	vpsrlw $1, t1, t1; \
	vpshufb .Lsp0222022222000222mask rRIP, t0, t0; \
	vpor t1, t3, t1; \
	\
	vpxor x, t4, t4; \
	vpshufb .Lsp3033303303303033mask rRIP, t1, t1; \
	vpxor t4, t0, t0; \
	vpxor t1, t0, t0; \
	vpsrldq $8, t0, x; \
	vpxor t0, x, x;

#define vec_rol128(in, out, nrol, t0) \
	vpshufd $0x4e, in, out; \
	vpsllq $(nrol), in, t0; \
	vpsrlq $(64-(nrol)), out, out; \
	vpaddd t0, out, out;

#define vec_ror128(in, out, nror, t0) \
	vpshufd $0x4e, in, out; \
	vpsrlq $(nror), in, t0; \
	vpsllq $(64-(nror)), out, out; \
	vpaddd t0, out, out;


.align 16
.Linv_shift_row_and_unpcklbw:
	.byte 0x00, 0xff, 0x0d, 0xff, 0x0a, 0xff, 0x07, 0xff
	.byte 0x04, 0xff, 0x01, 0xff, 0x0e, 0xff, 0x0b, 0xff
.Lsp0044440444044404mask:
	.long 0xffff0404, 0x0404ff04;
	.long 0x0d0dff0d, 0x0d0dff0d;
.Lsp1110111010011110mask:
	.long 0x000000ff, 0x000000ff;
	.long 0x0bffff0b, 0x0b0b0bff;
.Lsp0222022222000222mask:
	.long 0xff060606, 0xff060606;
	.long 0x0c0cffff, 0xff0c0c0c;
.Lsp3033303303303033mask:
	.long 0x04ff0404, 0x04ff0404;
	.long 0xff0a0aff, 0x0aff0a0a;
.Lsbox4_input_mask:
	.byte 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00;
.Lsigma1:
	.long 0x3BCC908B, 0xA09E667F;
.Lsigma2:
	.long 0x4CAA73B2, 0xB67AE858;
.Lsigma3:
	.long 0xE94F82BE, 0xC6EF372F;
.Lsigma4:
	.long 0xF1D36F1C, 0x54FF53A5;
.Lsigma5:
	.long 0xDE682D1D, 0x10E527FA;
.Lsigma6:
	.long 0xB3E6C1FD, 0xB05688C2;


.align 8
ELF(.type  __camellia_avx_setup128,@function;)
__camellia_avx_setup128:
	/* input:
	 *	%rdi: ctx, CTX; subkey storage at key_table(CTX)
	 *	%xmm0: key
	 */
	CFI_STARTPROC();

#define cmll_sub(n, ctx) (key_table+((n)*8))(ctx)
#define KL128 %xmm0
#define KA128 %xmm2

	vpshufb .Lbswap128_mask rRIP, KL128, KL128;

	vmovdqa .Linv_shift_row_and_unpcklbw rRIP, %xmm11;
	vmovq .Lsbox4_input_mask rRIP, %xmm12;
	vbroadcastss .L0f0f0f0f rRIP, %xmm13;
	vmovdqa .Lpre_tf_lo_s1 rRIP, %xmm14;
	vmovdqa .Lpre_tf_hi_s1 rRIP, %xmm15;

	/*
	 * Generate KA
	 */
	vpsrldq $8, KL128, %xmm2;
	vmovdqa KL128, %xmm3;
	vpslldq $8, %xmm3, %xmm3;
	vpsrldq $8, %xmm3, %xmm3;

	camellia_f(%xmm2, %xmm4, %xmm1,
		   %xmm5, %xmm6, %xmm7, %xmm8,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma1 rRIP);
	vpxor %xmm4, %xmm3, %xmm3;
	camellia_f(%xmm3, %xmm2, %xmm1,
		   %xmm5, %xmm6, %xmm7, %xmm8,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma2 rRIP);
	camellia_f(%xmm2, %xmm3, %xmm1,
		   %xmm5, %xmm6, %xmm7, %xmm8,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma3 rRIP);
	vpxor %xmm4, %xmm3, %xmm3;
	camellia_f(%xmm3, %xmm4, %xmm1,
		   %xmm5, %xmm6, %xmm7, %xmm8,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma4 rRIP);

	vpslldq $8, %xmm3, %xmm3;
	vpxor %xmm4, %xmm2, %xmm2;
	vpsrldq $8, %xmm3, %xmm3;
	vpslldq $8, %xmm2, KA128;
	vpor %xmm3, KA128, KA128;

        /*
         * Generate subkeys
         */
	vmovdqu KA128, cmll_sub(24, CTX);
	vec_rol128(KL128, %xmm3, 15, %xmm15);
	vec_rol128(KA128, %xmm4, 15, %xmm15);
	vec_rol128(KA128, %xmm5, 30, %xmm15);
	vec_rol128(KL128, %xmm6, 45, %xmm15);
	vec_rol128(KA128, %xmm7, 45, %xmm15);
	vec_rol128(KL128, %xmm8, 60, %xmm15);
	vec_rol128(KA128, %xmm9, 60, %xmm15);
	vec_ror128(KL128, %xmm10, 128-77, %xmm15);

	/* absorb kw2 to other subkeys */
	vpslldq $8, KL128, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, KA128, KA128;
	vpxor %xmm15, %xmm3, %xmm3;
	vpxor %xmm15, %xmm4, %xmm4;

	/* subl(1) ^= subr(1) & ~subr(9); */
	vpandn %xmm15, %xmm5, %xmm13;
	vpslldq $12, %xmm13, %xmm13;
	vpsrldq $8, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(1) & subl(9), subr(1) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm5, %xmm14;
	vpslld $1, %xmm14, %xmm11;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm11, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm6, %xmm6;
	vpxor %xmm15, %xmm8, %xmm8;
	vpxor %xmm15, %xmm9, %xmm9;

	/* subl(1) ^= subr(1) & ~subr(17); */
	vpandn %xmm15, %xmm10, %xmm13;
	vpslldq $12, %xmm13, %xmm13;
	vpsrldq $8, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(1) & subl(17), subr(1) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm10, %xmm14;
	vpslld $1, %xmm14, %xmm11;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm11, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpshufd $0x1b, KL128, KL128;
	vpshufd $0x1b, KA128, KA128;
	vpshufd $0x1b, %xmm3, %xmm3;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm5, %xmm5;
	vpshufd $0x1b, %xmm6, %xmm6;
	vpshufd $0x1b, %xmm7, %xmm7;
	vpshufd $0x1b, %xmm8, %xmm8;
	vpshufd $0x1b, %xmm9, %xmm9;
	vpshufd $0x1b, %xmm10, %xmm10;

	vmovdqu KL128, cmll_sub(0, CTX);
	vpshufd $0x1b, KL128, KL128;
	vmovdqu KA128, cmll_sub(2, CTX);
	vmovdqu %xmm3, cmll_sub(4, CTX);
	vmovdqu %xmm4, cmll_sub(6, CTX);
	vmovdqu %xmm5, cmll_sub(8, CTX);
	vmovdqu %xmm6, cmll_sub(10, CTX);
	vpsrldq $8, %xmm8, %xmm8;
	vmovq %xmm7, cmll_sub(12, CTX);
	vmovq %xmm8, cmll_sub(13, CTX);
	vmovdqu %xmm9, cmll_sub(14, CTX);
	vmovdqu %xmm10, cmll_sub(16, CTX);

	vmovdqu cmll_sub(24, CTX), KA128;

	vec_ror128(KL128, %xmm3, 128 - 94, %xmm7);
	vec_ror128(KA128, %xmm4, 128 - 94, %xmm7);
	vec_ror128(KL128, %xmm5, 128 - 111, %xmm7);
	vec_ror128(KA128, %xmm6, 128 - 111, %xmm7);

	vpxor %xmm15, %xmm3, %xmm3;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm5, %xmm5;
	vpslldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm6, %xmm6;

	/* absorb kw4 to other subkeys */
	vpslldq $8, %xmm6, %xmm15;
	vpxor %xmm15, %xmm5, %xmm5;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm3, %xmm3;

	/* subl(25) ^= subr(25) & ~subr(16); */
	vpshufd $0x1b, cmll_sub(16, CTX), %xmm10;
	vpandn %xmm15, %xmm10, %xmm13;
	vpslldq $4, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(25) & subl(16), subr(25) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm10, %xmm14;
	vpslld $1, %xmm14, %xmm11;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm11, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpshufd $0x1b, %xmm3, %xmm3;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm5, %xmm5;
	vpshufd $0x1b, %xmm6, %xmm6;

	vmovdqu %xmm3, cmll_sub(18, CTX);
	vmovdqu %xmm4, cmll_sub(20, CTX);
	vmovdqu %xmm5, cmll_sub(22, CTX);
	vmovdqu %xmm6, cmll_sub(24, CTX);

	vpshufd $0x1b, cmll_sub(14, CTX), %xmm3;
	vpshufd $0x1b, cmll_sub(12, CTX), %xmm4;
	vpshufd $0x1b, cmll_sub(10, CTX), %xmm5;
	vpshufd $0x1b, cmll_sub(8, CTX), %xmm6;

	vpxor %xmm15, %xmm3, %xmm3;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm5, %xmm5;

	/* subl(25) ^= subr(25) & ~subr(8); */
	vpandn %xmm15, %xmm6, %xmm13;
	vpslldq $4, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(25) & subl(8), subr(25) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm6, %xmm14;
	vpslld $1, %xmm14, %xmm11;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm11, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpshufd $0x1b, %xmm3, %xmm3;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm5, %xmm5;

	vmovdqu %xmm3, cmll_sub(14, CTX);
	vmovdqu %xmm4, cmll_sub(12, CTX);
	vmovdqu %xmm5, cmll_sub(10, CTX);

	vpshufd $0x1b, cmll_sub(6, CTX), %xmm6;
	vpshufd $0x1b, cmll_sub(4, CTX), %xmm4;
	vpshufd $0x1b, cmll_sub(2, CTX), %xmm2;
	vpshufd $0x1b, cmll_sub(0, CTX), %xmm0;

	vpxor %xmm15, %xmm6, %xmm6;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm2, %xmm2;
	vpxor %xmm15, %xmm0, %xmm0;

	vpshufd $0x1b, %xmm6, %xmm6;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm2, %xmm2;
	vpshufd $0x1b, %xmm0, %xmm0;

	vpsrldq $8, %xmm2, %xmm3;
	vpsrldq $8, %xmm4, %xmm5;
	vpsrldq $8, %xmm6, %xmm7;

        /*
	 * key XOR is end of F-function.
	 */
	vpxor %xmm2, %xmm0, %xmm0;
	vpxor %xmm4, %xmm2, %xmm2;

	vmovq %xmm0, cmll_sub(0, CTX);
	vmovq %xmm3, cmll_sub(2, CTX);
	vpxor %xmm5, %xmm3, %xmm3;
	vpxor %xmm6, %xmm4, %xmm4;
	vpxor %xmm7, %xmm5, %xmm5;
	vmovq %xmm2, cmll_sub(3, CTX);
	vmovq %xmm3, cmll_sub(4, CTX);
	vmovq %xmm4, cmll_sub(5, CTX);
	vmovq %xmm5, cmll_sub(6, CTX);

	vmovq cmll_sub(7, CTX), %xmm7;
	vmovq cmll_sub(8, CTX), %xmm8;
	vmovq cmll_sub(9, CTX), %xmm9;
	vmovq cmll_sub(10, CTX), %xmm10;
	/* tl = subl(10) ^ (subr(10) & ~subr(8)); */
	vpandn %xmm10, %xmm8, %xmm15;
	vpsrldq $4, %xmm15, %xmm15;
	vpxor %xmm15, %xmm10, %xmm0;
	/* dw = tl & subl(8), tr = subr(10) ^ CAMELLIA_RL1(dw); */
	vpand %xmm8, %xmm0, %xmm15;
	vpslld $1, %xmm15, %xmm14;
	vpsrld $31, %xmm15, %xmm15;
	vpaddd %xmm14, %xmm15, %xmm15;
	vpslldq $12, %xmm15, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm0, %xmm0;

	vpxor %xmm0, %xmm6, %xmm6;
	vmovq %xmm6, cmll_sub(7, CTX);

	vmovq cmll_sub(11, CTX), %xmm11;
	vmovq cmll_sub(12, CTX), %xmm12;
	vmovq cmll_sub(13, CTX), %xmm13;
	vmovq cmll_sub(14, CTX), %xmm14;
	vmovq cmll_sub(15, CTX), %xmm15;
	/* tl = subl(7) ^ (subr(7) & ~subr(9)); */
	vpandn %xmm7, %xmm9, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm7, %xmm0;
	/* dw = tl & subl(9), tr = subr(7) ^ CAMELLIA_RL1(dw); */
	vpand %xmm9, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vpxor %xmm11, %xmm0, %xmm0;
	vpxor %xmm12, %xmm10, %xmm10;
	vpxor %xmm13, %xmm11, %xmm11;
	vpxor %xmm14, %xmm12, %xmm12;
	vpxor %xmm15, %xmm13, %xmm13;
	vmovq %xmm0, cmll_sub(10, CTX);
	vmovq %xmm10, cmll_sub(11, CTX);
	vmovq %xmm11, cmll_sub(12, CTX);
	vmovq %xmm12, cmll_sub(13, CTX);
	vmovq %xmm13, cmll_sub(14, CTX);

	vmovq cmll_sub(16, CTX), %xmm6;
	vmovq cmll_sub(17, CTX), %xmm7;
	vmovq cmll_sub(18, CTX), %xmm8;
	vmovq cmll_sub(19, CTX), %xmm9;
	vmovq cmll_sub(20, CTX), %xmm10;
	/* tl = subl(18) ^ (subr(18) & ~subr(16)); */
	vpandn %xmm8, %xmm6, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm8, %xmm0;
	/* dw = tl & subl(16), tr = subr(18) ^ CAMELLIA_RL1(dw); */
	vpand %xmm6, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vpxor %xmm14, %xmm0, %xmm0;
	vmovq %xmm0, cmll_sub(15, CTX);

	/* tl = subl(15) ^ (subr(15) & ~subr(17)); */
	vpandn %xmm15, %xmm7, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm15, %xmm0;
	/* dw = tl & subl(17), tr = subr(15) ^ CAMELLIA_RL1(dw); */
	vpand %xmm7, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vmovq cmll_sub(21, CTX), %xmm1;
	vmovq cmll_sub(22, CTX), %xmm2;
	vmovq cmll_sub(23, CTX), %xmm3;
	vmovq cmll_sub(24, CTX), %xmm4;

	vpxor %xmm9, %xmm0, %xmm0;
	vpxor %xmm10, %xmm8, %xmm8;
	vpxor %xmm1, %xmm9, %xmm9;
	vpxor %xmm2, %xmm10, %xmm10;
	vpxor %xmm3, %xmm1, %xmm1;
	vpxor %xmm4, %xmm3, %xmm3;

	vmovq %xmm0, cmll_sub(18, CTX);
	vmovq %xmm8, cmll_sub(19, CTX);
	vmovq %xmm9, cmll_sub(20, CTX);
	vmovq %xmm10, cmll_sub(21, CTX);
	vmovq %xmm1, cmll_sub(22, CTX);
	vmovq %xmm2, cmll_sub(23, CTX);
	vmovq %xmm3, cmll_sub(24, CTX);

	/* kw2 and kw4 are unused now. */
	movq $0, cmll_sub(1, CTX);
	movq $0, cmll_sub(25, CTX);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __camellia_avx_setup128,.-__camellia_avx_setup128;)

.align 8
ELF(.type  __camellia_avx_setup256,@function;)

__camellia_avx_setup256:
	/* input:
	 *	%rdi: ctx, CTX; subkey storage at key_table(CTX)
	 *	%xmm0 & %xmm1: key
	 */
	CFI_STARTPROC();

#define KL128 %xmm0
#define KR128 %xmm1
#define KA128 %xmm2
#define KB128 %xmm3

	vpshufb .Lbswap128_mask rRIP, KL128, KL128;
	vpshufb .Lbswap128_mask rRIP, KR128, KR128;

	vmovdqa .Linv_shift_row_and_unpcklbw rRIP, %xmm11;
	vmovq .Lsbox4_input_mask rRIP, %xmm12;
	vbroadcastss .L0f0f0f0f rRIP, %xmm13;
	vmovdqa .Lpre_tf_lo_s1 rRIP, %xmm14;
	vmovdqa .Lpre_tf_hi_s1 rRIP, %xmm15;

	/*
	 * Generate KA
	 */
	vpxor KL128, KR128, %xmm3;
	vpsrldq $8, KR128, %xmm6;
	vpsrldq $8, %xmm3, %xmm2;
	vpslldq $8, %xmm3, %xmm3;
	vpsrldq $8, %xmm3, %xmm3;

	camellia_f(%xmm2, %xmm4, %xmm5,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma1 rRIP);
	vpxor %xmm4, %xmm3, %xmm3;
	camellia_f(%xmm3, %xmm2, %xmm5,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma2 rRIP);
	vpxor %xmm6, %xmm2, %xmm2;
	camellia_f(%xmm2, %xmm3, %xmm5,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma3 rRIP);
	vpxor %xmm4, %xmm3, %xmm3;
	vpxor KR128, %xmm3, %xmm3;
	camellia_f(%xmm3, %xmm4, %xmm5,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma4 rRIP);

	vpslldq $8, %xmm3, %xmm3;
	vpxor %xmm4, %xmm2, %xmm2;
	vpsrldq $8, %xmm3, %xmm3;
	vpslldq $8, %xmm2, KA128;
	vpor %xmm3, KA128, KA128;

	/*
	 * Generate KB
	 */
	vpxor KA128, KR128, %xmm3;
	vpsrldq $8, %xmm3, %xmm4;
	vpslldq $8, %xmm3, %xmm3;
	vpsrldq $8, %xmm3, %xmm3;

	camellia_f(%xmm4, %xmm5, %xmm6,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma5 rRIP);
	vpxor %xmm5, %xmm3, %xmm3;

	camellia_f(%xmm3, %xmm5, %xmm6,
		   %xmm7, %xmm8, %xmm9, %xmm10,
		   %xmm11, %xmm12, %xmm13, %xmm14, %xmm15, .Lsigma6 rRIP);
	vpslldq $8, %xmm3, %xmm3;
	vpxor %xmm5, %xmm4, %xmm4;
	vpsrldq $8, %xmm3, %xmm3;
	vpslldq $8, %xmm4, %xmm4;
	vpor %xmm3, %xmm4, KB128;

        /*
         * Generate subkeys
         */
	vmovdqu KB128, cmll_sub(32, CTX);
	vec_rol128(KR128, %xmm4, 15, %xmm15);
	vec_rol128(KA128, %xmm5, 15, %xmm15);
	vec_rol128(KR128, %xmm6, 30, %xmm15);
	vec_rol128(KB128, %xmm7, 30, %xmm15);
	vec_rol128(KL128, %xmm8, 45, %xmm15);
	vec_rol128(KA128, %xmm9, 45, %xmm15);
	vec_rol128(KL128, %xmm10, 60, %xmm15);
	vec_rol128(KR128, %xmm11, 60, %xmm15);
	vec_rol128(KB128, %xmm12, 60, %xmm15);

	/* absorb kw2 to other subkeys */
	vpslldq $8, KL128, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, KB128, KB128;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm5, %xmm5;

	/* subl(1) ^= subr(1) & ~subr(9); */
	vpandn %xmm15, %xmm6, %xmm13;
	vpslldq $12, %xmm13, %xmm13;
	vpsrldq $8, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(1) & subl(9), subr(1) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm6, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm7, %xmm7;
	vpxor %xmm15, %xmm8, %xmm8;
	vpxor %xmm15, %xmm9, %xmm9;

	vpshufd $0x1b, KL128, KL128;
	vpshufd $0x1b, KB128, KB128;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm5, %xmm5;
	vpshufd $0x1b, %xmm6, %xmm6;
	vpshufd $0x1b, %xmm7, %xmm7;
	vpshufd $0x1b, %xmm8, %xmm8;
	vpshufd $0x1b, %xmm9, %xmm9;

	vmovdqu KL128, cmll_sub(0, CTX);
	vpshufd $0x1b, KL128, KL128;
	vmovdqu KB128, cmll_sub(2, CTX);
	vmovdqu %xmm4, cmll_sub(4, CTX);
	vmovdqu %xmm5, cmll_sub(6, CTX);
	vmovdqu %xmm6, cmll_sub(8, CTX);
	vmovdqu %xmm7, cmll_sub(10, CTX);
	vmovdqu %xmm8, cmll_sub(12, CTX);
	vmovdqu %xmm9, cmll_sub(14, CTX);

	vmovdqu cmll_sub(32, CTX), KB128;

	/* subl(1) ^= subr(1) & ~subr(17); */
	vpandn %xmm15, %xmm10, %xmm13;
	vpslldq $12, %xmm13, %xmm13;
	vpsrldq $8, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(1) & subl(17), subr(1) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm10, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm11, %xmm11;
	vpxor %xmm15, %xmm12, %xmm12;

	vec_ror128(KL128, %xmm4, 128-77, %xmm14);
	vec_ror128(KA128, %xmm5, 128-77, %xmm14);
	vec_ror128(KR128, %xmm6, 128-94, %xmm14);
	vec_ror128(KA128, %xmm7, 128-94, %xmm14);
	vec_ror128(KL128, %xmm8, 128-111, %xmm14);
	vec_ror128(KB128, %xmm9, 128-111, %xmm14);

	vpxor %xmm15, %xmm4, %xmm4;

	vpshufd $0x1b, %xmm10, %xmm10;
	vpshufd $0x1b, %xmm11, %xmm11;
	vpshufd $0x1b, %xmm12, %xmm12;
	vpshufd $0x1b, %xmm4, %xmm4;

	vmovdqu %xmm10, cmll_sub(16, CTX);
	vmovdqu %xmm11, cmll_sub(18, CTX);
	vmovdqu %xmm12, cmll_sub(20, CTX);
	vmovdqu %xmm4, cmll_sub(22, CTX);

	/* subl(1) ^= subr(1) & ~subr(25); */
	vpandn %xmm15, %xmm5, %xmm13;
	vpslldq $12, %xmm13, %xmm13;
	vpsrldq $8, %xmm13, %xmm13;
	vpxor %xmm13, %xmm15, %xmm15;
	/* dw = subl(1) & subl(25), subr(1) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm5, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm6, %xmm6;
	vpxor %xmm15, %xmm7, %xmm7;
	vpxor %xmm15, %xmm8, %xmm8;
	vpslldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm9, %xmm9;

	/* absorb kw4 to other subkeys */
	vpslldq $8, %xmm9, %xmm15;
	vpxor %xmm15, %xmm8, %xmm8;
	vpxor %xmm15, %xmm7, %xmm7;
	vpxor %xmm15, %xmm6, %xmm6;

	/* subl(33) ^= subr(33) & ~subr(24); */
	vpandn %xmm15, %xmm5, %xmm14;
	vpslldq $4, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;
	/* dw = subl(33) & subl(24), subr(33) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm5, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpshufd $0x1b, %xmm5, %xmm5;
	vpshufd $0x1b, %xmm6, %xmm6;
	vpshufd $0x1b, %xmm7, %xmm7;
	vpshufd $0x1b, %xmm8, %xmm8;
	vpshufd $0x1b, %xmm9, %xmm9;

	vmovdqu %xmm5, cmll_sub(24, CTX);
	vmovdqu %xmm6, cmll_sub(26, CTX);
	vmovdqu %xmm7, cmll_sub(28, CTX);
	vmovdqu %xmm8, cmll_sub(30, CTX);
	vmovdqu %xmm9, cmll_sub(32, CTX);

	vpshufd $0x1b, cmll_sub(22, CTX), %xmm0;
	vpshufd $0x1b, cmll_sub(20, CTX), %xmm1;
	vpshufd $0x1b, cmll_sub(18, CTX), %xmm2;
	vpshufd $0x1b, cmll_sub(16, CTX), %xmm3;
	vpshufd $0x1b, cmll_sub(14, CTX), %xmm4;
	vpshufd $0x1b, cmll_sub(12, CTX), %xmm5;
	vpshufd $0x1b, cmll_sub(10, CTX), %xmm6;
	vpshufd $0x1b, cmll_sub(8, CTX), %xmm7;

	vpxor %xmm15, %xmm0, %xmm0;
	vpxor %xmm15, %xmm1, %xmm1;
	vpxor %xmm15, %xmm2, %xmm2;

	/* subl(33) ^= subr(33) & ~subr(24); */
	vpandn %xmm15, %xmm3, %xmm14;
	vpslldq $4, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;
	/* dw = subl(33) & subl(24), subr(33) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm3, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm5, %xmm5;
	vpxor %xmm15, %xmm6, %xmm6;

	vpshufd $0x1b, %xmm0, %xmm0;
	vpshufd $0x1b, %xmm1, %xmm1;
	vpshufd $0x1b, %xmm2, %xmm2;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm5, %xmm5;
	vpshufd $0x1b, %xmm6, %xmm6;

	vmovdqu %xmm0, cmll_sub(22, CTX);
	vmovdqu %xmm1, cmll_sub(20, CTX);
	vmovdqu %xmm2, cmll_sub(18, CTX);
	vmovdqu %xmm4, cmll_sub(14, CTX);
	vmovdqu %xmm5, cmll_sub(12, CTX);
	vmovdqu %xmm6, cmll_sub(10, CTX);

	vpshufd $0x1b, cmll_sub(6, CTX), %xmm6;
	vpshufd $0x1b, cmll_sub(4, CTX), %xmm4;
	vpshufd $0x1b, cmll_sub(2, CTX), %xmm2;
	vpshufd $0x1b, cmll_sub(0, CTX), %xmm0;

	/* subl(33) ^= subr(33) & ~subr(24); */
	vpandn %xmm15, %xmm7, %xmm14;
	vpslldq $4, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;
	/* dw = subl(33) & subl(24), subr(33) ^= CAMELLIA_RL1(dw); */
	vpand %xmm15, %xmm7, %xmm14;
	vpslld $1, %xmm14, %xmm13;
	vpsrld $31, %xmm14, %xmm14;
	vpaddd %xmm13, %xmm14, %xmm14;
	vpsrldq $12, %xmm14, %xmm14;
	vpslldq $8, %xmm14, %xmm14;
	vpxor %xmm14, %xmm15, %xmm15;

	vpxor %xmm15, %xmm6, %xmm6;
	vpxor %xmm15, %xmm4, %xmm4;
	vpxor %xmm15, %xmm2, %xmm2;
	vpxor %xmm15, %xmm0, %xmm0;

	vpshufd $0x1b, %xmm6, %xmm6;
	vpshufd $0x1b, %xmm4, %xmm4;
	vpshufd $0x1b, %xmm2, %xmm2;
	vpshufd $0x1b, %xmm0, %xmm0;

	vpsrldq $8, %xmm2, %xmm3;
	vpsrldq $8, %xmm4, %xmm5;
	vpsrldq $8, %xmm6, %xmm7;

        /*
	 * key XOR is end of F-function.
	 */
	vpxor %xmm2, %xmm0, %xmm0;
	vpxor %xmm4, %xmm2, %xmm2;

	vmovq %xmm0, cmll_sub(0, CTX);
	vmovq %xmm3, cmll_sub(2, CTX);
	vpxor %xmm5, %xmm3, %xmm3;
	vpxor %xmm6, %xmm4, %xmm4;
	vpxor %xmm7, %xmm5, %xmm5;
	vmovq %xmm2, cmll_sub(3, CTX);
	vmovq %xmm3, cmll_sub(4, CTX);
	vmovq %xmm4, cmll_sub(5, CTX);
	vmovq %xmm5, cmll_sub(6, CTX);

	vmovq cmll_sub(7, CTX), %xmm7;
	vmovq cmll_sub(8, CTX), %xmm8;
	vmovq cmll_sub(9, CTX), %xmm9;
	vmovq cmll_sub(10, CTX), %xmm10;
	/* tl = subl(10) ^ (subr(10) & ~subr(8)); */
	vpandn %xmm10, %xmm8, %xmm15;
	vpsrldq $4, %xmm15, %xmm15;
	vpxor %xmm15, %xmm10, %xmm0;
	/* dw = tl & subl(8), tr = subr(10) ^ CAMELLIA_RL1(dw); */
	vpand %xmm8, %xmm0, %xmm15;
	vpslld $1, %xmm15, %xmm14;
	vpsrld $31, %xmm15, %xmm15;
	vpaddd %xmm14, %xmm15, %xmm15;
	vpslldq $12, %xmm15, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm0, %xmm0;

	vpxor %xmm0, %xmm6, %xmm6;
	vmovq %xmm6, cmll_sub(7, CTX);

	vmovq cmll_sub(11, CTX), %xmm11;
	vmovq cmll_sub(12, CTX), %xmm12;
	vmovq cmll_sub(13, CTX), %xmm13;
	vmovq cmll_sub(14, CTX), %xmm14;
	vmovq cmll_sub(15, CTX), %xmm15;
	/* tl = subl(7) ^ (subr(7) & ~subr(9)); */
	vpandn %xmm7, %xmm9, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm7, %xmm0;
	/* dw = tl & subl(9), tr = subr(7) ^ CAMELLIA_RL1(dw); */
	vpand %xmm9, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vpxor %xmm11, %xmm0, %xmm0;
	vpxor %xmm12, %xmm10, %xmm10;
	vpxor %xmm13, %xmm11, %xmm11;
	vpxor %xmm14, %xmm12, %xmm12;
	vpxor %xmm15, %xmm13, %xmm13;
	vmovq %xmm0, cmll_sub(10, CTX);
	vmovq %xmm10, cmll_sub(11, CTX);
	vmovq %xmm11, cmll_sub(12, CTX);
	vmovq %xmm12, cmll_sub(13, CTX);
	vmovq %xmm13, cmll_sub(14, CTX);

	vmovq cmll_sub(16, CTX), %xmm6;
	vmovq cmll_sub(17, CTX), %xmm7;
	vmovq cmll_sub(18, CTX), %xmm8;
	vmovq cmll_sub(19, CTX), %xmm9;
	vmovq cmll_sub(20, CTX), %xmm10;
	/* tl = subl(18) ^ (subr(18) & ~subr(16)); */
	vpandn %xmm8, %xmm6, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm8, %xmm0;
	/* dw = tl & subl(16), tr = subr(18) ^ CAMELLIA_RL1(dw); */
	vpand %xmm6, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vpxor %xmm14, %xmm0, %xmm0;
	vmovq %xmm0, cmll_sub(15, CTX);

	/* tl = subl(15) ^ (subr(15) & ~subr(17)); */
	vpandn %xmm15, %xmm7, %xmm1;
	vpsrldq $4, %xmm1, %xmm1;
	vpxor %xmm1, %xmm15, %xmm0;
	/* dw = tl & subl(17), tr = subr(15) ^ CAMELLIA_RL1(dw); */
	vpand %xmm7, %xmm0, %xmm1;
	vpslld $1, %xmm1, %xmm2;
	vpsrld $31, %xmm1, %xmm1;
	vpaddd %xmm2, %xmm1, %xmm1;
	vpslldq $12, %xmm1, %xmm1;
	vpsrldq $8, %xmm1, %xmm1;
	vpxor %xmm1, %xmm0, %xmm0;

	vmovq cmll_sub(21, CTX), %xmm1;
	vmovq cmll_sub(22, CTX), %xmm2;
	vmovq cmll_sub(23, CTX), %xmm3;
	vmovq cmll_sub(24, CTX), %xmm4;

	vpxor %xmm9, %xmm0, %xmm0;
	vpxor %xmm10, %xmm8, %xmm8;
	vpxor %xmm1, %xmm9, %xmm9;
	vpxor %xmm2, %xmm10, %xmm10;
	vpxor %xmm3, %xmm1, %xmm1;

	vmovq %xmm0, cmll_sub(18, CTX);
	vmovq %xmm8, cmll_sub(19, CTX);
	vmovq %xmm9, cmll_sub(20, CTX);
	vmovq %xmm10, cmll_sub(21, CTX);
	vmovq %xmm1, cmll_sub(22, CTX);

	vmovq cmll_sub(25, CTX), %xmm5;
	vmovq cmll_sub(26, CTX), %xmm6;
	vmovq cmll_sub(27, CTX), %xmm7;
	vmovq cmll_sub(28, CTX), %xmm8;
	vmovq cmll_sub(29, CTX), %xmm9;
	vmovq cmll_sub(30, CTX), %xmm10;
	vmovq cmll_sub(31, CTX), %xmm11;
	vmovq cmll_sub(32, CTX), %xmm12;

	/* tl = subl(26) ^ (subr(26) & ~subr(24)); */
	vpandn %xmm6, %xmm4, %xmm15;
	vpsrldq $4, %xmm15, %xmm15;
	vpxor %xmm15, %xmm6, %xmm0;
	/* dw = tl & subl(26), tr = subr(24) ^ CAMELLIA_RL1(dw); */
	vpand %xmm4, %xmm0, %xmm15;
	vpslld $1, %xmm15, %xmm14;
	vpsrld $31, %xmm15, %xmm15;
	vpaddd %xmm14, %xmm15, %xmm15;
	vpslldq $12, %xmm15, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm0, %xmm0;

	vpxor %xmm0, %xmm2, %xmm2;
	vmovq %xmm2, cmll_sub(23, CTX);

	/* tl = subl(23) ^ (subr(23) &  ~subr(25)); */
	vpandn %xmm3, %xmm5, %xmm15;
	vpsrldq $4, %xmm15, %xmm15;
	vpxor %xmm15, %xmm3, %xmm0;
	/* dw = tl & subl(26), tr = subr(24) ^ CAMELLIA_RL1(dw); */
	vpand %xmm5, %xmm0, %xmm15;
	vpslld $1, %xmm15, %xmm14;
	vpsrld $31, %xmm15, %xmm15;
	vpaddd %xmm14, %xmm15, %xmm15;
	vpslldq $12, %xmm15, %xmm15;
	vpsrldq $8, %xmm15, %xmm15;
	vpxor %xmm15, %xmm0, %xmm0;

	vpxor %xmm7, %xmm0, %xmm0;
	vpxor %xmm8, %xmm6, %xmm6;
	vpxor %xmm9, %xmm7, %xmm7;
	vpxor %xmm10, %xmm8, %xmm8;
	vpxor %xmm11, %xmm9, %xmm9;
	vpxor %xmm12, %xmm11, %xmm11;

	vmovq %xmm0, cmll_sub(26, CTX);
	vmovq %xmm6, cmll_sub(27, CTX);
	vmovq %xmm7, cmll_sub(28, CTX);
	vmovq %xmm8, cmll_sub(29, CTX);
	vmovq %xmm9, cmll_sub(30, CTX);
	vmovq %xmm10, cmll_sub(31, CTX);
	vmovq %xmm11, cmll_sub(32, CTX);

	/* kw2 and kw4 are unused now. */
	movq $0, cmll_sub(1, CTX);
	movq $0, cmll_sub(33, CTX);

	vzeroall;

	ret_spec_stop;
	CFI_ENDPROC();
ELF(.size __camellia_avx_setup256,.-__camellia_avx_setup256;)

.align 8
.globl _gcry_camellia_aesni_avx_keygen
ELF(.type  _gcry_camellia_aesni_avx_keygen,@function;)

_gcry_camellia_aesni_avx_keygen:
	/* input:
	 *	%rdi: ctx, CTX
	 *	%rsi: key
	 *	%rdx: keylen
	 */
	CFI_STARTPROC();

	vzeroupper;

	vmovdqu (%rsi), %xmm0;
	cmpl $24, %edx;
	jb __camellia_avx_setup128;
	je .Lprepare_key192;

	vmovdqu 16(%rsi), %xmm1;
	jmp __camellia_avx_setup256;

.Lprepare_key192:
	vpcmpeqd %xmm2, %xmm2, %xmm2;
	vmovq 16(%rsi), %xmm1;

	vpxor %xmm1, %xmm2, %xmm2;
	vpslldq $8, %xmm2, %xmm2;
	vpor %xmm2, %xmm1, %xmm1;

	jmp __camellia_avx_setup256;
	CFI_ENDPROC();
ELF(.size _gcry_camellia_aesni_avx_keygen,.-_gcry_camellia_aesni_avx_keygen;)

#endif /*defined(ENABLE_AESNI_SUPPORT) && defined(ENABLE_AVX_SUPPORT)*/
#endif /*__x86_64*/
