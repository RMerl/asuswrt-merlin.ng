/*
 * bcm_ec.c
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *   <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bcm_ec.h>

typedef union {
	uint64 f;
	uint32 h[2];
} uu64;

#if _BYTE_ORDER == _LITTER_ENDIAN
#define uHI     U.h[1]
#define uLO     U.h[0]
#else
#define uHI     U.h[0]
#define uLO     U.h[1]
#endif // endif

#define uDW     U.f

/*       ************************************************************
 *                 E C P   9 - i n t e g e r   M u l t i p l y
 *        **********************************************************
 */

/*  256 bits as 9 integers in bits: (29, 28, 29, 28, 28, 29, 28, 28 29) */

static const uint32 hi29 = 0x1fffffff;
static const uint32 hi28 = 0x0fffffff;

static  /*  converts from  8  32-bit integers to  9  28-29 bit integers  */
void convert9(uint32* a)
{
	a[0] = a[1] >> 3; // 29
	a[1] = (a[1] << 25 | a[2] >> 7) & hi28; // 25 = 28 - 3     7 = 32 - 25
	a[2] = (a[2] << 22 | a[3] >> 10) & hi29; // 22 = 29 - 7    10 = 32 - 22
	a[3] = (a[3] << 18 | a[4] >> 14) & hi28;
	a[4] = (a[4] << 14 | a[5] >> 18) & hi28;
	a[5] = (a[5] << 11 | a[6] >> 21) & hi29;
	a[6] = (a[6] << 7 | a[7] >> 25) & hi28;
	a[7] = (a[7] << 3 | a[8] >> 29) & hi28;
	a[8] &= hi29;
}

static  /* converts from  9 integers to  8 integers  */
void convert8(uint32* a)
{
	a[8] |= a[7] << 29; // 29 + 0 = 29 + 3 = 32
	a[7] = a[7] >> 3 | a[6] << 25; // 28 - 3 = 25 + 7 = 32
	a[6] = a[6] >> 7 | a[5] << 21; // 28 - 7 = 21 + 11 = 32
	a[5] = a[5] >> 11 | a[4] << 18; // 29 - 11 = 18 + 14 = 32
	a[4] = a[4] >> 14 | a[3] << 14; // 28 - 14 = 14 + 18 = 32
	a[3] = a[3] >> 18 | a[2] << 10; // 28 - 18 = 10 + 22 = 32
	a[2] = a[2] >> 22 | a[1] << 7; // 29 - 22 = 7 + 25 = 32
	a[1] = a[1] >> 25 | a[0] << 3; // 28 - 25 = 3 + 29 = 32
	a[0] = 0;
}

static
void square_256(uint64* q, uint32* a)
{
	q[16] = (uint64)a[8] * a[8];	// pos.   0                       0     4*1  =  4

	q[15] = (uint64)a[8] * a[7] << 1;	 //  29                      29     2*2  =  4

	q[14] = ((uint64)a[8] * a[6] +	     //  57                      57     2*2 +
			(uint64)a[7] * a[7]) << 1;   //  58 --> 57    29 +  29 = 58     1*2  =  6

	q[13] = ((uint64)a[8] * a[5] << 1) + //  85                      85     2*4 +
			((uint64)a[7] * a[6] << 2);  //  86 --> 85    29 +  57 = 86     2*2  = 12

	q[12] = (((uint64)a[8] * a[4] +      // 114                      114    2*2 +
			(uint64)a[7] * a[5]) << 1) + // 114           29 +  85 = 114    2*2 +
			(uint64)a[6] * a[6];         // 114           57 +  57 = 114    1   =   9

	q[11] = (((uint64)a[8] * a[3] +      // 142                      142    2*2 +
			(uint64)a[6] * a[5]) << 1) + // 142           57 +  85 = 142    2*2 +
			((uint64)a[7] * a[4] << 2);  // 143 --> 142   29 + 114 = 143    2*2  = 12

	q[10] = ((uint64)a[8] * a[2] << 1) + // 170                      170    2*4 +
			(((uint64)a[7] * a[3] +      // 171 --> 170   29 + 142 = 171    2*2 +
			(uint64)a[6] * a[4]) << 2) + // 171 --> 170   57 + 114 = 171    2*2 +
			(uint64)a[5] * a[5];         // 170           85 +  85 = 170    1*4  = 20

	q[9] =  ((uint64)a[8] * a[1] +       // 199                      199    2*2 +
			(uint64)a[7] * a[2] +        // 199           29 + 170 = 199    2*2 +
			(uint64)a[6] * a[3] +        // 199           57 + 142 = 199    1*2 +
			(uint64)a[5] * a[4]) << 1;   // 199           85 + 114 = 199    2*2  = 14

	q[8] =  (((uint64)a[8] * a[0] +      // 227                      227    2*2 +
			(uint64)a[6] * a[2] +        // 227           57 + 170 = 227    2*2 +
			(uint64)a[5] * a[3] +        // 227           85 + 142 = 227    2*2 +
			(uint64)a[4] * a[4]) << 1) + // 228 --> 227  114 + 114 = 228    1*2 +
			((uint64)a[7] * a[1] << 2);  // 228 --> 227   29 + 199 = 228    1*4   = 18

	q[7] = (((uint64)a[7] * a[0] +       // 256 --> 255   29 + 227 = 256    2*2 +
			(uint64)a[6] * a[1] +        // 256           57 + 199 = 256    2*2 +
			(uint64)a[4] * a[3]) << 2) + // 256          114 + 142 = 256    2*2 +
			((uint64)a[5] * a[2] << 1);   // 255           85 + 170 = 255    2*4   = 20

	q[6] =  (((uint64)a[6] * a[0] +      // 284           57 + 227 = 284    2*1 +
			(uint64)a[5] * a[1] +        // 284           85 + 199 = 284    2*2 +
			(uint64)a[4] * a[2]) << 1) + // 284          114 + 170 = 284    2*2 +
			(uint64)a[3] * a[3];         // 284          142 + 142 = 284    1     = 11

	q[5] =  (((uint64)a[5] * a[0] +      // 312           85 + 227 = 312    2*2 +
			(uint64)a[3] * a[2]) << 1) + // 312          142 + 170 = 312    2*2 +
			((uint64)a[4] * a[1]<< 2);   // 313 --> 312  114 + 199 = 313    2*2   = 12

	q[4] = (((uint64)a[4] * a[0] +       // 341 --> 340  114 + 227 = 341    2*2 +
			(uint64)a[3] * a[1]) << 2) + // 341          142 + 199 = 341    2*2 +
			(uint64)a[2] * a[2];         // 340          170 + 170 = 340    1*4   = 12

	q[3] =  ((uint64)a[3] * a[0] +       // 369          142 + 227 = 369    1*2 +
			(uint64)a[2] * a[1]) << 1;   // 369          170 + 199 = 369    2*2  =  6

	q[2] =  ((uint64)a[2] * a[0] +       // 397          170 + 227 = 397    2*2 +
			(uint64)a[1] * a[1]) << 1;   // 398 --> 397  199 + 199 = 398    1*2  =  6

	q[1] =	(uint64)a[1] * a[0] << 2;    // 426 --> 425  199 + 227 = 426    2*2  =  4

	q[0] =  (uint64)a[0] * a[0];         // 454          227 + 227 = 454    1*1  =  1
}

