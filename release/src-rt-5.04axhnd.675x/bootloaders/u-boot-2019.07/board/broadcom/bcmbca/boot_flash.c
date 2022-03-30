/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */
#include <common.h>
#include <nand.h>
#include <mmc.h>
#include <blk.h>
#include <spl.h>
#include "boot_flash.h"

#ifdef CONFIG_MMC
#define MMC_DEV_NUM 0
#define MMC_USERDATA_PART 0
#endif

/* for emmc and spi flash*/
void* flash_dev;

#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT)
static int mmc_boot_part = 0;
#endif

int boot_flash_init(void)
{
	flash_dev = NULL;
#ifdef CONFIG_NAND
	nand_init();
#endif

#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT)
	struct mmc *mmc;
	int ret;
	ret = mmc_initialize(NULL);
	if(ret)	{
		printf("MMC Initialization Failed!!\n");
		hang();
	}

	mmc = find_mmc_device(MMC_DEV_NUM);
	mmc_init(mmc);

	/* Get active boot partition */
	mmc_boot_part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	/* Default to first boot partition if nothing is set */
	if( !mmc_boot_part )
		mmc_boot_part = 1;

	debug("Booted from eMMC Physical HW partition:%d part_conf:0x%02x\n", mmc_boot_part, mmc->part_config);
#endif

#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT)
	struct udevice *dev;
	struct mtd_info *mtd;
	int ret = -1;

	ret = uclass_get_device_by_driver(UCLASS_SPI_FLASH, DM_GET_DRIVER(spi_flash_std), &dev);
	if (ret){
		debug("SPI NOR failed to initialize. (error %d)\n", ret);
		hang();
	}
	mtd = get_mtd_device_nm(SPIFLASH_MTDNAME);
	if (IS_ERR_OR_NULL(mtd)){
		printf("%s:MTD device %s not found, ret %ld\n",__func__, SPIFLASH_MTDNAME,
		   PTR_ERR(mtd));
		hang();
	}
	else
		flash_dev = mtd;
#endif
	return 0;
}

/* return number of bytes read if success, negative value if fail */
int read_boot_device(uint32_t address, void *data, int len)
{
	int ret;
#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT)
	size_t retlen;	
	debug("read_boot_device:address=%d,len=%d\n",address,len);
	if ( flash_dev != NULL){
		ret = mtd_read(flash_dev, address, len, &retlen, data);
		if ( ret == 0)
			ret = retlen;
		else{
			debug("%s: Failed to read from SPI flash (err=%d)\n", __func__, ret);
			ret = -1;
		}
	}
	else
		ret = -1;
#endif
	
#ifdef CONFIG_NAND
	/* unfortunately nand spl api does not return the length.. */
	ret = nand_spl_load_image(address, len, data);
	if (ret == 0)
		ret = len;
	else
		ret = -1;
#endif

#if defined(CONFIG_MMC) && defined(CONFIG_SUPPORT_EMMC_BOOT)
	blk_select_hwpart_devnum(IF_TYPE_MMC, MMC_DEV_NUM, mmc_boot_part);
	/* Unfortunately mmc_spl_load_image has no return code */
	mmc_spl_load_image(address, len, data);
	ret = len;
	blk_select_hwpart_devnum(IF_TYPE_MMC, MMC_DEV_NUM, MMC_USERDATA_PART);
#endif

#if defined(CONFIG_SPL_XIP_SUPPORT)
	memcpy((uint8_t*)data, (uint8_t*)(address+NOR_XIP_BASE_ADDR), len);
#endif
	return ret;
}
