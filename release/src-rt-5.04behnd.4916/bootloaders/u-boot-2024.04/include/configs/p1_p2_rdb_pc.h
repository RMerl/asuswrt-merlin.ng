/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 */

/*
 * QorIQ RDB boards configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

#if defined(CONFIG_TARGET_P1020RDB_PC)
#define CFG_SLIC
#define __SW_BOOT_MASK		0x03
#define __SW_BOOT_NOR		0x5c
#define __SW_BOOT_SPI		0x1c
#define __SW_BOOT_SD		0x9c
#define __SW_BOOT_NAND		0xec
#define __SW_BOOT_PCIE		0x6c
#define __SW_NOR_BANK_MASK	0xfd
#define __SW_NOR_BANK_UP	0x00
#define __SW_NOR_BANK_LO	0x02
#define __SW_BOOT_NOR_BANK_UP	0x5c /* (__SW_BOOT_NOR | __SW_NOR_BANK_UP) */
#define __SW_BOOT_NOR_BANK_LO	0x5e /* (__SW_BOOT_NOR | __SW_NOR_BANK_LO) */
#define __SW_BOOT_NOR_BANK_MASK	0x01 /* (__SW_BOOT_MASK & __SW_NOR_BANK_MASK) */
#endif

/*
 * P1020RDB-PD board has user selectable switches for evaluating different
 * frequency and boot options for the P1020 device. The table that
 * follow describe the available options. The front six binary number was in
 * accordance with SW3[1:6].
 * 111101 533 533 267 667 NOR Core0 boot; Core1 hold-off
 * 101101 667 667 333 667 NOR Core0 boot; Core1 hold-off
 * 011001 800 800 400 667 NOR Core0 boot; Core1 hold-off
 * 001001 800 800 400 667 SD/MMC Core0 boot; Core1 hold-off
 * 001101 800 800 400 667 SPI Core0 boot; Core1 hold-off
 * 010001 800 800 400 667 NAND Core0 boot; Core1 hold-off
 * 011101 800 800 400 667 PCIe-2 Core0 boot; Core1 hold-off
 */
#if defined(CONFIG_TARGET_P1020RDB_PD)
#define CFG_SLIC
#define __SW_BOOT_MASK		0x03
#define __SW_BOOT_NOR		0x64
#define __SW_BOOT_SPI		0x34
#define __SW_BOOT_SD		0x24
#define __SW_BOOT_NAND		0x44
#define __SW_BOOT_PCIE		0x74
#define __SW_NOR_BANK_MASK	0xfd
#define __SW_NOR_BANK_UP	0x00
#define __SW_NOR_BANK_LO	0x02
#define __SW_BOOT_NOR_BANK_UP	0x64 /* (__SW_BOOT_NOR | __SW_NOR_BANK_UP) */
#define __SW_BOOT_NOR_BANK_LO	0x66 /* (__SW_BOOT_NOR | __SW_NOR_BANK_LO) */
#define __SW_BOOT_NOR_BANK_MASK	0x01 /* (__SW_BOOT_MASK & __SW_NOR_BANK_MASK) */
/*
 * Dynamic MTD Partition support with mtdparts
 */
#endif

#if defined(CONFIG_TARGET_P2020RDB)
#define __SW_BOOT_MASK		0x03
#define __SW_BOOT_NOR		0xc8
#define __SW_BOOT_SPI		0x28
#define __SW_BOOT_SD		0x68
#define __SW_BOOT_SD2		0x18
#define __SW_BOOT_NAND		0xe8
#define __SW_BOOT_PCIE		0xa8
#define __SW_NOR_BANK_MASK	0xfd
#define __SW_NOR_BANK_UP	0x00
#define __SW_NOR_BANK_LO	0x02
#define __SW_BOOT_NOR_BANK_UP	0xc8 /* (__SW_BOOT_NOR | __SW_NOR_BANK_UP) */
#define __SW_BOOT_NOR_BANK_LO	0xca /* (__SW_BOOT_NOR | __SW_NOR_BANK_LO) */
#define __SW_BOOT_NOR_BANK_MASK	0x01 /* (__SW_BOOT_MASK & __SW_NOR_BANK_MASK) */
/*
 * Dynamic MTD Partition support with mtdparts
 */
#endif

