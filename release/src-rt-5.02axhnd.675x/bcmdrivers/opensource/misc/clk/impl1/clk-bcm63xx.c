/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/stop_machine.h>

#include "bcm_map_part.h"
#include "pmc_drv.h"
#include "pmc/BPCM.h"
#include "clk_rst.h"


struct bcm63xx_cpuclk {
	struct clk_hw hw;
	const char *clk_name;
	unsigned long pllclk;
	int mdiv;
	int ratio;
	int ratio_base;
};

//#define CLK_BCM_DEBUG
#ifdef CLK_BCM_DEBUG
#define CLK_BCM_LOG(fmt, args...) printk("clk-bcm63xx: %s cpu %d " fmt,  __FUNCTION__, smp_processor_id(), ##args)
#else
#define CLK_BCM_LOG(fmt, args...)
#endif

#define FREQ_MHZ(x)	((x)*1000UL*1000UL)

/*
 * struct bcm63xx_cpuclk - bcm63xx cpu clock structure
 * @hw: clk_hw for cpuclk
 * @clk_name: clock name
 * @pllclk: cpu pll clk
 * @mdiv: pll mdiv for cpu base clock which is pllclk/mdiv
 * @ratio: for arm core channel, additional clock ratio applies to pll output
 * @ratio_base: ratio fraction base. Fcpu = Fpll*ratio/ratio_base
 */


#if defined(CONFIG_BCM963148)
struct core_set_param {
	int shift;
	unsigned int core_base_rate;
};

#define BUS_RANGE_3_DEFAULT_ULIMIT  0x3ffffU
#define BUS_RANGE_4_DEFAULT_ULIMIT 0x1bffffU

extern void arm_wfi_enable(unsigned int freqHz);
static volatile u32 core_set_freq_done, core_set_freq_core_1_rdy;
#endif

#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)

#if defined(CONFIG_BCM96858)
#define C0_CLK_CONTROL (0x70 >> 2)
#define C0_CLK_PATTERN (0x78 >> 2)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
extern void brcm_legacy_init(struct device_node *np);
#else
extern int brcm_legacy_init(struct device_node *np);
#endif

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
#endif


#define to_bcm63xx_cpuclk(p) container_of(p, struct bcm63xx_cpuclk, hw)

