/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

/*
 * P010 RDB board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/config_mpc85xx.h>
#define CONFIG_NAND_FSL_IFC

#ifdef CONFIG_SDCARD
#define CONFIG_SPL_FLUSH_IMAGE
#define CONFIG_SPL_TARGET		"u-boot-with-spl.bin"
#define CONFIG_SPL_PAD_TO		0x18000
#define CONFIG_SPL_MAX_SIZE		(96 * 1024)
#define CONFIG_SYS_MMC_U_BOOT_SIZE	(512 << 10)
#define CONFIG_SYS_MMC_U_BOOT_DST	(0x11000000)
#define CONFIG_SYS_MMC_U_BOOT_START	(0x11000000)
#define CONFIG_SYS_MMC_U_BOOT_OFFS	(96 << 10)
#define CONFIG_SYS_MPC85XX_NO_RESETVEC
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_COMMON_INIT_DDR
#endif
#endif

#ifdef CONFIG_SPIFLASH
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_RAMBOOT_SPIFLASH
#define CONFIG_RESET_VECTOR_ADDRESS	0x110bfffc
#else
#define CONFIG_SPL_SPI_FLASH_MINIMAL
#define CONFIG_SPL_FLUSH_IMAGE
#define CONFIG_SPL_TARGET		"u-boot-with-spl.bin"
#define CONFIG_SPL_PAD_TO			0x18000
#define CONFIG_SPL_MAX_SIZE			(96 * 1024)
#define CONFIG_SYS_SPI_FLASH_U_BOOT_SIZE	(512 << 10)
#define CONFIG_SYS_SPI_FLASH_U_BOOT_DST		(0x11000000)
#define CONFIG_SYS_SPI_FLASH_U_BOOT_START	(0x11000000)
#define CONFIG_SYS_SPI_FLASH_U_BOOT_OFFS	(96 << 10)
#define CONFIG_SYS_MPC85XX_NO_RESETVEC
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_COMMON_INIT_DDR
#endif
#endif
#endif

#ifdef CONFIG_NAND
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_SPL_INIT_MINIMAL
#define CONFIG_SPL_FLUSH_IMAGE
#define CONFIG_SPL_TARGET		"u-boot-with-spl.bin"

#define CONFIG_SPL_MAX_SIZE		8192
#define CONFIG_SPL_RELOC_TEXT_BASE	0x00100000
#define CONFIG_SPL_RELOC_STACK		0x00100000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	((768 << 10) - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_DST	(0x00200000 - CONFIG_SPL_MAX_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_START	0x00200000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0
#else
#ifdef CONFIG_TPL_BUILD
#define CONFIG_SPL_FLUSH_IMAGE
#define CONFIG_SPL_NAND_INIT
#define CONFIG_SPL_COMMON_INIT_DDR
#define CONFIG_SPL_MAX_SIZE		(128 << 10)
#define CONFIG_TPL_TEXT_BASE		0xD0001000
#define CONFIG_SYS_MPC85XX_NO_RESETVEC
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(576 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST	(0x11000000)
#define CONFIG_SYS_NAND_U_BOOT_START	(0x11000000)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	((128 + 128) << 10)
#elif defined(CONFIG_SPL_BUILD)
#define CONFIG_SPL_INIT_MINIMAL
#define CONFIG_SPL_NAND_MINIMAL
#define CONFIG_SPL_FLUSH_IMAGE
#define CONFIG_SPL_MAX_SIZE		8192
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(128 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST	0xD0000000
#define CONFIG_SYS_NAND_U_BOOT_START	0xD0000000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(128 << 10)
#endif
#define CONFIG_SPL_PAD_TO	0x20000
#define CONFIG_TPL_PAD_TO	0x20000
#define CONFIG_SPL_TARGET	"u-boot-with-spl.bin"
#endif
#endif

#ifdef CONFIG_NAND_SECBOOT	/* NAND Boot */
#define CONFIG_RAMBOOT_NAND
#define CONFIG_RESET_VECTOR_ADDRESS	0x110bfffc
#endif

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#ifdef CONFIG_TPL_BUILD
#define CONFIG_SYS_MONITOR_BASE	CONFIG_TPL_TEXT_BASE
#elif defined(CONFIG_SPL_BUILD)
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

