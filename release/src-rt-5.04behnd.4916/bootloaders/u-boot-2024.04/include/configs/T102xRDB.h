/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-2023 NXP
 */

/*
 * T1024/T1023 RDB board configuration file
 */

#ifndef __T1024RDB_H
#define __T1024RDB_H

#include <linux/stringify.h>

/* High Level Configuration Options */

#define CFG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS

#ifdef CONFIG_RAMBOOT_PBL
#define RESET_VECTOR_OFFSET		0x27FFC
#define BOOT_PAGE_OFFSET		0x27000

#ifdef CONFIG_MTD_RAW_NAND
#define CFG_SYS_NAND_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_NAND_U_BOOT_DST	0x30000000
#define CFG_SYS_NAND_U_BOOT_START	0x30000000
#endif

#ifdef CONFIG_SPIFLASH
#define CFG_RESET_VECTOR_ADDRESS		0x30000FFC
#define CFG_SYS_SPI_FLASH_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_SPI_FLASH_U_BOOT_DST		(0x30000000)
#define CFG_SYS_SPI_FLASH_U_BOOT_START	(0x30000000)
#define CFG_SYS_SPI_FLASH_U_BOOT_OFFS	(256 << 10)
#endif

#ifdef CONFIG_SDCARD
#define CFG_RESET_VECTOR_ADDRESS	0x30000FFC
#define CFG_SYS_MMC_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_MMC_U_BOOT_DST	(0x30000000)
#define CFG_SYS_MMC_U_BOOT_START	(0x30000000)
#define CFG_SYS_MMC_U_BOOT_OFFS	(260 << 10)
#endif

#endif /* CONFIG_RAMBOOT_PBL */

#ifndef CFG_RESET_VECTOR_ADDRESS
#define CFG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

/*
 * for slave u-boot IMAGE instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS1 0xfff00000ull
#define CFG_SRIO_PCIE_BOOT_IMAGE_SIZE     0x100000 /* 1M */
#ifdef CONFIG_PHYS_64BIT
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS 0xfef200000ull
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS2 0x3fff00000ull
#else
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS 0xef200000
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS2 0xfff00000
#endif
/*
 * for slave UCODE and ENV instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#ifdef CONFIG_PHYS_64BIT
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_PHYS 0xfef100000ull
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_BUS	 0x3ffe00000ull
#else
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_PHYS 0xef100000
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_BUS  0xffe00000
#endif
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_SIZE	0x40000 /* 256K */
/* slave core release by master*/
#define CFG_SRIO_PCIE_BOOT_BRR_OFFSET	0xe00e4
#define CFG_SRIO_PCIE_BOOT_RELEASE_MASK	0x00000001 /* release core 0 */

/* PCIe Boot - Slave */
#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
#define CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR 0xFFE00000
#define CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS \
		(0x300000000ull | CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR)
/* Set 1M boot space for PCIe boot */
#define CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR (CONFIG_TEXT_BASE & 0xfff00000)
#define CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS	\
		(0x300000000ull | CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR)
#define CFG_RESET_VECTOR_ADDRESS 0xfffffffc
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CFG_SYS_INIT_L2CSR0		L2CSR0_L2E

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CFG_SYS_INIT_L3_ADDR		0xFFFC0000
#define SPL_ENV_ADDR			(CONFIG_SPL_GD_ADDR + 4 * 1024)

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_DCSRBAR		0xf0000000
#define CFG_SYS_DCSRBAR_PHYS		0xf00000000ull
#endif

/*
 * DDR Setup
 */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#if defined(CONFIG_TARGET_T1024RDB)
#define SPD_EEPROM_ADDRESS	0x51
#define CFG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */
#elif defined(CONFIG_TARGET_T1023RDB)
#define CFG_SYS_SDRAM_SIZE   2048
#endif

/*
 * IFC Definitions
 */
#define CFG_SYS_FLASH_BASE	0xe8000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CFG_SYS_FLASH_BASE)
#else
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE
#endif

#define CFG_SYS_NOR0_CSPR_EXT	(0xf)
#define CFG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CFG_SYS_NOR_AMASK	IFC_AMASK(128*1024*1024)

/* NOR Flash Timing Params */
#if defined(CONFIG_TARGET_T1024RDB)
#define CFG_SYS_NOR_CSOR	CSOR_NAND_TRHZ_80
#elif defined(CONFIG_TARGET_T1023RDB)
#define CFG_SYS_NOR_CSOR    (CSOR_NOR_ADM_SHIFT(0) | \
				CSOR_NAND_TRHZ_80 | CSOR_NOR_ADM_SHFT_MODE_EN)
#endif
#define CFG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5))
#define CFG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x35) | \
				FTIM1_NOR_TRAD_NOR(0x1A) |\
				FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CFG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWPH(0x0E) | \
				FTIM2_NOR_TWP(0x1c))
