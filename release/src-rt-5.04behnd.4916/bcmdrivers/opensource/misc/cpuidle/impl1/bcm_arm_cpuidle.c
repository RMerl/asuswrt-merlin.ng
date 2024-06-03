/*
<:copyright-BRCM:2015:GPL/GPL:standard

	 Copyright (c) 2015 Broadcom
	 All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpuidle.h>
#include <linux/export.h>
#include <linux/sysfs.h>
#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/of_device.h>
#include "bcm_memc.h"

// available cpu idle state indices
// (see bcm_arm_idle_driver[] below)
enum bcm_cpuidle_states {
	bcs_nop,
	bcs_wfi,
	bcs_last
};

// Enabling WFI by default gives the same behaviour as when cpuidle is compiled out
static unsigned int wfi_enabled = 1;

static ssize_t show_bcm_cpuwait (struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", wfi_enabled);
}

static ssize_t store_bcm_cpuwait(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long long enable;
	int err = kstrtoull(buf, 0, &enable);

	if (err)
		return err;

	if (wfi_enabled != !!enable) {
		wfi_enabled = !!enable;
		cpu_idle_poll_ctrl(!wfi_enabled);
	}

	printk("Wait Instruction is %sabled\n", wfi_enabled ? "en" : "dis");

	return size;
}

static DEVICE_ATTR(bcm_cpuwait, 0644, show_bcm_cpuwait, store_bcm_cpuwait);

#if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
static ktime_t time_start, time_end;
static s64 time_in, time_out;
static unsigned int freq_max_index = 0;	 // the index of the max freq allowed by all limiters in this system
static unsigned int freq_oper_index = 0;	// the index of the freq that will be used while active (not sleeping)
static unsigned int freq_admin_index = 0; // the index of the max freq configured by the user

#if (defined (CONFIG_BCM96858) || defined (CONFIG_BCM96856))
static unsigned int bcm_arm_freq_table_mhz[] = {1500, 750, 375, 188};
#else
static unsigned int bcm_arm_freq_table_mhz[] = {1800, 900, 450, 225};
#endif

/* frequency to clock control pattern mapping */
static const unsigned int bcm_arm_freq_pattern[] = {
	0xffffffff, // 100.00% [32/32]
	0x55555555, //	50.00% [16/32]
	0x11111111, //	25.00%	[8/32]
	0x01010101, //	12.50%	[4/32]
};
#define NUM_PATTERN_INDEX (sizeof(bcm_arm_freq_pattern)/sizeof(unsigned))
#define LAST_PATTERN_INDEX (NUM_PATTERN_INDEX-1)

static unsigned int auto_clock_divide_enabled = 0;
static unsigned int __iomem
#if defined(CONFIG_BCM94908)
	*bcr_wfx_state,
#endif
	*bcr_cluster_clk_ctrl0,
	*bcr_cluster_clk_pattern0;

static void bcm_arm_cpuidle_update_freq( void )
{
	freq_oper_index = (freq_max_index > freq_admin_index) ? freq_max_index : freq_admin_index;
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96858)
	if (bcr_cluster_clk_pattern0)
		*bcr_cluster_clk_pattern0 = bcm_arm_freq_pattern[freq_oper_index];
#else
 #error BCM_CPUIDLE_CLK_DIVIDER not support on this chip
#endif
}

static int bcm_arm_cpuidle_divider_to_index(unsigned div)
{
	switch (div)
	{
		case 1: return 0;
		case 2: return 1;
		case 4: return 2;
		case 8: return 3;
		default: return -1;
	} 
}

// set maximum frequency for governor
void bcm_cpufreq_set_freq_max(unsigned maxdiv)
{
	int index = bcm_arm_cpuidle_divider_to_index(maxdiv);
	if (index != -1)
	{
		freq_max_index = index;
		bcm_arm_cpuidle_update_freq(); 
	}
}
EXPORT_SYMBOL(bcm_cpufreq_set_freq_max);

