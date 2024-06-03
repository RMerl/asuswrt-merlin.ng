// SPDX-License-Identifier: GPL-2.0+
/*
 * ashldi3.c extracted from gcc-2.7.2.3/libgcc2.c and
 *			   gcc-2.7.2.3/longlong.h
 *
 * Copyright (C) 1989-2015 Free Software Foundation, Inc.
 */

#define BITS_PER_UNIT 8

typedef		 int SItype	__attribute__ ((mode (SI)));
typedef unsigned int USItype	__attribute__ ((mode (SI)));
typedef		 int DItype	__attribute__ ((mode (DI)));
typedef int word_type __attribute__ ((mode (__word__)));

struct DIstruct {SItype high, low;};

typedef union
{
  struct DIstruct s;
  DItype ll;
} DIunion;

DItype __ashldi3 (DItype u, word_type b)
{
	DIunion w;
	word_type bm;
	DIunion uu;

	if (b == 0)
		return u;

	uu.ll = u;

	bm = (sizeof (SItype) * BITS_PER_UNIT) - b;
	if (bm <= 0)
	{
		w.s.low = 0;
		w.s.high = (USItype)uu.s.low << -bm;
	}
	else
	{
		USItype carries = (USItype)uu.s.low >> bm;
		w.s.low = (USItype)uu.s.low << b;
		w.s.high = ((USItype)uu.s.high << b) | carries;
	}

	return w.ll;
}