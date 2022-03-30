/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
 */

/*
 * xpedite517x board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SYS_BOARD_NAME	"XPedite5170"
#define CONFIG_SYS_FORM_3U_VPX	1
#define CONFIG_LINUX_RESET_VEC	0x100	/* Reset vector used by Linux */
#define CONFIG_BAT_RW		1	/* Use common BAT rw code */
#define CONFIG_ALTIVEC		1

#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup */
#define CONFIG_PCIE1		1	/* PCIE controller 1 */
#define CONFIG_PCIE2		1	/* PCIE controller 2 */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */

/*
 * DDR config
 */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */
#define CONFIG_DDR_SPD
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define SPD_EEPROM_ADDRESS1		0x54	/* Both channels use the */
#define SPD_EEPROM_ADDRESS2		0x54	/* same SPD data         */
#define SPD_EEPROM_OFFSET		0x200	/* OFFSET of SPD in EEPROM */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_MAX_DDR_BAT_SIZE	0x80000000	/* BAT mapping size */

/*
 * virtual address to be used for temporary mappings.  There
 * should be 128k free at this VA.
 */
#define CONFIG_SYS_SCRATCH_VA	0xe0000000

#ifndef __ASSEMBLY__
extern unsigned long get_board_sys_clk(unsigned long dummy);
#endif

#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk(0) /* sysclk for MPC86xx */

/*
 * L2CR setup
 */
#define CONFIG_SYS_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E)

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR		0xef000000	/* relocated CCSRBAR */
#define CONFIG_SYS_CCSRBAR_PHYS		CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH	0x0
#define CONFIG_SYS_IMMR			CONFIG_SYS_CCSRBAR

/*
 * Diagnostics
 */
#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x20000000
#define CONFIG_POST			(CONFIG_SYS_POST_MEMORY |\
					 CONFIG_SYS_POST_I2C)
/* The XPedite5170 can host an XMC which has an EEPROM at address 0x50 */
#define I2C_ADDR_IGNORE_LIST		{0x50}

/*
 * Memory map
 * 0x0000_0000	0x7fff_ffff	DDR			2G Cacheable
 * 0x8000_0000	0xbfff_ffff	PCIe1 Mem		1G non-cacheable
 * 0xc000_0000	0xcfff_ffff	PCIe2 Mem		256M non-cacheable
 * 0xe000_0000	0xe7ff_ffff	SRAM/SSRAM/L1 Cache	128M non-cacheable
 * 0xe800_0000	0xe87f_ffff	PCIe1 IO		8M non-cacheable
 * 0xe880_0000	0xe8ff_ffff	PCIe2 IO		8M non-cacheable
 * 0xef00_0000	0xef0f_ffff	CCSR/IMMR		1M non-cacheable
 * 0xef80_0000	0xef8f_ffff	NAND Flash		1M non-cacheable
 * 0xf000_0000	0xf7ff_ffff	NOR Flash 2		128M non-cacheable
 * 0xf800_0000	0xffff_ffff	NOR Flash 1		128M non-cacheable
 */

#define CONFIG_SYS_LBC_LCRR		(LCRR_CLKDIV_4 | LCRR_EADC_3)

/*
 * NAND flash configuration
 */
#define CONFIG_SYS_NAND_BASE		0xef800000
#define CONFIG_SYS_NAND_BASE2		0xef840000	/* Unused at this time */
#define CONFIG_SYS_NAND_BASE_LIST 	{CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_BASE2}
#define CONFIG_SYS_MAX_NAND_DEVICE	2
#define CONFIG_NAND_ACTL
#define CONFIG_SYS_NAND_ACTL_ALE 	(1 << 14)	/* C_LA14 */
#define CONFIG_SYS_NAND_ACTL_CLE 	(1 << 15)	/* C_LA15 */
#define CONFIG_SYS_NAND_ACTL_NCE	0		/* NCE not controlled by ADDR */
#define CONFIG_SYS_NAND_ACTL_DELAY	25
#define CONFIG_JFFS2_NAND

/*
 * NOR flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xf8000000
#define CONFIG_SYS_FLASH_BASE2		0xf0000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE2}
#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */
#define CONFIG_SYS_FLASH_AUTOPROTECT_LIST	{ {0xfff00000, 0xc0000}, \
						  {0xf7f00000, 0xc0000} }
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#define CONFIG_SYS_MONITOR_BASE_EARLY	0xfff00000	/* early monitor loc */

/*
 * Chip select configuration
 */
