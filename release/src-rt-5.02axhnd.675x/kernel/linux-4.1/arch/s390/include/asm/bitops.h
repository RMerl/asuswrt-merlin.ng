/*
 *    Copyright IBM Corp. 1999,2013
 *
 *    Author(s): Martin Schwidefsky <schwidefsky@de.ibm.com>,
 *
 * The description below was taken in large parts from the powerpc
 * bitops header file:
 * Within a word, bits are numbered LSB first.  Lot's of places make
 * this assumption by directly testing bits with (val & (1<<nr)).
 * This can cause confusion for large (> 1 word) bitmaps on a
 * big-endian system because, unlike little endian, the number of each
 * bit depends on the word size.
 *
 * The bitop functions are defined to work on unsigned longs, so for an
 * s390x system the bits end up numbered:
 *   |63..............0|127............64|191...........128|255...........192|
 * and on s390:
 *   |31.....0|63....32|95....64|127...96|159..128|191..160|223..192|255..224|
 *
 * There are a few little-endian macros used mostly for filesystem
 * bitmaps, these work on similar bit arrays layouts, but
 * byte-oriented:
 *   |7...0|15...8|23...16|31...24|39...32|47...40|55...48|63...56|
 *
 * The main difference is that bit 3-5 (64b) or 3-4 (32b) in the bit
 * number field needs to be reversed compared to the big-endian bit
 * fields. This can be achieved by XOR with 0x38 (64b) or 0x18 (32b).
 *
 * We also have special functions which work with an MSB0 encoding:
 * on an s390x system the bits are numbered:
 *   |0..............63|64............127|128...........191|192...........255|
 * and on s390:
 *   |0.....31|32....63|64....95|96...127|128..159|160..191|192..223|224..255|
 *
 * The main difference is that bit 0-63 (64b) or 0-31 (32b) in the bit
 * number field needs to be reversed compared to the LSB0 encoded bit
 * fields. This can be achieved by XOR with 0x3f (64b) or 0x1f (32b).
 *
 */

#ifndef _S390_BITOPS_H
#define _S390_BITOPS_H

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/typecheck.h>
#include <linux/compiler.h>
#include <asm/barrier.h>

#define __BITOPS_NO_BARRIER	"\n"

#ifdef CONFIG_HAVE_MARCH_Z196_FEATURES

#define __BITOPS_OR		"laog"
#define __BITOPS_AND		"lang"
#define __BITOPS_XOR		"laxg"
#define __BITOPS_BARRIER	"bcr	14,0\n"

#define __BITOPS_LOOP(__addr, __val, __op_string, __barrier)	\
({								\
	unsigned long __old;					\
								\
	typecheck(unsigned long *, (__addr));			\
	asm volatile(						\
		__barrier					\
		__op_string "	%0,%2,%1\n"			\
		__barrier					\
		: "=d" (__old),	"+Q" (*(__addr))		\
		: "d" (__val)					\
		: "cc", "memory");				\
	__old;							\
})

#else /* CONFIG_HAVE_MARCH_Z196_FEATURES */

#define __BITOPS_OR		"ogr"
#define __BITOPS_AND		"ngr"
#define __BITOPS_XOR		"xgr"
#define __BITOPS_BARRIER	"\n"

#define __BITOPS_LOOP(__addr, __val, __op_string, __barrier)	\
({								\
	unsigned long __old, __new;				\
								\
	typecheck(unsigned long *, (__addr));			\
	asm volatile(						\
		"	lg	%0,%2\n"			\
		"0:	lgr	%1,%0\n"			\
		__op_string "	%1,%3\n"			\
		"	csg	%0,%1,%2\n"			\
		"	jl	0b"				\
		: "=&d" (__old), "=&d" (__new), "+Q" (*(__addr))\
		: "d" (__val)					\
		: "cc", "memory");				\
	__old;							\
})

#endif /* CONFIG_HAVE_MARCH_Z196_FEATURES */

#define __BITOPS_WORDS(bits) (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)

static inline unsigned long *
__bitops_word(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long addr;

	addr = (unsigned long)ptr + ((nr ^ (nr & (BITS_PER_LONG - 1))) >> 3);
	return (unsigned long *)addr;
}

static inline unsigned char *
__bitops_byte(unsigned long nr, volatile unsigned long *ptr)
{
	return ((unsigned char *)ptr) + ((nr ^ (BITS_PER_LONG - 8)) >> 3);
}

