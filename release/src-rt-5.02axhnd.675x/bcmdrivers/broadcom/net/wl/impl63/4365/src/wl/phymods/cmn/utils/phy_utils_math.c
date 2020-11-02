/*
 * PHY utils - math library functions.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>

#include <wlc_phy_int.h>
#include <phy_utils_math.h>

void
phy_utils_computedB(uint32 *cmplx_pwr, int8 *p_cmplx_pwr_dB, uint8 core)
{
	uint8 shift_ct, lsb, msb, secondmsb, i;
	uint32 tmp;

	ASSERT(core <= PHY_CORE_MAX);

	PHY_INFORM(("wlc_phy_compute_dB: compute_dB for %d cores\n", core));
	for (i = 0; i < core; i++) {
		tmp = cmplx_pwr[i];
		shift_ct = msb = secondmsb = 0;
		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1)
				msb = shift_ct;
		}

		if (msb != 0)
		secondmsb = (uint8)((cmplx_pwr[i] >> (msb - 1)) & 1);

		p_cmplx_pwr_dB[i] = (int8)(3*msb + 2*secondmsb);
		PHY_INFORM(("wlc_phy_compute_dB: p_cmplx_pwr_dB[%d] %d\n", i, p_cmplx_pwr_dB[i]));
	}
}

/* Atan table for cordic >> num2str(atan(1./(2.^[0:17]))/pi*180,8) */
static const math_fixed AtanTbl[] = {
	2949120,
	1740967,
	919879,
	466945,
	234379,
	117304,
	58666,
	29335,
	14668,
	7334,
	3667,
	1833,
	917,
	458,
	229,
	115,
	57,
	29
};

