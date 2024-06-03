/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2019 Arcturus Networks, Inc.
 *           https://www.arcturusnetworks.com/products/ucp1020/
 * based on include/configs/p1_p2_rdb_pc.h
 * original copyright follows:
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

/*
 * QorIQ uCP1020-xx boards configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*** Arcturus FirmWare Environment */

#define MAX_SERIAL_SIZE 15
#define MAX_HWADDR_SIZE 17

#define MAX_FWENV_ADDR	4

#define FWENV_MMC	1
#define FWENV_SPI_FLASH	2
#define FWENV_NOR_FLASH	3
/*
 #define FWENV_TYPE    FWENV_MMC
 #define FWENV_TYPE    FWENV_SPI_FLASH
*/
#define FWENV_TYPE	FWENV_NOR_FLASH

#if (FWENV_TYPE == FWENV_MMC)
#ifndef CONFIG_SYS_MMC_ENV_DEV
#define CONFIG_SYS_MMC_ENV_DEV 1
#endif
#define FWENV_ADDR1 -1
#define FWENV_ADDR2 -1
#define FWENV_ADDR3 -1
#define FWENV_ADDR4 -1
#define EMPY_CHAR 0
#endif

#if (FWENV_TYPE == FWENV_SPI_FLASH)
#ifndef CONFIG_SF_DEFAULT_SPEED
#define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
#define CONFIG_SF_DEFAULT_MODE	SPI_MODE0
#endif
#ifndef CONFIG_SF_DEFAULT_CS
#define CONFIG_SF_DEFAULT_CS	0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
#define CONFIG_SF_DEFAULT_BUS	0
#endif
#define FWENV_ADDR1 (0x200 - sizeof(smac))
#define FWENV_ADDR2 (0x400 - sizeof(smac))
#define FWENV_ADDR3 (CONFIG_ENV_SECT_SIZE + 0x200 - sizeof(smac))
#define FWENV_ADDR4 (CONFIG_ENV_SECT_SIZE + 0x400 - sizeof(smac))
#define EMPY_CHAR 0xff
#endif

#if (FWENV_TYPE == FWENV_NOR_FLASH)
#define FWENV_ADDR1 0xEC080000
#define FWENV_ADDR2 -1
#define FWENV_ADDR3 -1
#define FWENV_ADDR4 -1
#define EMPY_CHAR 0xff
#endif
/***********************************/

#define CONFIG_PCIE1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2	/* PCIE controller 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	/* enable 64-bit PCI resources */

#if defined(CONFIG_TARTGET_UCP1020T1)

#define CONFIG_UCP1020_REV_1_3

#define CONFIG_BOARDNAME "uCP1020-64EE512-0U1-XR-T1"

#define CONFIG_TSEC1
#define CONFIG_TSEC3
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_ETHADDR		00:19:D3:FF:FF:FF
#define CONFIG_ETH1ADDR		00:19:D3:FF:FF:FE
#define CONFIG_ETH2ADDR		00:19:D3:FF:FF:FD
#define CONFIG_IPADDR		10.80.41.229
#define CONFIG_SERVERIP		10.80.41.227
#define CONFIG_NETMASK		255.255.252.0
#define CONFIG_ETHPRIME		"eTSEC3"

#define CONFIG_SYS_L2_SIZE	(256 << 10)

#endif

#if defined(CONFIG_TARGET_UCP1020)

#define CONFIG_UCP1020
#define CONFIG_UCP1020_REV_1_3

#define CONFIG_BOARDNAME_LOCAL "uCP1020-64EEE512-OU1-XR"

#define CONFIG_TSEC1
#define CONFIG_TSEC3
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_ETHADDR		00:06:3B:FF:FF:FF
#define CONFIG_ETH1ADDR		00:06:3B:FF:FF:FE
#define CONFIG_ETH2ADDR		00:06:3B:FF:FF:FD
#define CONFIG_IPADDR		192.168.1.81
#define CONFIG_IPADDR1		192.168.1.82
#define CONFIG_IPADDR2		192.168.1.83
#define CONFIG_SERVERIP		192.168.1.80
#define CONFIG_GATEWAYIP	102.168.1.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_ETHPRIME		"eTSEC1"

