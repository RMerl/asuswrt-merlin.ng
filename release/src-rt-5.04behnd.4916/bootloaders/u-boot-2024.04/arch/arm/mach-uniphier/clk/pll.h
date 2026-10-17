/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef MACH_PLL_H
#define MACH_PLL_H

#define UNIPHIER_PLL_FREQ_DEFAULT	(0)

void uniphier_ld4_dpll_ssc_en(void);

int uniphier_ld20_sscpll_init(unsigned long reg_base, unsigned int freq,
			      unsigned int ssc_rate, unsigned int divn);
int uniphier_ld20_sscpll_ssc_en(unsigned long reg_base);
int uniphier_ld20_sscpll_set_regi(unsigned long reg_base, unsigned regi);
int uniphier_ld20_vpll27_init(unsigned long reg_base);
int uniphier_ld20_dspll_init(unsigned long reg_base);

#endif /* MACH_PLL_H */
