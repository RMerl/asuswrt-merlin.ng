/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
 */

/*
 * MPC8610HPCD board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_LINUX_RESET_VEC	0x100	/* Reset vector used by Linux */

/* video */
#define CONFIG_FSL_DIU_FB

#ifdef CONFIG_FSL_DIU_FB
#define CONFIG_SYS_DIU_ADDR	(CONFIG_SYS_CCSRBAR + 0x2c000)
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#endif

#ifdef RUN_DIAG
#define CONFIG_SYS_DIAG_ADDR		0xff800000
#endif

/*
 * virtual address to be used for temporary mappings.  There
 * should be 128k free at this VA.
 */
#define CONFIG_SYS_SCRATCH_VA	0xc0000000

#define CONFIG_PCI1		1	/* PCI controller 1 */
#define CONFIG_PCIE1		1	/* PCIe 1 connected to ULI bridge */
#define CONFIG_PCIE2		1	/* PCIe 2 connected to slot */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

#define CONFIG_ENV_OVERWRITE
#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#define CONFIG_BAT_RW		1	/* Use common BAT rw code */
#define CONFIG_ALTIVEC		1

/*
 * L2CR setup -- make sure this is right for your board!
 */
#define CONFIG_SYS_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E |0x00100000 )

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0)
#endif

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR		0xe0000000	/* relocated CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses CONFIG_SYS_IMMR */

#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH	0x0
#define CONFIG_SYS_CCSRBAR_PHYS		CONFIG_SYS_CCSRBAR_PHYS_LOW

/* DDR Setup */
#define CONFIG_SPD_EEPROM		/* Use SPD for DDR */
#define CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_MAX_DDR_BAT_SIZE	0x80000000	/* BAT mapping size */
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */

/* These are used when DDR doesn't use SPD.  */
#define CONFIG_SYS_SDRAM_SIZE	256		/* DDR is 256MB */

#if 0 /* TODO */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000000F
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80010202	/* Enable, no interleaving */
#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	0x00260802
#define CONFIG_SYS_DDR_TIMING_1	0x3935d322
#define CONFIG_SYS_DDR_TIMING_2	0x14904cc8
#define CONFIG_SYS_DDR_MODE_1		0x00480432
#define CONFIG_SYS_DDR_MODE_2		0x00000000
#define CONFIG_SYS_DDR_INTERVAL	0x06180100
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_CLK_CTRL	0x03800000
#define CONFIG_SYS_DDR_OCD_CTRL	0x00000000
#define CONFIG_SYS_DDR_OCD_STATUS	0x00000000
#define CONFIG_SYS_DDR_CONTROL		0xe3008000	/* Type = DDR2 */
#define CONFIG_SYS_DDR_CONTROL2	0x04400010

#define CONFIG_SYS_DDR_ERR_INT_EN	0x00000000
#define CONFIG_SYS_DDR_ERR_DIS		0x00000000
#define CONFIG_SYS_DDR_SBE		0x000f0000

#endif

#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR     0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1

#define CONFIG_SYS_FLASH_BASE		0xf0000000 /* start of FLASH 128M */
#define CONFIG_SYS_FLASH_BASE2		0xf8000000

#define CONFIG_SYS_FLASH_BANKS_LIST {CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE2}

#define CONFIG_SYS_BR0_PRELIM		0xf8001001 /* port size 16bit */
#define CONFIG_SYS_OR0_PRELIM		0xf8006e65 /* 128MB NOR Flash*/

#define CONFIG_SYS_BR1_PRELIM		0xf0001001 /* port size 16bit */
#define CONFIG_SYS_OR1_PRELIM		0xf8006e65 /* 128MB Promjet */
#if 0 /* TODO */
#define CONFIG_SYS_BR2_PRELIM		0xf0000000
#define CONFIG_SYS_OR2_PRELIM		0xf0000000 /* 256MB NAND Flash - bank 1 */
#endif
#define CONFIG_SYS_BR3_PRELIM		0xe8000801 /* port size 8bit */
#define CONFIG_SYS_OR3_PRELIM		0xfff06ff7 /* 1MB PIXIS area*/

#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xe8000000	/* PIXIS registers */
#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_AUX		0x6	/* PIXIS Auxiliary register; Scratch */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_BRDCFG0		0x8	/* PIXIS Board Configuration Register0*/
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */
#define CONFIG_SYS_PIXIS_VBOOT_MASK	0xC0    /* Reset altbank mask */

