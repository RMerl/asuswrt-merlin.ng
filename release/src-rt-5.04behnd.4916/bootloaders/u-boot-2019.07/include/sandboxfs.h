/*
 * Copyright (c) 2012, Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __SANDBOX_FS__
#define __SANDBOX_FS__

int sandbox_fs_set_blk_dev(struct blk_desc *rbdd, disk_partition_t *info);

int sandbox_fs_read_at(const char *filename, loff_t pos, void *buffer,
		       loff_t maxsize, loff_t *actread);
int sandbox_fs_write_at(const char *filename, loff_t pos, void *buffer,
			loff_t maxsize, loff_t *actwrite);

void sandbox_fs_close(void);
int sandbox_fs_ls(const char *dirname);
int sandbox_fs_exists(const char *filename);
int sandbox_fs_size(const char *filename, loff_t *size);
int fs_read_sandbox(const char *filename, void *buf, loff_t offset, loff_t len,
		    loff_t *actread);
int fs_write_sandbox(const char *filename, void *buf, loff_t offset,
		     loff_t len, loff_t *actwrite);

#endif
