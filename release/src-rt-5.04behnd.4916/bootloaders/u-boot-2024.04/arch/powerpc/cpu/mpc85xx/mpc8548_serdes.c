// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		8

static u32 serdes1_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x3] = {PCIE1, PCIE1, PCIE1, PCIE1, SRIO1, SRIO1, SRIO1, SRIO1},
	[0x4] = {PCIE1, PCIE1, PCIE1, PCIE1, SRIO1, SRIO1, SRIO1, SRIO1},
	[0x5] = {NONE, NONE, NONE, NONE, SRIO1, SRIO1, SRIO1, SRIO1},
	[0x6] = {NONE, NONE, NONE, NONE, SRIO1, SRIO1, SRIO1, SRIO1},
	[0x7] = {PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1, PCIE1},
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
	u32 srds1_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	int lane;

	if (serdes1_prtcl_map & (1 << NONE))
		return;

	debug("PORDEVSR[IO_SEL] = %x\n", srds1_cfg);

	if (srds1_cfg >= ARRAY_SIZE(serdes1_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL] = %d\n", srds1_cfg);
		return;
	}

	for (lane = 0; lane < SRDS1_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes1_cfg_tbl[srds1_cfg][lane];
		serdes1_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes1_prtcl_map |= (1 << NONE);
}
