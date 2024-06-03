/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 1994 - 1997, 1999, 2000  Ralf Baechle (ralf@gnu.org)
 * Copyright (c) 2000  Silicon Graphics, Inc.
 */
#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H

#include <linux/types.h>
#include <asm/byteorder.h>		/* sigh ... */

#ifdef __KERNEL__

#include <asm/sgidefs.h>
#include <asm/system.h>

#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

/*
 * Only disable interrupt for kernel mode stuff to keep usermode stuff
 * that dares to use kernel include files alive.
 */
#define __bi_flags unsigned long flags
#define __bi_cli() __cli()
#define __bi_save_flags(x) __save_flags(x)
#define __bi_save_and_cli(x) __save_and_cli(x)
#define __bi_restore_flags(x) __restore_flags(x)
#else
#define __bi_flags
#define __bi_cli()
#define __bi_save_flags(x)
#define __bi_save_and_cli(x)
#define __bi_restore_flags(x)
#endif /* __KERNEL__ */

#ifdef CONFIG_CPU_HAS_LLSC

#include <asm/mipsregs.h>

/*
 * These functions for MIPS ISA > 1 are interrupt and SMP proof and
 * interrupt friendly
 */

/*
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is atomic and may not be reordered.  See __set_bit()
 * if you do not require the atomic guarantees.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static __inline__ void
set_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp;

	__asm__ __volatile__(
		"1:\tll\t%0, %1\t\t# set_bit\n\t"
		"or\t%0, %2\n\t"
		"sc\t%0, %1\n\t"
		"beqz\t%0, 1b"
		: "=&r" (temp), "=m" (*m)
		: "ir" (1UL << (nr & 0x1f)), "m" (*m));
}

/*
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static __inline__ void __set_bit(int nr, volatile void * addr)
{
	unsigned long * m = ((unsigned long *) addr) + (nr >> 5);

	*m |= 1UL << (nr & 31);
}
#define PLATFORM__SET_BIT

/*
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and may not be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */
static __inline__ void
clear_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp;

	__asm__ __volatile__(
		"1:\tll\t%0, %1\t\t# clear_bit\n\t"
		"and\t%0, %2\n\t"
		"sc\t%0, %1\n\t"
		"beqz\t%0, 1b\n\t"
		: "=&r" (temp), "=m" (*m)
		: "ir" (~(1UL << (nr & 0x1f))), "m" (*m));
}

/*
 * change_bit - Toggle a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * change_bit() is atomic and may not be reordered.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static __inline__ void
change_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp;

	__asm__ __volatile__(
		"1:\tll\t%0, %1\t\t# change_bit\n\t"
		"xor\t%0, %2\n\t"
		"sc\t%0, %1\n\t"
		"beqz\t%0, 1b"
		: "=&r" (temp), "=m" (*m)
		: "ir" (1UL << (nr & 0x1f)), "m" (*m));
}

/*
 * __change_bit - Toggle a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike change_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static __inline__ void __change_bit(int nr, volatile void * addr)
{
	unsigned long * m = ((unsigned long *) addr) + (nr >> 5);

	*m ^= 1UL << (nr & 31);
}

/*
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int
test_and_set_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp, res;

	__asm__ __volatile__(
		".set\tnoreorder\t\t# test_and_set_bit\n"
		"1:\tll\t%0, %1\n\t"
		"or\t%2, %0, %3\n\t"
		"sc\t%2, %1\n\t"
		"beqz\t%2, 1b\n\t"
		" and\t%2, %0, %3\n\t"
		".set\treorder"
		: "=&r" (temp), "=m" (*m), "=&r" (res)
		: "r" (1UL << (nr & 0x1f)), "m" (*m)
		: "memory");

	return res != 0;
}

/*
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_set_bit(int nr, volatile void * addr)
{
	int mask, retval;
	volatile int *a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;

	return retval;
}

/*
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int
test_and_clear_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp, res;

	__asm__ __volatile__(
		".set\tnoreorder\t\t# test_and_clear_bit\n"
		"1:\tll\t%0, %1\n\t"
		"or\t%2, %0, %3\n\t"
		"xor\t%2, %3\n\t"
		"sc\t%2, %1\n\t"
		"beqz\t%2, 1b\n\t"
		" and\t%2, %0, %3\n\t"
		".set\treorder"
		: "=&r" (temp), "=m" (*m), "=&r" (res)
		: "r" (1UL << (nr & 0x1f)), "m" (*m)
		: "memory");

	return res != 0;
}

/*
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;

	return retval;
}

/*
 * test_and_change_bit - Change a bit and return its new value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int
test_and_change_bit(int nr, volatile void *addr)
{
	unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
	unsigned long temp, res;

	__asm__ __volatile__(
		".set\tnoreorder\t\t# test_and_change_bit\n"
		"1:\tll\t%0, %1\n\t"
		"xor\t%2, %0, %3\n\t"
		"sc\t%2, %1\n\t"
		"beqz\t%2, 1b\n\t"
		" and\t%2, %0, %3\n\t"
		".set\treorder"
		: "=&r" (temp), "=m" (*m), "=&r" (res)
		: "r" (1UL << (nr & 0x1f)), "m" (*m)
		: "memory");

	return res != 0;
}

/*
 * __test_and_change_bit - Change a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;

	return retval;
}

#else /* MIPS I */