static ssize_t show_enable_auto_clkdiv(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", auto_clock_divide_enabled);
}

static ssize_t store_enable_auto_clkdiv(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long long value;
	int err = kstrtoull(buf, 0, &value);

	if (err)
		return err;

	auto_clock_divide_enabled = value?1:0;
	time_in = time_out = 0;

	return size;
}

static ssize_t show_time_inside_clkdiv(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%lld\n", time_in);
}

static ssize_t show_time_outside_clkdiv(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%lld\n", time_out);
}

static ssize_t show_valid_freq_list (struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int i = 0;
	buf[0] = 0;
	for ( ; i < NUM_PATTERN_INDEX; i++) {
		char buf2[16];
		snprintf(buf2, sizeof(buf2), "%d\n", bcm_arm_freq_table_mhz[i]);
		strcat(buf, buf2);
	}
	return strlen(buf);
}

static ssize_t show_oper_max_freq (struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", bcm_arm_freq_table_mhz[freq_oper_index]);
}

static ssize_t show_admin_max_freq (struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", bcm_arm_freq_table_mhz[freq_admin_index]);
}

static ssize_t store_admin_max_freq(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long long value;
	int index = 0;
	int err = kstrtoull(buf, 0, &value);

	if (err)
		return err;

	// If we can't fit in above the second lowest value, assume the lowest value
	freq_admin_index = NUM_PATTERN_INDEX - 1;

	// Search for a valid index matching the input
	for ( ; index < NUM_PATTERN_INDEX -1 ; index++ ) {
		if (value >= bcm_arm_freq_table_mhz[index]) {
			freq_admin_index = index;
			break;
		}
	}

	bcm_arm_cpuidle_update_freq(); 

	return size;
}

static ssize_t show_percent_inside_clkdiv(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	s64 per100 = 100*time_in/(time_in+time_out);
	s64 remainder = 100000*time_in/(time_in+time_out) - 1000*per100;

	return sprintf(buf, "%lld.%03lld\n", per100, remainder);
}

static ssize_t store_time_inside_clkdiv(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	time_in = time_out = 0;
	return size;
}
	
static DEVICE_ATTR(enable_auto_clkdiv, 0644, show_enable_auto_clkdiv, store_enable_auto_clkdiv);
static DEVICE_ATTR(time_inside_clkdiv, 0644, show_time_inside_clkdiv, store_time_inside_clkdiv);
static DEVICE_ATTR(time_outside_clkdiv, 0644, show_time_outside_clkdiv, store_time_inside_clkdiv);
static DEVICE_ATTR(percent_inside_clkdiv, 0644, show_percent_inside_clkdiv, store_time_inside_clkdiv);
static DEVICE_ATTR(valid_freq_list, 0444, show_valid_freq_list, NULL);
static DEVICE_ATTR(oper_max_freq, 0444, show_oper_max_freq, NULL);
static DEVICE_ATTR(admin_max_freq, 0644, show_admin_max_freq, store_admin_max_freq);

static struct attribute *bcm_arm_cpuidle_attrs[] = {
	&dev_attr_enable_auto_clkdiv.attr,
	&dev_attr_time_inside_clkdiv.attr,
	&dev_attr_time_outside_clkdiv.attr,
	&dev_attr_percent_inside_clkdiv.attr,
	&dev_attr_valid_freq_list.attr,
	&dev_attr_oper_max_freq.attr,
	&dev_attr_admin_max_freq.attr,
	NULL
};

static struct attribute_group bcm_arm_cpuidle_attr_group = {
	.attrs = bcm_arm_cpuidle_attrs,
	.name = "bcm_arm_cpuidle",
};

