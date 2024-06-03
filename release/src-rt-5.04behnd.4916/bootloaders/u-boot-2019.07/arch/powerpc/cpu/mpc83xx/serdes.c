// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale SerDes initialization routine
 *
 * Copyright 2007,2011 Freescale Semiconductor, Inc.
 * Copyright (C) 2008 MontaVista Software, Inc.
 *
 * Author: Li Yang <leoli@freescale.com>
 */

#ifndef CONFIG_MPC83XX_SERDES

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/fsl_mpc83xx_serdes.h>

/* SerDes registers */
#define FSL_SRDSCR0_OFFS		0x0
#define FSL_SRDSCR0_DPP_1V2		0x00008800
#define FSL_SRDSCR0_TXEQA_MASK		0x00007000
#define FSL_SRDSCR0_TXEQA_SATA		0x00001000
#define FSL_SRDSCR0_TXEQE_MASK		0x00000700
#define FSL_SRDSCR0_TXEQE_SATA		0x00000100
#define FSL_SRDSCR1_OFFS		0x4
#define FSL_SRDSCR1_PLLBW		0x00000040
#define FSL_SRDSCR2_OFFS		0x8
#define FSL_SRDSCR2_VDD_1V2		0x00800000
#define FSL_SRDSCR2_SEIC_MASK		0x00001c1c
#define FSL_SRDSCR2_SEIC_SATA		0x00001414
#define FSL_SRDSCR2_SEIC_PEX		0x00001010
#define FSL_SRDSCR2_SEIC_SGMII		0x00000101
#define FSL_SRDSCR3_OFFS		0xc
#define FSL_SRDSCR3_KFR_SATA		0x10100000
#define FSL_SRDSCR3_KPH_SATA		0x04040000
#define FSL_SRDSCR3_SDFM_SATA_PEX	0x01010000
#define FSL_SRDSCR3_SDTXL_SATA		0x00000505
#define FSL_SRDSCR4_OFFS		0x10
#define FSL_SRDSCR4_PROT_SATA		0x00000808
#define FSL_SRDSCR4_PROT_PEX		0x00000101
#define FSL_SRDSCR4_PROT_SGMII		0x00000505
#define FSL_SRDSCR4_PLANE_X2		0x01000000
#define FSL_SRDSRSTCTL_OFFS		0x20
#define FSL_SRDSRSTCTL_RST		0x80000000
#define FSL_SRDSRSTCTL_SATA_RESET	0xf

void fsl_setup_serdes(u32 offset, char proto, u32 rfcks, char vdd)
{
	void *regs = (void *)CONFIG_SYS_IMMR + offset;
	u32 tmp;

	/* 1.0V corevdd */
	if (vdd) {
		/* DPPE/DPPA = 0 */
		tmp = in_be32(regs + FSL_SRDSCR0_OFFS);
		tmp &= ~FSL_SRDSCR0_DPP_1V2;
		out_be32(regs + FSL_SRDSCR0_OFFS, tmp);

		/* VDD = 0 */
		tmp = in_be32(regs + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_VDD_1V2;
		out_be32(regs + FSL_SRDSCR2_OFFS, tmp);
	}

	/* protocol specific configuration */
	switch (proto) {
	case FSL_SERDES_PROTO_SATA:
		/* Set and clear reset bits */
		tmp = in_be32(regs + FSL_SRDSRSTCTL_OFFS);
		tmp |= FSL_SRDSRSTCTL_SATA_RESET;
		out_be32(regs + FSL_SRDSRSTCTL_OFFS, tmp);
		udelay(1000);
		tmp &= ~FSL_SRDSRSTCTL_SATA_RESET;
		out_be32(regs + FSL_SRDSRSTCTL_OFFS, tmp);

		/* Configure SRDSCR0 */
		clrsetbits_be32(regs + FSL_SRDSCR0_OFFS,
			FSL_SRDSCR0_TXEQA_MASK | FSL_SRDSCR0_TXEQE_MASK,
			FSL_SRDSCR0_TXEQA_SATA | FSL_SRDSCR0_TXEQE_SATA);

		/* Configure SRDSCR1 */
		tmp = in_be32(regs + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_PLLBW;
		out_be32(regs + FSL_SRDSCR1_OFFS, tmp);

		/* Configure SRDSCR2 */
		tmp = in_be32(regs + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_SEIC_MASK;
		tmp |= FSL_SRDSCR2_SEIC_SATA;
		out_be32(regs + FSL_SRDSCR2_OFFS, tmp);

		/* Configure SRDSCR3 */
		tmp = FSL_SRDSCR3_KFR_SATA | FSL_SRDSCR3_KPH_SATA |
			FSL_SRDSCR3_SDFM_SATA_PEX |
			FSL_SRDSCR3_SDTXL_SATA;
		out_be32(regs + FSL_SRDSCR3_OFFS, tmp);

		/* Configure SRDSCR4 */
		tmp = rfcks | FSL_SRDSCR4_PROT_SATA;
		out_be32(regs + FSL_SRDSCR4_OFFS, tmp);
		break;
	case FSL_SERDES_PROTO_PEX:
	case FSL_SERDES_PROTO_PEX_X2:
		/* Configure SRDSCR1 */
		tmp = in_be32(regs + FSL_SRDSCR1_OFFS);
		tmp |= FSL_SRDSCR1_PLLBW;
		out_be32(regs + FSL_SRDSCR1_OFFS, tmp);

		/* Configure SRDSCR2 */
		tmp = in_be32(regs + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_SEIC_MASK;
		tmp |= FSL_SRDSCR2_SEIC_PEX;
		out_be32(regs + FSL_SRDSCR2_OFFS, tmp);

		/* Configure SRDSCR3 */
		tmp = FSL_SRDSCR3_SDFM_SATA_PEX;
		out_be32(regs + FSL_SRDSCR3_OFFS, tmp);

		/* Configure SRDSCR4 */
		tmp = rfcks | FSL_SRDSCR4_PROT_PEX;
		if (proto == FSL_SERDES_PROTO_PEX_X2)
			tmp |= FSL_SRDSCR4_PLANE_X2;
		out_be32(regs + FSL_SRDSCR4_OFFS, tmp);
		break;
	case FSL_SERDES_PROTO_SGMII:
		/* Configure SRDSCR1 */
		tmp = in_be32(regs + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_PLLBW;
		out_be32(regs + FSL_SRDSCR1_OFFS, tmp);

		/* Configure SRDSCR2 */
		tmp = in_be32(regs + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_SEIC_MASK;
		tmp |= FSL_SRDSCR2_SEIC_SGMII;
		out_be32(regs + FSL_SRDSCR2_OFFS, tmp);

		/* Configure SRDSCR3 */
		out_be32(regs + FSL_SRDSCR3_OFFS, 0);

		/* Configure SRDSCR4 */
		tmp = rfcks | FSL_SRDSCR4_PROT_SGMII;
		out_be32(regs + FSL_SRDSCR4_OFFS, tmp);
		break;
	default:
		return;
	}

	/* Do a software reset */
	tmp = in_be32(regs + FSL_SRDSRSTCTL_OFFS);
	tmp |= FSL_SRDSRSTCTL_RST;
	out_be32(regs + FSL_SRDSRSTCTL_OFFS, tmp);
}

#endif /* !CONFIG_MPC83XX_SERDES */
