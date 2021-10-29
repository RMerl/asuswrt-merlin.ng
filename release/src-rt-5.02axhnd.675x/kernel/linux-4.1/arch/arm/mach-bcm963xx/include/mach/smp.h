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
/*
 * derived from arch/arm/mach-realview/include/mach/smp.h
 *
 * This file is required from common architecture code,
 * in arch/arm/include/asm/smp.h
 */

#ifndef __ASM_ARCH_SMP_H
#define __ASM_ARCH_SMP_H __FILE__

extern void platform_secondary_startup(void);

int platform_cpu_disable(unsigned int cpu);
void platform_cpu_die(unsigned int cpu);
int platform_cpu_kill(unsigned int cpu);

/* Used in hotplug.c */
#define hard_smp_processor_id()			\
	({						\
		unsigned int cpunum;			\
		__asm__("mrc p15, 0, %0, c0, c0, 5"	\
			: "=r" (cpunum));		\
		cpunum &= 0x0F;				\
	})

#endif /* __ASM_ARCH_SMP_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
