/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the Freescale i.MX31 PDK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */
#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_MACH_TYPE	MACH_TYPE_MX31_3DS

#define CONFIG_SPL_TARGET	"u-boot-with-spl.bin"
#define CONFIG_SPL_MAX_SIZE	2048

#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(2*CONFIG_ENV_SIZE + 2 * 128 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE

/* PMIC Controller */
#define CONFIG_POWER
#define CONFIG_POWER_SPI
#define CONFIG_POWER_FSL
#define CONFIG_FSL_PMIC_BUS	1
#define CONFIG_FSL_PMIC_CS	2
#define CONFIG_FSL_PMIC_CLK	1000000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32
#define CONFIG_RTC_MC13XXX

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"bootargs_base=setenv bootargs console=ttymxc0,115200\0"	\
	"bootargs_nfs=setenv bootargs $(bootargs) root=/dev/nfs "	\
		"ip=dhcp nfsroot=$(serverip):$(nfsrootfs),v3,tcp\0"	\
	"bootcmd=run bootcmd_net\0"					\
	"bootcmd_net=run bootargs_base bootargs_mtd bootargs_nfs; "	\
		"tftpboot 0x81000000 uImage-mx31; bootm\0"		\
	"prg_uboot=tftpboot 0x81000000 u-boot-with-spl.bin; "		\
		"nand erase 0x0 0x40000; "				\
		"nand write 0x81000000 0x0 0x40000\0"

/*
 * Miscellaneous configurable options
 */

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x80010000

/* default load address */
#define CONFIG_SYS_LOAD_ADDR		0x81000000

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		CSD0_BASE
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE)

/*
 * environment organization
 */
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_OFFSET_REDUND	0x60000
#define CONFIG_ENV_SIZE			(128 * 1024)

/*
 * NAND driver
 */
#define CONFIG_MXC_NAND_REGS_BASE      NFC_BASE_ADDR
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           NFC_BASE_ADDR
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_LARGEPAGE

/* NAND configuration for the NAND_SPL */

/* Start copying real U-Boot from the second page */
#define CONFIG_SYS_NAND_U_BOOT_OFFS	CONFIG_SPL_PAD_TO
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x3f800
/* Load U-Boot to this address */
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST

#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_SIZE		(256 * 1024 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0

/* Configuration of lowlevel_init.S (clocks and SDRAM) */
#define CCM_CCMR_SETUP		0x074B0BF5
#define CCM_PDR0_SETUP_532MHZ	(PDR0_CSI_PODF(0x3f) | PDR0_CSI_PRDF(7) | \
				 PDR0_PER_PODF(7) | PDR0_HSP_PODF(3) |    \
				 PDR0_NFC_PODF(5) | PDR0_IPG_PODF(1) |    \
				 PDR0_MAX_PODF(3) | PDR0_MCU_PODF(0))
#define CCM_MPCTL_SETUP_532MHZ	(PLL_PD(0) | PLL_MFD(51) | PLL_MFI(10) |  \
				 PLL_MFN(12))

#define ESDMISC_MDDR_SETUP	0x00000004
#define ESDMISC_MDDR_RESET_DL	0x0000000c
#define ESDCFG0_MDDR_SETUP	0x006ac73a

#define ESDCTL_ROW_COL		(ESDCTL_SDE | ESDCTL_ROW(2) | ESDCTL_COL(2))
#define ESDCTL_SETTINGS		(ESDCTL_ROW_COL | ESDCTL_SREFR(3) | \
				 ESDCTL_DSIZ(2) | ESDCTL_BL(1))
#define ESDCTL_PRECHARGE	(ESDCTL_ROW_COL | ESDCTL_CMD_PRECHARGE)
#define ESDCTL_AUTOREFRESH	(ESDCTL_ROW_COL | ESDCTL_CMD_AUTOREFRESH)
#define ESDCTL_LOADMODEREG	(ESDCTL_ROW_COL | ESDCTL_CMD_LOADMODEREG)
#define ESDCTL_RW		ESDCTL_SETTINGS

#endif /* __CONFIG_H */
