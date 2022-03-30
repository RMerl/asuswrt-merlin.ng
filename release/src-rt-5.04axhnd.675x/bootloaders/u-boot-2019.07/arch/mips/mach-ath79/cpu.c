// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ath79.h>
#include <mach/ar71xx_regs.h>

struct ath79_soc_desc {
	const enum ath79_soc_type soc;
	const char *chip;
	const int major;
	const int minor;
};

static const struct ath79_soc_desc desc[] = {
	{ATH79_SOC_AR7130,      "7130",
	 REV_ID_MAJOR_AR71XX,   AR71XX_REV_ID_MINOR_AR7130},
	{ATH79_SOC_AR7141,      "7141",
	 REV_ID_MAJOR_AR71XX,   AR71XX_REV_ID_MINOR_AR7141},
	{ATH79_SOC_AR7161,      "7161",
	 REV_ID_MAJOR_AR71XX,   AR71XX_REV_ID_MINOR_AR7161},
	{ATH79_SOC_AR7240,      "7240", REV_ID_MAJOR_AR7240,    0},
	{ATH79_SOC_AR7241,      "7241", REV_ID_MAJOR_AR7241,    0},
	{ATH79_SOC_AR7242,      "7242", REV_ID_MAJOR_AR7242,    0},
	{ATH79_SOC_AR9130,      "9130",
	 REV_ID_MAJOR_AR913X,   AR913X_REV_ID_MINOR_AR9130},
	{ATH79_SOC_AR9132,      "9132",
	 REV_ID_MAJOR_AR913X,   AR913X_REV_ID_MINOR_AR9132},
	{ATH79_SOC_AR9330,      "9330", REV_ID_MAJOR_AR9330,    0},
	{ATH79_SOC_AR9331,      "9331", REV_ID_MAJOR_AR9331,    0},
	{ATH79_SOC_AR9341,      "9341", REV_ID_MAJOR_AR9341,    0},
	{ATH79_SOC_AR9342,      "9342", REV_ID_MAJOR_AR9342,    0},
	{ATH79_SOC_AR9344,      "9344", REV_ID_MAJOR_AR9344,    0},
	{ATH79_SOC_QCA9533,     "9533", REV_ID_MAJOR_QCA9533,   0},
	{ATH79_SOC_QCA9533,     "9533",
	 REV_ID_MAJOR_QCA9533_V2,       0},
	{ATH79_SOC_QCA9556,     "9556", REV_ID_MAJOR_QCA9556,   0},
	{ATH79_SOC_QCA9558,     "9558", REV_ID_MAJOR_QCA9558,   0},
	{ATH79_SOC_TP9343,      "9343", REV_ID_MAJOR_TP9343,    0},
	{ATH79_SOC_QCA9561,     "9561", REV_ID_MAJOR_QCA9561,   0},
};

int mach_cpu_init(void)
{
	void __iomem *base;
	enum ath79_soc_type soc = ATH79_SOC_UNKNOWN;
	u32 id, major, minor = 0;
	u32 rev = 0, ver = 1;
	int i;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);

	id = readl(base + AR71XX_RESET_REG_REV_ID);
	major = id & REV_ID_MAJOR_MASK;
	switch (major) {
	case REV_ID_MAJOR_AR71XX:
	case REV_ID_MAJOR_AR913X:
		minor = id & AR71XX_REV_ID_MINOR_MASK;
		rev = id >> AR71XX_REV_ID_REVISION_SHIFT;
		rev &= AR71XX_REV_ID_REVISION_MASK;
		break;

	case REV_ID_MAJOR_QCA9533_V2:
		ver = 2;
		/* drop through */

	case REV_ID_MAJOR_AR9341:
	case REV_ID_MAJOR_AR9342:
	case REV_ID_MAJOR_AR9344:
	case REV_ID_MAJOR_QCA9533:
	case REV_ID_MAJOR_QCA9556:
	case REV_ID_MAJOR_QCA9558:
	case REV_ID_MAJOR_TP9343:
	case REV_ID_MAJOR_QCA9561:
		rev = id & AR71XX_REV_ID_REVISION2_MASK;
		break;
	default:
		rev = id & AR71XX_REV_ID_REVISION_MASK;
		break;
	}

	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		if ((desc[i].major == major) &&
		    (desc[i].minor == minor)) {
			soc = desc[i].soc;
			break;
		}
	}

	gd->arch.id = id;
	gd->arch.soc = soc;
	gd->arch.rev = rev;
	gd->arch.ver = ver;
	return 0;
}

int print_cpuinfo(void)
{
	enum ath79_soc_type soc = ATH79_SOC_UNKNOWN;
	const char *chip = "????";
	u32 id, rev, ver;
	int i;

	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		if (desc[i].soc == gd->arch.soc) {
			chip = desc[i].chip;
			soc = desc[i].soc;
			break;
		}
	}

	id = gd->arch.id;
	rev = gd->arch.rev;
	ver = gd->arch.ver;

	switch (soc) {
	case ATH79_SOC_QCA9533:
	case ATH79_SOC_QCA9556:
	case ATH79_SOC_QCA9558:
	case ATH79_SOC_QCA9561:
		printf("Qualcomm Atheros QCA%s ver %u rev %u\n", chip,
		       ver, rev);
		break;
	case ATH79_SOC_TP9343:
		printf("Qualcomm Atheros TP%s rev %u\n", chip, rev);
		break;
	case ATH79_SOC_UNKNOWN:
		printf("ATH79: unknown SoC, id:0x%08x", id);
		break;
	default:
		printf("Atheros AR%s rev %u\n", chip, rev);
	}

	return 0;
}
