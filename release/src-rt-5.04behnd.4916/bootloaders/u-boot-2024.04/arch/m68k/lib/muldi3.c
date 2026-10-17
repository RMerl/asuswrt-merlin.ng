// SPDX-License-Identifier: GPL-2.0+
/*
 * muldi3.c extracted from gcc-2.7.2.3/libgcc2.c and
 *			   gcc-2.7.2.3/longlong.h
 *
 * Copyright (C) 1989, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
 */

#define SI_TYPE_SIZE 32
#define __BITS4 (SI_TYPE_SIZE / 4)
#define __ll_B (1L << (SI_TYPE_SIZE / 2))
#define __ll_lowpart(t) ((USItype) (t) % __ll_B)
#define __ll_highpart(t) ((USItype) (t) / __ll_B)

#define umul_ppmm(w1, w0, u, v)						\
  do {									\
    USItype __x0, __x1, __x2, __x3;					\
    USItype __ul, __vl, __uh, __vh;					\
									\
    __ul = __ll_lowpart (u);						\
    __uh = __ll_highpart (u);						\
    __vl = __ll_lowpart (v);						\
    __vh = __ll_highpart (v);						\
									\
    __x0 = (USItype) __ul * __vl;					\
    __x1 = (USItype) __ul * __vh;					\
    __x2 = (USItype) __uh * __vl;					\
    __x3 = (USItype) __uh * __vh;					\
									\
    __x1 += __ll_highpart (__x0);/* this can't give carry */		\
    __x1 += __x2;		/* but this indeed can */		\
    if (__x1 < __x2)		/* did we get it? */			\
      __x3 += __ll_B;		/* yes, add it in the proper pos. */	\
									\
    (w1) = __x3 + __ll_highpart (__x1);					\
    (w0) = __ll_lowpart (__x1) * __ll_B + __ll_lowpart (__x0);		\
  } while (0)

#define __umulsidi3(u, v) \
  ({DIunion __w;							\
    umul_ppmm (__w.s.high, __w.s.low, u, v);				\
    __w.ll; })

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

DItype __muldi3 (DItype u, DItype v)
{
	DIunion w;
	DIunion uu, vv;

	uu.ll = u,
	vv.ll = v;

	w.ll = __umulsidi3 (uu.s.low, vv.s.low);
	w.s.high += ((USItype) uu.s.low * (USItype) vv.s.high
		+ (USItype) uu.s.high * (USItype) vv.s.low);

	return w.ll;
}