/*
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is atomic and may not be reordered.  See __set_bit()
 * if you do not require the atomic guarantees.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static __inline__ void set_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	*a |= mask;
	__bi_restore_flags(flags);
}

/*
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static __inline__ void __set_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	*a |= mask;
}

/*
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and may not be reordered.  However, it does
 * not contain a memory barrier, so if it is used for locking purposes,
 * you should call smp_mb__before_clear_bit() and/or smp_mb__after_clear_bit()
 * in order to ensure changes are visible on other processors.
 */
static __inline__ void clear_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	*a &= ~mask;
	__bi_restore_flags(flags);
}

/*
 * change_bit - Toggle a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * change_bit() is atomic and may not be reordered.
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static __inline__ void change_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	*a ^= mask;
	__bi_restore_flags(flags);
}

/*
 * __change_bit - Toggle a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike change_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static __inline__ void __change_bit(int nr, volatile void * addr)
{
	unsigned long * m = ((unsigned long *) addr) + (nr >> 5);

	*m ^= 1UL << (nr & 31);
}

/*
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int test_and_set_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a |= mask;
	__bi_restore_flags(flags);

	return retval;
}

/*
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_set_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a |= mask;

	return retval;
}

/*
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	__bi_restore_flags(flags);

	return retval;
}

/*
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a &= ~mask;

	return retval;
}

/*
 * test_and_change_bit - Change a bit and return its new value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static __inline__ int test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;
	__bi_flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	__bi_save_and_cli(flags);
	retval = (mask & *a) != 0;
	*a ^= mask;
	__bi_restore_flags(flags);

	return retval;
}

/*
 * __test_and_change_bit - Change a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static __inline__ int __test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile int	*a = addr;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	retval = (mask & *a) != 0;
	*a ^= mask;

	return retval;
}

#undef __bi_flags
#undef __bi_cli
#undef __bi_save_flags
#undef __bi_restore_flags

#endif /* MIPS I */

/*
 * test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 */
static __inline__ int test_bit(int nr, const volatile void *addr)
{
	return ((1UL << (nr & 31)) & (((const unsigned int *) addr)[nr >> 5])) != 0;
}

#ifndef __MIPSEB__

/* Little endian versions. */

/*
 * find_first_zero_bit - find the first zero bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit-number of the first zero bit, not the number of the byte
 * containing a bit.
 */
