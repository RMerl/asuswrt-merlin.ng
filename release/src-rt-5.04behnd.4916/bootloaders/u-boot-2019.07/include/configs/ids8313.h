/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (c) 2011 IDS GmbH, Germany
 * Sergej Stepanov <ste@ids.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_FSL_ELBC

#define CONFIG_BOOT_RETRY_TIME		900
#define CONFIG_BOOT_RETRY_MIN		30
#define CONFIG_RESET_TO_RETRY

#define CONFIG_SYS_SICRH	0x00000000
#define CONFIG_SYS_SICRL	(SICRL_LBC | SICRL_SPI_D)

#define CONFIG_HWCONFIG

/*
 * Definitions for initial stack pointer and data area (in DCACHE )
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000  /* End of used area in DPRAM */
#define CONFIG_SYS_GBL_DATA_SIZE	0x100
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
					 - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Internal Definitions
 */
/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

/*
 * Manually set up DDR parameters,
 * as this board has not the SPD connected to I2C.
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_CONFIG		(CSCONFIG_EN |\
					 0x00010000 |\
					 CSCONFIG_ROW_BIT_13 |\
					 CSCONFIG_COL_BIT_10)

#define CONFIG_SYS_DDR_CONFIG_256	(CONFIG_SYS_DDR_CONFIG | \
					 CSCONFIG_BANK_BIT_3)

#define CONFIG_SYS_DDR_TIMING_3	(1 << 16)	/* ext refrec */
#define CONFIG_SYS_DDR_TIMING_0	((3 << TIMING_CFG0_RWT_SHIFT) |\
				(3 << TIMING_CFG0_WRT_SHIFT) |\
				(3 << TIMING_CFG0_RRT_SHIFT) |\
				(3 << TIMING_CFG0_WWT_SHIFT) |\
				(6 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) |\
				(2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) |\
				(8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				(2 << TIMING_CFG0_MRS_CYC_SHIFT))
#define CONFIG_SYS_DDR_TIMING_1	((4 << TIMING_CFG1_PRETOACT_SHIFT) |\
				(12 << TIMING_CFG1_ACTTOPRE_SHIFT) |\
				(4 << TIMING_CFG1_ACTTORW_SHIFT) |\
				(7 << TIMING_CFG1_CASLAT_SHIFT) |\
				(4 << TIMING_CFG1_REFREC_SHIFT) |\
				(4 << TIMING_CFG1_WRREC_SHIFT) |\
				(2 << TIMING_CFG1_ACTTOACT_SHIFT) |\
				(2 << TIMING_CFG1_WRTORD_SHIFT))
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) |\
				(5 << TIMING_CFG2_CPO_SHIFT) |\
				(4 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) |\
				(2 << TIMING_CFG2_RD_TO_PRE_SHIFT) |\
				(0 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) |\
				(1 << TIMING_CFG2_CKE_PLS_SHIFT) |\
				(6 << TIMING_CFG2_FOUR_ACT_SHIFT))

#define CONFIG_SYS_DDR_INTERVAL	((0x800 << SDRAM_INTERVAL_REFINT_SHIFT) |\
				(0x800 << SDRAM_INTERVAL_BSTOPRE_SHIFT))

#define CONFIG_SYS_SDRAM_CFG		(SDRAM_CFG_SREN |\
					 SDRAM_CFG_2T_EN | SDRAM_CFG_HSE |\
					 SDRAM_CFG_DBW_32 |\
					 SDRAM_CFG_SDRAM_TYPE_DDR2)

#define CONFIG_SYS_SDRAM_CFG2		0x00401000
#define CONFIG_SYS_DDR_MODE		((0x0448 << SDRAM_MODE_ESD_SHIFT) |\
					 (0x0242 << SDRAM_MODE_SD_SHIFT))
#define CONFIG_SYS_DDR_MODE_2		0x00000000
#define CONFIG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN |\
					 DDRCDR_PZ_NOMZ |\
					 DDRCDR_NZ_NOMZ |\
					 DDRCDR_ODT |\
					 DDRCDR_M_ODR |\
					 DDRCDR_Q_DRN)

/*
 * on-board devices
 */
#define CONFIG_TSEC1
#define CONFIG_TSEC2

/*
 * NOR FLASH setup
 */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_FLASH_SHOW_PROGRESS	50

