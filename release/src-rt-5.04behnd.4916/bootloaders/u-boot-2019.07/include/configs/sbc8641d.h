/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007 Wind River Systems <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Joe Hamman <joe.hamman@embeddedspecialties.com>
 *
 * Copyright 2006 Freescale Semiconductor.
 *
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

/*
 * SBC8641D board configuration file
 *
 * Make sure you change the MAC address and other network params first,
 * search for CONFIG_SERVERIP, etc in this file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_LINUX_RESET_VEC  0x100   /* Reset vector used by Linux */

#ifdef RUN_DIAG
#define CONFIG_SYS_DIAG_ADDR        0xff800000
#endif

#define CONFIG_SYS_RESET_ADDRESS    0xfff00100

/*
 * virtual address to be used for temporary mappings.  There
 * should be 128k free at this VA.
 */
#define CONFIG_SYS_SCRATCH_VA	0xe8000000

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */

#define CONFIG_PCIE1		1	/* PCIE controller 1 (slot 1) */
#define CONFIG_PCIE2		1	/* PCIE controller 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT	1	/* Use common FSL init code */
#define CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */

#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAT_RW		1	/* Use common BAT rw code */

#undef CONFIG_SPD_EEPROM		/* Do not use SPD EEPROM for DDR setup*/
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */
#define CONFIG_MEM_INIT_VALUE		0xDeadBeef
#define CACHE_LINE_INTERLEAVING		0x20000000
#define PAGE_INTERLEAVING		0x21000000
#define BANK_INTERLEAVING		0x22000000
#define SUPER_BANK_INTERLEAVING		0x23000000

#define CONFIG_ALTIVEC          1

/*
 * L2CR setup -- make sure this is right for your board!
 */
#define CONFIG_SYS_L2
#define L2_INIT		0
#define L2_ENABLE	(L2CR_L2E)

#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ     get_board_sys_clk(0)
#endif

#undef	CONFIG_SYS_DRAM_TEST				/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 */
#define CONFIG_SYS_CCSRBAR		0xf8000000	/* relocated CCSRBAR */
#define CONFIG_SYS_IMMR		CONFIG_SYS_CCSRBAR	/* PQII uses CONFIG_SYS_IMMR */

#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH	0x0
#define CONFIG_SYS_CCSRBAR_PHYS		CONFIG_SYS_CCSRBAR_PHYS_LOW

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory */
#define CONFIG_SYS_DDR_SDRAM_BASE2	0x10000000	/* DDR bank 2 */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_BASE2		CONFIG_SYS_DDR_SDRAM_BASE2
#define CONFIG_SYS_MAX_DDR_BAT_SIZE	0x80000000	/* BAT mapping size */
#define CONFIG_VERY_BIG_RAM

#define CONFIG_DIMM_SLOTS_PER_CTLR	2
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

#if defined(CONFIG_SPD_EEPROM)
    /*
     * Determine DDR configuration from I2C interface.
     */
    #define SPD_EEPROM_ADDRESS1		0x51		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS2		0x52		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS3		0x53		/* DDR DIMM */
    #define SPD_EEPROM_ADDRESS4		0x54		/* DDR DIMM */

