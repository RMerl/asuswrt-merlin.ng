/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the VInCo platform.
 *
 * Based on the settings for the SAMA5-EK board
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 * Copyright (C) 2015 Free Electrons
 *		      Gregory CLEMENT gregory.clement@free-electrons.com
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

/* The value in the common file is too far away for the VInCo platform */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		0xfc00c000
#define CONFIG_USART_ID			30

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfc06863c

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE           0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x4000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* SerialFlash */

#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI0
#define CONFIG_SPI_FLASH_STMICRO
#endif

/* MMC */

#ifdef CONFIG_CMD_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define ATMEL_BASE_MMCI			0xfc000000
#define CONFIG_SYS_MMC_CLK_OD		500000

/* For generating MMC partitions */

#endif

/* USB device */

/* Ethernet Hardware */
#define CONFIG_PHY_SMSC
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_MACB_SEARCH_PHY

#ifdef CONFIG_SPI_BOOT
/* bootstrap + u-boot + env + linux in serial flash */
/* Use our own mapping for the VInCo platform */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#define CONFIG_ENV_OFFSET       0x10000
#define CONFIG_ENV_SIZE         0x10000

/* Update the bootcommand according to our mapping for the VInCo platform */
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND  "mmc dev 0 0;" \
			    "mmc read ${loadaddr} ${k_offset} ${k_blksize};" \
			    "mmc read ${oftaddr} ${dtb_offset} ${dtb_blksize};" \
			    "bootz ${loadaddr} -  ${oftaddr}"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_start=0x20000\0" \
	"kernel_size=0x800000\0" \
	"mmcblksize=0x200\0" \
	"oftaddr=0x21000000\0" \
	"loadaddr=0x22000000\0" \
	"update_uboot=tftp ${loadaddr} u-boot.bin;sf probe 0;" \
	"sf erase 0x20000 0x4B000; sf write ${loadaddr} 0x20000 0x4B000\0" \
	"create_partition=setexpr dtb_start ${kernel_start} + 0x400000;" \
	"setexpr rootfs_start ${kernel_start} + ${kernel_size};" \
	"setenv partitions 'name=kernel,size=${kernel_size}," \
	"start=${kernel_start};name=rootfs,size=-';" \
	"gpt write mmc 0 ${partitions} \0"\
	"f2blk_size=setexpr fileblksize ${filesize} / ${mmcblksize};" \
	"setexpr fileblksize ${fileblksize} + 1\0" \
	"store_kernel=tftp ${loadaddr} zImage; run f2blk_size;" \
	"setexpr k_blksize ${fileblksize};" \
	"setexpr k_offset ${kernel_start} / ${mmcblksize};" \
	"mmc write ${fileaddr} ${k_offset} ${fileblksize}\0" \
	"store_dtb=tftp ${loadaddr} at91-vinco.dtb; run f2blk_size;" \
	"setexpr dtb_blksize ${fileblksize};" \
	"setexpr dtb_offset ${dtb_start} / ${mmcblksize};" \
	"mmc write ${fileaddr} ${dtb_offset} ${fileblksize}\0" \
	"store_rootfs=tftp ${loadaddr} vinco-gateway-image-vinco.ext4;" \
	"setexpr rootfs_offset ${rootfs_start} / ${mmcblksize};" \
	"mmc write ${fileaddr} ${rootfs_offset} ${fileblksize}\0" \
	"bootdelay=0\0"

#endif

#endif
