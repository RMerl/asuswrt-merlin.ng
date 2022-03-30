/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007-2009,2010-2012 Freescale Semiconductor, Inc.
 */

/*
 * mpc8536ds board configuration file
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/freescale/common/ics307_clk.h"

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD		1
#define CONFIG_RESET_VECTOR_ADDRESS	0xf8fffffc
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH		1
#define CONFIG_RESET_VECTOR_ADDRESS	0xf8fffffc
#endif

#ifndef	CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_PCI1		1	/* Enable PCI controller 1 */
#define CONFIG_PCIE1		1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controller 2 (slot 2) */
#define CONFIG_PCIE3		1	/* PCIE controller 3 (ULI bridge) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */


#define CONFIG_ENV_OVERWRITE

#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk() /* sysclk for MPC85xx */
#define CONFIG_DDR_CLK_FREQ	get_board_ddr_clk()
#define CONFIG_ICS307_REFCLK_HZ	33333000  /* ICS307 clock chip ref freq */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

#define CONFIG_ENABLE_36BIT_PHYS	1

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP			1
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_SYS_MEMTEST_START 0x00010000	/* skip exception vectors */
#define CONFIG_SYS_MEMTEST_END   0x1f000000	/* skip u-boot at top of RAM */

/*
 * Config the L2 Cache as L2 SRAM
 */
#define CONFIG_SYS_INIT_L2_ADDR		0xf8f80000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	0xff8f80000ull
#else
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#endif
#define CONFIG_SYS_L2_SIZE		(512 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)

#define CONFIG_SYS_CCSRBAR		0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

#if defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_CCSR_DO_NOT_RELOCATE
#endif

/* DDR Setup */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	2

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */
#define CONFIG_SYS_SPD_BUS_NUM		1

/* These are used when DDR doesn't use SPD. */
#define CONFIG_SYS_SDRAM_SIZE		256	/* DDR is 256MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000001F
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80010102 /* Enable, no interleaving */
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
#define CONFIG_SYS_DDR_CONTROL	0xC3008000	/* Type = DDR2 */
#define CONFIG_SYS_DDR_CONTROL2	0x04400010

#define CONFIG_SYS_DDR_ERR_INT_EN	0x0000000d
#define CONFIG_SYS_DDR_ERR_DIS		0x00000000
#define CONFIG_SYS_DDR_SBE		0x00010000

/* Make sure required options are set */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Memory map -- xxx -this is wrong, needs updating
 *
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 * 0x8000_0000	0xbfff_ffff	PCI Express Mem		1G non-cacheable
 * 0xc000_0000	0xdfff_ffff	PCI			512M non-cacheable
 * 0xe100_0000	0xe3ff_ffff	PCI IO range		4M non-cacheable
 *
 * Localbus cacheable (TBD)
 * 0xXXXX_XXXX	0xXXXX_XXXX	SRAM			YZ M Cacheable
 *
 * Localbus non-cacheable
 * 0xe000_0000	0xe7ff_ffff	Promjet/free		128M non-cacheable
 * 0xe800_0000	0xefff_ffff	FLASH			128M non-cacheable
 * 0xffa0_0000	0xffaf_ffff	NAND			1M non-cacheable
 * 0xffdf_0000	0xffdf_7fff	PIXIS			32K non-cacheable TLB0
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_FLASH_BASE		0xe0000000	/* start of FLASH 128M */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_FLASH_BR_PRELIM \
	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000) | BR_PS_16 | BR_V)
#define CONFIG_FLASH_OR_PRELIM	0xf8000ff7

#define CONFIG_SYS_BR1_PRELIM \
		(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) \
		 | BR_PS_16 | BR_V)
#define CONFIG_SYS_OR1_PRELIM	0xf8000ff7

#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000, \
				      CONFIG_SYS_FLASH_BASE_PHYS }
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS 45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#if defined(CONFIG_RAMBOOT_SDCARD) || defined(CONFIG_RAMBOOT_SPIFLASH)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_AMD_CHECK_DQ7

