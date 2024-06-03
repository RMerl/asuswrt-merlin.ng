/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_NAND_U_BOOT_SIZE  (512 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST   0x00100000
#define CONFIG_SYS_NAND_U_BOOT_START 0x00100100
#define CONFIG_SYS_NAND_U_BOOT_OFFS  16384
#define CONFIG_SYS_NAND_U_BOOT_RELOC 0x00010000

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH		0x00000000
#define CONFIG_SYS_SICRL		0x00000000 /* 3.3V, no delay */

#define CONFIG_HWCONFIG

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN \
				| DDRCDR_PZ_LOZ \
				| DDRCDR_NZ_LOZ \
				| DDRCDR_ODT \
				| DDRCDR_Q_DRN)
				/* 0x7b880001 */
/*
 * Manually set up DDR parameters
 * consist of two chips HY5PS12621BFP-C4 from HYNIX
 */
#define CONFIG_SYS_DDR_SIZE		128 /* MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x00000007
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
				| CSCONFIG_ODT_RD_NEVER \
				| CSCONFIG_ODT_WR_ONLY_CURRENT \
				| CSCONFIG_ROW_BIT_13 \
				| CSCONFIG_COL_BIT_10)
				/* 0x80010102 */
#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00220802 */
#define CONFIG_SYS_DDR_TIMING_1	((2 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (7 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (2 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (6 << TIMING_CFG1_REFREC_SHIFT) \
				| (2 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x27256222 */
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (4 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (5 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x121048c5 */
#define CONFIG_SYS_DDR_INTERVAL	((0x0360 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x03600100 */
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32)
				/* 0x43080000 */
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000 /* 1 posted refresh */
#define CONFIG_SYS_DDR_MODE		((0x0448 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0232 << SDRAM_MODE_SD_SHIFT))
				/* ODT 150ohm CL=3, AL=1 on SDRAM */
#define CONFIG_SYS_DDR_MODE2	0x00000000

/*
 * Memory test
 */
#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00040000 /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00140000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_LEN	(512 * 1024) /* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define CONFIG_FSL_ELBC

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE		8	/* FLASH size is 8M */

#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
/* 127 64KB sectors and 8 8KB top sectors per device */
#define CONFIG_SYS_MAX_FLASH_SECT	135

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000 /* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500 /* Flash Write Timeout (ms) */

/*
 * NAND Flash on the Local Bus
 */

#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xFFF00000
#else
#define CONFIG_SYS_NAND_BASE		0xE0600000
#endif

#define CONFIG_MTD_PARTITION

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_FSL_ELBC		1
#define CONFIG_SYS_NAND_BLOCK_SIZE	16384
#define CONFIG_SYS_NAND_WINDOW_SIZE    (32 * 1024)     /* 0x00008000 */

#define CONFIG_SYS_NAND_U_BOOT_SIZE  (512 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST   0x00100000
#define CONFIG_SYS_NAND_U_BOOT_START 0x00100100
#define CONFIG_SYS_NAND_U_BOOT_OFFS  16384
#define CONFIG_SYS_NAND_U_BOOT_RELOC 0x00010000



/* Still needed for spl_minimal.c */
#define CONFIG_SYS_NAND_BR_PRELIM CONFIG_SYS_BR1_PRELIM
#define CONFIG_SYS_NAND_OR_PRELIM CONFIG_SYS_OR1_PRELIM

#if CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE && \
	!defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0))

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

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
 * Board info - revision and where boot from
 */
#define CONFIG_SYS_I2C_PCF8574A_ADDR	0x39

/*
 * Config on-board RTC
 */
#define CONFIG_RTC_DS1337	/* ds1339 on board, use ds1337 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68 /* at address 0x68 */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI_MEM_BASE		0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS		CONFIG_SYS_PCI_MEM_BASE
#define CONFIG_SYS_PCI_MEM_SIZE		0x10000000 /* 256M */
#define CONFIG_SYS_PCI_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI_MMIO_PHYS	CONFIG_SYS_PCI_MMIO_BASE
#define CONFIG_SYS_PCI_MMIO_SIZE	0x10000000 /* 256M */
#define CONFIG_SYS_PCI_IO_BASE		0x00000000
#define CONFIG_SYS_PCI_IO_PHYS		0xE0300000
#define CONFIG_SYS_PCI_IO_SIZE		0x100000 /* 1M */

#define CONFIG_SYS_PCI_SLV_MEM_LOCAL	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_PCI_SLV_MEM_BUS	0x00000000
#define CONFIG_SYS_PCI_SLV_MEM_SIZE	0x80000000

#define CONFIG_SYS_PCIE1_BASE		0xA0000000
#define CONFIG_SYS_PCIE1_MEM_BASE	0xA0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xA0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCIE1_CFG_BASE	0xB0000000
#define CONFIG_SYS_PCIE1_CFG_SIZE	0x01000000
#define CONFIG_SYS_PCIE1_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xB1000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000

#define CONFIG_SYS_PCIE2_BASE		0xC0000000
#define CONFIG_SYS_PCIE2_MEM_BASE	0xC0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xC0000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCIE2_CFG_BASE	0xD0000000
#define CONFIG_SYS_PCIE2_CFG_SIZE	0x01000000
#define CONFIG_SYS_PCIE2_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xD1000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00800000

#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCIE

#define CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */

#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_SYS_SCCR_USBDRCM		3

#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_PHY_TYPE	"utmi"
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

/*
 * TSEC
 */
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

/*
 * TSEC ethernet configuration
 */
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC1"

/*
 * SATA
 */
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1_OFFSET	0x18000
#define CONFIG_SYS_SATA1	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA1_OFFSET)
#define CONFIG_SYS_SATA1_FLAGS	FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2_OFFSET	0x19000
#define CONFIG_SYS_SATA2	(CONFIG_SYS_IMMR + CONFIG_SYS_SATA2_OFFSET)
#define CONFIG_SYS_SATA2_FLAGS	FLAGS_DMA

#ifdef CONFIG_FSL_SATA
#define CONFIG_LBA48
#endif

/*
 * Environment
 */
#if !defined(CONFIG_SYS_RAMBOOT)
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x10000 /* 64K(one sector) for env */
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
#define CONFIG_SYS_LOAD_ADDR		0x2000000 /* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * MMU Setup
 */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */

#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_LOADADDR 800000	/* default location for tftp and bootm */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS0\0"						\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=ramfs.83xx\0"					\
	"fdtaddr=780000\0"						\
	"fdtfile=mpc8315erdb.dtb\0"					\
	"usb_phy_type=utmi\0"						\
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
