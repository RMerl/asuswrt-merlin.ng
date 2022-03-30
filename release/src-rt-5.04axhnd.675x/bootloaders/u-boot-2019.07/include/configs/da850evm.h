/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
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
/* check if direct NOR boot config is used */
#ifndef CONFIG_DIRECT_NOR_BOOT
#define CONFIG_USE_SPIFLASH
#endif

/*
* Disable DM_* for SPL build and can be re-enabled after adding
* DM support in SPL
*/
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_I2C
#undef CONFIG_DM_I2C_COMPAT
#endif
/*
 * SoC Configuration
 */
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_SYS_CLK_FREQ		clk_get(DAVINCI_ARM_CLKID)
#define CONFIG_SYS_OSCIN_FREQ		24000000
#define CONFIG_SYS_TIMERBASE		DAVINCI_TIMER0_BASE
#define CONFIG_SYS_HZ_CLOCK		clk_get(DAVINCI_AUXCLK_CLKID)
#define CONFIG_SKIP_LOWLEVEL_INIT

#ifdef CONFIG_DIRECT_NOR_BOOT
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SYS_DV_NOR_BOOT_CFG	(0x11)
#endif

/*
 * Memory Info
 */
#define CONFIG_SYS_MALLOC_LEN	(0x10000 + 1*1024*1024) /* malloc() len */
#define PHYS_SDRAM_1		DAVINCI_DDR_EMIF_DATA_BASE /* DDR Start */
#define PHYS_SDRAM_1_SIZE	(64 << 20) /* SDRAM size 64MB */
#define CONFIG_MAX_RAM_BANK_SIZE (512 << 20) /* max size from SPRS586*/
#define CONFIG_SPL_BSS_START_ADDR DAVINCI_DDR_EMIF_DATA_BASE
#define CONFIG_SPL_BSS_MAX_SIZE 0x1080000
/* memtest start addr */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1 + 0x2000000)

/* memtest will be run on 16MB */
#define CONFIG_SYS_MEMTEST_END 	(PHYS_SDRAM_1 + 0x2000000 + 16*1024*1024)

#define CONFIG_SYS_DA850_SYSCFG_SUSPSRC (	\
	DAVINCI_SYSCFG_SUSPSRC_TIMER0 |		\
	DAVINCI_SYSCFG_SUSPSRC_SPI1 |		\
	DAVINCI_SYSCFG_SUSPSRC_UART2 |		\
	DAVINCI_SYSCFG_SUSPSRC_EMAC |		\
	DAVINCI_SYSCFG_SUSPSRC_I2C)

/*
 * PLL configuration
 */

#define CONFIG_SYS_DA850_PLL0_PLLM     24
#define CONFIG_SYS_DA850_PLL1_PLLM     21

/*
 * DDR2 memory configuration
 */
