// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include <common.h>
#include <compiler.h>
#include <fs_internal.h>

struct blk_desc *btrfs_blk_desc;
disk_partition_t *btrfs_part_info;

int btrfs_devread(u64 address, int byte_len, void *buf)
{
	lbaint_t sector;
	int byte_offset;

	sector = address >> btrfs_blk_desc->log2blksz;
	byte_offset = address % btrfs_blk_desc->blksz;

	return fs_devread(btrfs_blk_desc, btrfs_part_info, sector, byte_offset,
			  byte_len, buf);
}
