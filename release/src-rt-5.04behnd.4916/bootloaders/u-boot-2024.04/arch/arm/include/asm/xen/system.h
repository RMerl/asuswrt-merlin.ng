/* SPDX-License-Identifier: GPL-2.0
 *
 * (C) 2014 Karim Allah Ahmed <karim.allah.ahmed@gmail.com>
 * (C) 2020, EPAM Systems Inc.
 */
#ifndef _ASM_ARM_XEN_SYSTEM_H
#define _ASM_ARM_XEN_SYSTEM_H

#include <compiler.h>
#include <asm/bitops.h>

/* If *ptr == old, then store new there (and return new).
 * Otherwise, return the old value.
 * Atomic.
 */
#define synch_cmpxchg(ptr, old, new) \
({ __typeof__(*ptr) stored = old; \
	__atomic_compare_exchange_n(ptr, &stored, new, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? new : old; \
})

/* As test_and_clear_bit, but using __ATOMIC_SEQ_CST */
static inline int synch_test_and_clear_bit(int nr, volatile void *addr)
{
	u8 *byte = ((u8 *)addr) + (nr >> 3);
	u8 bit = 1 << (nr & 7);
	u8 orig;

	orig = __atomic_fetch_and(byte, ~bit, __ATOMIC_SEQ_CST);

	return (orig & bit) != 0;
}

/* As test_and_set_bit, but using __ATOMIC_SEQ_CST */
static inline int synch_test_and_set_bit(int nr, volatile void *base)
{
	u8 *byte = ((u8 *)base) + (nr >> 3);
	u8 bit = 1 << (nr & 7);
	u8 orig;

	orig = __atomic_fetch_or(byte, bit, __ATOMIC_SEQ_CST);

	return (orig & bit) != 0;
}

/* As set_bit, but using __ATOMIC_SEQ_CST */
static inline void synch_set_bit(int nr, volatile void *addr)
{
	synch_test_and_set_bit(nr, addr);
}

/* As clear_bit, but using __ATOMIC_SEQ_CST */
static inline void synch_clear_bit(int nr, volatile void *addr)
{
	synch_test_and_clear_bit(nr, addr);
}

/* As test_bit, but with a following memory barrier. */
//static inline int synch_test_bit(int nr, volatile void *addr)
static inline int synch_test_bit(int nr, const void *addr)
{
	int result;

	result = test_bit(nr, addr);
	barrier();
	return result;
}

#define xchg(ptr, v)	__atomic_exchange_n(ptr, v, __ATOMIC_SEQ_CST)
#define xchg(ptr, v)	__atomic_exchange_n(ptr, v, __ATOMIC_SEQ_CST)

#define xen_mb()	mb()
#define xen_rmb()	rmb()
#define xen_wmb()	wmb()

#define to_phys(x)		((unsigned long)(x))
#define to_virt(x)		((void *)(x))

#define PFN_UP(x)		(unsigned long)(((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)		(unsigned long)((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)		((unsigned long)(x) << PAGE_SHIFT)
#define PHYS_PFN(x)		(unsigned long)((x) >> PAGE_SHIFT)

#define virt_to_pfn(_virt)	(PFN_DOWN(to_phys(_virt)))
#define virt_to_mfn(_virt)	(PFN_DOWN(to_phys(_virt)))
#define mfn_to_virt(_mfn)	(to_virt(PFN_PHYS(_mfn)))
#define pfn_to_virt(_pfn)	(to_virt(PFN_PHYS(_pfn)))

#endif
