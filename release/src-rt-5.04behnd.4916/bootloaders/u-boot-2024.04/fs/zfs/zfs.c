// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * ZFS filesystem ported to u-boot by
 * Jorgen Lundman <lundman at lundman.net>
 *
 *	GRUB  --  GRand Unified Bootloader
 *	Copyright (C) 1999,2000,2001,2002,2003,2004
 *	Free Software Foundation, Inc.
 *	Copyright 2004	Sun Microsystems, Inc.
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include "zfs_common.h"
#include "div64.h"

struct blk_desc *zfs_dev_desc;

/*
 * The zfs plug-in routines for GRUB are:
 *
 * zfs_mount() - locates a valid uberblock of the root pool and reads
 *		in its MOS at the memory address MOS.
 *
 * zfs_open() - locates a plain file object by following the MOS
 *		and places its dnode at the memory address DNODE.
 *
 * zfs_read() - read in the data blocks pointed by the DNODE.
 *
 */

#include <zfs/zfs.h>
#include <zfs/zio.h>
#include <zfs/dnode.h>
#include <zfs/uberblock_impl.h>
#include <zfs/vdev_impl.h>
#include <zfs/zio_checksum.h>
#include <zfs/zap_impl.h>
#include <zfs/zap_leaf.h>
#include <zfs/zfs_znode.h>
#include <zfs/dmu.h>
#include <zfs/dmu_objset.h>
#include <zfs/sa_impl.h>
#include <zfs/dsl_dir.h>
#include <zfs/dsl_dataset.h>


#define	ZPOOL_PROP_BOOTFS		"bootfs"


/*
 * For nvlist manipulation. (from nvpair.h)
 */
#define	NV_ENCODE_NATIVE	0
#define	NV_ENCODE_XDR		1
#define	NV_BIG_ENDIAN			0
#define	NV_LITTLE_ENDIAN	1
#define	DATA_TYPE_UINT64	8
#define	DATA_TYPE_STRING	9
#define	DATA_TYPE_NVLIST	19
#define	DATA_TYPE_NVLIST_ARRAY	20


/*
 * Macros to get fields in a bp or DVA.
 */
#define	P2PHASE(x, align)		((x) & ((align) - 1))
#define	DVA_OFFSET_TO_PHYS_SECTOR(offset)					\
	((offset + VDEV_LABEL_START_SIZE) >> SPA_MINBLOCKSHIFT)

/*
 * return x rounded down to an align boundary
 * eg, P2ALIGN(1200, 1024) == 1024 (1*align)
 * eg, P2ALIGN(1024, 1024) == 1024 (1*align)
 * eg, P2ALIGN(0x1234, 0x100) == 0x1200 (0x12*align)
 * eg, P2ALIGN(0x5600, 0x100) == 0x5600 (0x56*align)
 */
#define	P2ALIGN(x, align)		((x) & -(align))

/*
 * FAT ZAP data structures
 */
#define	ZFS_CRC64_POLY 0xC96C5795D7870F42ULL	/* ECMA-182, reflected form */
#define	ZAP_HASH_IDX(hash, n)	(((n) == 0) ? 0 : ((hash) >> (64 - (n))))
#define	CHAIN_END	0xffff	/* end of the chunk chain */

/*
 * The amount of space within the chunk available for the array is:
 * chunk size - space for type (1) - space for next pointer (2)
 */
#define	ZAP_LEAF_ARRAY_BYTES (ZAP_LEAF_CHUNKSIZE - 3)

#define	ZAP_LEAF_HASH_SHIFT(bs)	(bs - 5)
#define	ZAP_LEAF_HASH_NUMENTRIES(bs) (1 << ZAP_LEAF_HASH_SHIFT(bs))
#define	LEAF_HASH(bs, h)												\
	((ZAP_LEAF_HASH_NUMENTRIES(bs)-1) &									\
	 ((h) >> (64 - ZAP_LEAF_HASH_SHIFT(bs)-l->l_hdr.lh_prefix_len)))

/*
 * The amount of space available for chunks is:
 * block size shift - hash entry size (2) * number of hash
 * entries - header space (2*chunksize)
 */
#define	ZAP_LEAF_NUMCHUNKS(bs)						\
	(((1<<bs) - 2*ZAP_LEAF_HASH_NUMENTRIES(bs)) /	\
	 ZAP_LEAF_CHUNKSIZE - 2)

/*
 * The chunks start immediately after the hash table.  The end of the
 * hash table is at l_hash + HASH_NUMENTRIES, which we simply cast to a
 * chunk_t.
 */
#define	ZAP_LEAF_CHUNK(l, bs, idx)										\
	((zap_leaf_chunk_t *)(l->l_hash + ZAP_LEAF_HASH_NUMENTRIES(bs)))[idx]
#define	ZAP_LEAF_ENTRY(l, bs, idx) (&ZAP_LEAF_CHUNK(l, bs, idx).l_entry)


/*
 * Decompression Entry - lzjb
 */
#ifndef	NBBY
#define	NBBY	8
#endif



typedef int zfs_decomp_func_t(void *s_start, void *d_start,
							  uint32_t s_len, uint32_t d_len);
typedef struct decomp_entry {
	char *name;
	zfs_decomp_func_t *decomp_func;
} decomp_entry_t;

typedef struct dnode_end {
	dnode_phys_t dn;
	zfs_endian_t endian;
} dnode_end_t;

struct zfs_data {
	/* cache for a file block of the currently zfs_open()-ed file */
	char *file_buf;
	uint64_t file_start;
	uint64_t file_end;

	/* XXX: ashift is per vdev, not per pool.  We currently only ever touch
	 * a single vdev, but when/if raid-z or stripes are supported, this
	 * may need revision.
	 */
	uint64_t vdev_ashift;
	uint64_t label_txg;
	uint64_t pool_guid;

	/* cache for a dnode block */
	dnode_phys_t *dnode_buf;
	dnode_phys_t *dnode_mdn;
	uint64_t dnode_start;
	uint64_t dnode_end;
	zfs_endian_t dnode_endian;

	uberblock_t current_uberblock;

	dnode_end_t mos;
	dnode_end_t mdn;
	dnode_end_t dnode;

	uint64_t vdev_phys_sector;

	int (*userhook)(const char *, const struct zfs_dirhook_info *);
	struct zfs_dirhook_info *dirinfo;

};




static int
zlib_decompress(void *s, void *d,
				uint32_t slen, uint32_t dlen)
{
	if (zlib_decompress(s, d, slen, dlen) < 0)
		return ZFS_ERR_BAD_FS;
	return ZFS_ERR_NONE;
}

static decomp_entry_t decomp_table[ZIO_COMPRESS_FUNCTIONS] = {
	{"inherit", NULL},		/* ZIO_COMPRESS_INHERIT */
	{"on", lzjb_decompress},	/* ZIO_COMPRESS_ON */
	{"off", NULL},		/* ZIO_COMPRESS_OFF */
	{"lzjb", lzjb_decompress},	/* ZIO_COMPRESS_LZJB */
	{"empty", NULL},		/* ZIO_COMPRESS_EMPTY */
	{"gzip-1", zlib_decompress},  /* ZIO_COMPRESS_GZIP1 */
	{"gzip-2", zlib_decompress},  /* ZIO_COMPRESS_GZIP2 */
	{"gzip-3", zlib_decompress},  /* ZIO_COMPRESS_GZIP3 */
	{"gzip-4", zlib_decompress},  /* ZIO_COMPRESS_GZIP4 */
	{"gzip-5", zlib_decompress},  /* ZIO_COMPRESS_GZIP5 */
	{"gzip-6", zlib_decompress},  /* ZIO_COMPRESS_GZIP6 */
	{"gzip-7", zlib_decompress},  /* ZIO_COMPRESS_GZIP7 */
	{"gzip-8", zlib_decompress},  /* ZIO_COMPRESS_GZIP8 */
	{"gzip-9", zlib_decompress},  /* ZIO_COMPRESS_GZIP9 */
};



static int zio_read_data(blkptr_t *bp, zfs_endian_t endian,
						 void *buf, struct zfs_data *data);

static int
zio_read(blkptr_t *bp, zfs_endian_t endian, void **buf,
		 size_t *size, struct zfs_data *data);

/*
 * Our own version of log2().  Same thing as highbit()-1.
 */
static int
zfs_log2(uint64_t num)
{
	int i = 0;

	while (num > 1) {
		i++;
		num = num >> 1;
	}

	return i;
}


/* Checksum Functions */
static void
zio_checksum_off(const void *buf __attribute__ ((unused)),
				 uint64_t size __attribute__ ((unused)),
				 zfs_endian_t endian __attribute__ ((unused)),
				 zio_cksum_t *zcp)
{
	ZIO_SET_CHECKSUM(zcp, 0, 0, 0, 0);
}

/* Checksum Table and Values */
static zio_checksum_info_t zio_checksum_table[ZIO_CHECKSUM_FUNCTIONS] = {
	{NULL, 0, 0, "inherit"},
	{NULL, 0, 0, "on"},
	{zio_checksum_off, 0, 0, "off"},
	{zio_checksum_SHA256, 1, 1, "label"},
	{zio_checksum_SHA256, 1, 1, "gang_header"},
	{NULL, 0, 0, "zilog"},
	{fletcher_2_endian, 0, 0, "fletcher2"},
	{fletcher_4_endian, 1, 0, "fletcher4"},
	{zio_checksum_SHA256, 1, 0, "SHA256"},
	{NULL, 0, 0, "zilog2"},
};

/*
 * zio_checksum_verify: Provides support for checksum verification.
 *
 * Fletcher2, Fletcher4, and SHA256 are supported.
 *
 */
