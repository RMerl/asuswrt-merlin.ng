// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008,2010 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

/* PORDEVSR register */
#define GUTS_PORDEVSR_OFFS		0xc
#define GUTS_PORDEVSR_SERDES2_IO_SEL	0x38000000
#define GUTS_PORDEVSR_SERDES2_IO_SEL_SHIFT	27

/* SerDes CR0 register */
#define	FSL_SRDSCR0_OFFS	0x0
#define FSL_SRDSCR0_TXEQA_MASK	0x00007000
#define FSL_SRDSCR0_TXEQA_SGMII	0x00004000
#define FSL_SRDSCR0_TXEQA_SATA	0x00001000
#define FSL_SRDSCR0_TXEQE_MASK	0x00000700
#define FSL_SRDSCR0_TXEQE_SGMII	0x00000400
#define FSL_SRDSCR0_TXEQE_SATA	0x00000100

/* SerDes CR1 register */
#define FSL_SRDSCR1_OFFS	0x4
#define FSL_SRDSCR1_LANEA_MASK	0x80200000
#define FSL_SRDSCR1_LANEA_OFF	0x80200000
#define FSL_SRDSCR1_LANEE_MASK	0x08020000
#define FSL_SRDSCR1_LANEE_OFF	0x08020000

/* SerDes CR2 register */
#define FSL_SRDSCR2_OFFS	0x8
#define FSL_SRDSCR2_EICA_MASK	0x00001f00
#define FSL_SRDSCR2_EICA_SGMII	0x00000400
#define FSL_SRDSCR2_EICA_SATA	0x00001400
#define FSL_SRDSCR2_EICE_MASK	0x0000001f
#define FSL_SRDSCR2_EICE_SGMII	0x00000004
#define FSL_SRDSCR2_EICE_SATA	0x00000014

/* SerDes CR3 register */
#define FSL_SRDSCR3_OFFS	0xc
#define FSL_SRDSCR3_LANEA_MASK	0x3f000700
#define FSL_SRDSCR3_LANEA_SGMII	0x00000000
#define FSL_SRDSCR3_LANEA_SATA	0x15000500
#define FSL_SRDSCR3_LANEE_MASK	0x003f0007
#define FSL_SRDSCR3_LANEE_SGMII	0x00000000
#define FSL_SRDSCR3_LANEE_SATA	0x00150005

#define SRDS1_MAX_LANES		8
#define SRDS2_MAX_LANES		2

static u32 serdes1_prtcl_map, serdes2_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x2] = {PCIE1, PCIE1, PCIE1, PCIE1, NONE, NONE, NONE, NONE},
	[0x3] = {PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1},
	[0x5] = {PCIE1, PCIE1, PCIE1, PCIE1, PCIE2, PCIE2, PCIE2, PCIE2},
	[0x7] = {PCIE1, PCIE1, PCIE1, PCIE1, PCIE2, PCIE2, PCIE3, PCIE3},
};

static u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x1] = {SATA1, SATA2},
	[0x3] = {SATA1, NONE},
	[0x4] = {SGMII_TSEC1, SGMII_TSEC3},
	[0x6] = {SGMII_TSEC1, NONE},
};

