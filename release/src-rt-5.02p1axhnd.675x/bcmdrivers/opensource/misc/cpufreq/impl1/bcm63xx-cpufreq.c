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

/* CPU Frequency scaling support for BCM63XX devices */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>

//#define BCM63XX_CPUFREQ_DEBUG  
#ifdef BCM63XX_CPUFREQ_DEBUG
#define BCM63XX_CPUFREQ_LOG(fmt, args...) printk("bcm63xx-cpufreq: %s cpu %d " fmt,  __FUNCTION__, smp_processor_id(), ##args)
#else
#define BCM63XX_CPUFREQ_LOG(fmt, args...)
#endif

struct private_data {
	struct device *cpu_dev;
};

static struct cpufreq_frequency_table bcm63xx_freq_table[] = {
/* frequency in units of kHz */
#ifdef CONFIG_BCM96858
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
#endif
#ifdef  CONFIG_BCM94908
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
#endif
#if defined CONFIG_BCM963138
/* support divisors of 2GHz */
	{0, 10,  200000},
	{0, 8,   250000},
	{0, 6,   333333},
	{0, 5,   400000},
	{0, 4,   500000},
	{0, 3,   666666},
	{0, 2,  1000000},
#endif
#if defined CONFIG_BCM963148
/* support divisors of 3GHz */
	{0, 16,  187500},
	{0, 8,   375000},
	{0, 4,   750000},
	{0, 2,  1500000},
#endif
	{0,0 , CPUFREQ_TABLE_END},
};

#if defined CONFIG_BCM963138
static void truncate_cpu_freq_table(void)
{
	int i=0;

	while(bcm63xx_freq_table[i].frequency != CPUFREQ_TABLE_END)
	{
		if(bcm63xx_freq_table[i].frequency > 666667)
		{
			bcm63xx_freq_table[i].flags=0;
			bcm63xx_freq_table[i].driver_data=0;
			bcm63xx_freq_table[i].frequency = CPUFREQ_TABLE_END;
		}
		i++;
	}

	return;
}
#endif

static int bcm63xx_cpufreq_target(struct cpufreq_policy *policy,
		unsigned int index)
{
	unsigned int new_freq;
	int ret = 0;

	if (policy->clk == NULL) 
		return -EINVAL;

	new_freq = bcm63xx_freq_table[index].frequency;

	ret = clk_set_rate(policy->clk, new_freq * 1000);

	return ret;
}

static int bcm63xx_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	struct private_data *priv;
	struct device *cpu_dev;
	struct clk *cpu_clk;
	unsigned int transition_latency = 0;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	cpu_dev = get_cpu_device(policy->cpu);
	if (!cpu_dev) {
		pr_err("failed to get cpu%d device\n", policy->cpu);
		return -ENODEV;
	}

	cpu_clk = clk_get(cpu_dev, "cpuclk");
	if (IS_ERR(cpu_clk)) {
		dev_err(cpu_dev, "failed to get cpu%d clock: %ld\n", policy->cpu, PTR_ERR(cpu_clk));
		return PTR_ERR(cpu_clk);
	}
	priv->cpu_dev = cpu_dev;
	policy->driver_data = priv;
	policy->clk = cpu_clk;

	// set the transition latency value in nanoseconds
#ifdef CONFIG_BCM96858
	transition_latency = 1000000;	// >1ms
#elif defined CONFIG_BCM94908
	transition_latency = 10000;	// 10us
#elif defined CONFIG_BCM963138
	// down 43..45us, up 82..87us
	transition_latency = 40000; // ~40-90us
#elif defined CONFIG_BCM963148
	// down 25..75us, up 200..210us
	transition_latency = 40000; // ~40-280us
#endif

#if defined CONFIG_BCM963138
	if (num_online_cpus() == 1)
		truncate_cpu_freq_table();
#endif
	return cpufreq_generic_init(policy, bcm63xx_freq_table, transition_latency);
}

static int bcm63xx_cpufreq_exit(struct cpufreq_policy *policy)
{
	struct private_data *priv = policy->driver_data;

	clk_put(policy->clk);
	kfree(priv);

	return 0;
}

static struct cpufreq_driver bcm63xx_cpufreq_driver = {
	.flags		= CPUFREQ_STICKY,
	.verify		= cpufreq_generic_frequency_table_verify,
	.target_index	= bcm63xx_cpufreq_target,
	.get		= cpufreq_generic_get,
	.init		= bcm63xx_cpufreq_cpu_init,
	.exit		= bcm63xx_cpufreq_exit,
	.name		= "bcm63xx_cpufreq",
	.attr		= cpufreq_generic_attr,
};

static int __init bcm63xx_cpufreq_init(void)
{
	return cpufreq_register_driver(&bcm63xx_cpufreq_driver);
}
late_initcall(bcm63xx_cpufreq_init);