#undef CONFIG_SYS_REDUNDAND_ENVIRONMENT

#define CONFIG_SYS_L2_SIZE	(256 << 10)

#endif

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD
#define CONFIG_SYS_RAMBOOT
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH
#define CONFIG_SYS_RAMBOOT
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#define CONFIG_SYS_TEXT_BASE_NOR	0xeff80000

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_LBA48

#define CONFIG_SYS_CLK_FREQ	66666666
#define CONFIG_DDR_CLK_FREQ	66666666

#define CONFIG_HWCONFIG

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE
#define CONFIG_BTB

#define CONFIG_ENABLE_36BIT_PHYS

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff

#define CONFIG_SYS_CCSRBAR		0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* IN case of NAND bootloader relocate CCSRBAR in RAMboot code not in the 4k
       SPL code*/
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_CCSR_DO_NOT_RELOCATE
#endif

/* DDR Setup */
#define CONFIG_DDR_ECC_ENABLE
#ifndef CONFIG_DDR_ECC_ENABLE
#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_DDR_SPD
#endif
#define CONFIG_SYS_SPD_BUS_NUM 1

#define CONFIG_SYS_SDRAM_SIZE_LAW	LAW_SIZE_512M
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_SYS_SDRAM_SIZE		(1u << (CONFIG_SYS_SDRAM_SIZE_LAW - 19))
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1

/* Default settings for DDR3 */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000003f
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014302
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_CS1_BNDS		0x0040007f
#define CONFIG_SYS_DDR_CS1_CONFIG	0x80014302
#define CONFIG_SYS_DDR_CS1_CONFIG_2	0x00000000

#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000

#define CONFIG_SYS_DDR_ZQ_CONTROL	0x89080600
#define CONFIG_SYS_DDR_WRLVL_CONTROL	0x8655A608
#define CONFIG_SYS_DDR_SR_CNTR		0x00000000
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#ifdef CONFIG_DDR_ECC_ENABLE
#define CONFIG_SYS_DDR_CONTROL		0xE70C0000	/* Type = DDR3 & ECC */
#else
#define CONFIG_SYS_DDR_CONTROL		0xC70C0000	/* Type = DDR3 */
#endif
#define CONFIG_SYS_DDR_CONTROL_2	0x04401050
#define CONFIG_SYS_DDR_TIMING_4		0x00220001
#define CONFIG_SYS_DDR_TIMING_5		0x03402400

#define CONFIG_SYS_DDR_TIMING_3		0x00020000
#define CONFIG_SYS_DDR_TIMING_0		0x00330004
#define CONFIG_SYS_DDR_TIMING_1		0x6f6B4846
#define CONFIG_SYS_DDR_TIMING_2		0x0FA8C8CF
#define CONFIG_SYS_DDR_CLK_CTRL		0x03000000
#define CONFIG_SYS_DDR_MODE_1		0x40461520
#define CONFIG_SYS_DDR_MODE_2		0x8000c000
#define CONFIG_SYS_DDR_INTERVAL		0x0C300000

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Memory map
 *
 * 0x0000_0000 0x7fff_ffff	DDR		Up to 2GB cacheable
 * 0x8000_0000 0xdfff_ffff	PCI Express Mem	1G non-cacheable(PCIe * 2)
 * 0xec00_0000 0xefff_ffff	NOR flash	Up to 64M non-cacheable	CS0/1
 * 0xf8f8_0000 0xf8ff_ffff	L2 SRAM		Up to 256K cacheable
 *   (early boot only)
 * 0xffc0_0000 0xffc3_ffff	PCI IO range	256k non-cacheable
 * 0xffd0_0000 0xffd0_3fff	L1 for stack	16K cacheable
 * 0xffe0_0000 0xffef_ffff	CCSR		1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* 64M */
#define CONFIG_SYS_FLASH_BASE		0xec000000

#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_FLASH_BR_PRELIM (BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) \
	| BR_PS_16 | BR_V)

