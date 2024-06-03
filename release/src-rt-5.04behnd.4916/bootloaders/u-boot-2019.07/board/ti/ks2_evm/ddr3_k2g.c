// SPDX-License-Identifier: GPL-2.0+
/*
 * K2G: DDR3 initialization
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include "ddr3_cfg.h"
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>
#include "board.h"

/* K2G GP EVM DDR3 Configuration */
static struct ddr3_phy_config ddr3phy_800_2g = {
	.pllcr          = 0x000DC000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0,
	.ptr3           = 0x06C30D40ul,
	.ptr4           = 0x06413880ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x550F6644ul,
	.dtpr1          = 0x328341E0ul,
	.dtpr2          = 0x50022A00ul,
	.mr0            = 0x00001430ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000000ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F03D09ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.datx8_2_mask   = 0,
	.datx8_2_val    = 0,
	.datx8_3_mask   = 0,
	.datx8_3_val    = 0,
	.datx8_4_mask   = 0,
	.datx8_4_val    = ((1 << 0)),
	.datx8_5_mask   = DXEN_MASK,
	.datx8_5_val    = 0,
	.datx8_6_mask   = DXEN_MASK,
	.datx8_6_val    = 0,
	.datx8_7_mask   = DXEN_MASK,
	.datx8_7_val    = 0,
	.datx8_8_mask   = DXEN_MASK,
	.datx8_8_val    = 0,
	.pir_v2         = 0x00000F81ul,
};

static struct ddr3_phy_config ddr3phy_1066_2g = {
	.pllcr          = 0x000DC000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (2 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0,
	.ptr3           = 0x0904111Dul,
	.ptr4           = 0x0859A072ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x6D147744ul,
	.dtpr1          = 0x32845A80ul,
	.dtpr2          = 0x50023600ul,
	.mr0            = 0x00001830ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000000ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F05159ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.datx8_2_mask   = 0,
	.datx8_2_val    = 0,
	.datx8_3_mask   = 0,
	.datx8_3_val    = 0,
	.datx8_4_mask   = 0,
	.datx8_4_val    = ((1 << 0)),
	.datx8_5_mask   = DXEN_MASK,
	.datx8_5_val    = 0,
	.datx8_6_mask   = DXEN_MASK,
	.datx8_6_val    = 0,
	.datx8_7_mask   = DXEN_MASK,
	.datx8_7_val    = 0,
	.datx8_8_mask   = DXEN_MASK,
	.datx8_8_val    = 0,
	.pir_v2         = 0x00000F81ul,
};

static struct ddr3_emif_config ddr3_800_2g = {
	.sdcfg          = 0x62005662ul,
	.sdtim1         = 0x0A385033ul,
	.sdtim2         = 0x00001CA5ul,
	.sdtim3         = 0x21ADFF32ul,
	.sdtim4         = 0x533F067Ful,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00000C34ul,
};

static struct ddr3_emif_config ddr3_1066_2g = {
	.sdcfg          = 0x62005662ul,
	.sdtim1         = 0x0E4C6843ul,
	.sdtim2         = 0x00001CC6ul,
	.sdtim3         = 0x323DFF32ul,
	.sdtim4         = 0x533F08AFul,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00001044ul,
};

/* K2G ICE evm DDR3 Configuration */
static struct ddr3_phy_config ddr3phy_800_512mb = {
	.pllcr          = 0x000DC000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (2 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0,
	.ptr3           = 0x06C30D40ul,
	.ptr4           = 0x06413880ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x550E6644ul,
	.dtpr1          = 0x32834200ul,
	.dtpr2          = 0x50022A00ul,
	.mr0            = 0x00001430ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000008ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F03D09ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.datx8_2_mask   = DXEN_MASK,
	.datx8_2_val    = 0,
	.datx8_3_mask   = DXEN_MASK,
	.datx8_3_val    = 0,
	.datx8_4_mask   = DXEN_MASK,
	.datx8_4_val    = 0,
	.datx8_5_mask   = DXEN_MASK,
	.datx8_5_val    = 0,
	.datx8_6_mask   = DXEN_MASK,
	.datx8_6_val    = 0,
	.datx8_7_mask   = DXEN_MASK,
	.datx8_7_val    = 0,
	.datx8_8_mask   = DXEN_MASK,
	.datx8_8_val    = 0,
	.pir_v2         = 0x00000F81ul,
};

static struct ddr3_emif_config ddr3_800_512mb = {
	.sdcfg          = 0x62006662ul,
	.sdtim1         = 0x0A385033ul,
	.sdtim2         = 0x00001CA5ul,
	.sdtim3         = 0x21ADFF32ul,
	.sdtim4         = 0x533F067Ful,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00000C34ul,
};

u32 ddr3_init(void)
{
	/* Reset DDR3 PHY after PLL enabled */
	ddr3_reset_ddrphy();
	if (board_is_k2g_g1()) {
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_1066_2g);
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_1066_2g);
	} else if (board_is_k2g_gp()) {
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_800_2g);
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_800_2g);
	} else if (board_is_k2g_ice()) {
		ddr3_init_ddrphy(KS2_DDR3A_DDRPHYC, &ddr3phy_800_512mb);
		ddr3_init_ddremif(KS2_DDR3A_EMIF_CTRL_BASE, &ddr3_800_512mb);
	}

	return 0;
}

inline int ddr3_get_size(void)
{
	return 2;
}
