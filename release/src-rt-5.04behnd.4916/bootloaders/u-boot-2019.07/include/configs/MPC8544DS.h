/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc.
 */

/*
 * mpc8544ds board configuration file
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_PCI1		1	/* PCI controller 1 */
#define CONFIG_PCIE1		1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controller 2 (slot 2) */
#define CONFIG_PCIE3		1	/* PCIE controller 3 (ULI bridge) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

#define CONFIG_ENV_OVERWRITE
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */

/* Make sure required options are set */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Memory map
 *
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 *
 * 0x8000_0000	0xbfff_ffff	PCI Express Mem		1G non-cacheable
 *
 * 0xc000_0000	0xdfff_ffff	PCI			512M non-cacheable
 *
 * 0xe000_0000	0xe00f_ffff	CCSR			1M non-cacheable
 * 0xe100_0000	0xe3ff_ffff	PCI IO range		4M non-cacheable
 *
 * Localbus cacheable
 *
 * 0xf000_0000	0xf3ff_ffff	SDRAM			64M Cacheable
 * 0xf401_0000	0xf401_3fff	L1 for stack		4K Cacheable TLB0
 *
 * Localbus non-cacheable
 *
 * 0xf800_0000	0xf80f_ffff	NVRAM/CADMUS (*)	1M non-cacheable
 * 0xff00_0000	0xff7f_ffff	FLASH (2nd bank)	8M non-cacheable
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M non-cacheable
 *
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_BOOT_BLOCK		0xfc000000	/* boot TLB */

#define CONFIG_SYS_FLASH_BASE		0xff800000	/* start of FLASH 8M */

#define CONFIG_SYS_BR0_PRELIM		0xff801001
#define CONFIG_SYS_BR1_PRELIM		0xfe801001

#define CONFIG_SYS_OR0_PRELIM		0xff806e65
#define CONFIG_SYS_OR1_PRELIM		0xff806e65

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}

#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */
#define CONFIG_FLASH_SHOW_PROGRESS 45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_LBC_NONCACHE_BASE	0xf8000000

#define CONFIG_SYS_BR2_PRELIM		0xf8201001	/* port size 16bit */
#define CONFIG_SYS_OR2_PRELIM		0xfff06ff7	/* 1MB Compact Flash area*/

#define CONFIG_SYS_BR3_PRELIM		0xf8100801	/* port size 8bit */
#define CONFIG_SYS_OR3_PRELIM		0xfff06ff7	/* 1MB PIXIS area*/

#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xf8100000	/* PIXIS registers */
#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_AUX		0x6	/* PIXIS Auxiliary register; Scratch
					 * register */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VBOOT_FMAP	0x80	/* VBOOT - CFG_FLASHMAP */
#define PIXIS_VBOOT_FBANK	0x40	/* VBOOT - CFG_FLASHBANK */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */
#define PIXIS_VSPEED2		0x1d	/* VELA VSpeed 2 */
#define CONFIG_SYS_PIXIS_VBOOT_MASK	0x40    /* Reset altbank mask*/
#define PIXIS_VSPEED2_TSEC1SER	0x2
#define PIXIS_VSPEED2_TSEC3SER	0x1
#define PIXIS_VCFGEN1_TSEC1SER	0x20
#define PIXIS_VCFGEN1_TSEC3SER	0x40
#define PIXIS_VSPEED2_MASK	(PIXIS_VSPEED2_TSEC1SER|PIXIS_VSPEED2_TSEC3SER)
#define PIXIS_VCFGEN1_MASK	(PIXIS_VCFGEN1_TSEC1SER|PIXIS_VCFGEN1_TSEC3SER)

#define CONFIG_SYS_INIT_RAM_LOCK      1
#define CONFIG_SYS_INIT_RAM_ADDR      0xf4010000      /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_SIZE       0x00004000      /* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCIE_VIRT		0x80000000	/* 1G PCIE TLB */
#define CONFIG_SYS_PCIE_PHYS		0x80000000	/* 1G PCIE TLB */
#define CONFIG_SYS_PCI_VIRT		0xc0000000	/* 512M PCI TLB */
#define CONFIG_SYS_PCI_PHYS		0xc0000000	/* 512M PCI TLB */

