/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * WindRiver SBC8349 U-Boot configuration file.
 * Copyright (c) 2006, 2007 Wind River Systems, Inc.
 *
 * Paul Gortmaker <paul.gortmaker@windriver.com>
 * Based on the MPC8349EMDS config.
 */

/*
 * sbc8349 board configuration file.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */

/* Don't enable PCI2 on sbc834x - it doesn't exist physically. */
#undef CONFIG_MPC83XX_PCI2		/* support for 2nd PCI controller */

#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * DDR Setup
 */
#undef CONFIG_DDR_ECC			/* only for ECC DDR module */
#undef CONFIG_DDR_ECC_CMD		/* use DDR ECC user commands */
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/
#define CONFIG_SYS_83XX_DDR_USES_CS0	/* WRS; Fsl board uses CS2/CS3 */

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

#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075)
#define CONFIG_DDR_2T_TIMING

#if defined(CONFIG_SPD_EEPROM)
/*
 * Determine DDR configuration from I2C interface.
 */
#define SPD_EEPROM_ADDRESS	0x52		/* DDR DIMM */

#else
/*
 * Manually set up DDR parameters
 * NB: manual DDR setup untested on sbc834x
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_CS2_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)
#define CONFIG_SYS_DDR_TIMING_1	0x36332321
#define CONFIG_SYS_DDR_TIMING_2	0x00000800	/* P9-45,may need tuning */
#define CONFIG_SYS_DDR_CONTROL	0xc2000000	/* unbuffered,no DYN_PWR */
#define CONFIG_SYS_DDR_INTERVAL	0x04060100	/* autocharge,no open page */

#if defined(CONFIG_DDR_32BIT)
/* set burst length to 8 for 32-bit data path */
				/* DLL,normal,seq,4/2.5, 8 burst len */
#define CONFIG_SYS_DDR_MODE	0x00000023
#else
/* the default burst length is 4 - for 64-bit data path */
				/* DLL,normal,seq,4/2.5, 4 burst len */
#define CONFIG_SYS_DDR_MODE	0x00000022
#endif
#endif

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xF0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xFF800000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8		/* flash size in MB */


#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
					/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000
					/* Size of used area in RAM*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)	/* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)	/* Reserved for malloc */

#undef CONFIG_SYS_LB_SDRAM	/* if board has SDRAM on local bus */

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE    1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2        (CONFIG_SYS_IMMR+0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69}, {1, 0x69} }
/* could also use CONFIG_I2C_MULTI_BUS and CONFIG_SYS_SPD_BUS_NUM... */

/* TSEC */
#define CONFIG_SYS_TSEC1_OFFSET 0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET 0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

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

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057  /* Motorola */

#endif	/* CONFIG_PCI */

/*
 * TSEC configuration
 */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_PHY_BCM5421S	1
#define TSEC1_PHY_ADDR		0x19
#define TSEC2_PHY_ADDR		0x1a
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
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + 0x40000)
	#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#else
	#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
	#define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

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
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000 /* PCIHOST  */

/* System IO Config */
#define CONFIG_SYS_SICRH 0
#define CONFIG_SYS_SICRL SICRL_LDP_A

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

#define CONFIG_HOSTNAME		"SBC8349"
#define CONFIG_ROOTPATH		"/tftpboot/rootfs"
#define CONFIG_BOOTFILE		"uImage"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		800000

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=sbc8349\0"						\
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
	"load=tftp 100000 /tftpboot/sbc8349/u-boot.bin\0"		\
	"update=protect off ff800000 ff83ffff; "			\
		"era ff800000 ff83ffff; cp.b 100000 ff800000 ${filesize}\0" \
	"upd=run load update\0"						\
	"fdtaddr=780000\0"						\
	"fdtfile=sbc8349.dtb\0"						\
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

#endif	/* __CONFIG_H */
