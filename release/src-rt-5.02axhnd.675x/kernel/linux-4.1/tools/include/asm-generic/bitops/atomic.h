#ifndef _TOOLS_LINUX_ASM_GENERIC_BITOPS_ATOMIC_H_
#define _TOOLS_LINUX_ASM_GENERIC_BITOPS_ATOMIC_H_

#include <asm/types.h>

static inline void set_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] |= 1UL << (nr % __BITS_PER_LONG);
}

static inline void clear_bit(int nr, unsigned long *addr)
{
	addr[nr / __BITS_PER_LONG] &= ~(1UL << (nr % __BITS_PER_LONG));
}

static __always_inline int test_bit(unsigned int nr, const unsigned long *addr)
{
	return ((1UL << (nr % __BITS_PER_LONG)) &
		(((unsigned long *)addr)[nr / __BITS_PER_LONG])) != 0;
}

#endif /* _TOOLS_LINUX_ASM_GENERIC_BITOPS_ATOMIC_H_ */
