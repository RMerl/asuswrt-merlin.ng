/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA
 *
 * GPL HEADER END
 */
/* Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * Author: liang@whamcloud.com
 */

#define DEBUG_SUBSYSTEM S_LNET

#include "../../include/linux/libcfs/libcfs.h"


/** destroy cpu-partition lock, see libcfs_private.h for more detail */
void
cfs_percpt_lock_free(struct cfs_percpt_lock *pcl)
{
	LASSERT(pcl->pcl_locks != NULL);
	LASSERT(!pcl->pcl_locked);

	cfs_percpt_free(pcl->pcl_locks);
	LIBCFS_FREE(pcl, sizeof(*pcl));
}
EXPORT_SYMBOL(cfs_percpt_lock_free);

/**
 * create cpu-partition lock, see libcfs_private.h for more detail.
 *
 * cpu-partition lock is designed for large-scale SMP system, so we need to
 * reduce cacheline conflict as possible as we can, that's the
 * reason we always allocate cacheline-aligned memory block.
 */
struct cfs_percpt_lock *
cfs_percpt_lock_alloc(struct cfs_cpt_table *cptab)
{
	struct cfs_percpt_lock	*pcl;
	spinlock_t		*lock;
	int			i;

	/* NB: cptab can be NULL, pcl will be for HW CPUs on that case */
	LIBCFS_ALLOC(pcl, sizeof(*pcl));
	if (pcl == NULL)
		return NULL;

	pcl->pcl_cptab = cptab;
	pcl->pcl_locks = cfs_percpt_alloc(cptab, sizeof(*lock));
	if (pcl->pcl_locks == NULL) {
		LIBCFS_FREE(pcl, sizeof(*pcl));
		return NULL;
	}

	cfs_percpt_for_each(lock, i, pcl->pcl_locks)
		spin_lock_init(lock);

	return pcl;
}
EXPORT_SYMBOL(cfs_percpt_lock_alloc);

/**
 * lock a CPU partition
 *
 * \a index != CFS_PERCPT_LOCK_EX
 *     hold private lock indexed by \a index
 *
 * \a index == CFS_PERCPT_LOCK_EX
 *     exclusively lock @pcl and nobody can take private lock
 */
void
cfs_percpt_lock(struct cfs_percpt_lock *pcl, int index)
{
	int	ncpt = cfs_cpt_number(pcl->pcl_cptab);
	int	i;

	LASSERT(index >= CFS_PERCPT_LOCK_EX && index < ncpt);

	if (ncpt == 1) {
		index = 0;
	} else { /* serialize with exclusive lock */
		while (pcl->pcl_locked)
			cpu_relax();
	}

	if (likely(index != CFS_PERCPT_LOCK_EX)) {
		spin_lock(pcl->pcl_locks[index]);
		return;
	}

	/* exclusive lock request */
	for (i = 0; i < ncpt; i++) {
		spin_lock(pcl->pcl_locks[i]);
		if (i == 0) {
			LASSERT(!pcl->pcl_locked);
			/* nobody should take private lock after this
			 * so I wouldn't starve for too long time */
			pcl->pcl_locked = 1;
		}
	}
}
EXPORT_SYMBOL(cfs_percpt_lock);

/** unlock a CPU partition */
void
cfs_percpt_unlock(struct cfs_percpt_lock *pcl, int index)
{
	int	ncpt = cfs_cpt_number(pcl->pcl_cptab);
	int	i;

	index = ncpt == 1 ? 0 : index;

	if (likely(index != CFS_PERCPT_LOCK_EX)) {
		spin_unlock(pcl->pcl_locks[index]);
		return;
	}

	for (i = ncpt - 1; i >= 0; i--) {
		if (i == 0) {
			LASSERT(pcl->pcl_locked);
			pcl->pcl_locked = 0;
		}
		spin_unlock(pcl->pcl_locks[i]);
	}
}
EXPORT_SYMBOL(cfs_percpt_unlock);


/** free cpu-partition refcount */
void
cfs_percpt_atomic_free(atomic_t **refs)
{
	cfs_percpt_free(refs);
}
EXPORT_SYMBOL(cfs_percpt_atomic_free);

/** allocate cpu-partition refcount with initial value @init_val */
atomic_t **
cfs_percpt_atomic_alloc(struct cfs_cpt_table *cptab, int init_val)
{
	atomic_t	**refs;
	atomic_t	*ref;
	int		i;

	refs = cfs_percpt_alloc(cptab, sizeof(*ref));
	if (refs == NULL)
		return NULL;

	cfs_percpt_for_each(ref, i, refs)
		atomic_set(ref, init_val);
	return refs;
}
EXPORT_SYMBOL(cfs_percpt_atomic_alloc);

/** return sum of cpu-partition refs */
int
cfs_percpt_atomic_summary(atomic_t **refs)
{
	atomic_t	*ref;
	int		i;
	int		val = 0;

	cfs_percpt_for_each(ref, i, refs)
		val += atomic_read(ref);

	return val;
}
EXPORT_SYMBOL(cfs_percpt_atomic_summary);
