/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2011 Graf-Syteco, Matthias Weisser
 * <weisserm@arcor.de>
 *
 * Configuation settings for the zmx25 board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_TIMER_RATE		32768
#define CONFIG_SYS_TIMER_COUNTER	\
	(&((struct gpt_regs *)IMX_GPT1_BASE)->counter)

#define CONFIG_MACH_TYPE	MACH_TYPE_ZMX25
/*
 * Environment settings
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"gs_fast_boot=setenv bootdelay 5\0" \
	"gs_slow_boot=setenv bootdelay 10\0" \
	"bootcmd=dcache off; mw.l 0x81000000 0 1024; usb start;" \
		"fatls usb 0; fatload usb 0 0x81000000 zmx25-init.bin;" \
		"bootm 0x81000000; bootelf 0x81000000\0"

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Hardware drivers
 */

/*
 * Serial
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART2_BASE

/*
 * Ethernet
 */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0x00

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

/*
 * Additional command
 */

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI_MXC
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	MXC_EHCI_MODE_SERIAL
#define CONFIG_MXC_USB_FLAGS	(MXC_EHCI_INTERNAL_PHY | MXC_EHCI_IPPUE_DOWN)
#define CONFIG_EHCI_IS_TDI
#endif /* CONFIG_CMD_USB */

/* SDRAM */
#define PHYS_SDRAM		0x80000000	/* start address of LPDDRRAM */
#define PHYS_SDRAM_SIZE		0x04000000	/* 64 megs */

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CONFIG_SYS_INIT_SP_ADDR	0x78020000	/* end of internal SRAM */

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_BASE		0xA0000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256

#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x00040000)
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_SIZE			(128 * 1024)

/*
 * CFI FLASH driver setup
 */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM + (512*1024))
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM + PHYS_SDRAM_SIZE)

#define CONFIG_PREBOOT  ""


/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(0x400000 - 0x8000)

#endif	/* __CONFIG_H */