/* High Level Configuration Options */
#define CONFIG_SYS_HAS_SERDES		/* common SERDES init code */

#if defined(CONFIG_PCI)
#define CONFIG_PCIE1			/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2			/* PCIE controller 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

/*
 * PCI Windows
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"mini PCIe Slot"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xfffc00000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc00000
#endif

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#if defined(CONFIG_TARGET_P1010RDB_PA)
#define CONFIG_SYS_PCIE2_NAME		"PCIe Slot"
#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define CONFIG_SYS_PCIE2_NAME		"mini PCIe Slot"
#endif
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xfffc10000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc10000
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#endif

#define CONFIG_ENV_OVERWRITE

#define CONFIG_DDR_CLK_FREQ	66666666 /* DDRCLK on P1010 RDB */
#define CONFIG_SYS_CLK_FREQ	66666666 /* SYSCLK for P1010 RDB */

#define CONFIG_HWCONFIG
/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */


#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP			1
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff

/* DDR Setup */
#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_DDR_SPD
#define CONFIG_SYS_SPD_BUS_NUM		1
#define SPD_EEPROM_ADDRESS		0x52

#define CONFIG_MEM_INIT_VALUE		0xDeadBeef

#ifndef __ASSEMBLY__
extern unsigned long get_sdram_size(void);
#endif
#define CONFIG_SYS_SDRAM_SIZE		get_sdram_size() /* DDR size */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1

/* DDR3 Controller Settings */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000003f
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014302
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000
#define CONFIG_SYS_DDR_ZQ_CONTROL	0x89080600
#define CONFIG_SYS_DDR_SR_CNTR		0x00000000
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#define CONFIG_SYS_DDR_CONTROL		0xc70c0008      /* Type = DDR3  */
#define CONFIG_SYS_DDR_CONTROL_2	0x24401000
#define CONFIG_SYS_DDR_TIMING_4		0x00000001
#define CONFIG_SYS_DDR_TIMING_5		0x03402400

#define CONFIG_SYS_DDR_TIMING_3_800	0x00030000
#define CONFIG_SYS_DDR_TIMING_0_800	0x00110104
#define CONFIG_SYS_DDR_TIMING_1_800	0x6f6b8644
#define CONFIG_SYS_DDR_TIMING_2_800	0x0FA888CF
#define CONFIG_SYS_DDR_CLK_CTRL_800	0x03000000
#define CONFIG_SYS_DDR_MODE_1_800	0x00441420
#define CONFIG_SYS_DDR_MODE_2_800	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_800	0x0C300100
#define CONFIG_SYS_DDR_WRLVL_CONTROL_800 0x8675f608

/* settings for DDR3 at 667MT/s */
#define CONFIG_SYS_DDR_TIMING_3_667		0x00010000
#define CONFIG_SYS_DDR_TIMING_0_667		0x00110004
#define CONFIG_SYS_DDR_TIMING_1_667		0x5d59e544
#define CONFIG_SYS_DDR_TIMING_2_667		0x0FA890CD
#define CONFIG_SYS_DDR_CLK_CTRL_667		0x03000000
#define CONFIG_SYS_DDR_MODE_1_667		0x00441210
#define CONFIG_SYS_DDR_MODE_2_667		0x00000000
#define CONFIG_SYS_DDR_INTERVAL_667		0x0a280000
#define CONFIG_SYS_DDR_WRLVL_CONTROL_667	0x8675F608

#define CONFIG_SYS_CCSRBAR			0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW		CONFIG_SYS_CCSRBAR

/* Don't relocate CCSRBAR while in NAND_SPL */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_CCSR_DO_NOT_RELOCATE
#endif

/*
 * Memory map
 *
 * 0x0000_0000	0x3fff_ffff	DDR			1G cacheable
 * 0x8000_0000  0xbfff_ffff	PCI Express Mem		1.5G non-cacheable
 * 0xffc0_0000  0xffc3_ffff	PCI IO range		256k non-cacheable
 *
 * Localbus non-cacheable
 * 0xff80_0000	0xff8f_ffff	NAND Flash		1M non-cacheable
 * 0xffb0_0000	0xffbf_ffff	Board CPLD		1M non-cacheable
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/*
 * IFC Definitions
 */
/* NOR Flash on IFC */