#define CONFIG_FLASH_OR_PRELIM		0xfc000ff7

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000 /* stack in RAM */
/* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
/* Size of used area in RAM */
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)/* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024)/* Reserved for malloc */

#define CONFIG_SYS_PMC_BASE	0xff980000
#define CONFIG_SYS_PMC_BASE_PHYS	CONFIG_SYS_PMC_BASE
#define CONFIG_PMC_BR_PRELIM	(BR_PHYS_ADDR(CONFIG_SYS_PMC_BASE_PHYS) | \
					BR_PS_8 | BR_V)
#define CONFIG_PMC_OR_PRELIM	(OR_AM_64KB | OR_GPCM_CSNT | OR_GPCM_XACS | \
				 OR_GPCM_SCY | OR_GPCM_TRLX | OR_GPCM_EHTR | \
				 OR_GPCM_EAD)

#define CONFIG_SYS_BR0_PRELIM	CONFIG_FLASH_BR_PRELIM	/* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM	CONFIG_FLASH_OR_PRELIM	/* NOR Options */
#ifdef CONFIG_NAND_FSL_ELBC
#define CONFIG_SYS_BR1_PRELIM	CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Addr */
#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */
#endif

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_INIT_MINIMAL)
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR + 0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x29} }
#define CONFIG_SYS_SPD_BUS_NUM		1 /* For rom_loc and flash bank */

#define CONFIG_RTC_DS1337
#define CONFIG_RTC_DS1337_NOOSC
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_I2C_PCA9557_ADDR	0x18
#define CONFIG_SYS_I2C_NCT72_ADDR	0x4C
#define CONFIG_SYS_I2C_IDT6V49205B	0x69

#if defined(CONFIG_PCI)
/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 2, direct to uli, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_NAME		"PCIe SLOT CON9"
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc10000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, Slot 2, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"PCIe SLOT CON10"
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

/*
 * Environment
 */
#ifdef CONFIG_ENV_FIT_UCBOOT

#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x20000)
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */

#else


#ifdef CONFIG_RAMBOOT_SPIFLASH

#define CONFIG_ENV_SIZE		0x3000		/* 12KB */
#define CONFIG_ENV_OFFSET	0x2000		/* 8KB */
#define CONFIG_ENV_SECT_SIZE	0x1000

#if defined(CONFIG_SYS_REDUNDAND_ENVIRONMENT)
/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#endif

#elif defined(CONFIG_RAMBOOT_SDCARD)
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_SYS_MMC_ENV_DEV	0

#elif defined(CONFIG_SYS_RAMBOOT)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000

#else
#define CONFIG_ENV_BASE		(CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR		(CONFIG_ENV_BASE + 0xC0000)
#if defined(CONFIG_SYS_REDUNDAND_ENVIRONMENT)
/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
#endif

#endif

#endif	/* CONFIG_ENV_FIT_UCBOOT */

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * USB
 */
#define CONFIG_HAS_FSL_DR_USB

#if defined(CONFIG_HAS_FSL_DR_USB)
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#endif
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#define CONFIG_MMC_SPI
#endif

/* Misc Extra Settings */
#undef CONFIG_WATCHDOG	/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_HZ		1000	/* decrementer freq: 1ms tick */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#if defined(CONFIG_TSEC_ENET)

#if defined(CONFIG_UCP1020_REV_1_2) || defined(CONFIG_UCP1020_REV_1_3)
#else
#error "UCP1020 module revision is not defined !!!"
#endif

#define CONFIG_BOOTP_SERVERIP

#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define TSEC1_PHY_ADDR	4
#define TSEC2_PHY_ADDR	0
#define TSEC2_PHY_ADDR_SGMII	0x00
#define TSEC3_PHY_ADDR	6

#define TSEC1_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS	(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX	0
#define TSEC2_PHYIDX	0
#define TSEC3_PHYIDX	0

#endif

#define CONFIG_HOSTNAME		"UCP1020"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin /* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#if defined(CONFIG_DONGLE)

