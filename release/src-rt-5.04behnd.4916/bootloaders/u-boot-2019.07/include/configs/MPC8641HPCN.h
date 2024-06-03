/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2006, 2010-2011 Freescale Semiconductor.
 *
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

/*
 * MPC8641HPCN board configuration file
 *
 * Make sure you change the MAC address and other network params first,
 * search for CONFIG_SERVERIP, etc. in this file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_LINUX_RESET_VEC	0x100	/* Reset vector used by Linux */
#define CONFIG_ADDR_MAP		1	/* Use addr map */

/*
 * default CCSRBAR is at 0xff700000
 * assume U-Boot is less than 0.5MB
 */

#ifdef RUN_DIAG
#define CONFIG_SYS_DIAG_ADDR	     CONFIG_SYS_FLASH_BASE
#endif

/*
 * virtual address to be used for temporary mappings.  There
 * should be 128k free at this VA.
 */
#define CONFIG_SYS_SCRATCH_VA	0xe0000000

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */

#define CONFIG_PCIE1		1	/* PCIE controller 1 (ULI bridge) */
#define CONFIG_PCIE2		1	/* PCIE controller 2 (slot) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAT_RW		1	/* Use common BAT rw code */
#define CONFIG_SYS_NUM_ADDR_MAP 8	/* Number of addr map slots = 8 dbats */

#define CONFIG_ALTIVEC		1

/*
 * L2CR setup -- make sure this is right for your board!
 */
#define CONFIG_SYS_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E)

#ifndef CONFIG_SYS_CLK_FREQ
#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0)
#endif

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * With the exception of PCI Memory and Rapid IO, most devices will simply
 * add CONFIG_SYS_PHYS_ADDR_HIGH to the front of the 32-bit VA to get the PA
 * when 36-bit is enabled.  When 36-bit is not enabled, these bits are 0.
 */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PHYS_ADDR_HIGH 0x0000000f
#else
#define CONFIG_SYS_PHYS_ADDR_HIGH 0x00000000
#endif

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR		0xffe00000	/* relocated CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses CONFIG_SYS_IMMR */

/* Physical addresses */
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH	CONFIG_SYS_PHYS_ADDR_HIGH
#define CONFIG_SYS_CCSRBAR_PHYS \
	PAIRED_PHYS_TO_PHYS(CONFIG_SYS_CCSRBAR_PHYS_LOW, \
			    CONFIG_SYS_CCSRBAR_PHYS_HIGH)

#define CONFIG_HWCONFIG	/* use hwconfig to control memory interleaving */

/*
 * DDR Setup
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_MAX_DDR_BAT_SIZE	0x80000000	/* BAT mapping size */
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	2
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/*
 * I2C addresses of SPD EEPROMs
 */
#define SPD_EEPROM_ADDRESS1	0x51	/* CTLR 0 DIMM 0 */
#define SPD_EEPROM_ADDRESS2	0x52	/* CTLR 0 DIMM 1 */
#define SPD_EEPROM_ADDRESS3	0x53	/* CTLR 1 DIMM 0 */
#define SPD_EEPROM_ADDRESS4	0x54	/* CTLR 1 DIMM 1 */

/*
 * These are used when DDR doesn't use SPD.
 */
#define CONFIG_SYS_SDRAM_SIZE		256		/* DDR is 256MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000000F
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80010102      /* Enable, no interleaving */
#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	0x00260802
#define CONFIG_SYS_DDR_TIMING_1	0x39357322
#define CONFIG_SYS_DDR_TIMING_2	0x14904cc8
#define CONFIG_SYS_DDR_MODE_1		0x00480432
#define CONFIG_SYS_DDR_MODE_2		0x00000000
#define CONFIG_SYS_DDR_INTERVAL	0x06090100
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_CLK_CTRL	0x03800000
#define CONFIG_SYS_DDR_OCD_CTRL	0x00000000
#define CONFIG_SYS_DDR_OCD_STATUS	0x00000000
#define CONFIG_SYS_DDR_CONTROL		0xe3008000	/* Type = DDR2 */
#define CONFIG_SYS_DDR_CONTROL2	0x04400000

#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR     0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1

#define CONFIG_SYS_FLASH_BASE		0xef800000     /* start of FLASH 8M */
#define CONFIG_SYS_FLASH_BASE_PHYS_LOW	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_FLASH_BASE_PHYS \
	PAIRED_PHYS_TO_PHYS(CONFIG_SYS_FLASH_BASE_PHYS_LOW, \
			    CONFIG_SYS_PHYS_ADDR_HIGH)

