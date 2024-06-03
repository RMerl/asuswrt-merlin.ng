// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <u-boot/crc.h>
#include <asm/unaligned.h>

static u32 btrfs_crc32c_table[256];

void btrfs_hash_init(void)
{
	static int inited = 0;

	if (!inited) {
		crc32c_init(btrfs_crc32c_table, 0x82F63B78);
		inited = 1;
	}
}

u32 btrfs_crc32c(u32 crc, const void *data, size_t length)
{
	return crc32c_cal(crc, (const char *) data, length,
			  btrfs_crc32c_table);
}

u32 btrfs_csum_data(char *data, u32 seed, size_t len)
{
	return btrfs_crc32c(seed, data, len);
}

void btrfs_csum_final(u32 crc, void *result)
{
	put_unaligned(cpu_to_le32(~crc), (u32 *)result);
}
