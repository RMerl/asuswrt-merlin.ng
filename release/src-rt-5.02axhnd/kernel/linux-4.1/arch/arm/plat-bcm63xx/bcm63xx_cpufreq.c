#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
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

/* CPU Frequency scaling support for BCM63xx ARM series */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/suspend.h>

#include <bcm_map_part.h>
#include <asm/cpu.h>

/* frequency in units of kHz */
/* note: index column specified in initialization */
/*       but may be used for other purposes */
struct cpufreq_frequency_table bcm63xx_freq_normal_table[] = {
#if defined CONFIG_BCM963138
/* support divisors of 2GHz */
	{0, 10,  200000},
	{0, 8,   250000},
	{0, 6,   333333},
	{0, 5,   400000},
	{0, 4,   500000},
	{0, 3,   666666},
	{0, 2,  1000000},
#elif defined CONFIG_BCM963148
/* support divisors of 3GHz */
	{0, 16,  187500},
	{0, 8,   375000},
	{0, 4,   750000},
	{0, 2,  1500000},
#endif
	{0, 0, CPUFREQ_TABLE_END},
};

/* frequency is in the unit of kHz */
/* note: index column specified in initialization */
/*       but may be used for other purposes */
struct cpufreq_frequency_table bcm63xx_freq_extended_table[] = {
#if defined CONFIG_BCM963138
	{0, 10,  200000},
	{0, 9,   222222},
	{0, 8,   250000},
	{0, 7,   285714},
	{0, 6,   333333},
	{0, 5,   400000},
	{0, 4,   500000},
	{0, 3,   666666},
	{0, 2,  1000000},
#elif defined CONFIG_BCM963148
	{0, 32,   93750},
//	{30,  100000},
//	{24,  125000},
//	{20,  150000},
	{0, 16,  187500},
//	{15,  200000},
//	{12,  250000},
//	{10,  300000},
	{0, 8,   375000},
//	{6,   500000},
//	{5,   600000},
	{0, 4,   750000},
//	{3,  1000000},
	{0, 2,  1500000},
#endif
	{0, 0, CPUFREQ_TABLE_END},
};

static void truncate_cpu_freq_table(void)
{
	int i=0;

	while(bcm63xx_freq_normal_table[i].frequency != CPUFREQ_TABLE_END)
	{
		if(bcm63xx_freq_normal_table[i].frequency > 666667)
		{
			bcm63xx_freq_normal_table[i].flags=0;
			bcm63xx_freq_normal_table[i].driver_data=0;
			bcm63xx_freq_normal_table[i].frequency = CPUFREQ_TABLE_END;
		}
		i++;
	}
	i=0;
	while(bcm63xx_freq_extended_table[i].frequency != CPUFREQ_TABLE_END)
	{
		if(bcm63xx_freq_extended_table[i].frequency > 666667)
		{
			bcm63xx_freq_extended_table[i].flags=0;
			bcm63xx_freq_extended_table[i].driver_data=0;
			bcm63xx_freq_extended_table[i].frequency = CPUFREQ_TABLE_END;
		}
		i++;
	}

}

struct cpufreq_frequency_table *bcm63xx_freq_table = bcm63xx_freq_normal_table;

unsigned int bcm63xx_cpufreq_getspeed(unsigned int cpu)
{
	struct clk *arm_clk;
	unsigned long arm_freq;

	arm_clk = clk_get_sys("cpu", "arm_pclk");
	BUG_ON(IS_ERR_OR_NULL(arm_clk));
	arm_freq = clk_get_rate(arm_clk);
	BUG_ON(!arm_freq);

	return (arm_freq / 1000);
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
		unsigned int  freq;
	} global_lpj_ref;
	unsigned cpu;

	if (freqs->flags & CPUFREQ_CONST_LOOPS)
		return;
	if (freqs->old == freqs->new)
		  return;
	if (!global_lpj_ref.freq) {
		global_lpj_ref.ref = loops_per_jiffy;
		global_lpj_ref.freq = freqs->old;
	}

	loops_per_jiffy =
		cpufreq_scale(global_lpj_ref.ref, global_lpj_ref.freq, freqs->new);

	for_each_cpu(cpu, cpus) {
		per_cpu(cpu_data, cpu).loops_per_jiffy = loops_per_jiffy;
	}
#endif
}

static int bcm63xx_cpufreq_target(struct cpufreq_policy *policy,
		unsigned int target_freq,
		unsigned int relation)
{
	struct cpufreq_freqs freqs;
	unsigned int index, old_index;
	struct clk *arm_clk;
	int ret = 0;

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

	arm_clk = clk_get_sys("cpu", "arm_pclk");
	BUG_ON(IS_ERR_OR_NULL(arm_clk));

	ret = clk_set_rate(arm_clk, freqs.new * 1000);
	if (ret != 0)
		freqs.new = freqs.old;

	adjust_jiffies(policy->cpus, &freqs);

	cpufreq_freq_transition_end(policy, &freqs, 0);
	
	return ret;
}

