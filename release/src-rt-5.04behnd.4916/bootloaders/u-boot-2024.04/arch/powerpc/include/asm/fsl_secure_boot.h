/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H
#include <asm/config_mpc85xx.h>

#ifdef CONFIG_NXP_ESBC
#if defined(CONFIG_FSL_CORENET)
#define CFG_SYS_PBI_FLASH_BASE		0xc0000000
#else
#define CFG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CFG_SYS_PBI_FLASH_WINDOW		0xcff80000

#if defined(CONFIG_TARGET_T2080QDS) || \
	defined(CONFIG_TARGET_T2080RDB) || \
	defined(CONFIG_TARGET_T1042D4RDB) || \
	defined(CONFIG_ARCH_T1024)
#undef CFG_SYS_INIT_L3_ADDR
#define CFG_SYS_INIT_L3_ADDR			0xbff00000
#endif

#if defined(CONFIG_RAMBOOT_PBL)
#undef CFG_SYS_INIT_L3_ADDR
#ifdef CFG_SYS_INIT_L3_VADDR
#define CFG_SYS_INIT_L3_ADDR	\
			(CFG_SYS_INIT_L3_VADDR & ~0xFFF00000) | \
					0xbff00000
#else
#define CFG_SYS_INIT_L3_ADDR		0xbff00000
#endif
#endif
#endif /* #ifdef CONFIG_NXP_ESBC */

#ifdef CONFIG_CHAIN_OF_TRUST
#ifdef CONFIG_SPL_BUILD
/*
 * PPAACT and SPAACT table for PAMU must be placed on DDR after DDR init
 * due to space crunch on CPC and thus malloc will not work.
 */
#define CFG_SPL_PPAACT_ADDR		0x2e000000
#define CFG_SPL_SPAACT_ADDR		0x2f000000
#define CFG_SPL_JR0_LIODN_S		454
#define CFG_SPL_JR0_LIODN_NS		458
#endif /* ifdef CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
#include <config_fsl_chain_trust.h>
#endif /* #ifndef CONFIG_SPL_BUILD */
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
