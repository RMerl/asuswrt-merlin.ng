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