static
void multiply_256(uint64* q, uint32* a, uint32* b)
{
	q[16] = (uint64)a[8] * b[8];	// pos. 0                       0     4*1  =  4

	q[15] = (uint64)a[8] * b[7] +
			(uint64)a[7] * b[8];    //     29                      29     2*2

	q[14] = (uint64)a[8] * b[6] +   //     57                      57     2*2 +
			(uint64)a[6] * b[8] +
			(uint64)a[7] * b[7] * 2;   //  58 --> 57     29 + 29 = 58     1*2  =  6

	q[13] = (uint64)a[8] * b[5] +   //     85                      85     2*4 +
			(uint64)a[5] * b[8] +
			((uint64)a[7] * b[6] +  //     86 --> 85     29 + 57 = 86     2*2  = 12
			(uint64)a[6] * b[7]) * 2;

	q[12] = (uint64)a[8] * b[4] +   //    114                      114    2*2 +
			(uint64)a[4] * b[8] +
			(uint64)a[7] * b[5] +   //    114            29 + 85 = 114    2*2 +
			(uint64)a[5] * b[7] +
			(uint64)a[6] * b[6];    //    114            57 + 57 = 114    1    =  9

	q[11] = (uint64)a[8] * b[3] +   //    142                      142    2*2 +
			(uint64)a[3] * b[8] +
			((uint64)a[7] * b[4] +  //    143 --> 142   29 + 114 = 143    2*2 +
			(uint64)a[4] * b[7]) * 2 +
			(uint64)a[6] * b[5] +   //    142           57 +  85 = 142    2*2  = 12
			(uint64)a[5] * b[6];

	q[10] = (uint64)a[8] * b[2] +  //     170                      170    2*4 +
			(uint64)a[2] * b[8] +
			((uint64)a[7] * b[3] + //     171 --> 170   29 + 142 = 171    2*2 +
			(uint64)a[3] * b[7] +
			(uint64)a[6] * b[4] +  //     171 --> 170   57 + 114 = 171    2*2 +
			(uint64)a[4] * b[6]) * 2 +
			(uint64)a[5] * b[5];   //     170           85 +  85 = 170    1*4  = 20

	q[9] =  (uint64)a[8] * b[1] + //      199                      199    2*2 +
			(uint64)a[1] * b[8] +
			(uint64)a[7] * b[2] + //      199           29 + 170 = 199    2*2 +
			(uint64)a[2] * b[7] +
			(uint64)a[6] * b[3] + //      199           57 + 142 = 199    1*2 +
			(uint64)a[3] * b[6] +
			(uint64)a[5] * b[4] + //      199           85 + 114 = 199    2*2  = 14
			(uint64)a[4] * b[5];

	q[8] =  (uint64)a[8] * b[0] + //      227                      227    2*2 +
			(uint64)a[0] * b[8] +
			((uint64)a[7] * b[1] + //     228 --> 227   29 + 199 = 228    2*2 +
			(uint64)a[1] * b[7] +
			(uint64)a[4] * b[4]) * 2 + // 228          114 + 114 = 228    1*2 +
			(uint64)a[6] * b[2] + //      227           57 + 170 = 227    2*2 +
			(uint64)a[2] * b[6] +
			(uint64)a[5] * b[3] + //      227           85 + 142 = 227    2*2  = 18
			(uint64)a[3] * b[5];

	q[7] = ((uint64)a[7] * b[0] + //      256 --> 255   29 + 227 = 256    2*2 +
			(uint64)a[0] * b[7] +
			(uint64)a[6] * b[1] + //      256           57 + 199 = 256    2*2 +
			(uint64)a[1] * b[6] +
			(uint64)a[3] * b[4] + //      256          114 + 142 = 256    2*2 +
			(uint64)a[4] * b[3]) * 2 +
			(uint64)a[2] * b[5] + //      255           85 + 170 = 255    2*4  = 20
			(uint64)a[5] * b[2];

	q[6] =  (uint64)a[6] * b[0] + //      284           57 + 227 = 284    2*1 +
			(uint64)a[0] * b[6] +
			(uint64)a[5] * b[1] + //      284           85 + 199 = 284    2*2 +
			(uint64)a[1] * b[5] +
			(uint64)a[4] * b[2] + //      284          114 + 170 = 284    2*2 +
			(uint64)a[2] * b[4] +
			(uint64)a[3] * b[3];  //      283          142 + 142 = 284    1    = 11

	q[5] =  (uint64)a[5] * b[0] + //      312           85 + 227 = 312    2*2 +
			(uint64)a[0] * b[5] +
			((uint64)a[4] * b[1] + //     313 --> 312  114 + 199 = 313    2*2 +
			(uint64)a[1] * b[4]) * 2 +
			(uint64)a[3] * b[2] + //      312          142 + 170 = 312    2*2  = 12
			(uint64)a[2] * b[3];

	q[4] = ((uint64)a[4] * b[0] + //      341 --> 340  114 + 227 = 341    2*2 +
			(uint64)a[0] * b[4] +
			(uint64)a[3] * b[1] + //      341          142 + 199 = 341    2*2 +
			(uint64)a[1] * b[3]) * 2 +
			(uint64)a[2] * b[2];  //      340          170 + 170 = 340    1*4  = 12

	q[3] =  (uint64)a[3] * b[0] + //      369          142 + 227 = 369    1*2 +
			(uint64)a[0] * b[3] +
			(uint64)a[2] * b[1] + //      369          170 + 199 = 369    2*2  =  6
			(uint64)a[1] * b[2];

	q[2] =  (uint64)a[2] * b[0] + //      397          170 + 227 = 397    2*2 +
			(uint64)a[0] * b[2] +
			(uint64)a[1] * b[1] * 2; //   398 --> 397  199 + 199 = 398    1*2  =  6

	q[1] = ((uint64)a[1] * b[0] + //      426 --> 425  199 + 227 = 426    2*2  =  4
			(uint64)a[0] * b[1]) * 2;

	q[0] =  (uint64)a[0] * b[0]; //       454          227 + 227 = 454    1*1  =  1
}

static
void reduce_256(uint64* t, uint32* a)
{
	uu64 U;
	uDW = t[16] + (uint32)(t[15] << 29);
	a[15] = uLO;
	uDW = (t[15] >>  3) + (uint32)(t[14] << 25) + uHI;
	a[14] = uLO;
	uDW = (t[14] >>  7) + (uint32)(t[13] << 21) + uHI;
	a[13] = uLO;
	uDW = (t[13] >> 11) + (uint32)(t[12] << 18) + uHI;
	a[12] = uLO;
	uDW = (t[12] >> 14) + (uint32)(t[11] << 14) + uHI;
	a[11] = uLO;
	uDW = (t[11] >> 18) + (uint32)(t[10] << 10) + uHI;
	a[10] = uLO;
	uDW = (t[10] >> 22) + (uint32)(t[9] << 7) + uHI;
	a[9] = uLO;
	uDW = (t[9] >> 25) + (uint32)(t[8] << 3) + (uint32)(t[7] << 31) + uHI;
	a[8] = uLO;
	uDW = (t[8] >> 29) + (t[7] >> 1) + (uint32)(t[6] << 28) + uHI; //256 - 284 = -28
	a[7] = uLO;
	uDW = (t[6] >>  4) + (uint32)(t[5] << 24) + uHI; //288 - 312 = -24;  288 - 284 = 4
	a[6] = uLO;
	uDW = (t[5] >>  8) + (uint32)(t[4] << 20) + uHI; //320 - 340 = -20;  320 - 312 = 8
	a[5] = uLO;
	uDW = (t[4] >> 12) + (uint32)(t[3] << 17) + uHI; //352 - 369 = -17;  352 - 340 = 12
	a[4] = uLO;
	uDW = (t[3] >> 15) + (uint32)(t[2] << 13) + uHI; //384 - 397 = -13;  384 - 369 = 15
	a[3] = uLO;
	uDW = (t[2] >> 19) + (uint32)(t[1] << 9) + uHI; // 416 - 425 = -9;   416 - 397 = 19
	a[2] = uLO;
	uDW = (t[1] >> 23) + (uint32)(t[0] << 6) + uHI; // 448 - 454 = -6;   448 - 425 = 23
	a[1] = uLO;
	uDW = (t[0] >> 26) + uHI; // 480 - 454 = 26
	a[0] = uLO;
}

static
void reduce_25519(uint64 *t, uint32 *a)
{
	uint64 R;
	t[9] += t[0] * 19;
	t[10] += t[1] * 19;
	t[11] += t[2] * 19;
	t[12] += t[3] * 19;
	t[13] += t[4] * 19;
	t[14] += t[5] * 19;
	t[15] += t[6] * 19;

	t[7] += (t[8] >> 28) + (t[9] >> 56);
	t[16] += t[7] << 3;
	t[16] += R = t[7] * 11;
	if (t[16] < R)
		t[14] += 128;

	a[8] = t[16] & hi29;

	R = (t[15] & hi28) + (t[16] >> 29);
	a[7] = R & hi28;

	R = (R >> 28) + (t[14] & hi28) + (t[15] >> 28);
	a[6] = R & hi28;

	R = (R >> 28) + (t[13] & hi29) + (t[14] >> 28);
	a[5] = R & hi29;

	R = (R >> 29) + (t[12] & hi28) + (t[13] >> 29);
	a[4] = R & hi28;

	R = (R >> 28) + (t[11] & hi28) + (t[12] >> 28);
	a[3] = R & hi28;

	R = (R >> 28) + (t[10] & hi29) + (t[11] >> 28);
	a[2] = R & hi29;

	R = (R >> 29) + (t[9] & hi28) + (t[10] >> 29);
	a[1] = R & hi28;

	R = (R >> 28) + (t[8] & hi28) + ((t[9] >> 28) & hi28);
	a[0] = (uint32)R;

	if (a[0] > hi28) {
		a[0] &= hi28;
		a[8] += 19;
		if (a[8] > hi29) {
			a[8] &= hi29;
			a[7]++;
			if (a[7] == hi28) {
				a[7] = 0;
				a[6]++;  // for now
			}
		}
	}
}

/*		**********************************************
 *		           E C P   S p e c i f i c
 *		**********************************************
 */

static
void add_word(uint32 *a, uint32 val)
{
	uu64 U;
	uDW = (uint64)*a + val;
	*a = uLO;
	while (uHI) {
		a--;
		uDW = (uint64)uHI + *a;
		*a = uLO;
	}
}

static
void sub_word(uint32 *a, uint32 val)
{
	uu64 U;
	uDW = (uint64)*a - val;
	*a = uLO;
	while (uHI) {
		a--;
		uLO = uHI;
		uDW += (uint64)*a;
		*a = uLO;
	}
}

static
void carry_521P(uint32* b, const uint32 k)
{
	b[0] &= 0x1ff;
	add_word(b + 16, k);
}

#define T521(j, k)	uDW = uHI + ((uint64)t[j] << 23) + t[k]; a[j] = uLO

static
void reduce_521P(const bn_t* tt, bn_t* aa)
{
	uint32 t[34], a[17];
	uu64 U;
	bn_get(tt, BN_FMT_LE, (uint8 *)t, sizeof(t));
	uDW = 0;
	T521(16, 33);
	T521(15, 32);
	T521(14, 31);
	T521(13, 30);
	T521(12, 29);
	T521(11, 28);
	T521(10, 27);
	T521(9, 26);
	T521(8, 25);
	T521(7, 24);
	T521(6, 23);
	T521(5, 22);
	T521(4, 21);
	T521(3, 20);
	T521(2, 19);
	T521(1, 18);
	T521(0, 17);
	carry_521P(a, uHI << 23 | a[0] >> 9);
	bn_set(aa, BN_FMT_LE, (uint8 *)a, sizeof(a));
}

static
void multiply_521P(bn_t* a, const bn_t* b, const bn_t* c,  bn_t* mm)
{
	bn_mul(mm, b, c, 0);
	reduce_521P(mm, a);
}

