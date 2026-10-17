// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 General Electric Company. All rights reserved.
 */

#include <bootcount.h>
#include <fs.h>
#include <mapmem.h>

#define BC_MAGIC	0xbd
#define BC_VERSION	1

typedef struct {
	u8 magic;
	u8 version;
	u8 bootcount;
	u8 upgrade_available;
} bootcount_ext_t;

static u8 upgrade_available = 1;

void bootcount_store(ulong a)
{
	bootcount_ext_t *buf;
	loff_t len;
	int ret;

	if (fs_set_blk_dev(CONFIG_SYS_BOOTCOUNT_EXT_INTERFACE,
			   CONFIG_SYS_BOOTCOUNT_EXT_DEVPART, FS_TYPE_EXT)) {
		puts("Error selecting device\n");
		return;
	}

	/* Only update bootcount during upgrade process */
	if (!upgrade_available)
		return;

	buf = map_sysmem(CONFIG_SYS_BOOTCOUNT_ADDR, sizeof(bootcount_ext_t));
	buf->magic = BC_MAGIC;
	buf->version = BC_VERSION;
	buf->bootcount = (a & 0xff);
	buf->upgrade_available = upgrade_available;
	unmap_sysmem(buf);

	ret = fs_write(CONFIG_SYS_BOOTCOUNT_EXT_NAME,
		       CONFIG_SYS_BOOTCOUNT_ADDR, 0, sizeof(bootcount_ext_t),
		       &len);
	if (ret != 0)
		puts("Error storing bootcount\n");
}

ulong bootcount_load(void)
{
	bootcount_ext_t *buf;
	loff_t len_read;
	int ret;

	if (fs_set_blk_dev(CONFIG_SYS_BOOTCOUNT_EXT_INTERFACE,
			   CONFIG_SYS_BOOTCOUNT_EXT_DEVPART, FS_TYPE_EXT)) {
		puts("Error selecting device\n");
		return 0;
	}

	ret = fs_read(CONFIG_SYS_BOOTCOUNT_EXT_NAME, CONFIG_SYS_BOOTCOUNT_ADDR,
		      0, sizeof(bootcount_ext_t), &len_read);
	if (ret != 0 || len_read != sizeof(bootcount_ext_t)) {
		puts("Error loading bootcount\n");
		return 0;
	}

	buf = map_sysmem(CONFIG_SYS_BOOTCOUNT_ADDR, sizeof(bootcount_ext_t));
	if (buf->magic == BC_MAGIC && buf->version == BC_VERSION) {
		upgrade_available = buf->upgrade_available;
		if (upgrade_available)
			ret = buf->bootcount;
	} else {
		puts("Incorrect bootcount file\n");
	}

	unmap_sysmem(buf);

	return ret;
}