static int
zio_checksum_verify(zio_cksum_t zc, uint32_t checksum,
					zfs_endian_t endian, char *buf, int size)
{
	zio_eck_t *zec = (zio_eck_t *) (buf + size) - 1;
	zio_checksum_info_t *ci = &zio_checksum_table[checksum];
	zio_cksum_t actual_cksum, expected_cksum;

	if (checksum >= ZIO_CHECKSUM_FUNCTIONS || ci->ci_func == NULL) {
		printf("zfs unknown checksum function %d\n", checksum);
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}

	if (ci->ci_eck) {
		expected_cksum = zec->zec_cksum;
		zec->zec_cksum = zc;
		ci->ci_func(buf, size, endian, &actual_cksum);
		zec->zec_cksum = expected_cksum;
		zc = expected_cksum;
	} else {
		ci->ci_func(buf, size, endian, &actual_cksum);
	}

	if ((actual_cksum.zc_word[0] != zc.zc_word[0])
		|| (actual_cksum.zc_word[1] != zc.zc_word[1])
		|| (actual_cksum.zc_word[2] != zc.zc_word[2])
		|| (actual_cksum.zc_word[3] != zc.zc_word[3])) {
		return ZFS_ERR_BAD_FS;
	}

	return ZFS_ERR_NONE;
}

/*
 * vdev_uberblock_compare takes two uberblock structures and returns an integer
 * indicating the more recent of the two.
 *	Return Value = 1 if ub2 is more recent
 *	Return Value = -1 if ub1 is more recent
 * The most recent uberblock is determined using its transaction number and
 * timestamp.  The uberblock with the highest transaction number is
 * considered "newer".	If the transaction numbers of the two blocks match, the
 * timestamps are compared to determine the "newer" of the two.
 */
static int
vdev_uberblock_compare(uberblock_t *ub1, uberblock_t *ub2)
{
	zfs_endian_t ub1_endian, ub2_endian;
	if (zfs_to_cpu64(ub1->ub_magic, LITTLE_ENDIAN) == UBERBLOCK_MAGIC)
		ub1_endian = LITTLE_ENDIAN;
	else
		ub1_endian = BIG_ENDIAN;
	if (zfs_to_cpu64(ub2->ub_magic, LITTLE_ENDIAN) == UBERBLOCK_MAGIC)
		ub2_endian = LITTLE_ENDIAN;
	else
		ub2_endian = BIG_ENDIAN;

	if (zfs_to_cpu64(ub1->ub_txg, ub1_endian)
		< zfs_to_cpu64(ub2->ub_txg, ub2_endian))
		return -1;
	if (zfs_to_cpu64(ub1->ub_txg, ub1_endian)
		> zfs_to_cpu64(ub2->ub_txg, ub2_endian))
		return 1;

	if (zfs_to_cpu64(ub1->ub_timestamp, ub1_endian)
		< zfs_to_cpu64(ub2->ub_timestamp, ub2_endian))
		return -1;
	if (zfs_to_cpu64(ub1->ub_timestamp, ub1_endian)
		> zfs_to_cpu64(ub2->ub_timestamp, ub2_endian))
		return 1;

	return 0;
}

/*
 * Three pieces of information are needed to verify an uberblock: the magic
 * number, the version number, and the checksum.
 *
 * Currently Implemented: version number, magic number, label txg
 * Need to Implement: checksum
 *
 */
static int
uberblock_verify(uberblock_t *uber, int offset, struct zfs_data *data)
{
	int err;
	zfs_endian_t endian = UNKNOWN_ENDIAN;
	zio_cksum_t zc;

	if (uber->ub_txg < data->label_txg) {
		debug("ignoring partially written label: uber_txg < label_txg %llu %llu\n",
			  uber->ub_txg, data->label_txg);
		return ZFS_ERR_BAD_FS;
	}

	if (zfs_to_cpu64(uber->ub_magic, LITTLE_ENDIAN) == UBERBLOCK_MAGIC
		&& zfs_to_cpu64(uber->ub_version, LITTLE_ENDIAN) > 0
		&& zfs_to_cpu64(uber->ub_version, LITTLE_ENDIAN) <= SPA_VERSION)
		endian = LITTLE_ENDIAN;

	if (zfs_to_cpu64(uber->ub_magic, BIG_ENDIAN) == UBERBLOCK_MAGIC
		&& zfs_to_cpu64(uber->ub_version, BIG_ENDIAN) > 0
		&& zfs_to_cpu64(uber->ub_version, BIG_ENDIAN) <= SPA_VERSION)
		endian = BIG_ENDIAN;

	if (endian == UNKNOWN_ENDIAN) {
		printf("invalid uberblock magic\n");
		return ZFS_ERR_BAD_FS;
	}

	memset(&zc, 0, sizeof(zc));
	zc.zc_word[0] = cpu_to_zfs64(offset, endian);
	err = zio_checksum_verify(zc, ZIO_CHECKSUM_LABEL, endian,
							  (char *) uber, UBERBLOCK_SIZE(data->vdev_ashift));

	if (!err) {
		/* Check that the data pointed by the rootbp is usable. */
		void *osp = NULL;
		size_t ospsize;
		err = zio_read(&uber->ub_rootbp, endian, &osp, &ospsize, data);
		free(osp);

		if (!err && ospsize < OBJSET_PHYS_SIZE_V14) {
			printf("uberblock rootbp points to invalid data\n");
			return ZFS_ERR_BAD_FS;
		}
	}

	return err;
}

/*
 * Find the best uberblock.
 * Return:
 *	  Success - Pointer to the best uberblock.
 *	  Failure - NULL
 */
static uberblock_t *find_bestub(char *ub_array, struct zfs_data *data)
{
	const uint64_t sector = data->vdev_phys_sector;
	uberblock_t *ubbest = NULL;
	uberblock_t *ubnext;
	unsigned int i, offset, pickedub = 0;
	int err = ZFS_ERR_NONE;

	const unsigned int UBCOUNT = UBERBLOCK_COUNT(data->vdev_ashift);
	const uint64_t UBBYTES = UBERBLOCK_SIZE(data->vdev_ashift);

	for (i = 0; i < UBCOUNT; i++) {
		ubnext = (uberblock_t *) (i * UBBYTES + ub_array);
		offset = (sector << SPA_MINBLOCKSHIFT) + VDEV_PHYS_SIZE + (i * UBBYTES);

		err = uberblock_verify(ubnext, offset, data);
		if (err)
			continue;

		if (ubbest == NULL || vdev_uberblock_compare(ubnext, ubbest) > 0) {
			ubbest = ubnext;
			pickedub = i;
		}
	}

	if (ubbest)
		debug("zfs Found best uberblock at idx %d, txg %llu\n",
			  pickedub, (unsigned long long) ubbest->ub_txg);

	return ubbest;
}

static inline size_t
get_psize(blkptr_t *bp, zfs_endian_t endian)
{
	return (((zfs_to_cpu64((bp)->blk_prop, endian) >> 16) & 0xffff) + 1)
			<< SPA_MINBLOCKSHIFT;
}

static uint64_t
dva_get_offset(dva_t *dva, zfs_endian_t endian)
{
	return zfs_to_cpu64((dva)->dva_word[1],
							 endian) << SPA_MINBLOCKSHIFT;
}

/*
 * Read a block of data based on the gang block address dva,
 * and put its data in buf.
 *
 */
static int
zio_read_gang(blkptr_t *bp, zfs_endian_t endian, dva_t *dva, void *buf,
			  struct zfs_data *data)
{
	zio_gbh_phys_t *zio_gb;
	uint64_t offset, sector;
	unsigned i;
	int err;
	zio_cksum_t zc;

	memset(&zc, 0, sizeof(zc));

	zio_gb = malloc(SPA_GANGBLOCKSIZE);
	if (!zio_gb)
		return ZFS_ERR_OUT_OF_MEMORY;

	offset = dva_get_offset(dva, endian);
	sector = DVA_OFFSET_TO_PHYS_SECTOR(offset);

	/* read in the gang block header */
	err = zfs_devread(sector, 0, SPA_GANGBLOCKSIZE, (char *) zio_gb);

	if (err) {
		free(zio_gb);
		return err;
	}

	/* XXX */
	/* self checksuming the gang block header */
	ZIO_SET_CHECKSUM(&zc, DVA_GET_VDEV(dva),
					 dva_get_offset(dva, endian), bp->blk_birth, 0);
	err = zio_checksum_verify(zc, ZIO_CHECKSUM_GANG_HEADER, endian,
							  (char *) zio_gb, SPA_GANGBLOCKSIZE);
	if (err) {
		free(zio_gb);
		return err;
	}

	endian = (zfs_to_cpu64(bp->blk_prop, endian) >> 63) & 1;

	for (i = 0; i < SPA_GBH_NBLKPTRS; i++) {
		if (zio_gb->zg_blkptr[i].blk_birth == 0)
			continue;

		err = zio_read_data(&zio_gb->zg_blkptr[i], endian, buf, data);
		if (err) {
			free(zio_gb);
			return err;
		}
		buf = (char *) buf + get_psize(&zio_gb->zg_blkptr[i], endian);
	}
	free(zio_gb);
	return ZFS_ERR_NONE;
}

/*
 * Read in a block of raw data to buf.
 */
static int
zio_read_data(blkptr_t *bp, zfs_endian_t endian, void *buf,
			  struct zfs_data *data)
{
	int i, psize;
	int err = ZFS_ERR_NONE;

	psize = get_psize(bp, endian);

