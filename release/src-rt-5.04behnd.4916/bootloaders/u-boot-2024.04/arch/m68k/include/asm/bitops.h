/*
 * bitops.h: Bit string operations on the m68k
 */

#ifndef _M68K_BITOPS_H
#define _M68K_BITOPS_H

#include <asm/byteorder.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

extern void set_bit(int nr, volatile void *addr);
extern void clear_bit(int nr, volatile void *addr);
extern void change_bit(int nr, volatile void *addr);
extern int test_and_clear_bit(int nr, volatile void *addr);
extern int test_and_change_bit(int nr, volatile void *addr);

#ifdef __KERNEL__


static inline int test_bit(int nr, __const__ volatile void *addr)
{
	__const__ unsigned int *p = (__const__ unsigned int *) addr;

	return (p[nr >> 5] & (1UL << (nr & 31))) != 0;
}

static inline int test_and_set_bit(int nr, volatile void *vaddr)
{
	char retval;

	volatile char *p = &((volatile char *)vaddr)[(nr^31) >> 3];
	__asm__ __volatile__ ("bset %2,(%4); sne %0"
	     : "=d" (retval), "=m" (*p)
	     : "di" (nr & 7), "m" (*p), "a" (p));

	return retval;
}

#define __ffs(x) (ffs(x) - 1)

/*
 *  * hweightN: returns the hamming weight (i.e. the number
 *   * of bits set) of a N-bit word
 *    */

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* __KERNEL__ */

#endif /* _M68K_BITOPS_H */
