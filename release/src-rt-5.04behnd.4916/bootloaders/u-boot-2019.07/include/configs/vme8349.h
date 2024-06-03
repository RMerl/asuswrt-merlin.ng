/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * esd vme8349 U-Boot configuration file
 * Copyright (c) 2008, 2009 esd gmbh Hannover Germany
 *
 * (C) Copyright 2006-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * reinhard.arlt@esd-electronics.de
 * Based on the MPC8349EMDS config.
 */

/*
 * vme8349 board configuration file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */

/* Don't enable PCI2 on vme834x - it doesn't exist physically. */
#undef CONFIG_MPC83XX_PCI2		/* support for 2nd PCI controller */

#undef CONFIG_SYS_DRAM_TEST			/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * DDR Setup
 */
#define CONFIG_DDR_ECC			/* only for ECC DDR module */
#define CONFIG_DDR_ECC_CMD		/* use DDR ECC user commands */
#define CONFIG_SPD_EEPROM
#define SPD_EEPROM_ADDRESS		0x54
#define CONFIG_SYS_READ_SPD		vme8349_read_spd
#define CONFIG_SYS_83XX_DDR_USES_CS0	/* esd; Fsl board uses CS2/CS3 */

/*
 * 32-bit data path mode.
 *
 * Please note that using this mode for devices with the real density of 64-bit
 * effectively reduces the amount of available memory due to the effect of
 * wrapping around while translating address to row/columns, for example in the
 * 256MB module the upper 128MB get aliased with contents of the lower
 * 128MB); normally this define should be used for devices with real 32-bit
 * data path.
 */
#undef CONFIG_DDR_32BIT

#define CONFIG_SYS_SDRAM_BASE		0x00000000	/* DDR is sys memory*/
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN \
					| DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)
#define CONFIG_DDR_2T_TIMING
#define CONFIG_SYS_DDRCDR		(DDRCDR_DHC_EN \
					| DDRCDR_ODT \
					| DDRCDR_Q_DRN)
					/* 0x80080001 */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xf8000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		128		/* flash size in MB */


#define CONFIG_SYS_WINDOW1_BASE		0xf0000000


#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device*/

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase TO (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write TO (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xF7000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE		0x1000		/* size */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB */
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)	/* Malloc size */

#undef CONFIG_SYS_LB_SDRAM	/* if board has SDRAM on local bus */

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1		(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2		(CONFIG_SYS_IMMR + 0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }
/* could also use CONFIG_I2C_MULTI_BUS and CONFIG_SYS_SPD_BUS_NUM... */

#define CONFIG_SYS_I2C_8574_ADDR2       0x20    /* I2C1, PCF8574 */

/* TSEC */
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET 0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR + CONFIG_SYS_TSEC2_OFFSET)

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI1_IO_BASE		0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS		0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE		0x00100000	/* 1M */

#define CONFIG_SYS_PCI2_MEM_BASE	0xA0000000
#define CONFIG_SYS_PCI2_MEM_PHYS	CONFIG_SYS_PCI2_MEM_BASE
#define CONFIG_SYS_PCI2_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_MMIO_BASE	0xB0000000
#define CONFIG_SYS_PCI2_MMIO_PHYS	CONFIG_SYS_PCI2_MMIO_BASE
#define CONFIG_SYS_PCI2_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI2_IO_BASE		0x00000000
#define CONFIG_SYS_PCI2_IO_PHYS		0xE2100000
#define CONFIG_SYS_PCI2_IO_SIZE		0x00100000	/* 1M */

#if defined(CONFIG_PCI)

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	0xFIXME
	#define PCI_ENET0_MEMADDR	0xFIXME
	#define PCI_IDSEL_NUMBER	0xFIXME
#endif

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID	0x1957	/* Freescale */

#endif	/* CONFIG_PCI */

/*
 * TSEC configuration
 */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_GMII			/* MII PHY management */
#define CONFIG_TSEC1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_PHY_M88E1111
#define TSEC1_PHY_ADDR		0x08
#define TSEC2_PHY_ADDR		0x10
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0xc0000)
	#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#else
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */
#define CONFIG_SYS_RTC_BUS_NUM  0x01
#define CONFIG_SYS_I2C_RTC_ADDR	0x32
#define CONFIG_RTC_RX8025

/* Pass Ethernet MAC to VxWorks */
#define CONFIG_SYS_VXWORKS_MAC_PTR	0x000043f0

#undef CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)	/* Init Memory map for Linux*/

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000 /* PCIHOST  */

/* System IO Config */
#define CONFIG_SYS_SICRH 0
#define CONFIG_SYS_SICRL SICRL_LDP_A

#define CONFIG_SYS_GPIO1_PRELIM
#define CONFIG_SYS_GPIO1_DIR	0x00100000
#define CONFIG_SYS_GPIO1_DAT	0x00100000

#define CONFIG_SYS_GPIO2_PRELIM
#define CONFIG_SYS_GPIO2_DIR	0x78900000
#define CONFIG_SYS_GPIO2_DAT	0x70100000

#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_HOSTNAME		"VME8349"
#define CONFIG_ROOTPATH		"/tftpboot/rootfs"
#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_LOADADDR		800000	/* def location for tftp and bootm */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=vme8349\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"	\
		"bootm\0"						\
	"load=tftp 100000 /tftpboot/bdi2000/vme8349.bin\0"		\
	"update=protect off fff00000 fff3ffff; "			\
		"era fff00000 fff3ffff; cp.b 100000 fff00000 ${filesize}\0" \
	"upd=run load update\0"						\
	"fdtaddr=780000\0"						\
	"fdtfile=vme8349.dtb\0"						\
	""

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
		"nfsroot=$serverip:$rootpath "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv bootargs root=/dev/ram rw "				\
		"console=$consoledev,$baudrate $othbootargs;"		\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND	"run flash_self"

#ifndef __ASSEMBLY__
int vme8349_read_spd(unsigned char chip, unsigned int addr, int alen,
		     unsigned char *buffer, int len);
#endif

#endif	/* __CONFIG_H */