#ifdef CONFIG_SDCARD
#define CFG_SYS_MMC_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_MMC_U_BOOT_DST	CONFIG_TEXT_BASE
#define CFG_SYS_MMC_U_BOOT_START	CONFIG_TEXT_BASE
#ifdef CONFIG_FSL_PREPBL_ESDHC_BOOT_SECTOR
#define CFG_SYS_MMC_U_BOOT_OFFS	(CONFIG_SPL_PAD_TO - CONFIG_FSL_PREPBL_ESDHC_BOOT_SECTOR_DATA*512)
#else
#define CFG_SYS_MMC_U_BOOT_OFFS	CONFIG_SPL_PAD_TO
#endif
#elif defined(CONFIG_SPIFLASH)
#define CFG_SYS_SPI_FLASH_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_SPI_FLASH_U_BOOT_DST		CONFIG_TEXT_BASE
#define CFG_SYS_SPI_FLASH_U_BOOT_START	CONFIG_TEXT_BASE
#define CFG_SYS_SPI_FLASH_U_BOOT_OFFS	CONFIG_SPL_PAD_TO
#elif defined(CONFIG_MTD_RAW_NAND)
#ifdef CONFIG_TPL_BUILD
#define CFG_SYS_NAND_U_BOOT_SIZE	(832 << 10)
#define CFG_SYS_NAND_U_BOOT_DST	(0x11000000)
#define CFG_SYS_NAND_U_BOOT_START	(0x11000000)
#elif defined(CONFIG_SPL_BUILD)
#define CFG_SYS_NAND_U_BOOT_SIZE	(128 << 10)
#define CFG_SYS_NAND_U_BOOT_DST	0xf8f80000
#define CFG_SYS_NAND_U_BOOT_START	0xf8f80000
#endif /* not CONFIG_TPL_BUILD */
#endif

#ifndef CFG_RESET_VECTOR_ADDRESS
#define CFG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#define CFG_SYS_CCSRBAR		0xffe00000
#define CFG_SYS_CCSRBAR_PHYS_LOW	CFG_SYS_CCSRBAR

/* DDR Setup */
#define SPD_EEPROM_ADDRESS 0x52

#if defined(CONFIG_TARGET_P1020RDB_PD)
#define CFG_SYS_SDRAM_SIZE_LAW	LAW_SIZE_2G
#else
#define CFG_SYS_SDRAM_SIZE_LAW	LAW_SIZE_1G
#endif
#define CFG_SYS_SDRAM_SIZE		(1u << (CFG_SYS_SDRAM_SIZE_LAW - 19))
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

/* Default settings for DDR3 */
#ifndef CONFIG_TARGET_P2020RDB
#define CFG_SYS_DDR_CS0_BNDS		0x0000003f
#define CFG_SYS_DDR_CS0_CONFIG	0x80014302
#define CFG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CFG_SYS_DDR_CS1_BNDS		0x0040007f
#define CFG_SYS_DDR_CS1_CONFIG	0x80014302
#define CFG_SYS_DDR_CS1_CONFIG_2	0x00000000

#define CFG_SYS_DDR_INIT_ADDR	0x00000000
#define CFG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CFG_SYS_DDR_MODE_CONTROL	0x00000000

#define CFG_SYS_DDR_ZQ_CONTROL	0x89080600
#define CFG_SYS_DDR_WRLVL_CONTROL	0x8655A608
#define CFG_SYS_DDR_SR_CNTR		0x00000000
#define CFG_SYS_DDR_RCW_1		0x00000000
#define CFG_SYS_DDR_RCW_2		0x00000000
#define CFG_SYS_DDR_CONTROL		0xC70C0000	/* Type = DDR3	*/
#define CFG_SYS_DDR_CONTROL_2	0x04401050
#define CFG_SYS_DDR_TIMING_4		0x00220001
#define CFG_SYS_DDR_TIMING_5		0x03402400

#define CFG_SYS_DDR_TIMING_3		0x00020000
#define CFG_SYS_DDR_TIMING_0		0x00330004
#define CFG_SYS_DDR_TIMING_1		0x6f6B4846
#define CFG_SYS_DDR_TIMING_2		0x0FA8C8CF
#define CFG_SYS_DDR_CLK_CTRL		0x03000000
#define CFG_SYS_DDR_MODE_1		0x40461520
#define CFG_SYS_DDR_MODE_2		0x8000c000
#define CFG_SYS_DDR_INTERVAL		0x0C300000
#endif

