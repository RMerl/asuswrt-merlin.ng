/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bluewater Systems Snapper 9G45 module
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC type is defined in boards.cfg */
#include <asm/hardware.h>
#include <linux/sizes.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768

/* CPU */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT_ONLY

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS6
#define CONFIG_SYS_SDRAM_SIZE		(128 * 1024 * 1024) /* 64MB */
#define CONFIG_SYS_INIT_SP_ADDR		(ATMEL_BASE_SRAM + 0x1000 - \
					 GENERATED_GBL_DATA_SIZE)

/* Mem test settings */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + (1024 * 1024))

/* NAND Flash */
#define CONFIG_SYS_NAND_ECC_BASE	ATMEL_BASE_ECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21) /* AD21 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22) /* AD22 */
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC8

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_AT91_WANTS_COMMON_PHY
#define CONFIG_TFTP_PORT
#define CONFIG_TFTP_TSIZE

/* MMC */
#define CONFIG_GENERIC_ATMEL_MCI

/* LCD */
#define CONFIG_ATMEL_LCD
#define CONFIG_GURNARD_SPLASH

/* GPIOs and IO expander */
#define CONFIG_ATMEL_LEGACY
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP		1

/* UARTs/Serial console */
#define CONFIG_ATMEL_USART

/* Boot options */
#define CONFIG_SYS_LOAD_ADDR		0x23000000

#define CONFIG_BOOTP_BOOTFILESIZE

/* Environment settings */
#define CONFIG_ENV_OFFSET		(512 << 10)
#define CONFIG_ENV_SIZE			(256 << 10)
#define CONFIG_ENV_OVERWRITE

#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:00:00:00:00:00\0" \
	"serial=0\0" \
	"stdout=serial_atmel\0" \
	"stderr=serial_atmel\0" \
	"stdin=serial_atmel\0" \
	"bootlimit=3\0" \
	"loadaddr=0x71000000\0" \
	"board_rev=2\0" \
	"bootfile=/tftpboot/uImage\0" \
	"bootargs_def=console=ttyS0,115200 panic=5 quiet lpj=997376\0" \
	"nfsroot=/export/root\0" \
	"boot_working=setenv bootargs $bootargs_def; nboot $loadaddr 0 0x20c0000 && bootm\0" \
	"boot_safe=setenv bootargs $bootargs_def; nboot $loadaddr 0 0xc0000 && bootm\0" \
	"boot_tftp=setenv bootargs $bootargs_def ip=any nfsroot=$nfsroot; setenv autoload y && bootp && bootm\0" \
	"boot_usb=setenv bootargs $bootargs_def; usb start && usb storage && fatload usb 0:1 $loadaddr dds-xm200.bin && bootm\0" \
	"boot_mmc=setenv bootargs $bootargs_def; mmc rescan && fatload mmc 0:1 $loadaddr dds-xm200.bin && bootm\0" \
	"bootcmd=run boot_mmc ; run boot_usb ; run boot_working ; run boot_safe\0" \
	"altbootcmd=run boot_mmc ; run boot_usb ; run boot_safe ; run boot_working\0"

/* Console settings */

/* U-Boot memory settings */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)

/* Command line configuration */
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_CACHE

#endif /* __CONFIG_H */
