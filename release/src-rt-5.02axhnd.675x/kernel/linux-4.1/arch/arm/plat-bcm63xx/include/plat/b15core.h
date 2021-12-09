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
 * Broadcom ARM based on Cortex A15 CORE
 *
 * Platform hardware information and internal API
 */

#ifndef	__PLAT_B15CORE_H
#define	__PLAT_B15CORE_H

#include <mach/hardware.h>

/* B15 CORE internally-connected IRQs */
#define	B15_IRQ_GLOBALTIMER	27
#define	B15_IRQ_LOCALTIMER	29
#define B15_IRQ_WDTIMER		30

/* 
 NOTE: B15 CORE physical based ontained at run-time,
 while its virtual base address is set at compile-time in memory.h
*/

/* B15 CORE register offsets */
#define	B15_SCU_OFF		0x0000	/* Coherency controller */
#define	B15_GTIMER_OFF		0x0200	/* Global timer */
#define	B15_LTIMER_OFF		0x0600	/* Local (private) timers */
#define	B15_GIC_DIST_OFF	0x1000	/* Interrupt distributor registers */
#define	B15_GIC_CPUIF_OFF	0x2000	/* Interrupt controller CPU interface */

/* FIXME! the following should be fixed once we verify whether B15 and CA9 share
 * the same timer or not */
#define CA9MP_IRQ_GLOBALTIMER	B15_IRQ_GLOBALTIMER
#define CA9MP_IRQ_LOCALTIMER	B15_IRQ_LOCALTIMER
#define CA9MP_GTIMER_OFF	B15_GTIMER_OFF
#define CA9MP_LTIMER_OFF	B15_LTIMER_OFF

#ifndef __ASSEMBLY__

extern void __init b15_fixup(void);
extern void __init b15_map_io(void);
extern void __init b15_init_gic(void);
extern void __init b15_init_early(void);

/* FIXME! the following should be fixed once we verify whether B15 and CA9 share
 * the same timer or not */
//extern void __init ca9mp_timer_init(unsigned long rate);

extern void __iomem * scu_base_addr(void);
extern void __cpuinit b15_power_up_cpu(int cpu_id);

#endif

#endif /* __PLAT_CA9MPCORE_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
