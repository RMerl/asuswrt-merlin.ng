/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H
#include <asm/config_mpc85xx.h>

#ifdef CONFIG_SECURE_BOOT
#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc0000000
#elif defined(CONFIG_TARGET_BSC9132QDS)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc8000000
#elif defined(CONFIG_TARGET_C29XPCIE)
#define CONFIG_SYS_PBI_FLASH_BASE		0xcc000000
#else
#define CONFIG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CONFIG_SYS_PBI_FLASH_WINDOW		0xcff80000

#if defined(CONFIG_TARGET_B4860QDS) || \
	defined(CONFIG_TARGET_B4420QDS) || \
	defined(CONFIG_TARGET_T4160QDS) || \
	defined(CONFIG_TARGET_T4240QDS) || \
	defined(CONFIG_TARGET_T2080QDS) || \
	defined(CONFIG_TARGET_T2080RDB) || \
	defined(CONFIG_TARGET_T1040QDS) || \
	defined(CONFIG_TARGET_T1040RDB) || \
	defined(CONFIG_TARGET_T1040D4RDB) || \
	defined(CONFIG_TARGET_T1042RDB) || \
	defined(CONFIG_TARGET_T1042D4RDB) || \
	defined(CONFIG_TARGET_T1042RDB_PI) || \
	defined(CONFIG_ARCH_T1023) || \
	defined(CONFIG_ARCH_T1024)
#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_CPC_REINIT_F
#endif
#define CONFIG_KEY_REVOCATION
#undef CONFIG_SYS_INIT_L3_ADDR
#define CONFIG_SYS_INIT_L3_ADDR			0xbff00000
#endif

#if defined(CONFIG_RAMBOOT_PBL)
#undef CONFIG_SYS_INIT_L3_ADDR
#ifdef CONFIG_SYS_INIT_L3_VADDR
#define CONFIG_SYS_INIT_L3_ADDR	\
			(CONFIG_SYS_INIT_L3_VADDR & ~0xFFF00000) | \
					0xbff00000
#else
#define CONFIG_SYS_INIT_L3_ADDR		0xbff00000
#endif
#endif

#if defined(CONFIG_TARGET_C29XPCIE)
#define CONFIG_KEY_REVOCATION
#endif

#if defined(CONFIG_ARCH_P3041)	||	\
	defined(CONFIG_ARCH_P4080) ||	\
	defined(CONFIG_ARCH_P5020) ||	\
	defined(CONFIG_ARCH_P5040) ||	\
	defined(CONFIG_ARCH_P2041)
	#define	CONFIG_FSL_TRUST_ARCH_v1
#endif

#if defined(CONFIG_FSL_CORENET) && !defined(CONFIG_SYS_RAMBOOT)
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC.
 * The feature is only applicable in case of NOR boot and is
 * not applicable in case of RAMBOOT (NAND, SD, SPI).
 */
#define CONFIG_FSL_ISBC_KEY_EXT
#endif
#endif /* #ifdef CONFIG_SECURE_BOOT */

#ifdef CONFIG_CHAIN_OF_TRUST
#ifdef CONFIG_SPL_BUILD
/*
 * PPAACT and SPAACT table for PAMU must be placed on DDR after DDR init
 * due to space crunch on CPC and thus malloc will not work.
 */
#define CONFIG_SPL_PPAACT_ADDR		0x2e000000
#define CONFIG_SPL_SPAACT_ADDR		0x2f000000
#define CONFIG_SPL_JR0_LIODN_S		454
#define CONFIG_SPL_JR0_LIODN_NS		458
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

#define CONFIG_FSL_SEC_MON

#ifndef CONFIG_SPL_BUILD
/*
 * fsl_setenv_chain_of_trust() must be called from
 * board_late_init()
 */

/* If Boot Script is not on NOR and is required to be copied on RAM */
#ifdef CONFIG_BOOTSCRIPT_COPY_RAM
#define CONFIG_BS_HDR_ADDR_RAM		0x00010000
#define CONFIG_BS_HDR_ADDR_DEVICE	0x00800000
#define CONFIG_BS_HDR_SIZE		0x00002000
#define CONFIG_BS_ADDR_RAM		0x00012000
#define CONFIG_BS_ADDR_DEVICE		0x00802000
#define CONFIG_BS_SIZE			0x00001000

#define CONFIG_BOOTSCRIPT_HDR_ADDR	CONFIG_BS_HDR_ADDR_RAM
#else

/* The bootscript header address is different for B4860 because the NOR
 * mapping is different on B4 due to reduced NOR size.
 */
#if defined(CONFIG_TARGET_B4860QDS) || defined(CONFIG_TARGET_B4420QDS)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xecc00000
#elif defined(CONFIG_FSL_CORENET)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xe8e00000
#elif defined(CONFIG_TARGET_BSC9132QDS)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0x88020000
#elif defined(CONFIG_TARGET_C29XPCIE)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xec020000
#else
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xee020000
#endif

#endif /* #ifdef CONFIG_BOOTSCRIPT_COPY_RAM */

#include <config_fsl_chain_trust.h>
#endif /* #ifndef CONFIG_SPL_BUILD */
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