static
void square_521P(bn_t* a, const bn_t* b, bn_t* mm)
{
	bn_sqr(mm, b, 0);
	reduce_521P(mm, a);
}

static
void add_521P(bn_t* a, const bn_t* b, const bn_t* c)
{
	bn_add(a, b, c, 0);
	bn_truncate(a, 23);
	if (bn_cmp(a, c) < 0)
		bn_iadd(a, a, 1, 0);
}

static
void sub_521P(bn_t *a, const bn_t* b, const bn_t* c)
{
	bool borrow = bn_cmp(b, c) < 0;
	bn_sub(a, b, c, 0);
	if (borrow) {
		bn_truncate(a, 23);
		bn_iadd(a, a, -1, 0);
	}
}

static
void mul_521P(bn_t* a, const bn_t* b, uint32 n)  /* n = 4 or 8 */
{
	uint32 bb, aa[17];
	bn_imul(a, b, n, 0);
	bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	bb = *aa >> 9;
	if (bb) {
		bn_truncate(a, 23);
		bn_iadd(a, a, bb, 0);
	}
}

static
void carry_384P(uint32* b, const uint32 k)
{
	add_word(b + 7, k);
	add_word(b + 8, k);
	sub_word(b + 10, k);
	add_word(b + 11, k);
}

static
void borrow_384P(uint32* b, const uint32 k)
{
	sub_word(b + 7, k);
	sub_word(b + 8, k);
	add_word(b + 10, k);
	sub_word(b + 11, k);
}

#define TT(k)  a[k] = uLO; uLO = uHI; uHI = (uHI > 100) ? minus1 : 0

static  /*  a = t % Modulus    length(a) = SZ  length(t) = 2*SZ    */
void reduce_384P(const bn_t* tt, bn_t* aa)
{
	uint32 t[24], a[12];
	uint32 minus1 = 0xffffffff;
	uu64 U;
	bn_get(tt, BN_FMT_LE, (uint8 *)t, sizeof(t));

	uDW = 0;
	uDW += (uint64)t[23] + t[11] + t[3] + t[2] - t[0]; TT(11);
	uDW += (uint64)t[22] - t[11] + t[10] - t[3] + t[1] + t[0]; TT(10);
	uDW += (uint64)t[21] - t[10] + t[9] - t[2] + t[0]; TT(9);
	uDW += (uint64)t[20] + t[11] - t[9] + t[8] + t[3] + t[2] - t[1] - t[0]; TT(8);
	uDW += (uint64)t[19] + t[11] + t[10] - t[8] + t[7] + t[3] + t[2] + t[2] + t[1]
		- t[0] - t[0]; TT(7);
	uDW += (uint64)t[18] + t[10] + t[9] - t[7] + t[6] + t[2] + t[1] + t[1] + t[0]; TT(6);
	uDW += (uint64)t[17] + t[9] + t[8] - t[6] + t[5] + t[1] + t[0] + t[0]; TT(5);
	uDW += (uint64)t[16] + t[8] + t[7] - t[5] + t[4] + t[0]; TT(4);
	uDW += (uint64)t[15] + t[7] + t[6] - t[4] + t[3]; TT(3);
	uDW += (uint64)t[14] + t[6] + t[5] - t[3] + t[2]; TT(2);
	uDW += (uint64)t[13] + t[5] + t[4] - t[2] + t[1]; TT(1);
	uDW += (uint64)t[12] + t[4] + t[3] - t[1] + t[0]; TT(0);
	if (uHI) {              /*  a negative value  */
		uLO = ~uLO + 1;
		borrow_384P(a, uLO);
	} else
		carry_384P(a, uLO);

	bn_set(aa, BN_FMT_LE, (uint8 *)a, sizeof(a));
}

static
void multiply_384P(bn_t* a, const bn_t* b, const bn_t* c,  bn_t* mm)
{
	bn_mul(mm, b, c, 0);
	reduce_384P(mm, a);
}

static
void square_384P(bn_t* a, const bn_t* b, bn_t* mm)
{
	bn_sqr(mm, b, 0);
	reduce_384P(mm, a);
}

