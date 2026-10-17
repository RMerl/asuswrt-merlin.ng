/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Texas Instruments Incorporated - https://www.ti.com/
 *
 * Based on davinci_dvevm.h. Original Copyrights follow:
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Board
 */

/*
 * SoC Configuration
 */
#define CFG_SYS_EXCEPTION_VECTORS_HIGH
#define CFG_SYS_OSCIN_FREQ		24000000
#define CFG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CFG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)

#ifdef CONFIG_MTD_NOR_FLASH
#define CFG_SYS_DV_NOR_BOOT_CFG	(0x11)
#endif

/*
 * Memory Info
 */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CFG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/
/* memtest start addr */

/* memtest will be run on 16MB */

#define CFG_SYS_DA850_SYSCFG_SUSPSRC (	\
	DAVINCI_SYSCFG_SUSPSRC_TIMER0 |		\
	DAVINCI_SYSCFG_SUSPSRC_SPI1 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART2 |		\
	DAVINCI_SYSCFG_SUSPSRC_EMAC |		\
	DAVINCI_SYSCFG_SUSPSRC_I2C)

/*
 * PLL configuration
 */

#define CFG_SYS_DA850_PLL0_PLLM     24
#define CFG_SYS_DA850_PLL1_PLLM     21

/*
 * DDR2 memory configuration
 */
#define CFG_SYS_DA850_DDR2_DDRPHYCR (DV_DDR_PHY_PWRDNEN | \
					DV_DDR_PHY_EXT_STRBEN | \
					(0x4 << DV_DDR_PHY_RD_LATENCY_SHIFT))

#define CFG_SYS_DA850_DDR2_SDBCR (		\
	(1 << DV_DDR_SDCR_MSDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDREN_SHIFT) |	\
	(1 << DV_DDR_SDCR_SDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT) |	\
	(0x3 << DV_DDR_SDCR_CL_SHIFT) |		\
	(0x2 << DV_DDR_SDCR_IBANK_SHIFT) |	\
	(0x2 << DV_DDR_SDCR_PAGESIZE_SHIFT))

/* SDBCR2 is only used if IBANK_POS bit in SDBCR is set */
#define CFG_SYS_DA850_DDR2_SDBCR2 0

#define CFG_SYS_DA850_DDR2_SDTIMR (		\
	(14 << DV_DDR_SDTMR1_RFC_SHIFT) |	\
	(2 << DV_DDR_SDTMR1_RP_SHIFT) |		\
	(2 << DV_DDR_SDTMR1_RCD_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_WR_SHIFT) |		\
	(5 << DV_DDR_SDTMR1_RAS_SHIFT) |	\
	(8 << DV_DDR_SDTMR1_RC_SHIFT) |		\
	(1 << DV_DDR_SDTMR1_RRD_SHIFT) |	\
	(0 << DV_DDR_SDTMR1_WTR_SHIFT))

#define CFG_SYS_DA850_DDR2_SDTIMR2 (		\
	(7 << DV_DDR_SDTMR2_RASMAX_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_XP_SHIFT) |		\
	(0 << DV_DDR_SDTMR2_ODT_SHIFT) |	\
	(17 << DV_DDR_SDTMR2_XSNR_SHIFT) |	\
	(199 << DV_DDR_SDTMR2_XSRD_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_RTP_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_CKE_SHIFT))

#define CFG_SYS_DA850_DDR2_SDRCR    0x00000494
#define CFG_SYS_DA850_DDR2_PBBPR    0x30

/*
 * Serial Driver info
 */
#define CFG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)

#define CFG_SYS_SPI_CLK		clk_get(DAVINCI_SPI1_CLKID)

/*
 * I2C Configuration
 */
#define CFG_SYS_I2C_EXPANDER_ADDR   0x20

/*
 * Flash & Environment
 */
#ifdef CONFIG_MTD_RAW_NAND
#define CFG_SYS_NAND_CS		3
#define CFG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CFG_SYS_NAND_MASK_CLE		0x10
#define CFG_SYS_NAND_MASK_ALE		0x8
#define CFG_SYS_NAND_U_BOOT_SIZE	0x40000
#define CFG_SYS_NAND_U_BOOT_DST	0xc1080000
#define CFG_SYS_NAND_U_BOOT_START	CFG_SYS_NAND_U_BOOT_DST
#define CFG_SYS_NAND_ECCPOS		{				\
				24, 25, 26, 27, 28, \
				29, 30, 31, 32, 33, 34, 35, 36, 37, 38, \
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48, \
				49, 50, 51, 52, 53, 54, 55, 56, 57, 58, \
				59, 60, 61, 62, 63 }
#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	10
#endif

#ifdef CONFIG_MTD_NOR_FLASH
#define CFG_SYS_FLASH_BASE		DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#define PHYS_FLASH_SIZE			(8 << 20) /* Flash size 8MB */
#endif

/*
 * U-Boot general configuration
 */

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)

#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0xc0700000\0" \
	"fdtaddr=0xc0600000\0" \
	"scriptaddr=0xc0600000\0"

#include <env/ti/mmc.h>

#define CFG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=da850-evm.dtb\0" \
	"boot_fdt=yes\0" \
	"boot_fit=0\0" \
	"console=ttyS2,115200n8\0" \
	"hwconfig=dsp:wake=yes"

#ifdef CONFIG_SPL_BUILD
/* defines for SPL */

#endif

/* Load U-Boot Image From MMC */

/* additions for new relocation code, must added to all boards */
#define CFG_SYS_SDRAM_BASE		0xc0000000

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */
