/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
 * Copyright 2020-2021 NXP
 */

/*
 * P2041 RDB board configuration file
 * Also supports P2040 RDB
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_RAMBOOT_PBL
#define CFG_RESET_VECTOR_ADDRESS	0xfffffffc
#endif

#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
/* Set 1M boot space */
#define CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR (CONFIG_TEXT_BASE & 0xfff00000)
#define CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS \
		(0x300000000ull | CFG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR)
#define CFG_RESET_VECTOR_ADDRESS 0xfffffffc
#endif

/* High Level Configuration Options */

#ifndef CFG_RESET_VECTOR_ADDRESS
#define CFG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#define CFG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS

#ifndef __ASSEMBLY__
#include <linux/stringify.h>
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CFG_SYS_INIT_L2CSR0		L2CSR0_L2E

#define CFG_POST CFG_SYS_POST_MEMORY	/* test POST memory test */

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CFG_SYS_INIT_L3_ADDR		CONFIG_TEXT_BASE
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_INIT_L3_ADDR_PHYS	(0xf00000000ull | CONFIG_TEXT_BASE)
#else
#define CFG_SYS_INIT_L3_ADDR_PHYS	CFG_SYS_INIT_L3_ADDR
#endif

#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_DCSRBAR		0xf0000000
#define CFG_SYS_DCSRBAR_PHYS		0xf00000000ull
#endif

/*
 * DDR Setup
 */
#define CFG_SYS_DDR_SDRAM_BASE	0x00000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

#define SPD_EEPROM_ADDRESS	0x52
#define CFG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * Local Bus Definitions
 */

/* Set the local bus clock 1/8 of platform clock */
#define CFG_SYS_LBC_LCRR		LCRR_CLKDIV_8

/*
 * This board doesn't have a promjet connector.
 * However, it uses commone corenet board LAW and TLB.
 * It is necessary to use the same start address with proper offset.
 */
#define CFG_SYS_FLASH_BASE		0xe0000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_FLASH_BASE_PHYS	0xfe0000000ull
#else
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE
#endif

#define CPLD_BASE		0xffdf0000	/* CPLD registers */
#ifdef CONFIG_PHYS_64BIT
#define CPLD_BASE_PHYS		0xfffdf0000ull
#else
#define CPLD_BASE_PHYS		CPLD_BASE
#endif

#define PIXIS_LBMAP_SWITCH	7
#define PIXIS_LBMAP_MASK	0xf0
#define PIXIS_LBMAP_SHIFT	4
#define PIXIS_LBMAP_ALTBANK	0x40

/* Nand Flash */
#ifdef CONFIG_NAND_FSL_ELBC
#define CFG_SYS_NAND_BASE		0xffa00000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_NAND_BASE_PHYS	0xfffa00000ull
#else
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE
#endif

#define CFG_SYS_NAND_BASE_LIST     {CFG_SYS_NAND_BASE}

/* NAND flash config */
#define CFG_SYS_NAND_BR_PRELIM  (BR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
			       | (2<<BR_DECC_SHIFT)    /* Use HW ECC */ \
			       | BR_PS_8	       /* Port Size = 8 bit */ \
			       | BR_MS_FCM	       /* MSEL = FCM */ \
			       | BR_V)		       /* valid */
#define CFG_SYS_NAND_OR_PRELIM  (0xFFFC0000	      /* length 256K */ \
			       | OR_FCM_PGS	       /* Large Page*/ \
			       | OR_FCM_CSCT \
			       | OR_FCM_CST \
			       | OR_FCM_CHT \
			       | OR_FCM_SCY_1 \
			       | OR_FCM_TRLX \
			       | OR_FCM_EHTR)
#endif /* CONFIG_NAND_FSL_ELBC */

#define CFG_SYS_FLASH_BANKS_LIST	{CFG_SYS_FLASH_BASE_PHYS + 0x8000000}

