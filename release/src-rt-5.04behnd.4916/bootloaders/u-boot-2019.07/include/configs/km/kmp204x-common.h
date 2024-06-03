/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 */

#ifndef _CONFIG_KMP204X_H
#define _CONFIG_KMP204X_H

#define CONFIG_KM_DEF_NETDEV	"netdev=eth0\0"

/* an additionnal option is required for UBI as subpage access is
 * supported in u-boot */
#define CONFIG_KM_UBI_PART_BOOT_OPTS		",2048"

#define CONFIG_NAND_ECC_BCH

/* common KM defines */
#include "keymile-common.h"

#define CONFIG_SYS_RAMBOOT
#define CONFIG_RAMBOOT_PBL
#define CONFIG_RAMBOOT_TEXT_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_RESET_VECTOR_ADDRESS	0xfffffffc
#define CONFIG_SYS_FSL_PBL_PBI board/keymile/kmp204x/pbi.cfg
#define CONFIG_SYS_FSL_PBL_RCW board/keymile/kmp204x/rcw_kmp204x.cfg

/* High Level Configuration Options */
#define CONFIG_SYS_BOOK3E_HV		/* Category E.HV supported */
#define CONFIG_FSL_CORENET		/* Freescale CoreNet platform */

#define CONFIG_SYS_FSL_CPC		/* Corenet Platform Cache */
#define CONFIG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS
#define CONFIG_PCIE1			/* PCIE controller 1 */
#define CONFIG_PCIE3			/* PCIE controller 3 */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_SYS_DPAA_RMAN		/* RMan */

/* Environment in SPI Flash */
#define CONFIG_ENV_OFFSET               0x100000	/* 1MB for u-boot */
#define CONFIG_ENV_SIZE			0x004000	/* 16K env */
#define CONFIG_ENV_SECT_SIZE            0x010000
#define CONFIG_ENV_OFFSET_REDUND	0x110000
#define CONFIG_ENV_TOTAL_SIZE		0x020000

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#ifndef __ASSEMBLY__
unsigned long get_board_sys_clk(unsigned long dummy);
#endif
#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0)

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_SYS_CACHE_STASHING
#define CONFIG_BACKSIDE_L2_CACHE
#define CONFIG_SYS_INIT_L2CSR0		L2CSR0_L2E
#define CONFIG_BTB			/* toggle branch predition */

#define CONFIG_ENABLE_36BIT_PHYS

#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		64	/* number of TLB1 entries */

#define CONFIG_POST CONFIG_SYS_POST_MEM_REGIONS	/* POST memory regions test */

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CONFIG_SYS_INIT_L3_ADDR		CONFIG_RAMBOOT_TEXT_BASE
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	(0xf00000000ull | \
		CONFIG_RAMBOOT_TEXT_BASE)
#define CONFIG_SYS_L3_SIZE		(1024 << 10)
#define CONFIG_SYS_INIT_L3_END (CONFIG_SYS_INIT_L3_ADDR + CONFIG_SYS_L3_SIZE)

#define CONFIG_SYS_DCSRBAR		0xf0000000
#define CONFIG_SYS_DCSRBAR_PHYS		0xf00000000ull

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(4 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define CONFIG_DDR_SPD

#define CONFIG_SYS_SPD_BUS_NUM	0
#define SPD_EEPROM_ADDRESS	0x54
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/******************************************************************************
 * (PRAM usage)
 * ... -------------------------------------------------------
 * ... |ROOTFSSIZE | PNVRAM |PHRAM |RESERVED_PRAM | END_OF_RAM
 * ... |<------------------- pram -------------------------->|
 * ... -------------------------------------------------------
 * @END_OF_RAM:
 * @CONFIG_KM_RESERVED_PRAM: reserved pram for special purpose
 * @CONFIG_KM_PHRAM: address for /var
 * @CONFIG_KM_PNVRAM: address for PNVRAM (for the application)
 * @CONFIG_KM_ROOTFSSIZE: address for rootfilesystem in RAM
 */

/* size of rootfs in RAM */
#define CONFIG_KM_ROOTFSSIZE	0x0
/* pseudo-non volatile RAM [hex] */
#define CONFIG_KM_PNVRAM	0x80000
/* physical RAM MTD size [hex] */
#define CONFIG_KM_PHRAM		0x100000
/* reserved pram area at the end of memory [hex]
 * u-boot reserves some memory for the MP boot page */
#define CONFIG_KM_RESERVED_PRAM	0x1000
/* set the default PRAM value to at least PNVRAM + PHRAM when pram env variable
 * is not valid yet, which is the case for when u-boot copies itself to RAM */
#define CONFIG_PRAM		((CONFIG_KM_PNVRAM + CONFIG_KM_PHRAM)>>10)

#define CONFIG_KM_CRAMFS_ADDR	0x2000000
#define CONFIG_KM_KERNEL_ADDR	0x1000000	/* max kernel size 15.5Mbytes */
#define CONFIG_KM_FDT_ADDR	0x1F80000	/* max dtb    size  0.5Mbytes */

/*
 * Local Bus Definitions
 */

/* Set the local bus clock 1/8 of plat clk, 2 clk delay LALE */
#define CONFIG_SYS_LBC_LCRR		(LCRR_CLKDIV_8 | LCRR_EADC_2)

/* Nand Flash */
#define CONFIG_NAND_FSL_ELBC
#define CONFIG_SYS_NAND_BASE		0xffa00000
#define CONFIG_SYS_NAND_BASE_PHYS	0xfffa00000ull

#define CONFIG_SYS_NAND_BASE_LIST     {CONFIG_SYS_NAND_BASE}
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BLOCK_SIZE    (128 * 1024)

/* NAND flash config */
#define CONFIG_SYS_NAND_BR_PRELIM  (BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
			       | BR_PS_8	       /* Port Size = 8 bit */ \
			       | BR_MS_FCM	       /* MSEL = FCM */ \
			       | BR_V)		       /* valid */

