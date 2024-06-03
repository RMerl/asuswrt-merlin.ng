/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on corenet_ds.h
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#if !defined(CONFIG_ARCH_P5020) && !defined(CONFIG_ARCH_P5040)
#error Must call Cyrus CONFIG with a specific CPU enabled.
#endif

#define CONFIG_SDCARD
#define CONFIG_FSL_SATA_V2
#define CONFIG_PCIE3
#define CONFIG_PCIE4
#ifdef CONFIG_ARCH_P5020
#define CONFIG_SYS_FSL_RAID_ENGINE
#define CONFIG_SYS_DPAA_RMAN
#endif
#define CONFIG_SYS_DPAA_PME

/*
 * Corenet DS style board configuration file
 */
#define CONFIG_RAMBOOT_TEXT_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_RESET_VECTOR_ADDRESS	0xfffffffc
#define CONFIG_SYS_FSL_PBL_PBI board/varisys/cyrus/pbi.cfg
#if defined(CONFIG_ARCH_P5020)
#define CONFIG_SYS_CLK_FREQ 133000000
#define CONFIG_SYS_FSL_PBL_RCW board/varisys/cyrus/rcw_p5020_v2.cfg
#elif defined(CONFIG_ARCH_P5040)
#define CONFIG_SYS_CLK_FREQ 100000000
#define CONFIG_SYS_FSL_PBL_RCW board/varisys/cyrus/rcw_p5040.cfg
#endif

/* High Level Configuration Options */
#define CONFIG_SYS_BOOK3E_HV		/* Category E.HV supported */

#define CONFIG_SYS_MMC_MAX_DEVICE     1

#define CONFIG_SYS_FSL_CPC		/* Corenet Platform Cache */
#define CONFIG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS
#define CONFIG_PCIE1			/* PCIE controller 1 */
#define CONFIG_PCIE2			/* PCIE controller 2 */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_SDCARD)
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		(512 * 1658)
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_SYS_CACHE_STASHING
#define CONFIG_BACKSIDE_L2_CACHE
#define CONFIG_SYS_INIT_L2CSR0		L2CSR0_L2E
#define CONFIG_BTB			/* toggle branch predition */
#define	CONFIG_DDR_ECC
#ifdef CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#endif

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		64	/* number of TLB1 entries */
#endif

/* test POST memory test */
#undef CONFIG_POST
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CONFIG_SYS_INIT_L3_ADDR		CONFIG_RAMBOOT_TEXT_BASE
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	(0xf00000000ull | CONFIG_RAMBOOT_TEXT_BASE)
#else
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	CONFIG_SYS_INIT_L3_ADDR
#endif
#define CONFIG_SYS_L3_SIZE		(1024 << 10)
#define CONFIG_SYS_INIT_L3_END (CONFIG_SYS_INIT_L3_ADDR + CONFIG_SYS_L3_SIZE)

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_DCSRBAR		0xf0000000
#define CONFIG_SYS_DCSRBAR_PHYS		0xf00000000ull
#endif

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(4 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define CONFIG_DDR_SPD

#define CONFIG_SYS_SPD_BUS_NUM	1
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * Local Bus Definitions
 */

#define CONFIG_SYS_LBC0_BASE		0xe0000000 /* Start of LBC Registers */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC0_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_LBC0_BASE_PHYS	CONFIG_SYS_LBC0_BASE
#endif

#define CONFIG_SYS_LBC1_BASE		0xe1000000 /* Start of LBC Registers */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC1_BASE_PHYS	0xfe1000000ull
#else
#define CONFIG_SYS_LBC1_BASE_PHYS	CONFIG_SYS_LBC1_BASE
#endif

/* Set the local bus clock 1/16 of platform clock */
#define CONFIG_SYS_LBC_LCRR		(LCRR_CLKDIV_16 | LCRR_EADC_1)

#define CONFIG_SYS_BR0_PRELIM \
(BR_PHYS_ADDR(CONFIG_SYS_LBC0_BASE_PHYS) | BR_PS_16 | BR_V)
#define CONFIG_SYS_BR1_PRELIM \
(BR_PHYS_ADDR(CONFIG_SYS_LBC1_BASE_PHYS) | BR_PS_16 | BR_V)

#define CONFIG_SYS_OR0_PRELIM	0xfff00010
#define CONFIG_SYS_OR1_PRELIM	0xfff00010

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if defined(CONFIG_RAMBOOT_PBL)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_HWCONFIG

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000	/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0)/2)

#define CONFIG_SYS_BAUDRATE_TABLE	\
{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x11C500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x11C600)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_CCSRBAR+0x11D500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_CCSRBAR+0x11D600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_FSL_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET		0x118000
#define CONFIG_SYS_FSL_I2C2_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C2_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET		0x118100
#define CONFIG_SYS_FSL_I2C3_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C3_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C3_OFFSET		0x119000
#define CONFIG_SYS_FSL_I2C4_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_FSL_I2C4_SLAVE		0x7F
#define CONFIG_SYS_FSL_I2C4_OFFSET		0x119100

#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57

#define CONFIG_SYS_I2C_GENERIC_MAC
#define CONFIG_SYS_I2C_MAC1_BUS 3
#define CONFIG_SYS_I2C_MAC1_CHIP_ADDR 0x57
#define CONFIG_SYS_I2C_MAC1_DATA_ADDR 0xf2
#define CONFIG_SYS_I2C_MAC2_BUS 0
#define CONFIG_SYS_I2C_MAC2_CHIP_ADDR 0x50
#define CONFIG_SYS_I2C_MAC2_DATA_ADDR 0xfa

