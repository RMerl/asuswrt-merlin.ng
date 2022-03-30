/*
 *  linux/include/asm-arm/arch-pxa/hardware.h
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Note: This file was taken from linux-2.4.19-rmk4-pxa1
 *
 * - 2003/01/20 implementation specifics activated
 *   Robert Schwebel <r.schwebel@pengutronix.de>
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/mach-types.h>

/*
 * Define CONFIG_CPU_MONAHANS in case some CPU of the PXA3xx family is selected.
 * PXA300/310/320 all have distinct register mappings in some cases, that's why
 * the exact CPU has to be selected. CONFIG_CPU_MONAHANS is a helper for common
 * drivers and compatibility glue with old source then.
 */
#ifndef	CONFIG_CPU_MONAHANS
#if	defined(CONFIG_CPU_PXA300) || \
	defined(CONFIG_CPU_PXA310) || \
	defined(CONFIG_CPU_PXA320)
#define	CONFIG_CPU_MONAHANS
#endif
#endif

/*
 * These are statically mapped PCMCIA IO space for designs using it as a
 * generic IO bus, typically with ISA parts, hardwired IDE interfaces, etc.
 * The actual PCMCIA code is mapping required IO region at run time.
 */
#define PCMCIA_IO_0_BASE	0xf6000000
#define PCMCIA_IO_1_BASE	0xf7000000


/*
 * We requires absolute addresses.
 */
#define PCIO_BASE		0

/*
 * Workarounds for at least 2 errata so far require this.
 * The mapping is set in mach-pxa/generic.c.
 */
#define UNCACHED_PHYS_0		0xff000000
#define UNCACHED_ADDR		UNCACHED_PHYS_0

/*
 * Intel PXA internal I/O mappings:
 *
 * 0x40000000 - 0x41ffffff <--> 0xf8000000 - 0xf9ffffff
 * 0x44000000 - 0x45ffffff <--> 0xfa000000 - 0xfbffffff
 * 0x48000000 - 0x49ffffff <--> 0xfc000000 - 0xfdffffff
 */

#include "pxa-regs.h"

#ifndef __ASSEMBLY__

/*
 * GPIO edge detection for IRQs:
 * IRQs are generated on Falling-Edge, Rising-Edge, or both.
 * This must be called *before* the corresponding IRQ is registered.
 * Use this instead of directly setting GRER/GFER.
 */
#define GPIO_FALLING_EDGE	1
#define GPIO_RISING_EDGE	2
#define GPIO_BOTH_EDGES		3

#endif

#endif	/* _ASM_ARCH_HARDWARE_H */