#define CONFIG_SYS_NAND_OR_PRELIM  (OR_AM_256KB	      /* length 256K */ \
			       | OR_FCM_BCTLD	/* LBCTL not ass */	\
			       | OR_FCM_SCY_1	/* 1 clk wait cycle */	\
			       | OR_FCM_RST	/* 1 clk read setup */	\
			       | OR_FCM_PGS	/* Large page size */	\
			       | OR_FCM_CST)	/* 0.25 command setup */

#define CONFIG_SYS_BR0_PRELIM  CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */

/* QRIO FPGA */
#define CONFIG_SYS_QRIO_BASE		0xfb000000
#define CONFIG_SYS_QRIO_BASE_PHYS	0xffb000000ull

#define CONFIG_SYS_QRIO_BR_PRELIM  (BR_PHYS_ADDR(CONFIG_SYS_QRIO_BASE_PHYS) \
				| BR_PS_8	/* Port Size 8 bits */ \
				| BR_DECC_OFF	/* no error corr */ \
				| BR_MS_GPCM	/* MSEL = GPCM */ \
				| BR_V)		/* valid */

#define CONFIG_SYS_QRIO_OR_PRELIM  (OR_AM_64KB	/* length 64K */ \
				| OR_GPCM_BCTLD /* no LCTL assert */ \
				| OR_GPCM_ACS_DIV4 /* LCS 1/4 clk after */ \
				| OR_GPCM_SCY_2 /* 2 clk wait cycles */ \
				| OR_GPCM_TRLX /* relaxed tmgs */ \
				| OR_GPCM_EAD) /* extra bus clk cycles */

#define CONFIG_SYS_BR1_PRELIM  CONFIG_SYS_QRIO_BR_PRELIM /* QRIO Base Address */
#define CONFIG_SYS_OR1_PRELIM  CONFIG_SYS_QRIO_OR_PRELIM /* QRIO Options */

#define CONFIG_MISC_INIT_F

#define CONFIG_HWCONFIG

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000	/* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#define CONFIG_SYS_INIT_RAM_SIZE	0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0)/2)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x11C500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x11C600)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_CCSRBAR+0x11D500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_CCSRBAR+0x11D600)

#define CONFIG_KM_CONSOLE_TTY	"ttyS0"

/* I2C */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_INIT_BOARD
#define CONFIG_SYS_I2C_SPEED		100000 /* deblocking */
#define CONFIG_SYS_NUM_I2C_BUSES	3
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_I2C_FSL		/* Use FSL I2C driver */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x118000
#define CONFIG_SYS_I2C_BUSES	{	{0, {I2C_NULL_HOP} }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 1 } } }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 2 } } }, \
				}
#ifndef __ASSEMBLY__
void set_sda(int state);
void set_scl(int state);
int get_sda(void);
int get_scl(void);
#endif

#define CONFIG_KM_IVM_BUS		1	/* I2C1 (Mux-Port 1)*/

/*
 * eSPI - Enhanced SPI
 */

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xff8000000ull
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc20000000ull
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xf8010000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE3_IO_PHYS	0xff8010000ull
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

