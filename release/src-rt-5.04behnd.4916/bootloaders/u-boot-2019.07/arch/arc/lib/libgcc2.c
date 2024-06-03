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
