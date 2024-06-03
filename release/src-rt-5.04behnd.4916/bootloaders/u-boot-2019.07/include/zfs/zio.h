/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 */
/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 */

#ifndef _ZIO_H
#define	_ZIO_H

#include <zfs/spa.h>

#define	ZEC_MAGIC	0x210da7ab10c7a11ULL	/* zio data bloc tail */

typedef struct zio_eck {
	uint64_t	zec_magic;	/* for validation, endianness	*/
	zio_cksum_t	zec_cksum;	/* 256-bit checksum		*/
} zio_eck_t;

/*
 * Gang block headers are self-checksumming and contain an array
 * of block pointers.
 */
#define	SPA_GANGBLOCKSIZE	SPA_MINBLOCKSIZE
#define	SPA_GBH_NBLKPTRS	((SPA_GANGBLOCKSIZE - \
	sizeof(zio_eck_t)) / sizeof(blkptr_t))
#define	SPA_GBH_FILLER		((SPA_GANGBLOCKSIZE - \
	sizeof(zio_eck_t) - \
	(SPA_GBH_NBLKPTRS * sizeof(blkptr_t))) /\
	sizeof(uint64_t))

#define	ZIO_GET_IOSIZE(zio)	\
	(BP_IS_GANG((zio)->io_bp) ? \
	SPA_GANGBLOCKSIZE : BP_GET_PSIZE((zio)->io_bp))

typedef struct zio_gbh {
	blkptr_t		zg_blkptr[SPA_GBH_NBLKPTRS];
	uint64_t		zg_filler[SPA_GBH_FILLER];
	zio_eck_t		zg_tail;
} zio_gbh_phys_t;

enum zio_checksum {
	ZIO_CHECKSUM_INHERIT = 0,
	ZIO_CHECKSUM_ON,
	ZIO_CHECKSUM_OFF,
	ZIO_CHECKSUM_LABEL,
	ZIO_CHECKSUM_GANG_HEADER,
	ZIO_CHECKSUM_ZILOG,
	ZIO_CHECKSUM_FLETCHER_2,
	ZIO_CHECKSUM_FLETCHER_4,
	ZIO_CHECKSUM_SHA256,
	ZIO_CHECKSUM_ZILOG2,
	ZIO_CHECKSUM_FUNCTIONS
};

#define	ZIO_CHECKSUM_ON_VALUE	ZIO_CHECKSUM_FLETCHER_2
#define	ZIO_CHECKSUM_DEFAULT	ZIO_CHECKSUM_ON

enum zio_compress {
	ZIO_COMPRESS_INHERIT = 0,
	ZIO_COMPRESS_ON,
	ZIO_COMPRESS_OFF,
	ZIO_COMPRESS_LZJB,
	ZIO_COMPRESS_EMPTY,
	ZIO_COMPRESS_GZIP1,
	ZIO_COMPRESS_GZIP2,
	ZIO_COMPRESS_GZIP3,
	ZIO_COMPRESS_GZIP4,
	ZIO_COMPRESS_GZIP5,
	ZIO_COMPRESS_GZIP6,
	ZIO_COMPRESS_GZIP7,
	ZIO_COMPRESS_GZIP8,
	ZIO_COMPRESS_GZIP9,
	ZIO_COMPRESS_FUNCTIONS
};

#endif	/* _ZIO_H */
