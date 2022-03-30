/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
/*
 * ve8313 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1

#define CONFIG_PCI_INDIRECT_BRIDGE 1
#define CONFIG_FSL_ELBC		1

/*
 * On-board devices
 *
 */
#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x07000000

/*
 * Device configurations
 */

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory*/

/*
 * Manually set up DDR parameters, as this board does not
 * have the SPD connected to I2C.
 */
#define CONFIG_SYS_DDR_SIZE	128	/* MB */
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
				| CSCONFIG_AP \
				| CSCONFIG_ODT_RD_NEVER \
				| CSCONFIG_ODT_WR_ALL \
				| CSCONFIG_ROW_BIT_13 \
				| CSCONFIG_COL_BIT_10)
				/* 0x80840102 */

#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (3 << TIMING_CFG0_RRT_SHIFT) \
				| (2 << TIMING_CFG0_WWT_SHIFT) \
				| (7 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x0e720802 */
#define CONFIG_SYS_DDR_TIMING_1	((2 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (6 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (2 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (6 << TIMING_CFG1_REFREC_SHIFT) \
				| (2 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x26256222 */
#define CONFIG_SYS_DDR_TIMING_2	((0 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (5 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (1 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (7 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x029028c7 */
#define CONFIG_SYS_DDR_INTERVAL	((0x320 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0x2000 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x03202000 */
#define CONFIG_SYS_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32)
				/* 0x43080000 */
#define CONFIG_SYS_SDRAM_CFG2	0x00401000
#define CONFIG_SYS_DDR_MODE	((0x4440 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0232 << SDRAM_MODE_SD_SHIFT))
				/* 0x44400232 */
#define CONFIG_SYS_DDR_MODE_2	0x8000C000

#define CONFIG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
				/*0x02000000*/
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN \
				| DDRCDR_PZ_NOMZ \
				| DDRCDR_NZ_NOMZ \
				| DDRCDR_M_ODR)
				/* 0x73000002 */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_SIZE		32	/* size in MB */
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* display empty sectors */

#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256		/* sectors per dev */

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)

/*
 * NAND settings
 */
#define CONFIG_SYS_NAND_BASE		0x61000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_FSL_ELBC 1
#define CONFIG_SYS_NAND_BLOCK_SIZE 16384



/* Still needed for spl_minimal.c */
#define CONFIG_SYS_NAND_BR_PRELIM CONFIG_SYS_BR1_PRELIM
#define CONFIG_SYS_NAND_OR_PRELIM CONFIG_SYS_OR1_PRELIM



/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

#if defined(CONFIG_PCI)
/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00100000	/* 1M */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#endif

/*
 * TSEC
 */

#define CONFIG_TSEC1
#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME	"TSEC1"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x01
#define TSEC1_FLAGS		0
#define TSEC1_PHYIDX		0
#endif

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC1"

/*
 * Environment
 */
#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#define CONFIG_ENV_SIZE		0x4000
/* Address and size of Redundant Environment Sector */
#define CONFIG_ENV_OFFSET_REDUND	\
			(CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */

#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot arg Buffer size */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

/* System IO Config */
#define CONFIG_SYS_SICRH	(0x01000000 | \
				SICRH_ETSEC2_B | \
				SICRH_ETSEC2_C | \
				SICRH_ETSEC2_D | \
				SICRH_ETSEC2_E | \
				SICRH_ETSEC2_F | \
				SICRH_ETSEC2_G | \
				SICRH_TSOBI1 | \
				SICRH_TSOBI2)
				/* 0x010fff03 */
#define CONFIG_SYS_SICRL	(SICRL_LBC | \
				SICRL_SPI_A | \
				SICRL_SPI_B | \
				SICRL_SPI_C | \
				SICRL_SPI_D | \
				SICRL_ETSEC2_A)
				/* 0x33fc0003) */

#define CONFIG_NETDEV		eth0

#define CONFIG_HOSTNAME		"ve8313"
#define CONFIG_UBOOTPATH	ve8313/u-boot.bin

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" __stringify(CONFIG_NETDEV) "\0"			\
	"ethprime=" __stringify(CONFIG_TSEC1_NAME) "\0"			\
	"u-boot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
	"u-boot_addr_r=100000\0"					\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off " __stringify(CONFIG_SYS_FLASH_BASE)	\
		" +${filesize};"	\
	"erase " __stringify(CONFIG_SYS_FLASH_BASE) " +${filesize};"	\
	"cp.b ${u-boot_addr_r} " __stringify(CONFIG_SYS_FLASH_BASE)	\
	" ${filesize};"							\
	"protect on " __stringify(CONFIG_SYS_FLASH_BASE) " +${filesize}\0" \

#endif	/* __CONFIG_H */
