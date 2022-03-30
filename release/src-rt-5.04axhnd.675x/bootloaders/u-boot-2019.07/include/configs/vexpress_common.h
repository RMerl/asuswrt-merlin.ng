/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 ARM Limited
 * (C) Copyright 2010 Linaro
 * Matt Waddel, <matt.waddel@linaro.org>
 *
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 */

#ifndef __VEXPRESS_COMMON_H
#define __VEXPRESS_COMMON_H

/*
 * Definitions copied from linux kernel:
 * arch/arm/mach-vexpress/include/mach/motherboard.h
 */
#ifdef CONFIG_VEXPRESS_ORIGINAL_MEMORY_MAP
/* CS register bases for the original memory map. */
#define V2M_PA_CS0		0x40000000
#define V2M_PA_CS1		0x44000000
#define V2M_PA_CS2		0x48000000
#define V2M_PA_CS3		0x4c000000
#define V2M_PA_CS7		0x10000000

#define V2M_PERIPH_OFFSET(x)	(x << 12)
#define V2M_SYSREGS		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(0))
#define V2M_SYSCTL		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(1))
#define V2M_SERIAL_BUS_PCI	(V2M_PA_CS7 + V2M_PERIPH_OFFSET(2))

#define V2M_BASE		0x60000000
#elif defined(CONFIG_VEXPRESS_EXTENDED_MEMORY_MAP)
/* CS register bases for the extended memory map. */
#define V2M_PA_CS0		0x08000000
#define V2M_PA_CS1		0x0c000000
#define V2M_PA_CS2		0x14000000
#define V2M_PA_CS3		0x18000000
#define V2M_PA_CS7		0x1c000000

#define V2M_PERIPH_OFFSET(x)	(x << 16)
#define V2M_SYSREGS		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(1))
#define V2M_SYSCTL		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(2))
#define V2M_SERIAL_BUS_PCI	(V2M_PA_CS7 + V2M_PERIPH_OFFSET(3))

#define V2M_BASE		0x80000000
#endif

/*
 * Physical addresses, offset from V2M_PA_CS0-3
 */
#define V2M_NOR0		(V2M_PA_CS0)
#define V2M_NOR1		(V2M_PA_CS1)
#define V2M_SRAM		(V2M_PA_CS2)
#define V2M_VIDEO_SRAM		(V2M_PA_CS3 + 0x00000000)
#define V2M_ISP1761		(V2M_PA_CS3 + 0x03000000)

/* Common peripherals relative to CS7. */
#define V2M_AACI		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(4))
#define V2M_MMCI		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(5))
#define V2M_KMI0		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(6))
#define V2M_KMI1		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(7))

#define V2M_UART0		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(9))
#define V2M_UART1		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(10))
#define V2M_UART2		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(11))
#define V2M_UART3		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(12))

#define V2M_WDT			(V2M_PA_CS7 + V2M_PERIPH_OFFSET(15))

#define V2M_TIMER01		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(17))
#define V2M_TIMER23		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(18))

#define V2M_SERIAL_BUS_DVI	(V2M_PA_CS7 + V2M_PERIPH_OFFSET(22))
#define V2M_RTC			(V2M_PA_CS7 + V2M_PERIPH_OFFSET(23))

#define V2M_CF			(V2M_PA_CS7 + V2M_PERIPH_OFFSET(26))

#define V2M_CLCD		(V2M_PA_CS7 + V2M_PERIPH_OFFSET(31))
#define V2M_SIZE_CS7		V2M_PERIPH_OFFSET(32)

/* System register offsets. */
#define V2M_SYS_CFGDATA		(V2M_SYSREGS + 0x0a0)
#define V2M_SYS_CFGCTRL		(V2M_SYSREGS + 0x0a4)
#define V2M_SYS_CFGSTAT		(V2M_SYSREGS + 0x0a8)

/*
 * Configuration
 */
#define SYS_CFG_START		(1 << 31)
#define SYS_CFG_WRITE		(1 << 30)
#define SYS_CFG_OSC		(1 << 20)
#define SYS_CFG_VOLT		(2 << 20)
#define SYS_CFG_AMP		(3 << 20)
#define SYS_CFG_TEMP		(4 << 20)
#define SYS_CFG_RESET		(5 << 20)
#define SYS_CFG_SCC		(6 << 20)
#define SYS_CFG_MUXFPGA		(7 << 20)
#define SYS_CFG_SHUTDOWN	(8 << 20)
#define SYS_CFG_REBOOT		(9 << 20)
#define SYS_CFG_DVIMODE		(11 << 20)
#define SYS_CFG_POWER		(12 << 20)
#define SYS_CFG_SITE_MB		(0 << 16)
#define SYS_CFG_SITE_DB1	(1 << 16)
#define SYS_CFG_SITE_DB2	(2 << 16)
#define SYS_CFG_STACK(n)	((n) << 12)

#define SYS_CFG_ERR		(1 << 1)
#define SYS_CFG_COMPLETE	(1 << 0)

/* Board info register */
#define SYS_ID				V2M_SYSREGS
#define CONFIG_REVISION_TAG		1

#define CONFIG_SYS_MEMTEST_START	V2M_BASE
#define CONFIG_SYS_MEMTEST_END		0x20000000

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_SYS_L2CACHE_OFF		1
#define CONFIG_INITRD_TAG		1

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 512 * 1024) /* >= 512 KiB */