#else
    /*
     * Manually set up DDR1 & DDR2 parameters
     */

    #define CONFIG_SYS_SDRAM_SIZE	512		/* DDR is 512MB */

    #define CONFIG_SYS_DDR_CS0_BNDS	0x0000000F
    #define CONFIG_SYS_DDR_CS1_BNDS	0x00000000
    #define CONFIG_SYS_DDR_CS2_BNDS	0x00000000
    #define CONFIG_SYS_DDR_CS3_BNDS	0x00000000
    #define CONFIG_SYS_DDR_CS0_CONFIG	0x80010102
    #define CONFIG_SYS_DDR_CS1_CONFIG	0x00000000
    #define CONFIG_SYS_DDR_CS2_CONFIG	0x00000000
    #define CONFIG_SYS_DDR_CS3_CONFIG	0x00000000
    #define CONFIG_SYS_DDR_TIMING_3 0x00000000
    #define CONFIG_SYS_DDR_TIMING_0	0x00220802
    #define CONFIG_SYS_DDR_TIMING_1	0x38377322
    #define CONFIG_SYS_DDR_TIMING_2	0x002040c7
    #define CONFIG_SYS_DDR_CFG_1A	0x43008008
    #define CONFIG_SYS_DDR_CFG_2	0x24401000
    #define CONFIG_SYS_DDR_MODE_1	0x23c00542
    #define CONFIG_SYS_DDR_MODE_2	0x00000000
    #define CONFIG_SYS_DDR_MODE_CTL	0x00000000
    #define CONFIG_SYS_DDR_INTERVAL	0x05080100
    #define CONFIG_SYS_DDR_DATA_INIT	0x00000000
    #define CONFIG_SYS_DDR_CLK_CTRL	0x03800000
    #define CONFIG_SYS_DDR_CFG_1B	0xC3008008

    #define CONFIG_SYS_DDR2_CS0_BNDS	0x0010001F
    #define CONFIG_SYS_DDR2_CS1_BNDS	0x00000000
    #define CONFIG_SYS_DDR2_CS2_BNDS	0x00000000
    #define CONFIG_SYS_DDR2_CS3_BNDS	0x00000000
    #define CONFIG_SYS_DDR2_CS0_CONFIG	0x80010102
    #define CONFIG_SYS_DDR2_CS1_CONFIG	0x00000000
    #define CONFIG_SYS_DDR2_CS2_CONFIG	0x00000000
    #define CONFIG_SYS_DDR2_CS3_CONFIG	0x00000000
    #define CONFIG_SYS_DDR2_EXT_REFRESH 0x00000000
    #define CONFIG_SYS_DDR2_TIMING_0	0x00220802
    #define CONFIG_SYS_DDR2_TIMING_1	0x38377322
    #define CONFIG_SYS_DDR2_TIMING_2	0x002040c7
    #define CONFIG_SYS_DDR2_CFG_1A	0x43008008
    #define CONFIG_SYS_DDR2_CFG_2	0x24401000
    #define CONFIG_SYS_DDR2_MODE_1	0x23c00542
    #define CONFIG_SYS_DDR2_MODE_2	0x00000000
    #define CONFIG_SYS_DDR2_MODE_CTL	0x00000000
    #define CONFIG_SYS_DDR2_INTERVAL	0x05080100
    #define CONFIG_SYS_DDR2_DATA_INIT	0x00000000
    #define CONFIG_SYS_DDR2_CLK_CTRL	0x03800000
    #define CONFIG_SYS_DDR2_CFG_1B	0xC3008008

#endif

/* #define CONFIG_ID_EEPROM	1
#define ID_EEPROM_ADDR 0x57 */

/*
 * The SBC8641D contains 16MB flash space at ff000000.
 */
#define CONFIG_SYS_FLASH_BASE      0xff000000  /* start of FLASH 16M */

/* Flash */
#define CONFIG_SYS_BR0_PRELIM		0xff001001	/* port size 16bit */
#define CONFIG_SYS_OR0_PRELIM		0xff006e65	/* 16MB Boot Flash area */

/* 64KB EEPROM */
#define CONFIG_SYS_BR1_PRELIM		0xf0000801	/* port size 16bit */
#define CONFIG_SYS_OR1_PRELIM		0xffff6e65	/* 64K EEPROM area */

/* EPLD - User switches, board id, LEDs */
#define CONFIG_SYS_BR2_PRELIM		0xf1000801	/* port size 16bit */
#define CONFIG_SYS_OR2_PRELIM		0xfff06e65	/* EPLD (switches, board ID, LEDs) area */

/* Local bus SDRAM 128MB */
#define CONFIG_SYS_BR3_PRELIM		0xe0001861	/* port size ?bit */
#define CONFIG_SYS_OR3_PRELIM		0xfc006cc0	/* 128MB local bus SDRAM area (1st half) */
#define CONFIG_SYS_BR4_PRELIM		0xe4001861	/* port size ?bit */
#define CONFIG_SYS_OR4_PRELIM		0xfc006cc0	/* 128MB local bus SDRAM area (2nd half) */

/* Disk on Chip (DOC) 128MB */
#define CONFIG_SYS_BR5_PRELIM		0xe8001001	/* port size ?bit */
#define CONFIG_SYS_OR5_PRELIM		0xf8006e65	/* 128MB local bus SDRAM area (2nd half) */

/* LCD */
#define CONFIG_SYS_BR6_PRELIM		0xf4000801	/* port size ?bit */
#define CONFIG_SYS_OR6_PRELIM		0xfff06e65	/* 128MB local bus SDRAM area (2nd half) */

/* Control logic & misc peripherals */
#define CONFIG_SYS_BR7_PRELIM		0xf2000801	/* port size ?bit */
#define CONFIG_SYS_OR7_PRELIM		0xfff06e65	/* 128MB local bus SDRAM area (2nd half) */

#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	131		/* sectors per device */

#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#define CONFIG_SYS_MONITOR_BASE_EARLY   0xfff00000	/* early monitor loc */

#define CONFIG_SYS_WRITE_SWAPPED_DATA
#define CONFIG_SYS_FLASH_EMPTY_INFO

