/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2020-2023 NXP
 */

/*
 * T4240 RDB board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

#define CFG_ICS307_REFCLK_HZ		25000000  /* ICS307 ref clk freq */

#ifdef CONFIG_RAMBOOT_PBL
#ifndef CONFIG_SDCARD
#define CFG_RESET_VECTOR_ADDRESS     0xfffffffc
#else
#define RESET_VECTOR_OFFSET		0x27FFC
#define BOOT_PAGE_OFFSET		0x27000

#ifdef	CONFIG_SDCARD
#define CFG_RESET_VECTOR_ADDRESS	0x200FFC
#define CFG_SYS_MMC_U_BOOT_SIZE	(768 << 10)
#define CFG_SYS_MMC_U_BOOT_DST	0x00200000
#define CFG_SYS_MMC_U_BOOT_START	0x00200000
#define CFG_SYS_MMC_U_BOOT_OFFS	(260 << 10)
#endif

#endif
#endif /* CONFIG_RAMBOOT_PBL */

/* High Level Configuration Options */

#ifndef CFG_RESET_VECTOR_ADDRESS
#define CFG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#define CFG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CFG_SYS_INIT_L3_ADDR		0xFFFC0000
#define SPL_ENV_ADDR			(CONFIG_SPL_GD_ADDR + 4 * 1024)

#define CFG_SYS_DCSRBAR		0xf0000000
#define CFG_SYS_DCSRBAR_PHYS		0xf00000000ull

/*
 * DDR Setup
 */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

/*
 * IFC Definitions
 */
#define CFG_SYS_FLASH_BASE	0xe0000000
#define CFG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CFG_SYS_FLASH_BASE)

/* define to use L1 as initial stack */
#define CFG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0xf
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW	0xfe03c000
/* The assembler doesn't like typecast */
#define CFG_SYS_INIT_RAM_ADDR_PHYS \
	((CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CFG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#define CFG_SYS_INIT_RAM_SIZE		0x00004000

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
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

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#define CFG_SYS_PCIE1_MEM_VIRT	0x80000000
#define CFG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#define CFG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CFG_SYS_PCIE1_IO_PHYS	0xff8000000ull

/* controller 2, Slot 2, tgtid 2, Base address 201000 */
#define CFG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CFG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#define CFG_SYS_PCIE2_IO_VIRT	0xf8010000
#define CFG_SYS_PCIE2_IO_PHYS	0xff8010000ull

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#define CFG_SYS_PCIE3_MEM_VIRT	0xc0000000
#define CFG_SYS_PCIE3_MEM_PHYS	0xc40000000ull

/* controller 4, Base address 203000 */
#define CFG_SYS_PCIE4_MEM_BUS	0xe0000000
#define CFG_SYS_PCIE4_MEM_PHYS	0xc60000000ull

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

#define HVBOOT					\
	"setenv bootargs config-addr=0x60000000; "	\
	"bootm 0x01000000 - 0x00f00000"

/*
 * DDR Setup
 */
#define SPD_EEPROM_ADDRESS1	0x52
#define SPD_EEPROM_ADDRESS2	0x54
#define SPD_EEPROM_ADDRESS3	0x56
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1	/* for p3041/p5010 */
#define CFG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * IFC Definitions
 */
#define CFG_SYS_NOR0_CSPR_EXT	(0xf)
#define CFG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS \
				+ 0x8000000) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CFG_SYS_NOR1_CSPR_EXT	(0xf)
#define CFG_SYS_NOR1_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CFG_SYS_NOR_AMASK	IFC_AMASK(128*1024*1024)
/* NOR Flash Timing Params */
#define CFG_SYS_NOR_CSOR	CSOR_NAND_TRHZ_80

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

#define CFG_SYS_FLASH_BANKS_LIST	{CFG_SYS_FLASH_BASE_PHYS \
					+ 0x8000000, CFG_SYS_FLASH_BASE_PHYS}

/* NAND Flash on IFC */
#define CFG_SYS_NAND_BASE		0xff800000
#define CFG_SYS_NAND_BASE_PHYS	(0xf00000000ull | CFG_SYS_NAND_BASE)

#define CFG_SYS_NAND_CSPR_EXT	(0xf)
#define CFG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CFG_SYS_NAND_AMASK	IFC_AMASK(64*1024)

#define CFG_SYS_NAND_CSOR    (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 2Byes */ \
				| CSOR_NAND_PGS_4K	/* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224	/* Spare size = 224 */ \
				| CSOR_NAND_PB(128))	/*Page Per Block = 128*/

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
#define CFG_SYS_CSPR2_EXT		CFG_SYS_NOR0_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_NOR0_CSPR
#define CFG_SYS_AMASK2		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS2_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS2_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS2_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS2_FTIM3		CFG_SYS_NOR_FTIM3
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
#define CFG_SYS_CSPR2_EXT		CFG_SYS_NOR1_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_NOR1_CSPR
#define CFG_SYS_AMASK2		CFG_SYS_NOR_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_NOR_CSOR
#define CFG_SYS_CS2_FTIM0		CFG_SYS_NOR_FTIM0
#define CFG_SYS_CS2_FTIM1		CFG_SYS_NOR_FTIM1
#define CFG_SYS_CS2_FTIM2		CFG_SYS_NOR_FTIM2
#define CFG_SYS_CS2_FTIM3		CFG_SYS_NOR_FTIM3

/* CPLD on IFC */
#define CFG_SYS_CPLD_BASE	0xffdf0000
#define CFG_SYS_CPLD_BASE_PHYS	(0xf00000000ull | CFG_SYS_CPLD_BASE)
#define CFG_SYS_CSPR3_EXT	(0xf)
#define CFG_SYS_CSPR3	(CSPR_PHYS_ADDR(CFG_SYS_CPLD_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)