static inline void set_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long mask;

#ifdef CONFIG_HAVE_MARCH_ZEC12_FEATURES
	if (__builtin_constant_p(nr)) {
		unsigned char *caddr = __bitops_byte(nr, ptr);

		asm volatile(
			"oi	%0,%b1\n"
			: "+Q" (*caddr)
			: "i" (1 << (nr & 7))
			: "cc", "memory");
		return;
	}
#endif
	mask = 1UL << (nr & (BITS_PER_LONG - 1));
	__BITOPS_LOOP(addr, mask, __BITOPS_OR, __BITOPS_NO_BARRIER);
}

static inline void clear_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long mask;

#ifdef CONFIG_HAVE_MARCH_ZEC12_FEATURES
	if (__builtin_constant_p(nr)) {
		unsigned char *caddr = __bitops_byte(nr, ptr);

		asm volatile(
			"ni	%0,%b1\n"
			: "+Q" (*caddr)
			: "i" (~(1 << (nr & 7)))
			: "cc", "memory");
		return;
	}
#endif
	mask = ~(1UL << (nr & (BITS_PER_LONG - 1)));
	__BITOPS_LOOP(addr, mask, __BITOPS_AND, __BITOPS_NO_BARRIER);
}

static inline void change_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long mask;

#ifdef CONFIG_HAVE_MARCH_ZEC12_FEATURES
	if (__builtin_constant_p(nr)) {
		unsigned char *caddr = __bitops_byte(nr, ptr);

		asm volatile(
			"xi	%0,%b1\n"
			: "+Q" (*caddr)
			: "i" (1 << (nr & 7))
			: "cc", "memory");
		return;
	}
#endif
	mask = 1UL << (nr & (BITS_PER_LONG - 1));
	__BITOPS_LOOP(addr, mask, __BITOPS_XOR, __BITOPS_NO_BARRIER);
}

static inline int
test_and_set_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long old, mask;

	mask = 1UL << (nr & (BITS_PER_LONG - 1));
	old = __BITOPS_LOOP(addr, mask, __BITOPS_OR, __BITOPS_BARRIER);
	return (old & mask) != 0;
}

static inline int
test_and_clear_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long old, mask;

	mask = ~(1UL << (nr & (BITS_PER_LONG - 1)));
	old = __BITOPS_LOOP(addr, mask, __BITOPS_AND, __BITOPS_BARRIER);
	return (old & ~mask) != 0;
}

static inline int
test_and_change_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned long *addr = __bitops_word(nr, ptr);
	unsigned long old, mask;

	mask = 1UL << (nr & (BITS_PER_LONG - 1));
	old = __BITOPS_LOOP(addr, mask, __BITOPS_XOR, __BITOPS_BARRIER);
	return (old & mask) != 0;
}

static inline void __set_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);

	*addr |= 1 << (nr & 7);
}

static inline void 
__clear_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);

	*addr &= ~(1 << (nr & 7));
}

static inline void __change_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);

	*addr ^= 1 << (nr & 7);
}

static inline int
__test_and_set_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);
	unsigned char ch;

	ch = *addr;
	*addr |= 1 << (nr & 7);
	return (ch >> (nr & 7)) & 1;
}

static inline int
__test_and_clear_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);
	unsigned char ch;

	ch = *addr;
	*addr &= ~(1 << (nr & 7));
	return (ch >> (nr & 7)) & 1;
}

static inline int
__test_and_change_bit(unsigned long nr, volatile unsigned long *ptr)
{
	unsigned char *addr = __bitops_byte(nr, ptr);
	unsigned char ch;

	ch = *addr;
	*addr ^= 1 << (nr & 7);
	return (ch >> (nr & 7)) & 1;
}

static inline int test_bit(unsigned long nr, const volatile unsigned long *ptr)
{
	const volatile unsigned char *addr;

	addr = ((const volatile unsigned char *)ptr);
	addr += (nr ^ (BITS_PER_LONG - 8)) >> 3;
	return (*addr >> (nr & 7)) & 1;
}

/*
 * Functions which use MSB0 bit numbering.
 * On an s390x system the bits are numbered:
 *   |0..............63|64............127|128...........191|192...........255|
 * and on s390:
 *   |0.....31|32....63|64....95|96...127|128..159|160..191|192..223|224..255|
 */
