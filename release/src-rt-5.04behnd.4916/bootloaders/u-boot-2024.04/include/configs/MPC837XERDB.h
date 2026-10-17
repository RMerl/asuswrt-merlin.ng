/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Kevin Lam <kevin.lam@freescale.com>
 * Joe D'Abbraccio <joe.d'abbraccio@freescale.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/stringify.h>

/*
 * High Level Configuration Options
 */

/* System performance - define the value i.e. CONFIG_SYS_XXX
*/

/* System Clock Configuration Register */
#define CFG_SYS_SCCR_TSEC1CM	1		/* eTSEC1 clock mode (0-3) */
#define CFG_SYS_SCCR_TSEC2CM	1		/* eTSEC2 clock mode (0-3) */
#define CFG_SYS_SCCR_SATACM	SCCR_SATACM_2	/* SATA1-4 clock mode (0-3) */

/*
 * System IO Config
 */
#define CFG_SYS_SICRH		0x08200000
#define CFG_SYS_SICRL		0x00000000

/*
 * Output Buffer Impedance
 */
#define CFG_SYS_OBIR		0x30100000

/*
 * Device configurations
 */

/* Vitesse 7385 */

#ifdef CONFIG_VSC7385_ENET

/* The flash address and size of the VSC7385 firmware image */
#define CFG_VSC7385_IMAGE		0xFE7FE000
#define CFG_VSC7385_IMAGE_SIZE	8192

#endif

/*
 * DDR Setup
 */
#define CFG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
#define CFG_SYS_DDR_SDRAM_CLK_CNTL	0x03000000

#define CFG_SYS_DDRCDR_VALUE	(DDRCDR_DHC_EN | DDRCDR_ODT | DDRCDR_Q_DRN)

/*
 * Manually set up DDR parameters
 */
#define CFG_SYS_SDRAM_SIZE		0x10000000 /* 256 MiB */
#define CFG_SYS_DDR_CS0_BNDS		0x0000000f
#define CFG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN \
					| CSCONFIG_ODT_WR_ONLY_CURRENT \
					| CSCONFIG_ROW_BIT_13 \
					| CSCONFIG_COL_BIT_10)

#define CFG_SYS_DDR_TIMING_3	0x00000000
#define CFG_SYS_DDR_TIMING_0	((0 << TIMING_CFG0_RWT_SHIFT) \
				| (0 << TIMING_CFG0_WRT_SHIFT) \
				| (0 << TIMING_CFG0_RRT_SHIFT) \
				| (0 << TIMING_CFG0_WWT_SHIFT) \
				| (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) \
				| (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) \
				| (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) \
				| (2 << TIMING_CFG0_MRS_CYC_SHIFT))
				/* 0x00260802 */ /* DDR400 */
#define CFG_SYS_DDR_TIMING_1	((3 << TIMING_CFG1_PRETOACT_SHIFT) \
				| (9 << TIMING_CFG1_ACTTOPRE_SHIFT) \
				| (3 << TIMING_CFG1_ACTTORW_SHIFT) \
				| (7 << TIMING_CFG1_CASLAT_SHIFT) \
				| (13 << TIMING_CFG1_REFREC_SHIFT) \
				| (3 << TIMING_CFG1_WRREC_SHIFT) \
				| (2 << TIMING_CFG1_ACTTOACT_SHIFT) \
				| (2 << TIMING_CFG1_WRTORD_SHIFT))
				/* 0x3937d322 */
#define CFG_SYS_DDR_TIMING_2	((0 << TIMING_CFG2_ADD_LAT_SHIFT) \
				| (5 << TIMING_CFG2_CPO_SHIFT) \
				| (3 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) \
				| (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) \
				| (3 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) \
				| (3 << TIMING_CFG2_CKE_PLS_SHIFT) \
				| (8 << TIMING_CFG2_FOUR_ACT_SHIFT))
				/* 0x02984cc8 */

#define CFG_SYS_DDR_INTERVAL	((1024 << SDRAM_INTERVAL_REFINT_SHIFT) \
				| (0 << SDRAM_INTERVAL_BSTOPRE_SHIFT))
				/* 0x06090100 */

