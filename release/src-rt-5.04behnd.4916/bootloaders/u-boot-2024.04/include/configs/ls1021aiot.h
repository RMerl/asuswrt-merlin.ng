/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

/*
 * DDR: 800 MHz ( 1600 MT/s data rate )
 */

#define DDR_SDRAM_CFG			0x470c0008
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x50550004
#define DDR_TIMING_CFG_1		0xbcb38c56
#define DDR_TIMING_CFG_2		0x0040d120
#define DDR_TIMING_CFG_3		0x010e1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x03401400
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00061c60
#define DDR_SDRAM_MODE_2		0x00180000
#define DDR_SDRAM_INTERVAL		0x18600618
#define DDR_DDR_WRLVL_CNTL		0x8655f605
#define DDR_DDR_WRLVL_CNTL_2	0x05060607
#define DDR_DDR_WRLVL_CNTL_3	0x05050505
#define DDR_DDR_CDR1			0x80040000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN	0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define DDR_CDR2_VREF_TRAIN_EN	0x00000080
#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG_BI			0x00000001

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE

/*
 * Serial Port
 */
#define CFG_SYS_NS16550_CLK		get_serial_clock()

/*
 * I2C
 */

/*
 * MMC
 */

/* SPI */

#define FSL_PCIE_COMPAT		"fsl,ls1021a-pcie"

#define CFG_SMP_PEN_ADDR		0x01ee0200

#define HWCONFIG_BUFFER_SIZE		256

#define CFG_EXTRA_ENV_SETTINGS	\
	"bootargs=root=/dev/ram0 rw console=ttyS0,115200\0" \
"initrd_high=0xffffffff\0"

/*
 * Miscellaneous configurable options
 */
#define CFG_SYS_BOOTMAPSZ		(256 << 20)

#include <asm/fsl_secure_boot.h>

#endif
