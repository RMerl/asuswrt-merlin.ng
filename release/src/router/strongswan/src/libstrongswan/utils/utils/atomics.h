/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup atomics_i atomics
 * @{ @ingroup utils_i
 */

#ifndef ATOMICS_H_
#define ATOMICS_H_

/**
 * Special type to count references
 */
typedef u_int refcount_t;

/* use __atomic* built-ins with clang, if available (note that clang also
 * defines __GNUC__, however only claims to be GCC 4.2) */
#if defined(__clang__)
# if __has_builtin(__atomic_add_fetch)
#  define HAVE_GCC_ATOMIC_OPERATIONS
# endif
/* use __atomic* built-ins with GCC 4.7 and newer */
#elif defined(__GNUC__)
# if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#  define HAVE_GCC_ATOMIC_OPERATIONS
# endif
#endif

#ifdef HAVE_GCC_ATOMIC_OPERATIONS

#define ref_get(ref) __atomic_add_fetch(ref, 1, __ATOMIC_RELAXED)
/* The relaxed memory model works fine for increments as these (usually) don't
 * change the state of refcounted objects.  But here we have to ensure that we
 * free the right stuff if ref counted objects are mutable.  So we have to sync
 * with other threads that call ref_put().  It would be sufficient to use
 * __ATOMIC_RELEASE here and then call __atomic_thread_fence() with
 * __ATOMIC_ACQUIRE if we reach 0, but since we don't have control over the use
 * of ref_put() we have to make sure. */
#define ref_put(ref) (!__atomic_sub_fetch(ref, 1, __ATOMIC_ACQ_REL))
#define ref_cur(ref) __atomic_load_n(ref, __ATOMIC_RELAXED)

#define _cas_impl(ptr, oldval, newval) ({ typeof(*ptr) _old = oldval; \
			__atomic_compare_exchange_n(ptr, &_old, newval, FALSE, \
										__ATOMIC_SEQ_CST, __ATOMIC_RELAXED); })
#define cas_bool(ptr, oldval, newval) _cas_impl(ptr, oldval, newval)
#define cas_ptr(ptr, oldval, newval) _cas_impl(ptr, oldval, newval)

#elif defined(HAVE_GCC_SYNC_OPERATIONS)

#define ref_get(ref) __sync_add_and_fetch(ref, 1)
#define ref_put(ref) (!__sync_sub_and_fetch(ref, 1))
#define ref_cur(ref) __sync_fetch_and_add(ref, 0)

#define cas_bool(ptr, oldval, newval) \
					(__sync_bool_compare_and_swap(ptr, oldval, newval))
#define cas_ptr(ptr, oldval, newval) \
					(__sync_bool_compare_and_swap(ptr, oldval, newval))

#else /* !HAVE_GCC_ATOMIC_OPERATIONS && !HAVE_GCC_SYNC_OPERATIONS */

/**
 * Get a new reference.
 *
 * Increments the reference counter atomically.
 *
 * @param ref	pointer to ref counter
 * @return		new value of ref
 */
refcount_t ref_get(refcount_t *ref);

/**
 * Put back a unused reference.
 *
 * Decrements the reference counter atomically and
 * says if more references available.
 *
 * @param ref	pointer to ref counter
 * @return		TRUE if no more references counted
 */
bool ref_put(refcount_t *ref);

/**
 * Get the current value of the reference counter.
 *
 * @param ref	pointer to ref counter
 * @return		current value of ref
 */
refcount_t ref_cur(refcount_t *ref);

/**
 * Atomically replace value of ptr with newval if it currently equals oldval.
 *
 * @param ptr		pointer to variable
 * @param oldval	old value of the variable
 * @param newval	new value set if possible
 * @return			TRUE if value equaled oldval and newval was written
 */
bool cas_bool(bool *ptr, bool oldval, bool newval);

/**
 * Atomically replace value of ptr with newval if it currently equals oldval.
 *
 * @param ptr		pointer to variable
 * @param oldval	old value of the variable
 * @param newval	new value set if possible
 * @return			TRUE if value equaled oldval and newval was written
 */
bool cas_ptr(void **ptr, void *oldval, void *newval);

#endif /* HAVE_GCC_ATOMIC_OPERATIONS */

/**
 * Initialize atomics utility functions
 */
void atomics_init();

/**
 * Clean up atomics utility functions
 */
void atomics_deinit();

#endif /** ATOMICS_H_ @} */
