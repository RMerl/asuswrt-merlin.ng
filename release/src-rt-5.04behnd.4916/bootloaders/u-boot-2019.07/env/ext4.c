// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2016 by VRT Technology
 *
 * Author:
 *  Stuart Longland <stuartl@vrt.com.au>
 *
 * Based on FAT environment driver
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 *
 * and EXT4 filesystem implementation
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <errno.h>
#include <ext4fs.h>
#include <mmc.h>

__weak const char *env_ext4_get_intf(void)
{
	return (const char *)CONFIG_ENV_EXT4_INTERFACE;
}

__weak const char *env_ext4_get_dev_part(void)
{
	return (const char *)CONFIG_ENV_EXT4_DEVICE_AND_PART;
}

#ifdef CONFIG_CMD_SAVEENV
static int env_ext4_save(void)
{
	env_t	env_new;
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;
	const char *ifname = env_ext4_get_intf();
	const char *dev_and_part = env_ext4_get_dev_part();

	err = env_export(&env_new);
	if (err)
		return err;

	part = blk_get_device_part_str(ifname, dev_and_part,
				       &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->devnum;
	ext4fs_set_blk_dev(dev_desc, &info);

	if (!ext4fs_mount(info.size)) {
		printf("\n** Unable to use %s %s for saveenv **\n",
		       ifname, dev_and_part);
		return 1;
	}

	err = ext4fs_write(CONFIG_ENV_EXT4_FILE, (void *)&env_new,
			   sizeof(env_t), FILETYPE_REG);
	ext4fs_close();

	if (err == -1) {
		printf("\n** Unable to write \"%s\" from %s%d:%d **\n",
			CONFIG_ENV_EXT4_FILE, ifname, dev, part);
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

static int env_ext4_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;
	loff_t off;
	const char *ifname = env_ext4_get_intf();
	const char *dev_and_part = env_ext4_get_dev_part();

#ifdef CONFIG_MMC
	if (!strcmp(ifname, "mmc"))
		mmc_initialize(NULL);
#endif

	part = blk_get_device_part_str(ifname, dev_and_part,
				       &dev_desc, &info, 1);
	if (part < 0)
		goto err_env_relocate;

	dev = dev_desc->devnum;
	ext4fs_set_blk_dev(dev_desc, &info);

	if (!ext4fs_mount(info.size)) {
		printf("\n** Unable to use %s %s for loading the env **\n",
		       ifname, dev_and_part);
		goto err_env_relocate;
	}

	err = ext4_read_file(CONFIG_ENV_EXT4_FILE, buf, 0, CONFIG_ENV_SIZE,
			     &off);
	ext4fs_close();

	if (err == -1) {
		printf("\n** Unable to read \"%s\" from %s%d:%d **\n",
			CONFIG_ENV_EXT4_FILE, ifname, dev, part);
		goto err_env_relocate;
	}

	return env_import(buf, 1);

err_env_relocate:
	set_default_env(NULL, 0);

	return -EIO;
}

U_BOOT_ENV_LOCATION(ext4) = {
	.location	= ENVL_EXT4,
	ENV_NAME("EXT4")
	.load		= env_ext4_load,
	.save		= env_save_ptr(env_ext4_save),
};