static ssize_t store_set_freq_table(struct cpufreq_policy *policy,
		const char *buf, size_t count)
{
	unsigned int ret;
	char str_freqtable[16];
	struct cpufreq_policy new_policy;

	ret = sscanf(buf, "%15s", str_freqtable);
	if (ret != 1)
		return -EINVAL;

	if (!strncasecmp(str_freqtable, "normal", 16)) {
		if (bcm63xx_freq_table == bcm63xx_freq_normal_table)
			return count;
		bcm63xx_freq_table = bcm63xx_freq_normal_table;
	} else if (!strncasecmp(str_freqtable, "extended", 16)) {
		if (bcm63xx_freq_table == bcm63xx_freq_extended_table)
			return count;
		bcm63xx_freq_table = bcm63xx_freq_extended_table;
	} else {
		return -EINVAL;
	}

	/* update the current policy info */
	ret = cpufreq_table_validate_and_show(policy, bcm63xx_freq_table);
	if (ret)
		return ret;

	/* to get the policy updated with the new freq_table */
	ret = cpufreq_get_policy(&new_policy, policy->cpu);
	if (ret)
		return ret;

	down_write(&policy->rwsem);
	ret = cpufreq_set_policy(policy, &new_policy);
	up_write(&policy->rwsem);
	
	if (ret)
		return ret;

	return count;
}

static ssize_t show_set_freq_table(struct cpufreq_policy *policy, char *buf)
{
	ssize_t i = 0;

	if (bcm63xx_freq_table == bcm63xx_freq_normal_table)
		i += sprintf(buf, "normal\n");
	else if (bcm63xx_freq_table == bcm63xx_freq_extended_table)
		i += sprintf(buf, "extended\n");
	else
		i += sprintf(buf, "error!\n");

	i += sprintf(&buf[i], "available tables: normal, extended\n");
	return i;
}

cpufreq_freq_attr_rw(set_freq_table);

static int bcm63xx_cpufreq_init_sysfs(struct cpufreq_policy *policy)
{
	/* creating the sysfs for changing freq table */
	int ret = sysfs_create_file(&policy->kobj, &set_freq_table.attr);
	if (ret)
		printk("%s:fail to create sysfs for set_freq_table\n", __func__);

	return ret;
}

static int bcm63xx_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	int ret;
	policy->cur = policy->min =
		policy->max = bcm63xx_cpufreq_getspeed(policy->cpu);
	/* set the transition latency value */
#if defined CONFIG_BCM963138
	// down 43..45us, up 82..87us
	policy->cpuinfo.transition_latency = 40000; // ~40-90us
#elif defined CONFIG_BCM963148
	// down 25..75us, up 200..210us
	policy->cpuinfo.transition_latency = 40000; // ~40-280us
#endif

	/*
	 * In BCM63xx, all ARM CPUs are set to the same speed.
	 * They all have the same clock source. */
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
		truncate_cpu_freq_table();
	} else {
		cpumask_setall(policy->cpus);
	}

	ret = cpufreq_table_validate_and_show(policy, bcm63xx_freq_table);
	if (ret != 0)
		return ret;

	if (policy->cur > policy->max) {
		bcm63xx_freq_table = bcm63xx_freq_extended_table;
		ret = cpufreq_table_validate_and_show(policy, bcm63xx_freq_table);
		if (ret != 0) {
			/* if unable to set up the extended cpufreq_table, then
			 * we go back use the normal one, it should work */
			bcm63xx_freq_table = bcm63xx_freq_normal_table;
			ret = cpufreq_table_validate_and_show(policy, bcm63xx_freq_table);
		}
	}

	return ret;
}

// TODO! As for October 2013, we do not support PM yet.
#ifdef CONFIG_PM
static int bcm63xx_cpufreq_suspend(struct cpufreq_policy *policy)
{
	return 0;
}

static int bcm63xx_cpufreq_resume(struct cpufreq_policy *policy)
{
	return 0;
}
#endif

static struct cpufreq_driver bcm63xx_cpufreq_driver = {
	.flags		= CPUFREQ_STICKY,
	.verify		= cpufreq_generic_frequency_table_verify,
	.target		= bcm63xx_cpufreq_target,
	.get		= bcm63xx_cpufreq_getspeed,
	.init		= bcm63xx_cpufreq_cpu_init,
	.name		= "bcm63xx_cpufreq",
	.attr		= cpufreq_generic_attr,
	.init_sysfs	= bcm63xx_cpufreq_init_sysfs,
#ifdef CONFIG_PM
	.suspend	= bcm63xx_cpufreq_suspend,
	.resume		= bcm63xx_cpufreq_resume,
#endif
};

static int __init bcm63xx_cpufreq_init(void)
{
	return cpufreq_register_driver(&bcm63xx_cpufreq_driver);
}
late_initcall(bcm63xx_cpufreq_init);
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */

