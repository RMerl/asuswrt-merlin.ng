/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1989-2013 Free Software Foundation, Inc.
 */

#ifndef __ASM_LIBGCC_H
#define __ASM_LIBGCC_H

#define UNITS_PER_WORD 4	/* for ARC */
#define BITS_PER_UNIT 8		/* for ARC */

#define W_TYPE_SIZE (4 * BITS_PER_UNIT)

#define MIN_UNITS_PER_WORD UNITS_PER_WORD

/* Work out the largest "word" size that we can deal with on this target.  */
#if MIN_UNITS_PER_WORD > 4
# define LIBGCC2_MAX_UNITS_PER_WORD 8
#elif (MIN_UNITS_PER_WORD > 2 \
       || (MIN_UNITS_PER_WORD > 1 && __SIZEOF_LONG_LONG__ > 4))
# define LIBGCC2_MAX_UNITS_PER_WORD 4
#else
# define LIBGCC2_MAX_UNITS_PER_WORD MIN_UNITS_PER_WORD
#endif

/* Work out what word size we are using for this compilation.
   The value can be set on the command line.  */
#ifndef LIBGCC2_UNITS_PER_WORD
#define LIBGCC2_UNITS_PER_WORD LIBGCC2_MAX_UNITS_PER_WORD
#endif

typedef		 int QItype	__attribute__ ((mode (QI)));
typedef unsigned int UQItype	__attribute__ ((mode (QI)));
typedef		 int HItype	__attribute__ ((mode (HI)));
typedef unsigned int UHItype	__attribute__ ((mode (HI)));
#if MIN_UNITS_PER_WORD > 1
/* These typedefs are usually forbidden on dsp's with UNITS_PER_WORD 1.  */
typedef		 int SItype	__attribute__ ((mode (SI)));
typedef unsigned int USItype	__attribute__ ((mode (SI)));
#if __SIZEOF_LONG_LONG__ > 4
/* These typedefs are usually forbidden on archs with UNITS_PER_WORD 2.  */
typedef		 int DItype	__attribute__ ((mode (DI)));
typedef unsigned int UDItype	__attribute__ ((mode (DI)));
#if MIN_UNITS_PER_WORD > 4
/* These typedefs are usually forbidden on archs with UNITS_PER_WORD 4.  */
typedef		 int TItype	__attribute__ ((mode (TI)));
typedef unsigned int UTItype	__attribute__ ((mode (TI)));
#endif
#endif
#endif

#if LIBGCC2_UNITS_PER_WORD == 8
#define W_TYPE_SIZE (8 * BITS_PER_UNIT)
#define Wtype	DItype
#define UWtype	UDItype
#define HWtype	DItype
#define UHWtype	UDItype
#define DWtype	TItype
#define UDWtype	UTItype
#ifdef LIBGCC2_GNU_PREFIX
#define __NW(a,b)	__gnu_ ## a ## di ## b
#define __NDW(a,b)	__gnu_ ## a ## ti ## b
#else
#define __NW(a,b)	__ ## a ## di ## b
#define __NDW(a,b)	__ ## a ## ti ## b
#endif
#elif LIBGCC2_UNITS_PER_WORD == 4
#define W_TYPE_SIZE (4 * BITS_PER_UNIT)
#define Wtype	SItype
#define UWtype	USItype
#define HWtype	SItype
#define UHWtype	USItype
#define DWtype	DItype
#define UDWtype	UDItype
#ifdef LIBGCC2_GNU_PREFIX
#define __NW(a,b)	__gnu_ ## a ## si ## b
#define __NDW(a,b)	__gnu_ ## a ## di ## b
#else
#define __NW(a,b)	__ ## a ## si ## b
#define __NDW(a,b)	__ ## a ## di ## b
#endif
#elif LIBGCC2_UNITS_PER_WORD == 2
#define W_TYPE_SIZE (2 * BITS_PER_UNIT)
#define Wtype	HItype
#define UWtype	UHItype
#define HWtype	HItype
#define UHWtype	UHItype
#define DWtype	SItype
#define UDWtype	USItype
#ifdef LIBGCC2_GNU_PREFIX
#define __NW(a,b)	__gnu_ ## a ## hi ## b
#define __NDW(a,b)	__gnu_ ## a ## si ## b
#else
#define __NW(a,b)	__ ## a ## hi ## b
#define __NDW(a,b)	__ ## a ## si ## b
#endif
#else
#define W_TYPE_SIZE BITS_PER_UNIT
#define Wtype	QItype
#define UWtype  UQItype
#define HWtype	QItype
#define UHWtype	UQItype
#define DWtype	HItype
#define UDWtype	UHItype
#ifdef LIBGCC2_GNU_PREFIX
#define __NW(a,b)	__gnu_ ## a ## qi ## b
#define __NDW(a,b)	__gnu_ ## a ## hi ## b
#else
#define __NW(a,b)	__ ## a ## qi ## b
#define __NDW(a,b)	__ ## a ## hi ## b
#endif
#endif

typedef int shift_count_type __attribute__((mode (__libgcc_shift_count__)));

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
	struct DWstruct {Wtype high, low;};
#else
	struct DWstruct {Wtype low, high;};
#endif

/* We need this union to unpack/pack DImode values, since we don't have
   any arithmetic yet.  Incoming DImode parameters are stored into the
   `ll' field, and the unpacked result is read from the struct `s'.  */

typedef union {
	struct DWstruct s;
	DWtype ll;
} DWunion;

#endif /* __ASM_LIBGCC_H */
