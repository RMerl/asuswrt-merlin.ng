/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor
 * Copyright 2019 NXP
 */

#ifndef __LS1046ARDB_H__
#define __LS1046ARDB_H__

#include "ls1046a_common.h"

/* Physical Memory Map */

#define SPD_EEPROM_ADDRESS		0x51

#if defined(CONFIG_QSPI_BOOT)
#define CFG_SYS_UBOOT_BASE		0x40100000
#endif

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
				| CSOR_NAND_ECC_MODE_8	/* 8-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3 Bytes */ \
				| CSOR_NAND_PGS_4K	/* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224	/* Spare size = 224 */ \
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

/*
 * CPLD
 */
#define CFG_SYS_CPLD_BASE		0x7fb00000
#define CPLD_BASE_PHYS			CFG_SYS_CPLD_BASE

#define CFG_SYS_CPLD_CSPR_EXT	(0x0)
#define CFG_SYS_CPLD_CSPR		(CSPR_PHYS_ADDR(CPLD_BASE_PHYS) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CFG_SYS_CPLD_AMASK		IFC_AMASK(64 * 1024)
#define CFG_SYS_CPLD_CSOR		CSOR_NOR_ADM_SHIFT(16)

/* CPLD Timing parameters for IFC GPCM */
#define CFG_SYS_CPLD_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CFG_SYS_CPLD_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CFG_SYS_CPLD_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0x3E))
#define CFG_SYS_CPLD_FTIM3		0x0

/* IFC Timing Params */
#define CFG_SYS_CSPR0_EXT		CFG_SYS_NAND_CSPR_EXT
#define CFG_SYS_CSPR0		CFG_SYS_NAND_CSPR
#define CFG_SYS_AMASK0		CFG_SYS_NAND_AMASK
#define CFG_SYS_CSOR0		CFG_SYS_NAND_CSOR
#define CFG_SYS_CS0_FTIM0		CFG_SYS_NAND_FTIM0
#define CFG_SYS_CS0_FTIM1		CFG_SYS_NAND_FTIM1
#define CFG_SYS_CS0_FTIM2		CFG_SYS_NAND_FTIM2
#define CFG_SYS_CS0_FTIM3		CFG_SYS_NAND_FTIM3

#define CFG_SYS_CSPR2_EXT		CFG_SYS_CPLD_CSPR_EXT
#define CFG_SYS_CSPR2		CFG_SYS_CPLD_CSPR
#define CFG_SYS_AMASK2		CFG_SYS_CPLD_AMASK
#define CFG_SYS_CSOR2		CFG_SYS_CPLD_CSOR
#define CFG_SYS_CS2_FTIM0		CFG_SYS_CPLD_FTIM0
#define CFG_SYS_CS2_FTIM1		CFG_SYS_CPLD_FTIM1
#define CFG_SYS_CS2_FTIM2		CFG_SYS_CPLD_FTIM2
#define CFG_SYS_CS2_FTIM3		CFG_SYS_CPLD_FTIM3

/* EEPROM */
#define I2C_RETIMER_ADDR			0x18

/* PMIC */

/*
 * Environment
 */
#define CFG_SYS_FSL_QSPI_BASE        0x40000000

#define AQR105_IRQ_MASK			0x80000000
/* FMan */
#ifndef SPL_NO_FMAN
#ifdef CONFIG_SYS_DPAA_FMAN
#define RGMII_PHY1_ADDR			0x1
#define RGMII_PHY2_ADDR			0x2

#define SGMII_PHY1_ADDR			0x3
#define SGMII_PHY2_ADDR			0x4

#define FM1_10GEC1_PHY_ADDR		0x0

#define FDT_SEQ_MACADDR_FROM_ENV
#endif

#endif

#ifndef SPL_NO_MISC
#ifdef CONFIG_TFABOOT
#define QSPI_NOR_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd; "	\
			   "env exists secureboot && esbc_halt;;"
#define SD_BOOTCOMMAND "run distro_bootcmd;run sd_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#endif
#endif

#include <asm/fsl_secure_boot.h>

#endif /* __LS1046ARDB_H__ */
