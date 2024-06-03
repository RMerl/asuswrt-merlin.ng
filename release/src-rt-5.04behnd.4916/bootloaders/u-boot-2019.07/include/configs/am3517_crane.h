/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * am3517_crane.h - Default configuration for AM3517 CraneBoard.
 *
 * Author: Srinath.R <srinath@mistralsolutions.com>
 *
 * Based on include/configs/am3517evm.h
 *
 * Copyright (C) 2011 Mistral Solutions pvt Ltd
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB sector */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))
						/* initial data */
/*
 * DDR related
 */
#define CONFIG_SYS_CS0_SIZE		(256 * 1024 * 1024)

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/*
 * USB configuration
 * Enable CONFIG_USB_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_USB_MUSB_UDC for Device functionalities.
 */

#ifdef CONFIG_USB_AM35X

#ifdef CONFIG_USB_MUSB_HCD

#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_PREBOOT "usb start"
#endif /* CONFIG_USB_KEYBOARD */

#endif /* CONFIG_USB_MUSB_HCD */

#ifdef CONFIG_USB_MUSB_UDC
/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"AM3517CRANE"
#endif /* CONFIG_USB_MUSB_UDC */

#endif /* CONFIG_USB_AM35X */

#define CONFIG_SYS_I2C

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access */
							/* nand at CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of */
							/* NAND devices */

#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* sz of jffs2 part */

/* Environment information */

#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyS2,115200n8\0" \
	"mmcdev=0\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=/dev/mtdblock4 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		32	/* max number of command */
						/* args */
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0) /* default load */
								/* address */

/*
 * AM3517 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		OMAP34XX_GPT2
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
#define CONFIG_SYS_MAX_FLASH_SECT	520	/* max number of sectors */
						/* on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of flash banks */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_SYS_FLASH_BASE		NAND_BASE

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB sector */
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_ADDR			0x260000

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100 * CONFIG_SYS_HZ)

/* Flash banks JFFS2 should use */
#define CONFIG_SYS_MAX_MTD_BANKS	(CONFIG_SYS_MAX_FLASH_BANKS + \
					CONFIG_SYS_MAX_NAND_DEVICE)
#define CONFIG_SYS_JFFS2_MEM_NAND
/* use flash_info[2] */
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#endif /* __CONFIG_H */