static
void add_384P(bn_t *a, const bn_t* b, const bn_t* c)
{
	bn_add(a, b, c, 0);
	if (bn_cmp(a, c) < 0) {
		uint32 aa[12];
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		carry_384P(aa, 1);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void sub_384P(bn_t *a, const bn_t* b, const bn_t* c)
{
	bool borrow = bn_cmp(b, c) < 0;
	bn_sub(a, b, c, 0);
	if (borrow) {
		uint32 aa[12];
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		borrow_384P(aa, 1);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void mul_384P(bn_t *a, const bn_t *b, uint32 n)  /* n = 4 or 8 */
{
	uint32 nn, aa[12], bb[12];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	nn = (n == 4) ? bb[0] >> 30 : bb[0] >> 29;
	bn_imul(a, b, n, 0);
	if (nn) {
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		carry_384P(aa, nn);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void carry_256P(uint32* b, const uint32 k)
{
	add_word(b, k);
	sub_word(b + 1, k);
	sub_word(b + 4, k);
	add_word(b + 7, k);
}

static
void borrow_256P(uint32* b, const uint32 k)
{
	sub_word(b, k);
	add_word(b + 1, k);
	add_word(b + 4, k);
	sub_word(b + 7, k);
}

static  /*  a = t % Modulus    length(a) = SZ  length(t) = 2*SZ    */
void reduce_256P(const uint32* t, uint32* a)
{
	//uint32 t[16], a[8];
	uint32 minus1 = 0xffffffff;
	uu64 U;
	//bn_get(tt, BN_FMT_LE, (uint8 *)t, sizeof(t));

	uDW = 0;
	uDW += (uint64)t[15] + t[7] + t[6] - t[4] - t[3] - t[2] - t[1]; TT(7);
	uDW += (uint64)t[14] + t[6] + t[5] - t[3] - t[2] - t[1] - t[0]; TT(6);
	uDW += (uint64)t[13] + t[5] + t[4] - t[2] - t[1] - t[0]; TT(5);
	uDW += (uint64)t[12] - t[7] - t[6] + t[4] + t[4] + t[3] + t[3] + t[2] - t[0]; TT(4);
	uDW += (uint64)t[11] - t[6] - t[5] + t[3] + t[3] + t[2] + t[2] + t[1]; TT(3);
	uDW += (uint64)t[10] - t[5] - t[4] + t[2] + t[2] + t[1] + t[1] + t[0]; TT(2);
	uDW += (uint64)t[9] - t[7] - t[6] + t[2] + t[1] + t[1] + t[1] + t[0] + t[0]; TT(1);
	uDW += (uint64)t[8] + t[7] - t[5] - t[4] - t[3] - t[2] + t[0] + t[0] + t[0]; TT(0);
	if (uHI) {              /*  a negative value  */
		uLO = ~uLO + 1;
		borrow_256P(a, uLO);
	} else
		carry_256P(a, uLO);

	//bn_set(aa, BN_FMT_LE, (uint8 *)a, sizeof(a));
}

static
void multiply_256P(bn_t* a, const bn_t* b, const bn_t* c,  bn_t* mm)
{
	uint32 aa[16], bb[9], cc[9], t[8];
	uint64 tt[17];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	convert9(bb);
	bn_get(c, BN_FMT_LE, (uint8 *)cc, sizeof(cc));
	convert9(cc);
	multiply_256(tt, bb, cc);
	reduce_256(tt, aa);
	reduce_256P(aa, t);
	bn_set(a, BN_FMT_LE, (uint8 *)t, sizeof(t));
}

static
void square_256P(bn_t* a, const bn_t* b, bn_t* mm)
{
	uint32 aa[16], bb[9], t[8];
	uint64 tt[17];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	convert9(bb);
	square_256(tt, bb);
	reduce_256(tt, aa);
	reduce_256P(aa, t);
	bn_set(a, BN_FMT_LE, (uint8 *)t, sizeof(t));
}

static
void add_256P(bn_t *a, const bn_t *b, const bn_t *c)
{
	bn_add(a, b, c, 0);
	if (bn_cmp(a, c) < 0) {
		uint32 aa[8];
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		carry_256P(aa, 1);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void sub_256P(bn_t *a, const bn_t *b, const bn_t* c)
{
	bool borrow = bn_cmp(b, c) < 0;
	bn_sub(a, b, c, 0);
	if (borrow) {
		uint32 aa[8];
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		borrow_256P(aa, 1);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void mul_256P(bn_t *a, const bn_t *b, uint32 n)  /* n = 4 or 8 */
{
	uint32 nn, aa[8], bb[8];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	nn = (n == 4) ? bb[0] >> 30 : bb[0] >> 29;
	bn_imul(a, b, n, 0);
	if (nn) {
		bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
		carry_256P(aa, nn);
		bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	}
}

static
void multiply_25519(bn_t* a, const bn_t* b, const bn_t* c,  bn_t* mm)
{
	uint32 aa[9], bb[9], cc[9];
	uint64 tt[17];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	convert9(bb);
	bn_get(c, BN_FMT_LE, (uint8 *)cc, sizeof(cc));
	convert9(cc);
	multiply_256(tt, bb, cc);
	reduce_25519(tt, aa);
	convert8(aa);
	bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
}

static
void square_25519(bn_t* a, const bn_t* b, bn_t* mm)
{
	uint32 aa[9], bb[9];
	uint64 tt[17];
	bn_get(b, BN_FMT_LE, (uint8 *)bb, sizeof(bb));
	convert9(bb);
	square_256(tt, bb);
	reduce_25519(tt, aa);
	convert8(aa);
	bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
}

static
void add_25519(bn_t* a, const bn_t* b, const bn_t* c)
{
	bn_add(a, b, c, 0);
	bn_truncate(a, 1);
	if (bn_cmp(a, c) < 0)
		bn_iadd(a, a, 19, 0);
}

static
void sub_25519(bn_t *a, const bn_t* b, const bn_t* c)
{
	bool borrow = bn_cmp(b, c) < 0;
	bn_sub(a, b, c, 0);
	if (borrow) {
		bn_truncate(a, 1);
		bn_iadd(a, a, -19, 0);
	}
}

static
void mul_25519(bn_t *a, const bn_t *b, uint32 a4)  /* a = b * A4  a = lengh 9,  b = length 8 */
{
	uint32 n, aa[9];
	bn_imul(a, b, a4, 0);
	bn_get(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	n = aa[0] * 38;
	aa[0] = 0;
	if (aa[1] & 0x80000000) {
		aa[1] ^= 0x80000000;
		n += 19;
	}
	//add_word(aa + 8, n);
	//bn_set(a, BN_FMT_LE, (uint8 *)aa, sizeof(aa));
	bn_truncate(a, 33);
	bn_iadd(a, a, n, 0);
}

/*		**********************************************************************************
 *					e c g    c o n s t a n t s
 *		**********************************************************************************
 */

static
const int coef25519[3] = {254, -3, 2};

static
const uint8 mod25519[] = {
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed
};

static
const uint8 ord25519[] = {
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x14, 0xde, 0xf9, 0xde, 0xa2, 0xf7, 0x9c, 0xd6,
	0x58, 0x12, 0x63, 0x1a, 0x5c, 0xf5, 0xd3, 0xed,
};

static
const uint8 B_25519[] = {  /* this is a root of -1 */
	0x2b, 0x83, 0x24, 0x80, 0x4f, 0xc1, 0xdf, 0x0b,
	0x2b, 0x4d, 0x00, 0x99, 0x3d, 0xfb, 0xd7, 0xa7,
	0x2f, 0x43, 0x18, 0x06, 0xad, 0x2f, 0xe4, 0x78,
	0xc4, 0xee, 0x1b, 0x27, 0x4a, 0x0e, 0xa0, 0xb0
};

static
const uint32 Gx_25519[] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000009
};

static
const uint32 Gy_25519[] = {
	0x20ae19a1, 0xb8a086b4, 0xe01edd2c, 0x7748d14c,
	0x923d4d7e, 0x6d7c61b2, 0x29e9c5a2, 0x7eced3d9
};

static
const int coef256[5] = {256, -224, 192, 96, 2};

static
const uint8 mod256[] = {
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static
const uint8 ord256[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
	0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};

static
const uint8 B_256[] = {
	0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7,
	0xB3, 0xEB, 0xBD, 0x55, 0x76, 0x98, 0x86, 0xBC,
	0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6,
	0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B
};

static
const uint32 Gx_256[] = {
	0x6B17D1F2, 0xE12C4247, 0xF8BCE6E5,
	0x63A440F2, 0x77037D81,
	0x2DEB33A0, 0xF4A13945, 0xD898C296
};

static
const uint32 Gy_256[] = {
	0x4FE342E2, 0xFE1A7F9B, 0x8EE7EB4A,
	0x7C0F9E16, 0x2BCE3357,
	0x6B315ECE, 0xCBB64068, 0x37BF51F5
};

static
const int coef384[5] = {384, -128, -96, 32, 2};

static
const uint8 mod384[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};

static
const uint8 ord384[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC7, 0x63, 0x4D, 0x81, 0xF4, 0x37, 0x2D, 0xDF,
	0x58, 0x1A, 0x0D, 0xB2, 0x48, 0xB0, 0xA7, 0x7A,
	0xEC, 0xEC, 0x19, 0x6A, 0xCC, 0xC5, 0x29, 0x73
};

static
const uint8 B_384[] = {
	0xB3, 0x31, 0x2F, 0xA7, 0xE2, 0x3E, 0xE7, 0xE4,
	0x98, 0x8E, 0x05, 0x6B, 0xE3, 0xF8, 0x2D, 0x19,
	0x18, 0x1D, 0x9C, 0x6E, 0xFE, 0x81, 0x41, 0x12,
	0x03, 0x14, 0x08, 0x8F, 0x50, 0x13, 0x87, 0x5A,
	0xC6, 0x56, 0x39, 0x8D, 0x8A, 0x2E, 0xD1, 0x9D,
	0x2A, 0x85, 0xC8, 0xED, 0xD3, 0xEC, 0x2A, 0xEF
};

static
const uint32 Gx_384[] = {
	0xAA87CA22, 0xBE8B0537, 0x8EB1C71E, 0xF320AD74,
	0x6E1D3B62, 0x8BA79B98, 0x59F741E0, 0x82542A38,
	0x5502F25D, 0xBF55296C, 0x3A545E38, 0x72760AB7
};

static
const uint32 Gy_384[] = {
	0x3617DE4A, 0x96262C6F, 0x5D9E98BF, 0x9292DC29,
	0xF8F41DBD, 0x289A147C, 0xE9DA3113, 0xB5F0B8C0,
	0x0A60B1CE, 0x1D7E819D, 0x7A431D7C, 0x90EA0E5F
};

static
const int coef521[2] = {521, 2};

static
const uint8 mod521[] = {0, 0,
	0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF
};

static
const uint8 ord521[] = {0, 0,
	0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFA, 0x51, 0x86, 0x87, 0x83, 0xBF, 0x2F,
	0x96, 0x6B, 0x7F, 0xCC, 0x01, 0x48, 0xF7, 0x09,
	0xA5, 0xD0, 0x3B, 0xB5, 0xC9, 0xB8, 0x89, 0x9C,
	0x47, 0xAE, 0xBB, 0x6F, 0xB7, 0x1E, 0x91, 0x38,
	0x64, 0x09
};

static
const uint8 B_521[] = {0, 0,
	0x00, 0x51, 0x95, 0x3E, 0xB9, 0x61, 0x8E, 0x1C,
	0x9A, 0x1F, 0x92, 0x9A, 0x21, 0xA0, 0xB6, 0x85,
	0x40, 0xEE, 0xA2, 0xDA, 0x72, 0x5B, 0x99, 0xB3,
	0x15, 0xF3, 0xB8, 0xB4, 0x89, 0x91, 0x8E, 0xF1,
	0x09, 0xE1, 0x56, 0x19, 0x39, 0x51, 0xEC, 0x7E,
	0x93, 0x7B, 0x16, 0x52, 0xC0, 0xBD, 0x3B, 0xB1,
	0xBF, 0x07, 0x35, 0x73, 0xDF, 0x88, 0x3D, 0x2C,
	0x34, 0xF1, 0xEF, 0x45, 0x1F, 0xD4, 0x6B, 0x50,
	0x3F, 0x00
};

static
const uint32 Gx_521[] = {0x00C6,
	0x858E06B7, 0x0404E9CD, 0x9E3ECB66, 0x2395B442,
	0x9C648139, 0x053FB521, 0xF828AF60, 0x6B4D3DBA,
	0xA14B5E77, 0xEFE75928, 0xFE1DC127, 0xA2FFA8DE,
	0x3348B3C1, 0x856A429B, 0xF97E7E31, 0xC2E5BD66
};

static
const uint32 Gy_521[] = {0x0118,
	0x39296A78, 0x9A3BC004, 0x5C8A5FB4, 0x2C7D1BD9,
	0x98F54449, 0x579B4468, 0x17AFBD17, 0x273E662C,
	0x97EE7299, 0x5EF42640, 0xC550B901, 0x3FAD0761,
	0x353C7086, 0xA272C240, 0x88BE9476, 0x9FD16650
};

struct ec_parms {
	ecg_type_t type;
	uint32 bit_len;
	uint8 len_prime;
	uint8 cofactor;
	void (*multiply)(bn_t* a, const bn_t* b, const bn_t* c, bn_t* m);
	void (*square)(bn_t* a, const bn_t* b, bn_t* m);
	void (*add)(bn_t* a, const bn_t* b, const bn_t* c);
	void (*sub)(bn_t *a, const bn_t* b, const bn_t* c);
	void (*mul)(bn_t *a, const bn_t* b, uint32 n);
	const int* coef;
	const uint8* prime;
	const uint8* order;
	const uint32* Gx;
	const uint32* Gy;
	int A;
	const uint8* B;
};

static const struct ec_parms ecp25519_parm = {
		ECG_25519, 255, 32, 8, multiply_25519, square_25519, add_25519, sub_25519,
		mul_25519, coef25519, mod25519, ord25519, Gx_25519, Gy_25519, 486662, B_25519
};

static const struct ec_parms ecp256_parm = {
		ECG_NIST_P256, 256, 32, 1, multiply_256P, square_256P, add_256P, sub_256P, mul_256P,
		coef256, mod256, ord256, Gx_256, Gy_256, -3, B_256
};

static const struct ec_parms ecp384_parm = {
		ECG_NIST_P384, 384, 48, 1, multiply_384P, square_384P, add_384P, sub_384P, mul_384P,
		coef384, mod384, ord384, Gx_384, Gy_384, -3, B_384
};

static const struct ec_parms ecp521_parm = {
		ECG_NIST_P521, 521, 68, 1, multiply_521P, square_521P, add_521P, sub_521P, mul_521P,
		coef521, mod521, ord521, Gx_521, Gy_521, -3, B_521
};

static
const struct ec_parms* get_parms(ecg_type_t type)
{
	if (type == ECG_NIST_P256)
		return &ecp256_parm;

	if (type == ECG_NIST_P384)
		return &ecp384_parm;

	if (type == ECG_NIST_P521)
		return &ecp521_parm;

	if (type == ECG_25519)
		return &ecp25519_parm;

	return 0;
}

/*		**********************************************************************************
 *					e c g    f u n c  t i o n s
 *		**********************************************************************************
 */

typedef struct {
	ecg_elt_t *base_elt;
	ecg_type_t type;
} ecg_s;

typedef struct {
	ecg_t *ec;
	bn_t *x;
	bn_t *y;
} ecg_elt_s;

static
int ecg_get_len_prime(ecg_type_t type)
{
	const struct ec_parms* ec = get_parms(type);
	return ec ? ec->len_prime : 0;
}

#define LEN_PRIME  ecg_get_len_prime(ecg_get_type(ecg))

#define BNX  bn_get_ctx((bn_t *)ecg)

#define BN_ALLOC(b)	bn_t* b = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME)

#define EC_PARMS  const struct ec_parms *ec = get_parms(type)

/* create an ec group -- allocate the base point on the bnx */
ecg_t* ecg_alloc(ecg_type_t type, ecg_alloc_fn_t alloc_fn, ecg_free_fn_t free_fn, void *ctx)
{
	const struct ec_parms *ec = get_parms(type);
	bn_ctx_t *bnx;
	ecg_t *ecg;
	ecg_s s;
	ecg_elt_s base;
	if (alloc_fn == NULL || free_fn == NULL || ctx == NULL || ec == NULL)
		return 0;

	bnx = bn_ctx_alloc(alloc_fn, free_fn, ctx, ec->bit_len);
	if (!bnx)
		return 0;

	ecg = (ecg_t *)bn_alloc(bnx, BN_FMT_BE, 0, sizeof(ecg_s));

	s.type = type;
	s.base_elt = (ecg_elt_t *)bn_alloc(bnx, BN_FMT_BE, 0, sizeof(ecg_elt_s));

	base.ec = ecg;
	base.x = bn_alloc(BNX, BN_FMT_LE, (uint8 *)ec->Gx, ec->len_prime);
	base.y = bn_alloc(BNX, BN_FMT_LE, (uint8 *)ec->Gy, ec->len_prime);
	bn_set((bn_t *)s.base_elt, BN_FMT_BE, (uint8 *)&base, sizeof(ecg_elt_s));

	bn_set((bn_t *)ecg, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_s));
	return ecg;
}

/* free an ecg, reset ecg to NULL */
void ecg_free(ecg_t **ecg)
{
	ecg_t *e = *ecg;
	bn_ctx_t *bnx = bn_get_ctx((bn_t *)e);
	ecg_s s;
	ecg_elt_s base;
	bn_get((bn_t *)e, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_s));
	bn_get((bn_t *)s.base_elt, BN_FMT_BE, (uint8 *)&base, sizeof(ecg_elt_s));

	ecg_elt_free(&s.base_elt);
	bn_free((bn_t **)ecg);
	bn_ctx_destroy(&bnx);
}

/* get the base element */
const ecg_elt_t* ecg_get_base_elt(const ecg_t *ecg)
{
	ecg_s s;
	bn_get((bn_t *)ecg, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_s));
	return s.base_elt;
}

/* get the ecg type */
ecg_type_t ecg_get_type(const ecg_t *ecg)
{
	ecg_s s;
	bn_get((bn_t *)ecg, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_s));
	return s.type;
}

/* get a bn parameter */
int ecg_get_bn_param(ecg_t *ecg, ecg_param_type_t type, bn_t *bn)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	if (!ec) {
		bn_set(bn, BN_FMT_BE, NULL, 0);
		return 0;
	}
	switch (type) {
		case ECG_PARAM_BN_PRIME:
			bn_set(bn, BN_FMT_BE, ec->prime, ec->len_prime);
			break;

		case ECG_PARAM_BN_ORDER:
			bn_set(bn, BN_FMT_BE, ec->order, ec->len_prime);
			break;

		case ECG_PARAM_BN_COFACTOR:
			if (bn)
				bn_set(bn, BN_FMT_BE, &ec->cofactor, 1);
			return ec->cofactor;

		case ECG_PARAM_BN_A:
			if (bn) {
				if (ec->A > 0)
					bn_set(bn, BN_FMT_BE, 0, 0);
				else
					bn_set(bn, BN_FMT_BE, ec->prime, ec->len_prime);

				bn_iadd(bn, bn, ec->A, 0);
			}
			return ec->A;

		case ECG_PARAM_BN_B:
			bn_set(bn, BN_FMT_BE, ec->B, ec->len_prime);
			break;

		case ECG_PARAM_BN_BIT_LEN:
			if (bn)
				bn_set(bn, BN_FMT_LE, (uint8 *)&ec->bit_len, 2);
			return ec->bit_len;

		case ECG_PARAM_BN_BYTE_LEN:
			if (bn)
				bn_set(bn, BN_FMT_BE, &ec->len_prime, 1);
			return ec->len_prime;
	}
	return 1;
}

/*		***********************************************************************************
 *					e c g _ e l t    f u n c  t i o n s
 *		***********************************************************************************
 */

/* allocate an ec group element */
ecg_elt_t* ecg_elt_alloc(ecg_t *ecg)
{
	ecg_elt_t *elt = (ecg_elt_t *)bn_alloc(BNX, BN_FMT_BE, 0, sizeof(ecg_elt_s));
	ecg_elt_s s;
	s.ec = ecg;
	s.x = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME);
	s.y = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME);
	bn_set((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	return elt;
}

/* free an ec group element */
void ecg_elt_free(ecg_elt_t **e)
{
	ecg_elt_t *elt = *e;
	ecg_elt_s s;
	bn_get((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	bn_free(&s.y);
	bn_free(&s.x);
	bn_free((bn_t **)e);
}

/* get ecg from elt */
ecg_t* ecg_elt_get_group(const ecg_elt_t *elt)
{
	ecg_elt_s s;
	bn_get((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	return s.ec;
}

/* get elt coordinates. x or y may be NULL */
void ecg_elt_get_xy(const ecg_elt_t *elt, const bn_t **x, const bn_t **y)
{
	ecg_elt_s s;
	bn_get((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	*x = s.x;
	*y = s.y;
}

/* initialize elt with coordiantes - may return error if not an elt */
int ecg_elt_init(ecg_elt_t *elt, const bn_t *x, const bn_t *y)
{
	ecg_elt_s s;
	bn_get((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	bn_copy(s.x, x);
	bn_copy(s.y, y);
	bn_set((bn_t *)elt, BN_FMT_BE, (uint8 *)&s, sizeof(ecg_elt_s));
	return 1;
}

/* equality */
bool ecg_elt_eq(const ecg_elt_t *e1, const ecg_elt_t *e2)
{
	const bn_t *b1x, *b1y, *b2x, *b2y;
	ecg_elt_get_xy(e1, &b1x, &b1y);
	ecg_elt_get_xy(e2, &b2x, &b2y);
	return (!bn_cmp(b1x, b2x) && !bn_cmp(b1y, b2y));
}

/* copy an element */
void ecg_elt_copy(ecg_elt_t *dst, const ecg_elt_t *src)
{
	const bn_t *bx, *by;
	ecg_elt_get_xy(src, &bx, &by);
	ecg_elt_set_xy(dst, bx, by);
}

/*		*********************************************************************************
 *					e c g _ e l t    a r i t h m e t i c
 *		*********************************************************************************
 */

static
void square_root_25519(ecg_t *ecg, bn_t *s, const bn_t *x)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	const int* coef = ec->coef;
	int n = 0, k = coef[0], kNext;
	uint32 sBase[9], s2[9], inv[9];
	uint64 tt[17];
	BN_ALLOC(xInv);
	BN_ALLOC(P);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	bn_inv(x, P, xInv);

	bn_get(x, BN_FMT_LE, (uint8 *)sBase, sizeof(sBase));
	convert9(sBase);
	bn_get(xInv, BN_FMT_LE, (uint8 *)inv, sizeof(inv));
	convert9(inv);
	memcpy(s2, sBase, sizeof(sBase));

	while (k > 2) {
		kNext = coef[++n];
		if (kNext < 0)
			kNext = -kNext;

		while (k > kNext) {
			square_256(tt, s2);
			reduce_25519(tt, s2);
			k--;
		}
		if (k > 2) {
			if (coef[n] < 0) {
				multiply_256(tt, s2, inv);
				reduce_25519(tt, s2);
			}
			else {
				multiply_256(tt, s2, sBase);
				reduce_25519(tt, s2);
			}
		}
	}
	convert8(s2);
	bn_set(s, BN_FMT_LE, (uint8 *)s2, sizeof(s2));
	bn_free(&P);
	bn_free(&xInv);
}

static   /*  x ^ (1/2) = x^((P+1) / 4)   */
void square_root(ecg_t *ecg, bn_t *s, const bn_t *x)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	const int* coef = ec->coef;
	int n = 0, k = coef[0], kNext;
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2*LEN_PRIME);
	BN_ALLOC(xInv);
	BN_ALLOC(P);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	bn_inv(x, P, xInv);

	bn_copy(s, x);
	while (k > 2) {
		kNext = coef[++n];
		if (kNext < 0)
			kNext = -kNext;

		while (k > kNext) {
			ec->square(s, s, mm);
			k--;
		}
		if (k > 2) {
			if (coef[n] < 0) {
				ec->multiply(s, s, xInv, mm);
			}
			else {
				ec->multiply(s, s, x, mm);
			}
		}
	}
	bn_free(&P);
	bn_free(&xInv);
	bn_free(&mm);
}

static  /* h  = x^3 - 3x + B */
void rand_get_H(bn_t* h, const bn_t* x, const bn_t* P, const bn_t* B)
{
	bn_sqr(h, x, P);			/* h = x^2 */
	bn_mul(h, h, x, P);			/* h = x^3 */
	bn_add(h, h, B, P);			/* h = x^3 + B */
	bn_sub(h, h, x, P);
	bn_sub(h, h, x, P);
	bn_sub(h, h, x, P);			/* h = x^3 - 3x + B */
}

static	/* x = B * (a+1) / (3 * a)  */
void rand_get_X(bn_t *x, bn_t* a, const bn_t* P, const bn_t* B)
{
	bn_imul(x, a, 3, P);	/* 3a */
	bn_inv(x, P, x);
	bn_iadd(a, a, 1, 0);	/* a + 1 */
	bn_mul(a, a, B, P);		/* B * (a + 1) */
	bn_mul(x, x, a, P);		/* B * (a+1) / 3a  */
}

static  /* point doubling -- no projection  */
void Double_Point(bn_t* x, bn_t* y, ecg_t *ecg)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2 * LEN_PRIME);
	BN_ALLOC(P);
	BN_ALLOC(s);
	BN_ALLOC(b1);
	BN_ALLOC(b2);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	ec->square(b1, x, mm);
	bn_imul(b2, b1, 3, P);  /* 3x ^ 2 */
	if (ec->type == ECG_25519) {
		bn_imul(b1, x, 2 * ec->A, P);
		bn_iadd(b1, b1, 1, 0); /* + 2Ax + 1 */
		ec->add(b2, b2, b1);
	} else
		bn_iadd(b2, b2, ec->A, 0); /* - 3 */

	ec->add(b1, y, y);
	bn_inv(b1, P, b1);
	ec->multiply(s, b1, b2, mm); /*   s = (3x^2 + (2Ax + 1 : -3)) / 2y   */

	ec->square(b1, s, mm);
	if (ec->type == ECG_25519)
		bn_iadd(b1, b1, -ec->A, 0);

	ec->sub(b1, b1, x);
	ec->sub(b1, b1, x);  /*  x3 = s^2 - A - 2x  */

	ec->sub(b2, x, b1);
	bn_copy(x, b1);
	ec->multiply(b2, b2, s, mm);
	ec->sub(y, b2, y);   /*  y3 = s * (x - x3) - y  */

	bn_free(&b2);
	bn_free(&b1);
	bn_free(&s);
	bn_free(&P);
	bn_free(&mm);
}

static  /* point addition -- no projection */
void add_Point(bn_t *x, bn_t *y, const bn_t *x2, const bn_t *y2, ecg_t *ecg)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2 * LEN_PRIME);
	BN_ALLOC(P);
	BN_ALLOC(s);
	BN_ALLOC(b1);
	BN_ALLOC(b2);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	ec->sub(s, y2, y);
	ec->sub(b1, x2, x);
	bn_inv(b1, P, b1);
	ec->multiply(s, s, b1, mm);  /*   s = (y2 - y) / (x2 - x)  */

	ec->square(b1, s, mm);
	if (ec->type == ECG_25519)
		bn_iadd(b1, b1, -ec->A, 0);

	ec->sub(b1, b1, x2);
	ec->sub(b1, b1, x);  /*  x3 = s^2 - A - x2 - x  */

	ec->sub(b2, x, b1);
	bn_copy(x, b1);
	ec->multiply(b2, b2, s, mm);
	ec->sub(y, b2, y);   /*  y2 = s * (x - x3) - y  */

	bn_free(&b2);
	bn_free(&b1);
	bn_free(&s);
	bn_free(&P);
	bn_free(&mm);
}

/* on curve */
bool ecg_is_member(const ecg_t *cecg, const bn_t *x, const bn_t *y)
{
	bool Ok;
	ecg_t *ecg = (ecg_t *)cecg;
	ecg_type_t type = ecg_get_type(ecg);
	BN_ALLOC(B);
	BN_ALLOC(P);
	BN_ALLOC(T);
	BN_ALLOC(X);
	BN_ALLOC(Y);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_B, B);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	/*  y^2  =  x^3 - 3x + b  =  x(x^2^ -3 ) + b  */
	bn_sqr(Y, y, P);

	if (type == ECG_25519) {  /* y^2 = x^3 + Ax^2 + x  */
		int AA = ecg_get_bn_param(ecg, ECG_PARAM_BN_A, 0);
		bn_sqr(X, x, P);		/* x^2              */
		bn_copy(T, X);
		bn_imul(X, X, AA, P);   /* A * x^2          */
		bn_add(X, X, x, P);		/* A * x^2 + x      */
		bn_mul(T, T, x, P);		/* x^3              */
		bn_add(X, X, T, P);		/* x^3 + Ax^2 + x   */
	}
	else {
		bn_sqr(X, x, P);		/* x^2              */
		bn_iadd(X, X, -3, 0);   /* x^2 - 3          */
		bn_mul(X, X, x, P);		/* x * (x^2 - 3)    */
		bn_add(X, X, B, P);		/* x * (x^2 - 3) + b */
	}
	Ok = !bn_cmp(X, Y);

	bn_free(&Y);
	bn_free(&X);
	bn_free(&T);
	bn_free(&P);
	bn_free(&B);

	return Ok;
}

/* inverse element */
void ecg_elt_inv(const ecg_elt_t *elt, ecg_elt_t *inv)
{
	const bn_t *x, *y;
	ecg_t* ecg = ecg_elt_get_group(elt);
	BN_ALLOC(P);
	BN_ALLOC(yn);
	ecg_elt_get_xy(elt, &x, &y);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);
	bn_sub(yn, P, y, 0);
	ecg_elt_set_xy(inv, x, yn);
	bn_free(&yn);
	bn_free(&P);
}

/* elt double b = 2 * a */
void ecg_elt_dbl(const ecg_elt_t *elt_a, ecg_elt_t *elt_b)
{
	const bn_t *x2, *y2;
	ecg_t* ecg = ecg_elt_get_group(elt_a);
	BN_ALLOC(x);
	BN_ALLOC(y);
	ecg_elt_get_xy(elt_a, &x2, &y2);
	bn_copy(x, x2);
	bn_copy(y, y2);

	Double_Point(x, y, ecg);
	ecg_elt_set_xy(elt_b, x, y);

	bn_free(&y);
	bn_free(&x);
}

/* element add c = a + b */
void ecg_elt_add(const ecg_elt_t *elt_a, const ecg_elt_t *elt_b, ecg_elt_t *elt_c)
{
	const bn_t *x2, *y2;
	ecg_t* ecg = ecg_elt_get_group(elt_a);
	BN_ALLOC(x);
	BN_ALLOC(y);
	ecg_elt_get_xy(elt_a, &x2, &y2);
	bn_copy(x, x2);
	bn_copy(y, y2);

	ecg_elt_get_xy(elt_b, &x2, &y2);
	add_Point(x, y, x2, y2, ecg);
	ecg_elt_set_xy(elt_c, x, y);

	bn_free(&y);
	bn_free(&x);
}

static /* generate a random element in the group  --  replaces the base point elt */
ecg_elt_t* ecg_elt_rand_nist(ecg_t *ecg, ecg_rand_fn_t rand_fn, void* rand_ctx)
{
	ecg_elt_t *rand_elt = ecg_elt_alloc(ecg);
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	bn_t *a = rand_fn(rand_ctx, BNX, bit_len);
	BN_ALLOC(P);
	BN_ALLOC(B);
	BN_ALLOC(x);
	BN_ALLOC(y);
	BN_ALLOC(y2);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_B, B);

		/*  a = -t ^ 2 */
	bn_sqr(a, a, P);
	bn_sub(a, y, a, P);	/*  y=0  --> a = -a  */

		/* y = a^2 + a */
	bn_sqr(y, a, P);
	bn_add(y, y, a, P);

	/* x = B * (y+1) / 3y  */
	rand_get_X(x, y, P, B);		/* y destroyed */

	rand_get_H(y2, x, P, B);
	square_root(ecg, y, y2);
	bn_sqr(B, y, P);
	if (bn_cmp(y2, B)) {
		bn_mul(x, x, a, P);
		ecg_get_bn_param(ecg, ECG_PARAM_BN_B, B);
		rand_get_H(y2, x, P, B);
		square_root(ecg, y, y2);	/* should always be a true square root */
	}
	ecg_elt_init(rand_elt, x, y);
	bn_free(&y2);
	bn_free(&y);
	bn_free(&x);
	bn_free(&B);
	bn_free(&P);
	bn_free(&a);

	return rand_elt;
}

static
bool get_y_from_x_25519(ecg_t *ecg, const bn_t *a, bn_t *y)
{
	bool Ok = 1;
	int AA = ecg_get_bn_param(ecg, ECG_PARAM_BN_A, 0);

	BN_ALLOC(P);
	BN_ALLOC(B);
	BN_ALLOC(x);
	BN_ALLOC(y2);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_B, B);

	/*  Y^2 = x^3 + Ax^2 + x  */
	bn_copy(y, a);
	bn_sqr(x, a, P);
	bn_mul(y2, x, a, P);	/* x^3 */
	bn_imul(x, x, AA, P);	/* Ax^2 */
	bn_add(y2, y2, x, P);
	bn_add(y2, y2, a, P);

	square_root_25519(ecg, y, y2);
	bn_sqr(x, y, P);

	if (bn_cmp(y2, x)) {
		bn_mul(y, y, B, P);
		bn_sqr(x, y, P);
		if (bn_cmp(y2, x)) {
			Ok = 0;
			bn_set(y, BN_FMT_BE, 0, 0);
		}
	}

	bn_free(&y2);
	bn_free(&x);
	bn_free(&B);
	bn_free(&P);

	return Ok;
}

static /* generate a random element in the group  --  replaces the base point elt */
ecg_elt_t* ecg_elt_rand_25519(ecg_t *ecg, ecg_rand_fn_t rand_fn, void* rand_ctx)
{
	ecg_elt_t *rand_elt = ecg_elt_alloc(ecg);
	int bit_len = ecg_get_bn_param(ecg, ECG_PARAM_BN_BIT_LEN, 0);
	int Ok = 0;
	BN_ALLOC(x);
	BN_ALLOC(y);

	while (!Ok) {
		bn_t *a = rand_fn(rand_ctx, BNX, bit_len);

		Ok = get_y_from_x_25519(ecg, a, y);
		bn_copy(x, a);
		bn_free(&a);
	}
	ecg_elt_init(rand_elt, x, y);
	bn_free(&y);
	bn_free(&x);

	ecg_elt_dbl(rand_elt, rand_elt); /* cofactor = 8 */
	ecg_elt_dbl(rand_elt, rand_elt);
	ecg_elt_dbl(rand_elt, rand_elt);
	return rand_elt;
}

/* generate a random element in the group  --  replaces the base point elt */
ecg_elt_t* ecg_elt_rand(ecg_t *ecg, ecg_rand_fn_t rand_fn, void* rand_ctx)
{
	if (ecg_get_type(ecg) == ECG_25519)
		return ecg_elt_rand_25519(ecg, rand_fn, rand_ctx);
	else
		return ecg_elt_rand_nist(ecg, rand_fn, rand_ctx);
}

/*		*********************************************************************************
 *					e c g _ e l t   n i s t   m u l t i p l y
 *		*********************************************************************************
 */

static	/*  convert from projectvie to affinei (z=1) */
void proj_to_affine(bn_t *x, bn_t *y, bn_t* z, ecg_t * ecg)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	uint8 One = 1;
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2*LEN_PRIME);
	BN_ALLOC(P);
	BN_ALLOC(zInv);
	BN_ALLOC(zzInv);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	bn_inv(z, P, zInv);

	ec->square(zzInv, zInv, mm);  /* z ^ -2 */
	ec->multiply(x, x, zzInv, mm);
	ec->multiply(zzInv, zzInv, zInv, mm);  /* z ^ -3 */
	ec->multiply(y, y, zzInv, mm);
	bn_set(z, BN_FMT_BE, &One, 1);

	bn_free(&zzInv);
	bn_free(&zInv);
	bn_free(&P);
	bn_free(&mm);
}

static  /* projective doubling of (x, y, z)  */
void doublePoint(bn_t *x, bn_t *y, bn_t *z, ecg_t* ecg)
{
	const struct ec_parms *ec = get_parms(ecg_get_type(ecg));
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2 * LEN_PRIME);
	BN_ALLOC(A);

	BN_ALLOC(C);
	BN_ALLOC(Y4);

		/*  A = 3 * (x^2 - z^4)		*/
	ec->square(A, x, mm);
	ec->square(Y4, z, mm);
	ec->square(Y4, Y4, mm);
	ec->sub(A, A, Y4);
	//ec->Mul(A, A, 3);
	ec->add(C, A, A);
	ec->add(A, A, C);

		/*  Z = Z * 2Y				*/
	ec->multiply(C, z, y, mm);
	ec->add(z, C, C);

		/*  C = X * 4y^2			*/
	ec->square(C, y, mm);
	ec->square(Y4, C, mm);		/* Y4 = y^4 */
	ec->multiply(C, C, x, mm);
	ec->mul(C, C, 4);

		/*	X =	A^2 - 2C			*/
	ec->square(x, A, mm);
	ec->sub(x, x, C);
	ec->sub(x, x, C);

		/*	Y =		A * (C - X) - 8 * y^4  */
	ec->sub(C, C, x);
	ec->multiply(A, A, C, mm);
	ec->mul(Y4, Y4, 8);
	ec->sub(y, A, Y4);

	bn_free(&Y4);
	bn_free(&C);
	bn_free(&A);
	bn_free(&mm);
}

