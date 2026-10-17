// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY LOGC_BOOT

#include <common.h>
#include <binman_sym.h>
#include <bootstage.h>
#include <dm.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spl.h>
#include <spi_flash.h>
#include <asm/fast_spi.h>
#include <asm/spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/iomap.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

/* This reads the next phase from mapped SPI flash */
static int rom_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev)
{
	ulong spl_pos = spl_get_image_pos();
	ulong spl_size = spl_get_image_size();
	struct udevice *dev;
	ulong map_base;
	size_t map_size;
	uint offset;
	int ret;

	spl_image->size = CONFIG_SYS_MONITOR_LEN;  /* We don't know SPL size */
	spl_image->entry_point = spl_get_image_text_base();
	spl_image->load_addr = spl_image->entry_point;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	log_debug("Reading from mapped SPI %lx, size %lx\n", spl_pos, spl_size);

	if (CONFIG_IS_ENABLED(SPI_FLASH_SUPPORT)) {
		ret = uclass_find_first_device(UCLASS_SPI_FLASH, &dev);
		if (ret)
			return log_msg_ret("spi_flash", ret);
		if (!dev)
			return log_msg_ret("spi_flash dev", -ENODEV);
		ret = dm_spi_get_mmap(dev, &map_base, &map_size, &offset);
		if (ret)
			return log_msg_ret("mmap", ret);
	} else {
		ret = fast_spi_get_bios_mmap(PCH_DEV_SPI, &map_base, &map_size,
					     &offset);
		if (ret)
			return ret;
	}
	spl_pos += map_base & ~0xff000000;
	log_debug(", base %lx, pos %lx, load %lx\n", map_base, spl_pos,
		  spl_image->load_addr);
	bootstage_start(BOOTSTAGE_ID_ACCUM_MMAP_SPI, "mmap_spi");
	memcpy((void *)spl_image->load_addr, (void *)spl_pos, spl_size);
	cpu_flush_l1d_to_l2();
	bootstage_accum(BOOTSTAGE_ID_ACCUM_MMAP_SPI);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("Mapped SPI", 2, BOOT_DEVICE_SPI_MMAP, rom_load_image);

#if CONFIG_IS_ENABLED(SPI_FLASH_SUPPORT)

static int apl_flash_std_read(struct udevice *dev, u32 offset, size_t len,
			      void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct mtd_info *mtd = &flash->mtd;
	size_t retlen;

	return log_ret(mtd->_read(mtd, offset, len, &retlen, buf));
}

static int apl_flash_probe(struct udevice *dev)
{
	return spi_flash_std_probe(dev);
}

static const struct dm_spi_flash_ops apl_flash_ops = {
	.read		= apl_flash_std_read,
};

static const struct udevice_id apl_flash_ids[] = {
	{ .compatible = "jedec,spi-nor" },
	{ }
};

U_BOOT_DRIVER(winbond_w25q128fw) = {
	.name		= "winbond_w25q128fw",
	.id		= UCLASS_SPI_FLASH,
	.of_match	= apl_flash_ids,
	.probe		= apl_flash_probe,
	.priv_auto	= sizeof(struct spi_nor),
	.ops		= &apl_flash_ops,
};

/* This uses a SPI flash device to read the next phase */
static int spl_fast_spi_load_image(struct spl_image_info *spl_image,
				   struct spl_boot_device *bootdev)
{
	ulong spl_pos = spl_get_image_pos();
	ulong spl_size = spl_get_image_size();
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_SPI_FLASH, &dev);
	if (ret)
		return ret;

	spl_image->size = CONFIG_SYS_MONITOR_LEN;  /* We don't know SPL size */
	spl_image->entry_point = spl_phase() == PHASE_TPL ?
		CONFIG_SPL_TEXT_BASE : CONFIG_TEXT_BASE;
	spl_image->load_addr = spl_image->entry_point;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
	spl_pos &= ~0xff000000;
	log_debug("Reading from flash %lx, size %lx\n", spl_pos, spl_size);
	ret = spi_flash_read_dm(dev, spl_pos, spl_size,
				(void *)spl_image->load_addr);
	cpu_flush_l1d_to_l2();
	if (ret)
		return ret;

	return 0;
}
SPL_LOAD_IMAGE_METHOD("Fast SPI", 1, BOOT_DEVICE_FAST_SPI,
		      spl_fast_spi_load_image);

void board_boot_order(u32 *spl_boot_list)
{
	bool use_spi_flash = IS_ENABLED(CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH);

	if (use_spi_flash) {
		spl_boot_list[0] = BOOT_DEVICE_FAST_SPI;
		spl_boot_list[1] = BOOT_DEVICE_SPI_MMAP;
	} else {
		spl_boot_list[0] = BOOT_DEVICE_SPI_MMAP;
		spl_boot_list[1] = BOOT_DEVICE_FAST_SPI;
	}
}

#else

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_SPI_MMAP;
}
#endif
