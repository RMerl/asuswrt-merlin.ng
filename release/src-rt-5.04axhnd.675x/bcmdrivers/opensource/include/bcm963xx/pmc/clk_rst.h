/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef CLK_RST_H
#define CLK_RST_H

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_)||defined(CONFIG_BCM9##num)||\
				defined(CONFIG_BCM##num))
#endif

/* pll dividers */
struct PLL_DIVIDERS {
	unsigned int pdiv;
	unsigned int ndiv_int;
	unsigned int ndiv_frac;
	unsigned int ka;
	unsigned int ki;
	unsigned int kp;
};

int pll_vco_freq_set(unsigned int pll_addr, struct PLL_DIVIDERS *divs);
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv);
int pll_ch_freq_set_offs(unsigned int pll_addr, unsigned int ch, unsigned int mdiv, unsigned int offset);
int ddr_freq_set(unsigned long freq);
int viper_freq_set(unsigned long freq);
int rdp_freq_set(unsigned long freq);
unsigned long get_rdp_freq(unsigned int *rdp_freq);
int biu_ch_freq_get(unsigned int ch, unsigned int *freq);

#if !defined(CONFIG_BRCM_IKOS) && (IS_BCMCHIP(6858) || IS_BCMCHIP(6846) || \
	IS_BCMCHIP(6856) || IS_BCMCHIP(6878) || IS_BCMCHIP(63158) || \
	IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(63146))
int bcm_enable_xtal_clk(void);
int bcm_disable_xtal_clk(void);
#else
static inline int bcm_enable_xtal_clk(void)
{
	return 0;
}
static inline int bcm_disable_xtal_clk(void)
{
	return 0;
}
#endif

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6856) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
int pll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco);
int pll_ch_freq_vco_set(unsigned int pll_addr, unsigned int ch,
			unsigned int mdiv, unsigned int use_vco);
#endif

#if IS_BCMCHIP(6855)
int pll_vco_config(unsigned int pll_addr, unsigned int ndivider, unsigned int pdivider);
#endif

void set_vreg_clk(void);

#if IS_BCMCHIP(4908) || IS_BCMCHIP(6858)

typedef enum {
	BCM_CPU_CLK_HIGH,
	BCM_CPU_CLK_LOW
} BCM_CPU_CLK;

int bcm_change_cpu_clk(BCM_CPU_CLK clock);
unsigned int get_cluster_clk_pattern(void);
void set_cluster_clk_pattern(unsigned int pattern);
void reset_cluster_clock(void);
#endif

void set_spike_mitigation(unsigned int spike_us);
#if IS_BCMCHIP(63148)
void set_b15_mdiv(unsigned int value);
#endif

#if IS_BCMCHIP(6858) || IS_BCMCHIP(6855)
#define XRDPPLL_RUNNER_CHANNEL   0
#endif
#if IS_BCMCHIP(6856) || IS_BCMCHIP(6846)
#define XRDPPLL_RUNNER_CHANNEL   1
#endif
#if IS_BCMCHIP(6878)
#define SYSPLL_RUNNER_CHANNEL   0
#endif
#if IS_BCMCHIP(63158)
/* TBD. Verify value. */
#define XRDPPLL_RUNNER_CHANNEL   1
#endif
#endif /* #ifndef CLK_RST_H */
