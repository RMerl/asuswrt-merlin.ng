/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 */
/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_UBERBLOCK_IMPL_H
#define	_SYS_UBERBLOCK_IMPL_H

#define UBMAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * The uberblock version is incremented whenever an incompatible on-disk
 * format change is made to the SPA, DMU, or ZAP.
 *
 * Note: the first two fields should never be moved.  When a storage pool
 * is opened, the uberblock must be read off the disk before the version
 * can be checked.	If the ub_version field is moved, we may not detect
 * version mismatch.  If the ub_magic field is moved, applications that
 * expect the magic number in the first word won't work.
 */
#define	UBERBLOCK_MAGIC		0x00bab10c		/* oo-ba-bloc!	*/
#define	UBERBLOCK_SHIFT		10			/* up to 1K	*/

typedef struct uberblock {
	uint64_t	ub_magic;	/* UBERBLOCK_MAGIC		*/
	uint64_t	ub_version;	/* ZFS_VERSION			*/
	uint64_t	ub_txg;		/* txg of last sync		*/
	uint64_t	ub_guid_sum;	/* sum of all vdev guids	*/
	uint64_t	ub_timestamp;	/* UTC time of last sync	*/
	blkptr_t	ub_rootbp;	/* MOS objset_phys_t		*/
} uberblock_t;

#define	VDEV_UBERBLOCK_SHIFT(as)	UBMAX(as, UBERBLOCK_SHIFT)
#define	UBERBLOCK_SIZE(as)		(1ULL << VDEV_UBERBLOCK_SHIFT(as))

/* Number of uberblocks that can fit in the ring at a given ashift */
#define UBERBLOCK_COUNT(as) (VDEV_UBERBLOCK_RING >> VDEV_UBERBLOCK_SHIFT(as))

#endif	/* _SYS_UBERBLOCK_IMPL_H */