#define CONFIG_SYS_FLASH_BANKS_LIST {CONFIG_SYS_FLASH_BASE_PHYS}

#define CONFIG_SYS_BR0_PRELIM	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) \
				 | 0x00001001)	/* port size 16bit */
#define CONFIG_SYS_OR0_PRELIM	0xff806ff7	/* 8MB Boot Flash area*/

#define CONFIG_SYS_BR2_PRELIM	(BR_PHYS_ADDR(CF_BASE_PHYS)		\
				 | 0x00001001)	/* port size 16bit */
#define CONFIG_SYS_OR2_PRELIM	0xffffeff7	/* 32k Compact Flash */

#define CONFIG_SYS_BR3_PRELIM	(BR_PHYS_ADDR(PIXIS_BASE_PHYS)	\
				 | 0x00000801) /* port size 8bit */
#define CONFIG_SYS_OR3_PRELIM	0xffffeff7	/* 32k PIXIS area*/

/*
 * The LBC_BASE is the base of the region that contains the PIXIS and the CF.
 * The PIXIS and CF by themselves aren't large enough to take up the 128k
 * required for the smallest BAT mapping, so there's a 64k hole.
 */
#define CONFIG_SYS_LBC_BASE		0xffde0000
#define CONFIG_SYS_LBC_BASE_PHYS_LOW	CONFIG_SYS_LBC_BASE

#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE		(CONFIG_SYS_LBC_BASE + 0x00010000)
#define PIXIS_BASE_PHYS_LOW	(CONFIG_SYS_LBC_BASE_PHYS_LOW + 0x00010000)
#define PIXIS_BASE_PHYS		PAIRED_PHYS_TO_PHYS(PIXIS_BASE_PHYS_LOW, \
						    CONFIG_SYS_PHYS_ADDR_HIGH)
#define PIXIS_SIZE		0x00008000	/* 32k */
#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_AUX		0x6	/* PIXIS Auxiliary register; Scratch register */
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
#define CONFIG_SYS_PIXIS_VBOOT_MASK	0x40	/* Reset altbank mask*/

/* Compact flash shares a BAT with PIXIS; make sure they're contiguous */
#define CF_BASE			(PIXIS_BASE + PIXIS_SIZE)
#define CF_BASE_PHYS		(PIXIS_BASE_PHYS + PIXIS_SIZE)

#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* sectors per device */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* start of monitor */
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
#define CONFIG_SYS_INIT_RAM_ADDR	0x0fd00000	/* Initial RAM address */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0xf8400000	/* Initial RAM address */
#endif
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	 /* Reserved for malloc */

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/*
 * RapidIO MMU
 */
#define CONFIG_SYS_SRIO1_MEM_BASE	0x80000000	/* base address */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_SRIO1_MEM_PHYS_LOW	0x00000000
#define CONFIG_SYS_SRIO1_MEM_PHYS_HIGH  0x0000000c
#else
#define CONFIG_SYS_SRIO1_MEM_PHYS_LOW	CONFIG_SYS_SRIO1_MEM_BASE
#define CONFIG_SYS_SRIO1_MEM_PHYS_HIGH  0x00000000
#endif
#define CONFIG_SYS_SRIO1_MEM_PHYS \
	PAIRED_PHYS_TO_PHYS(CONFIG_SYS_SRIO1_MEM_PHYS_LOW, \
			    CONFIG_SYS_SRIO1_MEM_PHYS_HIGH)
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x20000000	/* 128M */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */

#define CONFIG_SYS_PCIE1_NAME		"ULI"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS_LOW	0x00000000
#define CONFIG_SYS_PCIE1_MEM_PHYS_HIGH	0x0000000c
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	CONFIG_SYS_PCIE1_MEM_VIRT
#define CONFIG_SYS_PCIE1_MEM_PHYS_LOW	CONFIG_SYS_PCIE1_MEM_VIRT
#define CONFIG_SYS_PCIE1_MEM_PHYS_HIGH	0x00000000
#endif
#define CONFIG_SYS_PCIE1_MEM_PHYS \
	PAIRED_PHYS_TO_PHYS(CONFIG_SYS_PCIE1_MEM_PHYS_LOW, \
			    CONFIG_SYS_PCIE1_MEM_PHYS_HIGH)
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE1_IO_PHYS_LOW	CONFIG_SYS_PCIE1_IO_VIRT
#define CONFIG_SYS_PCIE1_IO_PHYS \
	PAIRED_PHYS_TO_PHYS(CONFIG_SYS_PCIE1_IO_PHYS_LOW, \
			    CONFIG_SYS_PHYS_ADDR_HIGH)
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64K */

