// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2020 NXP
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>

struct serdes_config {
	u8 protocol;
	u8 lanes[SRDS_MAX_LANES];
};

#if defined(CONFIG_ARCH_LX2162A)
static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x01, {PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x02, {SGMII6, SGMII5, SGMII4, SGMII3 } },
	{0x03, {XFI6, XFI5, XFI4, XFI3 } },
	{0x09, {SGMII6, SGMII5, SGMII4, PCIE1 } },
	{0x0B, {SGMII6, SGMII5, PCIE1, PCIE1 } },
	{0x0F, {_50GE2, _50GE2, _50GE1, _50GE1 } },
	{0x10, {_25GE6, _25GE5, _50GE1, _50GE1 } },
	{0x11, {_25GE6, _25GE5, _25GE4, _25GE3 } },
	{0x12, {_25GE6, _25GE5, XFI4, XFI3 } },
	{0x14, {_40GE1, _40GE1, _40GE1, _40GE1 } },
	{}
};
#else
static struct serdes_config serdes1_cfg_tbl[] = {
	/* SerDes 1 */
	{0x01, {PCIE2, PCIE2, PCIE2, PCIE2, PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x02, {PCIE2, PCIE2, PCIE2, PCIE2, SGMII6, SGMII5, SGMII4, SGMII3 } },
	{0x03, {PCIE2, PCIE2, PCIE2, PCIE2, XFI6, XFI5, XFI4,
		XFI3 } },
	{0x04, {SGMII10, SGMII9, SGMII8, SGMII7, SGMII6, SGMII5, SGMII4,
		SGMII3 } },
	{0x05, {XFI10, XFI9, XFI8, XFI7, PCIE1, PCIE1, PCIE1,
		PCIE1 } },
	{0x06, {SGMII10, SGMII9, SGMII8, SGMII7, SGMII6, SGMII5, XFI4,
		XFI3 } },
	{0x07, {SGMII10, SGMII9, SGMII8, SGMII7, XFI6, XFI5, XFI4,
		XFI3 } },
	{0x08, {XFI10, XFI9, XFI8, XFI7, XFI6, XFI5, XFI4, XFI3 } },
	{0x09, {SGMII10, SGMII9, SGMII8, PCIE2, SGMII6, SGMII5, SGMII4,
		PCIE1 } },
	{0x0A, {XFI10, XFI9, XFI8, PCIE2, XFI6, XFI5, XFI4, PCIE1 } },
	{0x0B, {SGMII10, SGMII9, PCIE2, PCIE2, SGMII6, SGMII5, PCIE1, PCIE1 } },
	{0x0C, {SGMII10, SGMII9, PCIE2, PCIE2, PCIE1, PCIE1, PCIE1, PCIE1 } },
	{0x0D, {_100GE2, _100GE2, _100GE2, _100GE2, _100GE1, _100GE1, _100GE1,
		_100GE1 } },
	{0x0E, {PCIE2, PCIE2, PCIE2, PCIE2, _100GE1, _100GE1, _100GE1,
		_100GE1 } },
	{0x0F, {PCIE2, PCIE2, PCIE2, PCIE2, _50GE2, _50GE2, _50GE1, _50GE1 } },
	{0x10, {PCIE2, PCIE2, PCIE2, PCIE2, _25GE6, _25GE5, _50GE1, _50GE1 } },
	{0x11, {PCIE2, PCIE2, PCIE2, PCIE2, _25GE6, _25GE5, _25GE4, _25GE3 } },
	{0x12, {XFI10, XFI9, XFI8, XFI7, _25GE6, _25GE5, XFI4,
		XFI3 } },
	{0x13, {_40GE2, _40GE2, _40GE2, _40GE2, _25GE6, _25GE5, XFI4, XFI3 } },
	{0x14, {_40GE2, _40GE2, _40GE2, _40GE2, _40GE1, _40GE1, _40GE1,
		_40GE1 } },
	{0x15, {_25GE10, _25GE9, PCIE2, PCIE2, _25GE6, _25GE5, _25GE4,
		_25GE3 } },
	{0x16, {XFI10, XFI9, PCIE2, PCIE2, XFI6, XFI5, XFI4, XFI3 } },
	{}
};
#endif

static struct serdes_config serdes2_cfg_tbl[] = {
	/* SerDes 2 */
	{0x01, {PCIE3, PCIE3, SATA1, SATA2, PCIE4, PCIE4, PCIE4, PCIE4 } },
	{0x02, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3, PCIE3 } },
	{0x03, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, PCIE4, PCIE4 } },
	{0x04, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, SATA1, SATA2 } },
	{0x05, {PCIE3, PCIE3, PCIE3, PCIE3, SATA3, SATA4, SATA1, SATA2 } },
	{0x06, {PCIE3, PCIE3, PCIE3, PCIE3, SGMII15, SGMII16, XFI13,
		XFI14 } },
	{0x07, {PCIE3, SGMII12, SGMII17, SGMII18, PCIE4, SGMII16, XFI13,
		XFI14 } },
	{0x08, {NONE, NONE, SATA1, SATA2, SATA3, SATA4, XFI13, XFI14 } },
	{0x09, {SGMII11, SGMII12, SGMII17, SGMII18, SGMII15, SGMII16, SGMII13,
		SGMII14} },
	{0x0A, {SGMII11, SGMII12, SGMII17, SGMII18, PCIE4, PCIE4, PCIE4,
		PCIE4 } },
	{0x0B, {PCIE3, SGMII12, SGMII17, SGMII18, PCIE4, SGMII16, SGMII13,
		SGMII14 } },
	{0x0C, {SGMII11, SGMII12, SGMII17, SGMII18, PCIE4, PCIE4, SATA1,
		SATA2 } },
	{0x0D, {PCIE3, PCIE3, PCIE3, PCIE3, PCIE4, PCIE4, SGMII13, SGMII14 } },
	{0x0E, {PCIE3, PCIE3, SGMII17, SGMII18, PCIE4, PCIE4, SGMII13,
		SGMII14 } },
	{}
};

static struct serdes_config serdes3_cfg_tbl[] = {
	/* SerDes 3 */
	{0x02, {PCIE5, PCIE5, PCIE5, PCIE5, PCIE5, PCIE5, PCIE5, PCIE5 } },
	{0x03, {PCIE5, PCIE5, PCIE5, PCIE5, PCIE6, PCIE6, PCIE6, PCIE6 } },
	{}
};

static struct serdes_config *serdes_cfg_tbl[] = {
	serdes1_cfg_tbl,
	serdes2_cfg_tbl,
	serdes3_cfg_tbl,
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
