/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 */
#ifndef __CONFIG_SOCFPGA_COMMON_H__
#define __CONFIG_SOCFPGA_COMMON_H__

#include <linux/stringify.h>

/*
 * Memory configurations
 */
#define PHYS_SDRAM_1			0x0
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#define CFG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CFG_SYS_INIT_RAM_SIZE	SOCFPGA_PHYS_OCRAM_SIZE
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#define CFG_SYS_INIT_RAM_ADDR	0xFFE00000
/* SPL memory allocation configuration, this is for FAT implementation */
#define CFG_SYS_INIT_RAM_SIZE	(SOCFPGA_PHYS_OCRAM_SIZE - \
					 CONFIG_SPL_SYS_MALLOC_SIZE)
#endif

/*
 * Some boards (e.g. socfpga_sr1500) use 8 bytes at the end of the internal
 * SRAM as bootcounter storage. Make sure to not put the stack directly
 * at this address to not overwrite the bootcounter by checking, if the
 * bootcounter address is located in the internal SRAM.
 */
#if ((CONFIG_SYS_BOOTCOUNT_ADDR > CFG_SYS_INIT_RAM_ADDR) &&	\
     (CONFIG_SYS_BOOTCOUNT_ADDR < (CFG_SYS_INIT_RAM_ADDR +	\
				   CFG_SYS_INIT_RAM_SIZE)))
#endif

/*
 * U-Boot stack setup: if SPL post-reloc uses DDR stack, use it in pre-reloc
 * phase of U-Boot, too. This prevents overwriting SPL data if stack/heap usage
 * in U-Boot pre-reloc is higher than in SPL.
 */

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/*
 * U-Boot general configurations
 */
						/* Print buffer size */

/*
 * Cache
 */
#define CFG_SYS_PL310_BASE		SOCFPGA_MPUL2_ADDRESS

/*
 * L4 OSC1 Timer 0
 */
#ifndef CONFIG_TIMER
#define CFG_SYS_TIMERBASE		SOCFPGA_OSC1TIMER0_ADDRESS
#define CFG_SYS_TIMER_COUNTER	(CFG_SYS_TIMERBASE + 0x4)
#ifndef CFG_SYS_TIMER_RATE
#define CFG_SYS_TIMER_RATE		25000000
#endif
#endif

/*
 * L4 Watchdog
 */
#define CFG_DW_WDT_CLOCK_KHZ		25000

/*
 * NAND Support
 */
#ifdef CONFIG_NAND_DENALI
#define CFG_SYS_NAND_REGS_BASE	SOCFPGA_NANDREGS_ADDRESS
#define CFG_SYS_NAND_DATA_BASE	SOCFPGA_NANDDATA_ADDRESS
#endif

/*
 * USB
 */

/*
 * USB Gadget (DFU, UMS)
 */
#if defined(CONFIG_CMD_DFU) || defined(CONFIG_CMD_USB_MASS_STORAGE)
#define DFU_DEFAULT_POLL_TIMEOUT	300
#endif

/*
 * U-Boot environment
 */

/* Environment for SDMMC boot */

/* Environment for QSPI boot */

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

/* SPL QSPI boot support */

/* SPL NAND boot support */

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

#ifndef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
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
