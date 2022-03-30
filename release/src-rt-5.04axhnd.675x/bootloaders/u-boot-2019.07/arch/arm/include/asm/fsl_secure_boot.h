/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_CHAIN_OF_TRUST
#define CONFIG_FSL_SEC_MON

#ifdef CONFIG_SPL_BUILD
/*
 * Define the key hash for U-Boot here if public/private key pair used to
 * sign U-boot are different from the SRK hash put in the fuse
 * Example of defining KEY_HASH is
 * #define CONFIG_SPL_UBOOT_KEY_HASH \
 *      "41066b564c6ffcef40ccbc1e0a5d0d519604000c785d97bbefd25e4d288d1c8b"
 * else leave it defined as NULL
 */

#define CONFIG_SPL_UBOOT_KEY_HASH	NULL
#endif /* ifdef CONFIG_SPL_BUILD */

#define CONFIG_KEY_REVOCATION

#if defined(CONFIG_FSL_LAYERSCAPE)
/*
 * For fsl layerscape based platforms, ESBC image Address in Header
 * is 64 bit.
 */
#define CONFIG_ESBC_ADDR_64BIT
#endif

#ifndef CONFIG_SPL_BUILD
#ifndef CONFIG_SYS_RAMBOOT
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC.
 * The feature is only applicable in case of NOR boot and is
 * not applicable in case of RAMBOOT (NAND, SD, SPI).
 * For LS, this feature is available for all device if IE Table
 * is copied to XIP memory
 * Also, for LS, ISBC doesn't verify this table.
 */
#define CONFIG_FSL_ISBC_KEY_EXT

#endif

#ifdef CONFIG_ARCH_LS2080A
#define CONFIG_EXTRA_ENV \
	"setenv fdt_high 0xa0000000;"	\
	"setenv initrd_high 0xcfffffff;"	\
	"setenv hwconfig \'fsl_ddr:ctlr_intlv=null,bank_intlv=null\';"
#else
#define CONFIG_EXTRA_ENV \
	"setenv fdt_high 0xffffffff;"	\
	"setenv initrd_high 0xffffffff;"	\
	"setenv hwconfig \'fsl_ddr:ctlr_intlv=null,bank_intlv=null\';"
#endif

/* Copying Bootscript and Header to DDR from NOR for LS2 and for rest, from
 * Non-XIP Memory (Nand/SD)*/
#if defined(CONFIG_SYS_RAMBOOT) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_SD_BOOT) || defined(CONFIG_NAND_BOOT)
#define CONFIG_BOOTSCRIPT_COPY_RAM
#endif
/* The address needs to be modified according to NOR, NAND, SD and
 * DDR memory map
 */
#ifdef CONFIG_FSL_LSCH3
#ifdef CONFIG_QSPI_BOOT
#define CONFIG_BS_ADDR_DEVICE		0x20600000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x20640000
#else /* NOR BOOT */
#define CONFIG_BS_ADDR_DEVICE		0x580600000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x580640000
#endif /*ifdef CONFIG_QSPI_BOOT */
#define CONFIG_BS_SIZE			0x00001000
#define CONFIG_BS_HDR_SIZE		0x00004000
#define CONFIG_BS_ADDR_RAM		0xa0600000
#define CONFIG_BS_HDR_ADDR_RAM		0xa0640000
#else
#ifdef CONFIG_SD_BOOT
/* For SD boot address and size are assigned in terms of sector
 * offset and no. of sectors respectively.
 */
#define CONFIG_BS_ADDR_DEVICE		0x00003000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x00003200
#define CONFIG_BS_SIZE			0x00000008
#define CONFIG_BS_HDR_SIZE		0x00000010
#elif defined(CONFIG_NAND_BOOT)
#define CONFIG_BS_ADDR_DEVICE		0x00600000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x00640000
#define CONFIG_BS_SIZE			0x00001000
#define CONFIG_BS_HDR_SIZE		0x00002000
#elif defined(CONFIG_QSPI_BOOT)
#define CONFIG_BS_ADDR_DEVICE		0x40600000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x40640000
#define CONFIG_BS_SIZE			0x00001000
#define CONFIG_BS_HDR_SIZE		0x00002000
#else /* Default NOR Boot */
#define CONFIG_BS_ADDR_DEVICE		0x60600000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x60640000
#define CONFIG_BS_SIZE			0x00001000
#define CONFIG_BS_HDR_SIZE		0x00002000
#endif
#define CONFIG_BS_ADDR_RAM		0x81000000
#define CONFIG_BS_HDR_ADDR_RAM		0x81020000
#endif

#ifdef CONFIG_BOOTSCRIPT_COPY_RAM
#define CONFIG_BOOTSCRIPT_ADDR		CONFIG_BS_ADDR_RAM
#define CONFIG_BOOTSCRIPT_HDR_ADDR	CONFIG_BS_HDR_ADDR_RAM
#else
#define CONFIG_BOOTSCRIPT_HDR_ADDR	CONFIG_BS_HDR_ADDR_DEVICE
/* BOOTSCRIPT_ADDR is not required */
#endif

#ifdef CONFIG_FSL_LS_PPA
/* Define the key hash here if SRK used for signing PPA image is
 * different from SRK hash put in SFP used for U-Boot.
 * Example
 * #define PPA_KEY_HASH \
 *	"41066b564c6ffcef40ccbc1e0a5d0d519604000c785d97bbefd25e4d288d1c8b"
 */
#define PPA_KEY_HASH		NULL
#endif /* ifdef CONFIG_FSL_LS_PPA */

#include <config_fsl_chain_trust.h>
#endif /* #ifndef CONFIG_SPL_BUILD */
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
