// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_ls102xa.h>

static u8 serdes_cfg_tbl[][SRDS_MAX_LANES] = {
	[0x00] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0x10] = {PCIE1, SATA1, PCIE2, PCIE2},
	[0x20] = {PCIE1, SGMII_TSEC1, PCIE2, SGMII_TSEC2},
	[0x30] = {PCIE1, SATA1, SGMII_TSEC1, SGMII_TSEC2},
	[0x40] = {PCIE1, PCIE1, SATA1, SGMII_TSEC2},
	[0x50] = {PCIE1, PCIE1, PCIE2, SGMII_TSEC2},
	[0x60] = {PCIE1, PCIE1, SGMII_TSEC1, SGMII_TSEC2},
	[0x70] = {PCIE1, SATA1, PCIE2, SGMII_TSEC2},
	[0x80] = {PCIE2, PCIE2, PCIE2, PCIE2},
};

enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane)
{
	return serdes_cfg_tbl[cfg][lane];
}

int is_serdes_prtcl_valid(int serdes, u32 prtcl)
{
	int i;

	if (prtcl >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_cfg_tbl[prtcl][i] != NONE)
			return 1;
	}

	return 0;
}
