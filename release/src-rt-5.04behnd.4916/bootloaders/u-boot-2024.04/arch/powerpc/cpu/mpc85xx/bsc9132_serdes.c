// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Author: Prabhakar Kushwaha <prabhakar@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	 [0] = {NONE, NONE, NONE, NONE},
	 [1] = {PCIE1, PCIE2, CPRI2, CPRI1},
	 [2] = {PCIE1, PCIE2, CPRI2, CPRI1},
	 [3] = {PCIE1, PCIE2, CPRI2, CPRI1},
	 [4] = {PCIE1, PCIE2, CPRI2, CPRI1},
	 [5] = {PCIE1, PCIE2, CPRI2, CPRI1},
	 [6] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	 [7] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	 [8] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	 [9] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[10] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[11] = {PCIE1, PCIE2, SGMII_TSEC1, SGMII_TSEC2},
	[12] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[13] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[14] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[15] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[16] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[17] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[18] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[19] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[20] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[21] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[22] = {PCIE1, PCIE2, CPRI2, CPRI1},
	[23] = {PCIE1, PCIE2, CPRI2, CPRI1},
	[24] = {PCIE1, PCIE2, CPRI2, CPRI1},
	[25] = {PCIE1, PCIE2, CPRI2, CPRI1},
	[26] = {PCIE1, PCIE2, CPRI2, CPRI1},
	[27] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[28] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[29] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[30] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[31] = {PCIE1, PCIE2, SGMII_TSEC1, CPRI1},
	[32] = {PCIE1, PCIE2, SGMII_TSEC1, SGMII_TSEC2},
	[33] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[34] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[35] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[36] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[37] = {PCIE1, SGMII_TSEC2, CPRI2, CPRI1},
	[38] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[39] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[40] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[41] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[42] = {PCIE1, SGMII_TSEC2, SGMII_TSEC1, CPRI1},
	[43] = {SGMII_TSEC1, SGMII_TSEC2, CPRI2, CPRI1},
	[44] = {SGMII_TSEC1, SGMII_TSEC2, CPRI2, CPRI1},
	[45] = {SGMII_TSEC1, SGMII_TSEC2, CPRI2, CPRI1},
	[46] = {SGMII_TSEC1, SGMII_TSEC2, CPRI2, CPRI1},
	[47] = {SGMII_TSEC1, SGMII_TSEC2, CPRI2, CPRI1},
};

int is_serdes_configured(enum srds_prtcl prtcl)
{
	if (!(serdes1_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	return (1 << prtcl) & serdes1_prtcl_map;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	int lane;

	if (serdes1_prtcl_map & (1 << NONE))
		return;

	debug("PORDEVSR[IO_SEL_SRDS] = %x\n", srds_cfg);

	if (srds_cfg >= ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes1_cfg_tbl[srds_cfg][lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes1_prtcl_map |= (1 << NONE);
}