#define	CONFIG_EXTRA_ENV_SETTINGS					\
"bootcmd=run prog_spi_mbrbootcramfs\0"					\
"bootfile=uImage\0"							\
"consoledev=ttyS0\0"							\
"cramfsfile=image.cramfs\0"						\
"dtbaddr=0x00c00000\0"							\
"dtbfile=image.dtb\0"							\
"ethaddr=" __stringify(CONFIG_ETHADDR) "\0"				\
"eth1addr=" __stringify(CONFIG_ETH1ADDR) "\0"				\
"eth2addr=" __stringify(CONFIG_ETH2ADDR) "\0"				\
"fileaddr=0x01000000\0"							\
"filesize=0x00080000\0"							\
"flashmbr=sf probe 0; "							\
	"tftp $loadaddr $mbr; "						\
	"sf erase $mbr_offset +$filesize; "				\
	"sf write $loadaddr $mbr_offset $filesize\0"			\
"flashrecovery=tftp $recoveryaddr $cramfsfile; "			\
	"protect off $nor_recoveryaddr +$filesize; "			\
	"erase $nor_recoveryaddr +$filesize; "				\
	"cp.b $recoveryaddr $nor_recoveryaddr $filesize; "		\
	"protect on $nor_recoveryaddr +$filesize\0 "			\
"flashuboot=tftp $ubootaddr $ubootfile; "				\
	"protect off $nor_ubootaddr +$filesize; "			\
	"erase $nor_ubootaddr +$filesize; "				\
	"cp.b $ubootaddr $nor_ubootaddr $filesize; "			\
	"protect on $nor_ubootaddr +$filesize\0 "			\
"flashworking=tftp $workingaddr $cramfsfile; "				\
	"protect off $nor_workingaddr +$filesize; "			\
	"erase $nor_workingaddr +$filesize; "				\
	"cp.b $workingaddr $nor_workingaddr $filesize; "		\
	"protect on $nor_workingaddr +$filesize\0 "			\
"hwconfig=usb1:dr_mode=host,phy_type=ulpi\0 "				\
"kerneladdr=0x01100000\0"						\
"kernelfile=uImage\0"							\
"loadaddr=0x01000000\0"							\
"mbr=uCP1020d.mbr\0"							\
"mbr_offset=0x00000000\0"						\
"mmbr=uCP1020Quiet.mbr\0"						\
"mmcpart=0:2\0"								\
"mmc__mbrd=fatload mmc $mmcpart $loadaddr $mbr; "			\
	"mmc erase 1 1; "						\
	"mmc write $loadaddr 1 1\0"					\
"mmc__uboot=fatload mmc $mmcpart $loadaddr $ubootfile; "		\
	"mmc erase 0x40 0x400; "					\
	"mmc write $loadaddr 0x40 0x400\0"				\
"netdev=eth0\0"								\
"nor_recoveryaddr=0xEC0A0000\0"						\
"nor_ubootaddr=0xEFF80000\0"						\
"nor_workingaddr=0xECFA0000\0"						\
"norbootrecovery=setenv bootargs $recoverybootargs"			\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"run norloadrecovery; "						\
	"bootm $kerneladdr - $dtbaddr\0"				\
"norbootworking=setenv bootargs $workingbootargs"			\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"run norloadworking; "						\
	"bootm $kerneladdr - $dtbaddr\0"				\
"norloadrecovery=mw.l $kerneladdr 0x0 0x00a00000; "			\
	"setenv cramfsaddr $nor_recoveryaddr; "				\
	"cramfsload $dtbaddr $dtbfile; "				\
	"cramfsload $kerneladdr $kernelfile\0"				\
"norloadworking=mw.l $kerneladdr 0x0 0x00a00000; "			\
	"setenv cramfsaddr $nor_workingaddr; "				\
	"cramfsload $dtbaddr $dtbfile; "				\
	"cramfsload $kerneladdr $kernelfile\0"				\
"prog_spi_mbr=run spi__mbr\0"						\
"prog_spi_mbrboot=run spi__mbr; run spi__boot1; run spi__boot2\0"	\
"prog_spi_mbrbootcramfs=run spi__mbr; run spi__boot1; run spi__boot2; "	\
	"run spi__cramfs\0"						\
