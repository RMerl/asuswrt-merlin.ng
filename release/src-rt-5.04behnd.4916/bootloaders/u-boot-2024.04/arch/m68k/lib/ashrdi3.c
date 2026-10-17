// SPDX-License-Identifier: GPL-2.0+
/*
 * ashrdi3.c extracted from gcc-2.7.2/libgcc2.c which is:
 * Copyright (C) 1989, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
 */

#define BITS_PER_UNIT 8

typedef		 int SItype	__attribute__((mode(SI)));
typedef unsigned int USItype	__attribute__((mode(SI)));
typedef		 int DItype	__attribute__((mode(DI)));
typedef int word_type           __attribute__((mode(__word__)));

struct DIstruct {
	SItype high, low;
};

typedef union {
	struct DIstruct s;
	DItype ll;
} di_union;

DItype __ashrdi3(DItype u, word_type b)
{
	di_union w;
	word_type bm;
	di_union uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof(SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0) {
		/* w.s.high = 1..1 or 0..0 */
		w.s.high = uu.s.high >> (sizeof(SItype) * BITS_PER_UNIT - 1);
		w.s.low = uu.s.high >> -bm;
	} else {
		USItype carries = (USItype)uu.s.high << bm;

		w.s.high = uu.s.high >> b;
		w.s.low = ((USItype)uu.s.low >> b) | carries;
	}

	return w.ll;
}

