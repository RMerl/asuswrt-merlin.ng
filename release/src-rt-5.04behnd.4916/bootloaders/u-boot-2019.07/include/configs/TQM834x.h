/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * TQM8349 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */

/* board pre init: do not call, nothing to do */

/* detect the number of flash banks */

/*
 * DDR Setup
 */
				/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE	0x00000000
#define DDR_CASLAT_25		/* CASLAT set to 2.5 */
#undef CONFIG_DDR_ECC		/* only for ECC DDR module */
#undef CONFIG_SPD_EEPROM	/* do not use SPD EEPROM for DDR setup */

#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * FLASH on the Local Bus
 */
#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_BASE		0x80000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8		/* FLASH size in MB */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* print 'E' for empty sectors */

/*
 * FLASH bank number detection
 */

/*
 * When CONFIG_SYS_MAX_FLASH_BANKS_DETECT is defined, the actual number of
 * Flash banks has to be determined at runtime and stored in a gloabl variable
 * tqm834x_num_flash_banks. The value of CONFIG_SYS_MAX_FLASH_BANKS_DETECT is
 * only used instead of CONFIG_SYS_MAX_FLASH_BANKS to allocate the array
 * flash_info, and should be made sufficiently large to accomodate the number
 * of banks that might actually be detected.  Since most (all?) Flash related
 * functions use CONFIG_SYS_MAX_FLASH_BANKS as the number of actual banks on
 * the board, it is defined as tqm834x_num_flash_banks.
 */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	2

#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max sectors per device */


/* disable remaining mappings */
#define CONFIG_SYS_BR1_PRELIM		0x00000000
#define CONFIG_SYS_OR1_PRELIM		0x00000000

#define CONFIG_SYS_BR2_PRELIM		0x00000000
#define CONFIG_SYS_OR2_PRELIM		0x00000000

#define CONFIG_SYS_BR3_PRELIM		0x00000000
#define CONFIG_SYS_OR3_PRELIM		0x00000000

/*
 * Monitor config
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
# define CONFIG_SYS_RAMBOOT
#else
# undef  CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0x20000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

				/* Reserve 384 kB = 3 sect. for Mon */
#define CONFIG_SYS_MONITOR_LEN	(384 * 1024)
				/* Reserve 512 kB for malloc */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/*
 * I2C
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000

/* I2C EEPROM, configuration for onboard EEPROMs 24C256 and 24C32 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2	/* 16 bit */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* 32 bytes/write */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	12	/* 10ms +/- 20% */

/* I2C RTC */
#define CONFIG_RTC_DS1337			/* use ds1337 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR		0x68	/* at address 0x68 */

/*
 * TSEC
 */

#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC2_OFFSET)

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		2
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

/* PCI1 host bridge */
#define CONFIG_SYS_PCI1_MEM_BASE	0x90000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	\
			(CONFIG_SYS_PCI1_MEM_BASE + CONFIG_SYS_PCI1_MEM_SIZE)
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0xe2000000
#define CONFIG_SYS_PCI1_IO_PHYS		CONFIG_SYS_PCI1_IO_BASE
#define CONFIG_SYS_PCI1_IO_SIZE		0x1000000	/* 16M */

#undef CONFIG_EEPRO100
#define CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	CONFIG_SYS_PCI1_IO_BASE
	#define PCI_ENET0_MEMADDR	CONFIG_SYS_PCI1_MEM_BASE
	#define PCI_IDSEL_NUMBER	0x1c    /* slot0 (IDSEL) = 28 */
#endif

#define CONFIG_SYS_PCI_SUBSYS_VENDORID		0x1957  /* Freescale */

#endif	/* CONFIG_PCI */

/*
 * Environment
 */
#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K (one sector) for env */
#define CONFIG_ENV_SIZE		0x8000	/*  32K max size */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO		1 /* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1 /* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

/* System IO Config */
#define CONFIG_SYS_SICRH	0
#define CONFIG_SYS_SICRL	SICRL_LDP_A

/* PCI */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		400000

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=tqm834x\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"flash_nfs_old=run nfsargs addip addcons;"			\
		"bootm ${kernel_addr}\0"				\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"flash_self_old=run ramargs addip addcons;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_self=run ramargs addip addcons;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"net_nfs_old=tftp 400000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addcons; "				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=tqm834x/uImage\0"					\
	"fdtfile=tqm834x/tqm834x.dtb\0"					\
	"kernel_addr_r=400000\0"					\
	"fdt_addr_r=600000\0"						\
	"ramdisk_addr_r=800000\0"					\
	"kernel_addr=800C0000\0"					\
	"fdt_addr=800A0000\0"						\
	"ramdisk_addr=80300000\0"					\
	"u-boot=tqm834x/u-boot.bin\0"					\
	"load=tftp 200000 ${u-boot}\0"					\
	"update=protect off 80000000 +${filesize};"			\
		"era 80000000 +${filesize};"				\
		"cp.b 200000 80000000 ${filesize}\0"			\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * JFFS2 partitions
 */
/* mtdparts command line support */

/* default mtd partition table */
#endif	/* __CONFIG_H */
