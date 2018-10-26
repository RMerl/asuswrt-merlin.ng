/*
 * Spin Table SMP initialisation
 *
 * Copyright (C) 2013 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/smp.h>
#include <linux/types.h>

#include <asm/cacheflush.h>
#include <asm/cpu_ops.h>
#include <asm/cputype.h>
#include <asm/io.h>
#include <asm/smp_plat.h>

extern void secondary_holding_pen(void);
volatile unsigned long secondary_holding_pen_release = INVALID_HWID;

static phys_addr_t cpu_release_addr[NR_CPUS];

/*
 * Write secondary_holding_pen_release in a way that is guaranteed to be
 * visible to all observers, irrespective of whether they're taking part
 * in coherency or not.  This is necessary for the hotplug code to work
 * reliably.
 */
static void write_pen_release(u64 val)
{
	void *start = (void *)&secondary_holding_pen_release;
	unsigned long size = sizeof(secondary_holding_pen_release);

	secondary_holding_pen_release = val;
	__flush_dcache_area(start, size);
}


static int smp_spin_table_cpu_init(struct device_node *dn, unsigned int cpu)
{
	/*
	 * Determine the address from which the CPU is polling.
	 */
	if (of_property_read_u64(dn, "cpu-release-addr",
				 &cpu_release_addr[cpu])) {
		pr_err("CPU %d: missing or invalid cpu-release-addr property\n",
		       cpu);

		return -1;
	}

	return 0;
}

static int smp_spin_table_cpu_prepare(unsigned int cpu)
{
	__le64 __iomem *release_addr;

	if (!cpu_release_addr[cpu])
		return -ENODEV;

	/*
	 * The cpu-release-addr may or may not be inside the linear mapping.
	 * As ioremap_cache will either give us a new mapping or reuse the
	 * existing linear mapping, we can use it to cover both cases. In
	 * either case the memory will be MT_NORMAL.
	 */
	release_addr = ioremap_cache(cpu_release_addr[cpu],
				     sizeof(*release_addr));
	if (!release_addr)
		return -ENOMEM;

	/*
	 * We write the release address as LE regardless of the native
	 * endianess of the kernel. Therefore, any boot-loaders that
	 * read this address need to convert this address to the
	 * boot-loader's endianess before jumping. This is mandated by
	 * the boot protocol.
	 */
	writeq_relaxed(__pa(secondary_holding_pen), release_addr);
	__flush_dcache_area((__force void *)release_addr,
			    sizeof(*release_addr));

	/*
	 * Send an event to wake up the secondary CPU.
	 */
	sev();

	iounmap(release_addr);

	return 0;
}

#if defined CONFIG_BCM_KF_ARM64_BCM963XX && defined CONFIG_HOTPLUG_CPU

# ifdef CONFIG_BCM94908
# include "bcm_map_part.h"

static int smp_spin_table_cpu_boot(unsigned int cpu)
{
	if (BIUCTRL->cpu_pwr_zone_ctrl[cpu] & BIU_CPU_CTRL_PWR_ZONE_CTRL_ZONE_RESET) {
		BIUCTRL->power_cfg |= BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON << cpu;
		BIUCTRL->cpu_pwr_zone_ctrl[cpu] = BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ |
			(BIUCTRL->cpu_pwr_zone_ctrl[cpu] & ~BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ);

		udelay(100); // wait for cpu to come out of reset
	}

	/*
	 * Update the pen release flag.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send an event, causing the secondaries to read pen_release.
	 */
	sev();

	return 0;
}

static void smp_spin_table_cpu_die(unsigned int cpu)
{
	wmb();
	cpu_cache_off();
	flush_cache_all();

	udelay(10); // delay after cache flush

	BIUCTRL->power_cfg &= ~(BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON << cpu);
	BIUCTRL->cpu_pwr_zone_ctrl[cpu] = BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_DN_REQ |
		(BIUCTRL->cpu_pwr_zone_ctrl[cpu] & ~BIU_CPU_CTRL_PWR_ZONE_CTRL_PWR_UP_REQ);

	while (1) cpu_do_idle();
	/*NOTREACHED*/
}

#define smp_spin_table_cpu_kill 0

# elif (defined (CONFIG_BCM96858) || defined(CONFIG_BCM963158))
# include "pmc_drv.h"
# include "BPCM.h"

static struct completion cpu_flush[NR_CPUS];

static const unsigned int pmb[] = {
	PMB_ADDR_ORION_CPU0, PMB_ADDR_ORION_CPU1,
	PMB_ADDR_ORION_CPU2, PMB_ADDR_ORION_CPU3,
};

static int smp_spin_table_cpu_boot(unsigned int cpu)
{
	BPCM_PWR_ZONE_N_CONTROL zctl;
	int rc;

	rc = ReadZoneRegister(pmb[cpu], 0, BPCMZoneOffset(control), &zctl.Reg32);
	if (rc == 0 && zctl.Bits.reset_state) {
		PowerOnZone(pmb[cpu], 0);

		udelay(100); // wait for cpu to come out of reset
	}

	/*
	 * Update the pen release flag.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send an event, causing the secondaries to read pen_release.
	 */
	sev();

	return 0;
}

static void smp_spin_table_cpu_die(unsigned int cpu)
{
	init_completion(&cpu_flush[cpu]);
	wmb();

//	cpu_cache_off();
	flush_cache_all();

	complete(&cpu_flush[cpu]);
	wmb();

	while (1) cpu_do_idle();
	/*NOTREACHED*/
}

static int smp_spin_table_cpu_kill(unsigned int cpu)
{
	if (wait_for_completion_timeout(&cpu_flush[cpu], msecs_to_jiffies(10)) == 0)
		return 0;

	udelay(10); // delay after cache flush

	return PowerOffZone(pmb[cpu], 0) == kPMC_NO_ERROR; // XXX repower flag ignored
}

# endif

#else
static int smp_spin_table_cpu_boot(unsigned int cpu)
{
	/*
	 * Update the pen release flag.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send an event, causing the secondaries to read pen_release.
	 */
	sev();

	return 0;
}
#endif

const struct cpu_operations smp_spin_table_ops = {
	.name		= "spin-table",
	.cpu_init	= smp_spin_table_cpu_init,
	.cpu_prepare	= smp_spin_table_cpu_prepare,
	.cpu_boot	= smp_spin_table_cpu_boot,
#if defined CONFIG_BCM_KF_ARM64_BCM963XX && defined CONFIG_HOTPLUG_CPU
	.cpu_kill	= smp_spin_table_cpu_kill,
	.cpu_die	= smp_spin_table_cpu_die,
#endif
};