static
void addPoint(bn_t *x, bn_t *y, bn_t *z, const bn_t *x2, const bn_t *y2, ecg_t *ecg)
{
	const struct ec_parms* ec = get_parms(ecg_get_type(ecg));
	bn_t* mm = bn_alloc(BNX, BN_FMT_BE, 0, 2 * LEN_PRIME);
	BN_ALLOC(A);
	BN_ALLOC(B);
	BN_ALLOC(C);

		/*	A =		z^3 * y2 - y		*/
	ec->square(B, z, mm);
	bn_copy(A, B);
	ec->multiply(A, A, z, mm);
	ec->multiply(A, A, y2, mm);
	ec->sub(A, A, y);

		/*	B =		z^2 * x2 - x		*/
	ec->multiply(B, B, x2, mm);
	ec->sub(B, B, x);

		/*	z =		z * B				*/
	ec->multiply(z, z, B, mm);

		/*	C =	    x * B^2				*/
	ec->square(C, B, mm);
	ec->multiply(B, B, C, mm);	/*	B = B^3		*/
	ec->multiply(C, C, x, mm);

		/*	X =     A^2 - (2C + B^3)	*/
	ec->square(x, A, mm);
	ec->sub(x, x, C);
	ec->sub(x, x, C);
	ec->sub(x, x, B);

		/*	Y =		A * (C - X) - Y * B^3	*/
	ec->sub(C, C, x);
	ec->multiply(C, C, A, mm);
	ec->multiply(B, B, y, mm);
	ec->sub(y, C, B);

	bn_free(&C);
	bn_free(&B);
	bn_free(&A);
	bn_free(&mm);
}