#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_SYS_INIT_RAM_LOCK	1
#ifndef CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0x0fd00000	/* Initial RAM address */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0xf8400000	/* Initial RAM address */
#endif
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)    /* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)   /* Reserved for malloc */

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE    1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2        (CONFIG_SYS_CCSRBAR+0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/*
 * RapidIO MMU
 */
#define CONFIG_SYS_SRIO1_MEM_BASE	0xc0000000	/* base address */
#define CONFIG_SYS_SRIO1_MEM_PHYS	CONFIG_SYS_SRIO1_MEM_BASE
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x20000000	/* 128M */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	CONFIG_SYS_PCIE1_MEM_BUS
#define CONFIG_SYS_PCIE1_MEM_VIRT	CONFIG_SYS_PCIE1_MEM_BUS
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_BUS		0xe2000000
#define CONFIG_SYS_PCIE1_IO_PHYS	CONFIG_SYS_PCIE1_IO_BUS
#define CONFIG_SYS_PCIE1_IO_VIRT	CONFIG_SYS_PCIE1_IO_BUS
#define CONFIG_SYS_PCIE1_IO_SIZE	0x1000000	/* 16M */

#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	CONFIG_SYS_PCIE2_MEM_BUS
#define CONFIG_SYS_PCIE2_MEM_VIRT	CONFIG_SYS_PCIE2_MEM_BUS
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCIE2_IO_BUS		0xe3000000
#define CONFIG_SYS_PCIE2_IO_PHYS	CONFIG_SYS_PCIE2_IO_BUS
#define CONFIG_SYS_PCIE2_IO_VIRT	CONFIG_SYS_PCIE2_IO_BUS
#define CONFIG_SYS_PCIE2_IO_SIZE	0x1000000	/* 16M */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
    #define PCI_ENET0_IOADDR	0xe0000000
    #define PCI_ENET0_MEMADDR	0xe0000000
    #define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_ULI5288
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	4
#define CONFIG_SYS_SCSI_MAX_LUN	1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * CONFIG_SYS_SCSI_MAX_LUN)
#endif

#endif	/* CONFIG_PCI */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_TSEC1    1
#define CONFIG_TSEC1_NAME       "eTSEC1"
#define CONFIG_TSEC2    1
#define CONFIG_TSEC2_NAME       "eTSEC2"
#define CONFIG_TSEC3    1
#define CONFIG_TSEC3_NAME       "eTSEC3"
#define CONFIG_TSEC4    1
#define CONFIG_TSEC4_NAME       "eTSEC4"

#define TSEC1_PHY_ADDR		0x1F
#define TSEC2_PHY_ADDR		0x00
#define TSEC3_PHY_ADDR		0x01
#define TSEC4_PHY_ADDR		0x02
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		TSEC_GIGABIT
#define TSEC4_FLAGS		TSEC_GIGABIT

#define CONFIG_SYS_TBIPA_VALUE	0x1e	/* Set TBI address not to conflict with TSEC1_PHY_ADDR */

#define CONFIG_ETHPRIME		"eTSEC1"

#endif	/* CONFIG_TSEC_ENET */

/*
 * BAT0         2G     Cacheable, non-guarded
 * 0x0000_0000  2G     DDR
 */
#define CONFIG_SYS_DBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT0U	(BATU_BL_2G | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT0L	(BATL_PP_RW | BATL_MEMCOHERENCE )
#define CONFIG_SYS_IBAT0U	CONFIG_SYS_DBAT0U

/*
 * BAT1         1G     Cache-inhibited, guarded
 * 0x8000_0000  512M   PCI-Express 1 Memory
 * 0xa000_0000  512M   PCI-Express 2 Memory
 *	Changed it for operating from 0xd0000000
 */
#define CONFIG_SYS_DBAT1L	( CONFIG_SYS_PCIE1_MEM_PHYS | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	(CONFIG_SYS_PCIE1_MEM_VIRT | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_PCIE1_MEM_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U	CONFIG_SYS_DBAT1U

/*
 * BAT2         512M   Cache-inhibited, guarded
 * 0xc000_0000  512M   RapidIO Memory
 */
#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_SRIO1_MEM_BASE | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	(CONFIG_SYS_SRIO1_MEM_BASE | BATU_BL_512M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_SRIO1_MEM_BASE | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	CONFIG_SYS_DBAT2U

/*
 * BAT3         4M     Cache-inhibited, guarded
 * 0xf800_0000  4M     CCSR
 */
#define CONFIG_SYS_DBAT3L	( CONFIG_SYS_CCSRBAR | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	(CONFIG_SYS_CCSRBAR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_CCSRBAR | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	CONFIG_SYS_DBAT3U

#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
#define CONFIG_SYS_CCSR_DEFAULT_DBATL (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATL_PP_RW | BATL_CACHEINHIBIT \
				       | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_CCSR_DEFAULT_DBATU (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_CCSR_DEFAULT_IBATL (CONFIG_SYS_CCSRBAR_DEFAULT \
				       | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_CCSR_DEFAULT_IBATU CONFIG_SYS_CCSR_DEFAULT_DBATU
#endif

/*
 * BAT4         32M    Cache-inhibited, guarded
 * 0xe200_0000  16M    PCI-Express 1 I/O
 * 0xe300_0000  16M    PCI-Express 2 I/0
 *    Note that this is at 0xe0000000
 */
#define CONFIG_SYS_DBAT4L	( CONFIG_SYS_PCIE1_IO_PHYS | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT4U	(CONFIG_SYS_PCIE1_IO_VIRT | BATU_BL_32M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_PCIE1_IO_PHYS | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT4U	CONFIG_SYS_DBAT4U

/*
 * BAT5         128K   Cacheable, non-guarded
 * 0xe401_0000  128K   Init RAM for stack in the CPU DCache (no backing memory)
 */
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT5U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT5L	CONFIG_SYS_DBAT5L
#define CONFIG_SYS_IBAT5U	CONFIG_SYS_DBAT5U

/*
 * BAT6         32M    Cache-inhibited, guarded
 * 0xfe00_0000  32M    FLASH
 */
#define CONFIG_SYS_DBAT6L	((CONFIG_SYS_FLASH_BASE & 0xfe000000) | BATL_PP_RW \
			| BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U	((CONFIG_SYS_FLASH_BASE & 0xfe000000) | BATU_BL_32M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT6L	((CONFIG_SYS_FLASH_BASE & 0xfe000000) | BATL_PP_RW | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	CONFIG_SYS_DBAT6U

/* Map the last 1M of flash where we're running from reset */
#define CONFIG_SYS_DBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT6U_EARLY	(CONFIG_SYS_TEXT_BASE | BATU_BL_1M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT6L_EARLY	(CONFIG_SYS_MONITOR_BASE_EARLY | BATL_PP_RW \
				 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U_EARLY	CONFIG_SYS_DBAT6U_EARLY

#define CONFIG_SYS_DBAT7L	0x00000000
#define CONFIG_SYS_DBAT7U	0x00000000
#define CONFIG_SYS_IBAT7L	0x00000000
#define CONFIG_SYS_IBAT7U	0x00000000

/*
 * Environment
 */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128k(one sector) for env */
#define CONFIG_ENV_SIZE		0x2000

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

/*
 * Environment Configuration
 */

#define CONFIG_HAS_ETH0		1
#define CONFIG_HAS_ETH1		1
#define CONFIG_HAS_ETH2		1
#define CONFIG_HAS_ETH3		1

#define CONFIG_IPADDR		192.168.0.50

#define CONFIG_HOSTNAME		"sbc8641d"
#define CONFIG_ROOTPATH		"/opt/eldk/ppc_74xx"
#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_NETMASK		255.255.255.0

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define	CONFIG_EXTRA_ENV_SETTINGS					\
   "netdev=eth0\0"							\
   "consoledev=ttyS0\0"							\
   "ramdiskaddr=2000000\0"						\
   "ramdiskfile=uRamdisk\0"						\
   "dtbaddr=400000\0"							\
   "dtbfile=sbc8641d.dtb\0"						\
   "en-wd=mw.b f8100010 0x08; echo -expect:- 08; md.b f8100010 1\0"	\
   "dis-wd=mw.b f8100010 0x00; echo -expect:- 00; md.b f8100010 1\0"	\
   "maxcpus=1"

#define CONFIG_NFSBOOTCOMMAND						\
   "setenv bootargs root=/dev/nfs rw "					\
      "nfsroot=$serverip:$rootpath "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr - $dtbaddr"

#define CONFIG_RAMBOOTCOMMAND						\
   "setenv bootargs root=/dev/ram rw "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "tftp $ramdiskaddr $ramdiskfile;"					\
   "tftp $loadaddr $bootfile;"						\
   "tftp $dtbaddr $dtbfile;"						\
   "bootm $loadaddr $ramdiskaddr $dtbaddr"

#define CONFIG_FLASHBOOTCOMMAND						\
   "setenv bootargs root=/dev/ram rw "					\
      "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "	\
      "console=$consoledev,$baudrate $othbootargs;"			\
   "bootm ffd00000 ffb00000 ffa00000"

#define CONFIG_BOOTCOMMAND  CONFIG_FLASHBOOTCOMMAND

#endif	/* __CONFIG_H */