#define CONFIG_RTC_MCP79411		1
#define CONFIG_SYS_RTC_BUS_NUM		3
#define CONFIG_SYS_I2C_RTC_ADDR		0x6f

/*
 * eSPI - Enhanced SPI
 */

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xff8000000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xf8000000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 201000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xf8010000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xff8010000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xf8010000
#endif
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_PCIE3_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xf8020000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_IO_PHYS	0xff8020000ull
#else
#define CONFIG_SYS_PCIE3_IO_PHYS	0xf8020000
#endif
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

/* controller 4, Base address 203000 */
#define CONFIG_SYS_PCIE4_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE4_MEM_PHYS	0xc60000000ull
#define CONFIG_SYS_PCIE4_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE4_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE4_IO_PHYS	0xff8030000ull
#define CONFIG_SYS_PCIE4_IO_SIZE	0x00010000	/* 64k */

/* Qman/Bman */
#define CONFIG_SYS_BMAN_NUM_PORTALS	10
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#else
#define CONFIG_SYS_BMAN_MEM_PHYS	CONFIG_SYS_BMAN_MEM_BASE
#endif
#define CONFIG_SYS_BMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_BMAN_SP_CENA_SIZE    0x4000
#define CONFIG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_BMAN_CENA_BASE       CONFIG_SYS_BMAN_MEM_BASE
#define CONFIG_SYS_BMAN_CENA_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_CINH_BASE       (CONFIG_SYS_BMAN_MEM_BASE + \
					 CONFIG_SYS_BMAN_CENA_SIZE)
#define CONFIG_SYS_BMAN_CINH_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_SWP_ISDR_REG   0xE08
#define CONFIG_SYS_QMAN_NUM_PORTALS	10
#define CONFIG_SYS_QMAN_MEM_BASE	0xf4200000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff4200000ull
#else
#define CONFIG_SYS_QMAN_MEM_PHYS	CONFIG_SYS_QMAN_MEM_BASE
#endif
#define CONFIG_SYS_QMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_QMAN_SP_CENA_SIZE    0x4000
#define CONFIG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_QMAN_CENA_BASE       CONFIG_SYS_QMAN_MEM_BASE
#define CONFIG_SYS_QMAN_CENA_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_CINH_BASE       (CONFIG_SYS_QMAN_MEM_BASE + \
					  CONFIG_SYS_QMAN_CENA_SIZE)
#define CONFIG_SYS_QMAN_CINH_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_SWP_ISDR_REG   0xE08

#define CONFIG_SYS_DPAA_FMAN
/* Default address of microcode for the Linux Fman driver */
/*
 * PBL SD boot image should stored at 0x1000(8 blocks), the size of the image is
 * about 825KB (1650 blocks), Env is stored after the image, and the env size is
 * 0x2000 (16 blocks), 8 + 1650 + 16 = 1674, enlarge it to 1680.
 */
#define CONFIG_SYS_FMAN_FW_ADDR	(512 * 1680)

#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#endif	/* CONFIG_PCI */

/* SATA */
#ifdef CONFIG_FSL_SATA_V2
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#define CONFIG_LBA48
#endif

#ifdef CONFIG_FMAN_ENET
#define CONFIG_SYS_TBIPA_VALUE	8
#define CONFIG_ETHPRIME		"FM1@DTSEC4"
#endif

/*
 * Environment
 */
#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * USB
 */
#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_HAS_FSL_MPH_USB

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_EHCI_IS_TDI
 /* _VIA_CONTROL_EP  */
#endif

#ifdef CONFIG_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR       CONFIG_SYS_MPC85xx_ESDHC_ADDR
#define CONFIG_SYS_FSL_ESDHC_BROKEN_TIMEOUT
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
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define __USB_PHY_TYPE	utmi

#define	CONFIG_EXTRA_ENV_SETTINGS \
"hwconfig=fsl_ddr:ctlr_intlv=cacheline,"		\
"bank_intlv=cs0_cs1;"					\
"usb1:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) ";"\
"usb2:dr_mode=host,phy_type=" __stringify(__USB_PHY_TYPE) "\0"\
"netdev=eth0\0"						\
"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"			\
"consoledev=ttyS0\0"					\
"ramdiskaddr=2000000\0"					\
"fdtaddr=1e00000\0"					\
"bdev=sda3\0"

#define CONFIG_HDBOOT					\
"setenv bootargs root=/dev/$bdev rw "		\
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $loadaddr $bootfile;"			\
"tftp $fdtaddr $fdtfile;"			\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND			\
"setenv bootargs root=/dev/nfs rw "	\
"nfsroot=$serverip:$rootpath "		\
"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $loadaddr $bootfile;"		\
"tftp $fdtaddr $fdtfile;"		\
"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND				\
"setenv bootargs root=/dev/ram rw "		\
"console=$consoledev,$baudrate $othbootargs;"	\
"tftp $ramdiskaddr $ramdiskfile;"		\
"tftp $loadaddr $bootfile;"			\
"tftp $fdtaddr $fdtfile;"			\
"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#include <asm/fsl_secure_boot.h>

#endif	/* __CONFIG_H */