#define CONFIG_HWCONFIG			/* enable hwconfig */
#define CONFIG_FSL_PIXIS	1	/* use common PIXIS code */
#define PIXIS_BASE	0xffdf0000	/* PIXIS registers */
#ifdef CONFIG_PHYS_64BIT
#define PIXIS_BASE_PHYS	0xfffdf0000ull
#else
#define PIXIS_BASE_PHYS	PIXIS_BASE
#endif

#define CONFIG_SYS_BR3_PRELIM	(BR_PHYS_ADDR(PIXIS_BASE_PHYS) | BR_PS_8 | BR_V)
#define CONFIG_SYS_OR3_PRELIM	0xffffeff7	/* 32KB but only 4k mapped */

#define PIXIS_ID		0x0	/* Board ID at offset 0 */
#define PIXIS_VER		0x1	/* Board version at offset 1 */
#define PIXIS_PVER		0x2	/* PIXIS FPGA version at offset 2 */
#define PIXIS_CSR   		0x3	/* PIXIS General control/status register */
#define PIXIS_RST		0x4	/* PIXIS Reset Control register */
#define PIXIS_PWR		0x5	/* PIXIS Power status register */
#define PIXIS_AUX		0x6	/* Auxiliary 1 register */
#define PIXIS_SPD		0x7	/* Register for SYSCLK speed */
#define PIXIS_AUX2		0x8	/* Auxiliary 2 register */
#define PIXIS_VCTL		0x10	/* VELA Control Register */
#define PIXIS_VSTAT		0x11	/* VELA Status Register */
#define PIXIS_VCFGEN0		0x12	/* VELA Config Enable 0 */
#define PIXIS_VCFGEN1		0x13	/* VELA Config Enable 1 */
#define PIXIS_VCORE0	 	0x14	/* VELA VCORE0 Register */
#define PIXIS_VBOOT		0x16	/* VELA VBOOT Register */
#define PIXIS_VBOOT_LBMAP	0xe0	/* VBOOT - CFG_LBMAP */
#define PIXIS_VBOOT_LBMAP_NOR0	0x00	/* cfg_lbmap - boot from NOR 0 */
#define PIXIS_VBOOT_LBMAP_NOR1	0x01	/* cfg_lbmap - boot from NOR 1 */
#define PIXIS_VBOOT_LBMAP_NOR2	0x02	/* cfg_lbmap - boot from NOR 2 */
#define PIXIS_VBOOT_LBMAP_NOR3	0x03	/* cfg_lbmap - boot from NOR 3 */
#define PIXIS_VBOOT_LBMAP_PJET	0x04	/* cfg_lbmap - boot from projet */
#define PIXIS_VBOOT_LBMAP_NAND	0x05	/* cfg_lbmap - boot from NAND */
#define PIXIS_VSPEED0		0x17	/* VELA VSpeed 0 */
#define PIXIS_VSPEED1		0x18	/* VELA VSpeed 1 */
#define PIXIS_VSPEED2		0x19	/* VELA VSpeed 2 */
#define PIXIS_VSYSCLK0		0x1A	/* VELA SYSCLK0 Register */
#define PIXIS_VSYSCLK1		0x1B	/* VELA SYSCLK1 Register */
#define PIXIS_VSYSCLK2		0x1C	/* VELA SYSCLK2 Register */
#define PIXIS_VDDRCLK0		0x1D	/* VELA DDRCLK0 Register */
#define PIXIS_VDDRCLK1		0x1E	/* VELA DDRCLK1 Register */
#define PIXIS_VDDRCLK2		0x1F	/* VELA DDRCLK2 Register */
#define PIXIS_VWATCH		0x24    /* Watchdog Register */
#define PIXIS_LED		0x25    /* LED Register */

#define PIXIS_SPD_SYSCLK	0x7	/* SYSCLK option */

