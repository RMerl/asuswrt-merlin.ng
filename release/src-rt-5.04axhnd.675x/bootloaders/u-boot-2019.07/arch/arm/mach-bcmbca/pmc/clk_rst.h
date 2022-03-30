// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 
*/

#ifndef CLK_RST_H
#define CLK_RST_H

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_)||defined(CONFIG_BCM9##num)||\
				defined(CONFIG_BCM##num))
#endif

// pll dividers 
struct PLL_DIVIDERS {
	unsigned int pdiv;
	unsigned int ndiv_int;
	unsigned int ndiv_frac;
	unsigned int ka;
	unsigned int ki;
	unsigned int kp;
};

int pll_vco_freq_set(unsigned int pll_addr, struct PLL_DIVIDERS *divs);
int pll_ch_reset(unsigned int pll_addr, unsigned int ch, unsigned int pll_reg_offset);
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv);
int pll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq);
int ddr_freq_set(unsigned long freq);
int viper_freq_set(unsigned long freq);
int rdp_freq_set(unsigned long freq);
unsigned long get_rdp_freq(unsigned int *rdp_freq);
#if IS_BCMCHIP(6858) || IS_BCMCHIP(6856) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
int pll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco);
int pll_ch_freq_vco_set(unsigned int pll_addr, unsigned int ch,
			unsigned int mdiv, unsigned int use_vco);
#endif

#if IS_BCMCHIP(6855)
int pll_vco_config(unsigned int pll_addr, unsigned int ndivider, unsigned int pdivider);
#endif

void set_vreg_clk(void);

#if IS_BCMCHIP(6858)

typedef enum {
	BCM_CPU_CLK_HIGH,
	BCM_CPU_CLK_LOW
} BCM_CPU_CLK;

int bcm_change_cpu_clk(BCM_CPU_CLK clock);

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
#endif //#ifndef CLK_RST_H
