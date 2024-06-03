// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <spl.h>
#include <linux/compiler.h>
#include <errno.h>
#include <asm/u-boot.h>
#include <errno.h>
#include <mmc.h>
#include <image.h>

static int mmc_load_legacy(struct spl_image_info *spl_image, struct mmc *mmc,
			   ulong sector, struct image_header *header)
{
	u32 image_size_sectors;
	unsigned long count;
	int ret;

	ret = spl_parse_image_header(spl_image, header);
	if (ret)
		return ret;

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image->size + mmc->read_bl_len - 1) /
			     mmc->read_bl_len;

	/* Read the header too to avoid extra memcpy */
	count = blk_dread(mmc_get_blk_desc(mmc), sector, image_size_sectors,
			  (void *)(ulong)spl_image->load_addr);
	debug("read %x sectors to %lx\n", image_size_sectors,
	      spl_image->load_addr);
	if (count != image_size_sectors)
		return -EIO;

	return 0;
}

static ulong h_spl_load_read(struct spl_load_info *load, ulong sector,
			     ulong count, void *buf)
{
	struct mmc *mmc = load->dev;

	return blk_dread(mmc_get_blk_desc(mmc), sector, count, buf);
}

static __maybe_unused
int mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct mmc *mmc, unsigned long sector)
{
	unsigned long count;
	struct image_header *header;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	int ret = 0;

	header = spl_get_load_buffer(-sizeof(*header), bd->blksz);

	/* read image header to find the image size & load address */
	count = blk_dread(bd, sector, 1, header);
	debug("hdr read sector %lx, count=%lu\n", sector, count);
	if (count == 0) {
		ret = -EIO;
		goto end;
	}

	if (CONFIG_IS_ENABLED(LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT\n");
		load.dev = mmc;
		load.priv = NULL;
		load.filename = NULL;
		load.bl_len = mmc->read_bl_len;
		load.read = h_spl_load_read;
		ret = spl_load_simple_fit(spl_image, &load, sector, header);
	} else {
		ret = mmc_load_legacy(spl_image, mmc, sector, header);
	}

end:
	if (ret) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("mmc_load_image_raw_sector: mmc block read error\n");
#endif
		return -1;
	}

	return 0;
}

static int spl_mmc_get_device_index(u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		return 1;
	}

#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	printf("spl: unsupported mmc boot device.\n");
#endif

	return -ENODEV;
}

static int spl_mmc_find_device(struct mmc **mmcp, u32 boot_device)
{
#if CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
#endif
	int err, mmc_dev;

	mmc_dev = spl_mmc_get_device_index(boot_device);
	if (mmc_dev < 0)
		return mmc_dev;

	err = mmc_initialize(NULL);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: could not initialize mmc. error: %d\n", err);
#endif
		return err;
	}

#if CONFIG_IS_ENABLED(DM_MMC)
	err = uclass_get_device(UCLASS_MMC, mmc_dev, &dev);
	if (!err)
		*mmcp = mmc_get_mmc_dev(dev);
#else
	*mmcp = find_mmc_device(mmc_dev);
	err = *mmcp ? 0 : -ENODEV;
#endif
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, err);
#endif
		return err;
	}

	return 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
static int mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct mmc *mmc, int partition)
{
	disk_partition_t info;
	int err;

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	int type_part;
	/* Only support MBR so DOS_ENTRY_NUMBERS */
	for (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		err = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		if (err)
			continue;
		if (info.sys_ind == 
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			partition = type_part;
			break;
		}
	}
#endif

	err = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: partition error\n");
#endif
		return -1;
	}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
	return mmc_load_image_raw_sector(spl_image, mmc,
			info.start + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#else
	return mmc_load_image_raw_sector(spl_image, mmc, info.start);
#endif
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
static int mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct mmc *mmc)
{
	int ret;

#if defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *) CONFIG_SYS_SPL_ARGS_ADDR);
	if (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("mmc_load_image_raw_os: mmc block read error\n");
#endif
		return -1;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ret = mmc_load_image_raw_sector(spl_image, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	if (ret)
		return ret;

	if (spl_image->os != IH_OS_LINUX) {
		puts("Expected Linux image is not found. Trying to start U-boot\n");
		return -ENOENT;
	}

	return 0;
}
#else
int spl_start_uboot(void)
{
	return 1;
}
static int mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct mmc *mmc)
{
	return -ENOSYS;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT_PARTITION
static int spl_mmc_do_fs_boot(struct spl_image_info *spl_image, struct mmc *mmc)
{
	int err = -ENOSYS;

#ifdef CONFIG_SPL_FS_FAT
	if (!spl_start_uboot()) {
		err = spl_load_image_fat_os(spl_image, mmc_get_blk_desc(mmc),
			CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
		if (!err)
			return err;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	err = spl_load_image_fat(spl_image, mmc_get_blk_desc(mmc),
				 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
				 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	if (!err)
		return err;
#endif
#endif
#ifdef CONFIG_SPL_FS_EXT4
	if (!spl_start_uboot()) {
		err = spl_load_image_ext_os(spl_image, mmc_get_blk_desc(mmc),
			CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
		if (!err)
			return err;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	err = spl_load_image_ext(spl_image, mmc_get_blk_desc(mmc),
				 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
				 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	if (!err)
		return err;
#endif
#endif

#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	err = -ENOENT;
#endif

	return err;
}
#else
static int spl_mmc_do_fs_boot(struct spl_image_info *spl_image, struct mmc *mmc)
{
	return -ENOSYS;
}
#endif

u32 __weak spl_boot_mode(const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#elif defined(CONFIG_SUPPORT_EMMC_BOOT)
	return MMCSD_MODE_EMMCBOOT;
#else
	return MMCSD_MODE_RAW;
#endif
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
__weak
int spl_boot_partition(const u32 boot_device)
{
	return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
}
#endif

int spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	struct mmc *mmc = NULL;
	u32 boot_mode;
	int err = 0;
	__maybe_unused int part;

	err = spl_mmc_find_device(&mmc, bootdev->boot_device);
	if (err)
		return err;

	err = mmc_init(mmc);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: mmc init failed with error: %d\n", err);
#endif
		return err;
	}

	boot_mode = spl_boot_mode(bootdev->boot_device);
	err = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
			/*
			 * We need to check what the partition is configured to.
			 * 1 and 2 match up to boot0 / boot1 and 7 is user data
			 * which is the first physical partition (0).
			 */
			part = (mmc->part_config >> 3) & PART_ACCESS_MASK;

			if (part == 7)
				part = 0;

			if (CONFIG_IS_ENABLED(MMC_TINY))
				err = mmc_switch_part(mmc, part);
			else
				err = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

			if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
				puts("spl: mmc partition switch failed\n");
#endif
				return err;
			}
			/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		if (!spl_start_uboot()) {
			err = mmc_load_image_raw_os(spl_image, mmc);
			if (!err)
				return err;
		}
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
		err = spl_boot_partition(bootdev->boot_device);
		if (!err)
			return err;

		err = mmc_load_image_raw_partition(spl_image, mmc, err);
		if (!err)
			return err;
#endif
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		err = mmc_load_image_raw_sector(spl_image, mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
		if (!err)
			return err;
#endif
		/* If RAW mode fails, try FS mode. */
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		err = spl_mmc_do_fs_boot(spl_image, mmc);
		if (!err)
			return err;

		break;
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	default:
		puts("spl: mmc: wrong boot mode\n");
#endif
	}

	return err;
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);