	/* pick a good dva from the block pointer */
	for (i = 0; i < SPA_DVAS_PER_BP; i++) {
		uint64_t offset, sector;

		if (bp->blk_dva[i].dva_word[0] == 0 && bp->blk_dva[i].dva_word[1] == 0)
			continue;

		if ((zfs_to_cpu64(bp->blk_dva[i].dva_word[1], endian)>>63) & 1) {
			err = zio_read_gang(bp, endian, &bp->blk_dva[i], buf, data);
		} else {
			/* read in a data block */
			offset = dva_get_offset(&bp->blk_dva[i], endian);
			sector = DVA_OFFSET_TO_PHYS_SECTOR(offset);

			err = zfs_devread(sector, 0, psize, buf);
		}

		if (!err) {
			/*Check the underlying checksum before we rule this DVA as "good"*/
			uint32_t checkalgo = (zfs_to_cpu64((bp)->blk_prop, endian) >> 40) & 0xff;

			err = zio_checksum_verify(bp->blk_cksum, checkalgo, endian, buf, psize);
			if (!err)
				return ZFS_ERR_NONE;
		}

		/* If read failed or checksum bad, reset the error.	 Hopefully we've got some more DVA's to try.*/
	}

	if (!err) {
		printf("couldn't find a valid DVA\n");
		err = ZFS_ERR_BAD_FS;
	}

	return err;
}

/*
 * Read in a block of data, verify its checksum, decompress if needed,
 * and put the uncompressed data in buf.
 */
static int
zio_read(blkptr_t *bp, zfs_endian_t endian, void **buf,
		 size_t *size, struct zfs_data *data)
{
	size_t lsize, psize;
	unsigned int comp;
	char *compbuf = NULL;
	int err;

	*buf = NULL;

	comp = (zfs_to_cpu64((bp)->blk_prop, endian)>>32) & 0xff;
	lsize = (BP_IS_HOLE(bp) ? 0 :
			 (((zfs_to_cpu64((bp)->blk_prop, endian) & 0xffff) + 1)
			  << SPA_MINBLOCKSHIFT));
	psize = get_psize(bp, endian);

	if (size)
		*size = lsize;

	if (comp >= ZIO_COMPRESS_FUNCTIONS) {
		printf("compression algorithm %u not supported\n", (unsigned int) comp);
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}

	if (comp != ZIO_COMPRESS_OFF && decomp_table[comp].decomp_func == NULL) {
		printf("compression algorithm %s not supported\n", decomp_table[comp].name);
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}

	if (comp != ZIO_COMPRESS_OFF) {
		compbuf = malloc(psize);
		if (!compbuf)
			return ZFS_ERR_OUT_OF_MEMORY;
	} else {
		compbuf = *buf = malloc(lsize);
	}

	err = zio_read_data(bp, endian, compbuf, data);
	if (err) {
		free(compbuf);
		*buf = NULL;
		return err;
	}

	if (comp != ZIO_COMPRESS_OFF) {
		*buf = malloc(lsize);
		if (!*buf) {
			free(compbuf);
			return ZFS_ERR_OUT_OF_MEMORY;
		}

		err = decomp_table[comp].decomp_func(compbuf, *buf, psize, lsize);
		free(compbuf);
		if (err) {
			free(*buf);
			*buf = NULL;
			return err;
		}
	}

	return ZFS_ERR_NONE;
}

/*
 * Get the block from a block id.
 * push the block onto the stack.
 *
 */
static int
dmu_read(dnode_end_t *dn, uint64_t blkid, void **buf,
		 zfs_endian_t *endian_out, struct zfs_data *data)
{
	int idx, level;
	blkptr_t *bp_array = dn->dn.dn_blkptr;
	int epbs = dn->dn.dn_indblkshift - SPA_BLKPTRSHIFT;
	blkptr_t *bp;
	void *tmpbuf = 0;
	zfs_endian_t endian;
	int err = ZFS_ERR_NONE;

	bp = malloc(sizeof(blkptr_t));
	if (!bp)
		return ZFS_ERR_OUT_OF_MEMORY;

	endian = dn->endian;
	for (level = dn->dn.dn_nlevels - 1; level >= 0; level--) {
		idx = (blkid >> (epbs * level)) & ((1 << epbs) - 1);
		*bp = bp_array[idx];
		if (bp_array != dn->dn.dn_blkptr) {
			free(bp_array);
			bp_array = 0;
		}

		if (BP_IS_HOLE(bp)) {
			size_t size = zfs_to_cpu16(dn->dn.dn_datablkszsec,
											dn->endian)
				<< SPA_MINBLOCKSHIFT;
			*buf = malloc(size);
			if (*buf) {
				err = ZFS_ERR_OUT_OF_MEMORY;
				break;
			}
			memset(*buf, 0, size);
			endian = (zfs_to_cpu64(bp->blk_prop, endian) >> 63) & 1;
			break;
		}
		if (level == 0) {
			err = zio_read(bp, endian, buf, 0, data);
			endian = (zfs_to_cpu64(bp->blk_prop, endian) >> 63) & 1;
			break;
		}
		err = zio_read(bp, endian, &tmpbuf, 0, data);
		endian = (zfs_to_cpu64(bp->blk_prop, endian) >> 63) & 1;
		if (err)
			break;
		bp_array = tmpbuf;
	}
	if (bp_array != dn->dn.dn_blkptr)
		free(bp_array);
	if (endian_out)
		*endian_out = endian;

	free(bp);
	return err;
}

/*
 * mzap_lookup: Looks up property described by "name" and returns the value
 * in "value".
 */
static int
mzap_lookup(mzap_phys_t *zapobj, zfs_endian_t endian,
			int objsize, char *name, uint64_t * value)
{
	int i, chunks;
	mzap_ent_phys_t *mzap_ent = zapobj->mz_chunk;

	chunks = objsize / MZAP_ENT_LEN - 1;
	for (i = 0; i < chunks; i++) {
		if (strcmp(mzap_ent[i].mze_name, name) == 0) {
			*value = zfs_to_cpu64(mzap_ent[i].mze_value, endian);
			return ZFS_ERR_NONE;
		}
	}

	printf("couldn't find '%s'\n", name);
	return ZFS_ERR_FILE_NOT_FOUND;
}

static int
mzap_iterate(mzap_phys_t *zapobj, zfs_endian_t endian, int objsize,
			 int (*hook)(const char *name,
						 uint64_t val,
						 struct zfs_data *data),
			 struct zfs_data *data)
{
	int i, chunks;
	mzap_ent_phys_t *mzap_ent = zapobj->mz_chunk;

	chunks = objsize / MZAP_ENT_LEN - 1;
	for (i = 0; i < chunks; i++) {
		if (hook(mzap_ent[i].mze_name,
				 zfs_to_cpu64(mzap_ent[i].mze_value, endian),
				 data))
			return 1;
	}

	return 0;
}

static uint64_t
zap_hash(uint64_t salt, const char *name)
{
	static uint64_t table[256];
	const uint8_t *cp;
	uint8_t c;
	uint64_t crc = salt;

	if (table[128] == 0) {
		uint64_t *ct = NULL;
		int i, j;
		for (i = 0; i < 256; i++) {
			for (ct = table + i, *ct = i, j = 8; j > 0; j--)
				*ct = (*ct >> 1) ^ (-(*ct & 1) & ZFS_CRC64_POLY);
		}
	}

	for (cp = (const uint8_t *) name; (c = *cp) != '\0'; cp++)
		crc = (crc >> 8) ^ table[(crc ^ c) & 0xFF];

	/*
	 * Only use 28 bits, since we need 4 bits in the cookie for the
	 * collision differentiator.  We MUST use the high bits, since
	 * those are the onces that we first pay attention to when
	 * chosing the bucket.
	 */
	crc &= ~((1ULL << (64 - ZAP_HASHBITS)) - 1);

	return crc;
}

/*
 * Only to be used on 8-bit arrays.
 * array_len is actual len in bytes (not encoded le_value_length).
 * buf is null-terminated.
 */
/* XXX */
static int
zap_leaf_array_equal(zap_leaf_phys_t *l, zfs_endian_t endian,
					 int blksft, int chunk, int array_len, const char *buf)
{
	int bseen = 0;

	while (bseen < array_len) {
		struct zap_leaf_array *la = &ZAP_LEAF_CHUNK(l, blksft, chunk).l_array;
		int toread = min(array_len - bseen, ZAP_LEAF_ARRAY_BYTES);

		if (chunk >= ZAP_LEAF_NUMCHUNKS(blksft))
			return 0;

		if (memcmp(la->la_array, buf + bseen, toread) != 0)
			break;
		chunk = zfs_to_cpu16(la->la_next, endian);
		bseen += toread;
	}
	return (bseen == array_len);
}

/* XXX */
static int
zap_leaf_array_get(zap_leaf_phys_t *l, zfs_endian_t endian, int blksft,
				   int chunk, int array_len, char *buf)
{
	int bseen = 0;

	while (bseen < array_len) {
		struct zap_leaf_array *la = &ZAP_LEAF_CHUNK(l, blksft, chunk).l_array;
		int toread = min(array_len - bseen, ZAP_LEAF_ARRAY_BYTES);

		if (chunk >= ZAP_LEAF_NUMCHUNKS(blksft))
			/* Don't use errno because this error is to be ignored.  */
			return ZFS_ERR_BAD_FS;

		memcpy(buf + bseen, la->la_array,  toread);
		chunk = zfs_to_cpu16(la->la_next, endian);
		bseen += toread;
	}
	return ZFS_ERR_NONE;
}


/*
 * Given a zap_leaf_phys_t, walk thru the zap leaf chunks to get the
 * value for the property "name".
 *
 */
