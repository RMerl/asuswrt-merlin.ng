/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Atheros AR71XX/AR724X/AR913X common definitions
 *
 * Copyright (C) 2018-2019 Rosy Song <rosysong@rosinson.com>
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 */

#ifndef __ASM_MACH_ATH79_H
#define __ASM_MACH_ATH79_H

#include <asm/global_data.h>
#include <linux/types.h>

DECLARE_GLOBAL_DATA_PTR;

enum ath79_soc_type {
	ATH79_SOC_UNKNOWN,
	ATH79_SOC_AR7130,
	ATH79_SOC_AR7141,
	ATH79_SOC_AR7161,
	ATH79_SOC_AR7240,
	ATH79_SOC_AR7241,
	ATH79_SOC_AR7242,
	ATH79_SOC_AR9130,
	ATH79_SOC_AR9132,
	ATH79_SOC_AR9330,
	ATH79_SOC_AR9331,
	ATH79_SOC_AR9341,
	ATH79_SOC_AR9342,
	ATH79_SOC_AR9344,
	ATH79_SOC_QCA9533,
	ATH79_SOC_QCA9556,
	ATH79_SOC_QCA9558,
	ATH79_SOC_TP9343,
	ATH79_SOC_QCA9561,
};

static inline int soc_is_ar71xx(void)
{
	return gd->arch.soc == ATH79_SOC_AR7130 ||
		gd->arch.soc == ATH79_SOC_AR7141 ||
		gd->arch.soc == ATH79_SOC_AR7161;
}

static inline int soc_is_ar724x(void)
{
	return gd->arch.soc == ATH79_SOC_AR7240 ||
		gd->arch.soc == ATH79_SOC_AR7241 ||
		gd->arch.soc == ATH79_SOC_AR7242;
}

static inline int soc_is_ar7240(void)
{
	return gd->arch.soc == ATH79_SOC_AR7240;
}

static inline int soc_is_ar7241(void)
{
	return gd->arch.soc == ATH79_SOC_AR7241;
}

static inline int soc_is_ar7242(void)
{
	return gd->arch.soc == ATH79_SOC_AR7242;
}

static inline int soc_is_ar913x(void)
{
	return gd->arch.soc == ATH79_SOC_AR9130 ||
		gd->arch.soc == ATH79_SOC_AR9132;
}

static inline int soc_is_ar933x(void)
{
	return gd->arch.soc == ATH79_SOC_AR9330 ||
		gd->arch.soc == ATH79_SOC_AR9331;
}

static inline int soc_is_ar9341(void)
{
	return gd->arch.soc == ATH79_SOC_AR9341;
}

static inline int soc_is_ar9342(void)
{
	return gd->arch.soc == ATH79_SOC_AR9342;
}

static inline int soc_is_ar9344(void)
{
	return gd->arch.soc == ATH79_SOC_AR9344;
}

static inline int soc_is_ar934x(void)
{
	return soc_is_ar9341() ||
		soc_is_ar9342() ||
		soc_is_ar9344();
}

static inline int soc_is_qca9533(void)
{
	return gd->arch.soc == ATH79_SOC_QCA9533;
}

static inline int soc_is_qca953x(void)
{
	return soc_is_qca9533();
}

static inline int soc_is_qca9556(void)
{
	return gd->arch.soc == ATH79_SOC_QCA9556;
}

static inline int soc_is_qca9558(void)
{
	return gd->arch.soc == ATH79_SOC_QCA9558;
}

static inline int soc_is_qca955x(void)
{
	return soc_is_qca9556() || soc_is_qca9558();
}

static inline int soc_is_tp9343(void)
{
	return gd->arch.soc == ATH79_SOC_TP9343;
}

static inline int soc_is_qca9561(void)
{
	return gd->arch.soc == ATH79_SOC_QCA9561;
}

static inline int soc_is_qca956x(void)
{
	return soc_is_tp9343() || soc_is_qca9561();
}

u32 ath79_get_bootstrap(void);
int ath79_eth_reset(void);
int ath79_usb_reset(void);

void ar934x_pll_init(const u16 cpu_mhz, const u16 ddr_mhz, const u16 ahb_mhz);
void ar934x_ddr_init(const u16 cpu_mhz, const u16 ddr_mhz, const u16 ahb_mhz);

void qca956x_pll_init(void);
void qca956x_ddr_init(void);
#endif /* __ASM_MACH_ATH79_H */
