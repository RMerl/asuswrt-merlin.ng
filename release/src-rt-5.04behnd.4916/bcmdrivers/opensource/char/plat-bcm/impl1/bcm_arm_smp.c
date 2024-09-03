#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
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
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic.h>
#include <asm/cputype.h>
#include <asm/smp.h>
#include <asm/smp_scu.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <linux/version.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <bcm_arm_smp.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#define __cpuinit
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
volatile int pen_release=-1;
#endif

extern void platform_secondary_startup(void);

static DEFINE_RAW_SPINLOCK(boot_lock);

#ifdef CONFIG_BCM_CA9_MPCORE
void __iomem *scu_base = NULL;

void __iomem * scu_base_addr(void)
{
	struct device_node *np;

	if (scu_base)
		return scu_base;

	np = of_find_compatible_node(NULL, NULL, "arm,cortex-a9-scu");
	if (np) {
		scu_base = of_iomap(np, 0);
		of_node_put(np);
	} else {
		pr_err("scu arm,cortex-a9-scu node not found!\n");
		return NULL;
	}

	if (IS_ERR_OR_NULL(scu_base)) {
		pr_warn("Failed to map scu base addr 0x%px\n", scu_base);
		scu_base = NULL;
	}

	return scu_base;
}
#endif

/* write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whatever they'are taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void __cpuinit write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
        sync_cache_w(&pen_release);
}

static void __cpuinit platform_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	raw_spin_lock(&boot_lock);
	raw_spin_unlock(&boot_lock);
}

static int __cpuinit platform_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;
	struct device_node *dn;
	u32 addr;
	void __iomem *reg;

	dn = of_get_cpu_node(cpu, NULL);
	if (dn && of_property_read_u32(dn, "cpu-release-addr", &addr) == 0) {
		reg = ioremap((resource_size_t)addr, 4);
		of_node_put(dn);
		if (reg == NULL)
			return -ENOMEM;

		/* covert the virtual starting address into physical, then
		 * write it to register for the bootloader to read from */
		writel(virt_to_phys(platform_secondary_startup), reg);
		iounmap(reg);
	} else {
		pr_err("CPU %d: missing or invalid smp-release-addr property\n", cpu);
		return -EINVAL;
	}

	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	raw_spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 *
	 * Note that "pen_release" is the hardware CPU ID, whereas
	 * "cpu" is Linux's internal ID.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	raw_spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

static void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
	/*
	 * Initialise the SCU and wake up the secondary core using
	 * wakeup_secondary().
	 */
#ifdef CONFIG_BCM_CA9_MPCORE
	scu_enable(scu_base_addr());
#endif
}

struct smp_operations bcm_smp_ops __initdata = {
	.smp_prepare_cpus	= platform_smp_prepare_cpus,
	.smp_secondary_init	= platform_secondary_init,
	.smp_boot_secondary	= platform_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= platform_cpu_die,
#endif
};

CPU_METHOD_OF_DECLARE(bcm_smp, "brcm,bca-smp", &bcm_smp_ops);

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
