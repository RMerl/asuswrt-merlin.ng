/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 */

/*
 * P010 RDB board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

#include <asm/config_mpc85xx.h>

#ifdef CONFIG_SDCARD
#define CFG_SYS_MMC_U_BOOT_SIZE	(512 << 10)
#define CFG_SYS_MMC_U_BOOT_DST	(0x11000000)
#define CFG_SYS_MMC_U_BOOT_START	(0x11000000)
#define CFG_SYS_MMC_U_BOOT_OFFS	(96 << 10)
#endif

#ifdef CONFIG_SPIFLASH
#ifdef CONFIG_NXP_ESBC
#define CFG_RESET_VECTOR_ADDRESS	0x110bfffc
#else
#define CFG_SYS_SPI_FLASH_U_BOOT_SIZE	(512 << 10)
#define CFG_SYS_SPI_FLASH_U_BOOT_DST		(0x11000000)
#define CFG_SYS_SPI_FLASH_U_BOOT_START	(0x11000000)
#define CFG_SYS_SPI_FLASH_U_BOOT_OFFS	(96 << 10)
#endif
#endif

#ifdef CONFIG_MTD_RAW_NAND
#ifdef CONFIG_NXP_ESBC
#define CFG_SYS_NAND_U_BOOT_SIZE	((768 << 10) - 0x2000)
#define CFG_SYS_NAND_U_BOOT_DST	(0x00200000 - CONFIG_SPL_MAX_SIZE)
#define CFG_SYS_NAND_U_BOOT_START	0x00200000
#else
#ifdef CONFIG_TPL_BUILD
#define CFG_SYS_NAND_U_BOOT_SIZE	(576 << 10)
#define CFG_SYS_NAND_U_BOOT_DST	(0x11000000)
#define CFG_SYS_NAND_U_BOOT_START	(0x11000000)
#elif defined(CONFIG_SPL_BUILD)
#define CFG_SYS_NAND_U_BOOT_SIZE	(128 << 10)
#define CFG_SYS_NAND_U_BOOT_DST	0xD0000000
#define CFG_SYS_NAND_U_BOOT_START	0xD0000000
#endif
#endif
#endif

#ifdef CONFIG_NAND_SECBOOT	/* NAND Boot */
#define CFG_RESET_VECTOR_ADDRESS	0x110bfffc
#endif

#ifndef CFG_RESET_VECTOR_ADDRESS
#define CFG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

/* High Level Configuration Options */

#if defined(CONFIG_PCI)
/*
 * PCI Windows
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
/* controller 1, Slot 1, tgtid 1, Base address a000 */
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

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
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
#endif

/* DDR Setup */
#define SPD_EEPROM_ADDRESS		0x52

#ifndef __ASSEMBLY__
extern unsigned long get_sdram_size(void);
#endif
#define CFG_SYS_SDRAM_SIZE		get_sdram_size() /* DDR size */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

#define CFG_SYS_CCSRBAR			0xffe00000
#define CFG_SYS_CCSRBAR_PHYS_LOW		CFG_SYS_CCSRBAR

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

#define CFG_SYS_FLASH_BASE		0xee000000

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CFG_SYS_FLASH_BASE)
#else
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE
#endif

#define CFG_SYS_NOR_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CFG_SYS_NOR_AMASK	IFC_AMASK(32*1024*1024)
#define CFG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(7)
/* NOR Flash Timing Params */
#define CFG_SYS_NOR_FTIM0	FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5)
#define CFG_SYS_NOR_FTIM1	FTIM1_NOR_TACO(0x1e) | \
				FTIM1_NOR_TRAD_NOR(0x0f)
#define CFG_SYS_NOR_FTIM2	FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWP(0x1c)
#define CFG_SYS_NOR_FTIM3	0x0

#define CFG_SYS_FLASH_BANKS_LIST	{CFG_SYS_FLASH_BASE_PHYS}

/* CFI for NOR Flash */

/* NAND Flash on IFC */
#define CFG_SYS_NAND_BASE		0xff800000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_NAND_BASE_PHYS	0xfff800000ull
#else
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE
#endif

#define CFG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8	\
				| CSPR_MSEL_NAND	\
				| CSPR_V)
#define CFG_SYS_NAND_AMASK	IFC_AMASK(64*1024)

#if defined(CONFIG_TARGET_P1010RDB_PA)
#define CFG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN	/* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_2	/* RAL = 2 Bytes */ \
				| CSOR_NAND_PGS_512	/* Page Size = 512b */ \
				| CSOR_NAND_SPRZ_16	/* Spare size = 16 */ \
				| CSOR_NAND_PB(32))	/* 32 Pages Per Block */

#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define CFG_SYS_NAND_CSOR   (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3       /* RAL = 3Byes */ \
				| CSOR_NAND_PGS_4K      /* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224    /* Spare size = 224 */ \
				| CSOR_NAND_PB(128))  /*Pages Per Block = 128 */
#endif

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

#if defined(CONFIG_TARGET_P1010RDB_PA)
/* NAND Flash Timing Params */
#define CFG_SYS_NAND_FTIM0		FTIM0_NAND_TCCST(0x01) | \
					FTIM0_NAND_TWP(0x0C)   | \
					FTIM0_NAND_TWCHT(0x04) | \
					FTIM0_NAND_TWH(0x05)
#define CFG_SYS_NAND_FTIM1		FTIM1_NAND_TADLE(0x1d) | \
					FTIM1_NAND_TWBE(0x1d)  | \
					FTIM1_NAND_TRR(0x07)   | \
					FTIM1_NAND_TRP(0x0c)
