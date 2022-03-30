/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

/*
 * QorIQ P1 Tower boards configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#if defined(CONFIG_TWR_P1025)
#define CONFIG_BOARDNAME "TWR-P1025"
#define CONFIG_PHY_ATHEROS
#define CONFIG_SYS_LBC_LBCR	0x00080000	/* Conversion of LBC addr */
#define CONFIG_SYS_LBC_LCRR	0x80000002	/* LB clock ratio reg */
#endif

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD
#define CONFIG_SYS_RAMBOOT
#define CONFIG_RESET_VECTOR_ADDRESS	0x110bfffc
#endif

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_PCIE1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2	/* PCIE controller 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	/* enable 64-bit PCI resources */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_LBA48

#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /*sysclk for TWR-P1025 */

#define CONFIG_DDR_CLK_FREQ	66666666

#define CONFIG_HWCONFIG
/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE
#define CONFIG_BTB

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff

#define CONFIG_SYS_CCSRBAR		0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */

#define CONFIG_SYS_SDRAM_SIZE_LAW	LAW_SIZE_512M
#define CONFIG_CHIP_SELECTS_PER_CTRL	1

#define CONFIG_SYS_SDRAM_SIZE		(1u << (CONFIG_SYS_SDRAM_SIZE_LAW - 19))
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1

/* Default settings for DDR3 */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000001f
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014202
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_CS1_BNDS		0x00000000
#define CONFIG_SYS_DDR_CS1_CONFIG	0x00000000
#define CONFIG_SYS_DDR_CS1_CONFIG_2	0x00000000

#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000

#define CONFIG_SYS_DDR_ZQ_CONTROL	0x89080600
#define CONFIG_SYS_DDR_WRLVL_CONTROL	0x8655a608
#define CONFIG_SYS_DDR_SR_CNTR		0x00000000
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#define CONFIG_SYS_DDR_CONTROL		0xc70c0000	/* Type = DDR3	*/
#define CONFIG_SYS_DDR_CONTROL_2	0x04401050
#define CONFIG_SYS_DDR_TIMING_4		0x00220001
#define CONFIG_SYS_DDR_TIMING_5		0x03402400

#define CONFIG_SYS_DDR_TIMING_3		0x00020000
#define CONFIG_SYS_DDR_TIMING_0		0x00220004
#define CONFIG_SYS_DDR_TIMING_1		0x5c5b6544
#define CONFIG_SYS_DDR_TIMING_2		0x0fa880de
#define CONFIG_SYS_DDR_CLK_CTRL		0x03000000
#define CONFIG_SYS_DDR_MODE_1		0x80461320
#define CONFIG_SYS_DDR_MODE_2		0x00008000
#define CONFIG_SYS_DDR_INTERVAL		0x09480000

/*
 * Memory map
 *
 * 0x0000_0000 0x1fff_ffff	DDR		Up to 512MB cacheable
 * 0x8000_0000 0xdfff_ffff	PCI Express Mem	1.5G non-cacheable(PCIe * 3)
 * 0xffc0_0000 0xffc3_ffff	PCI IO range	256k non-cacheable
 *
 * Localbus
 * 0xe000_0000 0xe002_0000	SSD1289		128K non-cacheable
 * 0xec00_0000 0xefff_ffff	FLASH		Up to 64M non-cacheable
 *
 * 0xff90_0000 0xff97_ffff	L2 SRAM		Up to 512K cacheable
 * 0xffd0_0000 0xffd0_3fff	init ram	16K Cacheable
 * 0xffe0_0000 0xffef_ffff	CCSR		1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* 64M */
#define CONFIG_SYS_FLASH_BASE		0xec000000

#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_FLASH_BR_PRELIM (BR_PHYS_ADDR((CONFIG_SYS_FLASH_BASE_PHYS)) \
	| BR_PS_16 | BR_V)

#define CONFIG_FLASH_OR_PRELIM	0xfc0000b1

#define CONFIG_SYS_SSD_BASE	0xe0000000
#define CONFIG_SYS_SSD_BASE_PHYS	CONFIG_SYS_SSD_BASE
#define CONFIG_SSD_BR_PRELIM	(BR_PHYS_ADDR(CONFIG_SYS_SSD_BASE_PHYS) | \
					BR_PS_16 | BR_V)
#define CONFIG_SSD_OR_PRELIM	(OR_AM_64KB | OR_GPCM_CSNT | OR_GPCM_XACS | \
				 OR_GPCM_ACS_DIV2 | OR_GPCM_SCY | \
				 OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CONFIG_SYS_BR2_PRELIM CONFIG_SSD_BR_PRELIM
#define CONFIG_SYS_OR2_PRELIM CONFIG_SSD_OR_PRELIM

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000
/* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
/* Size of used area in RAM */
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024)/* Reserved for malloc */