unsigned long find_first_bit_inv(const unsigned long *addr, unsigned long size);
unsigned long find_next_bit_inv(const unsigned long *addr, unsigned long size,
				unsigned long offset);

static inline void set_bit_inv(unsigned long nr, volatile unsigned long *ptr)
{
	return set_bit(nr ^ (BITS_PER_LONG - 1), ptr);
}

static inline void clear_bit_inv(unsigned long nr, volatile unsigned long *ptr)
{
	return clear_bit(nr ^ (BITS_PER_LONG - 1), ptr);
}

static inline void __set_bit_inv(unsigned long nr, volatile unsigned long *ptr)
{
	return __set_bit(nr ^ (BITS_PER_LONG - 1), ptr);
}

static inline void __clear_bit_inv(unsigned long nr, volatile unsigned long *ptr)
{
	return __clear_bit(nr ^ (BITS_PER_LONG - 1), ptr);
}

static inline int test_bit_inv(unsigned long nr,
			       const volatile unsigned long *ptr)
{
	return test_bit(nr ^ (BITS_PER_LONG - 1), ptr);
}

#ifdef CONFIG_HAVE_MARCH_Z9_109_FEATURES

/**
 * __flogr - find leftmost one
 * @word - The word to search
 *
 * Returns the bit number of the most significant bit set,
 * where the most significant bit has bit number 0.
 * If no bit is set this function returns 64.
 */
static inline unsigned char __flogr(unsigned long word)
{
	if (__builtin_constant_p(word)) {
		unsigned long bit = 0;

		if (!word)
			return 64;
		if (!(word & 0xffffffff00000000UL)) {
			word <<= 32;
			bit += 32;
		}
		if (!(word & 0xffff000000000000UL)) {
			word <<= 16;
			bit += 16;
		}
		if (!(word & 0xff00000000000000UL)) {
			word <<= 8;
			bit += 8;
		}
		if (!(word & 0xf000000000000000UL)) {
			word <<= 4;
			bit += 4;
		}
		if (!(word & 0xc000000000000000UL)) {
			word <<= 2;
			bit += 2;
		}
		if (!(word & 0x8000000000000000UL)) {
			word <<= 1;
			bit += 1;
		}
		return bit;
	} else {
		register unsigned long bit asm("4") = word;
		register unsigned long out asm("5");

		asm volatile(
			"       flogr   %[bit],%[bit]\n"
			: [bit] "+d" (bit), [out] "=d" (out) : : "cc");
		return bit;
	}
}

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
	return __flogr(-word & word) ^ (BITS_PER_LONG - 1);
}

/**
 * ffs - find first bit set
 * @word: the word to search
 *
 * This is defined the same way as the libc and
 * compiler builtin ffs routines (man ffs).
 */
static inline int ffs(int word)
{
	unsigned long mask = 2 * BITS_PER_LONG - 1;
	unsigned int val = (unsigned int)word;

	return (1 + (__flogr(-val & val) ^ (BITS_PER_LONG - 1))) & mask;
}

/**
 * __fls - find last (most-significant) set bit in a long word
 * @word: the word to search
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned long __fls(unsigned long word)
{
	return __flogr(word) ^ (BITS_PER_LONG - 1);
}

/**
 * fls64 - find last set bit in a 64-bit word
 * @word: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffsll, but returns the position of the most significant set bit.
 *
 * fls64(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 64.
 */
static inline int fls64(unsigned long word)
{
	unsigned long mask = 2 * BITS_PER_LONG - 1;

	return (1 + (__flogr(word) ^ (BITS_PER_LONG - 1))) & mask;
}

/**
 * fls - find last (most-significant) bit set
 * @word: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(int word)
{
	return fls64((unsigned int)word);
}

#else /* CONFIG_HAVE_MARCH_Z9_109_FEATURES */

#include <asm-generic/bitops/__ffs.h>
#include <asm-generic/bitops/ffs.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/fls64.h>

#endif /* CONFIG_HAVE_MARCH_Z9_109_FEATURES */

#include <asm-generic/bitops/ffz.h>
#include <asm-generic/bitops/find.h>
#include <asm-generic/bitops/hweight.h>
#include <asm-generic/bitops/lock.h>
#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/le.h>
#include <asm-generic/bitops/ext2-atomic-setbit.h>

#endif /* _S390_BITOPS_H */
