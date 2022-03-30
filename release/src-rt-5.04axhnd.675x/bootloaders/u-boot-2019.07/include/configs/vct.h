/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

/*
 * This file contains the configuration parameters for the VCT board
 * family:
 *
 * vct_premium
 * vct_premium_small
 * vct_premium_onenand
 * vct_premium_onenand_small
 * vct_platinum
 * vct_platinum_small
 * vct_platinum_onenand
 * vct_platinum_onenand_small
 * vct_platinumavc
 * vct_platinumavc_small
 * vct_platinumavc_onenand
 * vct_platinumavc_onenand_small
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CPU_CLOCK_RATE			324000000 /* Clock for the MIPS core */
#define CONFIG_SYS_MIPS_TIMER_FREQ	(CPU_CLOCK_RATE / 2)

#define CONFIG_SKIP_LOWLEVEL_INIT	/* SDRAM is initialized by the bootstrap code */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 << 10)
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

#if !defined(CONFIG_VCT_NAND) && !defined(CONFIG_VCT_ONENAND)
#define CONFIG_VCT_NOR
#endif

/*
 * UART
 */
#ifdef CONFIG_VCT_PLATINUMAVC
#define UART_1_BASE		0xBDC30000
#else
#define UART_1_BASE		0xBF89C000
#endif

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		UART_1_BASE
#define CONFIG_SYS_NS16550_CLK		921600

/*
 * SDRAM
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_MBYTES_SDRAM		128
#define CONFIG_SYS_MEMTEST_START	0x80200000
#define CONFIG_SYS_MEMTEST_END		0x80400000
#define CONFIG_SYS_LOAD_ADDR		0x80400000	/* default load address */

#if defined(CONFIG_VCT_PREMIUM) || defined(CONFIG_VCT_PLATINUM)
#define CONFIG_NET_RETRY_COUNT		20
#endif

/*
 * Commands
 */
#if defined(CONFIG_CMD_USB)

/*
 * USB/EHCI
 */
#define CONFIG_USB_EHCI_VCT		/* on VCT platform		*/
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_IS_TDI
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET /* re-init HCD after CMD_RESET */
#endif /* CONFIG_CMD_USB */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size	*/
#define CONFIG_TIMESTAMP			/* Print image info with timestamp */

/*
 * FLASH and environment organization
 */
#if defined(CONFIG_VCT_NOR)
#define CONFIG_FLASH_NOT_MEM_MAPPED

/*
 * We need special accessor functions for the CFI FLASH driver. This
 * can be enabled via the CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS option.
 */
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS

/*
 * For the non-memory-mapped NOR FLASH, we need to define the
 * NOR FLASH area. This can't be detected via the addr2info()
 * function, since we check for flash access in the very early
 * U-Boot code, before the NOR FLASH is detected.
 */
#define CONFIG_FLASH_BASE		0xb0000000
#define CONFIG_FLASH_END		0xbfffffff

/*
 * CFI driver settings
 */
#define CONFIG_SYS_FLASH_CFI_AMD_RESET	1	/* Use AMD (Spansion) reset cmd */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT	/* no byte writes on IXP4xx	*/

#define CONFIG_SYS_FLASH_BASE		0xb0000000
#define CONFIG_SYS_FLASH_BANKS_LIST    { CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000		/* size of one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */
#endif /* CONFIG_VCT_NOR */

#if defined(CONFIG_VCT_ONENAND)
#define CONFIG_USE_ONENAND_BOARD_INIT
#define	CONFIG_SYS_ONENAND_BASE		0x00000000	/* this is not real address */
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_ENV_ADDR			(128 << 10)	/* after compr. U-Boot image */
#define	CONFIG_ENV_SIZE			(128 << 10)	/* erase size */
#endif /* CONFIG_VCT_ONENAND */

/*
 * I2C/EEPROM
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	83000	/* 83 kHz is supposed to work */
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7f

/*
 * Software (bit-bang) I2C driver configuration
 */
#define CONFIG_SYS_GPIO_I2C_SCL		11
#define CONFIG_SYS_GPIO_I2C_SDA		10

#ifndef __ASSEMBLY__
int vct_gpio_dir(int pin, int dir);
void vct_gpio_set(int pin, int val);
int vct_gpio_get(int pin);
#endif

#define I2C_INIT	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SCL, 1)
#define I2C_ACTIVE	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SDA, 1)
#define I2C_TRISTATE	vct_gpio_dir(CONFIG_SYS_GPIO_I2C_SDA, 0)
#define I2C_READ	vct_gpio_get(CONFIG_SYS_GPIO_I2C_SDA)
#define I2C_SDA(bit)	vct_gpio_set(CONFIG_SYS_GPIO_I2C_SDA, bit)
#define I2C_SCL(bit)	vct_gpio_set(CONFIG_SYS_GPIO_I2C_SCL, bit)
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
/* CAT24WC32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* Bytes of address		*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5	/* The Catalyst CAT24WC32 has	*/
					/* 32 byte page write mode using*/
					/* last 5 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10   /* and takes up to 10 msec */

#define CONFIG_BOOTCOMMAND	"run test3"

/*
 * UBI configuration
 */

/*
 * We need a small, stripped down image to fit into the first 128k OneNAND
 * erase block (gzipped). This image only needs basic commands for FLASH
 * (NOR/OneNAND) usage and Linux kernel booting.
 */
#if defined(CONFIG_VCT_SMALL_IMAGE)
#undef CONFIG_SYS_I2C_SOFT
#undef CONFIG_SOURCE
#undef CONFIG_TIMESTAMP
#endif /* CONFIG_VCT_SMALL_IMAGE */

#endif  /* __CONFIG_H */
