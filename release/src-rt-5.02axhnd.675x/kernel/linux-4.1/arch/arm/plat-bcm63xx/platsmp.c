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
#include <asm/mach-types.h>
#include <asm/smp.h>
#include <asm/smp_scu.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>

#ifdef CONFIG_PLAT_CA9_MPCORE
#include <plat/ca9mpcore.h>
#endif
#include <plat/bsp.h>
#include <mach/hardware.h>
#include <mach/smp.h>
#include <bcm_map_part.h>

static DEFINE_RAW_SPINLOCK(boot_lock);

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

#ifdef CONFIG_PLAT_CA9_MPCORE
static void __init platform_smp_init_cpus(void)
{
    void __iomem *scu_base = scu_base_addr();
    unsigned int i, ncores;

    ncores = scu_base ? scu_get_core_count(scu_base) : 1;

	/* sanity check */
	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
}
#endif

static void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
	/*
	 * Initialise the SCU and wake up the secondary core using
	 * wakeup_secondary().
	 */
#ifdef CONFIG_PLAT_CA9_MPCORE
	scu_enable(scu_base_addr());
#endif

    /* 1) covert the virtual starting address into physical, then
       * write it to register for the bootloader to read from */
    GPIO->GeneralPurpose = virt_to_phys(platform_secondary_startup);
}

struct smp_operations bcm_smp_ops __initdata = {
#ifdef CONFIG_PLAT_CA9_MPCORE
	.smp_init_cpus		= platform_smp_init_cpus,
#endif
	.smp_prepare_cpus	= platform_smp_prepare_cpus,
	.smp_secondary_init	= platform_secondary_init,
	.smp_boot_secondary	= platform_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= platform_cpu_die,
#endif
};

CPU_METHOD_OF_DECLARE(bcm_smp, "brcm,bca-smp", &bcm_smp_ops);

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
