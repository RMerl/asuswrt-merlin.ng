#ifndef _ASM_X86_BARRIER_H
#define _ASM_X86_BARRIER_H

#include <asm/alternative.h>
#include <asm/nops.h>

/*
 * Force strict CPU ordering.
 * And yes, this is required on UP too when we're talking
 * to devices.
 */

#ifdef CONFIG_X86_32
/*
 * Some non-Intel clones support out of order store. wmb() ceases to be a
 * nop for these.
 */
#define mb() alternative("lock; addl $0,0(%%esp)", "mfence", X86_FEATURE_XMM2)
#define rmb() alternative("lock; addl $0,0(%%esp)", "lfence", X86_FEATURE_XMM2)
#define wmb() alternative("lock; addl $0,0(%%esp)", "sfence", X86_FEATURE_XMM)
#else
#define mb() 	asm volatile("mfence":::"memory")
#define rmb()	asm volatile("lfence":::"memory")
#define wmb()	asm volatile("sfence" ::: "memory")
#endif

/**
 * array_index_mask_nospec() - generate a mask that is ~0UL when the
 * 	bounds check succeeds and 0 otherwise
 * @index: array element index
 * @size: number of elements in array
 *
 * Returns:
 *     0 - (index < size)
 */
static inline unsigned long array_index_mask_nospec(unsigned long index,
		unsigned long size)
{
	unsigned long mask;

	asm ("cmp %1,%2; sbb %0,%0;"
			:"=r" (mask)
			:"r"(size),"r" (index)
			:"cc");
	return mask;
}

/* Override the default implementation from linux/nospec.h. */
#define array_index_mask_nospec array_index_mask_nospec

#ifdef CONFIG_X86_PPRO_FENCE
#define dma_rmb()	rmb()
#else
#define dma_rmb()	barrier()
#endif
#define dma_wmb()	barrier()

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	dma_rmb()
#define smp_wmb()	barrier()
#define set_mb(var, value) do { (void)xchg(&var, value); } while (0)
#else /* !SMP */
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#define set_mb(var, value) do { var = value; barrier(); } while (0)
#endif /* SMP */

#define read_barrier_depends()		do { } while (0)
#define smp_read_barrier_depends()	do { } while (0)

#if defined(CONFIG_X86_PPRO_FENCE)

/*
 * For this option x86 doesn't have a strong TSO memory
 * model and we should fall back to full barriers.
 */

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = ACCESS_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	smp_mb();							\
	___p1;								\
})

#else /* regular x86 TSO memory ordering */

#define smp_store_release(p, v)						\
do {									\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	ACCESS_ONCE(*p) = (v);						\
} while (0)

#define smp_load_acquire(p)						\
({									\
	typeof(*p) ___p1 = ACCESS_ONCE(*p);				\
	compiletime_assert_atomic_type(*p);				\
	barrier();							\
	___p1;								\
})

#endif

/* Atomic operations are already serializing on x86 */
#define smp_mb__before_atomic()	barrier()
#define smp_mb__after_atomic()	barrier()

/*
 * Stop RDTSC speculation. This is needed when you need to use RDTSC
 * (or get_cycles or vread that possibly accesses the TSC) in a defined
 * code region.
 */
static __always_inline void rdtsc_barrier(void)
{
	alternative_2("", "mfence", X86_FEATURE_MFENCE_RDTSC,
			  "lfence", X86_FEATURE_LFENCE_RDTSC);
}

#endif /* _ASM_X86_BARRIER_H */
