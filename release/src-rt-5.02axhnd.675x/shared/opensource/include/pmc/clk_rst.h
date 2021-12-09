/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

#ifndef CLK_RST_H
#define CLK_RST_H

// pll dividers 
struct PLL_DIVIDERS
{
	unsigned int pdiv;
	unsigned int ndiv_int;
	unsigned int ndiv_frac;
	unsigned int ka;
	unsigned int ki;
	unsigned int kp;
};

int pll_vco_freq_set(unsigned int pll_addr, struct PLL_DIVIDERS *divs);
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv);
int pll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int* freq);
int ddr_freq_set(unsigned long freq);
int viper_freq_set(unsigned long freq);
int rdp_freq_set(unsigned long freq);
unsigned long get_rdp_freq(unsigned int* rdp_freq);
#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96878_) || defined(CONFIG_BCM96878)
int pll_vco_freq_get(unsigned int pll_addr, unsigned int* fvco);
#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96878_) || defined(CONFIG_BCM96878)
int pll_ch_freq_vco_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv, unsigned int use_vco);
#endif
#endif

void set_vreg_clk(void);

#if defined(CONFIG_BCM96858)

typedef enum
{
    BCM_CPU_CLK_HIGH,
    BCM_CPU_CLK_LOW
} BCM_CPU_CLK;

int bcm_change_cpu_clk(BCM_CPU_CLK clock);

#endif

#if defined(_BCM96848_) || defined(CONFIG_BCM96848)
#define SYSPLL0_MIPS_CHANNEL     0
#define SYSPLL0_RUNNER_CHANNEL   1
#define SYSPLL0_UBUS_CHANNEL     2
#define SYSPLL0_HSPIM_CHANNEL    3
#define SYSPLL0_PCM_CHANNEL      4
#define SYSPLL0_RGMII_CHANNEL    5
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
#define XRDPPLL_RUNNER_CHANNEL   0
#endif
#if defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96846_) || defined(CONFIG_BCM96846)
#define XRDPPLL_RUNNER_CHANNEL   1
#endif
#if defined(_BCM96878_) || defined(CONFIG_BCM96878)
#define SYSPLL_RUNNER_CHANNEL   0
#endif
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
/* TBD. Verify value. */
#define XRDPPLL_RUNNER_CHANNEL   1
#endif
#endif //#ifndef CLK_RST_H