"ramboot=setenv bootargs root=/dev/ram ramdisk_size=$ramdisk_size ro"	\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"tftp $rootfsaddr $rootfsfile; "				\
	"tftp $loadaddr $kernelfile; "					\
	"tftp $dtbaddr $dtbfile; "					\
	"bootm $loadaddr $rootfsaddr $dtbaddr\0"			\
"ramdisk_size=120000\0"							\
"ramdiskfile=rootfs.ext2.gz.uboot\0"					\
"recoveryaddr=0x02F00000\0"						\
"recoverybootargs=root=/dev/mtdblock0 rootfstype=cramfs ro\0"		\
"releasefpga=mw.l 0xffe0f000 0x00400000; mw.l 0xffe0f004 0x00000000; "	\
	"mw.l 0xffe0f008 0x00400000\0"					\
"rootfsaddr=0x02F00000\0"						\
"rootfsfile=rootfs.ext2.gz.uboot\0"					\
"rootpath=/opt/nfsroot\0"						\
"spi__boot1=fatload mmc $mmcpart $loadaddr u-boot.bin; "		\
	"protect off 0xeC000000 +$filesize; "				\
	"erase 0xEC000000 +$filesize; "					\
	"cp.b $loadaddr 0xEC000000 $filesize; "				\
	"cmp.b $loadaddr 0xEC000000 $filesize; "			\
	"protect on 0xeC000000 +$filesize\0"				\
"spi__boot2=fatload mmc $mmcpart $loadaddr u-boot.bin; "		\
	"protect off 0xeFF80000 +$filesize; "				\
	"erase 0xEFF80000 +$filesize; "					\
	"cp.b $loadaddr 0xEFF80000 $filesize; "				\
	"cmp.b $loadaddr 0xEFF80000 $filesize; "			\
	"protect on 0xeFF80000 +$filesize\0"				\
"spi__bootd=fatload mmc $mmcpart $loadaddr $ubootd; "			\
	"sf probe 0; sf erase 0x8000 +$filesize; "			\
	"sf write $loadaddr 0x8000 $filesize\0"				\
"spi__cramfs=fatload mmc $mmcpart $loadaddr image.cramfs; "		\
	"protect off 0xec0a0000 +$filesize; "				\
	"erase 0xeC0A0000 +$filesize; "					\
	"cp.b $loadaddr 0xeC0A0000 $filesize; "				\
	"protect on 0xec0a0000 +$filesize\0"				\
"spi__mbr=fatload mmc $mmcpart $loadaddr $mmbr; "			\
	"sf probe 1; sf erase 0 +$filesize; "				\
	"sf write $loadaddr 0 $filesize\0"				\
"spi__mbrd=fatload mmc $mmcpart $loadaddr $mbr; "			\
	"sf probe 0; sf erase 0 +$filesize; "				\
	"sf write $loadaddr 0 $filesize\0"				\
"tftpflash=tftpboot $loadaddr $uboot; "					\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"erase " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize; " \
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize\0"\
"uboot= " __stringify(CONFIG_UBOOTPATH) "\0"				\
"ubootaddr=0x01000000\0"						\
"ubootfile=u-boot.bin\0"						\
"ubootd=u-boot4dongle.bin\0"						\
"upgrade=run flashworking\0"						\
"usb_phy_type=ulpi\0 "							\
"workingaddr=0x02F00000\0"						\
"workingbootargs=root=/dev/mtdblock1 rootfstype=cramfs ro\0"

#else

#if defined(CONFIG_UCP1020T1)

#define	CONFIG_EXTRA_ENV_SETTINGS					\
"bootcmd=run releasefpga; run norbootworking || run norbootrecovery\0"	\
"bootfile=uImage\0"							\
"consoledev=ttyS0\0"							\
"cramfsfile=image.cramfs\0"						\
"dtbaddr=0x00c00000\0"							\
"dtbfile=image.dtb\0"							\
"ethaddr=" __stringify(CONFIG_ETHADDR) "\0"				\
"eth1addr=" __stringify(CONFIG_ETH1ADDR) "\0"				\
"eth2addr=" __stringify(CONFIG_ETH2ADDR) "\0"				\
"fileaddr=0x01000000\0"							\
"filesize=0x00080000\0"							\
"flashmbr=sf probe 0; "							\
	"tftp $loadaddr $mbr; "						\
	"sf erase $mbr_offset +$filesize; "				\
	"sf write $loadaddr $mbr_offset $filesize\0"			\
