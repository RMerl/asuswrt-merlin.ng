// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_86xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4
#define SRDS2_MAX_LANES		4

static u32 serdes1_prtcl_map, serdes2_prtcl_map;

static u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x1] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x4] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x7] = {NONE, NONE, NONE, NONE},
};

static u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x0] = {PCIE2, PCIE2, PCIE2, PCIE2},
	[0x4] = {PCIE2, PCIE2, PCIE2, PCIE2},
	[0x7] = {NONE, NONE, NONE, NONE},
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
	immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	ccsr_gur_t *gur = &immap->im_gur;
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC8610_PORDEVSR_IO_SEL) >>
				MPC8610_PORDEVSR_IO_SEL_SHIFT;
	int lane;

	if (serdes1_prtcl_map & (1 << NONE) &&
	    serdes2_prtcl_map & (1 << NONE))
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

	if (srds_cfg >= ARRAY_SIZE(serdes2_cfg_tbl)) {
		printf("Invalid PORDEVSR[IO_SEL_SRDS] = %d\n", srds_cfg);
		return;
	}

	for (lane = 0; lane < SRDS2_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes2_cfg_tbl[srds_cfg][lane];
		serdes2_prtcl_map |= (1 << lane_prtcl);
	}

	/* Set the first bit to indicate serdes has been initialized */
	serdes2_prtcl_map |= (1 << NONE);
}
