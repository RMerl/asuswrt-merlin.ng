// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_lsch2.h>

struct serdes_config {
	u32 protocol;
	u8 lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	{0x2208, {SGMII_2500_FM1_DTSEC1, SGMII_2500_FM1_DTSEC2, NONE, SATA1} },
	{0x0008, {NONE, NONE, NONE, SATA1} },
	{0x3508, {SGMII_FM1_DTSEC1, PCIE1, NONE, SATA1} },
	{0x3305, {SGMII_FM1_DTSEC1, SGMII_FM1_DTSEC2, NONE, PCIE1} },
	{0x2205, {SGMII_2500_FM1_DTSEC1, SGMII_2500_FM1_DTSEC2, NONE, PCIE1} },
	{0x2305, {SGMII_2500_FM1_DTSEC1, SGMII_FM1_DTSEC2, NONE, PCIE1} },
	{0x9508, {TX_CLK, PCIE1, NONE, SATA1} },
	{0x3905, {SGMII_FM1_DTSEC1, TX_CLK, NONE, PCIE1} },
	{0x9305, {TX_CLK, SGMII_FM1_DTSEC2, NONE, PCIE1} },
	{}
};

static struct serdes_config *serdes_cfg_tbl[] = {
	serdes1_cfg_tbl,
};

enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane)
{
	struct serdes_config *ptr;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	ptr = serdes_cfg_tbl[serdes];
	while (ptr->protocol) {
		if (ptr->protocol == cfg)
			return ptr->lanes[lane];
		ptr++;
	}

	return 0;
}

int is_serdes_prtcl_valid(int serdes, u32 prtcl)
{
	int i;
	struct serdes_config *ptr;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	ptr = serdes_cfg_tbl[serdes];
	while (ptr->protocol) {
		if (ptr->protocol == prtcl)
			break;
		ptr++;
	}

	if (!ptr->protocol)
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (ptr->lanes[i] != NONE)
			return 1;
	}

	return 0;
}
