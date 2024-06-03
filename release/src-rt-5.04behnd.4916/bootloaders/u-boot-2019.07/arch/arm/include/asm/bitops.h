/*
 * Copyright 1995, Russell King.
 * Various bits and pieces copyrights include:
 *  Linus Torvalds (test_bit).
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 *
 * Please note that the code in this file should never be included
 * from user space.  Many of these are not implemented in assembler
 * since they would be too costly.  Also, they require priviledged
 * instructions (which are not available from user mode) to ensure
 * that they are atomic.
 */

#ifndef __ASM_ARM_BITOPS_H
#define __ASM_ARM_BITOPS_H

#include <asm-generic/bitops/__ffs.h>

#ifdef __KERNEL__

#include <asm/proc-armv/system.h>

#define smp_mb__before_clear_bit()	do { } while (0)
#define smp_mb__after_clear_bit()	do { } while (0)

/*
 * Function prototypes to keep gcc -Wall happy.
 */
extern void set_bit(int nr, volatile void * addr);

extern void clear_bit(int nr, volatile void * addr);

extern void change_bit(int nr, volatile void * addr);

static inline void __change_bit(int nr, volatile void *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p ^= mask;
}

static inline int __test_and_set_bit(int nr, volatile void *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old | mask;
	return (old & mask) != 0;
}

static inline int test_and_set_bit(int nr, volatile void * addr)
{
	unsigned long flags = 0;
	int out;

	local_irq_save(flags);
	out = __test_and_set_bit(nr, addr);
	local_irq_restore(flags);

	return out;
}

static inline int __test_and_clear_bit(int nr, volatile void *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old & ~mask;
	return (old & mask) != 0;
}

static inline int test_and_clear_bit(int nr, volatile void * addr)
{
	unsigned long flags = 0;
	int out;

	local_irq_save(flags);
	out = __test_and_clear_bit(nr, addr);
	local_irq_restore(flags);

	return out;
}

extern int test_and_change_bit(int nr, volatile void * addr);

static inline int __test_and_change_bit(int nr, volatile void *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old ^ mask;
	return (old & mask) != 0;
}

/*
 * This routine doesn't need to be atomic.
 */
static inline int test_bit(int nr, const void * addr)
{
    return ((unsigned char *) addr)[nr >> 3] & (1U << (nr & 7));
}

static inline int __ilog2(unsigned int x)
{
	return generic_fls(x) - 1;
}

#define ffz(x)  __ffs(~(x))

static inline int find_next_zero_bit(void *addr, int size, int offset)
{
	unsigned long *p = ((unsigned long *)addr) + (offset / BITS_PER_LONG);
	unsigned long result = offset & ~(BITS_PER_LONG - 1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG - 1);
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (BITS_PER_LONG - offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG - 1)) {
		tmp = *(p++);
		if (~tmp)
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
found_middle:
	return result + ffz(tmp);
}

/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#define find_first_zero_bit(addr, size) \
	find_next_zero_bit((addr), (size), 0)

#define ext2_set_bit			test_and_set_bit
#define ext2_clear_bit			test_and_clear_bit
#define ext2_test_bit			test_bit
#define ext2_find_first_zero_bit	find_first_zero_bit
#define ext2_find_next_zero_bit		find_next_zero_bit

/* Bitmap functions for the minix filesystem. */
#define minix_test_and_set_bit(nr,addr)	test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr)		set_bit(nr,addr)
#define minix_test_and_clear_bit(nr,addr)	test_and_clear_bit(nr,addr)
#define minix_test_bit(nr,addr)		test_bit(nr,addr)
#define minix_find_first_zero_bit(addr,size)	find_first_zero_bit(addr,size)

#endif /* __KERNEL__ */

#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/fls64.h>

#endif /* _ARM_BITOPS_H */
