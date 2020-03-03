/*
 * Copyright (C) 2014 Broadcom Corporation
 * Copyright 2014 Linaro Limited
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/sched.h>

#include <asm/smp.h>
#include <asm/smp_plat.h>
#include <asm/smp_scu.h>

/* Size of mapped Cortex A9 SCU address space */
#define CORTEX_A9_SCU_SIZE	0x58

#define SECONDARY_TIMEOUT_NS	NSEC_PER_MSEC	/* 1 msec (in nanoseconds) */
#define BOOT_ADDR_CPUID_MASK	0x3

/* Name of device node property defining secondary boot register location */
#define OF_SECONDARY_BOOT	"secondary-boot-reg"

/* I/O address of register used to coordinate secondary core startup */
static u32	secondary_boot;

/*
 * Enable the Cortex A9 Snoop Control Unit
 *
 * By the time this is called we already know there are multiple
 * cores present.  We assume we're running on a Cortex A9 processor,
 * so any trouble getting the base address register or getting the
 * SCU base is a problem.
 *
 * Return 0 if successful or an error code otherwise.
 */
static int __init scu_a9_enable(void)
{
	unsigned long config_base;
	void __iomem *scu_base;

	if (!scu_a9_has_base()) {
		pr_err("no configuration base address register!\n");
		return -ENXIO;
	}

	/* Config base address register value is zero for uniprocessor */
	config_base = scu_a9_get_base();
	if (!config_base) {
		pr_err("hardware reports only one core\n");
		return -ENOENT;
	}

	scu_base = ioremap((phys_addr_t)config_base, CORTEX_A9_SCU_SIZE);
	if (!scu_base) {
		pr_err("failed to remap config base (%lu/%u) for SCU\n",
			config_base, CORTEX_A9_SCU_SIZE);
		return -ENOMEM;
	}

	scu_enable(scu_base);

	iounmap(scu_base);	/* That's the last we'll need of this */

	return 0;
}

static void __init bcm_smp_prepare_cpus(unsigned int max_cpus)
{
	static cpumask_t only_cpu_0 = { CPU_BITS_CPU0 };
	struct device_node *node;
	int ret;

	BUG_ON(secondary_boot);		/* We're called only once */

	/*
	 * This function is only called via smp_ops->smp_prepare_cpu().
	 * That only happens if a "/cpus" device tree node exists
	 * and has an "enable-method" property that selects the SMP
	 * operations defined herein.
	 */
	node = of_find_node_by_path("/cpus");
	BUG_ON(!node);

	/*
	 * Our secondary enable method requires a "secondary-boot-reg"
	 * property to specify a register address used to request the
	 * ROM code boot a secondary code.  If we have any trouble
	 * getting this we fall back to uniprocessor mode.
	 */
	if (of_property_read_u32(node, OF_SECONDARY_BOOT, &secondary_boot)) {
		pr_err("%s: missing/invalid " OF_SECONDARY_BOOT " property\n",
			node->name);
		ret = -ENOENT;		/* Arrange to disable SMP */
		goto out;
	}

	/*
	 * Enable the SCU on Cortex A9 based SoCs.  If -ENOENT is
	 * returned, the SoC reported a uniprocessor configuration.
	 * We bail on any other error.
	 */
	ret = scu_a9_enable();
out:
	of_node_put(node);
	if (ret) {
		/* Update the CPU present map to reflect uniprocessor mode */
		BUG_ON(ret != -ENOENT);
		pr_warn("disabling SMP\n");
		init_cpu_present(&only_cpu_0);
	}
}

/*
 * The ROM code has the secondary cores looping, waiting for an event.
 * When an event occurs each core examines the bottom two bits of the
 * secondary boot register.  When a core finds those bits contain its
 * own core id, it performs initialization, including computing its boot
 * address by clearing the boot register value's bottom two bits.  The
 * core signals that it is beginning its execution by writing its boot
 * address back to the secondary boot register, and finally jumps to
 * that address.
 *
 * So to start a core executing we need to:
 * - Encode the (hardware) CPU id with the bottom bits of the secondary
 *   start address.
 * - Write that value into the secondary boot register.
 * - Generate an event to wake up the secondary CPU(s).
 * - Wait for the secondary boot register to be re-written, which
 *   indicates the secondary core has started.
 */
static int bcm_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	void __iomem *boot_reg;
	phys_addr_t boot_func;
	u64 start_clock;
	u32 cpu_id;
	u32 boot_val;
	bool timeout = false;

	cpu_id = cpu_logical_map(cpu);
	if (cpu_id & ~BOOT_ADDR_CPUID_MASK) {
		pr_err("bad cpu id (%u > %u)\n", cpu_id, BOOT_ADDR_CPUID_MASK);
		return -EINVAL;
	}

	if (!secondary_boot) {
		pr_err("required secondary boot register not specified\n");
		return -EINVAL;
	}

	boot_reg = ioremap_nocache((phys_addr_t)secondary_boot, sizeof(u32));
	if (!boot_reg) {
		pr_err("unable to map boot register for cpu %u\n", cpu_id);
		return -ENOSYS;
	}

	/*
	 * Secondary cores will start in secondary_startup(),
	 * defined in "arch/arm/kernel/head.S"
	 */
	boot_func = virt_to_phys(secondary_startup);
	BUG_ON(boot_func & BOOT_ADDR_CPUID_MASK);
	BUG_ON(boot_func > (phys_addr_t)U32_MAX);

	/* The core to start is encoded in the low bits */
	boot_val = (u32)boot_func | cpu_id;
	writel_relaxed(boot_val, boot_reg);

	sev();

	/* The low bits will be cleared once the core has started */
	start_clock = local_clock();
	while (!timeout && readl_relaxed(boot_reg) == boot_val)
		timeout = local_clock() - start_clock > SECONDARY_TIMEOUT_NS;

	iounmap(boot_reg);

	if (!timeout)
		return 0;

	pr_err("timeout waiting for cpu %u to start\n", cpu_id);

	return -ENOSYS;
}

static struct smp_operations bcm_smp_ops __initdata = {
	.smp_prepare_cpus	= bcm_smp_prepare_cpus,
	.smp_boot_secondary	= bcm_boot_secondary,
};
CPU_METHOD_OF_DECLARE(bcm_smp_bcm281xx, "brcm,bcm11351-cpu-method",
			&bcm_smp_ops);