/* XXX */
static int
zap_leaf_lookup(zap_leaf_phys_t *l, zfs_endian_t endian,
				int blksft, uint64_t h,
				const char *name, uint64_t *value)
{
	uint16_t chunk;
	struct zap_leaf_entry *le;

	/* Verify if this is a valid leaf block */
	if (zfs_to_cpu64(l->l_hdr.lh_block_type, endian) != ZBT_LEAF) {
		printf("invalid leaf type\n");
		return ZFS_ERR_BAD_FS;
	}
	if (zfs_to_cpu32(l->l_hdr.lh_magic, endian) != ZAP_LEAF_MAGIC) {
		printf("invalid leaf magic\n");
		return ZFS_ERR_BAD_FS;
	}

	for (chunk = zfs_to_cpu16(l->l_hash[LEAF_HASH(blksft, h)], endian);
		 chunk != CHAIN_END; chunk = le->le_next) {

		if (chunk >= ZAP_LEAF_NUMCHUNKS(blksft)) {
			printf("invalid chunk number\n");
			return ZFS_ERR_BAD_FS;
		}

		le = ZAP_LEAF_ENTRY(l, blksft, chunk);

		/* Verify the chunk entry */
		if (le->le_type != ZAP_CHUNK_ENTRY) {
			printf("invalid chunk entry\n");
			return ZFS_ERR_BAD_FS;
		}

		if (zfs_to_cpu64(le->le_hash, endian) != h)
			continue;

		if (zap_leaf_array_equal(l, endian, blksft,
								 zfs_to_cpu16(le->le_name_chunk, endian),
								 zfs_to_cpu16(le->le_name_length, endian),
								 name)) {
			struct zap_leaf_array *la;

			if (le->le_int_size != 8 || le->le_value_length != 1) {
				printf("invalid leaf chunk entry\n");
				return ZFS_ERR_BAD_FS;
			}
			/* get the uint64_t property value */
			la = &ZAP_LEAF_CHUNK(l, blksft, le->le_value_chunk).l_array;

			*value = be64_to_cpu(la->la_array64);

			return ZFS_ERR_NONE;
		}
	}

	printf("couldn't find '%s'\n", name);
	return ZFS_ERR_FILE_NOT_FOUND;
}


/* Verify if this is a fat zap header block */
static int
zap_verify(zap_phys_t *zap)
{
	if (zap->zap_magic != (uint64_t) ZAP_MAGIC) {
		printf("bad ZAP magic\n");
		return ZFS_ERR_BAD_FS;
	}

	if (zap->zap_flags != 0) {
		printf("bad ZAP flags\n");
		return ZFS_ERR_BAD_FS;
	}

	if (zap->zap_salt == 0) {
		printf("bad ZAP salt\n");
		return ZFS_ERR_BAD_FS;
	}

	return ZFS_ERR_NONE;
}

/*
 * Fat ZAP lookup
 *
 */
/* XXX */
static int
fzap_lookup(dnode_end_t *zap_dnode, zap_phys_t *zap,
			char *name, uint64_t *value, struct zfs_data *data)
{
	void *l;
	uint64_t hash, idx, blkid;
	int blksft = zfs_log2(zfs_to_cpu16(zap_dnode->dn.dn_datablkszsec,
											zap_dnode->endian) << DNODE_SHIFT);
	int err;
	zfs_endian_t leafendian;

	err = zap_verify(zap);
	if (err)
		return err;

	hash = zap_hash(zap->zap_salt, name);

	/* get block id from index */
	if (zap->zap_ptrtbl.zt_numblks != 0) {
		printf("external pointer tables not supported\n");
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}
	idx = ZAP_HASH_IDX(hash, zap->zap_ptrtbl.zt_shift);
	blkid = ((uint64_t *) zap)[idx + (1 << (blksft - 3 - 1))];

	/* Get the leaf block */
	if ((1U << blksft) < sizeof(zap_leaf_phys_t)) {
		printf("ZAP leaf is too small\n");
		return ZFS_ERR_BAD_FS;
	}
	err = dmu_read(zap_dnode, blkid, &l, &leafendian, data);
	if (err)
		return err;

	err = zap_leaf_lookup(l, leafendian, blksft, hash, name, value);
	free(l);
	return err;
}

/* XXX */
static int
fzap_iterate(dnode_end_t *zap_dnode, zap_phys_t *zap,
			 int (*hook)(const char *name,
						 uint64_t val,
						 struct zfs_data *data),
			 struct zfs_data *data)
{
	zap_leaf_phys_t *l;
	void *l_in;
	uint64_t idx, blkid;
	uint16_t chunk;
	int blksft = zfs_log2(zfs_to_cpu16(zap_dnode->dn.dn_datablkszsec,
											zap_dnode->endian) << DNODE_SHIFT);
	int err;
	zfs_endian_t endian;

	if (zap_verify(zap))
		return 0;

	/* get block id from index */
	if (zap->zap_ptrtbl.zt_numblks != 0) {
		printf("external pointer tables not supported\n");
		return 0;
	}
	/* Get the leaf block */
	if ((1U << blksft) < sizeof(zap_leaf_phys_t)) {
		printf("ZAP leaf is too small\n");
		return 0;
	}
	for (idx = 0; idx < zap->zap_ptrtbl.zt_numblks; idx++) {
		blkid = ((uint64_t *) zap)[idx + (1 << (blksft - 3 - 1))];

		err = dmu_read(zap_dnode, blkid, &l_in, &endian, data);
		l = l_in;
		if (err)
			continue;

		/* Verify if this is a valid leaf block */
		if (zfs_to_cpu64(l->l_hdr.lh_block_type, endian) != ZBT_LEAF) {
			free(l);
			continue;
		}
		if (zfs_to_cpu32(l->l_hdr.lh_magic, endian) != ZAP_LEAF_MAGIC) {
			free(l);
			continue;
		}

		for (chunk = 0; chunk < ZAP_LEAF_NUMCHUNKS(blksft); chunk++) {
			char *buf;
			struct zap_leaf_array *la;
			struct zap_leaf_entry *le;
			uint64_t val;
			le = ZAP_LEAF_ENTRY(l, blksft, chunk);

			/* Verify the chunk entry */
			if (le->le_type != ZAP_CHUNK_ENTRY)
				continue;

			buf = malloc(zfs_to_cpu16(le->le_name_length, endian)
						 + 1);
			if (zap_leaf_array_get(l, endian, blksft, le->le_name_chunk,
								   le->le_name_length, buf)) {
				free(buf);
				continue;
			}
			buf[le->le_name_length] = 0;

			if (le->le_int_size != 8
				|| zfs_to_cpu16(le->le_value_length, endian) != 1)
				continue;

			/* get the uint64_t property value */
			la = &ZAP_LEAF_CHUNK(l, blksft, le->le_value_chunk).l_array;
			val = be64_to_cpu(la->la_array64);
			if (hook(buf, val, data))
				return 1;
			free(buf);
		}
	}
	return 0;
}


/*
 * Read in the data of a zap object and find the value for a matching
 * property name.
 *
 */
static int
zap_lookup(dnode_end_t *zap_dnode, char *name, uint64_t *val,
		   struct zfs_data *data)
{
	uint64_t block_type;
	int size;
	void *zapbuf;
	int err;
	zfs_endian_t endian;

	/* Read in the first block of the zap object data. */
	size = zfs_to_cpu16(zap_dnode->dn.dn_datablkszsec,
							 zap_dnode->endian) << SPA_MINBLOCKSHIFT;
	err = dmu_read(zap_dnode, 0, &zapbuf, &endian, data);
	if (err)
		return err;
	block_type = zfs_to_cpu64(*((uint64_t *) zapbuf), endian);

	if (block_type == ZBT_MICRO) {
		err = (mzap_lookup(zapbuf, endian, size, name, val));
		free(zapbuf);
		return err;
	} else if (block_type == ZBT_HEADER) {
		/* this is a fat zap */
		err = (fzap_lookup(zap_dnode, zapbuf, name, val, data));
		free(zapbuf);
		return err;
	}

	printf("unknown ZAP type\n");
	free(zapbuf);
	return ZFS_ERR_BAD_FS;
}

static int
zap_iterate(dnode_end_t *zap_dnode,
			int (*hook)(const char *name, uint64_t val,
						struct zfs_data *data),
			struct zfs_data *data)
{
	uint64_t block_type;
	int size;
	void *zapbuf;
	int err;
	int ret;
	zfs_endian_t endian;

	/* Read in the first block of the zap object data. */
	size = zfs_to_cpu16(zap_dnode->dn.dn_datablkszsec, zap_dnode->endian) << SPA_MINBLOCKSHIFT;
	err = dmu_read(zap_dnode, 0, &zapbuf, &endian, data);
	if (err)
		return 0;
	block_type = zfs_to_cpu64(*((uint64_t *) zapbuf), endian);

	if (block_type == ZBT_MICRO) {
		ret = mzap_iterate(zapbuf, endian, size, hook, data);
		free(zapbuf);
		return ret;
	} else if (block_type == ZBT_HEADER) {
		/* this is a fat zap */
		ret = fzap_iterate(zap_dnode, zapbuf, hook, data);
		free(zapbuf);
		return ret;
	}
	printf("unknown ZAP type\n");
	free(zapbuf);
	return 0;
}


/*
 * Get the dnode of an object number from the metadnode of an object set.
 *
 * Input
 *	mdn - metadnode to get the object dnode
 *	objnum - object number for the object dnode
 *	buf - data buffer that holds the returning dnode
 */
