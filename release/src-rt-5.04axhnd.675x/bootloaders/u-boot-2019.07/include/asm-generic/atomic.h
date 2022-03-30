/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _ASM_GENERIC_ATOMIC_H
#define _ASM_GENERIC_ATOMIC_H

typedef struct { volatile int counter; } atomic_t;
#if BITS_PER_LONG == 32
typedef struct { volatile long long counter; } atomic64_t;
#else /* BIT_PER_LONG == 32 */
typedef struct { volatile long counter; } atomic64_t;
#endif

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	((v)->counter = (i))
#define atomic64_read(v)	atomic_read(v)
#define atomic64_set(v, i)	atomic_set(v, i)

static inline void atomic_add(int i, atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic_sub(int i, atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}

static inline void atomic_inc(atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	++v->counter;
	local_irq_restore(flags);
}

static inline void atomic_dec(atomic_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	--v->counter;
	local_irq_restore(flags);
}

static inline int atomic_dec_and_test(volatile atomic_t *v)
{
	unsigned long flags = 0;
	int val;

	local_irq_save(flags);
	val = v->counter;
	v->counter = val -= 1;
	local_irq_restore(flags);

	return val == 0;
}

static inline int atomic_add_negative(int i, volatile atomic_t *v)
{
	unsigned long flags = 0;
	int val;

	local_irq_save(flags);
	val = v->counter;
	v->counter = val += i;
	local_irq_restore(flags);

	return val < 0;
}

static inline void atomic_clear_mask(unsigned long mask, unsigned long *addr)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	*addr &= ~mask;
	local_irq_restore(flags);
}

#if BITS_PER_LONG == 32

static inline void atomic64_add(long long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic64_sub(long long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}

#else /* BIT_PER_LONG == 32 */

static inline void atomic64_add(long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += i;
	local_irq_restore(flags);
}

static inline void atomic64_sub(long i, volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= i;
	local_irq_restore(flags);
}
#endif

static inline void atomic64_inc(volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter += 1;
	local_irq_restore(flags);
}

static inline void atomic64_dec(volatile atomic64_t *v)
{
	unsigned long flags = 0;

	local_irq_save(flags);
	v->counter -= 1;
	local_irq_restore(flags);
}

#endif