/*
 * Memory map
 *
 * 0x0000_0000 0x7fff_ffff	DDR		Up to 2GB cacheable
 * 0x8000_0000 0xdfff_ffff	PCI Express Mem	1.5G non-cacheable(PCIe * 3)
 * 0xec00_0000 0xefff_ffff	NOR flash	Up to 64M non-cacheable	CS0/1
 * 0xf8f8_0000 0xf8ff_ffff	L2 SRAM		Up to 512K cacheable
 *   (early boot only)
 * 0xff80_0000 0xff80_7fff	NAND flash	32K non-cacheable	CS1/0
 * 0xffa0_0000 0xffaf_ffff	CPLD		1M non-cacheable	CS3
 * 0xffb0_0000 0xffbf_ffff	VSC7385 switch  1M non-cacheable	CS2
 * 0xffc0_0000 0xffc3_ffff	PCI IO range	256k non-cacheable
 * 0xffd0_0000 0xffd0_3fff	L1 for stack	16K cacheable
 * 0xffe0_0000 0xffef_ffff	CCSR		1M non-cacheable
 */

/*
 * Local Bus Definitions
 */
#if defined(CONFIG_TARGET_P1020RDB_PD)
#define CFG_SYS_FLASH_BASE		0xec000000
#else
#define CFG_SYS_FLASH_BASE		0xef000000
#endif

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CFG_SYS_FLASH_BASE)
#else
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE
#endif

#define CFG_FLASH_BR_PRELIM (BR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) \
	| BR_PS_16 | BR_V)

#define CFG_FLASH_OR_PRELIM	0xfc000ff7

#define CFG_SYS_FLASH_BANKS_LIST	{CFG_SYS_FLASH_BASE_PHYS}

/* Nand Flash */
#ifdef CONFIG_NAND_FSL_ELBC
#define CFG_SYS_NAND_BASE		0xff800000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_NAND_BASE_PHYS	0xfff800000ull
#else
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE
#endif

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

#define CFG_SYS_NAND_BR_PRELIM (BR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
	| (2<<BR_DECC_SHIFT)	/* Use HW ECC */ \
	| BR_PS_8	/* Port Size = 8 bit */ \
	| BR_MS_FCM	/* MSEL = FCM */ \
	| BR_V)	/* valid */
#if defined(CONFIG_TARGET_P1020RDB_PD)
#define CFG_SYS_NAND_OR_PRELIM	(OR_AM_32KB \
	| OR_FCM_PGS	/* Large Page*/ \
	| OR_FCM_CSCT \
	| OR_FCM_CST \
	| OR_FCM_CHT \
	| OR_FCM_SCY_1 \
	| OR_FCM_TRLX \
	| OR_FCM_EHTR)
#else
#define CFG_SYS_NAND_OR_PRELIM	(OR_AM_32KB	/* small page */ \
	| OR_FCM_CSCT \
	| OR_FCM_CST \
	| OR_FCM_CHT \
	| OR_FCM_SCY_1 \
	| OR_FCM_TRLX \
	| OR_FCM_EHTR)
#endif
#endif /* CONFIG_NAND_FSL_ELBC */

