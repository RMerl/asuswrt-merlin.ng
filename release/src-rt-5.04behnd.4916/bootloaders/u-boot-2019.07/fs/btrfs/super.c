// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <memalign.h>

#define BTRFS_SUPER_FLAG_SUPP	(BTRFS_HEADER_FLAG_WRITTEN	\
				 | BTRFS_HEADER_FLAG_RELOC	\
				 | BTRFS_SUPER_FLAG_ERROR	\
				 | BTRFS_SUPER_FLAG_SEEDING	\
				 | BTRFS_SUPER_FLAG_METADUMP)

#define BTRFS_SUPER_INFO_SIZE	4096

/*
 * checks if a valid root backup is present.
 * considers the case when all root backups empty valid.
 * returns -1 in case of invalid root backup and 0 for valid.
 */
static int btrfs_check_super_roots(struct btrfs_super_block *sb)
{
	struct btrfs_root_backup *root_backup;
	int i, newest = -1;
	int num_empty = 0;

	for (i = 0; i < BTRFS_NUM_BACKUP_ROOTS; ++i) {
		root_backup = sb->super_roots + i;

		if (root_backup->tree_root == 0 && root_backup->tree_root_gen == 0)
			num_empty++;

		if (root_backup->tree_root_gen == sb->generation)
			newest = i;
	}

	if (num_empty == BTRFS_NUM_BACKUP_ROOTS) {
		return 0;
	} else if (newest >= 0) {
		return 0;
	}

	return -1;
}

static inline int is_power_of_2(u64 x)
{
	return !(x & (x - 1));
}

static int btrfs_check_super_csum(char *raw_disk_sb)
{
	struct btrfs_super_block *disk_sb =
		(struct btrfs_super_block *) raw_disk_sb;
	u16 csum_type = le16_to_cpu(disk_sb->csum_type);

	if (csum_type == BTRFS_CSUM_TYPE_CRC32) {
		u32 crc = ~(u32) 0;
		const int csum_size = sizeof(crc);
		char result[csum_size];

		crc = btrfs_csum_data(raw_disk_sb + BTRFS_CSUM_SIZE, crc,
				      BTRFS_SUPER_INFO_SIZE - BTRFS_CSUM_SIZE);
		btrfs_csum_final(crc, result);

		if (memcmp(raw_disk_sb, result, csum_size))
			return -1;
	} else {
		return -1;
	}

	return 0;
}

static int btrfs_check_super(struct btrfs_super_block *sb)
{
	int ret = 0;

	if (sb->flags & ~BTRFS_SUPER_FLAG_SUPP) {
		printf("%s: Unsupported flags: %llu\n", __func__,
		       sb->flags & ~BTRFS_SUPER_FLAG_SUPP);
	}

	if (sb->root_level > BTRFS_MAX_LEVEL) {
		printf("%s: tree_root level too big: %d >= %d\n", __func__,
		       sb->root_level, BTRFS_MAX_LEVEL);
		ret = -1;
	}

	if (sb->chunk_root_level > BTRFS_MAX_LEVEL) {
		printf("%s: chunk_root level too big: %d >= %d\n", __func__,
		       sb->chunk_root_level, BTRFS_MAX_LEVEL);
		ret = -1;
	}

	if (sb->log_root_level > BTRFS_MAX_LEVEL) {
		printf("%s: log_root level too big: %d >= %d\n", __func__,
		       sb->log_root_level, BTRFS_MAX_LEVEL);
		ret = -1;
	}

	if (!is_power_of_2(sb->sectorsize) || sb->sectorsize < 4096 ||
	    sb->sectorsize > BTRFS_MAX_METADATA_BLOCKSIZE) {
		printf("%s: invalid sectorsize %u\n", __func__,
		       sb->sectorsize);
		ret = -1;
	}

	if (!is_power_of_2(sb->nodesize) || sb->nodesize < sb->sectorsize ||
	    sb->nodesize > BTRFS_MAX_METADATA_BLOCKSIZE) {
		printf("%s: invalid nodesize %u\n", __func__, sb->nodesize);
		ret = -1;
	}

	if (sb->nodesize != sb->__unused_leafsize) {
		printf("%s: invalid leafsize %u, should be %u\n", __func__,
		       sb->__unused_leafsize, sb->nodesize);
		ret = -1;
	}

	if (!IS_ALIGNED(sb->root, sb->sectorsize)) {
		printf("%s: tree_root block unaligned: %llu\n", __func__,
		       sb->root);
		ret = -1;
	}

	if (!IS_ALIGNED(sb->chunk_root, sb->sectorsize)) {
		printf("%s: chunk_root block unaligned: %llu\n", __func__,
		       sb->chunk_root);
		ret = -1;
	}

	if (!IS_ALIGNED(sb->log_root, sb->sectorsize)) {
		printf("%s: log_root block unaligned: %llu\n", __func__,
		       sb->log_root);
		ret = -1;
	}

	if (memcmp(sb->fsid, sb->dev_item.fsid, BTRFS_UUID_SIZE) != 0) {
		printf("%s: dev_item UUID does not match fsid\n", __func__);
		ret = -1;
	}

	if (sb->bytes_used < 6*sb->nodesize) {
		printf("%s: bytes_used is too small %llu\n", __func__,
		       sb->bytes_used);
		ret = -1;
	}

	if (!is_power_of_2(sb->stripesize)) {
		printf("%s: invalid stripesize %u\n", __func__, sb->stripesize);
		ret = -1;
	}

	if (sb->sys_chunk_array_size > BTRFS_SYSTEM_CHUNK_ARRAY_SIZE) {
		printf("%s: system chunk array too big %u > %u\n", __func__,
		       sb->sys_chunk_array_size, BTRFS_SYSTEM_CHUNK_ARRAY_SIZE);
		ret = -1;
	}

	if (sb->sys_chunk_array_size < sizeof(struct btrfs_key) +
	    sizeof(struct btrfs_chunk)) {
		printf("%s: system chunk array too small %u < %zu\n", __func__,
		       sb->sys_chunk_array_size, sizeof(struct btrfs_key)
		       + sizeof(struct btrfs_chunk));
		ret = -1;
	}

	return ret;
}