struct bcm_biucfg_reg_addr
{
	const char *name;
	struct resource *res;
	unsigned int __iomem **paddr;
};
#define REG_ADDR(reg)				\
	{ .name = #reg, .paddr = &bcr_##reg, }
static int __init bcm_biucfg_map_reg_addr(struct platform_device *pdev)
{
	int i, ret;
	struct bcm_biucfg_reg_addr bra[] = {
#if defined(CONFIG_BCM94908)
		REG_ADDR(wfx_state),
#endif
		REG_ADDR(cluster_clk_ctrl0),
		REG_ADDR(cluster_clk_pattern0),
	};

	for (i = 0; i < ARRAY_SIZE(bra); i++) {
		bra[i].res = platform_get_resource_byname(pdev,
				IORESOURCE_MEM, bra[i].name);
		if (!bra[i].res) {
			pr_err("Error: failed to get reg %s\n", bra[i].name);
			ret = -ENOENT;
			goto err_out;
		}

		*(bra[i].paddr) = ioremap(bra[i].res->start,
				resource_size(bra[i].res));
		if (!*(bra[i].paddr)) {
			pr_err("Error: failed to ioremap reg %s\n", bra[i].name);
			ret = -ENXIO;
			goto err_out;
		}

		pr_info("ioremapped reg %s <0x%llx 0x%llx> to %px\n",
			bra[i].name, (unsigned long long)bra[i].res->start,
			(unsigned long long)resource_size(bra[i].res),
			*(bra[i].paddr));
	}

	return 0;

err_out:
	for (i = 0; i < ARRAY_SIZE(bra) && *(bra[i].paddr); i++) {
		iounmap(*(bra[i].paddr));
		pr_info("iounmapped reg %s <0x%llx 0x%llx> from %px\n",
			bra[i].name, (unsigned long long)bra[i].res->start,
			(unsigned long long)resource_size(bra[i].res),
			*(bra[i].paddr));
		*(bra[i].paddr) = NULL;
	}
	return ret;
}

static struct of_device_id const bcm_biucfg_of_match[] = {
	{ .compatible = "brcm,bcm-biucfg" },
	{ /* end of list */ }
};

static int __init bcm_biucfg_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;

	match = of_match_device(bcm_biucfg_of_match, dev);
	if (!match) {
		dev_err(dev, "failed to match the biu controller\n");
		return -ENODEV;
	}
	dev_info(dev, "matched the biu controller to %s\n",
			match->compatible);

	return bcm_biucfg_map_reg_addr(pdev);
}

static struct platform_driver bcm_biucfg_platform_driver = {
	.driver = {
		.name = "bcm_biucfg",
		.of_match_table = bcm_biucfg_of_match,
	},
	.probe = bcm_biucfg_probe,
};

builtin_platform_driver_probe(bcm_biucfg_platform_driver, bcm_biucfg_probe);
#endif // #if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)