#ifdef CONFIG_PHYS_64BIT
/*
 * Use the same PCI bus address on PCIE1 and PCIE2 if we have PHYS_64BIT.
 * This will increase the amount of PCI address space available for
 * for mapping RAM.
 */
#define CONFIG_SYS_PCIE2_MEM_BUS	CONFIG_SYS_PCIE1_MEM_BUS
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	(CONFIG_SYS_PCIE1_MEM_BUS \
					 + CONFIG_SYS_PCIE1_MEM_SIZE)
#endif
#define CONFIG_SYS_PCIE2_MEM_VIRT 	(CONFIG_SYS_PCIE1_MEM_VIRT \
					 + CONFIG_SYS_PCIE1_MEM_SIZE)
#define CONFIG_SYS_PCIE2_MEM_PHYS_LOW	(CONFIG_SYS_PCIE1_MEM_PHYS_LOW \
					 + CONFIG_SYS_PCIE1_MEM_SIZE)
#define CONFIG_SYS_PCIE2_MEM_PHYS_HIGH	CONFIG_SYS_PCIE1_MEM_PHYS_HIGH
#define CONFIG_SYS_PCIE2_MEM_PHYS	(CONFIG_SYS_PCIE1_MEM_PHYS \
					 + CONFIG_SYS_PCIE1_MEM_SIZE)
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_VIRT 	(CONFIG_SYS_PCIE1_IO_VIRT \
					 + CONFIG_SYS_PCIE1_IO_SIZE)
#define CONFIG_SYS_PCIE2_IO_PHYS_LOW	(CONFIG_SYS_PCIE1_IO_PHYS_LOW \
					 + CONFIG_SYS_PCIE1_IO_SIZE)
#define CONFIG_SYS_PCIE2_IO_PHYS	(CONFIG_SYS_PCIE1_IO_PHYS \
					 + CONFIG_SYS_PCIE1_IO_SIZE)
#define CONFIG_SYS_PCIE2_IO_SIZE	CONFIG_SYS_PCIE1_IO_SIZE

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

/************************************************************
 * USB support
 ************************************************************/
#define CONFIG_PCI_OHCI			1
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"ohci_pci"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS	1

/*PCIE video card used*/
#define VIDEO_IO_OFFSET		CONFIG_SYS_PCIE2_IO_VIRT

/*PCI video card used*/
/*#define VIDEO_IO_OFFSET	CONFIG_SYS_PCIE1_IO_VIRT*/

/* video */

#if defined(CONFIG_VIDEO)
#define CONFIG_BIOSEMU
#define CONFIG_ATI_RADEON_FB
#define CONFIG_VIDEO_LOGO
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS CONFIG_SYS_PCIE2_IO_VIRT
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	4
#define CONFIG_SYS_SCSI_MAX_LUN	1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#endif

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3		1
#define CONFIG_TSEC3_NAME	"eTSEC3"
#define CONFIG_TSEC4		1
#define CONFIG_TSEC4_NAME	"eTSEC4"

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define CONFIG_ETHPRIME		"eTSEC1"

#endif	/* CONFIG_TSEC_ENET */

#ifdef CONFIG_PHYS_64BIT
#define PHYS_HIGH_TO_BXPN(x) ((x & 0x0000000e) << 8)
#define PHYS_HIGH_TO_BX(x) ((x & 0x00000001) << 2)

/* Put physical address into the BAT format */
#define BAT_PHYS_ADDR(low, high) \
	(low | PHYS_HIGH_TO_BXPN(high) | PHYS_HIGH_TO_BX(high))
/* Convert high/low pairs to actual 64-bit value */
#define PAIRED_PHYS_TO_PHYS(low, high) (low | ((u64)high << 32))
#else
/* 32-bit systems just ignore the "high" bits */
#define BAT_PHYS_ADDR(low, high)        (low)
#define PAIRED_PHYS_TO_PHYS(low, high)  (low)
#endif

/*
 * BAT0		DDR
 */
#define CONFIG_SYS_DBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)

/*
 * BAT1		LBC (PIXIS/CF)
 */
