/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 */
/*
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_ZIO_CHECKSUM_H
#define	_SYS_ZIO_CHECKSUM_H

/*
 * Signature for checksum functions.
 */
typedef void zio_checksum_t(const void *data, uint64_t size,
			    zfs_endian_t endian, zio_cksum_t *zcp);

/*
 * Information about each checksum function.
 */
typedef struct zio_checksum_info {
	zio_checksum_t	*ci_func; /* checksum function for each byteorder */
	int		ci_correctable;	/* number of correctable bits	*/
	int		ci_eck;		/* uses zio embedded checksum? */
	char		*ci_name;	/* descriptive name */
} zio_checksum_info_t;

extern void zio_checksum_SHA256(const void *, uint64_t,
				 zfs_endian_t endian, zio_cksum_t *);
extern void fletcher_2_endian(const void *, uint64_t, zfs_endian_t endian,
			zio_cksum_t *);
extern void fletcher_4_endian(const void *, uint64_t, zfs_endian_t endian,
			zio_cksum_t *);

#endif	/* _SYS_ZIO_CHECKSUM_H */