int btrfs_read_superblock(void)
{
	const u64 superblock_offsets[4] = {
		0x10000ull,
		0x4000000ull,
		0x4000000000ull,
		0x4000000000000ull
	};
	ALLOC_CACHE_ALIGN_BUFFER(char, raw_sb, BTRFS_SUPER_INFO_SIZE);
	struct btrfs_super_block *sb = (struct btrfs_super_block *) raw_sb;
	u64 dev_total_bytes;
	int i;

	dev_total_bytes = (u64) btrfs_part_info->size * btrfs_part_info->blksz;

	btrfs_info.sb.generation = 0;

	for (i = 0; i < 4; ++i) {
		if (superblock_offsets[i] + sizeof(sb) > dev_total_bytes)
			break;

		if (!btrfs_devread(superblock_offsets[i], BTRFS_SUPER_INFO_SIZE,
				   raw_sb))
			break;

		if (btrfs_check_super_csum(raw_sb)) {
			debug("%s: invalid checksum at superblock mirror %i\n",
			      __func__, i);
			continue;
		}

		btrfs_super_block_to_cpu(sb);

		if (sb->magic != BTRFS_MAGIC) {
			debug("%s: invalid BTRFS magic 0x%016llX at "
			      "superblock mirror %i\n", __func__, sb->magic, i);
		} else if (sb->bytenr != superblock_offsets[i]) {
			printf("%s: invalid bytenr 0x%016llX (expected "
			       "0x%016llX) at superblock mirror %i\n",
			       __func__, sb->bytenr, superblock_offsets[i], i);
		} else if (btrfs_check_super(sb)) {
			printf("%s: Checking superblock mirror %i failed\n",
			       __func__, i);
		} else if (sb->generation > btrfs_info.sb.generation) {
			memcpy(&btrfs_info.sb, sb, sizeof(*sb));
		} else {
			/* Nothing */
		}
	}

	if (!btrfs_info.sb.generation) {
		debug("%s: No valid BTRFS superblock found!\n", __func__);
		return -1;
	}

	if (btrfs_check_super_roots(&btrfs_info.sb)) {
		printf("%s: No valid root_backup found!\n", __func__);
		return -1;
	}

	if (btrfs_info.sb.num_devices != 1) {
		printf("%s: Unsupported number of devices (%lli). This driver "
		       "only supports filesystem on one device.\n", __func__,
		       btrfs_info.sb.num_devices);
		return -1;
	}

	debug("Chosen superblock with generation = %llu\n",
	      btrfs_info.sb.generation);

	return 0;
}
