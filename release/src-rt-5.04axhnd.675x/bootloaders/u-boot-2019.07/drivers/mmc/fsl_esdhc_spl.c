// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <mmc.h>
#include <malloc.h>

/*
 * The environment variables are written to just after the u-boot image
 * on SDCard, so we must read the MBR to get the start address and code
 * length of the u-boot image, then calculate the address of the env.
 */
#define ESDHC_BOOT_IMAGE_SIZE	0x48
#define ESDHC_BOOT_IMAGE_ADDR	0x50
#define MBRDBR_BOOT_SIG_55	0x1fe
#define MBRDBR_BOOT_SIG_AA	0x1ff
#define CONFIG_CFG_DATA_SECTOR	0


void mmc_spl_load_image(uint32_t offs, unsigned int size, void *vdst)
{
	uint blk_start, blk_cnt, err;

	struct mmc *mmc = find_mmc_device(0);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return;
	}

	blk_start = ALIGN(offs, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	err = mmc->block_dev.block_read(&mmc->block_dev, blk_start, blk_cnt,
					vdst);
	if (err != blk_cnt) {
		puts("spl: mmc read failed!!\n");
		hang();
	}
}

/*
 * The main entry for mmc booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from mmc into SDRAM and starts it from there.
 */

void __noreturn mmc_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);
	uint blk_start, blk_cnt, err;
#ifndef CONFIG_FSL_CORENET
	uchar *tmp_buf;
	u32 blklen;
	uchar val;
	uint i, byte_num;
#endif
	u32 offset, code_len;
	struct mmc *mmc;

	mmc = find_mmc_device(0);
	if (!mmc) {
		puts("spl: mmc device not found!!\n");
		hang();
	}

#ifdef CONFIG_FSL_CORENET
	offset = CONFIG_SYS_MMC_U_BOOT_OFFS;
	code_len = CONFIG_SYS_MMC_U_BOOT_SIZE;
#else
	blklen = mmc->read_bl_len;
	tmp_buf = malloc(blklen);
	if (!tmp_buf) {
		puts("spl: malloc memory failed!!\n");
		hang();
	}
	memset(tmp_buf, 0, blklen);

	/*
	* Read source addr from sd card
	*/
	err = mmc->block_dev.block_read(&mmc->block_dev,
					CONFIG_CFG_DATA_SECTOR, 1, tmp_buf);
	if (err != 1) {
		puts("spl: mmc read failed!!\n");
		free(tmp_buf);
		hang();
	}

	val = *(tmp_buf + MBRDBR_BOOT_SIG_55);
	if (0x55 != val) {
		puts("spl: mmc signature is not valid!!\n");
		free(tmp_buf);
		hang();
	}
	val = *(tmp_buf + MBRDBR_BOOT_SIG_AA);
	if (0xAA != val) {
		puts("spl: mmc signature is not valid!!\n");
		free(tmp_buf);
		hang();
	}

	byte_num = 4;
	offset = 0;
	for (i = 0; i < byte_num; i++) {
		val = *(tmp_buf + ESDHC_BOOT_IMAGE_ADDR + i);
		offset = (offset << 8) + val;
	}
	offset += CONFIG_SYS_MMC_U_BOOT_OFFS;
	/* Get the code size from offset 0x48 */
	byte_num = 4;
	code_len = 0;
	for (i = 0; i < byte_num; i++) {
		val = *(tmp_buf + ESDHC_BOOT_IMAGE_SIZE + i);
		code_len = (code_len << 8) + val;
	}
	code_len -= CONFIG_SYS_MMC_U_BOOT_OFFS;
	/*
	* Load U-Boot image from mmc into RAM
	*/
#endif
	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(code_len, mmc->read_bl_len) / mmc->read_bl_len;
	err = mmc->block_dev.block_read(&mmc->block_dev, blk_start, blk_cnt,
					(uchar *)CONFIG_SYS_MMC_U_BOOT_DST);
	if (err != blk_cnt) {
		puts("spl: mmc read failed!!\n");
#ifndef CONFIG_FSL_CORENET
		free(tmp_buf);
#endif
		hang();
	}

	/*
	* Clean d-cache and invalidate i-cache, to
	* make sure that no stale data is executed.
	*/
	flush_cache(CONFIG_SYS_MMC_U_BOOT_DST, CONFIG_SYS_MMC_U_BOOT_SIZE);

	/*
	* Jump to U-Boot image
	*/
	uboot = (void *)CONFIG_SYS_MMC_U_BOOT_START;
	(*uboot)();
}