/* Qman/Bman */
#define CONFIG_SYS_BMAN_NUM_PORTALS	10
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#define CONFIG_SYS_BMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_BMAN_SP_CENA_SIZE    0x4000
#define CONFIG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_BMAN_CENA_BASE       CONFIG_SYS_BMAN_MEM_BASE
#define CONFIG_SYS_BMAN_CENA_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_CINH_BASE       (CONFIG_SYS_BMAN_MEM_BASE + \
					CONFIG_SYS_BMAN_CENA_SIZE)
#define CONFIG_SYS_BMAN_CINH_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_SWP_ISDR_REG	0xE08
#define CONFIG_SYS_QMAN_NUM_PORTALS	10
#define CONFIG_SYS_QMAN_MEM_BASE	0xf4200000
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff4200000ull
#define CONFIG_SYS_QMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_QMAN_SP_CENA_SIZE    0x4000
#define CONFIG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_QMAN_CENA_BASE       CONFIG_SYS_QMAN_MEM_BASE
#define CONFIG_SYS_QMAN_CENA_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_CINH_BASE       (CONFIG_SYS_QMAN_MEM_BASE + \
					CONFIG_SYS_QMAN_CENA_SIZE)
#define CONFIG_SYS_QMAN_CINH_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_SWP_ISDR_REG	0xE08

#define CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_DPAA_PME
/* Default address of microcode for the Linux Fman driver
 * env is stored at 0x100000, sector size is 0x10000, x2 (redundant)
 * ucode is stored after env, so we got 0x120000.
 */
#define CONFIG_SYS_FMAN_FW_ADDR	0x120000
#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

#define CONFIG_PHYLIB_10G

#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

/* RGMII (FM1@DTESC5) is used as debug itf, it's the only one configured */
#define CONFIG_SYS_FM1_DTSEC5_PHY_ADDR	0x11
#define CONFIG_SYS_TBIPA_VALUE	8
#define CONFIG_ETHPRIME		"FM1@DTSEC5"

/*
 * Environment
 */
#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * Hardware Watchdog
 */
#define CONFIG_WATCHDOG			/* enable CPU watchdog */
#define CONFIG_WATCHDOG_PRESC 34	/* wdog prescaler 2^(64-34) (~10min) */
#define CONFIG_WATCHDOG_RC WRC_CHIP	/* reset chip on watchdog event */


/*
 * additionnal command line configuration.
 */

/* we don't need flash support */
#undef CONFIG_JFFS2_CMDLINE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory for Linux */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

#define __USB_PHY_TYPE	utmi
#define CONFIG_USB_EHCI_FSL

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV "km-common=empty\0"
#endif

/* architecture specific default bootargs */
#define CONFIG_KM_DEF_BOOT_ARGS_CPU		""

/* FIXME: FDT_ADDR is unspecified */
#define CONFIG_KM_DEF_ENV_CPU						\
	"boot=bootm ${load_addr_r} - ${fdt_addr_r}\0"			\
	"cramfsloadfdt="						\
		"cramfsload ${fdt_addr_r} "				\
		"fdt_0x${IVM_BoardId}_0x${IVM_HWKey}.dtb\0"		\
	"fdt_addr_r=" __stringify(CONFIG_KM_FDT_ADDR) "\0"		\
	"u-boot="CONFIG_HOSTNAME "/u-boot.pbl\0"		\
	"update="							\
		"sf probe 0;sf erase 0 +${filesize};"			\
		"sf write ${load_addr_r} 0 ${filesize};\0"		\
	"set_fdthigh=true\0"						\
	"checkfdt=true\0"						\
	""

#define CONFIG_HW_ENV_SETTINGS						\
	"hwconfig=fsl_ddr:ctlr_intlv=cacheline\0"			\
	"usb_phy_type=" __stringify(__USB_PHY_TYPE) "\0"		\
	"usb_dr_mode=host\0"

#define CONFIG_KM_NEW_ENV						\
	"newenv=sf probe 0;"						\
		"sf erase " __stringify(CONFIG_ENV_OFFSET) " "		\
		__stringify(CONFIG_ENV_TOTAL_SIZE)"\0"

/* ppc_82xx is the equivalent to ppc_6xx, the generic ppc toolchain */
#ifndef CONFIG_KM_DEF_ARCH
#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_DEF_ENV						\
	CONFIG_KM_DEF_ARCH						\
	CONFIG_KM_NEW_ENV						\
	CONFIG_HW_ENV_SETTINGS						\
	"EEprom_ivm=pca9547:70:9\0"					\
	""

#endif /* _CONFIG_KMP204X_H */