static
const uint32 Pos[32] = {
	0x80000000, 0x40000000, 0x20000000, 0x10000000,
	0x08000000, 0x04000000, 0x02000000, 0x01000000,
	0x00800000, 0x00400000, 0x00200000, 0x00100000,
	0x00080000, 0x00040000, 0x00020000, 0x00010000,
	0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001
};

#define PREV_BIT	if (!nBit) {nBit = 31; nByte--;} else nBit--
#define NEXT_BIT	if (nBit == 31) {nBit = 0; nByte++;} else nBit++
#define BIT_ON		naf[nByte] & Pos[nBit]

static
void naf2(uint32* naf, int SZ)
{
	uint32 nBit = 32, nByte = SZ - 1, kBit, kByte = SZ - 1;
	uint64 QW = naf[nByte];
	while (kByte > 0) {
		PREV_BIT;
		if (kByte > nByte) {
			kByte = nByte;
			QW = (uint64)(QW >> 32) + naf[nByte];
			naf[kByte] = (uint32)QW;
		}
		if (QW & Pos[nBit]) {
			kBit = nBit;
			PREV_BIT;
			PREV_BIT;
			QW += (uint64)Pos[kBit] * ((BIT_ON) ? 8 : 3);
			naf[kByte] = (uint32)QW;
		}
	}
}