#define CFG_SYS_NOR_FTIM3	0x0

#define CFG_SYS_FLASH_BANKS_LIST	{CFG_SYS_FLASH_BASE_PHYS}

#ifdef CONFIG_TARGET_T1024RDB
/* CPLD on IFC */
#define CFG_SYS_CPLD_BASE		0xffdf0000
#define CFG_SYS_CPLD_BASE_PHYS	(0xf00000000ull | CFG_SYS_CPLD_BASE)
#define CFG_SYS_CSPR2_EXT		(0xf)
#define CFG_SYS_CSPR2		(CSPR_PHYS_ADDR(CFG_SYS_CPLD_BASE) \
						| CSPR_PORT_SIZE_8 \
						| CSPR_MSEL_GPCM \
						| CSPR_V)
#define CFG_SYS_AMASK2		IFC_AMASK(64*1024)
#define CFG_SYS_CSOR2		0x0

/* CPLD Timing parameters for IFC CS2 */
#define CFG_SYS_CS2_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
						FTIM0_GPCM_TEADC(0x0e) | \
						FTIM0_GPCM_TEAHC(0x0e))
#define CFG_SYS_CS2_FTIM1		(FTIM1_GPCM_TACO(0x0e) | \
						FTIM1_GPCM_TRAD(0x1f))
#define CFG_SYS_CS2_FTIM2		(FTIM2_GPCM_TCS(0x0e) | \
						FTIM2_GPCM_TCH(0x8) | \
						FTIM2_GPCM_TWP(0x1f))
#define CFG_SYS_CS2_FTIM3		0x0
#endif

/* NAND Flash on IFC */
#define CFG_SYS_NAND_BASE		0xff800000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_NAND_BASE_PHYS	(0xf00000000ull | CFG_SYS_NAND_BASE)
#else
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE
#endif
#define CFG_SYS_NAND_CSPR_EXT	(0xf)
#define CFG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CFG_SYS_NAND_AMASK	IFC_AMASK(64*1024)

#if defined(CONFIG_TARGET_T1024RDB)
#define CFG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3Byes */ \
				| CSOR_NAND_PGS_4K	/* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224	/* Spare size = 224 */ \
				| CSOR_NAND_PB(64))	/*Pages Per Block = 64*/
#elif defined(CONFIG_TARGET_T1023RDB)
#define CFG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL 3Bytes */ \
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_128	/* Spare size = 128 */ \
				| CSOR_NAND_PB(64))	/*Pages Per Block = 64*/
#endif

/* ONFI NAND Flash mode0 Timing Params */
#define CFG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x07) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CFG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CFG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CFG_SYS_NAND_FTIM3		0x0

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

#if defined(CONFIG_MTD_RAW_NAND)
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3
#define CFG_SYS_CSPR1_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR1		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK1		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR1		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS1_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS1_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS1_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS1_FTIM3		CFG_SYS_NOR_FTIM3
#else
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NOR_FTIM3
#define CFG_SYS_CSPR1_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR1		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK1		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR1		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS1_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS1_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS1_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS1_FTIM3		CFG_SYS_NAND_FTIM3
#endif

