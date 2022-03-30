// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_lsch2.h>

struct serdes_config {
	u32 protocol;
	u8 lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x1555, {XFI_FM1_MAC9, PCIE1, PCIE2, PCIE3} },
	{0x2555, {SGMII_2500_FM1_DTSEC9, PCIE1, PCIE2, PCIE3} },
	{0x4555, {QSGMII_FM1_A, PCIE1, PCIE2, PCIE3} },
	{0x4558, {QSGMII_FM1_A, PCIE1, PCIE2, SATA1} },
	{0x1355, {XFI_FM1_MAC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
	{0x2355, {SGMII_2500_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
	{0x3335, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC5,
		  PCIE3} },
	{0x3355, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, PCIE3} },
	{0x3358, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, PCIE2, SATA1} },
	{0x3555, {SGMII_FM1_DTSEC9, PCIE1, PCIE2, PCIE3} },
	{0x3558, {SGMII_FM1_DTSEC9, PCIE1, PCIE2, SATA1} },
	{0x7000, {PCIE1, PCIE1, PCIE1, PCIE1} },
	{0x9998, {PCIE1, PCIE2, PCIE3, SATA1} },
	{0x6058, {PCIE1, PCIE1, PCIE2, SATA1} },
	{0x1455, {XFI_FM1_MAC9, QSGMII_FM1_A, PCIE2, PCIE3} },
	{0x2455, {SGMII_2500_FM1_DTSEC9, QSGMII_FM1_A, PCIE2, PCIE3} },
	{0x2255, {SGMII_2500_FM1_DTSEC9, SGMII_2500_FM1_DTSEC2, PCIE2, PCIE3} },
	{0x3333, {SGMII_FM1_DTSEC9, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC5,
		  SGMII_FM1_DTSEC6} },
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