#define CFG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SREN \
					| SDRAM_CFG_SDRAM_TYPE_DDR2)
					/* 0x43000000 */
#define CFG_SYS_DDR_SDRAM_CFG2	0x00001000 /* 1 posted refresh */
#define CFG_SYS_DDR_MODE		((0x0406 << SDRAM_MODE_ESD_SHIFT) \
					| (0x0442 << SDRAM_MODE_SD_SHIFT))
					/* 0x04400442 */ /* DDR400 */
#define CFG_SYS_DDR_MODE2		0x00000000

/*
 * Memory test
 */
#undef CFG_SYS_DRAM_TEST		/* memory test, takes time */

/*
 * The reserved memory
 */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */

/*
 * FLASH on the Local Bus
 */
#define CFG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CFG_SYS_FLASH_SIZE		8 /* max FLASH size is 32M */

/*
 * NAND Flash on the Local Bus
 */
#define CFG_SYS_NAND_BASE	0xE0600000


/* Vitesse 7385 */

#define CFG_SYS_VSC7385_BASE	0xF0000000

/*
 * Serial Port
 */
#if !CONFIG_IS_ENABLED(DM_SERIAL) && !CONFIG_IS_ENABLED(DM_CLK)
#define CFG_SYS_NS16550_CLK		get_bus_freq(0)
#endif

#define CFG_SYS_BAUDRATE_TABLE \
		{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CFG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CFG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* SERDES */
#define CFG_FSL_SERDES1	0xe3000
#define CFG_FSL_SERDES2	0xe3100

/* I2C */
#define CFG_SYS_I2C_NOPROBES		{ {0, 0x51} }

/*
 * Config on-board RTC
 */
#define CFG_SYS_I2C_RTC_ADDR	0x68 /* at address 0x68 */

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CFG_SYS_PCIE1_CFG_BASE	0xA0000000
#define CFG_SYS_PCIE1_CFG_SIZE	0x08000000
#define CFG_SYS_PCIE1_MEM_PHYS	0xA8000000
#define CFG_SYS_PCIE1_IO_PHYS	0xB8000000

#define CFG_SYS_PCIE2_CFG_BASE	0xC0000000
#define CFG_SYS_PCIE2_CFG_SIZE	0x08000000
#define CFG_SYS_PCIE2_MEM_PHYS	0xC8000000
#define CFG_SYS_PCIE2_IO_PHYS	0xD8000000

#ifdef CONFIG_MMC
#define CFG_SYS_FSL_ESDHC_ADDR	CFG_SYS_MPC83xx_ESDHC_ADDR
#endif

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

#define FDTFILE			"mpc8379_rdb.dtb"

#define CFG_EXTRA_ENV_SETTINGS \
	"netdev=eth1\0"				\
	"uboot=" CONFIG_UBOOTPATH "\0"					\
	"tftpflash=tftp $loadaddr $uboot;"				\
		"protect off " __stringify(CONFIG_TEXT_BASE)	\
			" +$filesize; "	\
		"erase " __stringify(CONFIG_TEXT_BASE)		\
			" +$filesize; "	\
		"cp.b $loadaddr " __stringify(CONFIG_TEXT_BASE)	\
			" $filesize; "	\
		"protect on " __stringify(CONFIG_TEXT_BASE)		\
			" +$filesize; "	\
		"cmp.b $loadaddr " __stringify(CONFIG_TEXT_BASE)	\
			" $filesize\0"	\
	"fdtaddr=780000\0"						\
	"fdtfile=" FDTFILE "\0"					\
	"ramdiskaddr=1000000\0"						\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"				\
	"console=ttyS0\0"						\
	"setbootargs=setenv bootargs "					\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0" \
	"setipargs=setenv bootargs nfsroot=$serverip:$rootpath "	\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:"	\
							"$netdev:off "	\
		"root=$rootdev rw console=$console,$baudrate $othbootargs\0"

#endif	/* __CONFIG_H */