#define CONFIG_SYS_FLASH_BASE		0xee000000
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* 32M */

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CONFIG_SYS_FLASH_BASE)
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_SYS_NOR_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CONFIG_SYS_NOR_AMASK	IFC_AMASK(32*1024*1024)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(7)
/* NOR Flash Timing Params */
#define CONFIG_SYS_NOR_FTIM0	FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5)
#define CONFIG_SYS_NOR_FTIM1	FTIM1_NOR_TACO(0x1e) | \
				FTIM1_NOR_TRAD_NOR(0x0f)
#define CONFIG_SYS_NOR_FTIM2	FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWP(0x1c)
#define CONFIG_SYS_NOR_FTIM3	0x0

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

/* CFI for NOR Flash */
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* NAND Flash on IFC */
#define CONFIG_SYS_NAND_BASE		0xff800000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_NAND_BASE_PHYS	0xfff800000ull
#else
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#endif

#define CONFIG_MTD_PARTITION

#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8	\
				| CSPR_MSEL_NAND	\
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64*1024)

#if defined(CONFIG_TARGET_P1010RDB_PA)
#define CONFIG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN	/* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_2	/* RAL = 2 Bytes */ \
				| CSOR_NAND_PGS_512	/* Page Size = 512b */ \
				| CSOR_NAND_SPRZ_16	/* Spare size = 16 */ \
				| CSOR_NAND_PB(32))	/* 32 Pages Per Block */
#define CONFIG_SYS_NAND_BLOCK_SIZE	(16 * 1024)

#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_CSOR   (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3       /* RAL = 3Byes */ \
				| CSOR_NAND_PGS_4K      /* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224    /* Spare size = 224 */ \
				| CSOR_NAND_PB(128))  /*Pages Per Block = 128 */
#define CONFIG_SYS_NAND_BLOCK_SIZE     (512 * 1024)
#endif

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#if defined(CONFIG_TARGET_P1010RDB_PA)
/* NAND Flash Timing Params */
#define CONFIG_SYS_NAND_FTIM0		FTIM0_NAND_TCCST(0x01) | \
					FTIM0_NAND_TWP(0x0C)   | \
					FTIM0_NAND_TWCHT(0x04) | \
					FTIM0_NAND_TWH(0x05)
#define CONFIG_SYS_NAND_FTIM1		FTIM1_NAND_TADLE(0x1d) | \
					FTIM1_NAND_TWBE(0x1d)  | \
					FTIM1_NAND_TRR(0x07)   | \
					FTIM1_NAND_TRP(0x0c)
#define CONFIG_SYS_NAND_FTIM2		FTIM2_NAND_TRAD(0x0c) | \
					FTIM2_NAND_TREH(0x05) | \
					FTIM2_NAND_TWHRE(0x0f)
#define CONFIG_SYS_NAND_FTIM3		FTIM3_NAND_TWW(0x04)

#elif defined(CONFIG_TARGET_P1010RDB_PB)
/* support MT29F16G08ABABAWP 4k-pagesize 2G-bytes NAND */
/* ONFI NAND Flash mode0 Timing Params */
#define CONFIG_SYS_NAND_FTIM0  (FTIM0_NAND_TCCST(0x07)| \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CONFIG_SYS_NAND_FTIM1  (FTIM1_NAND_TADLE(0x32)| \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CONFIG_SYS_NAND_FTIM2  (FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a)  | \
					FTIM2_NAND_TWHRE(0x1e))
#define CONFIG_SYS_NAND_FTIM3	0x0
#endif

#define CONFIG_SYS_NAND_DDR_LAW		11

/* Set up IFC registers for boot location NOR/NAND */
#if defined(CONFIG_NAND) || defined(CONFIG_NAND_SECBOOT)
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NOR_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NOR_FTIM3
#else
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NAND_FTIM3
#endif

/* CPLD on IFC */
#define CONFIG_SYS_CPLD_BASE		0xffb00000

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_CPLD_BASE_PHYS	0xfffb00000ull
#else
#define CONFIG_SYS_CPLD_BASE_PHYS	CONFIG_SYS_CPLD_BASE
#endif

#define CONFIG_SYS_CSPR3	(CSPR_PHYS_ADDR(CONFIG_SYS_CPLD_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CONFIG_SYS_AMASK3		IFC_AMASK(64*1024)
#define CONFIG_SYS_CSOR3		0x0
/* CPLD Timing parameters for IFC CS3 */
#define CONFIG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CONFIG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0x0e) | \
					FTIM1_GPCM_TRAD(0x1f))