static int
dnode_get(dnode_end_t *mdn, uint64_t objnum, uint8_t type,
		  dnode_end_t *buf, struct zfs_data *data)
{
	uint64_t blkid, blksz;	/* the block id this object dnode is in */
	int epbs;			/* shift of number of dnodes in a block */
	int idx;			/* index within a block */
	void *dnbuf;
	int err;
	zfs_endian_t endian;

	blksz = zfs_to_cpu16(mdn->dn.dn_datablkszsec,
							  mdn->endian) << SPA_MINBLOCKSHIFT;

	epbs = zfs_log2(blksz) - DNODE_SHIFT;
	blkid = objnum >> epbs;
	idx = objnum & ((1 << epbs) - 1);

	if (data->dnode_buf != NULL && memcmp(data->dnode_mdn, mdn,
										  sizeof(*mdn)) == 0
		&& objnum >= data->dnode_start && objnum < data->dnode_end) {
		memmove(&(buf->dn), &(data->dnode_buf)[idx], DNODE_SIZE);
		buf->endian = data->dnode_endian;
		if (type && buf->dn.dn_type != type)  {
			printf("incorrect dnode type: %02X != %02x\n", buf->dn.dn_type, type);
			return ZFS_ERR_BAD_FS;
		}
		return ZFS_ERR_NONE;
	}

	err = dmu_read(mdn, blkid, &dnbuf, &endian, data);
	if (err)
		return err;

	free(data->dnode_buf);
	free(data->dnode_mdn);
	data->dnode_mdn = malloc(sizeof(*mdn));
	if (!data->dnode_mdn) {
		data->dnode_buf = 0;
	} else {
		memcpy(data->dnode_mdn, mdn, sizeof(*mdn));
		data->dnode_buf = dnbuf;
		data->dnode_start = blkid << epbs;
		data->dnode_end = (blkid + 1) << epbs;
		data->dnode_endian = endian;
	}

	memmove(&(buf->dn), (dnode_phys_t *) dnbuf + idx, DNODE_SIZE);
	buf->endian = endian;
	if (type && buf->dn.dn_type != type) {
		printf("incorrect dnode type\n");
		return ZFS_ERR_BAD_FS;
	}

	return ZFS_ERR_NONE;
}

/*
 * Get the file dnode for a given file name where mdn is the meta dnode
 * for this ZFS object set. When found, place the file dnode in dn.
 * The 'path' argument will be mangled.
 *
 */
static int
dnode_get_path(dnode_end_t *mdn, const char *path_in, dnode_end_t *dn,
			   struct zfs_data *data)
{
	uint64_t objnum, version;
	char *cname, ch;
	int err = ZFS_ERR_NONE;
	char *path, *path_buf;
	struct dnode_chain {
		struct dnode_chain *next;
		dnode_end_t dn;
	};
	struct dnode_chain *dnode_path = 0, *dn_new, *root;

	dn_new = malloc(sizeof(*dn_new));
	if (!dn_new)
		return ZFS_ERR_OUT_OF_MEMORY;
	dn_new->next = 0;
	dnode_path = root = dn_new;

	err = dnode_get(mdn, MASTER_NODE_OBJ, DMU_OT_MASTER_NODE,
					&(dnode_path->dn), data);
	if (err) {
		free(dn_new);
		return err;
	}

	err = zap_lookup(&(dnode_path->dn), ZPL_VERSION_STR, &version, data);
	if (err) {
		free(dn_new);
		return err;
	}
	if (version > ZPL_VERSION) {
		free(dn_new);
		printf("too new ZPL version\n");
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}

	err = zap_lookup(&(dnode_path->dn), ZFS_ROOT_OBJ, &objnum, data);
	if (err) {
		free(dn_new);
		return err;
	}

	err = dnode_get(mdn, objnum, 0, &(dnode_path->dn), data);
	if (err) {
		free(dn_new);
		return err;
	}

	path = path_buf = strdup(path_in);
	if (!path_buf) {
		free(dn_new);
		return ZFS_ERR_OUT_OF_MEMORY;
	}

	while (1) {
		/* skip leading slashes */
		while (*path == '/')
			path++;
		if (!*path)
			break;
		/* get the next component name */
		cname = path;
		while (*path && *path != '/')
			path++;
		/* Skip dot.  */
		if (cname + 1 == path && cname[0] == '.')
			continue;
		/* Handle double dot.  */
		if (cname + 2 == path && cname[0] == '.' && cname[1] == '.')  {
			if (dn_new->next) {
				dn_new = dnode_path;
				dnode_path = dn_new->next;
				free(dn_new);
			} else {
				printf("can't resolve ..\n");
				err = ZFS_ERR_FILE_NOT_FOUND;
				break;
			}
			continue;
		}

		ch = *path;
		*path = 0;		/* ensure null termination */

		if (dnode_path->dn.dn.dn_type != DMU_OT_DIRECTORY_CONTENTS) {
			free(path_buf);
			printf("not a directory\n");
			return ZFS_ERR_BAD_FILE_TYPE;
		}
		err = zap_lookup(&(dnode_path->dn), cname, &objnum, data);
		if (err)
			break;

		dn_new = malloc(sizeof(*dn_new));
		if (!dn_new) {
			err = ZFS_ERR_OUT_OF_MEMORY;
			break;
		}
		dn_new->next = dnode_path;
		dnode_path = dn_new;

		objnum = ZFS_DIRENT_OBJ(objnum);
		err = dnode_get(mdn, objnum, 0, &(dnode_path->dn), data);
		if (err)
			break;

		*path = ch;
	}

	if (!err)
		memcpy(dn, &(dnode_path->dn), sizeof(*dn));

	while (dnode_path) {
		dn_new = dnode_path->next;
		free(dnode_path);
		dnode_path = dn_new;
	}
	free(path_buf);
	return err;
}


/*
 * Given a MOS metadnode, get the metadnode of a given filesystem name (fsname),
 * e.g. pool/rootfs, or a given object number (obj), e.g. the object number
 * of pool/rootfs.
 *
 * If no fsname and no obj are given, return the DSL_DIR metadnode.
 * If fsname is given, return its metadnode and its matching object number.
 * If only obj is given, return the metadnode for this object number.
 *
 */
static int
get_filesystem_dnode(dnode_end_t *mosmdn, char *fsname,
					 dnode_end_t *mdn, struct zfs_data *data)
{
	uint64_t objnum;
	int err;

	err = dnode_get(mosmdn, DMU_POOL_DIRECTORY_OBJECT,
					DMU_OT_OBJECT_DIRECTORY, mdn, data);
	if (err)
		return err;

	err = zap_lookup(mdn, DMU_POOL_ROOT_DATASET, &objnum, data);
	if (err)
		return err;

	err = dnode_get(mosmdn, objnum, DMU_OT_DSL_DIR, mdn, data);
	if (err)
		return err;

	while (*fsname) {
		uint64_t childobj;
		char *cname, ch;

		while (*fsname == '/')
			fsname++;

		if (!*fsname || *fsname == '@')
			break;

		cname = fsname;
		while (*fsname && !isspace(*fsname) && *fsname != '/')
			fsname++;
		ch = *fsname;
		*fsname = 0;

		childobj = zfs_to_cpu64((((dsl_dir_phys_t *) DN_BONUS(&mdn->dn)))->dd_child_dir_zapobj, mdn->endian);
		err = dnode_get(mosmdn, childobj,
						DMU_OT_DSL_DIR_CHILD_MAP, mdn, data);
		if (err)
			return err;

		err = zap_lookup(mdn, cname, &objnum, data);
		if (err)
			return err;

		err = dnode_get(mosmdn, objnum, DMU_OT_DSL_DIR, mdn, data);
		if (err)
			return err;

		*fsname = ch;
	}
	return ZFS_ERR_NONE;
}

static int
make_mdn(dnode_end_t *mdn, struct zfs_data *data)
{
	void *osp;
	blkptr_t *bp;
	size_t ospsize;
	int err;

	bp = &(((dsl_dataset_phys_t *) DN_BONUS(&mdn->dn))->ds_bp);
	err = zio_read(bp, mdn->endian, &osp, &ospsize, data);
	if (err)
		return err;
	if (ospsize < OBJSET_PHYS_SIZE_V14) {
		free(osp);
		printf("too small osp\n");
		return ZFS_ERR_BAD_FS;
	}

	mdn->endian = (zfs_to_cpu64(bp->blk_prop, mdn->endian)>>63) & 1;
	memmove((char *) &(mdn->dn),
			(char *) &((objset_phys_t *) osp)->os_meta_dnode, DNODE_SIZE);
	free(osp);
	return ZFS_ERR_NONE;
}

static int
dnode_get_fullpath(const char *fullpath, dnode_end_t *mdn,
				   uint64_t *mdnobj, dnode_end_t *dn, int *isfs,
				   struct zfs_data *data)
{
	char *fsname, *snapname;
	const char *ptr_at, *filename;
	uint64_t headobj;
	int err;

	ptr_at = strchr(fullpath, '@');
	if (!ptr_at) {
		*isfs = 1;
		filename = 0;
		snapname = 0;
		fsname = strdup(fullpath);
	} else {
		const char *ptr_slash = strchr(ptr_at, '/');

		*isfs = 0;
		fsname = malloc(ptr_at - fullpath + 1);
		if (!fsname)
			return ZFS_ERR_OUT_OF_MEMORY;
		memcpy(fsname, fullpath, ptr_at - fullpath);
		fsname[ptr_at - fullpath] = 0;
		if (ptr_at[1] && ptr_at[1] != '/') {
			snapname = malloc(ptr_slash - ptr_at);
			if (!snapname) {
				free(fsname);
				return ZFS_ERR_OUT_OF_MEMORY;
			}
			memcpy(snapname, ptr_at + 1, ptr_slash - ptr_at - 1);
			snapname[ptr_slash - ptr_at - 1] = 0;
		} else {
			snapname = 0;
		}
		if (ptr_slash)
			filename = ptr_slash;
		else
			filename = "/";
		printf("zfs fsname = '%s' snapname='%s' filename = '%s'\n",
			   fsname, snapname, filename);
	}


