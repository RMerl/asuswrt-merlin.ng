// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2019 NXP
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/io.h>

struct serdes_config {
	u8 ip_protocol;
	u8 lanes[SRDS_MAX_LANES];
	u8 rcw_lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x12, {SGMII3, SGMII7, SGMII1, SGMII2 }, {3, 3, 3, 3 }  },
	{0x15, {SGMII3, SGMII7, XFI1, XFI2 }, {3, 3, 1, 1 } },
	{0x16, {SGMII3, SGMII7, SGMII1, XFI2 }, {3, 3, 3, 1 } },
	{0x17, {SGMII3, SGMII7, SGMII1, SGMII2 }, {3, 3, 3, 2 } },
	{0x18, {SGMII3, SGMII7, SGMII1, SGMII2 }, {3, 3, 2, 2 } },
	{0x19, {SGMII3, QSGMII_B, XFI1, XFI2}, {3, 4, 1, 1 } },
	{0x1A, {SGMII3, QSGMII_B, SGMII1, XFI2 }, {3, 4, 3, 1 } },
	{0x1B, {SGMII3, QSGMII_B, SGMII1, SGMII2 }, {3, 4, 3, 2 } },
	{0x1C, {SGMII3, QSGMII_B, SGMII1, SGMII2 }, {3, 4, 2, 2 } },
	{0x1D, {QSGMII_A, QSGMII_B, XFI1, XFI2 }, {4, 4, 1, 1 } },
	{0x1E, {QSGMII_A, QSGMII_B, SGMII1, XFI2  }, {4, 4, 3, 1 } },
	{0x1F, {QSGMII_A, QSGMII_B, SGMII1, SGMII2  }, {4, 4, 3, 2 } },
	{0x20, {QSGMII_A, QSGMII_B, SGMII1, SGMII2 }, {4, 4, 2, 2 } },
	{0x35, {SGMII3, QSGMII_B, SGMII1, SGMII2 }, {3, 4, 3, 3 } },
	{0x36, {QSGMII_A, QSGMII_B, SGMII1, SGMII2 }, {4, 4, 3, 3 } },
	{0x3A, {SGMII3, PCIE1, SGMII1, SGMII2 }, {3, 5, 3, 3 } },
		{}
};

static struct serdes_config serdes2_cfg_tbl[] = {
	/* SerDes 2 */
	{0x0C, {PCIE1, PCIE1, PCIE1, PCIE1 }, {8, 8, 8, 8 } },
	{0x0D, {PCIE1, PCIE2, PCIE3, SATA1 }, {5, 5, 5, 9 }  },
	{0x0E, {PCIE1, PCIE1, PCIE2, SATA1 }, {7, 7, 6, 9 }  },
	{0x13, {PCIE1, PCIE1, PCIE3, PCIE3 }, {7, 7, 7, 7 }  },
	{0x14, {PCIE1, PCIE2, PCIE3, PCIE3 }, {5, 5, 7, 7 }  },
	{0x3C, {NONE, PCIE2, NONE, PCIE3 }, {0, 5, 0, 6 }  },
	{}
};

static struct serdes_config *serdes_cfg_tbl[] = {
	serdes1_cfg_tbl,
	serdes2_cfg_tbl,
};

bool soc_has_mac1(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	unsigned int svr = gur_in32(&gur->svr);
	unsigned int version = SVR_SOC_VER(svr);

	return (version == SVR_LS1088A || version == SVR_LS1084A);
}

int serdes_get_number(int serdes, int cfg)
{
	struct serdes_config *ptr;
	int i, j, index, lnk;
	int is_found, max_lane = SRDS_MAX_LANES;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;

	ptr = serdes_cfg_tbl[serdes];

	while (ptr->ip_protocol) {
		is_found = 1;
		for (i = 0, j = max_lane - 1; i < max_lane; i++, j--) {
			lnk = cfg & (0xf << 4 * i);
			lnk = lnk >> (4 * i);

			index = (serdes == FSL_SRDS_1) ? j : i;

			if (ptr->rcw_lanes[index] == lnk && is_found)
				is_found = 1;
			else
				is_found = 0;
		}

		if (is_found)
			return ptr->ip_protocol;
		ptr++;
	}

	return 0;
}

enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane)
{
	struct serdes_config *ptr;

	if (serdes >= ARRAY_SIZE(serdes_cfg_tbl))
		return 0;
	/*
	 * LS1044A/1048A  support only one 10GBase-R port
	 * Disable MAC1 for LS1044A/1048A
	 */
	if (serdes == FSL_SRDS_1 && lane == 2) {
		if (!soc_has_mac1())
			return 0;
	}
	ptr = serdes_cfg_tbl[serdes];
	while (ptr->ip_protocol) {
		if (ptr->ip_protocol == cfg)
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
	while (ptr->ip_protocol) {
		if (ptr->ip_protocol == prtcl)
			break;
		ptr++;
	}

	if (!ptr->ip_protocol)
		return 0;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (ptr->lanes[i] != NONE)
			return 1;
	}

	return 0;
}
