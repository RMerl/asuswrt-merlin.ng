/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 by Ralf Baechle
 */
#ifndef __ASM_MACH_GENERIC_IRQ_H
#define __ASM_MACH_GENERIC_IRQ_H

#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && (defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848))
#ifndef NR_IRQS
#define NR_IRQS	256
#endif
#endif
#ifndef NR_IRQS
#define NR_IRQS 128
#endif

#ifdef CONFIG_I8259
#ifndef I8259A_IRQ_BASE
#define I8259A_IRQ_BASE 0
#endif
#endif

#ifdef CONFIG_IRQ_CPU

#ifndef MIPS_CPU_IRQ_BASE
#ifdef CONFIG_I8259
#define MIPS_CPU_IRQ_BASE 16
#else
#define MIPS_CPU_IRQ_BASE 0
#endif /* CONFIG_I8259 */
#endif

#ifdef CONFIG_IRQ_CPU_RM7K
#ifndef RM7K_CPU_IRQ_BASE
#define RM7K_CPU_IRQ_BASE (MIPS_CPU_IRQ_BASE+8)
#endif
#endif

#endif /* CONFIG_IRQ_CPU */

#ifdef CONFIG_MIPS_GIC
#ifndef MIPS_GIC_IRQ_BASE
#define MIPS_GIC_IRQ_BASE (MIPS_CPU_IRQ_BASE + 8)
#endif
#endif /* CONFIG_MIPS_GIC */

#endif /* __ASM_MACH_GENERIC_IRQ_H */