#define CONFIG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0x0e) | \
					FTIM2_GPCM_TCH(0x8) | \
					FTIM2_GPCM_TWP(0x1f))
#define CONFIG_SYS_CS3_FTIM3		0x0

#if defined(CONFIG_RAMBOOT_SDCARD) || defined(CONFIG_RAMBOOT_SPIFLASH) || \
	defined(CONFIG_RAMBOOT_NAND)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_IFC_A003399
#if !defined(CONFIG_SPL) && !defined(CONFIG_SYS_RAMBOOT)
#define CONFIG_A003399_NOR_WORKAROUND
#endif
#endif

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000 /* stack in RAM */
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000 /* End of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
						- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc*/

/*
 * Config the L2 Cache as L2 SRAM
 */
#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
#define CONFIG_SYS_INIT_L2_ADDR		0xD0000000
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#define CONFIG_SYS_L2_SIZE		(256 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#define CONFIG_SPL_RELOC_TEXT_BASE	0xD0001000
#define CONFIG_SPL_RELOC_STACK		(CONFIG_SYS_INIT_L2_ADDR + 112 * 1024)
#define CONFIG_SPL_RELOC_MALLOC_ADDR	(CONFIG_SYS_INIT_L2_ADDR + 128 * 1024)
#define CONFIG_SPL_RELOC_MALLOC_SIZE	(128 << 10)
#define CONFIG_SPL_GD_ADDR		(CONFIG_SYS_INIT_L2_ADDR + 96 * 1024)
#elif defined(CONFIG_NAND)
#ifdef CONFIG_TPL_BUILD
#define CONFIG_SYS_INIT_L2_ADDR		0xD0000000
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#define CONFIG_SYS_L2_SIZE		(256 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#define CONFIG_SPL_RELOC_TEXT_BASE	0xD0001000
#define CONFIG_SPL_RELOC_STACK		(CONFIG_SYS_INIT_L2_ADDR + 192 * 1024)
#define CONFIG_SPL_RELOC_MALLOC_ADDR	(CONFIG_SYS_INIT_L2_ADDR + 208 * 1024)
#define CONFIG_SPL_RELOC_MALLOC_SIZE	(48 << 10)
#define CONFIG_SPL_GD_ADDR		(CONFIG_SYS_INIT_L2_ADDR + 176 * 1024)
#else
#define CONFIG_SYS_INIT_L2_ADDR		0xD0000000
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#define CONFIG_SYS_L2_SIZE		(256 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#define CONFIG_SPL_RELOC_TEXT_BASE	(CONFIG_SYS_INIT_L2_END - 0x3000)
#define CONFIG_SPL_RELOC_STACK		((CONFIG_SYS_INIT_L2_END - 1) & ~0xF)
#endif
#endif
#endif

/* Serial Port */
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_INIT_MINIMAL)
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define I2C_PCA9557_ADDR1		0x18
#define I2C_PCA9557_ADDR2		0x19
#define I2C_PCA9557_BUS_NUM		0

/* I2C EEPROM */
#if defined(CONFIG_TARGET_P1010RDB_PB)
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define MAX_NUM_PORTS			9 /* for 128Bytes EEPROM */
#endif
/* enable read and write access to EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

/* RTC */
#define CONFIG_RTC_PT7C4338
#define CONFIG_SYS_I2C_RTC_ADDR	0x68

/*
 * SPI interface will not be available in case of NAND boot SPI CS0 will be
 * used for SLIC
 */
#if !defined(CONFIG_NAND) || !defined(CONFIG_NAND_SECBOOT)
/* eSPI - Enhanced SPI */
#endif

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define TSEC1_PHY_ADDR		1
#define TSEC2_PHY_ADDR		0
#define TSEC3_PHY_ADDR		2

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

/* TBI PHY configuration for SGMII mode */
#define CONFIG_TSEC_TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)

#endif	/* CONFIG_TSEC_ENET */

/* SATA */
#define CONFIG_FSL_SATA_V2

#ifdef CONFIG_FSL_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#define CONFIG_LBA48
#endif /* #ifdef CONFIG_FSL_SATA  */

#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#endif

