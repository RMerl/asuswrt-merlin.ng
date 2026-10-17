// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 1989-2013 Free Software Foundation, Inc.
 */

#include "libgcc2.h"

DWtype
__ashldi3(DWtype u, shift_count_type b)
{
	if (b == 0)
		return u;

	const DWunion uu = {.ll = u};
	const shift_count_type bm = W_TYPE_SIZE - b;
	DWunion w;

	if (bm <= 0) {
		w.s.low = 0;
		w.s.high = (UWtype)uu.s.low << -bm;
	} else {
		const UWtype carries = (UWtype) uu.s.low >> bm;

		w.s.low = (UWtype)uu.s.low << b;
		w.s.high = ((UWtype)uu.s.high << b) | carries;
	}

	return w.ll;
}

DWtype
__ashrdi3(DWtype u, shift_count_type b)
{
	if (b == 0)
		return u;

	const DWunion uu = {.ll = u};
	const shift_count_type bm = W_TYPE_SIZE - b;
	DWunion w;

	if (bm <= 0) {
		/* w.s.high = 1..1 or 0..0 */
		w.s.high = uu.s.high >> (W_TYPE_SIZE - 1);
		w.s.low = uu.s.high >> -bm;
	} else {
		const UWtype carries = (UWtype) uu.s.high << bm;

		w.s.high = uu.s.high >> b;
		w.s.low = ((UWtype)uu.s.low >> b) | carries;
	}

	return w.ll;
}

DWtype
__lshrdi3(DWtype u, shift_count_type b)
{
	if (b == 0)
		return u;

	const DWunion uu = {.ll = u};
	const shift_count_type bm = W_TYPE_SIZE - b;
	DWunion w;

	if (bm <= 0) {
		w.s.high = 0;
		w.s.low = (UWtype)uu.s.high >> -bm;
	} else {
		const UWtype carries = (UWtype)uu.s.high << bm;

		w.s.high = (UWtype)uu.s.high >> b;
		w.s.low = ((UWtype)uu.s.low >> b) | carries;
	}

	return w.ll;
}

unsigned long
udivmodsi4(unsigned long num, unsigned long den, int modwanted)
{
	unsigned long bit = 1;
	unsigned long res = 0;

	while (den < num && bit && !(den & (1L<<31))) {
		den <<= 1;
		bit <<= 1;
	}

	while (bit) {
		if (num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>= 1;
		den >>= 1;
	}

	if (modwanted)
		return num;

	return res;
}

long
__divsi3(long a, long b)
{
	int neg = 0;
	long res;

	if (a < 0) {
		a = -a;
		neg = !neg;
	}

	if (b < 0) {
		b = -b;
		neg = !neg;
	}

	res = udivmodsi4(a, b, 0);

	if (neg)
		res = -res;

	return res;
}

long
__modsi3(long a, long b)
{
	int neg = 0;
	long res;

	if (a < 0) {
		a = -a;
		neg = 1;
	}

	if (b < 0)
		b = -b;

	res = udivmodsi4(a, b, 1);

	if (neg)
		res = -res;

	return res;
}

long
__udivsi3(long a, long b)
{
	return udivmodsi4(a, b, 0);
}

long
__umodsi3(long a, long b)
{
	return udivmodsi4(a, b, 1);
}

UDWtype
__udivmoddi4(UDWtype n, UDWtype d, UDWtype *rp)
{
	UDWtype q = 0, r = n, y = d;
	UWtype lz1, lz2, i, k;

	/*
	 * Implements align divisor shift dividend method. This algorithm
	 * aligns the divisor under the dividend and then perform number of
	 * test-subtract iterations which shift the dividend left. Number of
	 * iterations is k + 1 where k is the number of bit positions the
	 * divisor must be shifted left  to align it under the dividend.
	 * quotient bits can be saved in the rightmost positions of the
	 * dividend as it shifts left on each test-subtract iteration.
	 */

	if (y <= r) {
		lz1 = __builtin_clzll(d);
		lz2 = __builtin_clzll(n);

		k = lz1 - lz2;
		y = (y << k);

		/*
		 * Dividend can exceed 2 ^ (width - 1) - 1 but still be less
		 * than the aligned divisor. Normal iteration can drops the
		 * high order bit of the dividend. Therefore, first
		 * test-subtract iteration is a special case, saving its
		 * quotient bit in a separate location and not shifting
		 * the dividend.
		 */

		if (r >= y) {
			r = r - y;
			q = (1ULL << k);
		}

		if (k > 0) {
			y = y >> 1;

			/*
			 * k additional iterations where k regular test
			 * subtract shift dividend iterations are done.
			 */
			i = k;
			do {
				if (r >= y)
					r = ((r - y) << 1) + 1;
				else
					r = (r << 1);
				i = i - 1;
			} while (i != 0);

			/*
			 * First quotient bit is combined with the quotient
			 * bits resulting from the k regular iterations.
			 */
			q = q + r;
			r = r >> k;
			q = q - (r << k);
		}
	}

	if (rp)
		*rp = r;

	return q;
}

UDWtype
__udivdi3(UDWtype n, UDWtype d)
{
	return __udivmoddi4(n, d, (UDWtype *)0);
}
