/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Freescale I2C Controller
 *
 * Copyright 2006 Freescale Semiconductor, Inc.
 *
 * Based on earlier versions by Gleb Natapov <gnatapov@mrv.com>,
 * Xianghua Xiao <x.xiao@motorola.com>, Eran Liberty (liberty@freescale.com),
 * and Jeff Brown.
 * Some bits are taken from linux driver writen by adrian@humboldt.co.uk.
 */

#ifndef _ASM_FSL_I2C_H_
#define _ASM_FSL_I2C_H_

#include <asm/types.h>

typedef struct fsl_i2c_base {

	u8 adr;		/* I2C slave address */
	u8 res0[3];
#define I2C_ADR		0xFE
#define I2C_ADR_SHIFT	1
#define I2C_ADR_RES	~(I2C_ADR)

	u8 fdr;		/* I2C frequency divider register */
	u8 res1[3];
#define IC2_FDR		0x3F
#define IC2_FDR_SHIFT	0
#define IC2_FDR_RES	~(IC2_FDR)

	u8 cr;		/* I2C control redister	*/
	u8 res2[3];
#define I2C_CR_MEN	0x80
#define I2C_CR_MIEN	0x40
#define I2C_CR_MSTA	0x20
#define I2C_CR_MTX	0x10
#define I2C_CR_TXAK	0x08
#define I2C_CR_RSTA	0x04
#define I2C_CR_BIT6	0x02	/* required for workaround A004447 */
#define I2C_CR_BCST	0x01

	u8 sr;		/* I2C status register */
	u8 res3[3];
#define I2C_SR_MCF	0x80
#define I2C_SR_MAAS	0x40
#define I2C_SR_MBB	0x20
#define I2C_SR_MAL	0x10
#define I2C_SR_BCSTM	0x08
#define I2C_SR_SRW	0x04
#define I2C_SR_MIF	0x02
#define I2C_SR_RXAK	0x01

	u8 dr;		/* I2C data register */
	u8 res4[3];
#define I2C_DR		0xFF
#define I2C_DR_SHIFT	0
#define I2C_DR_RES	~(I2C_DR)

	u8 dfsrr;	/* I2C digital filter sampling rate register */
	u8 res5[3];
#define I2C_DFSRR	0x3F
#define I2C_DFSRR_SHIFT	0
#define I2C_DFSRR_RES	~(I2C_DR)

	/* Fill out the reserved block */
	u8 res6[0xE8];
} fsl_i2c_t;

#ifdef CONFIG_DM_I2C
struct fsl_i2c_dev {
	struct fsl_i2c_base __iomem *base;      /* register base */
	u32 i2c_clk;
	u32 index;
	u8 slaveadd;
	uint speed;
};
#endif

#endif	/* _ASM_I2C_H_ */
