/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#ifndef __BTRFS_BTRFS_H__
#define __BTRFS_BTRFS_H__

#include <linux/rbtree.h>
#include "ctree.h"

extern struct btrfs_info btrfs_info;
extern struct btrfs_fs_info *current_fs_info;

/* compression.c */
u32 btrfs_decompress(u8 type, const char *, u32, char *, u32);

/* inode.c */
int btrfs_readlink(struct btrfs_root *root, u64 ino, char *target);
int btrfs_file_read(struct btrfs_root *root, u64 ino, u64 file_offset, u64 len,
		    char *dest);

/* subvolume.c */
u64 btrfs_get_default_subvol_objectid(void);

#endif /* !__BTRFS_BTRFS_H__ */