#define CFG_SYS_NAND_FTIM2		FTIM2_NAND_TRAD(0x0c) | \
					FTIM2_NAND_TREH(0x05) | \
					FTIM2_NAND_TWHRE(0x0f)
#define CFG_SYS_NAND_FTIM3		FTIM3_NAND_TWW(0x04)

#elif defined(CONFIG_TARGET_P1010RDB_PB)
/* support MT29F16G08ABABAWP 4k-pagesize 2G-bytes NAND */
/* ONFI NAND Flash mode0 Timing Params */
#define CFG_SYS_NAND_FTIM0  (FTIM0_NAND_TCCST(0x07)| \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CFG_SYS_NAND_FTIM1  (FTIM1_NAND_TADLE(0x32)| \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CFG_SYS_NAND_FTIM2  (FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a)  | \
					FTIM2_NAND_TWHRE(0x1e))
#define CFG_SYS_NAND_FTIM3	0x0
#endif

/* Set up IFC registers for boot location NOR/NAND */
#if defined(CONFIG_MTD_RAW_NAND) || defined(CONFIG_NAND_SECBOOT)
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3
#define CFG_SYS_CSPR1		CFG_SYS_NOR_CSPR
#define CFG_SYS_AMASK1		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR1		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS1_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS1_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS1_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS1_FTIM3		CFG_SYS_NOR_FTIM3
#else
#define CFG_SYS_CSPR0		CFG_SYS_NOR_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NOR_FTIM3
#define CFG_SYS_CSPR1		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK1		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR1		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS1_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS1_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS1_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS1_FTIM3		CFG_SYS_NAND_FTIM3
#endif

/* CPLD on IFC */
#define CFG_SYS_CPLD_BASE		0xffb00000

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_CPLD_BASE_PHYS	0xfffb00000ull
#else
#define CFG_SYS_CPLD_BASE_PHYS	CFG_SYS_CPLD_BASE
#endif

#define CFG_SYS_CSPR3	(CSPR_PHYS_ADDR(CFG_SYS_CPLD_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CFG_SYS_AMASK3		IFC_AMASK(64*1024)
#define CFG_SYS_CSOR3		0x0
/* CPLD Timing parameters for IFC CS3 */
#define CFG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CFG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0x0e) | \
					FTIM1_GPCM_TRAD(0x1f))
#define CFG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0x0e) | \
					FTIM2_GPCM_TCH(0x8) | \
					FTIM2_GPCM_TWP(0x1f))
#define CFG_SYS_CS3_FTIM3		0x0

#define CFG_SYS_INIT_RAM_ADDR	0xffd00000 /* stack in RAM */
#define CFG_SYS_INIT_RAM_SIZE	0x00004000 /* End of used area in RAM */

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * Config the L2 Cache as L2 SRAM
 */
#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
#define CFG_SYS_INIT_L2_ADDR		0xD0000000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#elif defined(CONFIG_MTD_RAW_NAND)
#ifdef CONFIG_TPL_BUILD
#define CFG_SYS_INIT_L2_ADDR		0xD0000000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#else
#define CFG_SYS_INIT_L2_ADDR		0xD0000000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
#define CFG_SYS_INIT_L2_END	(CFG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)
#endif
#endif
#endif

/* Serial Port */
#define CFG_SYS_NS16550_CLK		get_bus_freq(0)

#define CFG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CFG_SYS_NS16550_COM1	(CFG_SYS_CCSRBAR+0x4500)
#define CFG_SYS_NS16550_COM2	(CFG_SYS_CCSRBAR+0x4600)

/* I2C */
#define I2C_PCA9557_ADDR1		0x18
#define I2C_PCA9557_ADDR2		0x19
#define I2C_PCA9557_BUS_NUM		0

/* I2C EEPROM */
#if defined(CONFIG_TARGET_P1010RDB_PB)
#define MAX_NUM_PORTS			9 /* for 128Bytes EEPROM */
#endif
/* enable read and write access to EEPROM */

/* RTC */
#define CFG_SYS_I2C_RTC_ADDR	0x68

/*
 * SPI interface will not be available in case of NAND boot SPI CS0 will be
 * used for SLIC
 */
#if !defined(CONFIG_MTD_RAW_NAND) || !defined(CONFIG_NAND_SECBOOT)
/* eSPI - Enhanced SPI */
#endif

#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR	CFG_SYS_MPC85xx_ESDHC_ADDR
#endif

/*
 * Environment
 */
#if defined(CONFIG_MTD_RAW_NAND)
#ifdef CONFIG_TPL_BUILD
#define SPL_ENV_ADDR		(CFG_SYS_INIT_L2_ADDR + (160 << 10))
#endif
#endif

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI_HCD) \
		 || defined(CONFIG_FSL_SATA)
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(64 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

#define	CFG_EXTRA_ENV_SETTINGS				\
	"hwconfig=" __stringify(CONFIG_DEF_HWCONFIG)  "\0"	\
	"netdev=eth0\0"						\
	"uboot=" CONFIG_UBOOTPATH "\0"		\
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
	BOOTMODE

#if defined(CONFIG_TARGET_P1010RDB_PA)
#define BOOTMODE \
	"boot_bank0=i2c dev 0; i2c mw 18 1 f1; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00009 0; reset\0" \
	"boot_bank1=i2c dev 0; i2c mw 18 1 f1; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00009 1; reset\0" \
	"boot_nand=i2c dev 0; i2c mw 18 1 f9; i2c mw 18 3 f0;" \
	"mw.b ffb00011 0; mw.b ffb00017 1; reset\0"

#elif defined(CONFIG_TARGET_P1010RDB_PB)
#define BOOTMODE \
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

#include <asm/fsl_secure_boot.h>

#endif	/* __CONFIG_H */