#define CFG_SYS_INIT_RAM_ADDR	0xffd00000 /* stack in RAM */
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW CFG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CFG_SYS_INIT_RAM_ADDR_PHYS \
	((CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CFG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
/* Initial L1 address */
#define CFG_SYS_INIT_RAM_ADDR_PHYS	CFG_SYS_INIT_RAM_ADDR
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW CFG_SYS_INIT_RAM_ADDR_PHYS
#endif
/* Size of used area in RAM */
#define CFG_SYS_INIT_RAM_SIZE	0x00004000

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define CFG_SYS_CPLD_BASE	0xffa00000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_CPLD_BASE_PHYS	0xfffa00000ull
#else
#define CFG_SYS_CPLD_BASE_PHYS	CFG_SYS_CPLD_BASE
#endif
/* CPLD config size: 1Mb */

/* Vsc7385 switch */
#ifdef CONFIG_VSC7385_ENET
#define __VSCFW_ADDR			"vscfw_addr=ef000000\0"
#define CFG_SYS_VSC7385_BASE		0xffb00000

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_VSC7385_BASE_PHYS	0xfffb00000ull
#else
#define CFG_SYS_VSC7385_BASE_PHYS	CFG_SYS_VSC7385_BASE
#endif

/* The size of the VSC7385 firmware image */
#define CFG_VSC7385_IMAGE_SIZE	8192
#endif

#ifndef __VSCFW_ADDR
#define __VSCFW_ADDR ""
#endif

/*
 * Config the L2 Cache as L2 SRAM
*/
#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
#define CFG_SYS_INIT_L2_ADDR		0xf8f80000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#elif defined(CONFIG_MTD_RAW_NAND)
#ifdef CONFIG_TPL_BUILD
#define CFG_SYS_INIT_L2_ADDR		0xf8f80000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#else
#define CFG_SYS_INIT_L2_ADDR		0xf8f80000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#endif /* CONFIG_TPL_BUILD */
#endif
#endif

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CFG_SYS_NS16550_CLK		get_bus_freq(0)

#define CFG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CFG_SYS_NS16550_COM1	(CFG_SYS_CCSRBAR+0x4500)
#define CFG_SYS_NS16550_COM2	(CFG_SYS_CCSRBAR+0x4600)

/* I2C */
#if !CONFIG_IS_ENABLED(DM_I2C)
#define CFG_SYS_I2C_NOPROBES		{ {0, 0x29} }
#endif

/*
 * I2C2 EEPROM
 */

#define CFG_SYS_I2C_RTC_ADDR		0x68
#define CFG_SYS_I2C_PCA9557_ADDR	0x18

/* enable read and write access to EEPROM */

#if defined(CONFIG_PCI)
/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 2, direct to uli, tgtid 2, Base address 9000 */
#define CFG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CFG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CFG_SYS_PCIE2_IO_VIRT	0xffc10000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_PCIE2_IO_PHYS	0xfffc10000ull
#else
#define CFG_SYS_PCIE2_IO_PHYS	0xffc10000
#endif

/* controller 1, Slot 2, tgtid 1, Base address a000 */
#define CFG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CFG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CFG_SYS_PCIE1_IO_VIRT	0xffc00000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_PCIE1_IO_PHYS	0xfffc00000ull
#else
#define CFG_SYS_PCIE1_IO_PHYS	0xffc00000
#endif
#endif /* CONFIG_PCI */

/*
 * Environment
 */
#if defined(CONFIG_MTD_RAW_NAND)
#ifdef CONFIG_TPL_BUILD
#define SPL_ENV_ADDR		(CFG_SYS_INIT_L2_ADDR + (160 << 10))
#endif
#endif

/*
 * USB
 */

#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR	CFG_SYS_MPC85xx_ESDHC_ADDR
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory for Linux*/

/*
 * Environment Configuration
 */

#include "p1_p2_bootsrc.h"

#define	CFG_EXTRA_ENV_SETTINGS	\
"netdev=eth0\0"	\
"uboot=" CONFIG_UBOOTPATH "\0"	\
"loadaddr=1000000\0"	\
"bootfile=uImage\0"	\
"tftpflash=tftpboot $loadaddr $uboot; "	\
	"protect off " __stringify(CONFIG_TEXT_BASE) " +$filesize; " \
	"erase " __stringify(CONFIG_TEXT_BASE) " +$filesize; "	\
	"cp.b $loadaddr " __stringify(CONFIG_TEXT_BASE) " $filesize; " \
	"protect on " __stringify(CONFIG_TEXT_BASE) " +$filesize; "	\
	"cmp.b $loadaddr " __stringify(CONFIG_TEXT_BASE) " $filesize\0" \
"hwconfig=usb1:dr_mode=host,phy_type=ulpi\0"    \
"consoledev=ttyS0\0"	\
"ramdiskaddr=2000000\0"	\
"ramdiskfile=rootfs.ext2.gz.uboot\0"	\
"fdtaddr=1e00000\0"	\
"bdev=sda1\0" \
"jffs2nor=mtdblock3\0"	\
"norbootaddr=ef080000\0"	\
"norfdtaddr=ef040000\0"	\
"jffs2nand=mtdblock9\0"	\
"nandbootaddr=100000\0"	\
"nandfdtaddr=80000\0"		\
"ramdisk_size=120000\0"	\
__VSCFW_ADDR	\
MAP_NOR_LO_CMD(map_lowernorbank) \
MAP_NOR_UP_CMD(map_uppernorbank) \
RST_NOR_CMD(norboot) \
RST_NOR_LO_CMD(norlowerboot) \
RST_NOR_UP_CMD(norupperboot) \
RST_SPI_CMD(spiboot) \
RST_SD_CMD(sdboot) \
RST_SD2_CMD(sd2boot) \
RST_NAND_CMD(nandboot) \
RST_PCIE_CMD(pciboot) \
RST_DEF_CMD(defboot) \
""

#endif /* __CONFIG_H */