#define CONFIG_SYS_PCI1_MEM_VIRT	0xc0000000
#define CONFIG_SYS_PCI1_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0xc0000000
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT	0xe1000000
#define CONFIG_SYS_PCI1_IO_BUS	0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS	0xe1000000
#define CONFIG_SYS_PCI1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 1, tgtid 1, Base address 9000 */
#define CONFIG_SYS_PCIE2_NAME		"Slot 1"
#define CONFIG_SYS_PCIE2_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCIE2_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xe1010000
#define CONFIG_SYS_PCIE2_IO_BUS	0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xe1010000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 1, Slot 2,tgtid 2, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"Slot 2"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe1020000
#define CONFIG_SYS_PCIE1_IO_BUS	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe1020000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 3, direct to uli, tgtid 3, Base address b000 */
#define CONFIG_SYS_PCIE3_NAME		"ULI"
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xb0000000
#define CONFIG_SYS_PCIE3_MEM_BUS	0xb0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xb0000000
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x00100000	/* 1M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xb0100000	/* reuse mem LAW */
#define CONFIG_SYS_PCIE3_IO_BUS	0x00000000
#define CONFIG_SYS_PCIE3_IO_PHYS	0xb0100000	/* reuse mem LAW */
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00100000	/* 1M */
#define CONFIG_SYS_PCIE3_MEM_VIRT2	0xb0200000
#define CONFIG_SYS_PCIE3_MEM_BUS2	0xb0200000
#define CONFIG_SYS_PCIE3_MEM_PHYS2	0xb0200000
#define CONFIG_SYS_PCIE3_MEM_SIZE2	0x00200000	/* 1M */

#if defined(CONFIG_PCI)

/*PCIE video card used*/
#define VIDEO_IO_OFFSET		CONFIG_SYS_PCIE2_IO_VIRT

/*PCI video card used*/
/*#define VIDEO_IO_OFFSET	CONFIG_SYS_PCI1_IO_VIRT*/

/* video */

#if defined(CONFIG_VIDEO)
#define CONFIG_BIOSEMU
#define CONFIG_ATI_RADEON_FB
#define CONFIG_VIDEO_LOGO
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS VIDEO_IO_OFFSET
#endif

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#ifndef CONFIG_PCI_PNP
	#define PCI_ENET0_IOADDR	CONFIG_SYS_PCI1_IO_BUS
	#define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI1_IO_BUS
	#define PCI_IDSEL_NUMBER	0x11	/* IDSEL = AD11 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	4
#define CONFIG_SYS_SCSI_MAX_LUN	1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#endif /* CONFIG_SCSI_AHCI */

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define CONFIG_PIXIS_SGMII_CMD
#define CONFIG_FSL_SGMII_RISER	1
#define SGMII_RISER_PHY_OFFSET	0x1c

#define TSEC1_PHY_ADDR		0
#define TSEC3_PHY_ADDR		1

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC3_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#define CONFIG_ENV_SECT_SIZE	0x10000 /* 64K (one sector) */
#if CONFIG_SYS_MONITOR_BASE > 0xfff80000
#define CONFIG_ENV_ADDR		0xfff80000
#else
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#endif
#define CONFIG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * USB
 */

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_PCI_EHCI_DEVICE			0
#endif

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */

/* The mac addresses for all ethernet interface */
#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_IPADDR	192.168.1.251

#define CONFIG_HOSTNAME	"8544ds_unknown"
#define CONFIG_ROOTPATH	"/nfs/mpc85xx"
#define CONFIG_BOOTFILE	"8544ds/uImage.uboot"
#define CONFIG_UBOOTPATH	8544ds/u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK	255.255.0.0

#define CONFIG_LOADADDR	1000000	/*default location for tftp and bootm*/

#define	CONFIG_EXTRA_ENV_SETTINGS				\
"netdev=eth0\0"						\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
"tftpflash=tftpboot $loadaddr $uboot; "			\
	"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" +$filesize; "	\
	"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" $filesize; "	\
	"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
		" +$filesize; "	\
	"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
		" $filesize\0"	\
"consoledev=ttyS0\0"				\
"ramdiskaddr=2000000\0"			\
"ramdiskfile=8544ds/ramdisk.uboot\0"		\
"fdtaddr=1e00000\0"				\
"fdtfile=8544ds/mpc8544ds.dtb\0"		\
"bdev=sda3\0"

#define CONFIG_NFSBOOTCOMMAND		\
 "setenv bootargs root=/dev/nfs rw "	\
 "nfsroot=$serverip:$rootpath "		\
 "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND		\
 "setenv bootargs root=/dev/ram rw "	\
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $ramdiskaddr $ramdiskfile;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		\
 "setenv bootargs root=/dev/$bdev rw "	\
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr - $fdtaddr"

#endif	/* __CONFIG_H */
