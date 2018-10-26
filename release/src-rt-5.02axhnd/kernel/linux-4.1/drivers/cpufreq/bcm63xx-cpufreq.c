#if defined CONFIG_BCM_KF_ARM64_BCM963XX
/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

/* CPU Frequency scaling support for B53 ARM */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/cpufreq.h>

#include <bcm_map_part.h>
#include <asm/cpu.h>

#ifdef CONFIG_BCM96858
#include "pmc/pmc_drv.h"
#include "pmc/BPCM.h"

#define C0_CLK_CONTROL (0x70 >> 2)
#define C0_CLK_PATTERN (0x78 >> 2)

/* frequency in units of kHz */
static struct cpufreq_frequency_table bcm63xx_freq_table[] = {
/* support fractions of 1.5GHz */
/* (assume 2x b53 pll post divisor) */
	{0, 0, 1500000},
	{0, 0, 1312500},
	{0, 0, 1125000},
	{0, 0,  937500},
	{0, 0,  750000},
	{0, 0,  562500},
	{0, 0,  515625},
	{0, 0,  375000},
	{0, 0,  187500},
	{0,0 , CPUFREQ_TABLE_END},
};

#else
/* frequency in units of kHz */
static struct cpufreq_frequency_table bcm63xx_freq_table[] = {
/* support fractions of 1.8GHz */
/* (assume 2x b53 pll post divisor) */
	{0, 0, 1800000},
	{0, 0, 1575000},
	{0, 0, 1350000},
	{0, 0, 1125000},
	{0, 0,  900000},
	{0, 0,  675000},
	{0, 0,  619000},
	{0, 0,  450000},
	{0, 0,  225000},
	{0, 0, CPUFREQ_TABLE_END},
};
#endif

// XXX unfortunate parallel since flags and driver_data are no longer unused
/* frequency to clock control pattern mapping */
static const unsigned bcm63xx_freq_pattern[] = {
	0xffffffff, // 100.00% [32/32]
	0x7f7f7f7f, //  87.50% [28/32]
	0x5b5b5b5b, //  75.00% [24/32]
	0x12491249, //  62.50% [20/32]
	0x55555555, //  50.00% [16/32]
	0x49494949, //  37.50% [12/32]
	0x49249249, //  34.37% [11/32]
	0x11111111, //  25.00%  [8/32]
	0x01010101, //  12.50%  [4/32]
};

// __builtin_popcount doesn't seem any better
static inline unsigned popcount(unsigned v)
{
	v = v - ((v >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return (((v + (v >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
}

static unsigned bcm63xx_cpufreq_getspeed(unsigned cpu)
{
	// 32 1-bits -> 32/32; 24 1-bits -> 24/32; 16 1-bits -> 16/32
#ifdef CONFIG_BCM96858
	const unsigned b53_clk = 1500000000 / 1000 / 32; // kHz per bit
	uint32 pattern;

	ReadBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, &pattern);
#else
	const unsigned b53_clk = 1800000000 / 1000 / 32; // kHz per bit
	unsigned pattern = BIUCTRL->cluster_clk_pattern[0];
#endif
	return b53_clk * popcount(pattern);
}

/*
 * loops_per_jiffy is not updated on SMP systems in cpufreq driver.
 * Update the per-CPU loops_per_jiffy value on frequency transition.
 * And don't forget to adjust the global one.
 */
static void adjust_jiffies(cpumask_var_t cpus, struct cpufreq_freqs *freqs)
{
#ifdef CONFIG_SMP
	extern unsigned long loops_per_jiffy;
	static struct lpj_info {
		unsigned long ref;
		unsigned int freq;
	} global_lpj_ref;

	if (!global_lpj_ref.freq) {
		global_lpj_ref.ref = loops_per_jiffy;
		global_lpj_ref.freq = freqs->old;
	}

	loops_per_jiffy =
		cpufreq_scale(global_lpj_ref.ref, global_lpj_ref.freq, freqs->new);
#endif
}

static int bcm63xx_cpufreq_target(struct cpufreq_policy *policy,
		unsigned int target_freq,
		unsigned int relation)
{
	struct cpufreq_freqs freqs;
	unsigned int index, old_index;

	freqs.old = policy->cur;

	if (cpufreq_frequency_table_target(policy, bcm63xx_freq_table,
				freqs.old, relation, &old_index))
		return -EINVAL;

	if (cpufreq_frequency_table_target(policy, bcm63xx_freq_table,
				target_freq, relation, &index))
		return -EINVAL;

	if (index == old_index)
		return 0;

	freqs.new = bcm63xx_freq_table[index].frequency;
	freqs.cpu = policy->cpu;

	cpufreq_freq_transition_begin(policy, &freqs);

#ifdef CONFIG_BCM96858
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, bcm63xx_freq_pattern[index]);
#else
	BIUCTRL->cluster_clk_pattern[0] = bcm63xx_freq_pattern[index];
#endif

	adjust_jiffies(policy->cpus, &freqs);

	cpufreq_freq_transition_end(policy, &freqs, 0);

	return 0;
}

static int bcm63xx_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
#ifdef CONFIG_BCM96858
	// cluster clock control: unreset
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000000);
	// cluster clock pattern: full-speed pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_PATTERN, ~0);
	// cluster clock control: enable pattern
	WriteBPCMRegister(PMB_ADDR_BIU_BPCM, C0_CLK_CONTROL, 0x80000010);
#else
	if ((BIUCTRL->cluster_clk_ctrl[0] & 1 << 4) == 0) {
		BIUCTRL->cluster_clk_pattern[0] = ~0;	// full-speed user clock-pattern
		BIUCTRL->cluster_clk_ctrl[0] = 1 << 4;	// enable user clock-patterns
	}
#endif

	policy->cur = policy->min = policy->max =
		bcm63xx_cpufreq_getspeed(policy->cpu);

	// set the transition latency value in nanoseconds
#ifdef CONFIG_BCM96858
	policy->cpuinfo.transition_latency = 1000000;	// >1ms
#else
	policy->cpuinfo.transition_latency = 10000;	// 10us
#endif

	// In BCM63xx, all ARM CPUs are set to the same speed.
	// They all have the same clock source.
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
	} else {
		cpumask_setall(policy->cpus);
	}

	return cpufreq_table_validate_and_show(policy, bcm63xx_freq_table);
}

static struct cpufreq_driver bcm63xx_cpufreq_driver = {
	.flags		= CPUFREQ_STICKY,
	.verify		= cpufreq_generic_frequency_table_verify,
	.target		= bcm63xx_cpufreq_target,
	.get		= bcm63xx_cpufreq_getspeed,
	.init		= bcm63xx_cpufreq_cpu_init,
	.name		= "bcm63xx_cpufreq",
	.attr		= cpufreq_generic_attr,
};

static int __init bcm63xx_cpufreq_init(void)
{
	return cpufreq_register_driver(&bcm63xx_cpufreq_driver);
}
late_initcall(bcm63xx_cpufreq_init);
#endif