	err = get_filesystem_dnode(&(data->mos), fsname, dn, data);

	if (err) {
		free(fsname);
		free(snapname);
		return err;
	}

	headobj = zfs_to_cpu64(((dsl_dir_phys_t *) DN_BONUS(&dn->dn))->dd_head_dataset_obj, dn->endian);

	err = dnode_get(&(data->mos), headobj, DMU_OT_DSL_DATASET, mdn, data);
	if (err) {
		free(fsname);
		free(snapname);
		return err;
	}

	if (snapname) {
		uint64_t snapobj;

		snapobj = zfs_to_cpu64(((dsl_dataset_phys_t *) DN_BONUS(&mdn->dn))->ds_snapnames_zapobj, mdn->endian);

		err = dnode_get(&(data->mos), snapobj,
						DMU_OT_DSL_DS_SNAP_MAP, mdn, data);
		if (!err)
			err = zap_lookup(mdn, snapname, &headobj, data);
		if (!err)
			err = dnode_get(&(data->mos), headobj, DMU_OT_DSL_DATASET, mdn, data);
		if (err) {
			free(fsname);
			free(snapname);
			return err;
		}
	}

	if (mdnobj)
		*mdnobj = headobj;

	make_mdn(mdn, data);

	if (*isfs) {
		free(fsname);
		free(snapname);
		return ZFS_ERR_NONE;
	}
	err = dnode_get_path(mdn, filename, dn, data);
	free(fsname);
	free(snapname);
	return err;
}

/*
 * For a given XDR packed nvlist, verify the first 4 bytes and move on.
 *
 * An XDR packed nvlist is encoded as (comments from nvs_xdr_create) :
 *
 *		encoding method/host endian		(4 bytes)
 *		nvl_version						(4 bytes)
 *		nvl_nvflag						(4 bytes)
 *	encoded nvpairs:
 *		encoded size of the nvpair		(4 bytes)
 *		decoded size of the nvpair		(4 bytes)
 *		name string size				(4 bytes)
 *		name string data				(sizeof(NV_ALIGN4(string))
 *		data type						(4 bytes)
 *		# of elements in the nvpair		(4 bytes)
 *		data
 *		2 zero's for the last nvpair
 *		(end of the entire list)	(8 bytes)
 *
 */

static int
nvlist_find_value(char *nvlist, char *name, int valtype, char **val,
				  size_t *size_out, size_t *nelm_out)
{
	int name_len, type, encode_size;
	char *nvpair, *nvp_name;

	/* Verify if the 1st and 2nd byte in the nvlist are valid. */
	/* NOTE: independently of what endianness header announces all
	   subsequent values are big-endian.  */
	if (nvlist[0] != NV_ENCODE_XDR || (nvlist[1] != NV_LITTLE_ENDIAN
									   && nvlist[1] != NV_BIG_ENDIAN)) {
		printf("zfs incorrect nvlist header\n");
		return ZFS_ERR_BAD_FS;
	}

	/* skip the header, nvl_version, and nvl_nvflag */
	nvlist = nvlist + 4 * 3;
	/*
	 * Loop thru the nvpair list
	 * The XDR representation of an integer is in big-endian byte order.
	 */
	while ((encode_size = be32_to_cpu(*(uint32_t *) nvlist))) {
		int nelm;

		nvpair = nvlist + 4 * 2;	/* skip the encode/decode size */

		name_len = be32_to_cpu(*(uint32_t *) nvpair);
		nvpair += 4;

		nvp_name = nvpair;
		nvpair = nvpair + ((name_len + 3) & ~3);	/* align */

		type = be32_to_cpu(*(uint32_t *) nvpair);
		nvpair += 4;

		nelm = be32_to_cpu(*(uint32_t *) nvpair);
		if (nelm < 1) {
			printf("empty nvpair\n");
			return ZFS_ERR_BAD_FS;
		}

		nvpair += 4;

		if ((strncmp(nvp_name, name, name_len) == 0) && type == valtype) {
			*val = nvpair;
			*size_out = encode_size;
			if (nelm_out)
				*nelm_out = nelm;
			return 1;
		}

		nvlist += encode_size;	/* goto the next nvpair */
	}
	return 0;
}

int
zfs_nvlist_lookup_uint64(char *nvlist, char *name, uint64_t *out)
{
	char *nvpair;
	size_t size;
	int found;

	found = nvlist_find_value(nvlist, name, DATA_TYPE_UINT64, &nvpair, &size, 0);
	if (!found)
		return 0;
	if (size < sizeof(uint64_t)) {
		printf("invalid uint64\n");
		return ZFS_ERR_BAD_FS;
	}

	*out = be64_to_cpu(*(uint64_t *) nvpair);
	return 1;
}

char *
zfs_nvlist_lookup_string(char *nvlist, char *name)
{
	char *nvpair;
	char *ret;
	size_t slen;
	size_t size;
	int found;

	found = nvlist_find_value(nvlist, name, DATA_TYPE_STRING, &nvpair, &size, 0);
	if (!found)
		return 0;
	if (size < 4) {
		printf("invalid string\n");
		return 0;
	}
	slen = be32_to_cpu(*(uint32_t *) nvpair);
	if (slen > size - 4)
		slen = size - 4;
	ret = malloc(slen + 1);
	if (!ret)
		return 0;
	memcpy(ret, nvpair + 4, slen);
	ret[slen] = 0;
	return ret;
}

char *
zfs_nvlist_lookup_nvlist(char *nvlist, char *name)
{
	char *nvpair;
	char *ret;
	size_t size;
	int found;

	found = nvlist_find_value(nvlist, name, DATA_TYPE_NVLIST, &nvpair,
							  &size, 0);
	if (!found)
		return 0;
	ret = calloc(1, size + 3 * sizeof(uint32_t));
	if (!ret)
		return 0;
	memcpy(ret, nvlist, sizeof(uint32_t));

	memcpy(ret + sizeof(uint32_t), nvpair, size);
	return ret;
}

int
zfs_nvlist_lookup_nvlist_array_get_nelm(char *nvlist, char *name)
{
	char *nvpair;
	size_t nelm, size;
	int found;

	found = nvlist_find_value(nvlist, name, DATA_TYPE_NVLIST, &nvpair,
							  &size, &nelm);
	if (!found)
		return -1;
	return nelm;
}

char *
zfs_nvlist_lookup_nvlist_array(char *nvlist, char *name,
									size_t index)
{
	char *nvpair, *nvpairptr;
	int found;
	char *ret;
	size_t size;
	unsigned i;
	size_t nelm;

	found = nvlist_find_value(nvlist, name, DATA_TYPE_NVLIST, &nvpair,
							  &size, &nelm);
	if (!found)
		return 0;
	if (index >= nelm) {
		printf("trying to lookup past nvlist array\n");
		return 0;
	}

	nvpairptr = nvpair;

	for (i = 0; i < index; i++) {
		uint32_t encode_size;

		/* skip the header, nvl_version, and nvl_nvflag */
		nvpairptr = nvpairptr + 4 * 2;

		while (nvpairptr < nvpair + size
			   && (encode_size = be32_to_cpu(*(uint32_t *) nvpairptr)))
			nvlist += encode_size;	/* goto the next nvpair */

		nvlist = nvlist + 4 * 2;	/* skip the ending 2 zeros - 8 bytes */
	}

	if (nvpairptr >= nvpair + size
		|| nvpairptr + be32_to_cpu(*(uint32_t *) (nvpairptr + 4 * 2))
		>= nvpair + size) {
		printf("incorrect nvlist array\n");
		return 0;
	}

	ret = calloc(1, be32_to_cpu(*(uint32_t *) (nvpairptr + 4 * 2))
				 + 3 * sizeof(uint32_t));
	if (!ret)
		return 0;
	memcpy(ret, nvlist, sizeof(uint32_t));

	memcpy(ret + sizeof(uint32_t), nvpairptr, size);
	return ret;
}

static int
int_zfs_fetch_nvlist(struct zfs_data *data, char **nvlist)
{
	int err;

	*nvlist = malloc(VDEV_PHYS_SIZE);
	/* Read in the vdev name-value pair list (112K). */
	err = zfs_devread(data->vdev_phys_sector, 0, VDEV_PHYS_SIZE, *nvlist);
	if (err) {
		free(*nvlist);
		*nvlist = 0;
		return err;
	}
	return ZFS_ERR_NONE;
}

/*
 * Check the disk label information and retrieve needed vdev name-value pairs.
 *
 */
