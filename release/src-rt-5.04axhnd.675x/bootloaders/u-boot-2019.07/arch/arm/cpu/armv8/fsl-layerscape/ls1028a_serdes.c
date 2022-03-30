// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>

struct serdes_config {
	u32 protocol;
	u8 lanes[SRDS_MAX_LANES];
	u8 rcw_lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0xCC5B, {PCIE1, QSGMII_B, PCIE2, PCIE2} },
	{0xEB99, {SGMII1, SGMII1, PCIE2, SATA1} },
	{0xCC99, {SGMII1, SGMII1, PCIE2, PCIE2} },
	{0xBB99, {SGMII1, SGMII1, PCIE2, PCIE1} },
	{0x9999, {SGMII1, SGMII2, SGMII3, SGMII4} },
	{0xEBCC, {PCIE1, PCIE1, PCIE2, SATA1} },
	{0xCCCC, {PCIE1, PCIE1, PCIE2, PCIE2} },
	{0xDDDD, {PCIE1, PCIE1, PCIE1, PCIE1} },
	{0xE031, {SXGMII1, QXGMII2, NONE, SATA1} },
	{0xB991, {SXGMII1, SGMII1, SGMII2, PCIE1} },
	{0xBB31, {SXGMII1, QXGMII2, PCIE1, PCIE1} },
	{0xCC31, {SXGMII1, QXGMII2, PCIE2, PCIE2} },
	{0xBB51, {SXGMII1, QSGMII_B, PCIE2, PCIE1} },
	{0xBB38, {SGMII_T1, QXGMII2, PCIE2, PCIE1} },
	{0xCC38, {SGMII_T1, QXGMII2, PCIE2, PCIE2} },
	{0xBB58, {SGMII_T1, QSGMII_B, PCIE2, PCIE1} },
	{0xCC58, {SGMII_T1, QSGMII_B, PCIE2, PCIE2} },
	{0xCC8B, {PCIE1, SGMII_T1, PCIE2, PCIE2} },
	{0xEB58, {SGMII_T1, QSGMII_B, PCIE2, SATA1} },
	{0xEB8B, {PCIE1, SGMII_T1, PCIE2, SATA1} },
	{0xE8CC, {PCIE1, PCIE1, SGMII_T1, SATA1} },
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
