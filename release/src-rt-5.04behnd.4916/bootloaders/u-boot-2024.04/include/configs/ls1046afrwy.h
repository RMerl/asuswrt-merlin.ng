/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 */

#ifndef __LS1046AFRWY_H__
#define __LS1046AFRWY_H__

#include "ls1046a_common.h"

#define CFG_SYS_UBOOT_BASE		0x40100000

/*
 * NAND Flash Definitions
 */

#define CFG_SYS_NAND_BASE		0x7e800000
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE

#define CFG_SYS_NAND_CSPR_EXT	(0x0)
#define CFG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8	\
				| CSPR_MSEL_NAND	\
				| CSPR_V)
#define CFG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024)
#define CFG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN	/* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3 Bytes */ \
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_128	/* Spare size = 128 */ \
				| CSOR_NAND_PB(64))	/* 64 Pages Per Block */

#define CFG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x7) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x7) | \
					FTIM0_NAND_TWH(0xa))
#define CFG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0xe)   | \
					FTIM1_NAND_TRP(0x18))
#define CFG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0xf) | \
					FTIM2_NAND_TREH(0xa) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CFG_SYS_NAND_FTIM3		0x0

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

/* IFC Timing Params */
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3

/* EEPROM */
#define I2C_RETIMER_ADDR			0x18

/* I2C bus multiplexer */
#define I2C_MUX_PCA_ADDR_PRI			0x77 /* Primary Mux*/
#define I2C_MUX_CH_DEFAULT			0x1 /* Channel 0*/
#define I2C_MUX_CH_RTC				0x1 /* Channel 0*/

/* RTC */
#define CFG_SYS_I2C_RTC_ADDR		0x51  /* Channel 0 I2C bus 0*/
#define CFG_SYS_RTC_BUS_NUM			0

/*
 * Environment
 */
#define CFG_SYS_FSL_QSPI_BASE	0x40000000

#undef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

/* FMan */
#ifdef CONFIG_SYS_DPAA_FMAN

#define QSGMII_PORT1_PHY_ADDR		0x1c
#define QSGMII_PORT2_PHY_ADDR		0x1d
#define QSGMII_PORT3_PHY_ADDR		0x1e
#define QSGMII_PORT4_PHY_ADDR		0x1f

#define FDT_SEQ_MACADDR_FROM_ENV

#endif

#define QSPI_NOR_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd; "	\
			   "env exists secureboot && esbc_halt;;"
#define SD_BOOTCOMMAND "run distro_bootcmd;run sd_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"

#include <asm/fsl_secure_boot.h>

#endif /* __LS1046AFRWY_H__ */
