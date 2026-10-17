/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common board functions for Siemens TAURUS (AT91SAM9G20) based boards
 * (C) Copyright 2013 Siemens AG
 *
 * Based on:
 * U-Boot file: include/configs/at91sam9260ek.h
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>
#include <linux/sizes.h>

/*
 * Warning: changing CONFIG_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CFG_SYS_AT91_MAIN_CLOCK	18432000	/* main clock xtal */

/* Misc CPU related */

#define CFG_USART_BASE		ATMEL_BASE_DBGU
#define CFG_USART_ID			ATMEL_ID_SYS

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CFG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CFG_SYS_SDRAM_SIZE		(128 * SZ_1M)

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#define CFG_SYS_INIT_RAM_SIZE	0x1000
#define CFG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CFG_SYS_NAND_MASK_ALE	(1 << 21)
#define CFG_SYS_NAND_MASK_CLE	(1 << 22)
#define CFG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CFG_SYS_NAND_READY_PIN	AT91_PIN_PC13
#endif

#if defined(CONFIG_BOARD_TAURUS)
/* DFU class support */
#define DFU_MANIFEST_POLL_TIMEOUT	25000
#endif

/* SPI EEPROM */
#define TAURUS_SPI_MASK (1 << 4)

/* bootstrap in spi flash , u-boot + env + linux in nandflash */

#if defined(CONFIG_BOARD_AXM)
#define CFG_EXTRA_ENV_SETTINGS \
	"addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:" \
		"${gatewayip}:${netmask}:${hostname}:${netdev}::off\0" \
	"addtest=setenv bootargs ${bootargs} loglevel=4 test\0" \
	"boot_file=setenv bootfile /${project_dir}/kernel/uImage\0" \
	"boot_retries=0\0" \
	"ethact=macb0\0" \
	"flash_nfs=run nand_kernel;run nfsargs;run addip;" \
		"upgrade_available;bootm ${kernel_ram};reset\0" \
	"flash_self=run nand_kernel;run setbootargs;upgrade_available;" \
		"bootm ${kernel_ram};reset\0" \
	"flash_self_test=run nand_kernel;run setbootargs addtest;" \
		"upgrade_available;bootm ${kernel_ram};reset\0" \
	"hostname=systemone\0" \
	"kernel_Off=0x00200000\0" \
	"kernel_Off_fallback=0x03800000\0" \
	"kernel_ram=0x21500000\0" \
	"kernel_size=0x00400000\0" \
	"kernel_size_fallback=0x00400000\0" \
	"loads_echo=1\0" \
	"nand_kernel=nand read.e ${kernel_ram} ${kernel_Off} " \
		"${kernel_size}\0" \
	"net_nfs=run boot_file;tftp ${kernel_ram} ${bootfile};" \
		"run nfsargs;run addip;upgrade_available;" \
		"bootm ${kernel_ram};reset\0" \
	"netdev=eth0\0" \
	"nfsargs=run root_path;setenv bootargs ${bootargs} root=/dev/nfs " \
		"rw nfsroot=${serverip}:${rootpath} " \
		"at91sam9_wdt.wdt_timeout=16\0" \
	"partitionset_active=A\0" \
	"preboot=echo;echo Type 'run flash_self' to use kernel and root " \
		"filesystem on memory;echo Type 'run flash_nfs' to use " \
		"kernel from memory and root filesystem over NFS;echo Type " \
		"'run net_nfs' to get Kernel over TFTP and mount root " \
		"filesystem over NFS;echo\0" \
	"project_dir=systemone\0" \
	"root_path=setenv rootpath /home/projects/${project_dir}/rootfs\0" \
	"rootfs=/dev/mtdblock5\0" \
	"rootfs_fallback=/dev/mtdblock7\0" \
	"setbootargs=setenv bootargs ${bootargs} console=ttyMTD,mtdoops " \
		"root=${rootfs} rootfstype=jffs2 panic=7 " \
		"at91sam9_wdt.wdt_timeout=16\0" \
	"stderr=serial\0" \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"upgrade_available=0\0"
#endif

/* Defines for SPL */

#define CFG_SYS_NAND_U_BOOT_SIZE	SZ_512K
#define	CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE
#define CFG_SYS_NAND_U_BOOT_DST	CONFIG_TEXT_BASE

#define CFG_SYS_NAND_ECCSIZE		256
#define CFG_SYS_NAND_ECCBYTES	3
#define CFG_SYS_NAND_ECCPOS		{ 40, 41, 42, 43, 44, 45, 46, 47, \
					  48, 49, 50, 51, 52, 53, 54, 55, \
					  56, 57, 58, 59, 60, 61, 62, 63, }

#define CFG_SYS_MASTER_CLOCK		132096000
#define AT91_PLL_LOCK_TIMEOUT		1000000
#define CFG_SYS_AT91_PLLA		0x202A3F01
#define CFG_SYS_MCKR			0x1300
#define CFG_SYS_MCKR_CSS		(0x02 | CFG_SYS_MCKR)
#define CFG_SYS_AT91_PLLB		0x10193F05

#endif
