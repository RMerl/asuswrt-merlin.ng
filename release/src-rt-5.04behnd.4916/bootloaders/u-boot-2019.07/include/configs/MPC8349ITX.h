/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 */

/*
 MPC8349E-mITX and MPC8349E-mITX-GP board configuration file

 Memory map:

 0x0000_0000-0x0FFF_FFFF DDR SDRAM (256 MB)
 0x8000_0000-0x9FFF_FFFF PCI1 memory space (512 MB)
 0xA000_0000-0xBFFF_FFFF PCI2 memory space (512 MB)
 0xE000_0000-0xEFFF_FFFF IMMR (1 MB)
 0xE200_0000-0xE2FF_FFFF PCI1 I/O space (16 MB)
 0xE300_0000-0xE3FF_FFFF PCI2 I/O space (16 MB)
 0xF000_0000-0xF000_FFFF Compact Flash (MPC8349E-mITX only)
 0xF001_0000-0xF001_FFFF Local bus expansion slot
 0xF800_0000-0xF801_FFFF Vitesse 7385 Parallel Interface (MPC8349E-mITX only)
 0xFE00_0000-0xFE7F_FFFF First 8MB bank of Flash memory
 0xFE80_0000-0xFEFF_FFFF Second 8MB bank of Flash memory (MPC8349E-mITX only)

 I2C address list:
						Align.	Board
 Bus	Addr	Part No.	Description	Length	Location
 ----------------------------------------------------------------
 I2C0	0x50	M24256-BWMN6P	Board EEPROM	2	U64

 I2C1	0x20	PCF8574		I2C Expander	0	U8
 I2C1	0x21	PCF8574		I2C Expander	0	U10
 I2C1	0x38	PCF8574A	I2C Expander	0	U8
 I2C1	0x39	PCF8574A	I2C Expander	0	U10
 I2C1	0x51	(DDR)		DDR EEPROM	1	U1
 I2C1	0x68	DS1339		RTC		1	U68

 Note that a given board has *either* a pair of 8574s or a pair of 8574As.
*/

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MISC_INIT_F

/*
 * On-board devices
 */

#ifdef CONFIG_TARGET_MPC8349ITX
/* The CF card interface on the back of the board */
#define CONFIG_COMPACT_FLASH
#define CONFIG_VSC7385_ENET	/* VSC7385 ethernet support */
#define CONFIG_SYS_USB_HOST	/* use the EHCI USB controller */
#endif

#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C

/*
 * Device configurations
 */

/* I2C */
#ifdef CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

#define CONFIG_SYS_SPD_BUS_NUM		1	/* The I2C bus for SPD */
#define CONFIG_SYS_RTC_BUS_NUM		1	/* The I2C bus for RTC */

#define CONFIG_SYS_I2C_8574_ADDR1	0x20	/* I2C1, PCF8574 */
#define CONFIG_SYS_I2C_8574_ADDR2	0x21	/* I2C1, PCF8574 */
#define CONFIG_SYS_I2C_8574A_ADDR1	0x38	/* I2C1, PCF8574A */
#define CONFIG_SYS_I2C_8574A_ADDR2	0x39	/* I2C1, PCF8574A */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* I2C0, Board EEPROM */
#define CONFIG_SYS_I2C_RTC_ADDR		0x68	/* I2C1, DS1339 RTC*/
#define SPD_EEPROM_ADDRESS		0x51	/* I2C1, DDR */

/* Don't probe these addresses: */
#define CONFIG_SYS_I2C_NOPROBES	{ {1, CONFIG_SYS_I2C_8574_ADDR1}, \
				 {1, CONFIG_SYS_I2C_8574_ADDR2}, \
				 {1, CONFIG_SYS_I2C_8574A_ADDR1}, \
				 {1, CONFIG_SYS_I2C_8574A_ADDR2} }
/* Bit definitions for the 8574[A] I2C expander */
				/* Board revision, 00=0.0, 01=0.1, 10=1.0 */
#define I2C_8574_REVISION	0x03
#define I2C_8574_CF		0x08	/* 1=Compact flash absent, 0=present */
#define I2C_8574_MPCICLKRN	0x10	/* MiniPCI Clk Run */
#define I2C_8574_PCI66		0x20	/* 0=33MHz PCI, 1=66MHz PCI */
#define I2C_8574_FLASHSIDE	0x40	/* 0=Reset vector from U4, 1=from U7*/

#endif

/* Compact Flash */
#ifdef CONFIG_COMPACT_FLASH

#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	1

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_CF_BASE
#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0200
#define CONFIG_SYS_ATA_STRIDE		2

/* If a CF card is not inserted, time out quickly */
#define ATA_RESET_TIME	1

#endif

/*
 * SATA
 */
#ifdef CONFIG_SATA_SIL3114

#define CONFIG_SYS_SATA_MAX_DEVICE      4
#define CONFIG_LBA48

#endif

#ifdef CONFIG_SYS_USB_HOST
/*
 * Support USB
 */
#define CONFIG_USB_EHCI_FSL

/* Current USB implementation supports the only USB controller,
 * so we have to choose between the MPH or the DR ones */
#if 1
#define CONFIG_HAS_FSL_MPH_USB
#else
#define CONFIG_HAS_FSL_DR_USB
#endif

#endif

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_83XX_DDR_USES_CS0
#define CONFIG_SYS_MEMTEST_START	0x1000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x2000

#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN \
					| DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)

#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED   ((phys_size_t)256 << 20)

#ifdef CONFIG_SYS_I2C
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/
#endif

