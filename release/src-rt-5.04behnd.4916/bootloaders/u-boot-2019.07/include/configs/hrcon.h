/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */

#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC83xx_ESDHC_ADDR

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
 * consist of one chip NT5TU64M16HG from NANYA
 */

#define CONFIG_SYS_DDR_SIZE		128 /* MB */

#define CONFIG_SYS_DDR_CS0_BNDS	0x00000007
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
				| CSCONFIG_ODT_RD_NEVER \
				| CSCONFIG_ODT_WR_ONLY_CURRENT \
				| CSCONFIG_BANK_BIT_3 \
				| CSCONFIG_ROW_BIT_13 | CSCONFIG_COL_BIT_10)
				/* 0x80010102 */
#define CONFIG_SYS_DDR_TIMING_3	0
#define CONFIG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00260802 */
#define CONFIG_SYS_DDR_TIMING_1	((2 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (6 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (2 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (7 << TIMING_CFG1_CASLAT_SHIFT) \
				| (9 << TIMING_CFG1_REFREC_SHIFT) \
				| (2 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x26279222 */
#define CONFIG_SYS_DDR_TIMING_2	((0 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (4 << TIMING_CFG2_CPO_SHIFT) \
				| (3 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (5 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x021848c5 */
#define CONFIG_SYS_DDR_INTERVAL	((0x0824 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x08240100 */
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
				| SDRAM_CFG_SDRAM_TYPE_DDR2 \
				| SDRAM_CFG_DBW_16)
				/* 0x43100000 */

#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000 /* 1 posted refresh */
#define CONFIG_SYS_DDR_MODE		((0x0440 << SDRAM_MODE_ESD_SHIFT) \
				| (0x0242 << SDRAM_MODE_SD_SHIFT))
				/* ODT 150ohm CL=4, AL=0 on SDRAM */
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
#if 1
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_LEGACY_512Kx16
#endif

#define CONFIG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE		8 /* FLASH size is up to 8M */


#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	135

#define CONFIG_SYS_FLASH_ERASE_TOUT	60000 /* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500 /* Flash Write Timeout (ms) */

#define CONFIG_SYS_FPGA_DONE(k)		0x0010

#define CONFIG_SYS_FPGA_COUNT		1

#define CONFIG_SYS_MCLINK_MAX		3

#define CONFIG_SYS_FPGA_PTR \
	{ (struct ihs_fpga *)CONFIG_SYS_FPGA0_BASE, NULL, NULL, NULL }

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR + 0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR + 0x4600)

/* Pass open firmware flat tree */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000

#define CONFIG_PCA953X			/* NXP PCA9554 */
#define CONFIG_PCA9698			/* NXP PCA9698 */

#define CONFIG_SYS_I2C_IHS
#define CONFIG_SYS_I2C_IHS_CH0
#define CONFIG_SYS_I2C_IHS_SPEED_0		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_0		0x7F
#define CONFIG_SYS_I2C_IHS_CH1
#define CONFIG_SYS_I2C_IHS_SPEED_1		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_1		0x7F
#define CONFIG_SYS_I2C_IHS_CH2
#define CONFIG_SYS_I2C_IHS_SPEED_2		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_2		0x7F
#define CONFIG_SYS_I2C_IHS_CH3
#define CONFIG_SYS_I2C_IHS_SPEED_3		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_3		0x7F

#ifdef CONFIG_HRCON_DH
#define CONFIG_SYS_I2C_IHS_DUAL
#define CONFIG_SYS_I2C_IHS_CH0_1
#define CONFIG_SYS_I2C_IHS_SPEED_0_1		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_0_1		0x7F
#define CONFIG_SYS_I2C_IHS_CH1_1
#define CONFIG_SYS_I2C_IHS_SPEED_1_1		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_1_1		0x7F
#define CONFIG_SYS_I2C_IHS_CH2_1
#define CONFIG_SYS_I2C_IHS_SPEED_2_1		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_2_1		0x7F
#define CONFIG_SYS_I2C_IHS_CH3_1
#define CONFIG_SYS_I2C_IHS_SPEED_3_1		50000
#define CONFIG_SYS_I2C_IHS_SLAVE_3_1		0x7F
#endif

/*
 * Software (bit-bang) I2C driver configuration
 */
#define CONFIG_SYS_I2C_SOFT
#define CONFIG_SYS_I2C_SOFT_SPEED		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE		0x7F
#define I2C_SOFT_DECLARATIONS2
#define CONFIG_SYS_I2C_SOFT_SPEED_2		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_2		0x7F
#define I2C_SOFT_DECLARATIONS3
#define CONFIG_SYS_I2C_SOFT_SPEED_3		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_3		0x7F
#define I2C_SOFT_DECLARATIONS4
#define CONFIG_SYS_I2C_SOFT_SPEED_4		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_4		0x7F
#define I2C_SOFT_DECLARATIONS5
#define CONFIG_SYS_I2C_SOFT_SPEED_5		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_5		0x7F
#define I2C_SOFT_DECLARATIONS6
#define CONFIG_SYS_I2C_SOFT_SPEED_6		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_6		0x7F
#define I2C_SOFT_DECLARATIONS7
#define CONFIG_SYS_I2C_SOFT_SPEED_7		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_7		0x7F
#define I2C_SOFT_DECLARATIONS8
#define CONFIG_SYS_I2C_SOFT_SPEED_8		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_8		0x7F

