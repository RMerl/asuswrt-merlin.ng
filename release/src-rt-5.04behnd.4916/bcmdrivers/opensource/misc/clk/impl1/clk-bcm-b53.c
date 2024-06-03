/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/

#include <linux/kernel.h>
#include <linux/version.h>
#include "clk_rst.h"
#include "clk-bcm63xx.h"

/* frequency to clock control pattern mapping */
struct  bcm63xx_freq_pattern {
	int ratio;
	unsigned int pattern;
};

static const struct bcm63xx_freq_pattern freq_pattern[] = {
	{ 32, 0xffffffff },  // 100.00% [32/32]
	{ 28, 0x7f7f7f7f },  //  87.50% [28/32]
	{ 24, 0x5f5f5f5f },  //  75.00% [24/32]
	{ 20, 0x57575757 },  //  62.50% [20/32]
	{ 16, 0x55555555 },  //  50.00% [16/32]
	{ 12, 0x49494949 },  //  37.50% [12/32]
	{ 11, 0x49249249 },  //  34.37% [11/32]
	{  8, 0x11111111 },  //  25.00%  [8/32]
	{  4, 0x01010101 },  //  12.50%  [4/32]
	{  0, 0 }
};

/* __builtin_popcount doesn't seem any better */
static inline unsigned popcount(unsigned v)
{
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return (((v + (v >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
}

/* round down the supported clock ratio from the pattern table and return
   the index to pattern */
static int round_clk_ratio_and_index(int* ratio)
{
	int i = 0, table_ratio;

	CLK_BCM_LOG("find_clk_ratio input ratio %d\n", *ratio);
	while ((table_ratio = freq_pattern[i].ratio)) {
		if ( table_ratio == 0) {
			i = i - 1; /* round to smallest possible ratio */
			*ratio = freq_pattern[i].ratio;
			break;
		}

		if (*ratio >= table_ratio) {
			*ratio = table_ratio;
			break;
		}
		i++;
	}
	CLK_BCM_LOG("return ratio %d and index %d\n", *ratio, i);
	return i;
}

int get_arm_core_ratio(struct bcm63xx_cpuclk *cpuclk, int *ratio, int *ratio_base, int *mdiv)
{
#if defined(CONFIG_BCM_PMC)
	u32 pattern = 0;
    pattern = get_cluster_clk_pattern();	
	CLK_BCM_LOG("pattern register 0x%x\n", pattern);
	/* no need to update mdiv. Fixed post divider */
	*ratio_base = 32;
	*ratio = popcount(pattern);
#endif

	return 0;
}

long round_arm_core_rate(struct bcm63xx_cpuclk *cpuclk, unsigned long rate)
{
	long new_rate = 0;

	/* b53 always use fix pll mdiv_in(2 by default) output(base_rate) and then apply cpu 
	   core ratio between 1 and 1/32. Only support ratio in the freq_pattern table */
	unsigned long cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int ratio = (rate*32)/cpu_base_rate;

	round_clk_ratio_and_index(&ratio);
	new_rate = (cpu_base_rate*ratio)/32;

	return new_rate;

}

int set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate)
{
	int cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int ratio = (rate*32)/cpu_base_rate, index;

	index = round_clk_ratio_and_index(&ratio);

#if defined(CONFIG_BCM_PMC)
	/* apply clock ratio pattern */
	set_cluster_clk_pattern(freq_pattern[index].pattern);
#endif

	return 0;
}

int init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk)
{
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908)
#if defined(CONFIG_BCM96858)
	cpuclk->pllclk = FREQ_MHZ(1500);
#elif defined(CONFIG_BCM94908)
	cpuclk->pllclk = FREQ_MHZ(1800);
#endif
	cpuclk->mdiv = 1;
#if defined(CONFIG_BCM_PMC)
    reset_cluster_clock();
#endif
#endif
	return 0;
}