static
void get_p_p3(const bn_t *x1, const bn_t *y1, bn_t *y1n,
		bn_t *x3, bn_t *y3, bn_t *y3n, ecg_t* ecg)
{
	BN_ALLOC(Modulus);
	BN_ALLOC(z3);
	bn_iadd(z3, z3, 1, 0);
	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, Modulus);
	bn_sub(y1n, Modulus, y1, 0);

	bn_copy(x3, x1);
	bn_copy(y3, y1);
	doublePoint(x3, y3, z3, ecg);
	addPoint(x3, y3, z3, x1, y1, ecg);
	proj_to_affine(x3, y3, z3, ecg);
	bn_sub(y3n, Modulus, y3, 0);

	bn_free(&z3);
	bn_free(&Modulus);
}

static /* scalar multiply res = bn * elt */
void ecg_elt_mul_nist(const ecg_elt_t *elt, const bn_t *bn, ecg_elt_t *res)
{
	const bn_t *x1, *y1;
	bn_t *x, *y, *z;
	uint32 *naf;
	uint32 naf1[17];
	uint32 SZ;
	uint32 nBit = 12, nByte = 0;
	ecg_t* ecg = ecg_elt_get_group(elt);
	BN_ALLOC(y1n);
	BN_ALLOC(x3);
	BN_ALLOC(y3);
	BN_ALLOC(y3n);
	ecg_elt_get_xy(elt, &x1, &y1);
	get_p_p3(x1, y1, y1n, x3, y3, y3n, ecg);

	x = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME);
	y = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME);
	z = bn_alloc(BNX, BN_FMT_BE, 0, LEN_PRIME);
	bn_iadd(z, z, 1, 0);

	SZ = (ecg_get_type(ecg) == ECG_NIST_P521) ? LEN_PRIME : LEN_PRIME+4;
	bn_get(bn, BN_FMT_LE, (uint8 *)naf1, LEN_PRIME + 4);
	SZ = (LEN_PRIME + 4) >> 2;
	naf2(naf1, SZ);  /* NAF representation */
	SZ--;
	naf = naf1;

	while (nByte < SZ || nBit < 31) {
		NEXT_BIT;
		if (BIT_ON) {
			NEXT_BIT;
			if (BIT_ON) {
				NEXT_BIT;
				if (BIT_ON) {		/* 1 1 1 = -P   */
					bn_copy(x, x1);
					bn_copy(y, y1n);
				} else {			/* 1 1 0 = 3P   */
					bn_copy(x, x3);
					bn_copy(y, y3);
				}
			}
			else {
				NEXT_BIT;
				if (BIT_ON) {		/* 1 0 1 = -3P  */
					bn_copy(x, x3);
					bn_copy(y, y3n);
				} else {			/* 1 0 0 =   P  */
					bn_copy(x, x1);
					bn_copy(y, y1);
				}
			}
			break;
		}
	}

	while (nByte < SZ || nBit < 31) {
		doublePoint(x, y, z, ecg);
		NEXT_BIT;
		if (BIT_ON) {
			doublePoint(x, y, z, ecg);
			doublePoint(x, y, z, ecg);
			NEXT_BIT;
			if (BIT_ON) {
				NEXT_BIT;
				if (BIT_ON)	        /* 1 1 1 =  -P  */
					addPoint(x, y, z, x1, y1n, ecg);
				else				/* 1 1 0 =  3P  */
					addPoint(x, y, z, x3, y3, ecg);
			} else {
				NEXT_BIT;
				if (BIT_ON)			/* 1 0 1 = -3P  */
					addPoint(x, y, z, x3, y3n, ecg);
				else				/* 1 0 0 =   P  */
					addPoint(x, y, z, x1, y1, ecg);
			}
		}
	}
	bn_copy(x3, x);		/* to save space */
	bn_copy(y3, y);
	bn_copy(y1n, z);

	bn_free(&z);
	bn_free(&y);
	bn_free(&x);
	bn_free(&y3n);

	proj_to_affine(x3, y3, y1n, ecg);
	ecg_elt_set_xy(res, x3, y3);

	bn_free(&y3);
	bn_free(&x3);
	bn_free(&y1n);
}

