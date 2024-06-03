/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2013
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * Thomas Petazzoni, Free Electrons, <thomas.petazzoni@free-electrons.com>
 * Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Settings for Calao USB-A9263 board
 *
 * U-Boot image has to be less than 200704 bytes, otherwise at91bootstrap
 * installed on board will not be able to load it properly.
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <asm/hardware.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768

#define CONFIG_MACH_TYPE		MACH_TYPE_USB_A9263

#define CONFIG_ARCH_CPU_INIT

#define CONFIG_CMDLINE_TAG	/* enable passing of ATAGs      */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Hardware drivers
 */
/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		0x04000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		GPIO_PIN_PD(15)
#define CONFIG_SYS_NAND_READY_PIN		GPIO_PIN_PA(22)
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT			20
#define CONFIG_AT91_WANTS_COMMON_PHY

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif

#define CONFIG_SYS_LOAD_ADDR			0x22000000

#define CONFIG_SYS_MEMTEST_START		CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END			0x23e00000

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_OFFSET	0x2000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#define CONFIG_BOOTCOMMAND	"nboot 21000000 0"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)

#endif