/* NOR Flash 0 on CS0 */
#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE	|\
				 BR_PS_16		|\
				 BR_V)
#define CONFIG_SYS_OR0_PRELIM	(OR_AM_128MB		|\
				 OR_GPCM_CSNT		|\
				 OR_GPCM_XACS		|\
				 OR_GPCM_ACS_DIV2	|\
				 OR_GPCM_SCY_8		|\
				 OR_GPCM_TRLX		|\
				 OR_GPCM_EHTR		|\
				 OR_GPCM_EAD)

/* NOR Flash 1 on CS1 */
#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_FLASH_BASE2	|\
				 BR_PS_16		|\
				 BR_V)
#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR0_PRELIM

/* NAND flash on CS2 */
#define CONFIG_SYS_BR2_PRELIM	(CONFIG_SYS_NAND_BASE	|\
				 BR_PS_8		|\
				 BR_V)
#define CONFIG_SYS_OR2_PRELIM	(OR_AM_256KB		|\
				 OR_GPCM_BCTLD		|\
				 OR_GPCM_CSNT		|\
				 OR_GPCM_ACS_DIV4	|\
				 OR_GPCM_SCY_4		|\
				 OR_GPCM_TRLX		|\
				 OR_GPCM_EHTR)

/* Optional NAND flash on CS3 */
#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_NAND_BASE2	|\
				 BR_PS_8		|\
				 BR_V)
#define CONFIG_SYS_OR3_PRELIM	CONFIG_SYS_OR2_PRELIM

/*
 * Use L1 as initial stack
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe0000000
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)	/* Reserve 512 KB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}
#define CONFIG_LOADS_ECHO		1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	100000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	100000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

/* PEX8518 slave I2C interface */
#define CONFIG_SYS_I2C_PEX8518_ADDR	0x70

/* I2C DS1631 temperature sensor */
#define CONFIG_SYS_I2C_LM90_ADDR	0x4c

/* I2C EEPROM - AT24C128B */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6	/* 64 byte pages */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* take up to 10 msec */

/* I2C RTC */
#define CONFIG_RTC_M41T11		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	2000

/* GPIO */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR0	0x18
#define CONFIG_SYS_I2C_PCA953X_ADDR1	0x1c
#define CONFIG_SYS_I2C_PCA953X_ADDR2	0x1e
#define CONFIG_SYS_I2C_PCA953X_ADDR3	0x1f
#define CONFIG_SYS_I2C_PCA953X_ADDR	CONFIG_SYS_I2C_PCA953X_ADDR0
#define CONFIG_SYS_I2C_PCA9553_ADDR	0x62

/*
 * PU = pulled high, PD = pulled low
 * I = input, O = output, IO = input/output
 */
/* PCA9557 @ 0x18*/
#define CONFIG_SYS_PCA953X_C0_SER0_EN		0x01 /* PU; UART0 enable (1: enabled) */
#define CONFIG_SYS_PCA953X_C0_SER0_MODE		0x02 /* PU; UART0 serial mode select */
#define CONFIG_SYS_PCA953X_C0_SER1_EN		0x04 /* PU; UART1 enable (1: enabled) */
#define CONFIG_SYS_PCA953X_C0_SER1_MODE		0x08 /* PU; UART1 serial mode select */
#define CONFIG_SYS_PCA953X_C0_FLASH_PASS_CS	0x10 /* PU; Boot flash CS select */
#define CONFIG_SYS_PCA953X_NVM_WP		0x20 /* PU; Set to 0 to enable NVM writing */

/* PCA9557 @ 0x1c*/
#define CONFIG_SYS_PCA953X_XMC0_ROOT0		0x01 /* PU; Low if XMC is RC */
#define CONFIG_SYS_PCA953X_PLUG_GPIO0		0x02 /* Samtec connector GPIO */
#define CONFIG_SYS_PCA953X_XMC0_WAKE		0x04 /* PU; XMC wake */
#define CONFIG_SYS_PCA953X_XMC0_BIST		0x08 /* PU; XMC built in self test */
#define CONFIG_SYS_PCA953X_XMC_PRESENT		0x10 /* PU; Low if XMC module installed */
#define CONFIG_SYS_PCA953X_PMC_PRESENT		0x20 /* PU; Low if PMC module installed */
#define CONFIG_SYS_PCA953X_PMC0_MONARCH		0x40 /* PMC monarch mode enable */
#define CONFIG_SYS_PCA953X_PMC0_EREADY		0x80 /* PU; PMC PCI eready */