/* __builtin_popcount doesn't seem any better */
static inline unsigned popcount(unsigned v)
{
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return (((v + (v >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
}

/*
 * SoC specific cpu clock functions
 */
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
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
#endif

#if defined(CONFIG_BCM963138)
static int core_set_freq(void *p)
{

	unsigned int mdiv;
	struct bcm63xx_cpuclk *cpuclk = (struct bcm63xx_cpuclk *)p;

	mdiv = cpuclk->mdiv;
	ARMCFG->proc_clk.pllarmc = (ARMCFG->proc_clk.pllarmc & ~ARM_PROC_CLK_PLLARMC_MDIV_MASK) | mdiv;

	return 0;
}
#endif

#if defined(CONFIG_BCM963148)
/*
 * CPU frequency can be changed via the B15 pll or clock-ratio
 *
 * Access to the pll is through bpcm so reads/writes are slow.
 * Access to the clock-ratio is through a fast soc register.
 *
 * To change the frequency from:
 *
 * 1:1 to 1:n
 * - stop all write traffic (i.e. stop all CPUs)
 * - set safe-clock-mode (clock configuration register)
 * - DSB
 * - set clock-divisor (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * 1:n to 1:1
 * - stop all write traffic (i.e. stop all CPUs)
 * - clear clock-divisor (clock configuration register)
 * - DSB
 * - clear safe-clock-mode (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * The configuration changes should be done close together and
 * as quickly as possible to limit the down time for other CPUS.
 * [this makes changing the clock-ratio preferrable to the pll]
 */

static int core_set_freq(void* p)
{
	unsigned ratio = B15CTRL->cpu_ctrl.clock_cfg;
	const unsigned safe_mode = 16;
	struct core_set_param* param = (struct core_set_param*)p;
	int shift = param->shift;

	// only one core running, no idlers;
	// enable/disable wfi for idlers
	arm_wfi_enable(param->core_base_rate >> shift);

	if (shift != 0) {
		//A barrier here to ensure there are no pending memory accesses
		//when entering safe mode.
		smp_wmb();
		//Switching ARM DDR access over to UBUS temporarily. We need to make sure there's no
		//MCP activity when we enter Safe mode.
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
		// set safe_clk_mode if < 1000MHz (2x 500MHz MCP)
		ratio |= safe_mode;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // set safe-mode
		//UBUS fast-ack makes above write operation a posted write.
		//Counter fast-ack by reading back the register. We want to
		//be sure the clock_cfg change has taken effect before
		//moving on.
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();

		ratio = (ratio & ~7) | shift;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // new divisor
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
		//Switching ARM DDR access back to MCP
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
	} else {
		shift = ratio & 7;
		while (shift--) {
			// frequency doubling one step at a time
			ratio = (ratio & ~7) | shift;
			B15CTRL->cpu_ctrl.clock_cfg = ratio;
			//Counter fast-ack
			B15CTRL->cpu_ctrl.clock_cfg;
			if (shift <= 1) {
				// 50us spike mitigation at 750 & 1500MHz
				// tmrctl = enable | microseconds | 50
				PMC->ctrl.gpTmr0Ctl = (1 << 31) | (1 << 29) | 50;
				while (PMC->ctrl.gpTmr0Ctl & (1 << 31));
			}
		}

		//A barrier here to ensure there are no pending memory accesses
		//when exiting safe mode.
		smp_wmb();
		// clear safe_clk_mode if >= 1000MHz (2x 500MHz MCP)
		B15CTRL->cpu_ctrl.clock_cfg = ratio & ~safe_mode; // clear safe-mode
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
	}

	return 0;
}

static int core_set_freq_sync(void *p) {
	//Load variables used into cache. We don't want DDR accesses
	//in the code sequence below.
	(void)core_set_freq_core_1_rdy;
	(void)core_set_freq_done;

	if (smp_processor_id()==0) {
		//Core0 is doing the frequency change. Wait until core1
		//is ready for it. We have to make sure core1 is not
		//doing any memory accesses while core0 is changing
		//CPU frequency.
		//Deliberately using cached variables for inter-core
		//synchronization instead of atomic variables.
		//Atomic variable primitives would generate a memory
		//access because MegaBarriers are used.
		// check if the remote cpu is online
		if(cpumask_test_cpu(1, cpu_online_mask))
			while(!core_set_freq_core_1_rdy);

		core_set_freq(p);
		core_set_freq_done=1;
	}
	else {

		core_set_freq_core_1_rdy=1;
		//Wait until core0 is done changing frequency before moving on.
		while(!core_set_freq_done);

	}

	return 0;
}
#endif

int get_arm_core_ratio(int *ratio, int *ratio_base, int *mdiv)
{
#if defined(CONFIG_BCM963138)
	/* A9 arm core does not have addition cpu clock ratio.
	 * Use only mdiv for pll output channel
	 */

	*ratio = 1;
	*ratio_base = 1;
	*mdiv = (ARMCFG->proc_clk.pllarmc & ARM_PROC_CLK_PLLARMC_MDIV_MASK) >> ARM_PROC_CLK_PLLARMC_MDIV_SHIFT;

#elif defined(CONFIG_BCM963148)
	u32 shift = B15CTRL->cpu_ctrl.clock_cfg&0xf;

	/* no need to update mdiv. Fixed post divider */
	*ratio_base = 16;
	*ratio = 16>>shift;
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
	u32 pattern = 0;
#if defined(CONFIG_BCM94908)
	pattern = BIUCTRL->cluster_clk_pattern[0];
#endif
#if defined(CONFIG_BCM96858)
	ReadBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, &pattern);
#endif
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
#if defined(CONFIG_BCM963138)
	int mdiv;
	/* valid mdiv is 2 to 10 */
	mdiv = cpuclk->pllclk /rate;

	if (mdiv < 2 )
		mdiv = 2;
	else if (mdiv > 10)
		mdiv = 10;

	return cpuclk->pllclk/mdiv;
#elif defined(CONFIG_BCM963148)
	/* b15 always use fix pll mdiv_in(2 by default) output(base_rate) and then apply cpu 
	   core ratio between 1, 1/2, ..., 1/16 to further scale down cpu clock */
	unsigned long cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int shift;

	// find power-of-2 divisor
	for (shift = 0; shift <= 4; shift++)
		if ((cpu_base_rate >> shift) <= rate)
			break;
	if (shift > 4) {
		pr_warn("Invalid cpu frequency %luMHz, limit to smallest possible clock rate\n", 
			rate / FREQ_MHZ(1));
		shift = 4;
	}
	new_rate = cpu_base_rate>>shift;
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
	/* b53 always use fix pll mdiv_in(2 by default) output(base_rate) and then apply cpu 
	   core ratio between 1 and 1/32. Only support ratio in the freq_pattern table */
	unsigned long cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int ratio = (rate*32)/cpu_base_rate;

	round_clk_ratio_and_index(&ratio);
	new_rate = (cpu_base_rate*ratio)/32;
#endif
	return new_rate;

}

int set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate)
{
#if defined(CONFIG_BCM963138)
	const struct cpumask *cpus;

	cpuclk->mdiv = cpuclk->pllclk/rate;

	/* tie up cores to change frequency */
	cpus = cpumask_of(smp_processor_id());
	/* interrupts disabled in stop_machine */
	stop_machine(core_set_freq, cpuclk, cpus);
#elif defined(CONFIG_BCM963148)
	int cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int shift;
	struct core_set_param param;

	// find power-of-2 divisor
	for (shift = 0; shift <= 4; shift++)
		if ((cpu_base_rate >> shift) == rate)
			break;
	if (shift > 4) {
		pr_warn("Invalid cpu frequency %luMHz\n", rate / FREQ_MHZ(1));
		return -EINVAL;
	}

	cpuclk->ratio = 16>>shift;
	smp_mb();

	core_set_freq_done=0;
	core_set_freq_core_1_rdy=0;
	param.shift = shift;
	param.core_base_rate = cpu_base_rate;
	stop_machine(core_set_freq_sync, &param, cpu_online_mask);
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
	int cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int ratio = (rate*32)/cpu_base_rate, index;

	index = round_clk_ratio_and_index(&ratio);

	/* apply clock ratio pattern */
#ifdef CONFIG_BCM96858
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, freq_pattern[index].pattern);
#else
	BIUCTRL->cluster_clk_pattern[0] = freq_pattern[index].pattern;
#endif

#endif
	return 0;
}

int init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk)
{
#if defined(CONFIG_BCM963138)
	u32 pll = ARM_PROC_CLK_POLICY_FREQ_ALL(ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW);
	u32 policy = ARMCFG->proc_clk.policy_freq;
	const int mdiv_en = 1 << 11;

	/* arm pll at 2GHz and cpu base freq at 1GHz */
	cpuclk->pllclk = FREQ_MHZ(2000);
	cpuclk->mdiv = 2;

	//if its setup for nosmp mode, assume it has to run at a lower frequency 
	//instead of doing this if(strstr(boot_command_line, "nosmp ") != NULL)
	// we can just check the exported variable
	if(setup_max_cpus == 0)
	{
		cpuclk->mdiv = 3;
	}

	/* change policy to use ARMPLL_SLOW in case cfe isn't up-to-date */
	if ((policy & ARM_PROC_CLK_POLICY_FREQ_MASK) != pll) {

		pr_warn("%s update arm clk policy from 0x%x to 0x%x mdiv %d\n", __func__, policy, pll, cpuclk->mdiv);

		ARMCFG->proc_clk.pllarmc = (ARMCFG->proc_clk.pllarmc & ~ARM_PROC_CLK_PLLARMC_MDIV_MASK) | mdiv_en | cpuclk->mdiv;
		ARMCFG->proc_clk.policy_freq = (policy & ~ARM_PROC_CLK_POLICY_FREQ_MASK) | pll;

		/* enable policy and wait for policy to be activated */
		ARMCFG->proc_clk.policy_ctl |= ARM_PROC_CLK_POLICY_CTL_GO_AC|ARM_PROC_CLK_POLICY_CTL_GO;
		while (ARMCFG->proc_clk.policy_ctl & ARM_PROC_CLK_POLICY_CTL_GO);
	}
#endif

#if defined(CONFIG_BCM963148)
	PLL_CHCFG_REG ch01_cfg;

	/* arm pll at 3.0GHz and cpu base freq at 1.5GHz */
	cpuclk->pllclk = FREQ_MHZ(3000);

	ReadBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(ch01_cfg), &ch01_cfg.Reg32);
	if (ch01_cfg.Bits.mdiv0 != 2 ) {
		pr_warn("%s: set CPU PLL mdiv to 2 and start cpu freq at 1.5GHz\n", __func__);
		ch01_cfg.Bits.mdiv0 = 2;
		WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(ch01_cfg), ch01_cfg.Reg32);
	}
	cpuclk->mdiv = 2;
