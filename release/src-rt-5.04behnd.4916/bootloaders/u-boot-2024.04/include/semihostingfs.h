/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022, Sean Anderson <sean.anderson@seco.com>
 * Copyright (c) 2012, Google Inc.
 */

#ifndef __SEMIHOSTING_FS__
#define __SEMIHOSTING_FS__

struct blk_desc;
struct disk_partition;

int smh_fs_set_blk_dev(struct blk_desc *rbdd, struct disk_partition *info);
void smh_fs_close(void);
int smh_fs_size(const char *filename, loff_t *size);
int smh_fs_read(const char *filename, void *buf, loff_t offset, loff_t len,
		loff_t *actread);
int smh_fs_write(const char *filename, void *buf, loff_t offset,
		 loff_t len, loff_t *actwrite);

#endif
