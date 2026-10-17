// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#include <common.h>
#include <env.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>
#include <asm/u-boot.h>
#include <fat.h>
#include <errno.h>
#include <image.h>
#include <linux/libfdt.h>

static int fat_registered;

void spl_fat_force_reregister(void)
{
	fat_registered = 0;
}

static int spl_register_fat_device(struct blk_desc *block_dev, int partition)
{
	int err = 0;

	if (fat_registered)
		return err;

	err = fat_register_device(block_dev, partition);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: fat register err - %d\n", __func__, err);
#endif
		return err;
	}

	fat_registered = 1;

	return err;
}

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	loff_t actread;
	int ret;
	char *filename = load->priv;

	ret = fat_read_file(filename, buf, file_offset, size, &actread);
	if (ret)
		return ret;

	return actread;
}

int spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, int partition,
		       const char *filename)
{
	int err;
	loff_t size;
	struct spl_load_info load;

	err = spl_register_fat_device(block_dev, partition);
	if (err)
		goto end;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
		err = fat_size(filename, &size);
		if (err)
			goto end;
	} else {
		size = 0;
	}

	load.read = spl_fit_read;
	if (IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN))
		spl_set_bl_len(&load, ARCH_DMA_MINALIGN);
	else
		spl_set_bl_len(&load, 1);
	load.priv = (void *)filename;
	err = spl_load(spl_image, bootdev, &load, size, 0);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
#endif

	return err;
}

#if CONFIG_IS_ENABLED(OS_BOOT)
int spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, int partition)
{
	int err;
	__maybe_unused char *file;

	err = spl_register_fat_device(block_dev, partition);
	if (err)
		return err;

#if defined(CONFIG_SPL_ENV_SUPPORT) && defined(CONFIG_SPL_OS_BOOT)
	file = env_get("falcon_args_file");
	if (file) {
		err = file_fat_read(file, (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
		if (err <= 0) {
			printf("spl: error reading image %s, err - %d, falling back to default\n",
			       file, err);
			goto defaults;
		}
		file = env_get("falcon_image_file");
		if (file) {
			err = spl_load_image_fat(spl_image, bootdev, block_dev,
						 partition, file);
			if (err != 0) {
				puts("spl: falling back to default\n");
				goto defaults;
			}

			return 0;
		} else
			puts("spl: falcon_image_file not set in environment, falling back to default\n");
	} else
		puts("spl: falcon_args_file not set in environment, falling back to default\n");

defaults:
#endif

	err = file_fat_read(CONFIG_SPL_FS_LOAD_ARGS_NAME,
			    (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0);
	if (err <= 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
#endif
		return -1;
	}

	return spl_load_image_fat(spl_image, bootdev, block_dev, partition,
			CONFIG_SPL_FS_LOAD_KERNEL_NAME);
}
#else
int spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, int partition)
{
	return -ENOSYS;
}
#endif