static int bcm_arm_cpuidle_wfi_enter(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index)
{
#if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
	const int do_clk_div = auto_clock_divide_enabled;
	static s64 diff;
	unsigned int online_mask = cpumask_bits(cpu_online_mask)[0];
	bool update_stats = false;
#if defined (CONFIG_BCM96856) || defined (CONFIG_BCM96858)
	static volatile int wfi_state = 0; // assuming only 1 cluster for now
#endif
#endif
#if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER) || \
	defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
	// Assume cpu does not change through this function
	const int cpu_mask = 1 << raw_smp_processor_id();
#endif

#ifdef CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE
	bcm_memc_self_refresh_pre_wfi(cpu_mask);
#endif

#if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)

	time_end = ktime_get();
	if (!ktime_to_ns(time_start)) {
		time_start = time_end;
	}
	diff = ktime_to_us(ktime_sub(time_end, time_start));
	time_start = time_end;
	if (diff > S64_MAX)
		diff = S64_MAX;
	time_out += diff;

	if (do_clk_div) {
		// Check if this is the last cpu entering wfi
		// in which case we can reduce the core frequency.
		// The frequency will be raised back immediately after wfi
		// since the code below is running with local interrupts disabled
#if defined(CONFIG_BCM94908)
		if (bcr_wfx_state && bcr_cluster_clk_pattern0) {
			unsigned int wfx_mask = *bcr_wfx_state & ((1 << 4) - 1);

			if ((((wfx_mask & online_mask) | cpu_mask) ^
						online_mask) == 0)
				*bcr_cluster_clk_pattern0 =
					bcm_arm_freq_pattern[LAST_PATTERN_INDEX];
		}
#elif defined(CONFIG_BCM96856) || defined(CONFIG_BCM96858)
		wfi_state |= cpu_mask;

		// if all CPUs in the cluster are in WFI, lower the clock frequency
		if ((wfi_state ^ online_mask) == 0 && bcr_cluster_clk_pattern0)
			*bcr_cluster_clk_pattern0 =
				bcm_arm_freq_pattern[LAST_PATTERN_INDEX];
#endif

		dsb(ish);
		wfi();

		if (bcr_cluster_clk_pattern0) {
			if (*bcr_cluster_clk_pattern0 !=
					bcm_arm_freq_pattern[freq_oper_index]) {
				update_stats = true; 
#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM96858)
				// all CPUs are in WFI, so the current CPU is the first out of WFI
				wfi_state &= ~(cpu_mask);
#endif
			}
			*bcr_cluster_clk_pattern0 =
				bcm_arm_freq_pattern[freq_oper_index];
		}

		if (update_stats == true) {
			time_end = ktime_get();
			diff = ktime_to_us(ktime_sub(time_end, time_start));
			time_start = time_end;
			if (diff > S64_MAX)
				diff = S64_MAX;
			time_in += diff;
		}
	} else
#endif
	{
		dsb(ish);
		wfi();
	}

#ifdef CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE
	bcm_memc_self_refresh_post_wfi(cpu_mask);
#endif

	return index;
}

static int bcm_arm_cpuidle_nop_enter(struct cpuidle_device *dev,
		struct cpuidle_driver *drv, int index)
{
	return index;
}

static struct cpuidle_driver bcm_arm_idle_driver = {
	.name			= "bcm_wfi",
	.owner			= THIS_MODULE,
	.states[bcs_nop]	= {
		.enter		= bcm_arm_cpuidle_nop_enter,
		.desc		= "nop",
		.name		= "nop",
		.power_usage	= 900,
	},
	.states[bcs_wfi]		= {
		.enter			= bcm_arm_cpuidle_wfi_enter,
		.name			= "wfi",
		.desc			= "Wait For Interrupt",
		.power_usage		= 500,
	},

	.state_count = bcs_last,
};

static int __init bcm_arm_cpuidle_init(void)
{
int ret;

	ret=sysfs_create_file(&cpu_subsys.dev_root->kobj, &dev_attr_bcm_cpuwait.attr);
	if(ret)
		printk(KERN_ERR "sysfs_create_file : %d\n", ret);

#if defined(CONFIG_BCM_CPUIDLE_CLK_DIVIDER)
	ret=sysfs_create_group(&cpu_subsys.dev_root->kobj, &bcm_arm_cpuidle_attr_group);
	if(ret)
		printk(KERN_ERR "sysfs_create_group : %d\n", ret);

#if defined(CONFIG_BCM94908)
	if (bcr_cluster_clk_ctrl0 && (*bcr_cluster_clk_ctrl0 & (1 << 4)) == 0) {
		if (bcr_cluster_clk_pattern0)
			*bcr_cluster_clk_pattern0 = ~0;	// full-speed user clock-pattern
		*bcr_cluster_clk_ctrl0 = 1 << 4;	// enable user clock-patterns
	}
#elif defined(CONFIG_BCM96856) || defined(CONFIG_BCM96858)
	if (bcr_cluster_clk_pattern0)
		*bcr_cluster_clk_pattern0 = ~0;	// full-speed user clock-pattern
	// enable clock control and select clock pattern register
	if (bcr_cluster_clk_ctrl0)
		*bcr_cluster_clk_ctrl0 = ((1 << 4) | (1 << 31));
#endif
#endif

	return cpuidle_register(&bcm_arm_idle_driver, NULL);
}
late_initcall(bcm_arm_cpuidle_init);