#ifdef CONFIG_HRCON_DH
#define I2C_SOFT_DECLARATIONS9
#define CONFIG_SYS_I2C_SOFT_SPEED_9		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_9		0x7F
#define I2C_SOFT_DECLARATIONS10
#define CONFIG_SYS_I2C_SOFT_SPEED_10		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_10		0x7F
#define I2C_SOFT_DECLARATIONS11
#define CONFIG_SYS_I2C_SOFT_SPEED_11		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_11		0x7F
#define I2C_SOFT_DECLARATIONS12
#define CONFIG_SYS_I2C_SOFT_SPEED_12		50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_12		0x7F
#endif

#ifdef CONFIG_HRCON_DH
#define CONFIG_SYS_ICS8N3QV01_I2C		{13, 14, 15, 16, 17, 18, 19, 20}
#define CONFIG_SYS_DP501_I2C			{1, 3, 5, 7, 2, 4, 6, 8}
#define CONFIG_HRCON_FANS			{ {10, 0x4c}, {11, 0x4c}, \
						  {12, 0x4c} }
#else
#define CONFIG_SYS_ICS8N3QV01_I2C		{9, 10, 11, 12}
#define CONFIG_SYS_DP501_I2C			{1, 2, 3, 4}
#define CONFIG_HRCON_FANS			{ {6, 0x4c}, {7, 0x4c}, \
						  {8, 0x4c} }
#endif

#ifndef __ASSEMBLY__
void fpga_gpio_set(unsigned int bus, int pin);
void fpga_gpio_clear(unsigned int bus, int pin);
int fpga_gpio_get(unsigned int bus, int pin);
void fpga_control_set(unsigned int bus, int pin);
void fpga_control_clear(unsigned int bus, int pin);
#endif

#define I2C_SDA_GPIO	((I2C_ADAP_HWNR > 3) ? 0x0040 : 0x0200)
#define I2C_SCL_GPIO	((I2C_ADAP_HWNR > 3) ? 0x0020 : 0x0100)
#define I2C_FPGA_IDX	(I2C_ADAP_HWNR % 4)

#ifdef CONFIG_HRCON_DH
#define I2C_ACTIVE \
	do { \
		if (I2C_ADAP_HWNR > 7) \
			fpga_control_set(I2C_FPGA_IDX, 0x0004); \
		else \
			fpga_control_clear(I2C_FPGA_IDX, 0x0004); \
	} while (0)
#else
#define I2C_ACTIVE	{ }
#endif
#define I2C_TRISTATE	{ }
#define I2C_READ \
	(fpga_gpio_get(I2C_FPGA_IDX, I2C_SDA_GPIO) ? 1 : 0)
#define I2C_SDA(bit) \
	do { \
		if (bit) \
			fpga_gpio_set(I2C_FPGA_IDX, I2C_SDA_GPIO); \
		else \
			fpga_gpio_clear(I2C_FPGA_IDX, I2C_SDA_GPIO); \
	} while (0)
#define I2C_SCL(bit) \
	do { \
		if (bit) \
			fpga_gpio_set(I2C_FPGA_IDX, I2C_SCL_GPIO); \
		else \
			fpga_gpio_clear(I2C_FPGA_IDX, I2C_SCL_GPIO); \
	} while (0)
#define I2C_DELAY	udelay(25)	/* 1/4 I2C clock duration */

/*
 * Software (bit-bang) MII driver configuration
 */
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management */
#define CONFIG_BITBANGMII_MULTI

/*
 * OSD Setup
 */
#define CONFIG_SYS_OSD_SCREENS		1
#define CONFIG_SYS_DP501_DIFFERENTIAL
#define CONFIG_SYS_DP501_VCAPCTRL0	0x01 /* DDR mode 0, DE for H/VSYNC */

#ifdef CONFIG_HRCON_DH
#define CONFIG_SYS_OSD_DH
#endif

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

/*
 * TSEC ethernet configuration
 */
#define CONFIG_TSEC1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define TSEC1_PHY_ADDR		1
#define TSEC1_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT

/* Options are: eTSEC[0-1] */
#define CONFIG_ETHPRIME		"eTSEC0"

/*
 * Environment
 */
#if 1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
				 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x10000 /* 64K(one sector) for env */
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
#else
#define CONFIG_ENV_SIZE		0x2000		/* 8KB */
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * Command line configuration.
 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LOAD_ADDR		0x2000000 /* default load address */
#define CONFIG_SYS_HZ		1000	/* decrementer freq: 1ms ticks */

#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size */

#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#endif

#define CONFIG_LOADADDR	800000	/* default location for tftp and bootm */


#define CONFIG_HOSTNAME		"hrcon"
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"

#define CONFIG_PREBOOT		/* enable preboot variable */

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS1\0"						\
	"u-boot=u-boot.bin\0"						\
	"kernel_addr=1000000\0"					\
	"fdt_addr=C00000\0"						\
	"fdtfile=hrcon.dtb\0"				\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\

#define CONFIG_NFSBOOTCOMMAND						\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"			\
	"tftp ${kernel_addr} $bootfile;"				\
	"tftp ${fdt_addr} $fdtfile;"					\
	"bootm ${kernel_addr} - ${fdt_addr}"

#define CONFIG_MMCBOOTCOMMAND						\
	"setenv bootargs root=/dev/mmcblk0p3 rw rootwait "		\
	"console=$consoledev,$baudrate $othbootargs;"			\
	"ext2load mmc 0:2 ${kernel_addr} $bootfile;"			\
	"ext2load mmc 0:2 ${fdt_addr} $fdtfile;"			\
	"bootm ${kernel_addr} - ${fdt_addr}"

#define CONFIG_BOOTCOMMAND		CONFIG_MMCBOOTCOMMAND

#endif	/* __CONFIG_H */
