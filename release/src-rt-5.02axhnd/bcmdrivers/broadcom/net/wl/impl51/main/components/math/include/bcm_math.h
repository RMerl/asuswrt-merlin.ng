/*
 * Math component interface file.
 * General API for calculation purposes.
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id:$
 */

#ifndef _bcm_math_h_
#define _bcm_math_h_

#include "bcm_math32.h"
#include "bcm_math64.h"
#include "bcm_math_defs.h"

#define CORDIC32_LOG2_PI_OVER_TWO	18 /* LOG2 PI over 2 */
#define ROUND(x, s) (((x) >> (s)) + (((x) >> ((s) - 1)) & (s != 0)))

/* ============================================
 * Complex number routines
 * ============================================
 */
void math_cmplx_add_cint32(const cint32 *in1, const cint32 *in2, cint32 *out);
void math_cmplx_power_cint32(const cint32 *in1, uint32 *pwr);
void math_cmplx_mult_cint32_cfixed(const cint32 *in1, const cint32 *in2, const uint8 prec,
	cint32 *out, bool conj);
void math_cmplx_power_cint32_arr(const cint32 *in1, const uint16 *idx_arr,
	uint16 len, uint32 *pwr);
void math_cmplx_cordic(math_fixed theta, math_cint32 *val);
void math_cmplx_invcordic(math_cint32 val, int32 *angle);
void math_cmplx_computedB(uint32 *cmplx_pwr, int8 *p_cmplx_pwr_dB, uint8 core);
void math_cmplx_angle_to_phasor_lut(uint16 angle, uint16 *packed_word);

/* ============================================
 * Matrix routines
 * ============================================
 */

/* Compute matrix rho (m x 3).
 * column 1 = all 1s
 * column 2 = n[i]
 * column 3 = - n[i] * P[i]
 */
void math_mat_rho(int64 *n, int64 *p, int64 *rho, int m);

/* Matrix transpose routine.
 *
 * a - pointer to matrix m x n
 * b - pointer to result matrix a_transpose(n x m)
 * m - number of rows of matrix a
 * n - number of cols of matrix a
 */
void math_mat_transpose(int64 *a, int64 *b, int m, int n);

/* Matrix multiply routine. Assumes matrices are allocated
 * in memory contiguously one row after the other.
 *
 * a - pointer to matrix m x n
 * b - pointer to matrix n x r
 * c - pointer to result matrix m x r
 * m - number of rows of matrix a
 * n - number of cols of matrix a and number of rows of matrix b
 * r - number of cols of matrix b
 */
void math_mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r);

/* Matrix inverse of a 3x3 matrix * det(matrix).
 *
 * a - pointer to matrix of 3x3
 * b - pointer to the result
 */
void math_mat_inv_prod_det(int64 *a, int64 *b);

/* Calculates determinant of 3x3 matrix.
 *
 * a - pointer to matrix of 3x3
 * det - pointer to the result
 */
void math_mat_det(int64 *a, int64 *det);

/* ============================================
 * Fast fourier transform sin/cos routines
 * ============================================
 */

/* Get cos value from the table */
int32 math_fft_cos(int i);

/* Get cos value from the table */
int32 math_fft_cos_seq20(int i);

/* Get cos value from the table */
int32 math_fft_cos_128(int i);

/* Get sin value from the table */
int32 math_fft_sin(int i);

/* Get sin value from the table */
int32 math_fft_sin_seq20(int i);

/* Get sin value from the table */
int32 math_fft_sin_128(int i);

/* ============================================
 * Miscellaneous
 * ============================================
 */
int32 math_cordic(cint32 phasor);
int32 math_cordic_ptr(void* value);
int32 math_cos_tbl(int32 theta);
int32 math_sin_tbl(int32 theta);

#endif /* _bcm_math_h_ */