#define CONFIG_HAS_FSL_DR_USB

#if defined(CONFIG_HAS_FSL_DR_USB)
#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#endif
#endif

/*
 * Environment
 */
#if defined(CONFIG_SDCARD)
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x2000
#elif defined(CONFIG_SPIFLASH)
#define CONFIG_ENV_OFFSET	0x100000	/* 1MB */
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_SIZE		0x2000
#elif defined(CONFIG_NAND)
#ifdef CONFIG_TPL_BUILD
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_INIT_L2_ADDR + (160 << 10))
#else
#if defined(CONFIG_TARGET_P1010RDB_PA)
#define CONFIG_ENV_SIZE		CONFIG_SYS_NAND_BLOCK_SIZE
#define CONFIG_ENV_RANGE	(3 * CONFIG_ENV_SIZE) /* 3*16=48K for env */
#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define CONFIG_ENV_SIZE		(16 * 1024)
#define CONFIG_ENV_RANGE	(32 * CONFIG_ENV_SIZE) /* new block size 512K */
#endif
#endif
#define CONFIG_ENV_OFFSET	(1024 * 1024)
#elif defined(CONFIG_SYS_RAMBOOT)
#define CONFIG_ENV_ADDR			(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE			0x2000
#else
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI_HCD) \
		 || defined(CONFIG_FSL_SATA)
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
#define CONFIG_SYS_BOOTM_LEN	(64 << 20) /* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"hwconfig=" __stringify(CONFIG_DEF_HWCONFIG)  "\0"	\
	"netdev=eth0\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"		\
	"loadaddr=1000000\0"			\
	"consoledev=ttyS0\0"				\
	"ramdiskaddr=2000000\0"			\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"		\
	"fdtaddr=1e00000\0"				\
	"fdtfile=p1010rdb.dtb\0"		\
	"bdev=sda1\0"	\
	"hwconfig=usb1:dr_mode=host,phy_type=utmi\0"	\
	"othbootargs=ramdisk_size=600000\0" \
	"usbfatboot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"fatload usb 0:2 $loadaddr $bootfile;"		\
	"fatload usb 0:2 $fdtaddr $fdtfile;"	\
	"fatload usb 0:2 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"usbext2boot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"ext2load usb 0:4 $loadaddr $bootfile;"		\
	"ext2load usb 0:4 $fdtaddr $fdtfile;"	\
	"ext2load usb 0:4 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"	\
	CONFIG_BOOTMODE

#if defined(CONFIG_TARGET_P1010RDB_PA)
#define CONFIG_BOOTMODE \
	"boot_bank0=i2c dev 0; i2c mw 18 1 f1; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00009 0; reset\0" \
	"boot_bank1=i2c dev 0; i2c mw 18 1 f1; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00009 1; reset\0" \
	"boot_nand=i2c dev 0; i2c mw 18 1 f9; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00017 1; reset\0"

#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define CONFIG_BOOTMODE \
	"boot_bank0=i2c dev 0; i2c mw 18 1 fe; i2c mw 18 3 0;" \
	"i2c mw 19 1 2; i2c mw 19 3 e1; reset\0" \
	"boot_bank1=i2c dev 0; i2c mw 18 1 fe; i2c mw 18 3 0;" \
	"i2c mw 19 1 12; i2c mw 19 3 e1; reset\0" \
	"boot_nand=i2c dev 0; i2c mw 18 1 fc; i2c mw 18 3 0;" \
	"i2c mw 19 1 8; i2c mw 19 3 f7; reset\0" \
	"boot_spi=i2c dev 0; i2c mw 18 1 fa; i2c mw 18 3 0;" \
	"i2c mw 19 1 0; i2c mw 19 3 f7; reset\0" \
	"boot_sd=i2c dev 0; i2c mw 18 1 f8; i2c mw 18 3 0;" \
	"i2c mw 19 1 4; i2c mw 19 3 f3; reset\0"
#endif

#define CONFIG_RAMBOOTCOMMAND		\
	"setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"tftp $ramdiskaddr $ramdiskfile;"	\
	"tftp $loadaddr $bootfile;"		\
	"tftp $fdtaddr $fdtfile;"		\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND CONFIG_RAMBOOTCOMMAND

#include <asm/fsl_secure_boot.h>

#endif	/* __CONFIG_H */
