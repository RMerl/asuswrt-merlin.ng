/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ti816x_evm.h
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 */

#ifndef __CONFIG_TI816X_EVM_H
#define __CONFIG_TI816X_EVM_H

#include <configs/ti_armv7_omap.h>
#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_MACH_TYPE		MACH_TYPE_TI8168EVM

#define CONFIG_EXTRA_ENV_SETTINGS	\
	DEFAULT_LINUX_BOOT_ENV \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \

#define CONFIG_BOOTCOMMAND			\
	"mmc rescan;"				\
	"fatload mmc 0 ${loadaddr} uImage;"	\
	"bootm ${loadaddr}"			\

/* Clock Defines */
#define V_OSCK          24000000    /* Clock output from T2 */
#define V_SCLK          (V_OSCK >> 1)

#define CONFIG_CMD_ASKENV

#define CONFIG_MAX_RAM_BANK_SIZE	(2048 << 20)	/* 2048MB */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/**
 * Platform/Board specific defs
 */
#define CONFIG_SYS_CLK_FREQ     27000000
#define CONFIG_SYS_TIMERBASE    0x4802E000
#define CONFIG_SYS_PTV          2   /* Divisor: 2^(PTV+1) => 8 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (48000000)
#define CONFIG_SYS_NS16550_COM1     0x48024000  /* Base EVM has UART2 */

/* allow overwriting serial config and ethaddr */
#define CONFIG_ENV_OVERWRITE


/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* NAND: SPL related configs */

/* NAND: device related configs */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
/* NAND: driver related configs */
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x000c0000
#define CONFIG_ENV_OFFSET		0x001c0000
#define CONFIG_ENV_OFFSET_REDUND	0x001e0000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE

/* SPL */
/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10

/* Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * Disable MMC DM for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_MMC
#undef CONFIG_TIMER
#endif
#endif
