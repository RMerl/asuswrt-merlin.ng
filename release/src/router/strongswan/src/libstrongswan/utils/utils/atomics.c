/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include <utils/utils.h>

#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)

#include <threading/spinlock.h>

/**
 * Spinlock for ref_get/put
 */
static spinlock_t *ref_lock;

/**
 * Increase refcount
 */
refcount_t ref_get(refcount_t *ref)
{
	refcount_t current;

	ref_lock->lock(ref_lock);
	current = ++(*ref);
	ref_lock->unlock(ref_lock);

	return current;
}

/**
 * Decrease refcount
 */
bool ref_put(refcount_t *ref)
{
	bool more_refs;

	ref_lock->lock(ref_lock);
	more_refs = --(*ref) > 0;
	ref_lock->unlock(ref_lock);
	return !more_refs;
}

/**
 * Current refcount
 */
refcount_t ref_cur(refcount_t *ref)
{
	refcount_t current;

	ref_lock->lock(ref_lock);
	current = *ref;
	ref_lock->unlock(ref_lock);

	return current;
}

/**
 * Spinlock for all compare and swap operations.
 */
static spinlock_t *cas_lock;

/**
 * Compare and swap if equal to old value
 */
#define _cas_impl(name, type) \
bool cas_##name(type *ptr, type oldval, type newval) \
{ \
	bool swapped; \
	cas_lock->lock(cas_lock); \
	if ((swapped = (*ptr == oldval))) { *ptr = newval; } \
	cas_lock->unlock(cas_lock); \
	return swapped; \
}

_cas_impl(bool, bool)
_cas_impl(ptr, void*)

#endif /* !HAVE_GCC_ATOMIC_OPERATIONS && !HAVE_GCC_SYNC_OPERATIONS */

/**
 * See header
 */
void atomics_init()
{
#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)
	ref_lock = spinlock_create();
	cas_lock = spinlock_create();
#endif
}

/**
 * See header
 */
void atomics_deinit()
{
#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)
	ref_lock->destroy(ref_lock);
	cas_lock->destroy(cas_lock);
#endif
}