#endif

#if defined(CONFIG_BCM96858)
	cpuclk->pllclk = FREQ_MHZ(1500);
	cpuclk->mdiv = 1;
	// cluster clock control: unreset
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000000);
	// cluster clock pattern: full-speed pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, ~0);
	// cluster clock control: enable pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000010);
#endif

#if defined(CONFIG_BCM94908)
	cpuclk->pllclk = FREQ_MHZ(1800);
	cpuclk->mdiv = 1;

	if ((BIUCTRL->cluster_clk_ctrl[0] & 1 << 4) == 0) {
		BIUCTRL->cluster_clk_pattern[0] = ~0;	// full-speed user clock-pattern
		BIUCTRL->cluster_clk_ctrl[0] = 1 << 4;	// enable user clock-patterns
	}
#endif
	return 0;
}

/*
 * Initialize the cpu default pll frequency
 */
static int bcm63xx_cpuclk_init_default(struct device_node *node, struct bcm63xx_cpuclk *cpuclk)
{
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)
	brcm_legacy_init(node);
	pmc_initmode();
#endif

	return init_arm_core_pll(cpuclk);
}

static unsigned long bcm63xx_cpuclk_recalc_rate(struct clk_hw *hwclk,
					 unsigned long parent_rate)
{
	struct bcm63xx_cpuclk *cpuclk  = to_bcm63xx_cpuclk(hwclk);
	u32 ratio, base;
	unsigned long rate;

	ratio = base = 0;
	get_arm_core_ratio(&ratio, &base, &cpuclk->mdiv);
	cpuclk->ratio = ratio;
	cpuclk->ratio_base = base;

	rate = (cpuclk->pllclk/cpuclk->mdiv/base)*ratio;

	CLK_BCM_LOG("ratio %d base %d mdiv %d\n", ratio, base, cpuclk->mdiv);
	CLK_BCM_LOG("parent rate %ld return rate %ld\n", parent_rate, rate);

	return rate;
}