static int
check_pool_label(struct zfs_data *data)
{
	uint64_t pool_state;
	char *nvlist;			/* for the pool */
	char *vdevnvlist;		/* for the vdev */
	uint64_t diskguid;
	uint64_t version;
	int found;
	int err;

	err = int_zfs_fetch_nvlist(data, &nvlist);
	if (err)
		return err;

	found = zfs_nvlist_lookup_uint64(nvlist, ZPOOL_CONFIG_POOL_STATE,
										  &pool_state);
	if (!found) {
		free(nvlist);
		printf("zfs pool state not found\n");
		return ZFS_ERR_BAD_FS;
	}

	if (pool_state == POOL_STATE_DESTROYED) {
		free(nvlist);
		printf("zpool is marked as destroyed\n");
		return ZFS_ERR_BAD_FS;
	}

	data->label_txg = 0;
	found = zfs_nvlist_lookup_uint64(nvlist, ZPOOL_CONFIG_POOL_TXG,
										  &data->label_txg);
	if (!found) {
		free(nvlist);
		printf("zfs pool txg not found\n");
		return ZFS_ERR_BAD_FS;
	}

	/* not an active device */
	if (data->label_txg == 0) {
		free(nvlist);
		printf("zpool is not active\n");
		return ZFS_ERR_BAD_FS;
	}

	found = zfs_nvlist_lookup_uint64(nvlist, ZPOOL_CONFIG_VERSION,
										  &version);
	if (!found) {
		free(nvlist);
		printf("zpool config version not found\n");
		return ZFS_ERR_BAD_FS;
	}

	if (version > SPA_VERSION) {
		free(nvlist);
		printf("SPA version too new %llu > %llu\n",
			   (unsigned long long) version,
			   (unsigned long long) SPA_VERSION);
		return ZFS_ERR_NOT_IMPLEMENTED_YET;
	}

	vdevnvlist = zfs_nvlist_lookup_nvlist(nvlist, ZPOOL_CONFIG_VDEV_TREE);
	if (!vdevnvlist) {
		free(nvlist);
		printf("ZFS config vdev tree not found\n");
		return ZFS_ERR_BAD_FS;
	}

	found = zfs_nvlist_lookup_uint64(vdevnvlist, ZPOOL_CONFIG_ASHIFT,
										  &data->vdev_ashift);
	free(vdevnvlist);
	if (!found) {
		free(nvlist);
		printf("ZPOOL config ashift not found\n");
		return ZFS_ERR_BAD_FS;
	}

	found = zfs_nvlist_lookup_uint64(nvlist, ZPOOL_CONFIG_GUID, &diskguid);
	if (!found) {
		free(nvlist);
		printf("ZPOOL config guid not found\n");
		return ZFS_ERR_BAD_FS;
	}

	found = zfs_nvlist_lookup_uint64(nvlist, ZPOOL_CONFIG_POOL_GUID, &data->pool_guid);
	if (!found) {
		free(nvlist);
		printf("ZPOOL config pool guid not found\n");
		return ZFS_ERR_BAD_FS;
	}

	free(nvlist);

	printf("ZFS Pool GUID: %llu (%016llx) Label: GUID: %llu (%016llx), txg: %llu, SPA v%llu, ashift: %llu\n",
		   (unsigned long long) data->pool_guid,
		   (unsigned long long) data->pool_guid,
		   (unsigned long long) diskguid,
		   (unsigned long long) diskguid,
		   (unsigned long long) data->label_txg,
		   (unsigned long long) version,
		   (unsigned long long) data->vdev_ashift);

	return ZFS_ERR_NONE;
}

/*
 * vdev_label_start returns the physical disk offset (in bytes) of
 * label "l".
 */
static uint64_t vdev_label_start(uint64_t psize, int l)
{
	return (l * sizeof(vdev_label_t) + (l < VDEV_LABELS / 2 ?
										0 : psize -
										VDEV_LABELS * sizeof(vdev_label_t)));
}

void
zfs_unmount(struct zfs_data *data)
{
	free(data->dnode_buf);
	free(data->dnode_mdn);
	free(data->file_buf);
	free(data);
}

/*
 * zfs_mount() locates a valid uberblock of the root pool and read in its MOS
 * to the memory address MOS.
 *
 */
struct zfs_data *
zfs_mount(device_t dev)
{
	struct zfs_data *data = 0;
	int label = 0, bestlabel = -1;
	char *ub_array;
	uberblock_t *ubbest;
	uberblock_t *ubcur = NULL;
	void *osp = 0;
	size_t ospsize;
	int err;

	data = malloc(sizeof(*data));
	if (!data)
		return 0;
	memset(data, 0, sizeof(*data));

	ub_array = malloc(VDEV_UBERBLOCK_RING);
	if (!ub_array) {
		zfs_unmount(data);
		return 0;
	}

	ubbest = malloc(sizeof(*ubbest));
	if (!ubbest) {
		free(ub_array);
		zfs_unmount(data);
		return 0;
	}
	memset(ubbest, 0, sizeof(*ubbest));

	/*
	 * some eltorito stacks don't give us a size and
	 * we end up setting the size to MAXUINT, further
	 * some of these devices stop working once a single
	 * read past the end has been issued. Checking
	 * for a maximum part_length and skipping the backup
	 * labels at the end of the slice/partition/device
	 * avoids breaking down on such devices.
	 */
	const int vdevnum =
		dev->part_length == 0 ?
		VDEV_LABELS / 2 : VDEV_LABELS;

	/* Size in bytes of the device (disk or partition) aligned to label size*/
	uint64_t device_size =
		dev->part_length << SECTOR_BITS;

	const uint64_t alignedbytes =
		P2ALIGN(device_size, (uint64_t) sizeof(vdev_label_t));

	for (label = 0; label < vdevnum; label++) {
		uint64_t labelstartbytes = vdev_label_start(alignedbytes, label);
		uint64_t labelstart = labelstartbytes >> SECTOR_BITS;

		debug("zfs reading label %d at sector %llu (byte %llu)\n",
			  label, (unsigned long long) labelstart,
			  (unsigned long long) labelstartbytes);

		data->vdev_phys_sector = labelstart +
			((VDEV_SKIP_SIZE + VDEV_BOOT_HEADER_SIZE) >> SECTOR_BITS);

		err = check_pool_label(data);
		if (err) {
			printf("zfs error checking label %d\n", label);
			continue;
		}

		/* Read in the uberblock ring (128K). */
		err = zfs_devread(data->vdev_phys_sector  +
						  (VDEV_PHYS_SIZE >> SECTOR_BITS),
						  0, VDEV_UBERBLOCK_RING, ub_array);
		if (err) {
			printf("zfs error reading uberblock ring for label %d\n", label);
			continue;
		}

		ubcur = find_bestub(ub_array, data);
		if (!ubcur) {
			printf("zfs No good uberblocks found in label %d\n", label);
			continue;
		}

		if (vdev_uberblock_compare(ubcur, ubbest) > 0) {
			/* Looks like the block is good, so use it.*/
			memcpy(ubbest, ubcur, sizeof(*ubbest));
			bestlabel = label;
			debug("zfs Current best uberblock found in label %d\n", label);
		}
	}
	free(ub_array);

	/* We zero'd the structure to begin with.  If we never assigned to it,
	   magic will still be zero. */
	if (!ubbest->ub_magic) {
		printf("couldn't find a valid ZFS label\n");
		zfs_unmount(data);
		free(ubbest);
		return 0;
	}

	debug("zfs ubbest %p in label %d\n", ubbest, bestlabel);

	zfs_endian_t ub_endian =
		zfs_to_cpu64(ubbest->ub_magic, LITTLE_ENDIAN) == UBERBLOCK_MAGIC
		? LITTLE_ENDIAN : BIG_ENDIAN;

	debug("zfs endian set to %s\n", !ub_endian ? "big" : "little");

	err = zio_read(&ubbest->ub_rootbp, ub_endian, &osp, &ospsize, data);

	if (err) {
		printf("couldn't zio_read object directory\n");
		zfs_unmount(data);
		free(osp);
		free(ubbest);
		return 0;
	}

	if (ospsize < OBJSET_PHYS_SIZE_V14) {
		printf("osp too small\n");
		zfs_unmount(data);
		free(osp);
		free(ubbest);
		return 0;
	}

	/* Got the MOS. Save it at the memory addr MOS. */
	memmove(&(data->mos.dn), &((objset_phys_t *) osp)->os_meta_dnode, DNODE_SIZE);
	data->mos.endian =
		(zfs_to_cpu64(ubbest->ub_rootbp.blk_prop, ub_endian) >> 63) & 1;
	memmove(&(data->current_uberblock), ubbest, sizeof(uberblock_t));

	free(osp);
	free(ubbest);

	return data;
}

int
zfs_fetch_nvlist(device_t dev, char **nvlist)
{
	struct zfs_data *zfs;
	int err;

	zfs = zfs_mount(dev);
	if (!zfs)
		return ZFS_ERR_BAD_FS;
	err = int_zfs_fetch_nvlist(zfs, nvlist);
	zfs_unmount(zfs);
	return err;
}

/*
 * zfs_open() locates a file in the rootpool by following the
 * MOS and places the dnode of the file in the memory address DNODE.
 */
