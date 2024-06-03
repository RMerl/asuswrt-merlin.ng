// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_serdes.h>
#include <asm/processor.h>
#include <asm/io.h>


static u8 serdes_cfg_tbl[][SRDS_MAX_LANES] = {
	[0x40] = {PCIE1, PCIE1, PCIE1, PCIE1},
	[0xD5] = {QSGMII_FM1_A, PCIE3, PCIE2, PCIE1},
	[0xD6] = {QSGMII_FM1_A, PCIE3, PCIE2, SATA1},
	[0x95] = {XFI_FM1_MAC1, PCIE3, PCIE2, PCIE1},
	[0x99] = {XFI_FM1_MAC1, PCIE3, SGMII_FM1_DTSEC2, PCIE1},
	[0x46] = {PCIE1, PCIE1, PCIE2, SATA1},
	[0x47] = {PCIE1, PCIE1, PCIE2, SGMII_FM1_DTSEC1},
	[0x56] = {PCIE1, PCIE3, PCIE2, SATA1},
	[0x5A] = {PCIE1, PCIE3, SGMII_FM1_DTSEC2, SATA1},
	[0x5B] = {PCIE1, PCIE3, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC1},
	[0x5F] = {PCIE1, PCIE3, SGMII_2500_FM1_DTSEC2, SGMII_2500_FM1_DTSEC1},
	[0x6A] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC2, SATA1},
	[0x6B] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_FM1_DTSEC2, SGMII_FM1_DTSEC1},
	[0x6F] = {PCIE1, SGMII_FM1_DTSEC3, SGMII_2500_FM1_DTSEC2,
		  SGMII_2500_FM1_DTSEC1},
	[0x77] = {PCIE1, SGMII_2500_FM1_DTSEC3, PCIE2, SGMII_FM1_DTSEC1},
	[0x7F] = {PCIE1, SGMII_2500_FM1_DTSEC3, SGMII_2500_FM1_DTSEC2,
		  SGMII_2500_FM1_DTSEC1},
	[0x119] = {AURORA, PCIE3, SGMII_FM1_DTSEC2, PCIE1},
	[0x135] = {AURORA, SGMII_2500_FM1_DTSEC3, PCIE2, PCIE1},
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
