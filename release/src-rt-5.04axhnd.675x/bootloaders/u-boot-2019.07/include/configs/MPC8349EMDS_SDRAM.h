/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * mpc8349emds board configuration file
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */

#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000      /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * DDR Setup
 */
#define CONFIG_DDR_ECC			/* support DDR ECC function */
#define CONFIG_DDR_ECC_CMD		/* use DDR ECC user commands */
#define CONFIG_SPD_EEPROM		/* use SPD EEPROM for DDR setup*/

/*
 * SYS_FSL_DDR2 is selected in Kconfig to use unified DDR driver
 * unselect it to use old spd_sdram.c
 */
#define CONFIG_SYS_SPD_BUS_NUM	0
#define SPD_EEPROM_ADDRESS1	0x52
#define SPD_EEPROM_ADDRESS2	0x51
#define CONFIG_DIMM_SLOTS_PER_CTLR	2
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

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

#define CONFIG_SYS_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN \
					| DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)
#undef  CONFIG_DDR_2T_TIMING

/*
 * DDRCDR - DDR Control Driver Register
 */
#define CONFIG_SYS_DDRCDR_VALUE	0x80080001

#if defined(CONFIG_SPD_EEPROM)
/*
 * Determine DDR configuration from I2C interface.
 */
#define SPD_EEPROM_ADDRESS	0x51		/* DDR DIMM */
#else
/*
 * Manually set up DDR parameters
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#if defined(CONFIG_DDR_II)
#define CONFIG_SYS_DDRCDR		0x80080001
#define CONFIG_SYS_DDR_CS2_BNDS		0x0000000f
#define CONFIG_SYS_DDR_CS2_CONFIG	0x80330102
#define CONFIG_SYS_DDR_TIMING_0		0x00220802
#define CONFIG_SYS_DDR_TIMING_1		0x38357322
#define CONFIG_SYS_DDR_TIMING_2		0x2f9048c8
#define CONFIG_SYS_DDR_TIMING_3		0x00000000
#define CONFIG_SYS_DDR_CLK_CNTL		0x02000000
#define CONFIG_SYS_DDR_MODE		0x47d00432
#define CONFIG_SYS_DDR_MODE2		0x8000c000
#define CONFIG_SYS_DDR_INTERVAL		0x03cf0080
#define CONFIG_SYS_DDR_SDRAM_CFG	0x43000000
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#else
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
#endif

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xF0000000	/* Localbus SDRAM */
#define CONFIG_SYS_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		32	/* max flash size in MB */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef  CONFIG_SYS_RAMBOOT
#endif

/*
 * BCSR register on local bus 32KB, 8-bit wide for MDS config reg
 */
#define CONFIG_SYS_BCSR			0xE2400000
					/* Access window base at BCSR base */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(512 * 1024)	/* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)	/* Reserved for malloc */

/*
 * The MPC834xEA MDS for 834xE rev3.1 may not be assembled SDRAM memory.
 */

/* Local bus BR2, OR2 definition for SDRAM if soldered on the MDS board */
/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CONFIG_SYS_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = F0001861
 */

/*
 * The SDRAM size in MB, CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR2, need:
 *    64MB mask for AM, OR2[0:7] = 1111 1100
 *                 XAM, OR2[17:18] = 11
 *    9 columns OR2[19-21] = 010
 *    13 rows   OR2[23-25] = 100
 *    EAD set for extra time OR[31] = 1
 *
 * 0    4    8    12   16   20   24   28
 * 1111 1100 0000 0000 0110 1001 0000 0001 = FC006901
 */


				/* LB sdram refresh timer, about 6us */
#define CONFIG_SYS_LBC_LSRT	0x32000000
				/* LB refresh timer prescal, 266MHz/32 */
#define CONFIG_SYS_LBC_MRTPR	0x20000000

#define CONFIG_SYS_LBC_LSDMR_COMMON    (LSDMR_RFEN	\
				| LSDMR_BSMA1516	\
				| LSDMR_RFCR8		\
				| LSDMR_PRETOACT6	\
				| LSDMR_ACTTORW3	\
				| LSDMR_BL8		\
				| LSDMR_WRC3		\
				| LSDMR_CL3)

/*
 * SDRAM Controller configuration sequence.
 */
#define CONFIG_SYS_LBC_LSDMR_1	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_PCHALL)
#define CONFIG_SYS_LBC_LSDMR_2	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_3	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_ARFRSH)
#define CONFIG_SYS_LBC_LSDMR_4	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_MRW)
#define CONFIG_SYS_LBC_LSDMR_5	(CONFIG_SYS_LBC_LSDMR_COMMON | LSDMR_OP_NORMAL)

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
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/* SPI */
#undef CONFIG_SOFT_SPI			/* SPI bit-banged */

/* GPIOs.  Used as SPI chip selects */
#define CONFIG_SYS_GPIO1_PRELIM
#define CONFIG_SYS_GPIO1_DIR		0xC0000000  /* SPI CS on 0, LED on 1 */
#define CONFIG_SYS_GPIO1_DAT		0xC0000000  /* Both are active LOW */

/* TSEC */
#define CONFIG_SYS_TSEC1_OFFSET 0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET 0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

/* USB */
#define CONFIG_SYS_USE_MPC834XSYS_USB_PHY	1 /* Use SYS board PHY */

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

#define CONFIG_83XX_PCI_STREAMING

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP

#if !defined(CONFIG_PCI_PNP)
	#define PCI_ENET0_IOADDR	0xFIXME
	#define PCI_ENET0_MEMADDR	0xFIXME
	#define PCI_IDSEL_NUMBER	0x0c	/* slot0->3(IDSEL)=12->15 */
#endif

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957  /* Freescale */

#endif	/* CONFIG_PCI */

/*
 * TSEC configuration
 */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_GMII		1	/* MII PHY management */
#define CONFIG_TSEC1		1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC2		1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME		"TSEC0"

#endif	/* CONFIG_TSEC_ENET */

/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_DS1374		/* use ds1374 rtc via i2c */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68	/* at address 0x68 */

/*
 * Environment
 */
#ifndef CONFIG_SYS_RAMBOOT
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
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
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000 /* PCIHOST  */

/*
 * System performance
 */
#define CONFIG_SYS_SPCR_TSEC1EP	3	/* TSEC1 emergency priority (0-3) */
#define CONFIG_SYS_SPCR_TSEC2EP	3	/* TSEC2 emergency priority (0-3) */
#define CONFIG_SYS_SCCR_TSEC1CM	1	/* TSEC1 clock mode (0-3) */
#define CONFIG_SYS_SCCR_TSEC2CM	1	/* TSEC2 & I2C0 clock mode (0-3) */

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
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH0
#endif

#define CONFIG_HOSTNAME		"mpc8349emds"
#define CONFIG_ROOTPATH		"/nfsroot/rootfs"
#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=mpc8349emds\0"					\
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
	"load=tftp 100000 /tftpboot/mpc8349emds/u-boot.bin\0"		\
	"update=protect off fe000000 fe03ffff; "			\
		"era fe000000 fe03ffff; cp.b 100000 fe000000 ${filesize}\0"\
	"upd=run load update\0"						\
	"fdtaddr=780000\0"						\
	"fdtfile=mpc834x_mds.dtb\0"					\
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