#define CONFIG_SYS_BR0_PRELIM	CONFIG_FLASH_BR_PRELIM	/* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM	CONFIG_FLASH_OR_PRELIM	/* NOR Options */

/* Serial Port
 * open - index 2
 * shorted - index 1
 */
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL			/* Use FSL common I2C driver */
#define CONFIG_SYS_FSL_I2C_SPEED	400000	/* I2C spd and slave address */
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x52

/*
 * I2C2 EEPROM
 */
#define CONFIG_SYS_FSL_I2C2_SPEED	400000	/* I2C spd and slave address */
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

#define CONFIG_SYS_I2C_PCA9555_ADDR	0x23

/* enable read and write access to EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

#if defined(CONFIG_PCI)
/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 2, direct to uli, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_NAME		"TWR-ELEV PCIe SLOT"
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc10000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"mini PCIe SLOT"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc00000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

#define CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#endif /* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#undef CONFIG_TSEC2
#undef CONFIG_TSEC2_NAME
#define CONFIG_TSEC3
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define TSEC1_PHY_ADDR	2
#define TSEC2_PHY_ADDR	0
#define TSEC3_PHY_ADDR	1

#define TSEC1_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX	0
#define TSEC2_PHYIDX	0
#define TSEC3_PHYIDX	0

#define CONFIG_ETHPRIME	"eTSEC1"

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#undef CONFIG_HAS_ETH2
#endif /* CONFIG_TSEC_ENET */

#ifdef CONFIG_QE
/* QE microcode/firmware address */
#define CONFIG_SYS_QE_FW_ADDR	0xefec0000
#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#endif /* CONFIG_QE */

#ifdef CONFIG_TWR_P1025
/*
 * QE UEC ethernet configuration
 */
#define CONFIG_MIIM_ADDRESS	(CONFIG_SYS_CCSRBAR + 0x82120)

#undef CONFIG_UEC_ETH
#define CONFIG_PHY_MODE_NEED_CHANGE

#define CONFIG_UEC_ETH1	/* ETH1 */
#define CONFIG_HAS_ETH0

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	0	/* UCC1 */
#define CONFIG_SYS_UEC1_RX_CLK	QE_CLK12 /* CLK12 for MII */
#define CONFIG_SYS_UEC1_TX_CLK	QE_CLK9 /* CLK9 for MII */
#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	0x18	/* 0x18 for MII */
#define CONFIG_SYS_UEC1_INTERFACE_TYPE PHY_INTERFACE_MODE_MII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100
#endif /* CONFIG_UEC_ETH1 */

#define CONFIG_UEC_ETH5	/* ETH5 */
#define CONFIG_HAS_ETH1

#ifdef CONFIG_UEC_ETH5
#define CONFIG_SYS_UEC5_UCC_NUM	4	/* UCC5 */
#define CONFIG_SYS_UEC5_RX_CLK	QE_CLK_NONE
#define CONFIG_SYS_UEC5_TX_CLK	QE_CLK13 /* CLK 13 for RMII */
#define CONFIG_SYS_UEC5_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC5_PHY_ADDR	0x19	/* 0x19 for RMII */
#define CONFIG_SYS_UEC5_INTERFACE_TYPE PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC5_INTERFACE_SPEED	100
#endif /* CONFIG_UEC_ETH5 */
#endif /* CONFIG_TWR-P1025 */

/*
 * Dynamic MTD Partition support with mtdparts
 */

/*
 * Environment
 */
#ifdef CONFIG_SYS_RAMBOOT
#ifdef CONFIG_RAMBOOT_SDCARD
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_SYS_MMC_ENV_DEV	0
#else
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif
#else
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * USB
 */
#define CONFIG_HAS_FSL_DR_USB

#if defined(CONFIG_HAS_FSL_DR_USB)
#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#endif
#endif

#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#endif

#undef CONFIG_WATCHDOG	/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */
#define CONFIG_HOSTNAME		"unknown"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin /* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR	1000000

#define	CONFIG_EXTRA_ENV_SETTINGS	\
"netdev=eth0\0"	\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"	\
"loadaddr=1000000\0"	\
"bootfile=uImage\0"	\
"dtbfile=twr-p1025twr.dtb\0"	\
"ramdiskfile=rootfs.ext2.gz.uboot\0"	\
"qefirmwarefile=fsl_qe_ucode_1021_10_A.bin\0"	\
"tftpflash=tftpboot $loadaddr $uboot; "	\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; " \
	"erase " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE) " $filesize; " \
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE) " +$filesize; "	\
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE) " $filesize\0" \
"kernelflash=tftpboot $loadaddr $bootfile; "	\
	"protect off 0xefa80000 +$filesize; "	\
	"erase 0xefa80000 +$filesize; "	\
	"cp.b $loadaddr 0xefa80000 $filesize; "	\
	"protect on 0xefa80000 +$filesize; "	\
	"cmp.b $loadaddr 0xefa80000 $filesize\0"	\
