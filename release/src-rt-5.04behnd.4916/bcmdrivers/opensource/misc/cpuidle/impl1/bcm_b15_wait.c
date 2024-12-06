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

/*
 * BCM63148 SoC main platform file.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/stop_machine.h>
#include <asm/system_misc.h>

#define FREQ_MHZ(x)	((x)*1000*1000)


static int arm_wfi_allowed = 1; // administratively allowed

/* power is significantly reduced by re-enabling interrupts
 * and looping locally until a reschedule is needed.
 * nops would help further but create droops/spikes.
 */
__attribute__ (( aligned(16),hot ))
static void bcm63xx_arm_pm_idle(void)
{
	local_irq_enable();
	while (!need_resched());
}

// selective wfi enable/disable based on frequency
void arm_wfi_enable(unsigned int freqHz)
{
	/* enable only if administratively allowed and under 1500MHz */
	if (arm_wfi_allowed && freqHz < FREQ_MHZ(1500)) {
		arm_pm_idle = 0;
	} else {
		arm_pm_idle = bcm63xx_arm_pm_idle;
	}
}

static unsigned int get_arm_core_clk(void)
{
	struct device *cpu_dev = NULL;
	struct clk *arm_clk = NULL;
	unsigned int clk_rate;

	cpu_dev = get_cpu_device(smp_processor_id());
	if (!cpu_dev) {
		return FREQ_MHZ(1500);
	}

	arm_clk = clk_get(cpu_dev, "cpuclk");
	if( IS_ERR_OR_NULL(arm_clk) ) {
	 	pr_err("%s: failed to get cpu clk\n", __func__);
		return FREQ_MHZ(1500);
	}

	clk_rate =  clk_get_rate(arm_clk);

	return clk_rate;
}

/*
 * Functions to allow enabling/disabling WAIT instruction
 */
void set_cpu_arm_wait(int enable)
{
	arm_wfi_allowed = enable;
	printk("wait instruction: %s\n", enable ? "enabled" : "disabled");
	arm_wfi_enable(get_arm_core_clk());
	kick_all_cpus_sync();
}
EXPORT_SYMBOL(set_cpu_arm_wait);

int get_cpu_arm_wait(void)
{
	return arm_wfi_allowed;
}
EXPORT_SYMBOL(get_cpu_arm_wait);

static int __init bcm963xx_idle_init(void)
{
	arm_wfi_enable(get_arm_core_clk());
	return 0;
}
arch_initcall(bcm963xx_idle_init);
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