/* define to use L1 as initial stack */
#define CFG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0xf
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW	0xfe03c000
/* The assembler doesn't like typecast */
#define CFG_SYS_INIT_RAM_ADDR_PHYS \
	((CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CFG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CFG_SYS_INIT_RAM_ADDR_PHYS	0xfe03c000 /* Initial L1 address */
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW CFG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CFG_SYS_INIT_RAM_SIZE		0x00004000

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* Serial Port */
#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CFG_SYS_NS16550_CLK		(get_bus_freq(0)/2)
#endif

#define CFG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CFG_SYS_NS16550_COM1	(CFG_SYS_CCSRBAR+0x11C500)
#define CFG_SYS_NS16550_COM2	(CFG_SYS_CCSRBAR+0x11C600)
#define CFG_SYS_NS16550_COM3	(CFG_SYS_CCSRBAR+0x11D500)
#define CFG_SYS_NS16550_COM4	(CFG_SYS_CCSRBAR+0x11D600)

/* I2C */

#define I2C_PCA6408_BUS_NUM		1
#define I2C_PCA6408_ADDR		0x20

/* I2C bus multiplexer */
#define I2C_MUX_CH_DEFAULT	0x8

/*
 * RTC configuration
 */
#define CFG_SYS_I2C_RTC_ADDR	0x68

/*
 * eSPI - Enhanced SPI
 */

/*
 * General PCIe
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

#ifdef CONFIG_PCI
/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#ifdef CONFIG_PCIE1
#define	CFG_SYS_PCIE1_MEM_VIRT	0x80000000
#define	CFG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#define CFG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CFG_SYS_PCIE1_IO_PHYS	0xff8000000ull
#endif

/* controller 2, Slot 2, tgtid 2, Base address 201000 */
#ifdef CONFIG_PCIE2
#define CFG_SYS_PCIE2_MEM_VIRT	0x90000000
#define CFG_SYS_PCIE2_MEM_PHYS	0xc10000000ull
#define CFG_SYS_PCIE2_IO_VIRT	0xf8010000
#define CFG_SYS_PCIE2_IO_PHYS	0xff8010000ull
#endif

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#ifdef CONFIG_PCIE3
#define CFG_SYS_PCIE3_MEM_VIRT	0xa0000000
#define CFG_SYS_PCIE3_MEM_PHYS	0xc20000000ull
#endif
#endif	/* CONFIG_PCI */

/*
 * USB
 */

/*
 * SDHC
 */
#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR	CFG_SYS_MPC85xx_ESDHC_ADDR
#endif

/* Qman/Bman */
#ifndef CONFIG_NOBQFMAN
#define CFG_SYS_BMAN_NUM_PORTALS	10
#define CFG_SYS_BMAN_MEM_BASE	0xf4000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#else
#define CFG_SYS_BMAN_MEM_PHYS	CFG_SYS_BMAN_MEM_BASE
#endif
#define CFG_SYS_BMAN_MEM_SIZE	0x02000000
#define CFG_SYS_BMAN_SP_CENA_SIZE    0x4000
#define CFG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_BMAN_CENA_BASE       CFG_SYS_BMAN_MEM_BASE
#define CFG_SYS_BMAN_CENA_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_CINH_BASE       (CFG_SYS_BMAN_MEM_BASE + \
					CFG_SYS_BMAN_CENA_SIZE)
#define CFG_SYS_BMAN_CINH_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_SWP_ISDR_REG	0xE08
#define CFG_SYS_QMAN_NUM_PORTALS	10
#define CFG_SYS_QMAN_MEM_BASE	0xf6000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_QMAN_MEM_PHYS	0xff6000000ull
#else
#define CFG_SYS_QMAN_MEM_PHYS	CFG_SYS_QMAN_MEM_BASE
#endif
#define CFG_SYS_QMAN_MEM_SIZE	0x02000000
#define CFG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_QMAN_CENA_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_CINH_BASE       (CFG_SYS_QMAN_MEM_BASE + \
					CFG_SYS_QMAN_CENA_SIZE)
#define CFG_SYS_QMAN_CINH_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_SWP_ISDR_REG	0xE08

#endif /* CONFIG_NOBQFMAN */

#ifdef CONFIG_SYS_DPAA_FMAN
#if defined(CONFIG_TARGET_T1024RDB)
#define RGMII_PHY1_ADDR		0x2
#define RGMII_PHY2_ADDR		0x6
#define SGMII_AQR_PHY_ADDR	0x2
#define FM1_10GEC1_PHY_ADDR	0x1
#elif defined(CONFIG_TARGET_T1023RDB)
#define RGMII_PHY1_ADDR		0x1
#define SGMII_RTK_PHY_ADDR	0x3
#define SGMII_AQR_PHY_ADDR	0x2
#endif
#endif

/*
 * Dynamic MTD Partition support with mtdparts
 */

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial map for Linux*/

/*
 * Environment Configuration
 */
#define __USB_PHY_TYPE		utmi

#ifdef CONFIG_ARCH_T1024
#define ARCH_EXTRA_ENV_SETTINGS \
	"bank_intlv=cs0_cs1\0"			\
	"ramdiskfile=t1024rdb/ramdisk.uboot\0"	\
	"fdtfile=t1024rdb/t1024rdb.dtb\0"
#else
#define ARCH_EXTRA_ENV_SETTINGS \
	"bank_intlv=null\0"			\
	"ramdiskfile=t1023rdb/ramdisk.uboot\0"	\
	"fdtfile=t1023rdb/t1023rdb.dtb\0"
#endif

#define	CFG_EXTRA_ENV_SETTINGS				\
	ARCH_EXTRA_ENV_SETTINGS					\
	"hwconfig=fsl_ddr:ctlr_intlv=cacheline,"		\
	"usb1:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) "\0"  \
	"uboot=" CONFIG_UBOOTPATH "\0"		\
	"ubootaddr=" __stringify(CONFIG_TEXT_BASE) "\0"	\
	"bootargs=root=/dev/ram rw console=ttyS0,115200\0" \
	"netdev=eth0\0"						\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
	"protect off $ubootaddr +$filesize && "			\
	"erase $ubootaddr +$filesize && "			\
	"cp.b $loadaddr $ubootaddr $filesize && "		\
	"protect on $ubootaddr +$filesize && "			\
	"cmp.b $loadaddr $ubootaddr $filesize\0"		\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=2000000\0"					\
	"fdtaddr=1e00000\0"					\
	"bdev=sda3\0"

#include <asm/fsl_secure_boot.h>

#endif	/* __T1024RDB_H */