int
zfs_open(struct zfs_file *file, const char *fsfilename)
{
	struct zfs_data *data;
	int err;
	int isfs;

	data = zfs_mount(file->device);
	if (!data)
		return ZFS_ERR_BAD_FS;

	err = dnode_get_fullpath(fsfilename, &(data->mdn), 0,
							 &(data->dnode), &isfs, data);
	if (err) {
		zfs_unmount(data);
		return err;
	}

	if (isfs) {
		zfs_unmount(data);
		printf("Missing @ or / separator\n");
		return ZFS_ERR_FILE_NOT_FOUND;
	}

	/* We found the dnode for this file. Verify if it is a plain file. */
	if (data->dnode.dn.dn_type != DMU_OT_PLAIN_FILE_CONTENTS) {
		zfs_unmount(data);
		printf("not a file\n");
		return ZFS_ERR_BAD_FILE_TYPE;
	}

	/* get the file size and set the file position to 0 */

	/*
	 * For DMU_OT_SA we will need to locate the SIZE attribute
	 * attribute, which could be either in the bonus buffer
	 * or the "spill" block.
	 */
	if (data->dnode.dn.dn_bonustype == DMU_OT_SA) {
		void *sahdrp;
		int hdrsize;

		if (data->dnode.dn.dn_bonuslen != 0) {
			sahdrp = (sa_hdr_phys_t *) DN_BONUS(&data->dnode.dn);
		} else if (data->dnode.dn.dn_flags & DNODE_FLAG_SPILL_BLKPTR) {
			blkptr_t *bp = &data->dnode.dn.dn_spill;

			err = zio_read(bp, data->dnode.endian, &sahdrp, NULL, data);
			if (err)
				return err;
		} else {
			printf("filesystem is corrupt :(\n");
			return ZFS_ERR_BAD_FS;
		}

		hdrsize = SA_HDR_SIZE(((sa_hdr_phys_t *) sahdrp));
		file->size = *(uint64_t *) ((char *) sahdrp + hdrsize + SA_SIZE_OFFSET);
		if ((data->dnode.dn.dn_bonuslen == 0) &&
			(data->dnode.dn.dn_flags & DNODE_FLAG_SPILL_BLKPTR))
			free(sahdrp);
	} else {
		file->size = zfs_to_cpu64(((znode_phys_t *) DN_BONUS(&data->dnode.dn))->zp_size, data->dnode.endian);
	}

	file->data = data;
	file->offset = 0;

	return ZFS_ERR_NONE;
}

uint64_t
zfs_read(zfs_file_t file, char *buf, uint64_t len)
{
	struct zfs_data *data = (struct zfs_data *) file->data;
	int blksz, movesize;
	uint64_t length;
	int64_t red;
	int err;

	if (data->file_buf == NULL) {
		data->file_buf = malloc(SPA_MAXBLOCKSIZE);
		if (!data->file_buf)
			return -1;
		data->file_start = data->file_end = 0;
	}

	/*
	 * If offset is in memory, move it into the buffer provided and return.
	 */
	if (file->offset >= data->file_start
		&& file->offset + len <= data->file_end) {
		memmove(buf, data->file_buf + file->offset - data->file_start,
				len);
		return len;
	}

	blksz = zfs_to_cpu16(data->dnode.dn.dn_datablkszsec,
							  data->dnode.endian) << SPA_MINBLOCKSHIFT;

	/*
	 * Entire Dnode is too big to fit into the space available.	 We
	 * will need to read it in chunks.	This could be optimized to
	 * read in as large a chunk as there is space available, but for
	 * now, this only reads in one data block at a time.
	 */
	length = len;
	red = 0;
	while (length) {
		void *t;
		/*
		 * Find requested blkid and the offset within that block.
		 */
		uint64_t blkid = file->offset + red;
		blkid = do_div(blkid, blksz);
		free(data->file_buf);
		data->file_buf = 0;

		err = dmu_read(&(data->dnode), blkid, &t,
					   0, data);
		data->file_buf = t;
		if (err)
			return -1;

		data->file_start = blkid * blksz;
		data->file_end = data->file_start + blksz;

		movesize = min(length, data->file_end - (int)file->offset - red);

		memmove(buf, data->file_buf + file->offset + red
				- data->file_start, movesize);
		buf += movesize;
		length -= movesize;
		red += movesize;
	}

	return len;
}

int
zfs_close(zfs_file_t file)
{
	zfs_unmount((struct zfs_data *) file->data);
	return ZFS_ERR_NONE;
}

int
zfs_getmdnobj(device_t dev, const char *fsfilename,
				   uint64_t *mdnobj)
{
	struct zfs_data *data;
	int err;
	int isfs;

	data = zfs_mount(dev);
	if (!data)
		return ZFS_ERR_BAD_FS;

	err = dnode_get_fullpath(fsfilename, &(data->mdn), mdnobj,
							 &(data->dnode), &isfs, data);
	zfs_unmount(data);
	return err;
}

static void
fill_fs_info(struct zfs_dirhook_info *info,
			 dnode_end_t mdn, struct zfs_data *data)
{
	int err;
	dnode_end_t dn;
	uint64_t objnum;
	uint64_t headobj;

	memset(info, 0, sizeof(*info));

	info->dir = 1;

	if (mdn.dn.dn_type == DMU_OT_DSL_DIR) {
		headobj = zfs_to_cpu64(((dsl_dir_phys_t *) DN_BONUS(&mdn.dn))->dd_head_dataset_obj, mdn.endian);

		err = dnode_get(&(data->mos), headobj, DMU_OT_DSL_DATASET, &mdn, data);
		if (err) {
			printf("zfs failed here 1\n");
			return;
		}
	}
	make_mdn(&mdn, data);
	err = dnode_get(&mdn, MASTER_NODE_OBJ, DMU_OT_MASTER_NODE,
					&dn, data);
	if (err) {
		printf("zfs failed here 2\n");
		return;
	}

	err = zap_lookup(&dn, ZFS_ROOT_OBJ, &objnum, data);
	if (err) {
		printf("zfs failed here 3\n");
		return;
	}

	err = dnode_get(&mdn, objnum, 0, &dn, data);
	if (err) {
		printf("zfs failed here 4\n");
		return;
	}

	info->mtimeset = 1;
	info->mtime = zfs_to_cpu64(((znode_phys_t *) DN_BONUS(&dn.dn))->zp_mtime[0], dn.endian);

	return;
}

static int iterate_zap(const char *name, uint64_t val, struct zfs_data *data)
{
	struct zfs_dirhook_info info;
	dnode_end_t dn;

	memset(&info, 0, sizeof(info));

	dnode_get(&(data->mdn), val, 0, &dn, data);
	info.mtimeset = 1;
	info.mtime = zfs_to_cpu64(((znode_phys_t *) DN_BONUS(&dn.dn))->zp_mtime[0], dn.endian);
	info.dir = (dn.dn.dn_type == DMU_OT_DIRECTORY_CONTENTS);
	debug("zfs type=%d, name=%s\n",
		  (int)dn.dn.dn_type, (char *)name);
	if (!data->userhook)
		return 0;
	return data->userhook(name, &info);
}

static int iterate_zap_fs(const char *name, uint64_t val, struct zfs_data *data)
{
	struct zfs_dirhook_info info;
	dnode_end_t mdn;
	int err;
	err = dnode_get(&(data->mos), val, 0, &mdn, data);
	if (err)
		return 0;
	if (mdn.dn.dn_type != DMU_OT_DSL_DIR)
		return 0;

	fill_fs_info(&info, mdn, data);

	if (!data->userhook)
		return 0;
	return data->userhook(name, &info);
}

static int iterate_zap_snap(const char *name, uint64_t val, struct zfs_data *data)
{
	struct zfs_dirhook_info info;
	char *name2;
	int ret = 0;
	dnode_end_t mdn;
	int err;

	err = dnode_get(&(data->mos), val, 0, &mdn, data);
	if (err)
		return 0;

	if (mdn.dn.dn_type != DMU_OT_DSL_DATASET)
		return 0;

	fill_fs_info(&info, mdn, data);

	name2 = malloc(strlen(name) + 2);
	name2[0] = '@';
	memcpy(name2 + 1, name, strlen(name) + 1);
	if (data->userhook)
		ret = data->userhook(name2, &info);
	free(name2);
	return ret;
}

int
zfs_ls(device_t device, const char *path,
	   int (*hook)(const char *, const struct zfs_dirhook_info *))
{
	struct zfs_data *data;
	int err;
	int isfs;

	data = zfs_mount(device);
	if (!data)
		return ZFS_ERR_BAD_FS;

	data->userhook = hook;

	err = dnode_get_fullpath(path, &(data->mdn), 0, &(data->dnode), &isfs, data);
	if (err) {
		zfs_unmount(data);
		return err;
	}
	if (isfs) {
		uint64_t childobj, headobj;
		uint64_t snapobj;
		dnode_end_t dn;
		struct zfs_dirhook_info info;

		fill_fs_info(&info, data->dnode, data);
		hook("@", &info);

		childobj = zfs_to_cpu64(((dsl_dir_phys_t *) DN_BONUS(&data->dnode.dn))->dd_child_dir_zapobj, data->dnode.endian);
		headobj = zfs_to_cpu64(((dsl_dir_phys_t *) DN_BONUS(&data->dnode.dn))->dd_head_dataset_obj, data->dnode.endian);
		err = dnode_get(&(data->mos), childobj,
						DMU_OT_DSL_DIR_CHILD_MAP, &dn, data);
		if (err) {
			zfs_unmount(data);
			return err;
		}


		zap_iterate(&dn, iterate_zap_fs, data);

		err = dnode_get(&(data->mos), headobj, DMU_OT_DSL_DATASET, &dn, data);
		if (err) {
			zfs_unmount(data);
			return err;
		}

		snapobj = zfs_to_cpu64(((dsl_dataset_phys_t *) DN_BONUS(&dn.dn))->ds_snapnames_zapobj, dn.endian);

		err = dnode_get(&(data->mos), snapobj,
						DMU_OT_DSL_DS_SNAP_MAP, &dn, data);
		if (err) {
			zfs_unmount(data);
			return err;
		}

		zap_iterate(&dn, iterate_zap_snap, data);
	} else {
		if (data->dnode.dn.dn_type != DMU_OT_DIRECTORY_CONTENTS) {
			zfs_unmount(data);
			printf("not a directory\n");
			return ZFS_ERR_BAD_FILE_TYPE;
		}
		zap_iterate(&(data->dnode), iterate_zap, data);
	}
	zfs_unmount(data);
	return ZFS_ERR_NONE;
}
