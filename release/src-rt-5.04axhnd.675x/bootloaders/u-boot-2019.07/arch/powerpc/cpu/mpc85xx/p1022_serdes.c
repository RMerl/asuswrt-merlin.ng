// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Author: Timur Tabi <timur@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

#define SRDS1_MAX_LANES		4
#define SRDS2_MAX_LANES		2

static u32 serdes1_prtcl_map, serdes2_prtcl_map;

static const u8 serdes1_cfg_tbl[][SRDS1_MAX_LANES] = {
	[0x00] = {NONE, NONE, NONE, NONE},
	[0x01] = {NONE, NONE, NONE, NONE},
	[0x02] = {NONE, NONE, NONE, NONE},
	[0x03] = {NONE, NONE, NONE, NONE},
	[0x04] = {NONE, NONE, NONE, NONE},
	[0x06] = {PCIE1, PCIE3, SGMII_TSEC1, PCIE2},
	[0x07] = {PCIE1, PCIE3, SGMII_TSEC1, PCIE2},
	[0x09] = {PCIE1, NONE, NONE, NONE},
	[0x0a] = {PCIE1, PCIE3, SGMII_TSEC1, SGMII_TSEC2},
	[0x0b] = {PCIE1, PCIE3, SGMII_TSEC1, SGMII_TSEC2},
	[0x0d] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x0e] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x0f] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x15] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x16] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x17] = {PCIE1, PCIE3, PCIE2, PCIE2},
	[0x18] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x19] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1a] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1b] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1c] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x1d] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1e] = {PCIE1, PCIE1, PCIE2, PCIE2},
	[0x1f] = {PCIE1, PCIE1, PCIE2, PCIE2},
};

static const u8 serdes2_cfg_tbl[][SRDS2_MAX_LANES] = {
	[0x00] = {PCIE3, PCIE3},
	[0x01] = {PCIE2, PCIE3},
	[0x02] = {SATA1, SATA2},
	[0x03] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x04] = {NONE, NONE},
	[0x06] = {SATA1, SATA2},
	[0x07] = {NONE, NONE},
	[0x09] = {PCIE3, PCIE2},
	[0x0a] = {SATA1, SATA2},
	[0x0b] = {NONE, NONE},
	[0x0d] = {PCIE3, PCIE2},
	[0x0e] = {SATA1, SATA2},
	[0x0f] = {NONE, NONE},
	[0x15] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x16] = {SATA1, SATA2},
	[0x17] = {NONE, NONE},
	[0x18] = {PCIE3, PCIE3},
	[0x19] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x1a] = {SATA1, SATA2},
	[0x1b] = {NONE, NONE},
	[0x1c] = {PCIE3, PCIE3},
	[0x1d] = {SGMII_TSEC1, SGMII_TSEC2},
	[0x1e] = {SATA1, SATA2},
	[0x1f] = {NONE, NONE},
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
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
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
