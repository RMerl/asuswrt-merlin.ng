/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 2017 by Marek Behún <kabel@kernel.org>
 *
 * Derived from code in ext4/dev.c, which was based on reiserfs/dev.c
 */

#ifndef __U_BOOT_FS_INTERNAL_H__
#define __U_BOOT_FS_INTERNAL_H__

#include <part.h>

int fs_devread(struct blk_desc *, struct disk_partition *, lbaint_t, int, int,
	       char *);

#endif /* __U_BOOT_FS_INTERNAL_H__ */