/* old pixis referenced names */
#define PIXIS_VCLKH		0x19	/* VELA VCLKH register */
#define PIXIS_VCLKL		0x1A	/* VELA VCLKL register */
#define CONFIG_SYS_PIXIS_VBOOT_MASK	0x4e

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000	/* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000	/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET \
		(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024) /* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(1024 * 1024)	/* Reserved for malloc */

#ifndef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xffa00000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_NAND_BASE_PHYS	0xfffa00000ull
#else
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#endif
#else
#define CONFIG_SYS_NAND_BASE		0xfff00000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_NAND_BASE_PHYS	0xffff00000ull
#else
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#endif
#endif
#define CONFIG_SYS_NAND_BASE_LIST     { CONFIG_SYS_NAND_BASE,\
				CONFIG_SYS_NAND_BASE + 0x40000, \
				CONFIG_SYS_NAND_BASE + 0x80000, \
				CONFIG_SYS_NAND_BASE + 0xC0000}
#define CONFIG_SYS_MAX_NAND_DEVICE	4
#define CONFIG_NAND_FSL_ELBC	1
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)

/* NAND boot: 4K NAND loader config */
#define CONFIG_SYS_NAND_SPL_SIZE	0x1000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	((768 << 10) - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_DST	(CONFIG_SYS_INIT_L2_ADDR)
#define CONFIG_SYS_NAND_U_BOOT_START \
		(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_NAND_SPL_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(0)
#define CONFIG_SYS_NAND_U_BOOT_RELOC	(CONFIG_SYS_INIT_L2_END - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP ((CONFIG_SYS_INIT_L2_END - 1) & ~0xF)

/* NAND flash config */
#define CONFIG_SYS_NAND_BR_PRELIM \
		(BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
		| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
		| BR_PS_8		/* Port Size = 8 bit */ \
		| BR_MS_FCM		/* MSEL = FCM */ \
		| BR_V)			/* valid */
#define CONFIG_SYS_NAND_OR_PRELIM	(0xFFFC0000	/* length 256K */ \
		| OR_FCM_PGS		/* Large Page*/ \
		| OR_FCM_CSCT \
		| OR_FCM_CST \
		| OR_FCM_CHT \
		| OR_FCM_SCY_1 \
		| OR_FCM_TRLX \
		| OR_FCM_EHTR)

#define CONFIG_SYS_BR0_PRELIM  CONFIG_FLASH_BR_PRELIM	/* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_FLASH_OR_PRELIM	/* NOR Options */
#define CONFIG_SYS_BR2_PRELIM  CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR2_PRELIM  CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */

#define CONFIG_SYS_BR4_PRELIM \
		(BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS + 0x40000) \
		| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
		| BR_PS_8		/* Port Size = 8 bit */ \
		| BR_MS_FCM		/* MSEL = FCM */ \
		| BR_V)			/* valid */
#define CONFIG_SYS_OR4_PRELIM	CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */
#define CONFIG_SYS_BR5_PRELIM \
		(BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS + 0x80000) \
		| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
		| BR_PS_8		/* Port Size = 8 bit */ \
		| BR_MS_FCM		/* MSEL = FCM */ \
		| BR_V)			/* valid */
#define CONFIG_SYS_OR5_PRELIM	CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */

#define CONFIG_SYS_BR6_PRELIM \
		(BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS + 0xc0000) \
		| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
		| BR_PS_8		/* Port Size = 8 bit */ \
		| BR_MS_FCM		/* MSEL = FCM */ \
		| BR_V)			/* valid */
#define CONFIG_SYS_OR6_PRELIM	CONFIG_SYS_NAND_OR_PRELIM	/* NAND Options */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#ifdef CONFIG_NAND_SPL
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR + 0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x29} }

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_BUS_NUM	1

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_MEM_BUS		0xf0000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCI1_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_VIRT		0xffc00000
#define CONFIG_SYS_PCI1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_IO_PHYS		0xfffc00000ull
#else
#define CONFIG_SYS_PCI1_IO_PHYS		0xffc00000
#endif
#define CONFIG_SYS_PCI1_IO_SIZE		0x00010000	/* 64k */

