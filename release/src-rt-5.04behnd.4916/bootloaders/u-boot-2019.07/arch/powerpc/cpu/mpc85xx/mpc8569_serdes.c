// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4

static u32 serdes1_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x0] = {PCIE1, NONE, NONE, NONE},
	[0x1] = {SRIO1, SRIO2, SGMII_TSEC1, SGMII_TSEC2},
	[0x2] = {SRIO1, SRIO2, SGMII_TSEC1, SGMII_TSEC2},
	[0x3] = {SRIO1, SRIO2, NONE, NONE},
	[0x4] = {PCIE1, NONE, SGMII_TSEC1, SGMII_TSEC2},
	[0x5] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x6] = {PCIE1, NONE, SRIO1, SRIO2},
	[0x7] = {PCIE1, PCIE1, SRIO1, SRIO2},
	[0x8] = {PCIE1, PCIE1, SRIO1, SRIO2},
	[0x9] = {SRIO1, SRIO1, SRIO1, SRIO1},
	[0xa] = {SRIO1, SRIO1, SRIO1, SRIO1},
	[0xb] = {SRIO1, SRIO1, SRIO1, SRIO1},
	[0xc] = {PCIE1, SRIO1, SGMII_TSEC1, SGMII_TSEC2},
	[0xf] = {PCIE1, PCIE1, PCIE1, PCIE1},
};

int is_serdes_configured(enum srds_prtcl prtcl)
{
	if (!(serdes1_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	return (1 << prtcl) & serdes1_prtcl_map;
}

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
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
