/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Sentec Cobra Board.
 *
 * (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

/*
 * configuration for ASTRO "Urmel" board.
 * Originating from Cobra5272 configuration, messed up by
 * Wolfgang Wegner <w.wegner@astro-kom.de>
 * Please do not bother the original author with bug reports
 * concerning this file.
 */

#ifndef _CONFIG_ASTRO_MCF5373L_H
#define _CONFIG_ASTRO_MCF5373L_H

#include <linux/stringify.h>

/*
 * set the card type to actually compile for; either of
 * the possibilities listed below has to be used!
 */
#define ASTRO_V532	1

#if ASTRO_V532
#define ASTRO_ID	0xF8
#elif ASTRO_V512
#define ASTRO_ID	0xFA
#elif ASTRO_TWIN7S2
#define ASTRO_ID	0xF9
#elif ASTRO_V912
#define ASTRO_ID	0xFC
#elif ASTRO_COFDMDUOS2
#define ASTRO_ID	0xFB
#else
#error No card type defined!
#endif

/* I2C */

/*
 * Defines processor clock - important for correct timings concerning serial
 * interface etc.
 */

#define CFG_SYS_CLK			80000000
#define CFG_SYS_CPU_CLK		(CFG_SYS_CLK * 3)
#define CFG_SYS_SDRAM_SIZE		32		/* SDRAM size in MB */

/*
 * Define baudrate for UART1 (console output, tftp, ...)
 * default value of CONFIG_BAUDRATE for Sentec board: 19200 baud
 * CFG_SYS_BAUDRATE_TABLE defines values that can be selected
 * in u-boot command interface
 */

#define CFG_SYS_UART_PORT		(2)
#define CFG_SYS_UART2_ALT3_GPIO

/* here we put our FPGA configuration... */

/* Define user parameters that have to be customized most likely */

/* AUTOBOOT settings - booting images automatically by u-boot after power on */

/*
 * The following settings will be contained in the environment block ; if you
 * want to use a neutral environment all those settings can be manually set in
 * u-boot: 'set' command
 */

#define CFG_EXTRA_ENV_SETTINGS			\
	"loaderversion=11\0"				\
	"card_id="__stringify(ASTRO_ID)"\0"			\
	"alterafile=0\0"				\
	"xilinxfile=0\0"				\
	"xilinxload=imxtract 0x540000 $xilinxfile 0x41000000&&"\
		"fpga load 0 0x41000000 $filesize\0" \
	"alteraload=imxtract 0x6c0000 $alterafile 0x41000000&&"\
		"fpga load 1 0x41000000 $filesize\0" \
	"env_default=1\0"				\
	"env_check=if test $env_default -eq 1;"\
		" then setenv env_default 0;saveenv;fi\0"

/*
 * "update" is a non-standard command that has to be supplied
 * by external update.c; This is not included in mainline because
 * it needs non-blocking CFI routines.
 */

#define CFG_SYS_FPGA_WAIT		1000

/* End of user parameters to be customized */

/* Defines memory range for test */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/* Base register address */

#define CFG_SYS_MBAR		0xFC000000	/* Register Base Addrs */

/* System Conf. Reg. & System Protection Reg. */

#define CFG_SYS_SCR		0x0003;
#define CFG_SYS_SPR		0xffff;

/*
 * Definitions for initial stack pointer and data area (in internal SRAM)
 */
#define CFG_SYS_INIT_RAM_ADDR	0x80000000
#define CFG_SYS_INIT_RAM_SIZE		0x8000
#define CFG_SYS_INIT_RAM_CTRL	0x221

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * for MCF5373, the allowable range is 0x40000000 to 0x7FF00000
 */
#define CFG_SYS_SDRAM_BASE		0x40000000

/*
 * Chipselect bank definitions
 *
 * CS0 - Flash 32MB (first 16MB)
 * CS1 - Flash 32MB (second half)
 * CS2 - FPGA
 * CS3 - FPGA
 * CS4 - unused
 * CS5 - unused
 */
#define CFG_SYS_CS0_BASE		0
#define CFG_SYS_CS0_MASK		0x00ff0001
#define CFG_SYS_CS0_CTRL		0x00001fc0

#define CFG_SYS_CS1_BASE		0x01000000
#define CFG_SYS_CS1_MASK		0x00ff0001
#define CFG_SYS_CS1_CTRL		0x00001fc0

#define CFG_SYS_CS2_BASE		0x20000000
#define CFG_SYS_CS2_MASK		0x00ff0001
#define CFG_SYS_CS2_CTRL		0x0000fec0

#define CFG_SYS_CS3_BASE		0x21000000
#define CFG_SYS_CS3_MASK		0x00ff0001
#define CFG_SYS_CS3_CTRL		0x0000fec0

#define CFG_SYS_FLASH_BASE		0x00000000

/* Reserve 256 kB for Monitor */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CFG_SYS_BOOTMAPSZ		(CFG_SYS_SDRAM_BASE + \
						(CFG_SYS_SDRAM_SIZE << 20))

/* FLASH organization */

#define CFG_SYS_FLASH_SIZE		0x2000000

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*)

/* Cache Configuration */

#define ICACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CFG_SYS_INIT_RAM_ADDR + \
					 CFG_SYS_INIT_RAM_SIZE - 4)
#define CFG_SYS_ICACHE_INV		(CF_CACR_CINVA)
#define CFG_SYS_CACHE_ACR0		(CFG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CFG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CFG_SYS_CACHE_ICACR		(CF_CACR_EC | CF_CACR_CINVA | \
					 CF_CACR_DCM_P)


#endif	/* _CONFIG_ASTRO_MCF5373L_H */
