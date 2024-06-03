#ifndef __ASM_SH_BITOPS_H
#define __ASM_SH_BITOPS_H

#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

#ifdef __KERNEL__
#include <asm/irqflags.h>
/* For __swab32 */
#include <asm/byteorder.h>

static inline void set_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a |= mask;
	local_irq_restore(flags);
}

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()
static inline void clear_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a &= ~mask;
	local_irq_restore(flags);
}

static inline void change_bit(int nr, volatile void * addr)
{
	int	mask;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	*a ^= mask;
	local_irq_restore(flags);
}

static inline int test_and_set_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a |= mask;
	local_irq_restore(flags);

	return retval;
}

static inline int test_and_clear_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a &= ~mask;
	local_irq_restore(flags);

	return retval;
}

static inline int test_and_change_bit(int nr, volatile void * addr)
{
	int	mask, retval;
	volatile unsigned int *a = addr;
	unsigned long flags;

	a += nr >> 5;
	mask = 1 << (nr & 0x1f);
	local_irq_save(flags);
	retval = (mask & *a) != 0;
	*a ^= mask;
	local_irq_restore(flags);

	return retval;
}

static inline unsigned long ffz(unsigned long word)
{
	unsigned long result;

	__asm__("1:\n\t"
		"shlr	%1\n\t"
		"bt/s	1b\n\t"
		" add	#1, %0"
		: "=r" (result), "=r" (word)
		: "0" (~0L), "1" (word)
		: "t");
	return result;
}

/**
 * ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline int ffs (int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}
#define PLATFORM_FFS

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* __KERNEL__ */

#endif /* __ASM_SH_BITOPS_H */