#define CONFIG_SYS_DA850_DDR2_DDRPHYCR (DV_DDR_PHY_PWRDNEN | \
					DV_DDR_PHY_EXT_STRBEN | \
					(0x4 << DV_DDR_PHY_RD_LATENCY_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDBCR (		\
	(1 << DV_DDR_SDCR_MSDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_DDREN_SHIFT) |	\
	(1 << DV_DDR_SDCR_SDRAMEN_SHIFT) |	\
	(1 << DV_DDR_SDCR_BUS_WIDTH_SHIFT) |	\
	(0x3 << DV_DDR_SDCR_CL_SHIFT) |		\
	(0x2 << DV_DDR_SDCR_IBANK_SHIFT) |	\
	(0x2 << DV_DDR_SDCR_PAGESIZE_SHIFT))

/* SDBCR2 is only used if IBANK_POS bit in SDBCR is set */
#define CONFIG_SYS_DA850_DDR2_SDBCR2 0

#define CONFIG_SYS_DA850_DDR2_SDTIMR (		\
	(14 << DV_DDR_SDTMR1_RFC_SHIFT) |	\
	(2 << DV_DDR_SDTMR1_RP_SHIFT) |		\
	(2 << DV_DDR_SDTMR1_RCD_SHIFT) |	\
	(1 << DV_DDR_SDTMR1_WR_SHIFT) |		\
	(5 << DV_DDR_SDTMR1_RAS_SHIFT) |	\
	(8 << DV_DDR_SDTMR1_RC_SHIFT) |		\
	(1 << DV_DDR_SDTMR1_RRD_SHIFT) |	\
	(0 << DV_DDR_SDTMR1_WTR_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDTIMR2 (		\
	(7 << DV_DDR_SDTMR2_RASMAX_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_XP_SHIFT) |		\
	(0 << DV_DDR_SDTMR2_ODT_SHIFT) |	\
	(17 << DV_DDR_SDTMR2_XSNR_SHIFT) |	\
	(199 << DV_DDR_SDTMR2_XSRD_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_RTP_SHIFT) |	\
	(0 << DV_DDR_SDTMR2_CKE_SHIFT))

#define CONFIG_SYS_DA850_DDR2_SDRCR    0x00000494
#define CONFIG_SYS_DA850_DDR2_PBBPR    0x30

/*
 * Serial Driver info
 */

#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_COM1	DAVINCI_UART2_BASE /* Base address of UART2 */
#endif
#define CONFIG_SYS_NS16550_CLK	clk_get(DAVINCI_UART2_CLKID)

#define CONFIG_SYS_SPI_CLK		clk_get(DAVINCI_SPI1_CLKID)
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_SPI_BASE		DAVINCI_SPI1_BASE
#endif

#ifdef CONFIG_USE_SPIFLASH
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8000
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x40000
#endif

/*
 * I2C Configuration
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SYS_I2C_EXPANDER_ADDR   0x20
#endif

/*
 * Flash & Environment
 */
#ifdef CONFIG_NAND
#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x0 /* Block 0--not used by bootcode */
#define CONFIG_ENV_SIZE			(128 << 10)
#define CONFIG_ENV_SECT_SIZE	(128 << 10)
#endif
#define	CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
#define	CONFIG_SYS_NAND_PAGE_2K
#define CONFIG_SYS_NAND_CS		3
#define CONFIG_SYS_NAND_BASE		DAVINCI_ASYNC_EMIF_DATA_CE3_BASE
#define CONFIG_SYS_NAND_MASK_CLE		0x10
#define CONFIG_SYS_NAND_MASK_ALE		0x8
#undef CONFIG_SYS_NAND_HW_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1 /* Max number of NAND devices */
#define CONFIG_SYS_NAND_HW_ECC_OOBFIRST
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	(2 << 10)
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 << 10)
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x40000
#define CONFIG_SYS_NAND_U_BOOT_DST	0xc1080000
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP	(CONFIG_SYS_NAND_U_BOOT_DST - \
					CONFIG_SYS_NAND_U_BOOT_SIZE - \
					CONFIG_SYS_MALLOC_LEN -       \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_NAND_ECCPOS		{				\
				24, 25, 26, 27, 28, \
				29, 30, 31, 32, 33, 34, 35, 36, 37, 38, \
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48, \
				49, 50, 51, 52, 53, 54, 55, 56, 57, 58, \
				59, 60, 61, 62, 63 }
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	10
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_NAND_LOAD
#endif

/*
 * Network & Ethernet Configuration
 */
#ifdef CONFIG_DRIVER_TI_EMAC
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
#endif

#ifdef CONFIG_USE_NOR
#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* max number of flash banks */
#define CONFIG_SYS_FLASH_SECT_SZ	(128 << 10) /* 128KB */
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_FLASH_SECT_SZ * 3)
#define CONFIG_ENV_SIZE			(10 << 10) /* 10KB */
#define CONFIG_SYS_FLASH_BASE		DAVINCI_ASYNC_EMIF_DATA_CE2_BASE
#define PHYS_FLASH_SIZE			(8 << 20) /* Flash size 8MB */
#define CONFIG_SYS_MAX_FLASH_SECT ((PHYS_FLASH_SIZE/CONFIG_SYS_FLASH_SECT_SZ)\
	       + 3)
#define CONFIG_ENV_SECT_SIZE		CONFIG_SYS_FLASH_SECT_SZ
#endif

#ifdef CONFIG_USE_SPIFLASH
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE			(64 << 10)
#define CONFIG_ENV_OFFSET		(512 << 10)
#define CONFIG_ENV_SECT_SIZE	(64 << 10)
#endif
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_SPI_FLASH_MTD
#endif
#endif

/*
 * U-Boot general configuration
 */
#define CONFIG_BOOTFILE		"uImage" /* Boot file name */
#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Args Buffer Size */
#define CONFIG_SYS_LOAD_ADDR	(PHYS_SDRAM_1 + 0x700000)
#define CONFIG_MX_CYCLIC

/*
 * Linux Information
 */
#define LINUX_BOOT_PARAM_ADDR	(PHYS_SDRAM_1 + 0x100)
#define CONFIG_HWCONFIG		/* enable hwconfig */
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS

#define CONFIG_BOOTCOMMAND \
		"run envboot; " \
		"run mmcboot; "

#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0xc0700000\0" \
	"fdtaddr=0xc0600000\0" \
	"scriptaddr=0xc0600000\0"

#include <environment/ti/mmc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
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

#ifdef CONFIG_CMD_BDI
#define CONFIG_CLOCKS
#endif

#if !defined(CONFIG_NAND) && \
	!defined(CONFIG_USE_NOR) && \
	!defined(CONFIG_USE_SPIFLASH)
#define CONFIG_ENV_SIZE		(16 << 10)
#endif

/* USB Configs */
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x01E25000
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"da850evm"

#ifndef CONFIG_DIRECT_NOR_BOOT
/* defines for SPL */
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE - \
						CONFIG_SYS_MALLOC_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SPL_STACK	0x8001ff00
#define CONFIG_SPL_MAX_FOOTPRINT	32768
#define CONFIG_SPL_PAD_TO	32768
#endif

/* Load U-Boot Image From MMC */

/* additions for new relocation code, must added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0xc0000000

#ifdef CONFIG_DIRECT_NOR_BOOT
#define CONFIG_SYS_INIT_SP_ADDR		0x8001ff00
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - /* Fix this */ \
					GENERATED_GBL_DATA_SIZE)
#endif /* CONFIG_DIRECT_NOR_BOOT */

#include <asm/arch/hardware.h>

#endif /* __CONFIG_H */