static __inline__ int find_first_zero_bit (void *addr, unsigned size)
{
	unsigned long dummy;
	int res;

	if (!size)
		return 0;

	__asm__ (".set\tnoreorder\n\t"
		".set\tnoat\n"
		"1:\tsubu\t$1,%6,%0\n\t"
		"blez\t$1,2f\n\t"
		"lw\t$1,(%5)\n\t"
		"addiu\t%5,4\n\t"
#if (_MIPS_ISA == _MIPS_ISA_MIPS2 ) || (_MIPS_ISA == _MIPS_ISA_MIPS3 ) || \
    (_MIPS_ISA == _MIPS_ISA_MIPS4 ) || (_MIPS_ISA == _MIPS_ISA_MIPS5 ) || \
    (_MIPS_ISA == _MIPS_ISA_MIPS32) || (_MIPS_ISA == _MIPS_ISA_MIPS64)
		"beql\t%1,$1,1b\n\t"
		"addiu\t%0,32\n\t"
#else
		"addiu\t%0,32\n\t"
		"beq\t%1,$1,1b\n\t"
		"nop\n\t"
		"subu\t%0,32\n\t"
#endif
#ifdef __MIPSEB__
#error "Fix this for big endian"
#endif /* __MIPSEB__ */
		"li\t%1,1\n"
		"1:\tand\t%2,$1,%1\n\t"
		"beqz\t%2,2f\n\t"
		"sll\t%1,%1,1\n\t"
		"bnez\t%1,1b\n\t"
		"add\t%0,%0,1\n\t"
		".set\tat\n\t"
		".set\treorder\n"
		"2:"
		: "=r" (res), "=r" (dummy), "=r" (addr)
		: "0" ((signed int) 0), "1" ((unsigned int) 0xffffffff),
		  "2" (addr), "r" (size)
		: "$1");

	return res;
}

/*
 * find_next_zero_bit - find the first zero bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The maximum size to search
 */
static __inline__ int find_next_zero_bit (void * addr, int size, int offset)
{
	unsigned int *p = ((unsigned int *) addr) + (offset >> 5);
	int set = 0, bit = offset & 31, res;
	unsigned long dummy;

	if (bit) {
		/*
		 * Look for zero in first byte
		 */
#ifdef __MIPSEB__
#error "Fix this for big endian byte order"
#endif
		__asm__(".set\tnoreorder\n\t"
			".set\tnoat\n"
			"1:\tand\t$1,%4,%1\n\t"
			"beqz\t$1,1f\n\t"
			"sll\t%1,%1,1\n\t"
			"bnez\t%1,1b\n\t"
			"addiu\t%0,1\n\t"
			".set\tat\n\t"
			".set\treorder\n"
			"1:"
			: "=r" (set), "=r" (dummy)
			: "0" (0), "1" (1 << bit), "r" (*p)
			: "$1");
		if (set < (32 - bit))
			return set + offset;
		set = 32 - bit;
		p++;
	}
	/*
	 * No zero yet, search remaining full bytes for a zero
	 */
	res = find_first_zero_bit(p, size - 32 * (p - (unsigned int *) addr));
	return offset + set + res;
}

#endif /* !(__MIPSEB__) */

/*
 * ffz - find first zero in word.
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static __inline__ unsigned long ffz(unsigned long word)
{
	unsigned int	__res;
	unsigned int	mask = 1;

	__asm__ (
		".set\tnoreorder\n\t"
		".set\tnoat\n\t"
		"move\t%0,$0\n"
		"1:\tand\t$1,%2,%1\n\t"
		"beqz\t$1,2f\n\t"
		"sll\t%1,1\n\t"
		"bnez\t%1,1b\n\t"
		"addiu\t%0,1\n\t"
		".set\tat\n\t"
		".set\treorder\n"
		"2:\n\t"
		: "=&r" (__res), "=r" (mask)
		: "r" (word), "1" (mask)
		: "$1");

	return __res;
}

#ifdef __KERNEL__

/*
 * hweightN - returns the hamming weight of a N-bit word
 * @x: the word to weigh
 *
 * The Hamming Weight of a number is the total number of bits set in it.
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* __KERNEL__ */

#ifdef __MIPSEB__
/*
 * find_next_zero_bit - find the first zero bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The maximum size to search
 */
