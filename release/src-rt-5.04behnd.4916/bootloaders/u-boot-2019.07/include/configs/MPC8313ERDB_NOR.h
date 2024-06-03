/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006, 2010.
 */
/*
 * mpc8313epb board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

#define CONFIG_PCI_INDIRECT_BRIDGE
#define CONFIG_FSL_ELBC 1

/*
 * On-board devices
 *
 * TSEC1 is VSC switch
 * TSEC2 is SoC TSEC
 */
#define CONFIG_VSC7385_ENET
#define CONFIG_TSEC2

#define CONFIG_SYS_MEMTEST_START	0x00001000
#define CONFIG_SYS_MEMTEST_END		0x07f00000

/* Early revs of this board will lock up hard when attempting
 * to access the PMC registers, unless a JTAG debugger is
 * connected, or some resistor modifications are made.
 */
#define CONFIG_SYS_8313ERDB_BROKEN_PMC 1

/*
 * Device configurations
 */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

#define CONFIG_TSEC1

/* The flash address and size of the VSC7385 firmware image */
#define CONFIG_VSC7385_IMAGE		0xFE7FE000
#define CONFIG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory*/

/*
 * Manually set up DDR parameters, as this board does not
 * seem to have the SPD connected to I2C.
 */
#define CONFIG_SYS_DDR_SIZE	128		/* MB */
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
#define CONFIG_SYS_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (8 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (5 << TIMING_CFG1_CASLAT_SHIFT) \
				| (10 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3835a322 */
#define CONFIG_SYS_DDR_TIMING_2	((1 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (5 << TIMING_CFG2_CPO_SHIFT) \
				| (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (6 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x129048c6 */ /* P9-45,may need tuning */
#define CONFIG_SYS_DDR_INTERVAL	((1296 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (1280 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x05100500 */
#if defined(CONFIG_DDR_2T_TIMING)
#define CONFIG_SYS_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32 \
				| SDRAM_CFG_2T_EN)
				/* 0x43088000 */
#else
#define CONFIG_SYS_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_32)
				/* 0x43080000 */
#endif
#define CONFIG_SYS_SDRAM_CFG2		0x00401000
/* set burst length to 8 for 32-bit data path */
#define CONFIG_SYS_DDR_MODE	((0x4448 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0632 << SDRAM_MODE_SD_SHIFT))
				/* 0x44480632 */
#define CONFIG_SYS_DDR_MODE_2	0x8000C000

#define CONFIG_SYS_DDR_CLK_CNTL	DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05
				/*0x02000000*/
#define CONFIG_SYS_DDRCDR_VALUE	(DDRCDR_EN \
				| DDRCDR_PZ_NOMZ \
				| DDRCDR_NZ_NOMZ \
				| DDRCDR_M_ODR)

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		8	/* flash size in MB */
#define CONFIG_SYS_FLASH_PROTECTION	1	/* Use h/w Flash protection. */
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* display empty sectors */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	135	/* sectors per device */

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE) && \
	!defined(CONFIG_SPL_BUILD)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xFD000000	/* Initial RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000	/* Size of used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/* CONFIG_SYS_MONITOR_LEN must be a multiple of CONFIG_ENV_SECT_SIZE */
#define CONFIG_SYS_MONITOR_LEN	(512 * 1024)	/* Reserve 512 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)	/* Reserved for malloc */

/* drivers/mtd/nand/nand.c */
#define CONFIG_SYS_NAND_BASE		0xE2800000

#define CONFIG_MTD_PARTITION

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_NAND_FSL_ELBC 1
#define CONFIG_SYS_NAND_BLOCK_SIZE 16384
#define CONFIG_SYS_NAND_WINDOW_SIZE (32 * 1024)

/* Still needed for spl_minimal.c */
#define CONFIG_SYS_NAND_BR_PRELIM CONFIG_SYS_BR1_PRELIM
#define CONFIG_SYS_NAND_OR_PRELIM CONFIG_SYS_OR1_PRELIM

/* local bus write LED / read status buffer (BCSR) mapping */
#define CONFIG_SYS_BCSR_ADDR		0xFA000000
#define CONFIG_SYS_BCSR_SIZE		(32 * 1024)	/* 0x00008000 */
					/* map at 0xFA000000 on LCS3 */
/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

					/* VSC7385 Base address on LCS2 */
#define CONFIG_SYS_VSC7385_BASE		0xF0000000
#define CONFIG_SYS_VSC7385_SIZE		(128 * 1024)	/* 0x00020000 */


#endif

#define CONFIG_MPC83XX_GPIO 1

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

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
#define CONFIG_SYS_PCI1_IO_BASE	0x00000000
#define CONFIG_SYS_PCI1_IO_PHYS	0xE2000000
#define CONFIG_SYS_PCI1_IO_SIZE	0x00100000	/* 1M */

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1057	/* Motorola */

/*
 * TSEC
 */

#define CONFIG_GMII			/* MII PHY management */

#ifdef CONFIG_TSEC1
#define CONFIG_HAS_ETH0
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_SYS_TSEC1_OFFSET	0x24000
#define TSEC1_PHY_ADDR		0x1c
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC1_PHYIDX		0
#endif

#ifdef CONFIG_TSEC2
#define CONFIG_HAS_ETH1
#define CONFIG_TSEC2_NAME	"TSEC1"
#define CONFIG_SYS_TSEC2_OFFSET	0x25000
#define TSEC2_PHY_ADDR		4
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC2_PHYIDX		0
#endif

/* Options are: TSEC[0-1] */
#define CONFIG_ETHPRIME			"TSEC1"

/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR		0x68

/*
 * Environment
 */
#if !defined(CONFIG_SYS_RAMBOOT)
	#define CONFIG_ENV_ADDR		\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
	#define CONFIG_ENV_SECT_SIZE	0x10000	/* 64K(one sector) for env */
	#define CONFIG_ENV_SIZE		0x2000

/* Address and size of Redundant Environment Sector */
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

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */

				/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
				/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#define CONFIG_SYS_RCWH_PCIHOST 0x80000000	/* PCIHOST  */

#define CONFIG_SYS_NS16550_CLK (get_bus_freq(0))

/* System IO Config */
#define CONFIG_SYS_SICRH	(SICRH_TSOBI1 | SICRH_TSOBI2)	/* RGMII */
			/* Enable Internal USB Phy and GPIO on LCD Connector */
#define CONFIG_SYS_SICRL	(SICRL_USBDR_10 | SICRL_LBC)

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_NETDEV		"eth1"

#define CONFIG_HOSTNAME		"mpc8313erdb"
#define CONFIG_ROOTPATH		"/nfs/root/path"
#define CONFIG_BOOTFILE		"uImage"
				/* U-Boot image on TFTP server */
#define CONFIG_UBOOTPATH	"u-boot.bin"
#define CONFIG_FDTFILE		"mpc8313erdb.dtb"

				/* default location for tftp and bootm */
#define CONFIG_LOADADDR		800000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"netdev=" CONFIG_NETDEV "\0"					\
	"ethprime=TSEC1\0"						\
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
	"fdtfile=" CONFIG_FDTFILE "\0"					\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	 \
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"\
							"$netdev:off " \
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv rootdev /dev/nfs;"					\
	"run setbootargs;"						\
	"run setipargs;"						\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"setenv rootdev /dev/ram;"					\
	"run setbootargs;"						\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#endif	/* __CONFIG_H */
