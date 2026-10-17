// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 * Author: Prabhakar Kushwaha <prabhakar@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4
#define SRDS2_MAX_LANES		2

static u32 serdes1_prtcl_map, serdes2_prtcl_map;

static const u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x00] = {NONE, NONE, NONE, NONE},
	[0x01] = {PCIE1, PCIE2, SGMII_TSEC2, SGMII_TSEC3},
	[0x02] = {PCIE1, SGMII_TSEC1, SGMII_TSEC2, SGMII_TSEC3},
	[0x03] = {NONE, SGMII_TSEC1, SGMII_TSEC2, SGMII_TSEC3},
};

static const u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x00] = {NONE, NONE},
	[0x01] = {SATA1, SATA2},
	[0x02] = {SATA1, SATA2},
	[0x03] = {PCIE1, PCIE2},
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
	ccsr_gur_t *gur = (void *)CFG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	u32 srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
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
