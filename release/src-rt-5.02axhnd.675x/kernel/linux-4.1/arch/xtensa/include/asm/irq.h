/*
 * include/asm-xtensa/irq.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2001 - 2005 Tensilica Inc.
 */

#ifndef _XTENSA_IRQ_H
#define _XTENSA_IRQ_H

#include <linux/init.h>
#include <platform/hardware.h>
#include <variant/core.h>

#ifdef CONFIG_VARIANT_IRQ_SWITCH
#include <variant/irq.h>
#else
static inline void variant_irq_enable(unsigned int irq) { }
static inline void variant_irq_disable(unsigned int irq) { }
#endif

#ifndef VARIANT_NR_IRQS
# define VARIANT_NR_IRQS 0
#endif
#ifndef PLATFORM_NR_IRQS
# define PLATFORM_NR_IRQS 0
#endif
#define XTENSA_NR_IRQS XCHAL_NUM_INTERRUPTS
#define NR_IRQS (XTENSA_NR_IRQS + VARIANT_NR_IRQS + PLATFORM_NR_IRQS + 1)
#define XTENSA_PIC_LINUX_IRQ(hwirq) ((hwirq) + 1)

#if VARIANT_NR_IRQS == 0
static inline void variant_init_irq(void) { }
#else
void variant_init_irq(void) __init;
#endif

static __inline__ int irq_canonicalize(int irq)
{
	return (irq);
}

struct irqaction;
struct irq_domain;

void migrate_irqs(void);
int xtensa_irq_domain_xlate(const u32 *intspec, unsigned int intsize,
		unsigned long int_irq, unsigned long ext_irq,
		unsigned long *out_hwirq, unsigned int *out_type);
int xtensa_irq_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hw);
unsigned xtensa_map_ext_irq(unsigned ext_irq);
unsigned xtensa_get_ext_irq_no(unsigned irq);

#endif	/* _XTENSA_IRQ_H */
