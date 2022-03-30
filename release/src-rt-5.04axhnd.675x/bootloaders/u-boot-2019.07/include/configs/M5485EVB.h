/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF5485 FireEngine board.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5485EVB_H
#define _M5485EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)

#undef CONFIG_HW_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT	5000	/* timeout in milliseconds, max timeout is 6.71sec */

#define CONFIG_SLTTMR

#define CONFIG_FSLDMAFEC
#ifdef CONFIG_FSLDMAFEC
#	define CONFIG_MII_INIT		1
#	define CONFIG_HAS_ETH1

#	define CONFIG_SYS_DMA_USE_INTSRAM	1
#	define CONFIG_SYS_DISCOVER_PHY
#	define CONFIG_SYS_RX_ETH_BUFFER	32
#	define CONFIG_SYS_TX_ETH_BUFFER	48
#	define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#	define CONFIG_SYS_FEC0_PINMUX		0
#	define CONFIG_SYS_FEC0_MIIBASE		CONFIG_SYS_FEC0_IOBASE
#	define CONFIG_SYS_FEC1_PINMUX		0
#	define CONFIG_SYS_FEC1_MIIBASE		CONFIG_SYS_FEC0_IOBASE

#	define MCFFEC_TOUT_LOOP		50000
/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CONFIG_SYS_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#			define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CONFIG_SYS_DISCOVER_PHY */

#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1

#endif

#ifdef CONFIG_CMD_USB
#	define CONFIG_USB_OHCI_NEW
/*#	define CONFIG_PCI_OHCI*/
#	define CONFIG_SYS_USB_OHCI_REGS_BASE		0x80041000
#	define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#	define CONFIG_SYS_USB_OHCI_SLOT_NAME		"isp1561"
#	define CONFIG_SYS_OHCI_SWAP_REG_ACCESS
#endif

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	80000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x00008F00
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

/* PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_SYS_PCI_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS	CONFIG_SYS_PCI_MEM_BUS
#define CONFIG_SYS_PCI_MEM_SIZE	0x10000000

#define CONFIG_SYS_PCI_IO_BUS		0x71000000
#define CONFIG_SYS_PCI_IO_PHYS		CONFIG_SYS_PCI_IO_BUS
#define CONFIG_SYS_PCI_IO_SIZE		0x01000000

#define CONFIG_SYS_PCI_CFG_BUS		0x70000000
#define CONFIG_SYS_PCI_CFG_PHYS	CONFIG_SYS_PCI_CFG_BUS
#define CONFIG_SYS_PCI_CFG_SIZE	0x01000000
#endif

#define CONFIG_UDP_CHECKSUM

#define CONFIG_HOSTNAME		"M548xEVB"
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"loadaddr=10000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off bank 1;"			\
	"era ff800000 ff83ffff;"		\
	"cp.b ${loadaddr} ff800000 ${filesize};"\
	"save\0"				\
	""

#define CONFIG_PRAM		512	/* 512 KB */

#define CONFIG_SYS_LOAD_ADDR		0x00010000

#define CONFIG_SYS_CLK			CONFIG_SYS_BUSCLK
#define CONFIG_SYS_CPU_CLK		CONFIG_SYS_CLK * 2

#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_INTSRAM		(CONFIG_SYS_MBAR + 0x10000)
#define CONFIG_SYS_INTSRAMSZ		0x8000

/*#define CONFIG_SYS_LATCH_ADDR		(CONFIG_SYS_CS1_BASE + 0x80000)*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0xF2000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM_CTRL	0x21
#define CONFIG_SYS_INIT_RAM1_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
#define CONFIG_SYS_INIT_RAM1_END	0x1000	/* End of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM1_CTRL	0x21
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE) - 0x10)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_CFG1		0x73711630
#define CONFIG_SYS_SDRAM_CFG2		0x46770000
#define CONFIG_SYS_SDRAM_CTRL		0xE10B0000
#define CONFIG_SYS_SDRAM_EMOD		0x40010000
#define CONFIG_SYS_SDRAM_MODE		0x018D0000
#define CONFIG_SYS_SDRAM_DRVSTRENGTH	0x000002AA
#ifdef CONFIG_SYS_DRAMSZ1
#	define CONFIG_SYS_SDRAM_SIZE	(CONFIG_SYS_DRAMSZ + CONFIG_SYS_DRAMSZ1)
#else
#	define CONFIG_SYS_SDRAM_SIZE	CONFIG_SYS_DRAMSZ
#endif

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE + 0x400
#define CONFIG_SYS_MEMTEST_END		((CONFIG_SYS_SDRAM_SIZE - 3) << 20)

#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024

/* Reserve 256 kB for malloc() */
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)
/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI
#	define CONFIG_SYS_FLASH_BASE		(CONFIG_SYS_CS0_BASE)
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#ifdef CONFIG_SYS_NOR1SZ
#	define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks */
#	define CONFIG_SYS_FLASH_SIZE		((CONFIG_SYS_NOR1SZ + CONFIG_SYS_BOOTSZ) << 20)
#	define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_CS0_BASE, CONFIG_SYS_CS1_BASE }
#else
#	define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#	define CONFIG_SYS_FLASH_SIZE		(CONFIG_SYS_BOOTSZ << 20)
#endif
#endif

/* Configuration for environment
 * Environment is not embedded in u-boot. First time runing may have env
 * crc error warning if there is no correct environment on the flash.
 */
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_SECT_SIZE	0x10000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_BCINVA + CF_CACR_ICINVA + \
					 CF_CACR_IDCM)
#define CONFIG_SYS_DCACHE_INV		(CF_CACR_DCINVA)
#define CONFIG_SYS_CACHE_ACR2		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_BEC | CF_CACR_BCINVA | \
					 CF_CACR_IEC | CF_CACR_ICINVA)
#define CONFIG_SYS_CACHE_DCACR		((CONFIG_SYS_CACHE_ICACR | \
					 CF_CACR_DEC | CF_CACR_DDCM_P | \
					 CF_CACR_DCINVA) & ~CF_CACR_ICINVA)

/*-----------------------------------------------------------------------
 * Chipselect bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - NOR Flash
 * CS2 - Available
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 */
#define CONFIG_SYS_CS0_BASE		0xFF800000
#define CONFIG_SYS_CS0_MASK		(((CONFIG_SYS_BOOTSZ << 20) - 1) & 0xFFFF0001)
#define CONFIG_SYS_CS0_CTRL		0x00101980

#ifdef CONFIG_SYS_NOR1SZ
#define CONFIG_SYS_CS1_BASE		0xE0000000
#define CONFIG_SYS_CS1_MASK		(((CONFIG_SYS_NOR1SZ << 20) - 1) & 0xFFFF0001)
#define CONFIG_SYS_CS1_CTRL		0x00101D80
#endif

#endif				/* _M5485EVB_H */
