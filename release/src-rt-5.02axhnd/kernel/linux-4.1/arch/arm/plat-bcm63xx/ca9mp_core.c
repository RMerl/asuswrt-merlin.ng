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
* ARM Cortex A9 MPCORE Platform base
*/


#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/clockchips.h>
#include <linux/ioport.h>
#include <linux/cpumask.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <linux/irqchip.h>
#include <linux/irqchip/arm-gic.h>
#include <mach/hardware.h>
#include <plat/ca9mpcore.h>
#include <bcm_map_part.h>

void __iomem * scu_base_addr(void)
{
	return __io_address(SCU_PHYS_BASE + CA9MP_SCU_OFF);
}

void __init ca9mp_fixup(void)
{
	/* in case of any fixup that needs to be done for processor, such
	 * as cache invalidation. */
}

/* map_io should be called the first, so we have the register base
 * address for the core. */
void __init ca9mp_map_io(void)
{
	struct map_desc desc;

#if 0
	/* 
	 * Cortex A9 Architecture Manual specifies this as a way to get
	 * MPCORE PERHIPHBASE address at run-time
	 */
	asm("mrc p15,4,%0,c15,c0,0 @ Read Configuration Base Address Register" 
			: "=&r" (base_addr) : : "cc");

	printk(KERN_INFO "CA9 MPCORE found at %p\n", (void *)base_addr); 
#endif

	/* Fix-map the entire PERIPHBASE 2*4K register block */
	desc.virtual = IO_ADDRESS(SCU_PHYS_BASE);
	desc.pfn = __phys_to_pfn(SCU_PHYS_BASE);
	desc.length = SZ_8K;
	desc.type = MT_DEVICE;
	iotable_init(&desc, 1);
}

void __init ca9mp_init_gic(void)
{
	printk(KERN_INFO "Cortex A9 MPCORE GIC init\n");
	printk(KERN_INFO "DIST at %p, CPU_IF at %p\n",
			(void *)IO_ADDRESS(SCU_PHYS_BASE) + CA9MP_GIC_DIST_OFF,
			(void *)IO_ADDRESS(SCU_PHYS_BASE) + CA9MP_GIC_CPUIF_OFF);

	// FIXME!! hardcored value below for the interrupt line#, will need to define
	// the interrupt line# in a header file for all different chips
	gic_init(0, 27, (void *)IO_ADDRESS(SCU_PHYS_BASE) + CA9MP_GIC_DIST_OFF,
			(void *)IO_ADDRESS(SCU_PHYS_BASE) + CA9MP_GIC_CPUIF_OFF);

	//irq_set_handler(CA9MP_IRQ_GLOBALTIMER, handle_percpu_irq);
	/* try it.. handle_edge_irq, handle_percpu_irq, or handle_level_irq */
}

void __init ca9mp_init_early(void)
{
	/* NOP */
}

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