static __inline__ int find_next_zero_bit(void *addr, int size, int offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (32-offset);
		if (size < 32)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while (size & ~31UL) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
found_middle:
	return result + ffz(tmp);
}

/* Linus sez that gcc can optimize the following correctly, we'll see if this
 * holds on the Sparc as it does for the ALPHA.
 */

#if 0 /* Fool kernel-doc since it doesn't do macros yet */
/*
 * find_first_zero_bit - find the first zero bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit-number of the first zero bit, not the number of the byte
 * containing a bit.
 */
static int find_first_zero_bit (void *addr, unsigned size);
#endif

#define find_first_zero_bit(addr, size) \
	find_next_zero_bit((addr), (size), 0)

#endif /* (__MIPSEB__) */

/* Now for the ext2 filesystem bit operations and helper routines. */

#ifdef __MIPSEB__
static __inline__ int ext2_set_bit(int nr, void * addr)
{
	int		mask, retval, flags;
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	save_and_cli(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR |= mask;
	restore_flags(flags);
	return retval;
}

static __inline__ int ext2_clear_bit(int nr, void * addr)
{
	int		mask, retval, flags;
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	save_and_cli(flags);
	retval = (mask & *ADDR) != 0;
	*ADDR &= ~mask;
	restore_flags(flags);
	return retval;
}

static __inline__ int ext2_test_bit(int nr, const void * addr)
{
	int			mask;
	const unsigned char	*ADDR = (const unsigned char *) addr;

	ADDR += nr >> 3;
	mask = 1 << (nr & 0x07);
	return ((mask & *ADDR) != 0);
}

#define ext2_find_first_zero_bit(addr, size) \
	ext2_find_next_zero_bit((addr), (size), 0)

static __inline__ unsigned long ext2_find_next_zero_bit(void *addr, unsigned long size, unsigned long offset)
{
	unsigned long *p = ((unsigned long *) addr) + (offset >> 5);
	unsigned long result = offset & ~31UL;
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= 31UL;
	if(offset) {
		/* We hold the little endian value in tmp, but then the
		 * shift is illegal. So we could keep a big endian value
		 * in tmp, like this:
		 *
		 * tmp = __swab32(*(p++));
		 * tmp |= ~0UL >> (32-offset);
		 *
		 * but this would decrease preformance, so we change the
		 * shift:
		 */
		tmp = *(p++);
		tmp |= __swab32(~0UL >> (32-offset));
		if(size < 32)
			goto found_first;
		if(~tmp)
			goto found_middle;
		size -= 32;
		result += 32;
	}
	while(size & ~31UL) {
		if(~(tmp = *(p++)))
			goto found_middle;
		result += 32;
		size -= 32;
	}
	if(!size)
		return result;
	tmp = *p;

found_first:
	/* tmp is little endian, so we would have to swab the shift,
	 * see above. But then we have to swab tmp below for ffz, so
	 * we might as well do this here.
	 */
	return result + ffz(__swab32(tmp) | (~0UL << size));
found_middle:
	return result + ffz(__swab32(tmp));
}
#else /* !(__MIPSEB__) */

/* Native ext2 byte ordering, just collapse using defines. */
#define ext2_set_bit(nr, addr) test_and_set_bit((nr), (addr))
#define ext2_clear_bit(nr, addr) test_and_clear_bit((nr), (addr))
#define ext2_test_bit(nr, addr) test_bit((nr), (addr))
#define ext2_find_first_zero_bit(addr, size) find_first_zero_bit((addr), (size))
#define ext2_find_next_zero_bit(addr, size, offset) \
		find_next_zero_bit((addr), (size), (offset))

#endif /* !(__MIPSEB__) */

/*
 * Bitmap functions for the minix filesystem.
 * FIXME: These assume that Minix uses the native byte/bitorder.
 * This limits the Minix filesystem's value for data exchange very much.
 */
#define minix_test_and_set_bit(nr,addr) test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr) test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr) test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size) find_first_zero_bit(addr,size)

#endif /* _ASM_BITOPS_H */