/* PCA9557 @ 0x1e*/
#define CONFIG_SYS_PCA953X_P0_GA0		0x01 /* PU; VPX Geographical address */
#define CONFIG_SYS_PCA953X_P0_GA1		0x02 /* PU; VPX Geographical address */
#define CONFIG_SYS_PCA953X_P0_GA2		0x04 /* PU; VPX Geographical address */
#define CONFIG_SYS_PCA953X_P0_GA3		0x08 /* PU; VPX Geographical address */
#define CONFIG_SYS_PCA953X_P0_GA4		0x10 /* PU; VPX Geographical address */
#define CONFIG_SYS_PCA953X_P0_GAP		0x20 /* PU; VPX Geographical address parity */
#define CONFIG_SYS_PCA953X_P1_SYSEN		0x80 /* PU; VPX P1 SYSCON */

/* PCA9557 @ 0x1f */
#define CONFIG_SYS_PCA953X_VPX_GPIO0		0x01 /* PU; VPX P15 GPIO */
#define CONFIG_SYS_PCA953X_VPX_GPIO1		0x02 /* PU; VPX P15 GPIO */
#define CONFIG_SYS_PCA953X_VPX_GPIO2		0x04 /* PU; VPX P15 GPIO */
#define CONFIG_SYS_PCA953X_VPX_GPIO3		0x08 /* PU; VPX P15 GPIO */

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
/* PCIE1 - PEX8518 */
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	CONFIG_SYS_PCIE1_MEM_BUS
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x40000000	/* 1G */
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe8000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000	/* 8M */

/* PCIE2 - VPX P1 */
#define CONFIG_SYS_PCIE2_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	CONFIG_SYS_PCIE2_MEM_BUS
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_PHYS	0xe8800000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00800000	/* 8M */

/*
 * Networking options
 */
#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC1_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define CONFIG_HAS_ETH0

#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_PHY_ADDR		2
#define TSEC2_PHYIDX		0
#define CONFIG_HAS_ETH1

/*
 * BAT mappings
 */
#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
#define CONFIG_SYS_CCSR_DEFAULT_DBATL	(CONFIG_SYS_CCSRBAR_DEFAULT	|\
					 BATL_PP_RW			|\
					 BATL_CACHEINHIBIT		|\
					 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_CCSR_DEFAULT_DBATU	(CONFIG_SYS_CCSRBAR_DEFAULT	|\
					 BATU_BL_1M			|\
					 BATU_VS			|\
					 BATU_VP)
#define CONFIG_SYS_CCSR_DEFAULT_IBATL	(CONFIG_SYS_CCSRBAR_DEFAULT	|\
					 BATL_PP_RW			|\
					 BATL_CACHEINHIBIT)
#define CONFIG_SYS_CCSR_DEFAULT_IBATU	CONFIG_SYS_CCSR_DEFAULT_DBATU
#endif

/*
 * BAT0		2G	Cacheable, non-guarded
 * 0x0000_0000	2G	DDR
 */
#define CONFIG_SYS_DBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT0U	(BATU_BL_2G | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	CONFIG_SYS_DBAT0U

/*
 * BAT1		1G	Cache-inhibited, guarded
 * 0x8000_0000	1G	PCI-Express 1 Memory
 */
#define CONFIG_SYS_DBAT1L	(CONFIG_SYS_PCIE1_MEM_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	(CONFIG_SYS_PCIE1_MEM_PHYS	|\
				 BATU_BL_1G			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCIE1_MEM_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U	CONFIG_SYS_DBAT1U

/*
 * BAT2		512M	Cache-inhibited, guarded
 * 0xc000_0000	512M	PCI-Express 2 Memory
 */
#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_PCIE2_MEM_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_PCIE2_MEM_PHYS	|\
				 BATU_BL_512M			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCIE2_MEM_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	CONFIG_SYS_DBAT2U

/*
 * BAT3		1M	Cache-inhibited, guarded
 * 0xe000_0000	1M	CCSR
 */
#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_CCSRBAR		|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	(CONFIG_SYS_CCSRBAR		|\
				 BATU_BL_1M			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_CCSRBAR		|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	CONFIG_SYS_DBAT3U

/*
 * BAT4		32M	Cache-inhibited, guarded
 * 0xe200_0000	16M	PCI-Express 1 I/O
 * 0xe300_0000	16M	PCI-Express 2 I/0
 */
#define CONFIG_SYS_DBAT4L	(CONFIG_SYS_PCIE1_IO_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	(CONFIG_SYS_PCIE1_IO_PHYS	|\
				 BATU_BL_32M			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_PCIE1_IO_PHYS	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U	CONFIG_SYS_DBAT4U