/* define to use L1 as initial stack */
#define CFG_SYS_INIT_RAM_ADDR	0xffd00000	/* Initial L1 address */
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW CFG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CFG_SYS_INIT_RAM_ADDR_PHYS \
	((CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CFG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CFG_SYS_INIT_RAM_ADDR_PHYS	CFG_SYS_INIT_RAM_ADDR
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW CFG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CFG_SYS_INIT_RAM_SIZE	0x00004000

#define CFG_SYS_INIT_SP_OFFSET	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CFG_SYS_NS16550_CLK		(get_bus_freq(0)/2)

#define CFG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CFG_SYS_NS16550_COM1	(CFG_SYS_CCSRBAR+0x11C500)
#define CFG_SYS_NS16550_COM2	(CFG_SYS_CCSRBAR+0x11C600)
#define CFG_SYS_NS16550_COM3	(CFG_SYS_CCSRBAR+0x11D500)
#define CFG_SYS_NS16550_COM4	(CFG_SYS_CCSRBAR+0x11D600)

/* I2C */


/*
 * RapidIO
 */
#define CFG_SYS_SRIO1_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_SRIO1_MEM_PHYS	0xc20000000ull
#else
#define CFG_SYS_SRIO1_MEM_PHYS	0xa0000000
#endif
#define CFG_SYS_SRIO1_MEM_SIZE	0x10000000	/* 256M */

#define CFG_SYS_SRIO2_MEM_VIRT	0xb0000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_SRIO2_MEM_PHYS	0xc30000000ull
#else
#define CFG_SYS_SRIO2_MEM_PHYS	0xb0000000
#endif
#define CFG_SYS_SRIO2_MEM_SIZE	0x10000000	/* 256M */

/*
 * for slave u-boot IMAGE instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_PHYS 0xfef200000ull
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS1 0xfff00000ull
#define CFG_SRIO_PCIE_BOOT_IMAGE_SIZE 0x100000	/* 1M */
#define CFG_SRIO_PCIE_BOOT_IMAGE_MEM_BUS2 0x3fff00000ull
/*
 * for slave UCODE and ENV instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_PHYS 0xfef100000ull
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_MEM_BUS 0x3ffe00000ull
#define CFG_SRIO_PCIE_BOOT_UCODE_ENV_SIZE 0x40000	/* 256K */

/* slave core release by master*/
#define CFG_SRIO_PCIE_BOOT_BRR_OFFSET 0xe00e4
#define CFG_SRIO_PCIE_BOOT_RELEASE_MASK 0x00000001 /* release core 0 */

/*
 * SRIO_PCIE_BOOT - SLAVE
 */
#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
#define CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR 0xFFE00000
#define CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS \
		(0x300000000ull | CFG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR)
#endif

/*
 * eSPI - Enhanced SPI
 */

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

/* Qman/Bman */
#define CFG_SYS_BMAN_NUM_PORTALS	10
#define CFG_SYS_BMAN_MEM_BASE	0xf4000000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#else
#define CFG_SYS_BMAN_MEM_PHYS	CFG_SYS_BMAN_MEM_BASE
#endif
#define CFG_SYS_BMAN_MEM_SIZE	0x00200000
#define CFG_SYS_BMAN_SP_CENA_SIZE    0x4000
#define CFG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_BMAN_CENA_BASE       CFG_SYS_BMAN_MEM_BASE
#define CFG_SYS_BMAN_CENA_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_CINH_BASE       (CFG_SYS_BMAN_MEM_BASE + \
					CFG_SYS_BMAN_CENA_SIZE)
#define CFG_SYS_BMAN_CINH_SIZE       (CFG_SYS_BMAN_MEM_SIZE >> 1)
#define CFG_SYS_BMAN_SWP_ISDR_REG	0xE08
#define CFG_SYS_QMAN_NUM_PORTALS	10
#define CFG_SYS_QMAN_MEM_BASE	0xf4200000
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_QMAN_MEM_PHYS	0xff4200000ull
#else
#define CFG_SYS_QMAN_MEM_PHYS	CFG_SYS_QMAN_MEM_BASE
#endif
#define CFG_SYS_QMAN_MEM_SIZE	0x00200000
#define CFG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CFG_SYS_QMAN_CENA_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_CINH_BASE       (CFG_SYS_QMAN_MEM_BASE + \
					CFG_SYS_QMAN_CENA_SIZE)
#define CFG_SYS_QMAN_CINH_SIZE       (CFG_SYS_QMAN_MEM_SIZE >> 1)
#define CFG_SYS_QMAN_SWP_ISDR_REG	0xE08

#ifdef CONFIG_FMAN_ENET
#define CFG_SYS_FM1_DTSEC1_PHY_ADDR	0x2
#define CFG_SYS_FM1_DTSEC2_PHY_ADDR	0x3
#define CFG_SYS_FM1_DTSEC3_PHY_ADDR	0x4
#define CFG_SYS_FM1_DTSEC4_PHY_ADDR	0x1
#define CFG_SYS_FM1_DTSEC5_PHY_ADDR	0x0

#define CFG_SYS_FM1_DTSEC1_RISER_PHY_ADDR	0x1c
#define CFG_SYS_FM1_DTSEC2_RISER_PHY_ADDR	0x1d
#define CFG_SYS_FM1_DTSEC3_RISER_PHY_ADDR	0x1e
#define CFG_SYS_FM1_DTSEC4_RISER_PHY_ADDR	0x1f

#define CFG_SYS_FM1_10GEC1_PHY_ADDR	0

#define CFG_SYS_TBIPA_VALUE	8
#endif

#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR       CFG_SYS_MPC85xx_ESDHC_ADDR
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory for Linux */

/*
 * Environment Configuration
 */

#define __USB_PHY_TYPE	utmi

#define	CFG_EXTRA_ENV_SETTINGS				\
	"hwconfig=fsl_ddr:ctlr_intlv=cacheline,"		\
	"bank_intlv=cs0_cs1\0"					\
	"netdev=eth0\0"						\
	"uboot=" CONFIG_UBOOTPATH "\0"			\
	"ubootaddr=" __stringify(CONFIG_TEXT_BASE) "\0"		\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
	"protect off $ubootaddr +$filesize && "			\
	"erase $ubootaddr +$filesize && "			\
	"cp.b $loadaddr $ubootaddr $filesize && "		\
	"protect on $ubootaddr +$filesize && "			\
	"cmp.b $loadaddr $ubootaddr $filesize\0"		\
	"consoledev=ttyS0\0"					\
	"usb_phy_type=" __stringify(__USB_PHY_TYPE) "\0"		\
	"usb_dr_mode=host\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=p2041rdb/ramdisk.uboot\0"			\
	"fdtaddr=1e00000\0"					\
	"fdtfile=p2041rdb/p2041rdb.dtb\0"			\
	"bdev=sda3\0"

#include <asm/fsl_secure_boot.h>

#endif	/* __CONFIG_H */