"dtbflash=tftpboot $loadaddr $dtbfile; "	\
	"protect off 0xefe80000 +$filesize; "	\
	"erase 0xefe80000 +$filesize; "	\
	"cp.b $loadaddr 0xefe80000 $filesize; "	\
	"protect on 0xefe80000 +$filesize; "	\
	"cmp.b $loadaddr 0xefe80000 $filesize\0"	\
"ramdiskflash=tftpboot $loadaddr $ramdiskfile; "	\
	"protect off 0xeeb80000 +$filesize; "	\
	"erase 0xeeb80000 +$filesize; "	\
	"cp.b $loadaddr 0xeeb80000 $filesize; "	\
	"protect on 0xeeb80000 +$filesize; "	\
	"cmp.b $loadaddr 0xeeb80000 $filesize\0"	\
"qefirmwareflash=tftpboot $loadaddr $qefirmwarefile; "	\
	"protect off 0xefec0000 +$filesize; "	\
	"erase 0xefec0000 +$filesize; "	\
	"cp.b $loadaddr 0xefec0000 $filesize; "	\
	"protect on 0xefec0000 +$filesize; "	\
	"cmp.b $loadaddr 0xefec0000 $filesize\0"	\
"consoledev=ttyS0\0"	\
"ramdiskaddr=2000000\0"	\
"ramdiskfile=rootfs.ext2.gz.uboot\0"	\
"fdtaddr=1e00000\0"	\
"bdev=sda1\0"	\
"norbootaddr=ef080000\0"	\
"norfdtaddr=ef040000\0"	\
"ramdisk_size=120000\0" \
"usbboot=setenv bootargs root=/dev/sda1 rw rootdelay=5 " \
"console=$consoledev,$baudrate $othbootargs ; bootm 0xefa80000 - 0xefe80000"

#define CONFIG_NFSBOOTCOMMAND	\
"setenv bootargs root=/dev/nfs rw "	\
"nfsroot=$serverip:$rootpath "	\
"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
"console=$consoledev,$baudrate $othbootargs;" \
"tftp $loadaddr $bootfile&&"	\
"tftp $fdtaddr $fdtfile&&"	\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_HDBOOT	\
"setenv bootargs root=/dev/$bdev rw rootdelay=30 "	\
"console=$consoledev,$baudrate $othbootargs;" \
"usb start;"	\
"ext2load usb 0:1 $loadaddr /boot/$bootfile;"	\
"ext2load usb 0:1 $fdtaddr /boot/$fdtfile;"	\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_USB_FAT_BOOT	\
"setenv bootargs root=/dev/ram rw "	\
"console=$consoledev,$baudrate $othbootargs " \
"ramdisk_size=$ramdisk_size;"	\
"usb start;"	\
"fatload usb 0:2 $loadaddr $bootfile;"	\
"fatload usb 0:2 $fdtaddr $fdtfile;"	\
"fatload usb 0:2 $ramdiskaddr $ramdiskfile;"	\
"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_USB_EXT2_BOOT	\
"setenv bootargs root=/dev/ram rw "	\
"console=$consoledev,$baudrate $othbootargs " \
"ramdisk_size=$ramdisk_size;"	\
"usb start;"	\
"ext2load usb 0:4 $loadaddr $bootfile;"	\
"ext2load usb 0:4 $fdtaddr $fdtfile;" \
"ext2load usb 0:4 $ramdiskaddr $ramdiskfile;" \
"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_NORBOOT	\
"setenv bootargs root=/dev/mtdblock3 rw "	\
"console=$consoledev,$baudrate rootfstype=jffs2 $othbootargs;"	\
"bootm $norbootaddr - $norfdtaddr"

#define CONFIG_RAMBOOTCOMMAND_TFTP	\
"setenv bootargs root=/dev/ram rw "	\
"console=$consoledev,$baudrate $othbootargs " \
"ramdisk_size=$ramdisk_size;"	\
"tftp $ramdiskaddr $ramdiskfile;"	\
"tftp $loadaddr $bootfile;"	\
"tftp $fdtaddr $fdtfile;"	\
"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND	\
"setenv bootargs root=/dev/ram rw "	\
"console=$consoledev,$baudrate $othbootargs " \
"ramdisk_size=$ramdisk_size;"	\
"bootm 0xefa80000 0xeeb80000 0xefe80000"

#define CONFIG_BOOTCOMMAND	CONFIG_RAMBOOTCOMMAND

#endif /* __CONFIG_H */