/*
 * BAT5		128K	Cacheable, non-guarded
 * 0xe400_1000	128K	Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_INIT_RAM_ADDR	|\
				 BATL_PP_RW			|\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT5U	(CONFIG_SYS_INIT_RAM_ADDR	|\
				 BATU_BL_128K			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT5L	CONFIG_SYS_DBAT5L
#define CONFIG_SYS_IBAT5U	CONFIG_SYS_DBAT5U

/*
 * BAT6		256M	Cache-inhibited, guarded
 * 0xf000_0000	256M	FLASH
 */
#define CONFIG_SYS_DBAT6L	(CONFIG_SYS_FLASH_BASE2		|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	(CONFIG_SYS_FLASH_BASE		|\
				 BATU_BL_256M			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_FLASH_BASE		|\
				 BATL_PP_RW			|\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	CONFIG_SYS_DBAT6U

/* Map the last 1M of flash where we're running from reset */
#define CONFIG_SYS_DBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY	|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U_EARLY	(CONFIG_SYS_TEXT_BASE			|\
				 BATU_BL_1M			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY	|\
				 BATL_PP_RW			|\
				 BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U_EARLY	CONFIG_SYS_DBAT6U_EARLY

/*
 * BAT7		64M	Cache-inhibited, guarded
 * 0xe800_0000	64K	NAND FLASH
 * 0xe804_0000	128K	DUART Registers
 */
#define CONFIG_SYS_DBAT7L	(CONFIG_SYS_NAND_BASE		|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT		|\
				 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT7U 	(CONFIG_SYS_NAND_BASE		|\
				 BATU_BL_512K			|\
				 BATU_VS			|\
				 BATU_VP)
#define CONFIG_SYS_IBAT7L	(CONFIG_SYS_NAND_BASE		|\
				 BATL_PP_RW			|\
				 BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT7U	CONFIG_SYS_DBAT7U

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_LOADADDR		0x1000000	/* default location for tftp and bootm */
#define CONFIG_PREBOOT				/* enable preboot variable */
#define CONFIG_INTEGRITY			/* support booting INTEGRITY OS */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(16 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(16 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */
#define CONFIG_ENV_SECT_SIZE	0x20000		/* 128k (one sector) for env */
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

/*
 * Flash memory map:
 * fffc0000 - ffffffff	Pri FDT (256KB)
 * fff80000 - fffbffff	Pri U-Boot Environment (256 KB)
 * fff00000 - fff7ffff	Pri U-Boot (512 KB)
 * fef00000 - ffefffff	Pri OS image (16MB)
 * f8000000 - feefffff	Pri OS Use/Filesystem (111MB)
 *
 * f7fc0000 - f7ffffff	Sec FDT (256KB)
 * f7f80000 - f7fbffff	Sec U-Boot Environment (256 KB)
 * f7f00000 - f7f7ffff	Sec U-Boot (512 KB)
 * f6f00000 - f7efffff	Sec OS image (16MB)
 * f0000000 - f6efffff	Sec OS Use/Filesystem (111MB)
 */
#define CONFIG_UBOOT1_ENV_ADDR	__stringify(0xfff00000)
#define CONFIG_UBOOT2_ENV_ADDR	__stringify(0xf7f00000)
#define CONFIG_FDT1_ENV_ADDR	__stringify(0xfffc0000)
#define CONFIG_FDT2_ENV_ADDR	__stringify(0xf7fc0000)
#define CONFIG_OS1_ENV_ADDR	__stringify(0xfef00000)
#define CONFIG_OS2_ENV_ADDR	__stringify(0xf6f00000)

