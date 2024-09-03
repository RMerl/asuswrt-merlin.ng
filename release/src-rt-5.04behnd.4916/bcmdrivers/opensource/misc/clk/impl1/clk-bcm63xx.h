/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/

#ifndef __CLK_BCM63XX_H__
#define __CLK_BCM63XX_H__

#include <linux/clk-provider.h>

/*
 * struct bcm63xx_cpuclk - bcm63xx cpu clock structure
 * @hw: clk_hw for cpuclk
 * @clk_name: clock name
 * @pllclk: cpu pll clk
 * @mdiv: pll mdiv for cpu base clock which is pllclk/mdiv
 * @ratio: for arm core channel, additional clock ratio applies to pll output
 * @ratio_base: ratio fraction base. Fcpu = Fpll*ratio/ratio_base
 */
struct bcm63xx_cpuclk {
	struct clk_hw hw;
	const char *clk_name;
	unsigned long pllclk;
	int mdiv;
	int ratio;
	int ratio_base;
	void __iomem *reg;
};

//#define CLK_BCM_DEBUG
#ifdef CLK_BCM_DEBUG
#define CLK_BCM_LOG(fmt, args...) printk("clk-bcm63xx: %s cpu %d " fmt,  __FUNCTION__, smp_processor_id(), ##args)
#else
#define CLK_BCM_LOG(fmt, args...)
#endif
#define FREQ_MHZ(x)	((x)*1000UL*1000UL)
#define to_bcm63xx_cpuclk(p) container_of(p, struct bcm63xx_cpuclk, hw)

int init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk);
int get_arm_core_ratio(struct bcm63xx_cpuclk *cpuclk, int *ratio, int *ratio_base, int *mdiv);
int set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate);
long round_arm_core_rate(struct bcm63xx_cpuclk *cpuclk, unsigned long rate);

#endif
