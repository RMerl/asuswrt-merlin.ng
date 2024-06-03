/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009-2010 Freescale Semiconductor, Inc.
 * Copyright (C) 2010 Ilya Yanok, Emcraft Systems, yanok@emcraft.com
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */

/*
 * On-board devices
 *
 * TSECs
 */
#define CONFIG_TSEC1
#define CONFIG_TSEC2

#define CONFIG_SYS_GPIO1_PRELIM
/* GPIO Default input/output settings */
#define CONFIG_SYS_GPIO1_DIR        0x7AAF8C00
/*
 * Default GPIO values:
 * LED#1 enabled; WLAN enabled; Both COM LED on (orange)
 */
#define CONFIG_SYS_GPIO1_DAT        0x08008C00

/*
 * SERDES
 */
#define CONFIG_FSL_SERDES
#define CONFIG_FSL_SERDES1	0xe3000

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN \
				| DDRCDR_PZ_LOZ \
				| DDRCDR_NZ_LOZ \
				| DDRCDR_ODT \
				| DDRCDR_Q_DRN)
				/* 0x7b880001 */
/*
 * Manually set up DDR parameters
 * consist of two chips HY5PS12621BFP-C4 from HYNIX
 */

#define CONFIG_SYS_DDR_SIZE		128 /* MB */

#define CONFIG_SYS_DDR_CS0_BNDS	0x00000007
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ODT_RD_NEVER \
					| CSCONFIG_ODT_WR_ONLY_CURRENT \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)
					/* 0x80010102 */
#define CONFIG_SYS_DDR_TIMING_3	0x00000000
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00220802 */
#define CONFIG_SYS_DDR_TIMING_1	((2 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (7 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (2 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (6 << TIMING_CFG1_REFREC_SHIFT) \
				| (2 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x27256222 */
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (4 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (5 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x121048c5 */
#define CONFIG_SYS_DDR_INTERVAL	((0x0360 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x03600100 */
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32)
				/* 0x43080000 */

#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000 /* 1 posted refresh */
#define CONFIG_SYS_DDR_MODE		((0x0448 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0232 << SDRAM_MODE_SD_SHIFT))
				/* ODT 150ohm CL=3, AL=1 on SDRAM */
#define CONFIG_SYS_DDR_MODE2		0x00000000

/*
 * Memory test
 */
#define CONFIG_SYS_MEMTEST_START	0x00001000 /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x07f00000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE /* start of monitor */

#define CONFIG_SYS_MONITOR_LEN	(384 * 1024) /* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT

#define CONFIG_SYS_FLASH_BASE		0xFC000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE		64 /* FLASH size is 64M */


#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(1000 * 1024)
/* Flash Write Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(500 * 1024)

/*
 * SJA1000 CAN controller on Local Bus
 */
#define CONFIG_SYS_SJA1000_BASE	0xFBFF0000


/*
 * CPLD on Local Bus
 */
#define CONFIG_SYS_CPLD_BASE	0xFBFF8000


/*
 * Serial Port
 */
#undef CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_SYS_PCIE1_BASE		0xA0000000
#define CONFIG_SYS_PCIE1_MEM_BASE	0xA0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xA0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x10000000
#define CONFIG_SYS_PCIE1_CFG_BASE	0xB0000000
#define CONFIG_SYS_PCIE1_CFG_SIZE	0x01000000
#define CONFIG_SYS_PCIE1_IO_BASE	0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xB1000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000

/* enable PCIE clock */
#define CONFIG_SYS_SCCR_PCIEXP1CM	1

#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_PCIE

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */
#define CONFIG_83XX_GENERIC_PCIE_REGISTER_HOSES 1

/*
 * TSEC
 */
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define CONFIG_SYS_TSEC1	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC1_OFFSET)
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define CONFIG_SYS_TSEC2	(CONFIG_SYS_IMMR+CONFIG_SYS_TSEC2_OFFSET)

/*
 * TSEC ethernet configuration
 */
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2_NAME	"eTSEC1"
#define TSEC1_PHY_ADDR		1
#define TSEC2_PHY_ADDR		2
#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC1_FLAGS		0
#define TSEC2_FLAGS		0

/* Options are: eTSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC0"

/*
 * Environment
 */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
				 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * Command line configuration.
 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR		0x2000000 /* default load address */

#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#endif

#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */


#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS0\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=${consoledev},${baudrate}\0"			\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addmisc=setenv bootargs ${bootargs}\0"				\
	"kernel_addr=FC0A0000\0"					\
	"fdt_addr=FC2A0000\0"						\
	"ramdisk_addr=FC2C0000\0"					\
	"u-boot=mpc8308_p1m/u-boot.bin\0"				\
	"kernel_addr_r=1000000\0"					\
	"fdt_addr_r=C00000\0"						\
	"hostname=mpc8308_p1m\0"					\
	"bootfile=mpc8308_p1m/uImage\0"					\
	"fdtfile=mpc8308_p1m/mpc8308_p1m.dtb\0"				\
	"rootpath=/opt/eldk-4.2/ppc_6xx\0"				\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${fdt_addr_r} ${fdtfile};"			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"bootcmd=run flash_self\0"					\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\

#endif	/* __CONFIG_H */