/* No SPD? Then manually set up DDR parameters */
#ifndef CONFIG_SPD_EEPROM
    #define CONFIG_SYS_DDR_SIZE		256	/* Mb */
    #define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)

    #define CONFIG_SYS_DDR_TIMING_1	0x26242321
    #define CONFIG_SYS_DDR_TIMING_2	0x00000800  /* P9-45, may need tuning */
#endif

/*
 *Flash on the Local Bus
 */

#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_EMPTY_INFO
/* 127 64KB sectors + 8 8KB sectors per device */
#define CONFIG_SYS_MAX_FLASH_SECT	135
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

/* The ITX has two flash chips, but the ITX-GP has only one.  To support both
boards, we say we have two, but don't display a message if we find only one. */
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	\
		{CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE + 0x800000}
#define CONFIG_SYS_FLASH_SIZE		16	/* FLASH size in MB */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_TSEC2

/* The flash address and size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE		0xFEFFE000
#define CONFIG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * BRx, ORx, LBLAWBARx, and LBLAWARx
 */


/* Vitesse 7385 */

#define CONFIG_SYS_VSC7385_BASE	0xF8000000

#define CONFIG_SYS_LED_BASE	0xF9000000


/* Compact Flash */

#ifdef CONFIG_COMPACT_FLASH

#define CONFIG_SYS_CF_BASE	0xF0000000


#endif

/*
 * U-Boot memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(512 * 1024) /* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024) /* Reserved for malloc */

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONSOLE			ttyS0

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/*
 * PCI
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_MPC83XX_PCI2

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	\
			(CONFIG_SYS_PCI1_MEM_BASE + CONFIG_SYS_PCI1_MEM_SIZE)
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x01000000	/* 16M */

#ifdef CONFIG_MPC83XX_PCI2
#define CONFIG_SYS_PCI2_MEM_BASE	\
			(CONFIG_SYS_PCI1_MMIO_BASE + CONFIG_SYS_PCI1_MMIO_SIZE)
#define CONFIG_SYS_PCI2_MEM_PHYS	CONFIG_SYS_PCI2_MEM_BASE
#define CONFIG_SYS_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_MMIO_BASE	\
			(CONFIG_SYS_PCI2_MEM_BASE + CONFIG_SYS_PCI2_MEM_SIZE)
#define CONFIG_SYS_PCI2_MMIO_PHYS	CONFIG_SYS_PCI2_MMIO_BASE
#define CONFIG_SYS_PCI2_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_IO_BASE		0x00000000
#define CONFIG_SYS_PCI2_IO_PHYS		\
			(CONFIG_SYS_PCI1_IO_PHYS + CONFIG_SYS_PCI1_IO_SIZE)
#define CONFIG_SYS_PCI2_IO_SIZE		0x01000000	/* 16M */
#endif

#ifndef CONFIG_PCI_PNP
    #define PCI_ENET0_IOADDR	0x00000000
    #define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI2_MEM_BASE
    #define PCI_IDSEL_NUMBER	0x0f	/* IDSEL = AD15 */
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif

/* TSEC */

#ifdef CONFIG_TSEC_ENET
#define CONFIG_TSEC1

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME  "TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c	/* VSC8201 uses address 0x1c */
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME  "TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000

#define TSEC2_PHY_ADDR		4
#define TSEC2_PHYIDX		0
#define TSEC2_FLAGS		TSEC_GIGABIT
#endif

#define CONFIG_ETHPRIME		"Freescale TSEC"

#endif

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE

#ifndef CONFIG_SYS_RAMBOOT
  #define CONFIG_ENV_ADDR	\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
  #define CONFIG_ENV_SECT_SIZE	0x10000 /* 64K (one sector) for environment */
  #define CONFIG_ENV_SIZE	0x2000
#else
  #define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - 0x1000)
  #define CONFIG_ENV_SIZE	0x2000
#endif

#define CONFIG_LOADS_ECHO	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Watchdog */
#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */

#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * System performance
 */
#define CONFIG_SYS_SCCR_TSEC1CM	1	/* TSEC1 clock mode (0-3) */
#define CONFIG_SYS_SCCR_TSEC2CM	1	/* TSEC2 & I2C0 clock mode (0-3) */
#define CONFIG_SYS_SCCR_USBMPHCM 3	/* USB MPH controller's clock */
#define CONFIG_SYS_SCCR_USBDRCM	0	/* USB DR controller's clock */

/*
 * System IO Config
 */
/* Needed for gigabit to work on TSEC 1 */
#define CONFIG_SYS_SICRH SICRH_TSOBI1
				/* USB DR as device + USB MPH as host */
#define CONFIG_SYS_SICRL	(SICRL_LDP_A | SICRL_USB1)

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_NETDEV		"eth0"

/* Default path and filenames */
#define CONFIG_ROOTPATH		"/nfsroot/rootfs"
#define CONFIG_BOOTFILE		"uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot.bin"

#ifdef CONFIG_TARGET_MPC8349ITX
#define CONFIG_FDTFILE		"mpc8349emitx.dtb"
#else
#define CONFIG_FDTFILE		"mpc8349emitxgp.dtb"
#endif


#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=" __stringify(CONSOLE) "\0"			\
	"netdev=" CONFIG_NETDEV "\0"					\
	"uboot=" CONFIG_UBOOTPATH "\0"					\
	"tftpflash=tftpboot $loadaddr $uboot; "				\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" +$filesize; "	\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize; "	\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize\0"	\
	"fdtaddr=780000\0"						\
	"fdtfile=" CONFIG_FDTFILE "\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath"	\
	" ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off "\
	" console=$console,$baudrate $othbootargs; "			\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw"				\
	" console=$console,$baudrate $othbootargs; "			\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#endif