#define SCTL_BASE			V2M_SYSCTL
#define VEXPRESS_FLASHPROG_FLVPPEN	(1 << 0)

#define CONFIG_SYS_TIMER_RATE		1000000
#define CONFIG_SYS_TIMER_COUNTER	(V2M_TIMER01 + 0x4)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK		24000000
#define CONFIG_PL01x_PORTS		{(void *)CONFIG_SYS_SERIAL0, \
					 (void *)CONFIG_SYS_SERIAL1}

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_SERIAL0		V2M_UART0
#define CONFIG_SYS_SERIAL1		V2M_UART1

#define CONFIG_ARM_PL180_MMCI
#define CONFIG_ARM_PL180_MMCI_BASE	V2M_MMCI
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	127
#define CONFIG_ARM_PL180_MMCI_CLOCK_FREQ 6250000

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(V2M_BASE + 0x8000)
#define LINUX_BOOT_PARAM_ADDR		(V2M_BASE + 0x2000)

/* Physical Memory Map */
#define PHYS_SDRAM_1			(V2M_BASE)	/* SDRAM Bank #1 */
#define PHYS_SDRAM_2			(((unsigned int)V2M_BASE) + \
					((unsigned int)0x20000000))
#define PHYS_SDRAM_1_SIZE		0x20000000	/* 512 MB */
#define PHYS_SDRAM_2_SIZE		0x20000000	/* 512 MB */

/* additions for new relocation code */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE		0x1000
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_BASE + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_GBL_DATA_OFFSET

/* Basic environment settings */
#define BOOT_TARGET_DEVICES(func) \
        func(MMC, mmc, 1) \
        func(MMC, mmc, 0) \
        func(PXE, pxe, na) \
        func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#ifdef CONFIG_VEXPRESS_ORIGINAL_MEMORY_MAP
#define CONFIG_PLATFORM_ENV_SETTINGS \
		"loadaddr=0x80008000\0" \
		"ramdisk_addr_r=0x61000000\0" \
		"kernel_addr=0x44100000\0" \
		"ramdisk_addr=0x44800000\0" \
		"maxramdisk=0x1800000\0" \
		"pxefile_addr_r=0x88000000\0" \
		"scriptaddr=0x88000000\0" \
		"kernel_addr_r=0x80008000\0"
#elif defined(CONFIG_VEXPRESS_EXTENDED_MEMORY_MAP)
#define CONFIG_PLATFORM_ENV_SETTINGS \
		"loadaddr=0xa0008000\0" \
		"ramdisk_addr_r=0x81000000\0" \
		"kernel_addr=0x0c100000\0" \
		"ramdisk_addr=0x0c800000\0" \
		"maxramdisk=0x1800000\0" \
		"pxefile_addr_r=0xa8000000\0" \
		"scriptaddr=0xa8000000\0" \
		"kernel_addr_r=0xa0008000\0"
#endif
#define CONFIG_EXTRA_ENV_SETTINGS \
		CONFIG_PLATFORM_ENV_SETTINGS \
                BOOTENV \
		"console=ttyAMA0,38400n8\0" \
		"dram=1024M\0" \
		"root=/dev/sda1 rw\0" \
		"mtd=armflash:1M@0x800000(uboot),7M@0x1000000(kernel)," \
			"24M@0x2000000(initrd)\0" \
		"flashargs=setenv bootargs root=${root} console=${console} " \
			"mem=${dram} mtdparts=${mtd} mmci.fmax=190000 " \
			"devtmpfs.mount=0  vmalloc=256M\0" \
		"bootflash=run flashargs; " \
			"cp ${ramdisk_addr} ${ramdisk_addr_r} ${maxramdisk}; " \
			"bootm ${kernel_addr} ${ramdisk_addr_r}\0"

/* FLASH and environment organization */
#define PHYS_FLASH_SIZE			0x04000000	/* 64MB */
#define CONFIG_SYS_FLASH_SIZE		0x04000000
#define CONFIG_SYS_MAX_FLASH_BANKS	2
#define CONFIG_SYS_FLASH_BASE0		V2M_NOR0
#define CONFIG_SYS_FLASH_BASE1		V2M_NOR1
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE0

/* Timeout values in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2 * CONFIG_SYS_HZ) /* Erase Timeout */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2 * CONFIG_SYS_HZ) /* Write Timeout */

/* 255 0x40000 sectors + first or last sector may have 4 erase regions = 259 */
#define CONFIG_SYS_MAX_FLASH_SECT	259		/* Max sectors */
#define FLASH_MAX_SECTOR_SIZE		0x00040000	/* 256 KB sectors */

/* Room required on the stack for the environment data */
#define CONFIG_ENV_SIZE			FLASH_MAX_SECTOR_SIZE

/*
 * Amount of flash used for environment:
 * We don't know which end has the small erase blocks so we use the penultimate
 * sector location for the environment
 */
#define CONFIG_ENV_SECT_SIZE		FLASH_MAX_SECTOR_SIZE
#define CONFIG_ENV_OVERWRITE		1

/* Store environment at top of flash */
#define CONFIG_ENV_OFFSET		(PHYS_FLASH_SIZE - \
					(2 * CONFIG_ENV_SECT_SIZE))
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE1 + \
					 CONFIG_ENV_OFFSET)
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE0, \
					  CONFIG_SYS_FLASH_BASE1 }

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#endif /* VEXPRESS_COMMON_H */