"flashrecovery=tftp $recoveryaddr $cramfsfile; "			\
	"protect off $nor_recoveryaddr +$filesize; "			\
	"erase $nor_recoveryaddr +$filesize; "				\
	"cp.b $recoveryaddr $nor_recoveryaddr $filesize; "		\
	"protect on $nor_recoveryaddr +$filesize\0 "			\
"flashuboot=tftp $ubootaddr $ubootfile; "				\
	"protect off $nor_ubootaddr +$filesize; "			\
	"erase $nor_ubootaddr +$filesize; "				\
	"cp.b $ubootaddr $nor_ubootaddr $filesize; "			\
	"protect on $nor_ubootaddr +$filesize\0 "			\
"flashworking=tftp $workingaddr $cramfsfile; "				\
	"protect off $nor_workingaddr +$filesize; "			\
	"erase $nor_workingaddr +$filesize; "				\
	"cp.b $workingaddr $nor_workingaddr $filesize; "		\
	"protect on $nor_workingaddr +$filesize\0 "			\
"hwconfig=usb1:dr_mode=host,phy_type=ulpi\0 "				\
"kerneladdr=0x01100000\0"						\
"kernelfile=uImage\0"							\
"loadaddr=0x01000000\0"							\
"mbr=uCP1020.mbr\0"							\
"mbr_offset=0x00000000\0"						\
"netdev=eth0\0"								\
"nor_recoveryaddr=0xEC0A0000\0"						\
"nor_ubootaddr=0xEFF80000\0"						\
"nor_workingaddr=0xECFA0000\0"						\
"norbootrecovery=setenv bootargs $recoverybootargs"			\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"run norloadrecovery; "						\
	"bootm $kerneladdr - $dtbaddr\0"				\
"norbootworking=setenv bootargs $workingbootargs"			\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"run norloadworking; "						\
	"bootm $kerneladdr - $dtbaddr\0"				\
"norloadrecovery=mw.l $kerneladdr 0x0 0x00a00000; "			\
	"setenv cramfsaddr $nor_recoveryaddr; "				\
	"cramfsload $dtbaddr $dtbfile; "				\
	"cramfsload $kerneladdr $kernelfile\0"				\
"norloadworking=mw.l $kerneladdr 0x0 0x00a00000; "			\
	"setenv cramfsaddr $nor_workingaddr; "				\
	"cramfsload $dtbaddr $dtbfile; "				\
	"cramfsload $kerneladdr $kernelfile\0"				\
"othbootargs=quiet\0"							\
"ramboot=setenv bootargs root=/dev/ram ramdisk_size=$ramdisk_size ro"	\
	" console=$consoledev,$baudrate $othbootargs; "			\
	"tftp $rootfsaddr $rootfsfile; "				\
	"tftp $loadaddr $kernelfile; "					\
	"tftp $dtbaddr $dtbfile; "					\
	"bootm $loadaddr $rootfsaddr $dtbaddr\0"			\
"ramdisk_size=120000\0"							\
"ramdiskfile=rootfs.ext2.gz.uboot\0"					\
"recoveryaddr=0x02F00000\0"						\
"recoverybootargs=root=/dev/mtdblock0 rootfstype=cramfs ro\0"		\
"releasefpga=mw.l 0xffe0f000 0x00400000; mw.l 0xffe0f004 0x00000000; "	\
	"mw.l 0xffe0f008 0x00400000\0"					\
"rootfsaddr=0x02F00000\0"						\
"rootfsfile=rootfs.ext2.gz.uboot\0"					\
"rootpath=/opt/nfsroot\0"						\
"silent=1\0"								\
"tftpflash=tftpboot $loadaddr $uboot; "					\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"erase " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize; " \
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize\0"\
"uboot= " __stringify(CONFIG_UBOOTPATH) "\0"				\
"ubootaddr=0x01000000\0"						\
"ubootfile=u-boot.bin\0"						\
"upgrade=run flashworking\0"						\
"workingaddr=0x02F00000\0"						\
"workingbootargs=root=/dev/mtdblock1 rootfstype=cramfs ro\0"

