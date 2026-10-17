/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 *
 * squashfs.h: SquashFS filesystem implementation.
 */

#ifndef _SQFS_H_
#define _SQFS_H_

struct disk_partition;

int sqfs_opendir(const char *filename, struct fs_dir_stream **dirsp);
int sqfs_readdir(struct fs_dir_stream *dirs, struct fs_dirent **dentp);
int sqfs_probe(struct blk_desc *fs_dev_desc,
	       struct disk_partition *fs_partition);
int sqfs_read(const char *filename, void *buf, loff_t offset,
	      loff_t len, loff_t *actread);
int sqfs_size(const char *filename, loff_t *size);
int sqfs_exists(const char *filename);
void sqfs_close(void);
void sqfs_closedir(struct fs_dir_stream *dirs);

#endif /* SQFS_H  */