#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* sectors per device */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#define CONFIG_SYS_MONITOR_BASE_EARLY   0xfff00000	/* early monitor loc */

#define CONFIG_SYS_FLASH_EMPTY_INFO

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#if defined(CONFIG_SYS_RAMBOOT)
#undef CONFIG_SPD_EEPROM
#define CONFIG_SYS_SDRAM_SIZE	256
#endif

#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_SYS_INIT_RAM_LOCK	1
#ifndef CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4000000	/* Initial RAM address */
#endif
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 KB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(6 * 1024 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BUS
#define CONFIG_SYS_PCI1_MEM_VIRT	CONFIG_SYS_PCI1_MEM_BUS
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BUS	0x0000000
#define CONFIG_SYS_PCI1_IO_PHYS	0xe1000000
#define CONFIG_SYS_PCI1_IO_VIRT	0xe1000000
#define CONFIG_SYS_PCI1_IO_SIZE	0x00100000	/* 1M */

/* controller 1, Base address 0xa000 */
#define CONFIG_SYS_PCIE1_NAME		"ULI"
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	CONFIG_SYS_PCIE1_MEM_BUS
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe3000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00100000	/* 1M */

/* controller 2, Base Address 0x9000 */
#define CONFIG_SYS_PCIE2_NAME		"Slot 1"
#define CONFIG_SYS_PCIE2_MEM_BUS	0x90000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	CONFIG_SYS_PCIE2_MEM_BUS
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000	/* reuse mem LAW */
#define CONFIG_SYS_PCIE2_IO_PHYS	0xe2000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00100000	/* 1M */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#define CONFIG_ULI526X

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_PCI_OHCI		1
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"ohci_pci"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 15
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS	1

#if !defined(CONFIG_PCI_PNP)
#define PCI_ENET0_IOADDR	0xe0000000
#define PCI_ENET0_MEMADDR	0xe0000000
#define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	4
#define CONFIG_SYS_SCSI_MAX_LUN	1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#endif

#endif	/* CONFIG_PCI */

/*
 * BAT0		2G	Cacheable, non-guarded
 * 0x0000_0000	2G	DDR
 */
#define CONFIG_SYS_DBAT0L	(BATL_PP_RW)
#define CONFIG_SYS_IBAT0L	(BATL_PP_RW)

/*
 * BAT1		1G	Cache-inhibited, guarded
 * 0x8000_0000	256M	PCI-1 Memory
 * 0xa000_0000	256M	PCI-Express 1 Memory
 * 0x9000_0000	256M	PCI-Express 2 Memory
 */

#define CONFIG_SYS_DBAT1L	(CONFIG_SYS_PCI1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	(CONFIG_SYS_PCI1_MEM_VIRT | BATU_BL_1G | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCI1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U	CONFIG_SYS_DBAT1U

/*
 * BAT2		16M	Cache-inhibited, guarded
 * 0xe100_0000	1M	PCI-1 I/O
 */

#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_PCI1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_PCI1_IO_VIRT | BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	CONFIG_SYS_DBAT2U

/*
 * BAT3		4M	Cache-inhibited, guarded
 * 0xe000_0000	4M	CCSR
 */

#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	(CONFIG_SYS_CCSRBAR | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	CONFIG_SYS_DBAT3U

#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
#define CONFIG_SYS_CCSR_DEFAULT_DBATL (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATL_PP_RW | BATL_CACHEINHIBIT \
				       | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_CCSR_DEFAULT_DBATU (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_CCSR_DEFAULT_IBATL (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_CCSR_DEFAULT_IBATU CONFIG_SYS_CCSR_DEFAULT_DBATU
#endif

/*
 * BAT4		32M	Cache-inhibited, guarded
 * 0xe200_0000	1M	PCI-Express 2 I/O
 * 0xe300_0000	1M	PCI-Express 1 I/O
 */

#define CONFIG_SYS_DBAT4L	(CONFIG_SYS_PCIE2_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	(CONFIG_SYS_PCIE2_IO_PHYS | BATU_BL_32M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_PCIE2_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U	CONFIG_SYS_DBAT4U

/*
 * BAT5		128K	Cacheable, non-guarded
 * 0xe400_0000	128K	Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT5U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT5L	CONFIG_SYS_DBAT5L
#define CONFIG_SYS_IBAT5U	CONFIG_SYS_DBAT5U

/*
 * BAT6		256M	Cache-inhibited, guarded
 * 0xf000_0000	256M	FLASH
 */
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_FLASH_BASE	 | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	(CONFIG_SYS_FLASH_BASE	 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_FLASH_BASE | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	CONFIG_SYS_DBAT6U

/* Map the last 1M of flash where we're running from reset */
#define CONFIG_SYS_DBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U_EARLY	(CONFIG_SYS_TEXT_BASE | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U_EARLY	CONFIG_SYS_DBAT6U_EARLY

/*
 * BAT7		4M	Cache-inhibited, guarded
 * 0xe800_0000	4M	PIXIS
 */
#define CONFIG_SYS_DBAT7L	(PIXIS_BASE | BATL_PP_RW | BATL_CACHEINHIBIT \
			| BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT7U	(PIXIS_BASE | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT7L	(PIXIS_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT7U	CONFIG_SYS_DBAT7U

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 126k (one sector) for env */
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

#define CONFIG_WATCHDOG			/* watchdog enabled */
#define CONFIG_SYS_WATCHDOG_FREQ	5000	/* Feed interval, 5s */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(256 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_IPADDR		192.168.1.100

#define CONFIG_HOSTNAME		"unknown"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	8610hpcd/u-boot.bin

#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		0x10000000

#if defined(CONFIG_PCI1)
#define PCI_ENV \
 "pcireg=md ${a}000 3; echo o;md ${a}c00 25; echo i; md ${a}da0 15;" \
	"echo e;md ${a}e00 9\0" \
 "pci1regs=setenv a e0008; run pcireg\0" \
 "pcierr=md ${a}e00 8; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;" \
	"pci d.w $b.0 56 1\0" \
 "pcierrc=mw ${a}e00 ffffffff; pci w.b $b.0 7 ff; pci w.w $b.0 1e ffff;" \
	"pci w.w $b.0 56 ffff\0"	\
 "pci1err=setenv a e0008; run pcierr\0"	\
 "pci1errc=setenv a e0008; run pcierrc\0"
#else
#define	PCI_ENV ""
#endif

#if defined(CONFIG_PCIE1) || defined(CONFIG_PCIE2)
#define PCIE_ENV \
 "pciereg=md ${a}000 6; md ${a}020 4; md ${a}bf8 2; echo o;md ${a}c00 25;" \
	"echo i; md ${a}da0 15; echo e;md ${a}e00 e; echo d; md ${a}f00 c\0" \
 "pcie1regs=setenv a e000a; run pciereg\0"	\
 "pcie2regs=setenv a e0009; run pciereg\0"	\
 "pcieerr=md ${a}020 1; md ${a}e00; pci d.b $b.0 7 1; pci d.w $b.0 1e 1;"\
	"pci d.w $b.0 56 1; pci d $b.0 104 1; pci d $b.0 110 1;"	\
	"pci d $b.0 130 1\0" \
 "pcieerrc=mw ${a}020 ffffffff; mw ${a}e00 ffffffff; pci w.b $b.0 7 ff;"\
	"pci w.w $b.0 1e ffff; pci w.w $b.0 56 ffff; pci w $b.0 104 ffffffff;" \
	"pci w $b.0 110 ffffffff; pci w $b.0 130 ffffffff\0"		\
 "pciecfg=pci d $b.0 0 20; pci d $b.0 100 e; pci d $b.0 400 69\0"	\
 "pcie1err=setenv a e000a; run pcieerr\0"	\
 "pcie2err=setenv a e0009; run pcieerr\0"	\
 "pcie1errc=setenv a e000a; run pcieerrc\0"	\
 "pcie2errc=setenv a e0009; run pcieerrc\0"
#else
#define	PCIE_ENV ""
#endif

#define DMA_ENV \
 "dma0=mw ${d}104 ffffffff;mw ${d}110 50000;mw ${d}114 $sad0;mw ${d}118 50000;"\
	"mw ${d}120 $bc0;mw ${d}100 f03c404; mw ${d}11c $dad0; md ${d}100 9\0" \
 "dma1=mw ${d}184 ffffffff;mw ${d}190 50000;mw ${d}194 $sad1;mw ${d}198 50000;"\
	"mw ${d}1a0 $bc1;mw ${d}180 f03c404; mw ${d}19c $dad1; md ${d}180 9\0" \
 "dma2=mw ${d}204 ffffffff;mw ${d}210 50000;mw ${d}214 $sad2;mw ${d}218 50000;"\
	"mw ${d}220 $bc2;mw ${d}200 f03c404; mw ${d}21c $dad2; md ${d}200 9\0" \
 "dma3=mw ${d}284 ffffffff;mw ${d}290 50000;mw ${d}294 $sad3;mw ${d}298 50000;"\
	"mw ${d}2a0 $bc3;mw ${d}280 f03c404; mw ${d}29c $dad3; md ${d}280 9\0"

#ifdef ENV_DEBUG
#define	CONFIG_EXTRA_ENV_SETTINGS				\
"netdev=eth0\0"							\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
"tftpflash=tftpboot $loadaddr $uboot; "				\
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
"consoledev=ttyS0\0"						\
"ramdiskaddr=0x18000000\0"					\
"ramdiskfile=8610hpcd/ramdisk.uboot\0"				\
"fdtaddr=0x17c00000\0"						\
"fdtfile=8610hpcd/mpc8610_hpcd.dtb\0"				\
"bdev=sda3\0"					\
"en-wd=mw.b f8100010 0x08; echo -expect:- 08; md.b f8100010 1\0" \
"dis-wd=mw.b f8100010 0x00; echo -expect:- 00; md.b f8100010 1\0" \
"maxcpus=1"	\
"eoi=mw e00400b0 0\0"						\
"iack=md e00400a0 1\0"						\
"ddrreg=md ${a}000 8; md ${a}080 8;md ${a}100 d; md ${a}140 4;" \
	"md ${a}bf0 4; md ${a}e00 3; md ${a}e20 3; md ${a}e40 7;" \
	"md ${a}f00 5\0" \
"ddr1regs=setenv a e0002; run ddrreg\0" \
"gureg=md ${a}000 2c; md ${a}0b0 1; md ${a}0c0 1; md ${a}800 1;" \
	"md ${a}900 6; md ${a}a00 1; md ${a}b20 3; md ${a}e00 1;" \
	"md ${a}e60 1; md ${a}ef0 1d\0" \
"guregs=setenv a e00e0; run gureg\0" \
"mcmreg=md ${a}000 1b; md ${a}bf8 2; md ${a}e00 5\0" \
"mcmregs=setenv a e0001; run mcmreg\0" \
"diuregs=md e002c000 1d\0" \
"dium=mw e002c01c\0" \
"diuerr=md e002c014 1\0" \
"pmregs=md e00e1000 2b\0" \
"lawregs=md e0000c08 4b\0" \
"lbcregs=md e0005000 36\0" \
"dma0regs=md e0021100 12\0" \
"dma1regs=md e0021180 12\0" \
"dma2regs=md e0021200 12\0" \
"dma3regs=md e0021280 12\0" \
 PCI_ENV \
 PCIE_ENV \
 DMA_ENV
#else
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"netdev=eth0\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"		\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=0x18000000\0"				\
	"ramdiskfile=8610hpcd/ramdisk.uboot\0"			\
	"fdtaddr=0x17c00000\0"					\
	"fdtfile=8610hpcd/mpc8610_hpcd.dtb\0"			\
	"bdev=sda3\0"
#endif

#define CONFIG_NFSBOOTCOMMAND					\
 "setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "				\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"		\
 "tftp $loadaddr $bootfile;"					\
 "tftp $fdtaddr $fdtfile;"					\
 "bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND \
 "setenv bootargs root=/dev/ram rw "				\
	"console=$consoledev,$baudrate $othbootargs;"		\
 "tftp $ramdiskaddr $ramdiskfile;"				\
 "tftp $loadaddr $bootfile;"					\
 "tftp $fdtaddr $fdtfile;"					\
 "bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		\
 "setenv bootargs root=/dev/$bdev rw "	\
	"console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"		\
 "tftp $fdtaddr $fdtfile;"		\
 "bootm $loadaddr - $fdtaddr"

#endif	/* __CONFIG_H */