#else /* For Arcturus Modules */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
"bootcmd=run norkernel\0"						\
"bootfile=uImage\0"							\
"consoledev=ttyS0\0"							\
"dtbaddr=0x00c00000\0"							\
"dtbfile=image.dtb\0"							\
"ethaddr=" __stringify(CONFIG_ETHADDR) "\0"				\
"eth1addr=" __stringify(CONFIG_ETH1ADDR) "\0"				\
"eth2addr=" __stringify(CONFIG_ETH2ADDR) "\0"				\
"fileaddr=0x01000000\0"							\
"filesize=0x00080000\0"							\
"flashmbr=sf probe 0; "							\
	"tftp $loadaddr $mbr; "						\
	"sf erase $mbr_offset +$filesize; "				\
	"sf write $loadaddr $mbr_offset $filesize\0"			\
"flashuboot=tftp $loadaddr $ubootfile; "				\
	"protect off $nor_ubootaddr0 +$filesize; "			\
	"erase $nor_ubootaddr0 +$filesize; "				\
	"cp.b $loadaddr $nor_ubootaddr0 $filesize; "			\
	"protect on $nor_ubootaddr0 +$filesize; "			\
	"protect off $nor_ubootaddr1 +$filesize; "			\
	"erase $nor_ubootaddr1 +$filesize; "				\
	"cp.b $loadaddr $nor_ubootaddr1 $filesize; "			\
	"protect on $nor_ubootaddr1 +$filesize\0 "			\
"format0=protect off $part0base +$part0size; "				\
	"erase $part0base +$part0size\0"				\
"format1=protect off $part1base +$part1size; "				\
	"erase $part1base +$part1size\0"				\
"format2=protect off $part2base +$part2size; "				\
	"erase $part2base +$part2size\0"				\
"format3=protect off $part3base +$part3size; "				\
	"erase $part3base +$part3size\0"				\
"hwconfig=usb1:dr_mode=host,phy_type=ulpi\0 "				\
"kerneladdr=0x01100000\0"						\
"kernelargs=root=/dev/mtdblock1 rootfstype=cramfs ro\0"			\
"kernelfile=uImage\0"							\
"loadaddr=0x01000000\0"							\
"mbr=uCP1020.mbr\0"							\
"mbr_offset=0x00000000\0"						\
"netdev=eth0\0"								\
"nor_ubootaddr0=0xEC000000\0"						\
"nor_ubootaddr1=0xEFF80000\0"						\
"norkernel=setenv bootargs $kernelargs console=$consoledev,$baudrate; "	\
	"run norkernelload; "						\
	"bootm $kerneladdr - $dtbaddr\0"				\
"norkernelload=mw.l $kerneladdr 0x0 0x00a00000; "			\
	"setenv cramfsaddr $part0base; "				\
	"cramfsload $dtbaddr $dtbfile; "				\
	"cramfsload $kerneladdr $kernelfile\0"				\
"part0base=0xEC100000\0"						\
"part0size=0x00700000\0"						\
"part1base=0xEC800000\0"						\
"part1size=0x02000000\0"						\
"part2base=0xEE800000\0"						\
"part2size=0x00800000\0"						\
"part3base=0xEF000000\0"						\
"part3size=0x00F80000\0"						\
"partENVbase=0xEC080000\0"						\
"partENVsize=0x00080000\0"						\
"program0=tftp part0-000000.bin; "					\
	"protect off $part0base +$filesize; "				\
	"erase $part0base +$filesize; "					\
	"cp.b $loadaddr $part0base $filesize; "				\
	"echo Verifying...; "						\
	"cmp.b $loadaddr $part0base $filesize\0"			\
"program1=tftp part1-000000.bin; "					\
	"protect off $part1base +$filesize; "				\
	"erase $part1base +$filesize; "					\
	"cp.b $loadaddr $part1base $filesize; "				\
	"echo Verifying...; "						\
	"cmp.b $loadaddr $part1base $filesize\0"			\