#define CONFIG_SYS_FLASH_BASE		0xFF800000
#define CONFIG_SYS_FLASH_SIZE		8


#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

/*
 * NAND FLASH setup
 */
#define CONFIG_SYS_NAND_BASE		0xE1000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_NAND_FSL_ELBC
#define CONFIG_SYS_NAND_PAGE_SIZE	(2048)
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 << 10)
#define NAND_CACHE_PAGES		64


/*
 * MRAM setup
 */
#define CONFIG_SYS_MRAM_BASE		0xE2000000
#define CONFIG_SYS_MRAM_SIZE		0x20000	/* 128 Kb */

#define CONFIG_SYS_OR_TIMING_MRAM


/*
 * CPLD setup
 */
#define CONFIG_SYS_CPLD_BASE		0xE3000000
#define CONFIG_SYS_CPLD_SIZE		0x8000

#define CONFIG_SYS_OR_TIMING_MRAM


/*
 * HW-Watchdog
 */
#define CONFIG_WATCHDOG		1
#define CONFIG_SYS_WATCHDOG_VALUE	0xFFFF

/*
 * I2C setup
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3100
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR	0x51

/*
 * Ethernet setup
 */
#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME		"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR			0x1
#define TSEC1_FLAGS			TSEC_GIGABIT
#define TSEC1_PHYIDX			0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME		"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define TSEC2_PHY_ADDR			0x3
#define TSEC2_FLAGS			TSEC_GIGABIT
#define TSEC2_PHYIDX			0
#endif
#define CONFIG_ETHPRIME		"TSEC1"

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0))

#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_SYS_SCCR_USBDRCM	3

/*
 * U-Boot environment setup
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(8 * 1024 * 1024)

/*
 * Environment Configuration
 */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE \
				+ CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_NETDEV			eth1
#define CONFIG_HOSTNAME		"ids8313"
#define CONFIG_ROOTPATH		"/opt/eldk-4.2/ppc_6xx"
#define CONFIG_BOOTFILE		"ids8313/uImage"
#define CONFIG_UBOOTPATH		"ids8313/u-boot.bin"
#define CONFIG_FDTFILE			"ids8313/ids8313.dtb"
#define CONFIG_LOADADDR		0x400000
#define CONFIG_ENV_FLAGS_LIST_STATIC "ethaddr:mo,eth1addr:mo"

/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ		(256 << 20)

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x00C00000

#define CONFIG_SYS_LOAD_ADDR		0x100000
#define CONFIG_LOADS_ECHO
#define CONFIG_TIMESTAMP
#define CONFIG_PREBOOT			"echo;" \
					"echo Type \\\"run nfsboot\\\" " \
					"to mount root filesystem over NFS;echo"
#define CONFIG_BOOTCOMMAND		"run boot_cramfs"
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_DEV		"0"

/* mtdparts command line support */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" __stringify(CONFIG_NETDEV) "\0"			\
	"ethprime=TSEC1\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
	"tftpflash=tftpboot ${loadaddr} ${uboot}; "			\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" +${filesize}; "					\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +${filesize}; "					\
		"cp.b ${loadaddr} " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" ${filesize}; "					\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +${filesize}; "					\
		"cmp.b ${loadaddr} " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" ${filesize}\0"					\
	"console=ttyS0\0"						\
	"fdtaddr=0x780000\0"						\
	"kernel_addr=ff800000\0"					\
	"fdtfile=" __stringify(CONFIG_FDTFILE) "\0"			\
	"setbootargs=setenv bootargs "					\
		"root=${rootdev} rw console=${console},"		\
			"${baudrate} ${othbootargs}\0"			\
	"setipargs=setenv bootargs root=${rootdev} rw "			\
			"nfsroot=${serverip}:${rootpath} "		\
			"ip=${ipaddr}:${serverip}:${gatewayip}:"	\
			"${netmask}:${hostname}:${netdev}:off "		\
			"console=${console},${baudrate} ${othbootargs}\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"					\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"				\
	"\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setipargs;run addmtd;"					\
	"tftp ${loadaddr} ${bootfile};"				\
	"tftp ${fdtaddr} ${fdtfile};"					\
	"fdt addr ${fdtaddr};"						\
	"bootm ${loadaddr} - ${fdtaddr}"

/* UBI Support */

#endif	/* __CONFIG_H */
