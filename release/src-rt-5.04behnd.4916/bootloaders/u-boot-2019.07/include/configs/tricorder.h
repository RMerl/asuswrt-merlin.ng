/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * (C) Copyright 2012
 * Corscience GmbH & Co. KG
 * Thomas Weber <weber@corscience.de>
 *
 * Configuration settings for the Tricorder board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MACH_TYPE		MACH_TYPE_TRICORDER
/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/* Clock Defines */
#define V_OSCK				26000000 /* Clock output from T2 */
#define V_SCLK				(V_OSCK >> 1)

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(1024*1024)

/* Hardware drivers */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000 /* 48MHz (APLL96/2) */

/* select serial console configuration */
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/* I2C */
#define CONFIG_SYS_I2C
 

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_EEPROM_BUS_NUM	1

/* TWL4030 */

/* Board NAND Info */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_SYS_NAND_MAX_OOBFREE	2
#define CONFIG_SYS_NAND_MAX_ECCPOS	56

/* needed for ubi */

/* Environment information (this is the common part) */


/* hang() the board on panic() */

/* environment placement (for NAND), is different for FLASHCARD but does not
 * harm there */
#define CONFIG_ENV_OFFSET		0x120000    /* env start */
#define CONFIG_ENV_OFFSET_REDUND	0x2A0000    /* redundant env start */
#define CONFIG_ENV_SIZE			(16 << 10)  /* use 16KiB for env */
#define CONFIG_ENV_RANGE		(384 << 10) /* allow badblocks in env */

/* the loadaddr is the same as CONFIG_SYS_LOAD_ADDR, unfortunately the defiend
 * value can not be used here! */
#define CONFIG_LOADADDR		0x82000000

#define CONFIG_COMMON_ENV_SETTINGS \
	"console=ttyO2,115200n8\0" \
	"mmcdev=0\0" \
	"vram=3M\0" \
	"defaultdisplay=lcd\0" \
	"kernelopts=mtdoops.mtddev=3\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"commonargs=" \
		"setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${kernelopts} " \
		"vt.global_cursor_default=0 " \
		"vram=${vram} " \
		"omapdss.def_disp=${defaultdisplay}\0"

#define CONFIG_BOOTCOMMAND "run autoboot"

/* specific environment settings for different use cases
 * FLASHCARD: used to run a rdimage from sdcard to program the device
 * 'NORMAL': used to boot kernel from sdcard, nand, ...
 *
 * The main aim for the FLASHCARD skin is to have an embedded environment
 * which will not be influenced by any data already on the device.
 */
#ifdef CONFIG_FLASHCARD
/* the rdaddr is 16 MiB before the loadaddr */
#define CONFIG_ENV_RDADDR	"rdaddr=0x81000000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_COMMON_ENV_SETTINGS \
	CONFIG_ENV_RDADDR \
	"autoboot=" \
	"run commonargs; " \
	"setenv bootargs ${bootargs} " \
		"flashy_updateimg=/dev/mmcblk0p1:corscience_update.img " \
		"rdinit=/sbin/init; " \
	"mmc dev ${mmcdev}; mmc rescan; " \
	"fatload mmc ${mmcdev} ${loadaddr} uImage; " \
	"fatload mmc ${mmcdev} ${rdaddr} uRamdisk; " \
	"bootm ${loadaddr} ${rdaddr}\0"

#else /* CONFIG_FLASHCARD */

#define CONFIG_ENV_OVERWRITE /* allow to overwrite serial and ethaddr */

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_COMMON_ENV_SETTINGS \
	"mmcargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 " \
		"rootwait " \
		"rw\0" \
	"nandargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=ubi0:root " \
		"ubi.mtd=7 " \
		"rootfstype=ubifs " \
		"ro\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"loaduimage_ubi=ubi part ubi; " \
		"ubifsmount ubi:root; " \
		"ubifsload ${loadaddr} /boot/uImage\0" \
	"loaduimage_nand=nand read ${loadaddr} kernel 0x500000\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"run loaduimage_nand; " \
		"bootm ${loadaddr}\0" \
	"autoboot=mmc dev ${mmcdev}; if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"if run loaduimage; then " \
					"run mmcboot; " \
				"else run nandboot; " \
				"fi; " \
			"fi; " \
		"else run nandboot; fi\0"

#endif /* CONFIG_FLASHCARD */

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0 + 0x00000000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					0x07000000) /* 112 MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0 + 0x02000000)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2 /* Divisor: 2^(PTV+1) => 8 */

/*  Physical Memory Map  */
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2			OMAP34XX_SDRC_CS1

/* NAND and environment organization  */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

/* SRAM config */
#define CONFIG_SYS_SRAM_START		0x40200000
#define CONFIG_SYS_SRAM_SIZE		0x10000

/* Defines for SPL */

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME        "u-boot.img"
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1

#define CONFIG_SPL_MAX_SIZE		(SRAM_SCRATCH_SPACE_ADDR - \
					 CONFIG_SPL_TEXT_BASE)

#define CONFIG_SPL_BSS_START_ADDR	0x80000000 /*CONFIG_SYS_SDRAM_BASE*/
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, \
					 13, 14, 16, 17, 18, 19, 20, 21, 22, \
					 23, 24, 25, 26, 27, 28, 30, 31, 32, \
					 33, 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 44, 45, 46, 47, 48, 49, 50, 51, \
					 52, 53, 54, 55, 56}

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	13
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x100000

#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */

#define CONFIG_SYS_MEMTEST_SCRATCH	0x81000000
#endif /* __CONFIG_H */