"program2=tftp part2-000000.bin; "					\
	"protect off $part2base +$filesize; "				\
	"erase $part2base +$filesize; "					\
	"cp.b $loadaddr $part2base $filesize; "				\
	"echo Verifying...; "						\
	"cmp.b $loadaddr $part2base $filesize\0"			\
"ramboot=setenv bootargs root=/dev/ram ramdisk_size=$ramdisk_size ro"	\
	"  console=$consoledev,$baudrate $othbootargs; "		\
	"tftp $rootfsaddr $rootfsfile; "				\
	"tftp $loadaddr $kernelfile; "					\
	"tftp $dtbaddr $dtbfile; "					\
	"bootm $loadaddr $rootfsaddr $dtbaddr\0"			\
"ramdisk_size=120000\0"							\
"ramdiskfile=rootfs.ext2.gz.uboot\0"					\
"releasefpga=mw.l 0xffe0f000 0x00400000; mw.l 0xffe0f004 0x00000000; "	\
	"mw.l 0xffe0f008 0x00400000\0"					\
"rootfsaddr=0x02F00000\0"						\
"rootfsfile=rootfs.ext2.gz.uboot\0"					\
"rootpath=/opt/nfsroot\0"						\
"spi__mbr=fatload mmc $mmcpart $loadaddr $mmbr; "			\
	"sf probe 0; sf erase 0 +$filesize; "				\
	"sf write $loadaddr 0 $filesize\0"				\
"spi__boot=fatload mmc $mmcpart $loadaddr u-boot.bin; "			\
	"protect off 0xeC000000 +$filesize; "				\
	"erase 0xEC000000 +$filesize; "					\
	"cp.b $loadaddr 0xEC000000 $filesize; "				\
	"cmp.b $loadaddr 0xEC000000 $filesize; "			\
	"protect on 0xeC000000 +$filesize\0"				\
"tftpflash=tftpboot $loadaddr $uboot; "					\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"erase " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize; " \
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " +$filesize; " \
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE_NOR) " $filesize\0"\
"uboot= " __stringify(CONFIG_UBOOTPATH) "\0"				\
"ubootfile=u-boot.bin\0"						\
"upgrade=run flashuboot\0"						\
"usb_phy_type=ulpi\0 "							\
"boot_nfs= "								\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"			\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr\0"					\
"boot_hd = "								\
	"setenv bootargs root=/dev/$bdev rw rootdelay=30 "		\
	"console=$consoledev,$baudrate $othbootargs;"			\
	"usb start;"							\
	"ext2load usb 0:1 $loadaddr /boot/$bootfile;"			\
	"ext2load usb 0:1 $fdtaddr /boot/$fdtfile;"			\
	"bootm $loadaddr - $fdtaddr\0"					\
"boot_usb_fat = "							\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs "			\
	"ramdisk_size=$ramdisk_size;"					\
	"usb start;"							\
	"fatload usb 0:2 $loadaddr $bootfile;"				\
	"fatload usb 0:2 $fdtaddr $fdtfile;"				\
	"fatload usb 0:2 $ramdiskaddr $ramdiskfile;"			\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0 "			\
"boot_usb_ext2 = "							\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs "			\
	"ramdisk_size=$ramdisk_size;"					\
	"usb start;"							\
	"ext2load usb 0:4 $loadaddr $bootfile;"				\
	"ext2load usb 0:4 $fdtaddr $fdtfile;"				\
	"ext2load usb 0:4 $ramdiskaddr $ramdiskfile;"			\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0 "			\
"boot_nor = "								\
	"setenv bootargs root=/dev/$jffs2nor rw "			\
	"console=$consoledev,$baudrate rootfstype=jffs2 $othbootargs;"	\
	"bootm $norbootaddr - $norfdtaddr\0 "				\
"boot_ram = "								\
	"setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs "			\
	"ramdisk_size=$ramdisk_size;"					\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"

#endif
#endif

#endif /* __CONFIG_H */
