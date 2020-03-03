/*
 * Assembly implementation of the mutex fastpath, based on atomic
 * decrement/increment.
 *
 * started by Ingo Molnar:
 *
 *  Copyright (C) 2004, 2005, 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 */
#ifndef _ASM_X86_MUTEX_32_H
#define _ASM_X86_MUTEX_32_H

#include <asm/alternative.h>

/**
 *  __mutex_fastpath_lock - try to take the lock by moving the count
 *                          from 1 to a 0 value
 *  @count: pointer of type atomic_t
 *  @fn: function to call if the original value was not 1
 *
 * Change the count from 1 to a value lower than 1, and call <fn> if it
 * wasn't 1 originally. This function MUST leave the value lower than 1
 * even when the "1" assertion wasn't true.
 */
#define __mutex_fastpath_lock(count, fail_fn)			\
do {								\
	unsigned int dummy;					\
								\
	typecheck(atomic_t *, count);				\
	typecheck_fn(void (*)(atomic_t *), fail_fn);		\
								\
	asm volatile(LOCK_PREFIX "   decl (%%eax)\n"		\
		     "   jns 1f	\n"				\
		     "   call " #fail_fn "\n"			\
		     "1:\n"					\
		     : "=a" (dummy)				\
		     : "a" (count)				\
		     : "memory", "ecx", "edx");			\
} while (0)


/**
 *  __mutex_fastpath_lock_retval - try to take the lock by moving the count
 *                                 from 1 to a 0 value
 *  @count: pointer of type atomic_t
 *
 * Change the count from 1 to a value lower than 1. This function returns 0
 * if the fastpath succeeds, or -1 otherwise.
 */
static inline int __mutex_fastpath_lock_retval(atomic_t *count)
{
	if (unlikely(atomic_dec_return(count) < 0))
		return -1;
	else
		return 0;
}

/**
 *  __mutex_fastpath_unlock - try to promote the mutex from 0 to 1
 *  @count: pointer of type atomic_t
 *  @fail_fn: function to call if the original value was not 0
 *
 * try to promote the mutex from 0 to 1. if it wasn't 0, call <fail_fn>.
 * In the failure case, this function is allowed to either set the value
 * to 1, or to set it to a value lower than 1.
 *
 * If the implementation sets it to a value of lower than 1, the
 * __mutex_slowpath_needs_to_unlock() macro needs to return 1, it needs
 * to return 0 otherwise.
 */
#define __mutex_fastpath_unlock(count, fail_fn)			\
do {								\
	unsigned int dummy;					\
								\
	typecheck(atomic_t *, count);				\
	typecheck_fn(void (*)(atomic_t *), fail_fn);		\
								\
	asm volatile(LOCK_PREFIX "   incl (%%eax)\n"		\
		     "   jg	1f\n"				\
		     "   call " #fail_fn "\n"			\
		     "1:\n"					\
		     : "=a" (dummy)				\
		     : "a" (count)				\
		     : "memory", "ecx", "edx");			\
} while (0)

#define __mutex_slowpath_needs_to_unlock()	1

/**
 * __mutex_fastpath_trylock - try to acquire the mutex, without waiting
 *
 *  @count: pointer of type atomic_t
 *  @fail_fn: fallback function
 *
 * Change the count from 1 to a value lower than 1, and return 0 (failure)
 * if it wasn't 1 originally, or return 1 (success) otherwise. This function
 * MUST leave the value lower than 1 even when the "1" assertion wasn't true.
 * Additionally, if the value was < 0 originally, this function must not leave
 * it to 0 on failure.
 */
static inline int __mutex_fastpath_trylock(atomic_t *count,
					   int (*fail_fn)(atomic_t *))
{
	/* cmpxchg because it never induces a false contention state. */
	if (likely(atomic_cmpxchg(count, 1, 0) == 1))
		return 1;

	return 0;
}

#endif /* _ASM_X86_MUTEX_32_H */