static long bcm63xx_cpuclk_round_rate(struct clk_hw *hwclk, unsigned long rate,
			       unsigned long *parent_rate)
{
	long rate_round;
	struct bcm63xx_cpuclk *cpuclk = to_bcm63xx_cpuclk(hwclk);

	rate_round = round_arm_core_rate(cpuclk, rate);

	CLK_BCM_LOG("parent rate %ld rate %ld round rate %ld\n", *parent_rate, rate, (unsigned long)rate_round);


	return rate_round;
}

static int bcm63xx_cpuclk_set_rate(struct clk_hw *hwclk, unsigned long rate,
			    unsigned long parent_rate)
{

	struct bcm63xx_cpuclk *cpuclk = to_bcm63xx_cpuclk(hwclk);
	CLK_BCM_LOG("parent rate %ld rate %ld\n", parent_rate, rate);

	return set_arm_core_clock(cpuclk, parent_rate, rate);
}

static const struct clk_ops bcm63xx_cpuclk_ops = {
	.recalc_rate = bcm63xx_cpuclk_recalc_rate,
	.round_rate = bcm63xx_cpuclk_round_rate,
	.set_rate = bcm63xx_cpuclk_set_rate,
};

/**
 * bcm63xx_cpuclk_init - initialize bcm63xx cpu clock through DT
 * @node: device tree node for this clock
 */
static void __init bcm63xx_cpuclk_init(struct device_node *node)
{
	struct clk *clk;
	const char *clk_name = node->name;
	struct clk_init_data init;
	struct bcm63xx_cpuclk *cpuclk;

	cpuclk = kmalloc(sizeof(struct bcm63xx_cpuclk), GFP_KERNEL);
	if (!cpuclk) {
		pr_err("%s: could not allocate bcm63xx cpuclk\n", __func__);
		return;
	}

	bcm63xx_cpuclk_init_default(node, cpuclk);

	of_property_read_string(node, "clock-output-names", &clk_name);

	cpuclk->hw.init = &init;
	init.name = clk_name;
	init.ops = &bcm63xx_cpuclk_ops;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init.flags = CLK_IS_ROOT;
#endif
	init.parent_names = NULL;
	init.num_parents = 0;

	clk = clk_register(NULL, &cpuclk->hw);
	if (IS_ERR(clk))
		kfree(cpuclk);
	else
		of_clk_add_provider(node, of_clk_src_simple_get, clk);

	return;
}

/**
 * of_bcm63xx_cpuclk_init - initialize bcm63xx cpu clock through DT
 * @node: device tree node for this clock
 */
static void __init of_bcm63xx_cpuclk_init(struct device_node *node)
{
	printk("clk-bcm63xx: BCM63XX CPU Clock driver\n");
	bcm63xx_cpuclk_init(node);
}
CLK_OF_DECLARE(bcm63xx_cpuclk, "brcm,63xx_cpuclk", of_bcm63xx_cpuclk_init);
