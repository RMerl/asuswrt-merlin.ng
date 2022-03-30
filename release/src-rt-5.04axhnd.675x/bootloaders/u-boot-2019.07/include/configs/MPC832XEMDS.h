/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 family */

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRL		0x00000000

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE	0x00000000	/* DDR is system memory */
#define CONFIG_SYS_DDRCDR	0x73000002	/* DDR II voltage is 1.8V */

#undef CONFIG_SPD_EEPROM
#if defined(CONFIG_SPD_EEPROM)
/* Determine DDR configuration from I2C interface
 */
#define SPD_EEPROM_ADDRESS	0x51	/* DDR SODIMM */
#else
/* Manually set up DDR parameters
 */
#define CONFIG_SYS_DDR_SIZE		128	/* MB */
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_AP \
					| CSCONFIG_ODT_WR_CFG \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)
					/* 0x80840102 */
#define CONFIG_SYS_DDR_TIMING_0		((0 << TIMING_CFG0_RWT_SHIFT) \
					| (0 << TIMING_CFG0_WRT_SHIFT) \
					| (0 << TIMING_CFG0_RRT_SHIFT) \
					| (0 << TIMING_CFG0_WWT_SHIFT) \
					| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
					| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
					| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
					| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
					/* 0x00220802 */
#define CONFIG_SYS_DDR_TIMING_1		((3 << TIMING_CFG1_PRETOACT_SHIFT) \
					| (9 << TIMING_CFG1_ACTTOPRE_SHIFT) \
					| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
					| (5 << TIMING_CFG1_CASLAT_SHIFT) \
					| (13 << TIMING_CFG1_REFREC_SHIFT) \
					| (3 << TIMING_CFG1_WRREC_SHIFT) \
					| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
					| (2 << TIMING_CFG1_WRTORD_SHIFT))
					/* 0x3935D322 */
#define CONFIG_SYS_DDR_TIMING_2		((0 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (31 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (10 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x0F9048CA */
#define CONFIG_SYS_DDR_TIMING_3		0x00000000
#define CONFIG_SYS_DDR_CLK_CNTL		DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
					/* 0x02000000 */
#define CONFIG_SYS_DDR_MODE		((0x4440 << SDRAM_MODE_ESD_SHIFT) \
					| (0x0232 << SDRAM_MODE_SD_SHIFT))
					/* 0x44400232 */
#define CONFIG_SYS_DDR_MODE2		0x8000c000
#define CONFIG_SYS_DDR_INTERVAL		((800 << SDRAM_INTERVAL_REFINT_SHIFT) \
					| (100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
					/* 0x03200064 */
#define CONFIG_SYS_DDR_CS0_BNDS		0x00000007
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
					| SDRAM_CFG_SDRAM_TYPE_DDR2 \
					| SDRAM_CFG_32_BE)
					/* 0x43080000 */
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#endif

/*
 * Memory test
 */
#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(512 * 1024)	/* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)	/* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE	0xFE000000	/* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE	16	/* FLASH size is 16M */


#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM

/*
 * BCSR on the Local Bus
 */
#define CONFIG_SYS_BCSR			0xF8000000
					/* Access window base at BCSR base */


/*
 * Windows to access PIB via local bus
 */
					/* PIB window base 0xF8008000 */
#define CONFIG_SYS_PIB_BASE		0xF8008000
#define CONFIG_SYS_PIB_WINDOW_SIZE	(32 * 1024)

/*
 * CS2 on Local Bus, to PIB
 */


/*
 * CS3 on Local Bus, to PIB
 */


/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x51} }

/*
 * Config on-board RTC
 */
#define CONFIG_RTC_DS1374		/* use ds1374 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68	/* at address 0x68 */

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
#define CONFIG_SYS_PCI1_IO_PHYS		0xE0300000
#define CONFIG_SYS_PCI1_IO_SIZE		0x100000	/* 1M */

#define CONFIG_SYS_PCI_SLV_MEM_LOCAL	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_PCI_SLV_MEM_BUS	0x00000000
#define CONFIG_SYS_PCI_SLV_MEM_SIZE	0x80000000

#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_83XX_PCI_STREAMING

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID	0x1957	/* Freescale */

#endif	/* CONFIG_PCI */

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"UEC0"

#define CONFIG_UEC_ETH1		/* ETH3 */

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	2	/* UCC3 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK9
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK10
#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	3
#define CONFIG_SYS_UEC1_INTERFACE_TYPE	PHY_INTERFACE_MODE_MII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100
#endif

#define CONFIG_UEC_ETH2		/* ETH4 */

#ifdef CONFIG_UEC_ETH2
#define CONFIG_SYS_UEC2_UCC_NUM	3	/* UCC4 */
#define CONFIG_SYS_UEC2_RX_CLK		QE_CLK7
#define CONFIG_SYS_UEC2_TX_CLK		QE_CLK8
#define CONFIG_SYS_UEC2_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC2_PHY_ADDR	4
#define CONFIG_SYS_UEC2_INTERFACE_TYPE	PHY_INTERFACE_MODE_MII
#define CONFIG_SYS_UEC2_INTERFACE_SPEED	100
#endif

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x20000
	#define CONFIG_ENV_SIZE		0x2000
#else
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
					/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ		(256 << 20)
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */ #define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS0\0"						\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=ramfs.83xx\0"					\
	"fdtaddr=780000\0"						\
	"fdtfile=mpc832x_mds.dtb\0"					\
	""

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
		"nfsroot=$serverip:$rootpath "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
