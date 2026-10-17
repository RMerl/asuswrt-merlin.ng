/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Hitachi Power Grids. All rights reserved.
 */

#ifndef __CONFIG_PG_WCOM_SELI8_H
#define __CONFIG_PG_WCOM_SELI8_H

/* PAXK FPGA Definitions */
#define CFG_SYS_CSPR3_EXT	(0x00)
#define CFG_SYS_CSPR3	(CSPR_PHYS_ADDR(CONFIG_SYS_PAX_BASE) | \
				CSPR_PORT_SIZE_8 | \
				CSPR_MSEL_GPCM | \
				CSPR_V)
#define CFG_SYS_AMASK3	IFC_AMASK(64 * 1024)
#define CFG_SYS_CSOR3	(CSOR_GPCM_ADM_SHIFT(0x4) | \
				CSOR_GPCM_TRHZ_40)
#define CFG_SYS_CS3_FTIM0	(FTIM0_GPCM_TACSE(0x6) | \
				FTIM0_GPCM_TEADC(0x7) | \
				FTIM0_GPCM_TEAHC(0x2))
#define CFG_SYS_CS3_FTIM1	(FTIM1_GPCM_TACO(0x2) | \
				FTIM1_GPCM_TRAD(0x12))
#define CFG_SYS_CS3_FTIM2	(FTIM2_GPCM_TCS(0x3) | \
				FTIM2_GPCM_TCH(0x1) | \
				FTIM2_GPCM_TWP(0x12))
#define CFG_SYS_CS3_FTIM3	0x04000000

/* PRST */
#define KM_LIU_RST		0
#define KM_PAXK_RST		1
#define KM_DBG_ETH_RST		15

/* QRIO GPIOs used for deblocking */
#define KM_I2C_DEBLOCK_PORT	QRIO_GPIO_A
#define KM_I2C_DEBLOCK_SCL	20
#define KM_I2C_DEBLOCK_SDA	21

#include "km/pg-wcom-ls102xa.h"

#endif /* __CONFIG_PG_WCOM_SELI8_H */
