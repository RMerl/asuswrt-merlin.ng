// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>

struct serdes_config {
	u8 protocol;
	u8 lanes[SRDS_MAX_LANES];
};

static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x03, {PCIE2, PCIE2, PCIE2, PCIE2, PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x05, {PCIE2, PCIE2, PCIE2, PCIE2, SGMII4, SGMII3, SGMII2, SGMII1 } },
	{0x07, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, SGMII2,
		SGMII1 } },
	{0x09, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, SGMII2,
		SGMII1 } },
	{0x0A, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, SGMII2,
		SGMII1 } },
	{0x0C, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, SGMII2,
		SGMII1 } },
	{0x0E, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, SGMII2,
		SGMII1 } },
	{0x26, {SGMII8, SGMII7, SGMII6, SGMII5, SGMII4, SGMII3, XFI2, XFI1 } },
	{0x28, {SGMII8, SGMII7, SGMII6, SGMII5, XFI4, XFI3, XFI2, XFI1 } },
	{0x2A, {XFI8, XFI7, XFI6, XFI5, XFI4, XFI3, XFI2, XFI1 } },
	{0x2B, {SGMII8, SGMII7, SGMII6, SGMII5, XAUI1, XAUI1, XAUI1, XAUI1  } },
	{0x32, {XAUI2, XAUI2, XAUI2, XAUI2, XAUI1, XAUI1, XAUI1, XAUI1  } },
	{0x33, {PCIE2, PCIE2, PCIE2, PCIE2, QSGMII_D, QSGMII_C, QSGMII_B,
		QSGMII_A} },
	{0x35, {QSGMII_D, QSGMII_C, QSGMII_B, PCIE2, XFI4, XFI3, XFI2, XFI1 } },
	{0x39, {SGMII8, SGMII7, SGMII6, PCIE2, SGMII4, SGMII3, SGMII2,
		PCIE1 } },
	{0x3B, {XFI8, XFI7, XFI6, PCIE2, XFI4, XFI3, XFI2, PCIE1 } },
	{0x4B, {PCIE2, PCIE2, PCIE2, PCIE2, XFI4, XFI3, XFI2, XFI1 } },
	{0x4C, {XFI8, XFI7, XFI6, XFI5, PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x4D, {SGMII8, SGMII7, PCIE2, PCIE2, SGMII4, SGMII3, PCIE1, PCIE1 } },
		{}
};
static struct serdes_config serdes2_cfg_tbl[] = {
	/* SerDes 2 */
	{0x07, {SGMII9, SGMII10, SGMII11, SGMII12, SGMII13, SGMII14, SGMII15,
		SGMII16 } },
	{0x09, {SGMII9, SGMII10, SGMII11, SGMII12, SGMII13, SGMII14, SGMII15,
		SGMII16 } },
	{0x0A, {SGMII9, SGMII10, SGMII11, SGMII12, SGMII13, SGMII14, SGMII15,
		SGMII16 } },
	{0x0C, {SGMII9, SGMII10, SGMII11, SGMII12, SGMII13, SGMII14, SGMII15,
		SGMII16 } },
	{0x0E, {SGMII9, SGMII10, SGMII11, SGMII12, SGMII13, SGMII14, SGMII15,
		SGMII16 } },
	{0x3D, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3 } },
	{0x3E, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3 } },
	{0x3F, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, PCIE4, PCIE4 } },
	{0x40, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, PCIE4, PCIE4 } },
	{0x41, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, SATA1, SATA2 } },
	{0x42, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, SATA1, SATA2 } },
	{0x43, {PCIE3, PCIE3, PCIE3, PCIE3, NONE, NONE, SATA1, SATA2 } },
	{0x44, {PCIE3, PCIE3, PCIE3, PCIE3, NONE, NONE, SATA1, SATA2 } },
	{0x45, {SGMII9, SGMII10, SGMII11, SGMII12, PCIE4, PCIE4, PCIE4,
		PCIE4 } },
	{0x47, {PCIE3, SGMII10, SGMII11, SGMII12, PCIE4, SGMII14, SGMII15,
		SGMII16 } },
	{0x49, {SGMII9, SGMII10, SGMII11, SGMII12, PCIE4, PCIE4, SATA1,
		SATA2 } },
	{0x4A, {SGMII9, SGMII10, SGMII11, SGMII12, PCIE4, PCIE4, SATA1,
		SATA2 } },
	{0x51, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, PCIE4, PCIE4 } },
	{0x57, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, SGMII15, SGMII16 } },
	{}
};

static struct serdes_config *serdes_cfg_tbl[] = {
	serdes1_cfg_tbl,
	serdes2_cfg_tbl,
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