#define CONFIG_SYS_DBAT1L	(BAT_PHYS_ADDR(CONFIG_SYS_LBC_BASE_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	(CONFIG_SYS_LBC_BASE | BATU_BL_128K \
				 | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT1L	(BAT_PHYS_ADDR(CONFIG_SYS_LBC_BASE_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT1U	CONFIG_SYS_DBAT1U

/* if CONFIG_PCI:
 * BAT2		PCIE1 and PCIE1 MEM
 * if CONFIG_RIO
 * BAT2		Rapidio Memory
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_SYS_DBAT2L	(BAT_PHYS_ADDR(CONFIG_SYS_PCIE1_MEM_PHYS_LOW, \
					       CONFIG_SYS_PCIE1_MEM_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT \
				 | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_PCIE1_MEM_VIRT | BATU_BL_1G \
				 | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT2L	(BAT_PHYS_ADDR(CONFIG_SYS_PCIE1_MEM_PHYS_LOW, \
					       CONFIG_SYS_PCIE1_MEM_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	CONFIG_SYS_DBAT2U
#else /* CONFIG_RIO */
#define CONFIG_SYS_DBAT2L	(BAT_PHYS_ADDR(CONFIG_SYS_SRIO1_MEM_PHYS_LOW, \
					       CONFIG_SYS_SRIO1_MEM_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT | \
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_SRIO1_MEM_BASE | BATU_BL_512M \
				 | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT2L	(BAT_PHYS_ADDR(CONFIG_SYS_SRIO1_MEM_PHYS_LOW, \
					       CONFIG_SYS_SRIO1_MEM_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	CONFIG_SYS_DBAT2U
#endif

/*
 * BAT3		CCSR Space
 */
#define CONFIG_SYS_DBAT3L	(BAT_PHYS_ADDR(CONFIG_SYS_CCSRBAR_PHYS_LOW, \
					       CONFIG_SYS_CCSRBAR_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT \
				 | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	(CONFIG_SYS_CCSRBAR | BATU_BL_1M | BATU_VS \
				 | BATU_VP)
#define CONFIG_SYS_IBAT3L	(BAT_PHYS_ADDR(CONFIG_SYS_CCSRBAR_PHYS_LOW, \
					       CONFIG_SYS_CCSRBAR_PHYS_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT)
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
 * BAT4		PCIE1_IO and PCIE2_IO
 */
#define CONFIG_SYS_DBAT4L	(BAT_PHYS_ADDR(CONFIG_SYS_PCIE1_IO_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT \
				 | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	(CONFIG_SYS_PCIE1_IO_VIRT | BATU_BL_128K \
				 | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT4L	(BAT_PHYS_ADDR(CONFIG_SYS_PCIE1_IO_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U	CONFIG_SYS_DBAT4U

/*
 * BAT5		Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT5U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT5L	CONFIG_SYS_DBAT5L
#define CONFIG_SYS_IBAT5U	CONFIG_SYS_DBAT5U

/*
 * BAT6		FLASH
 */
#define CONFIG_SYS_DBAT6L	(BAT_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_CACHEINHIBIT \
				 | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	(CONFIG_SYS_FLASH_BASE | BATU_BL_8M | BATU_VS \
				 | BATU_VP)
#define CONFIG_SYS_IBAT6L	(BAT_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS_LOW, \
					       CONFIG_SYS_PHYS_ADDR_HIGH) \
				 | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	CONFIG_SYS_DBAT6U

/* Map the last 1M of flash where we're running from reset */
#define CONFIG_SYS_DBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U_EARLY	(CONFIG_SYS_TEXT_BASE | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U_EARLY	CONFIG_SYS_DBAT6U_EARLY

/*
 * BAT7		FREE - used later for tmp mappings
 */
#define CONFIG_SYS_DBAT7L 0x00000000
#define CONFIG_SYS_DBAT7U 0x00000000
#define CONFIG_SYS_IBAT7L 0x00000000
#define CONFIG_SYS_IBAT7U 0x00000000

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
    #define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
    #define CONFIG_ENV_SECT_SIZE		0x10000	/* 64K(one sector) for env */
#else
    #define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#endif
#define CONFIG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

#undef CONFIG_WATCHDOG			/* watchdog disabled */

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

#define CONFIG_HAS_ETH0		1
#define CONFIG_HAS_ETH1		1
#define CONFIG_HAS_ETH2		1
#define CONFIG_HAS_ETH3		1

#define CONFIG_IPADDR		192.168.1.100

#define CONFIG_HOSTNAME		"unknown"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		0x10000000

#define	CONFIG_EXTRA_ENV_SETTINGS					\
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
	"ramdiskaddr=0x18000000\0"						\
	"ramdiskfile=your.ramdisk.u-boot\0"				\
	"fdtaddr=0x17c00000\0"						\
	"fdtfile=mpc8641_hpcn.dtb\0"					\
	"en-wd=mw.b ffdf0010 0x08; echo -expect:- 08; md.b ffdf0010 1\0"			\
	"dis-wd=mw.b ffdf0010 0x00; echo -expect:- 00; md.b ffdf0010 1\0" \
	"maxcpus=2"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
	      "nfsroot=$serverip:$rootpath "				\
	      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
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

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