#define CONFIG_PROG_UBOOT1						\
	"$download_cmd $loadaddr $ubootfile; "				\
	"if test $? -eq 0; then "					\
		"protect off "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"erase "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"cp.w $loadaddr "CONFIG_UBOOT1_ENV_ADDR" 40000; "	\
		"protect on "CONFIG_UBOOT1_ENV_ADDR" +80000; "		\
		"cmp.b $loadaddr "CONFIG_UBOOT1_ENV_ADDR" 80000; "	\
		"if test $? -ne 0; then "				\
			"echo PROGRAM FAILED; "				\
		"else; "						\
			"echo PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_UBOOT2						\
	"$download_cmd $loadaddr $ubootfile; "				\
	"if test $? -eq 0; then "					\
		"protect off "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"erase "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"cp.w $loadaddr "CONFIG_UBOOT2_ENV_ADDR" 40000; "	\
		"protect on "CONFIG_UBOOT2_ENV_ADDR" +80000; "		\
		"cmp.b $loadaddr "CONFIG_UBOOT2_ENV_ADDR" 80000; "	\
		"if test $? -ne 0; then "				\
			"echo PROGRAM FAILED; "				\
		"else; "						\
			"echo PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_BOOT_OS_NET						\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"if test -n $fdtaddr; then "				\
			"$download_cmd $fdtaddr $fdtfile; "		\
			"if test $? -eq 0; then "			\
				"bootm $osaddr - $fdtaddr; "		\
			"else; "					\
				"echo FDT DOWNLOAD FAILED; "		\
			"fi; "						\
		"else; "						\
			"bootm $osaddr; "				\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_OS1							\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_OS1_ENV_ADDR" +$filesize; "		\
		"cp.b $osaddr "CONFIG_OS1_ENV_ADDR" $filesize; "	\
		"cmp.b $osaddr "CONFIG_OS1_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo OS PROGRAM FAILED; "			\
		"else; "						\
			"echo OS PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_OS2							\
	"$download_cmd $osaddr $osfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_OS2_ENV_ADDR" +$filesize; "		\
		"cp.b $osaddr "CONFIG_OS2_ENV_ADDR" $filesize; "	\
		"cmp.b $osaddr "CONFIG_OS2_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo OS PROGRAM FAILED; "			\
		"else; "						\
			"echo OS PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo OS DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_FDT1						\
	"$download_cmd $fdtaddr $fdtfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_FDT1_ENV_ADDR" +$filesize;"		\
		"cp.b $fdtaddr "CONFIG_FDT1_ENV_ADDR" $filesize; "	\
		"cmp.b $fdtaddr "CONFIG_FDT1_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo FDT PROGRAM FAILED; "			\
		"else; "						\
			"echo FDT PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo FDT DOWNLOAD FAILED; "				\
	"fi;"

#define CONFIG_PROG_FDT2						\
	"$download_cmd $fdtaddr $fdtfile; "				\
	"if test $? -eq 0; then "					\
		"erase "CONFIG_FDT2_ENV_ADDR" +$filesize;"		\
		"cp.b $fdtaddr "CONFIG_FDT2_ENV_ADDR" $filesize; "	\
		"cmp.b $fdtaddr "CONFIG_FDT2_ENV_ADDR" $filesize; "	\
		"if test $? -ne 0; then "				\
			"echo FDT PROGRAM FAILED; "			\
		"else; "						\
			"echo FDT PROGRAM SUCCEEDED; "			\
		"fi; "							\
	"else; "							\
		"echo FDT DOWNLOAD FAILED; "				\
	"fi;"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"autoload=yes\0"						\
	"download_cmd=tftp\0"						\
	"console_args=console=ttyS0,115200\0"				\
	"root_args=root=/dev/nfs rw\0"					\
	"misc_args=ip=on\0"						\
	"set_bootargs=setenv bootargs ${console_args} ${root_args} ${misc_args}\0" \
	"bootfile=/home/user/file\0"					\
	"osfile=/home/user/board.uImage\0"				\
	"fdtfile=/home/user/board.dtb\0"				\
	"ubootfile=/home/user/u-boot.bin\0"				\
	"fdtaddr=0x1e00000\0"						\
	"osaddr=0x1000000\0"						\
	"loadaddr=0x1000000\0"						\
	"prog_uboot1="CONFIG_PROG_UBOOT1"\0"				\
	"prog_uboot2="CONFIG_PROG_UBOOT2"\0"				\
	"prog_os1="CONFIG_PROG_OS1"\0"					\
	"prog_os2="CONFIG_PROG_OS2"\0"					\
	"prog_fdt1="CONFIG_PROG_FDT1"\0"				\
	"prog_fdt2="CONFIG_PROG_FDT2"\0"				\
	"bootcmd_net=run set_bootargs; "CONFIG_BOOT_OS_NET"\0"		\
	"bootcmd_flash1=run set_bootargs; "				\
		"bootm "CONFIG_OS1_ENV_ADDR" - "CONFIG_FDT1_ENV_ADDR"\0"\
	"bootcmd_flash2=run set_bootargs; "				\
		"bootm "CONFIG_OS2_ENV_ADDR" - "CONFIG_FDT2_ENV_ADDR"\0"\
	"bootcmd=run bootcmd_flash1\0"
#endif	/* __CONFIG_H */