/*		*********************************************************************************
 *					e c g _ e l t   e c 2 5 5 1 9   m u l t i p l y
 *		*********************************************************************************
 */

#define CARRY(n, h) carry; if (a[n] > h) {a[n] &= h; carry = 1;} else carry = 0
#define BORROW(n, h) borrow; if (a[n] > h) {a[n] &= h; borrow = 1;} else borrow = 0

void add9(uint32* a, uint32* b, uint32* c)
{
	uint32 carry;
	a[8] = b[8] + c[8];
	if (a[8] > hi29) {
		a[8] &= hi29;
		carry = 1;
	} else
		carry = 0;

	a[7] = b[7] + c[7] + CARRY(7, hi28);
	a[6] = b[6] + c[6] + CARRY(6, hi28);
	a[5] = b[5] + c[5] + CARRY(5, hi29);
	a[4] = b[4] + c[4] + CARRY(4, hi28);
	a[3] = b[3] + c[3] + CARRY(3, hi28);
	a[2] = b[2] + c[2] + CARRY(2, hi29);
	a[1] = b[1] + c[1] + CARRY(1, hi28);
	a[0] = b[0] + c[0] + carry;
	if (a[0] > hi28) {
		a[0] &= hi28;
		a[8] += 19;
		if (a[8] > hi29) {
			a[8] &= hi29;
			a[7]++;
		}
	}
}

void sub9(uint32* a, uint32* b, uint32* c)
{
	uint32 borrow;
	a[8] = b[8] - c[8];
	if (a[8] > hi29) {
		a[8] &= hi29;
		borrow = 1;
	} else
		borrow = 0;

	a[7] = b[7] - c[7] - BORROW(7, hi28);
	a[6] = b[6] - c[6] - BORROW(6, hi28);
	a[5] = b[5] - c[5] - BORROW(5, hi29);
	a[4] = b[4] - c[4] - BORROW(4, hi28);
	a[3] = b[3] - c[3] - BORROW(3, hi28);
	a[2] = b[2] - c[2] - BORROW(2, hi29);
	a[1] = b[1] - c[1] - BORROW(1, hi28);
	a[0] = b[0] - c[0] - borrow;
	if (a[0] > hi28) {
		a[0] &= hi28;
		a[8] -= 19;
		if (a[8] > hi29) {
			a[8] &= hi29;
			a[7]--;
		}
	}
}

#define RB(n, hi, last)  r = (uint64)a[n] * val + (r >> last); b[n] = (uint32)r & hi

static
void mul9(uint32* b, uint32* a, uint32 val)
{
	uint64 r = (uint64)a[8] * val;
	uint32 k;
	b[8] = (uint32)r & hi29;
	RB(7, hi28, 29);
	RB(6, hi28, 28);
	RB(5, hi29, 28);
	RB(4, hi28, 29);
	RB(3, hi28, 28);
	RB(2, hi29, 28);
	RB(1, hi28, 29);
	RB(0, hi28, 28);

	k = r >> 28;
	if (k) {
		b[8] += k * 19;
		if (b[8] > hi29) {
			b[8] &= hi29;
			b[7]++;
			if (b[7] > hi28) {
				b[7] &= hi29;
				b[6]++;
			}
		}
	}
}

static	/* (x, z) = Dbl(x, z)  */
void mont_25519_dbl(uint32 *x, uint32 *z)
{
	uint64 tt[17];
	uint32 A4 = 121665;  // (A = 486662) >> 2

	uint32 xz[9], xzn[9], bb[9];

	add9(xz, x, z);
	square_256(tt, xz);
	reduce_25519(tt, xz);

	sub9(xzn, x, z);
	square_256(tt, xzn);
	reduce_25519(tt, xzn);

	multiply_256(tt, xz, xzn);
	reduce_25519(tt, x);

	sub9(xzn, xz, xzn);
	mul9(bb, xzn, A4);
	add9(z, bb, xz);
	multiply_256(tt, z, xzn);
	reduce_25519(tt, z);
}

static  /* (x, z) += (x2, z2)  (x2, z2) = dbl(x2, z2) */
void mont_25519_ladder(uint32 *x, uint32 *z, uint32 *x2, uint32 *z2, uint32 *xBase, bool isNine)
{
	uint64 tt[17];
	uint32 xz[9], xzn[9], b[9];
	add9(xz, x, z);       /* x + z */
	sub9(xzn, x2, z2);    /* x2 - z2 */
	multiply_256(tt, xz, xzn);	 /*  (x + z) * (x2 - z2) */
	reduce_25519(tt, xz);

	sub9(xzn, x, z);      /* x - z */
	add9(b, x2, z2);      /* x2 + z2 */
	multiply_256(tt, xzn, b);   /*  (x - z) * (x2 + z2) */
	reduce_25519(tt, xzn);

	add9(b, xzn, xz);     /*  (  ) + (  )  */
	square_256(tt, b);
	reduce_25519(tt, x);

	sub9(b, xzn, xz);     /*  (  ) - ( )   */
	square_256(tt, b);
	reduce_25519(tt, z);

	if (isNine)
		mul9(z, z, 9);
	else {
		multiply_256(tt, z, xBase);
		reduce_25519(tt, z);
	}

	mont_25519_dbl(x2, z2);
}

static /* scalar multiply res = bn * elt */
void ecg_elt_mul_25519(const ecg_elt_t *elt, const bn_t *bn, ecg_elt_t *res)
{
	const bn_t *x1, *y1; /* y1 not used */
	ecg_t* ecg = ecg_elt_get_group(elt);
	uint32 nBit = 0, nByte = 0;
	uint32 SZ = 8;
	bool isNine = 0;
	bool getY;
	uint32 naf[8];
	uint32 Ax[9], Bx[9], xBase[9];
	uint32 Az[9] = {0, 0, 0, 0, 0, 0, 0, 0, 1};
	uint32 Bz[9] = {0, 0, 0, 0, 0, 0, 0, 0, 1};
	uint32 xNine[9] = {0, 0, 0, 0, 0, 0, 0, 0, 9};

	BN_ALLOC(P);
	BN_ALLOC(AX);
	BN_ALLOC(AZ);

	bn_get(bn, BN_FMT_LE, (uint8 *)naf, sizeof(naf));
	getY = !(naf[0] & 0x40000000);

	ecg_elt_get_xy(elt, &x1, &y1);
	bn_get(x1, BN_FMT_LE, (uint8 *)xBase, sizeof(xBase));
	if (memcmp(xBase, xNine, sizeof(xNine)))
		convert9(xBase);
	else
		isNine = 1;

	memcpy(Ax, xBase, sizeof(Ax));
	memcpy(Bx, xBase, sizeof(Ax));

	while (nByte < SZ) {
		if (BIT_ON) {
			mont_25519_dbl(Bx, Bz);
			NEXT_BIT;
			break;
		}
		NEXT_BIT;
	}

	while (nByte < SZ) {
		if (BIT_ON)
			mont_25519_ladder(Ax, Az, Bx, Bz, xBase, isNine);
		else
			mont_25519_ladder(Bx, Bz, Ax, Az, xBase, isNine);
		NEXT_BIT;
	}

	convert8(Az);
	bn_set(AZ, BN_FMT_LE, (uint8 *)Az, sizeof(Az));
	convert8(Ax);
	bn_set(AX, BN_FMT_LE, (uint8 *)Ax, sizeof(Ax));

	ecg_get_bn_param(ecg, ECG_PARAM_BN_PRIME, P);

	bn_inv(AZ, P, AZ);
	bn_mul(AX, AX, AZ, P);
	if (getY)
		get_y_from_x_25519(ecg, AX, AZ);
	else
		bn_set(AZ, BN_FMT_BE, 0, 0);

	ecg_elt_set_xy(res, AX, AZ);

	bn_free(&AZ);
	bn_free(&AX);
	bn_free(&P);
}

/* scalar multiply res = bn * elt */
void ecg_elt_mul(const ecg_elt_t *elt, const bn_t *bn, ecg_elt_t *res)
{
	ecg_t* ecg = ecg_elt_get_group(elt);
	if (ecg_get_type(ecg) == ECG_25519)
		ecg_elt_mul_25519(elt, bn, res);
	else
		ecg_elt_mul_nist(elt, bn, res);
}
