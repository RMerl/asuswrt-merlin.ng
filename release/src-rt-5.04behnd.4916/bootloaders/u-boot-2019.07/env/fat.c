// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2011 by Tigris Elektronik GmbH
 *
 * Author:
 *  Maximilian Schwerin <mvs@tigris.de>
 */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <search.h>
#include <errno.h>
#include <fat.h>
#include <mmc.h>

#ifdef CONFIG_SPL_BUILD
/* TODO(sjg@chromium.org): Figure out why this is needed */
# if !defined(CONFIG_TARGET_AM335X_EVM) || defined(CONFIG_SPL_OS_BOOT)
#  define LOADENV
# endif
#else
# define LOADENV
# if defined(CONFIG_CMD_SAVEENV)
#  define CMD_SAVEENV
# endif
#endif

#ifdef CMD_SAVEENV
static int env_fat_save(void)
{
	env_t __aligned(ARCH_DMA_MINALIGN) env_new;
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;
	loff_t size;

	err = env_export(&env_new);
	if (err)
		return err;

	part = blk_get_device_part_str(CONFIG_ENV_FAT_INTERFACE,
					CONFIG_ENV_FAT_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		return 1;

	dev = dev_desc->devnum;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to use %s %d:%d... ",
		       CONFIG_ENV_FAT_INTERFACE, dev, part);
		return 1;
	}

	err = file_fat_write(CONFIG_ENV_FAT_FILE, (void *)&env_new, 0, sizeof(env_t),
			     &size);
	if (err == -1) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to write \"%s\" from %s%d:%d... ",
			CONFIG_ENV_FAT_FILE, CONFIG_ENV_FAT_INTERFACE, dev, part);
		return 1;
	}

	return 0;
}
#endif /* CMD_SAVEENV */

#ifdef LOADENV
static int env_fat_load(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	struct blk_desc *dev_desc = NULL;
	disk_partition_t info;
	int dev, part;
	int err;

#ifdef CONFIG_MMC
	if (!strcmp(CONFIG_ENV_FAT_INTERFACE, "mmc"))
		mmc_initialize(NULL);
#endif

	part = blk_get_device_part_str(CONFIG_ENV_FAT_INTERFACE,
					CONFIG_ENV_FAT_DEVICE_AND_PART,
					&dev_desc, &info, 1);
	if (part < 0)
		goto err_env_relocate;

	dev = dev_desc->devnum;
	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to use %s %d:%d... ",
		       CONFIG_ENV_FAT_INTERFACE, dev, part);
		goto err_env_relocate;
	}

	err = file_fat_read(CONFIG_ENV_FAT_FILE, buf, CONFIG_ENV_SIZE);
	if (err == -1) {
		/*
		 * This printf is embedded in the messages from env_save that
		 * will calling it. The missing \n is intentional.
		 */
		printf("Unable to read \"%s\" from %s%d:%d... ",
			CONFIG_ENV_FAT_FILE, CONFIG_ENV_FAT_INTERFACE, dev, part);
		goto err_env_relocate;
	}

	return env_import(buf, 1);

err_env_relocate:
	set_default_env(NULL, 0);

	return -EIO;
}
#endif /* LOADENV */

U_BOOT_ENV_LOCATION(fat) = {
	.location	= ENVL_FAT,
	ENV_NAME("FAT")
#ifdef LOADENV
	.load		= env_fat_load,
#endif
#ifdef CMD_SAVEENV
	.save		= env_save_ptr(env_fat_save),
#endif
};
