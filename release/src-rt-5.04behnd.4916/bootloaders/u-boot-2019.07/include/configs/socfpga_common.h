/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 */
#ifndef __CONFIG_SOCFPGA_COMMON_H__
#define __CONFIG_SOCFPGA_COMMON_H__

/*
 * High level configuration
 */
#define CONFIG_CLOCKS

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

/*
 * Memory configurations
 */
#define PHYS_SDRAM_1			0x0
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		PHYS_SDRAM_1_SIZE
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFE00000
/* SPL memory allocation configuration, this is for FAT implementation */
#ifndef CONFIG_SYS_SPL_MALLOC_SIZE
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x10000
#endif
#define CONFIG_SYS_INIT_RAM_SIZE	(0x40000 - CONFIG_SYS_SPL_MALLOC_SIZE)
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE)
#endif

/*
 * Some boards (e.g. socfpga_sr1500) use 8 bytes at the end of the internal
 * SRAM as bootcounter storage. Make sure to not put the stack directly
 * at this address to not overwrite the bootcounter by checking, if the
 * bootcounter address is located in the internal SRAM.
 */
#if ((CONFIG_SYS_BOOTCOUNT_ADDR > CONFIG_SYS_INIT_RAM_ADDR) &&	\
     (CONFIG_SYS_BOOTCOUNT_ADDR < (CONFIG_SYS_INIT_RAM_ADDR +	\
				   CONFIG_SYS_INIT_RAM_SIZE)))
#define CONFIG_SPL_STACK		CONFIG_SYS_BOOTCOUNT_ADDR
#else
#define CONFIG_SPL_STACK			\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#endif

/*
 * U-Boot stack setup: if SPL post-reloc uses DDR stack, use it in pre-reloc
 * phase of U-Boot, too. This prevents overwriting SPL data if stack/heap usage
 * in U-Boot pre-reloc is higher than in SPL.
 */
#if defined(CONFIG_SPL_STACK_R_ADDR) && CONFIG_SPL_STACK_R_ADDR
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SPL_STACK_R_ADDR
#else
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SPL_STACK
#endif

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */

/*
 * Cache
 */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		SOCFPGA_MPUL2_ADDRESS

/*
 * Ethernet on SoC (EMAC)
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_DW_ALTDESCRIPTOR
#endif

/*
 * FPGA Driver
 */
#ifdef CONFIG_CMD_FPGA
#define CONFIG_FPGA_COUNT		1
#endif

/*
 * L4 OSC1 Timer 0
 */
#ifndef CONFIG_TIMER
/* This timer uses eosc1, whose clock frequency is fixed at any condition. */
#define CONFIG_SYS_TIMERBASE		SOCFPGA_OSC1TIMER0_ADDRESS
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMERBASE + 0x4)
#define CONFIG_SYS_TIMER_RATE		25000000
#endif

/*
 * L4 Watchdog
 */
#ifdef CONFIG_HW_WATCHDOG
#define CONFIG_DESIGNWARE_WATCHDOG
#define CONFIG_DW_WDT_BASE		SOCFPGA_L4WD0_ADDRESS
#define CONFIG_DW_WDT_CLOCK_KHZ		25000
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	30000
#endif

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
/* FIXME */
/* using smaller max blk cnt to avoid flooding the limited stack we have */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	256	/* FIXME -- SPL only? */
#endif

/*
 * NAND Support
 */
#ifdef CONFIG_NAND_DENALI
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_REGS_BASE	SOCFPGA_NANDREGS_ADDRESS
#define CONFIG_SYS_NAND_DATA_BASE	SOCFPGA_NANDDATA_ADDRESS
#endif

/*
 * QSPI support
 */
/* Enable multiple SPI NOR flash manufacturers */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SPI_FLASH_MTD
#endif
/* QSPI reference clock */
#ifndef __ASSEMBLY__
unsigned int cm_get_qspi_controller_clk_hz(void);
#define CONFIG_CQSPI_REF_CLK		cm_get_qspi_controller_clk_hz()
#endif

/*
 * USB
 */

/*
 * USB Gadget (DFU, UMS)
 */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(16 * 1024 * 1024)
#define DFU_DEFAULT_POLL_TIMEOUT	300

/* USB IDs */
#define CONFIG_G_DNL_UMS_VENDOR_NUM	0x0525
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	0xA4A5
#endif

/*
 * U-Boot environment
 */
#if !defined(CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE			(8 * 1024)
#endif

/* Environment for SDMMC boot */
#if defined(CONFIG_ENV_IS_IN_MMC) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0 /* device 0 */
#define CONFIG_ENV_OFFSET		(34 * 512) /* just after the GPT */
#endif

/* Environment for QSPI boot */
#if defined(CONFIG_ENV_IS_IN_SPI_FLASH) && !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET		0x00100000
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#endif

/*
 * SPL
 *
 * SRAM Memory layout for gen 5:
 *
 * 0xFFFF_0000 ...... Start of SRAM
 * 0xFFFF_xxxx ...... Top of stack (grows down)
 * 0xFFFF_yyyy ...... Global Data
 * 0xFFFF_zzzz ...... Malloc area
 * 0xFFFF_FFFF ...... End of SRAM
 *
 * SRAM Memory layout for Arria 10:
 * 0xFFE0_0000 ...... Start of SRAM (bottom)
 * 0xFFEx_xxxx ...... Top of stack (grows down to bottom)
 * 0xFFEy_yyyy ...... Global Data
 * 0xFFEz_zzzz ...... Malloc area (grows up to top)
 * 0xFFE3_FFFF ...... End of SRAM (top)
 */
#ifndef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_MAX_SIZE		CONFIG_SYS_INIT_RAM_SIZE
#endif

/* SPL SDMMC boot support */
#ifdef CONFIG_SPL_MMC_SUPPORT
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot-dtb.img"
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#endif
#else
#ifndef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION	1
#endif
#endif

/* SPL QSPI boot support */
#ifdef CONFIG_SPL_SPI_SUPPORT
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x100000
#endif
#endif

/* SPL NAND boot support */
#ifdef CONFIG_SPL_NAND_SUPPORT
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x100000
#endif
#endif

/* Extra Environment */
#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_CMD_DHCP
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"bootm_size=0xa000000\0" \
	"kernel_addr_r="__stringify(CONFIG_SYS_LOAD_ADDR)"\0" \
	"fdt_addr_r=0x02000000\0" \
	"scriptaddr=0x02100000\0" \
	"pxefile_addr_r=0x02200000\0" \
	"ramdisk_addr_r=0x02300000\0" \
	"socfpga_legacy_reset_compat=1\0" \
	BOOTENV

#endif
#endif

#endif	/* __CONFIG_SOCFPGA_COMMON_H__ */