#define CFG_SYS_AMASK3	IFC_AMASK(64 * 1024)
#define CFG_SYS_CSOR3	0x0

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

/* I2C */
#define I2C_MUX_PCA_ADDR_PRI		0x77 /* I2C bus multiplexer,primary */
#define I2C_MUX_PCA_ADDR_SEC		0x76 /* I2C bus multiplexer,secondary */

#define I2C_MUX_CH_DEFAULT	0x8
#define I2C_MUX_CH_VOL_MONITOR	0xa
#define I2C_MUX_CH_VSC3316_FS	0xc
#define I2C_MUX_CH_VSC3316_BS	0xd

/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_ADDR		0x40
#define I2C_VOL_MONITOR_BUS_V_OFFSET	0x2
#define I2C_VOL_MONITOR_BUS_V_OVF	0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT	3

/* The lowest and highest voltage allowed for T4240RDB */
#define VDD_MV_MIN			819
#define VDD_MV_MAX			1212

/*
 * eSPI - Enhanced SPI
 */

/* Qman/Bman */
#ifndef CONFIG_NOBQFMAN
#define CFG_SYS_BMAN_NUM_PORTALS	50
#define CFG_SYS_BMAN_MEM_BASE	0xf4000000
#define CFG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#define CFG_SYS_BMAN_MEM_SIZE	0x02000000
#define CFG_SYS_BMAN_SP_CENA_SIZE    0x4000
#define CFG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_BMAN_CENA_BASE       CFG_SYS_BMAN_MEM_BASE
#define CFG_SYS_BMAN_CENA_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_CINH_BASE       (CFG_SYS_BMAN_MEM_BASE + \
					CFG_SYS_BMAN_CENA_SIZE)
#define CFG_SYS_BMAN_CINH_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_SWP_ISDR_REG    0xE08
#define CFG_SYS_QMAN_NUM_PORTALS	50
#define CFG_SYS_QMAN_MEM_BASE	0xf6000000
#define CFG_SYS_QMAN_MEM_PHYS	0xff6000000ull
#define CFG_SYS_QMAN_MEM_SIZE	0x02000000
#define CFG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_QMAN_CENA_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_CINH_BASE       (CFG_SYS_QMAN_MEM_BASE + \
					CFG_SYS_QMAN_CENA_SIZE)
#define CFG_SYS_QMAN_CINH_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_SWP_ISDR_REG	0xE08
#endif /* CONFIG_NOBQFMAN */

#ifdef CONFIG_SYS_DPAA_FMAN
#define SGMII_PHY_ADDR1 0x0
#define SGMII_PHY_ADDR2 0x1
#define SGMII_PHY_ADDR3 0x2
#define SGMII_PHY_ADDR4 0x3
#define SGMII_PHY_ADDR5 0x4
#define SGMII_PHY_ADDR6 0x5
#define SGMII_PHY_ADDR7 0x6
#define SGMII_PHY_ADDR8 0x7
#define FM1_10GEC1_PHY_ADDR	0x10
#define FM1_10GEC2_PHY_ADDR	0x11
#define FM2_10GEC1_PHY_ADDR	0x12
#define FM2_10GEC2_PHY_ADDR	0x13
#define CORTINA_PHY_ADDR1	FM1_10GEC1_PHY_ADDR
#define CORTINA_PHY_ADDR2	FM1_10GEC2_PHY_ADDR
#define CORTINA_PHY_ADDR3	FM2_10GEC1_PHY_ADDR
#define CORTINA_PHY_ADDR4	FM2_10GEC2_PHY_ADDR
#endif

/*
* USB
*/

#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR       CFG_SYS_MPC85xx_ESDHC_ADDR
#endif


#define __USB_PHY_TYPE	utmi

/*
 * T4240 has 3 DDR controllers. Default to 3-way interleaving. It can be
 * 3way_1KB, 3way_4KB, 3way_8KB. T4160 has 2 DDR controllers. Default to 2-way
 * interleaving. It can be cacheline, page, bank, superbank.
 * See doc/README.fsl-ddr for details.
 */
#ifdef CONFIG_ARCH_T4240
#define CTRL_INTLV_PREFERED 3way_4KB
#else
#define CTRL_INTLV_PREFERED cacheline
#endif

#define	CFG_EXTRA_ENV_SETTINGS				\
	"hwconfig=fsl_ddr:"					\
	"ctlr_intlv=" __stringify(CTRL_INTLV_PREFERED) ","	\
	"bank_intlv=auto;"					\
	"usb1:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) "\0"\
	"netdev=eth0\0"						\
	"uboot=" CONFIG_UBOOTPATH "\0"		\
	"ubootaddr=" __stringify(CONFIG_TEXT_BASE) "\0"	\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
	"protect off $ubootaddr +$filesize && "			\
	"erase $ubootaddr +$filesize && "			\
	"cp.b $loadaddr $ubootaddr $filesize && "		\
	"protect on $ubootaddr +$filesize && "			\
	"cmp.b $loadaddr $ubootaddr $filesize\0"		\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=t4240rdb/ramdisk.uboot\0"			\
	"fdtaddr=1e00000\0"					\
	"fdtfile=t4240rdb/t4240rdb.dtb\0"			\
	"bdev=sda3\0"

#define HVBOOT					\
	"setenv bootargs config-addr=0x60000000; "	\
	"bootm 0x01000000 - 0x00f00000"

#include <asm/fsl_secure_boot.h>

#endif	/* __CONFIG_H */
