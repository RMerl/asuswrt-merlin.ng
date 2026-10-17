/*
 * Header file for UBI support for U-Boot
 *
 * Adaptation from kernel to U-Boot
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __UBOOT_UBI_H
#define __UBOOT_UBI_H

#include <compiler.h>
#include <linux/compat.h>
#include <malloc.h>
#include <div64.h>
#include <linux/math64.h>
#include <linux/crc32.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/string.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/ubi.h>

#ifdef CONFIG_CMD_ONENAND
#include <onenand_uboot.h>
#endif

#include <linux/errno.h>

/* build.c */
#define get_device(...)
#define put_device(...)
#define ubi_sysfs_init(...)		0
#define ubi_sysfs_close(...)		do { } while (0)

#ifndef __UBIFS_H__
#include "../drivers/mtd/ubi/ubi.h"
#endif

/* functions */
extern int ubi_mtd_param_parse(const char *val, struct kernel_param *kp);
extern int ubi_init(void);
extern void ubi_exit(void);
extern int ubi_part(char *part_name, const char *vid_header_offset);
extern int ubi_volume_write(char *volume, void *buf, size_t size);
extern int ubi_volume_read(char *volume, char *buf, size_t size);

extern struct ubi_device *ubi_devices[];
int cmd_ubifs_mount(char *vol_name);
int cmd_ubifs_umount(void);

#endif