/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"Slot 1"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x90000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xf8000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc10000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x90000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x90000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x08000000	/* 128M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xfffc10000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc10000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_NAME		"Slot 2"
#define CONFIG_SYS_PCIE2_MEM_VIRT	0x98000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xf8000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc18000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0x98000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0x98000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x08000000	/* 128M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc20000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xfffc20000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc20000
#endif
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 3, direct to uli, tgtid 3, Base address 8000 */
#define CONFIG_SYS_PCIE3_NAME		"Slot 3"
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE3_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xffc30000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_IO_PHYS	0xfffc30000ull
#else
#define CONFIG_SYS_PCIE3_IO_PHYS	0xffc30000
#endif
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

#if defined(CONFIG_PCI)
/*PCIE video card used*/
#define VIDEO_IO_OFFSET		CONFIG_SYS_PCIE3_IO_VIRT

/*PCI video card used*/
/*#define VIDEO_IO_OFFSET	CONFIG_SYS_PCI1_IO_VIRT*/

/* video */

#if defined(CONFIG_VIDEO)
#define CONFIG_BIOSEMU
#define CONFIG_ATI_RADEON_FB
#define CONFIG_VIDEO_LOGO
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS CONFIG_SYS_PCIE3_IO_VIRT
#endif

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#ifndef CONFIG_PCI_PNP
	#define PCI_ENET0_IOADDR	CONFIG_SYS_PCI1_IO_BUS
	#define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI1_IO_BUS
	#define PCI_IDSEL_NUMBER	0x11	/* IDSEL = AD11 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */

/* SATA */
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#ifdef CONFIG_FSL_SATA
#define CONFIG_LBA48
#endif

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define CONFIG_FSL_SGMII_RISER	1
#define SGMII_RISER_PHY_OFFSET	0x1c

#define TSEC1_PHY_ADDR		1	/* TSEC1 -> PHY1 */
#define TSEC3_PHY_ADDR		0	/* TSEC3 -> PHY0 */

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC3_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */

#if defined(CONFIG_SYS_RAMBOOT)
#if defined(CONFIG_RAMBOOT_SPIFLASH)
#define CONFIG_ENV_SIZE		0x2000	/* 8KB */
#define CONFIG_ENV_OFFSET	0xF0000
#define CONFIG_ENV_SECT_SIZE	0x10000
#elif defined(CONFIG_RAMBOOT_SDCARD)
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_SYS_MMC_ENV_DEV  0
#else
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif
#else
	#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
	#define CONFIG_ENV_SIZE		0x2000
	#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#endif

/*
 * USB
 */
#define CONFIG_HAS_FSL_MPH_USB
#ifdef CONFIG_HAS_FSL_MPH_USB
#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#endif
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20) /* Initial Memory map for Linux */
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
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#endif

#define CONFIG_IPADDR		192.168.1.254

#define CONFIG_HOSTNAME		"unknown"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin /* U-Boot image on TFTP server */

#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

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
"ramdiskfile=8536ds/ramdisk.uboot\0"		\
"fdtaddr=1e00000\0"				\
"fdtfile=8536ds/mpc8536ds.dtb\0"		\
"bdev=sda3\0"					\
"hwconfig=usb1:dr_mode=host,phy_type=ulpi\0"

#define CONFIG_HDBOOT				\
 "setenv bootargs root=/dev/$bdev rw "		\
 "console=$consoledev,$baudrate $othbootargs;"	\
 "tftp $loadaddr $bootfile;"			\
 "tftp $fdtaddr $fdtfile;"			\
 "bootm $loadaddr - $fdtaddr"

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

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#endif	/* __CONFIG_H */