int is_serdes_configured(enum srds_prtcl device)
{
	int ret;

	if (!(serdes1_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	ret = (1 << device) & serdes1_prtcl_map;

	if (ret)
		return ret;

	if (!(serdes2_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	return (1 << device) & serdes2_prtcl_map;
}

void fsl_serdes_init(void)
{
	void *guts = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	void *sd = (void *)CONFIG_SYS_MPC85xx_SERDES2_ADDR;
	u32 pordevsr = in_be32(guts + GUTS_PORDEVSR_OFFS);
	u32 srds1_io_sel, srds2_io_sel;
	u32 tmp;
	int lane;

	if (serdes1_prtcl_map & (1 << NONE) &&
	    serdes2_prtcl_map & (1 << NONE))
		return;

	srds1_io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;

	/* parse the SRDS2_IO_SEL of PORDEVSR */
	srds2_io_sel = (pordevsr & GUTS_PORDEVSR_SERDES2_IO_SEL)
		       >> GUTS_PORDEVSR_SERDES2_IO_SEL_SHIFT;

	debug("PORDEVSR[SRDS1_IO_SEL] = %x\n", srds1_io_sel);
	debug("PORDEVSR[SRDS2_IO_SEL] = %x\n", srds2_io_sel);

	switch (srds2_io_sel) {
	case 1:	/* Lane A - SATA1, Lane E - SATA2 */
		/* CR 0 */
		tmp = in_be32(sd + FSL_SRDSCR0_OFFS);
		tmp &= ~FSL_SRDSCR0_TXEQA_MASK;
		tmp |= FSL_SRDSCR0_TXEQA_SATA;
		tmp &= ~FSL_SRDSCR0_TXEQE_MASK;
		tmp |= FSL_SRDSCR0_TXEQE_SATA;
		out_be32(sd + FSL_SRDSCR0_OFFS, tmp);
		/* CR 1 */
		tmp = in_be32(sd + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_LANEA_MASK;
		tmp &= ~FSL_SRDSCR1_LANEE_MASK;
		out_be32(sd + FSL_SRDSCR1_OFFS, tmp);
		/* CR 2 */
		tmp = in_be32(sd + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_EICA_MASK;
		tmp |= FSL_SRDSCR2_EICA_SATA;
		tmp &= ~FSL_SRDSCR2_EICE_MASK;
		tmp |= FSL_SRDSCR2_EICE_SATA;
		out_be32(sd + FSL_SRDSCR2_OFFS, tmp);
		/* CR 3 */
		tmp = in_be32(sd + FSL_SRDSCR3_OFFS);
		tmp &= ~FSL_SRDSCR3_LANEA_MASK;
		tmp |= FSL_SRDSCR3_LANEA_SATA;
		tmp &= ~FSL_SRDSCR3_LANEE_MASK;
		tmp |= FSL_SRDSCR3_LANEE_SATA;
		out_be32(sd + FSL_SRDSCR3_OFFS, tmp);
		break;
	case 3: /* Lane A - SATA1, Lane E - disabled */
		/* CR 0 */
		tmp = in_be32(sd + FSL_SRDSCR0_OFFS);
		tmp &= ~FSL_SRDSCR0_TXEQA_MASK;
		tmp |= FSL_SRDSCR0_TXEQA_SATA;
		out_be32(sd + FSL_SRDSCR0_OFFS, tmp);
		/* CR 1 */
		tmp = in_be32(sd + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_LANEE_MASK;
		tmp |= FSL_SRDSCR1_LANEE_OFF;
		out_be32(sd + FSL_SRDSCR1_OFFS, tmp);
		/* CR 2 */
		tmp = in_be32(sd + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_EICA_MASK;
		tmp |= FSL_SRDSCR2_EICA_SATA;
		out_be32(sd + FSL_SRDSCR2_OFFS, tmp);
		/* CR 3 */
		tmp = in_be32(sd + FSL_SRDSCR3_OFFS);
		tmp &= ~FSL_SRDSCR3_LANEA_MASK;
		tmp |= FSL_SRDSCR3_LANEA_SATA;
		out_be32(sd + FSL_SRDSCR3_OFFS, tmp);
		break;
	case 4: /* Lane A - eTSEC1 SGMII, Lane E - eTSEC3 SGMII */
		/* CR 0 */
		tmp = in_be32(sd + FSL_SRDSCR0_OFFS);
		tmp &= ~FSL_SRDSCR0_TXEQA_MASK;
		tmp |= FSL_SRDSCR0_TXEQA_SGMII;
		tmp &= ~FSL_SRDSCR0_TXEQE_MASK;
		tmp |= FSL_SRDSCR0_TXEQE_SGMII;
		out_be32(sd + FSL_SRDSCR0_OFFS, tmp);
		/* CR 1 */
		tmp = in_be32(sd + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_LANEA_MASK;
		tmp &= ~FSL_SRDSCR1_LANEE_MASK;
		out_be32(sd + FSL_SRDSCR1_OFFS, tmp);
		/* CR 2 */
		tmp = in_be32(sd + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_EICA_MASK;
		tmp |= FSL_SRDSCR2_EICA_SGMII;
		tmp &= ~FSL_SRDSCR2_EICE_MASK;
		tmp |= FSL_SRDSCR2_EICE_SGMII;
		out_be32(sd + FSL_SRDSCR2_OFFS, tmp);
		/* CR 3 */
		tmp = in_be32(sd + FSL_SRDSCR3_OFFS);
		tmp &= ~FSL_SRDSCR3_LANEA_MASK;
		tmp |= FSL_SRDSCR3_LANEA_SGMII;
		tmp &= ~FSL_SRDSCR3_LANEE_MASK;
		tmp |= FSL_SRDSCR3_LANEE_SGMII;
		out_be32(sd + FSL_SRDSCR3_OFFS, tmp);
		break;
	case 6: /* Lane A - eTSEC1 SGMII, Lane E - disabled */
		/* CR 0 */
		tmp = in_be32(sd + FSL_SRDSCR0_OFFS);
		tmp &= ~FSL_SRDSCR0_TXEQA_MASK;
		tmp |= FSL_SRDSCR0_TXEQA_SGMII;
		out_be32(sd + FSL_SRDSCR0_OFFS, tmp);
		/* CR 1 */
		tmp = in_be32(sd + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_LANEE_MASK;
		tmp |= FSL_SRDSCR1_LANEE_OFF;
		out_be32(sd + FSL_SRDSCR1_OFFS, tmp);
		/* CR 2 */
		tmp = in_be32(sd + FSL_SRDSCR2_OFFS);
		tmp &= ~FSL_SRDSCR2_EICA_MASK;
		tmp |= FSL_SRDSCR2_EICA_SGMII;
		out_be32(sd + FSL_SRDSCR2_OFFS, tmp);
		/* CR 3 */
		tmp = in_be32(sd + FSL_SRDSCR3_OFFS);
		tmp &= ~FSL_SRDSCR3_LANEA_MASK;
		tmp |= FSL_SRDSCR3_LANEA_SGMII;
		out_be32(sd + FSL_SRDSCR3_OFFS, tmp);
		break;
	case 7: /* Lane A - disabled, Lane E - disabled */
		/* CR 1 */
		tmp = in_be32(sd + FSL_SRDSCR1_OFFS);
		tmp &= ~FSL_SRDSCR1_LANEA_MASK;
		tmp |= FSL_SRDSCR1_LANEA_OFF;
		tmp &= ~FSL_SRDSCR1_LANEE_MASK;
		tmp |= FSL_SRDSCR1_LANEE_OFF;
		out_be32(sd + FSL_SRDSCR1_OFFS, tmp);
		break;
	default:
		break;
	}

	if (srds1_io_sel >= ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[SRDS1_IO_SEL] = %d\n", srds1_io_sel);
		return;
	}
	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes1_cfg_tbl[srds1_io_sel][lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes1_prtcl_map |= (1 << NONE);

	if (srds2_io_sel >= ARRAY_SIZE(serdes2_cfg_tbl)) {
		printf("Invalid PORDEVSR[SRDS2_IO_SEL] = %d\n", srds2_io_sel);
		return;
	}

	for (lane = 0; lane < SRDS2_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes2_cfg_tbl[srds2_io_sel][lane];
		serdes2_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes2_prtcl_map |= (1 << NONE);
}
