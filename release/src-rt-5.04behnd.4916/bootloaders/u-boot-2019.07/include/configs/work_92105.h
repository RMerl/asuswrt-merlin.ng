/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * WORK Microwave work_92105 board configuration file
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#ifndef __CONFIG_WORK_92105_H__
#define __CONFIG_WORK_92105_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Define work_92105 machine type by hand -- done only for compatibility
 * with original board code
 */
#define CONFIG_MACH_TYPE		736

#if !defined(CONFIG_SPL_BUILD)
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * Memory configurations
 */
#define CONFIG_SYS_MALLOC_LEN		SZ_1M
#define CONFIG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_128M
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + SZ_32K)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - SZ_1M)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_32K)

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + SZ_512K \
					 - GENERATED_GBL_DATA_SIZE)

/*
 * Serial Driver
 */
#define CONFIG_SYS_LPC32XX_UART		5   /* UART5 - NS16550 */

/*
 * Ethernet Driver
 */

#define CONFIG_PHY_SMSC
#define CONFIG_LPC32XX_ETH
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
/* FIXME: remove "Waiting for PHY auto negotiation to complete..." message */

/*
 * I2C driver
 */

#define CONFIG_SYS_I2C_LPC32XX
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SPEED 350000

/*
 * I2C EEPROM
 */

#define CONFIG_SYS_I2C_EEPROM_ADDR 0x56
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2

/*
 * I2C RTC
 */

#define CONFIG_RTC_DS1374

/*
 * U-Boot General Configurations
 */
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * NAND chip timings for FIXME: which one?
 */

#define CONFIG_LPC32XX_NAND_MLC_TCEA_DELAY  333333333
#define CONFIG_LPC32XX_NAND_MLC_BUSY_DELAY   10000000
#define CONFIG_LPC32XX_NAND_MLC_NAND_TA      18181818
#define CONFIG_LPC32XX_NAND_MLC_RD_HIGH      31250000
#define CONFIG_LPC32XX_NAND_MLC_RD_LOW       45454545
#define CONFIG_LPC32XX_NAND_MLC_WR_HIGH      40000000
#define CONFIG_LPC32XX_NAND_MLC_WR_LOW       83333333

/*
 * NAND
 */

/* driver configuration */
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_MAX_NAND_CHIPS 1
#define CONFIG_SYS_NAND_BASE MLC_NAND_BASE
#define CONFIG_NAND_LPC32XX_MLC

/*
 * GPIO
 */

#define CONFIG_LPC32XX_GPIO

/*
 * SSP/SPI/DISPLAY
 */

#define CONFIG_LPC32XX_SSP_TIMEOUT 100000
/*
 * Environment
 */

#define CONFIG_ENV_SIZE			0x00020000
#define CONFIG_ENV_OFFSET		0x00100000
#define CONFIG_ENV_OFFSET_REDUND	0x00120000
#define CONFIG_ENV_ADDR			0x80000100

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			0x80008000

/*
 * SPL
 */

/* SPL will be executed at offset 0 */
/* SPL will use SRAM as stack */
#define CONFIG_SPL_STACK     0x0000FFF8
/* Use the framework and generic lib */
/* SPL will use serial */
/* SPL will load U-Boot from NAND offset 0x40000 */
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS  0x00040000
#define CONFIG_SPL_PAD_TO 0x20000
/* U-Boot will be 0x40000 bytes, loaded and run at CONFIG_SYS_TEXT_BASE */
#define CONFIG_SYS_MONITOR_LEN 0x40000 /* actually, MAX size */
#define CONFIG_SYS_NAND_U_BOOT_START CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST   CONFIG_SYS_TEXT_BASE

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_WORK_92105_H__*/
