// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <errno.h>
#include <fat.h>
#include <mmc.h>
#include <scsi.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <linux/stddef.h>

#ifdef CONFIG_SPL_BUILD
/* TODO(sjg@chromium.org): Figure out why this is needed */
# if !defined(CONFIG_TARGET_AM335X_EVM) || defined(CONFIG_SPL_OS_BOOT)
#  define LOADENV
# endif
#else
# define LOADENV
#endif

DECLARE_GLOBAL_DATA_PTR;

__weak const char *env_fat_get_intf(void)
{
	return (const char *)CONFIG_ENV_FAT_INTERFACE;
}

__weak char *env_fat_get_dev_part(void)
{
#ifdef CONFIG_MMC
	static char *part_str;

	if (!part_str) {
		part_str = CONFIG_ENV_FAT_DEVICE_AND_PART;
		if (!strcmp(CONFIG_ENV_FAT_INTERFACE, "mmc") && part_str[0] == ':') {
			part_str = "0" CONFIG_ENV_FAT_DEVICE_AND_PART;
			part_str[0] += mmc_get_env_dev();
		}
	}

	return part_str;
#else
	return CONFIG_ENV_FAT_DEVICE_AND_PART;
#endif
}

static int env_fat_save(void)
{
	env_t __aligned(ARCH_DMA_MINALIGN) env_new;
	struct blk_desc *dev_desc = NULL;
	struct disk_partition info;
	const char *file = CONFIG_ENV_FAT_FILE;
	int dev, part;
	int err;
	loff_t size;
	const char *ifname = env_fat_get_intf();
	const char *dev_and_part = env_fat_get_dev_part();

	err = env_export(&env_new);
	if (err)
		return err;

	part = blk_get_device_part_str(ifname, dev_and_part,
				       &dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->devnum;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to use %s %d:%d...\n", ifname, dev, part);
		return 1;
	}

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	if (gd->env_valid == ENV_VALID)
		file = CONFIG_ENV_FAT_FILE_REDUND;
#endif

	err = file_fat_write(file, (void *)&env_new, 0, sizeof(env_t), &size);
	if (err == -1) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to write \"%s\" from %s%d:%d...\n", file, ifname, dev, part);
		return 1;
	}

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	gd->env_valid = (gd->env_valid == ENV_REDUND) ? ENV_VALID : ENV_REDUND;
#endif

	return 0;
}

#ifdef LOADENV
static int env_fat_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf1, CONFIG_ENV_SIZE);
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	ALLOC_CACHE_ALIGN_BUFFER(char, buf2, CONFIG_ENV_SIZE);
	int err2;
#endif
	struct blk_desc *dev_desc = NULL;
	struct disk_partition info;
	int dev, part;
	int err1;
	const char *ifname = env_fat_get_intf();
	const char *dev_and_part = env_fat_get_dev_part();

#ifdef CONFIG_MMC
	if (!strcmp(ifname, "mmc"))
		mmc_initialize(NULL);
#endif
#ifndef CONFIG_SPL_BUILD
#if defined(CONFIG_AHCI) || defined(CONFIG_SCSI)
	if (!strcmp(CONFIG_ENV_FAT_INTERFACE, "scsi"))
		scsi_scan(true);
#endif
#endif
	part = blk_get_device_part_str(ifname, dev_and_part,
				       &dev_desc, &info, 1);
	if (part < 0)
		goto err_env_relocate;

	dev = dev_desc->devnum;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to use %s %d:%d...\n", ifname, dev, part);
		goto err_env_relocate;
	}

	err1 = file_fat_read(CONFIG_ENV_FAT_FILE, buf1, CONFIG_ENV_SIZE);
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	err2 = file_fat_read(CONFIG_ENV_FAT_FILE_REDUND, buf2, CONFIG_ENV_SIZE);

	err1 = (err1 >= 0) ? 0 : -1;
	err2 = (err2 >= 0) ? 0 : -1;
	return env_import_redund(buf1, err1, buf2, err2, H_EXTERNAL);
#else
	if (err1 < 0) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to read \"%s\" from %s%d:%d... \n",
			CONFIG_ENV_FAT_FILE, ifname, dev, part);
		goto err_env_relocate;
	}

	return env_import(buf1, 1, H_EXTERNAL);
#endif

err_env_relocate:
	env_set_default(NULL, 0);

	return -EIO;
}
#endif /* LOADENV */

U_BOOT_ENV_LOCATION(fat) = {
	.location	= ENVL_FAT,
	ENV_NAME("FAT")
#ifdef LOADENV
	.load		= env_fat_load,
#endif
	.save		= ENV_SAVE_PTR(env_fat_save),
};