void
phy_utils_cordic(math_fixed theta, math_cint32 *val)
{
	math_fixed angle, valtmp;
	unsigned iter;
	int signx = 1;
	int signtheta;

	val[0].i = CORDIC_AG;
	val[0].q = 0;
	angle    = 0;

	/* limit angle to -180 .. 180 */
	signtheta = (theta < 0) ? -1 : 1;
	theta = ((theta+FIXED(180)*signtheta)% FIXED(360))-FIXED(180)*signtheta;

	/* rotate if not in quadrant one or four */
	if (FLOAT(theta) > 90) {
		theta -= FIXED(180);
		signx = -1;
	} else if (FLOAT(theta) < -90) {
		theta += FIXED(180);
		signx = -1;
	}

	/* run cordic iterations */
	for (iter = 0; iter < CORDIC_NI; iter++) {
		if (theta > angle) {
			valtmp = val[0].i - (val[0].q >> iter);
			val[0].q = (val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle += AtanTbl[iter];
		} else {
			valtmp = val[0].i + (val[0].q >> iter);
			val[0].q = -(val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle -= AtanTbl[iter];
		}
	}

	/* re-rotate quadrant two and three points */
	val[0].i = val[0].i*signx;
	val[0].q = val[0].q*signx;
}

void
phy_utils_invcordic(math_cint32 val, int32 *angle)
{
	int valtmp;
	unsigned iter;

	*angle = 0;
	val.i = val.i << 4;
	val.q = val.q << 4;

	/* run cordic iterations */
	for (iter = 0; iter < CORDIC_NI; iter++) {
		if (val.q < 0) {
			valtmp = val.i - (val.q >> iter);
			val.q = (val.i >> iter) + val.q;
			val.i = valtmp;
			*angle -= AtanTbl[iter];
		} else {
			valtmp = val.i + (val.q >> iter);
			val.q = -(val.i >> iter) + val.q;
			val.i = valtmp;
			*angle += AtanTbl[iter];
		}
	}

}

uint8
phy_utils_nbits(int32 value)
{
	int32 abs_val;
	uint8 nbits = 0;

	abs_val = ABS(value);
	while ((abs_val >> nbits) > 0) nbits++;

	return nbits;
}

uint32
phy_utils_sqrt_int(uint32 value)
{
	uint32 root = 0, shift = 0;

	/* Compute integer nearest to square root of input integer value */
	for (shift = 0; shift < 32; shift += 2) {
		if (((0x40000000 >> shift) + root) <= value) {
			value -= ((0x40000000 >> shift) + root);
			root = (root >> 1) | (0x40000000 >> shift);
		} else {
			root = root >> 1;
		}
	}

	/* round to the nearest integer */
	if (root < value) ++root;

	return root;
}

uint32
phy_utils_qdiv(uint32 dividend, uint32 divisor, uint8 precision, bool round)
{
	uint32 quotient, remainder, roundup, rbit;

	ASSERT(divisor);

	quotient = dividend / divisor;
	remainder = dividend % divisor;
	rbit = divisor & 1;
	roundup = (divisor >> 1) + rbit;

	while (precision--) {
		quotient <<= 1;
		if (remainder >= roundup) {
			quotient++;
			remainder = ((remainder - roundup) << 1) + rbit;
		} else {
			remainder <<= 1;
		}
	}

	/* Final rounding */
	if (round && (remainder >= roundup))
		quotient++;

	return quotient;
}

uint32
phy_utils_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision)
{
	return phy_utils_qdiv(dividend, divisor, precision, TRUE);
}

/*
 * Compute matrix rho ( m x 3)
 *
 * column 1 = all 1s
 * column 2 = n[i]
 * column 3 = - n[i] * P[i]
 */
void
phy_utils_mat_rho(int64 *n, int64 *p, int64 *rho, int m)
{
	int i;
	int q1 = 2;

	for (i = 0; i < m; i++) {
		*(rho + (i * 3) + 0) = 1;
		*(rho + (i * 3) + 1) = *(n + (i * 1) + 0);
		*(rho + (i * 3) + 2) =
			- (*(n + (i * 1) + 0) * (*(p + (i * 1) + 0)));
		*(rho + (i * 3) + 2) = (*(rho + (i * 3) + 2) + (int64)(1<<(q1-1))) >> q1;
	}
}

/*
 * Matrix transpose routine
 * matrix a (m x n)
 * matrix b = a_transpose(n x m)
 */
void
phy_utils_mat_transpose(int64 *a, int64 *b, int m, int n)
{
	int i, j;

	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			/* b[j][i] = a[i][j]; */
			*(b + (j * m) + i) = *(a + (i * n) + j);
}

/*
 * Matrix multiply routine.
 * matrix a (m x n)
 * matrix b (n x r)
 * c = result matrix (m x r)
 * m = number of rows of matrix a
 * n = number of cols of matrix a and number of rows of matrix b
 * r = number of cols of matrix b
 * assumes matrixes are allocated in memory contiguously one row after
 * the other
 *
 */
void
phy_utils_mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r)
{
	int i, j, k;

	for (i = 0; i < m; i++)
		for (j = 0; j < r; j++) {
			*(c + (i * r) + j) = 0;
			for (k = 0; k < n; k++)
				/* c[i][j] += a[i][k] * b[k][j]; */
				*(c + (i * r) + j)
					+= *(a + (i * n) + k) *
					*(b + (k * r) + j);
		}
}

/*
 * Matrix inverse of a 3x3 matrix * det(matrix)
 * a and b: matrices of 3x3
 */
void
phy_utils_mat_inv_prod_det(int64 *a, int64 *b)
{

	/* C2_calc = [	a22*a33 - a32*a23  a13*a32 - a12*a33  a12*a23 - a13*a22
					a23*a31 - a21*a33  a11*a33 - a13*a31  a13*a21 - a11*a23
					a21*a32 - a31*a22  a12*a31 - a11*a32  a11*a22 - a12*a21];
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*(b + (0 * 3) + 0) = a22 * a33 - a32 * a23;
	*(b + (0 * 3) + 1) = a13 * a32 - a12 * a33;
	*(b + (0 * 3) + 2) = a12 * a23 - a13 * a22;

	*(b + (1 * 3) + 0) = a23 * a31 - a21 * a33;
	*(b + (1 * 3) + 1) = a11 * a33 - a13 * a31;
	*(b + (1 * 3) + 2) = a13 * a21 - a11 * a23;

	*(b + (2 * 3) + 0) = a21 * a32 - a31 * a22;
	*(b + (2 * 3) + 1) =  a12 * a31 - a11 * a32;
	*(b + (2 * 3) + 2) = a11 * a22 - a12 * a21;
}

void
phy_utils_mat_det(int64 *a, int64 *det)
{

	/* det_C1 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32 -
				 a11*a23*a32 - a12*a21*a33 - a13*a22*a31;
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*det = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
		a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31;
}
