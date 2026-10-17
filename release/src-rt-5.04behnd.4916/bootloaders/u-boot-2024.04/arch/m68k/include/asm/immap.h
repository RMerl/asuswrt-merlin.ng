/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ColdFire Internal Memory Map and Defines
 *
 * Copyright 2004-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_H
#define __IMMAP_H

#include <config.h>
#if defined(CONFIG_MCF520x)
#include <asm/immap_520x.h>
#include <asm/m520x.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x4000))

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprh0)
#define CFG_SYS_TMRINTR_NO		(INT0_HI_DTMR1)
#define CFG_SYS_TMRINTR_MASK		(INTC_IPRH_INT33)
#define CFG_SYS_TMRINTR_PEND		(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(6)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)
#endif				/* CONFIG_M520x */

#ifdef CONFIG_M5235
#include <asm/immap_5235.h>
#include <asm/m5235.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR3)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprl0)
#define CFG_SYS_TMRINTR_NO		(INT0_LO_DTMR3)
#define CFG_SYS_TMRINTR_MASK	(INTC_IPRL_INT22)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(0x1E)		/* Level must include inorder to work */
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)
#endif				/* CONFIG_M5235 */

#ifdef CONFIG_M5249
#include <asm/immap_5249.h>
#include <asm/m5249.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

#define CFG_SYS_INTR_BASE		(MMAP_INTC)
#define CFG_SYS_NUM_IRQS		(64)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(mbar_readLong(MCFSIM_IPR))
#define CFG_SYS_TMRINTR_NO		(31)
#define CFG_SYS_TMRINTR_MASK	(0x00000400)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(MCFSIM_ICR_AUTOVEC | MCFSIM_ICR_LEVEL7 | MCFSIM_ICR_PRI3)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 2000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif				/* CONFIG_M5249 */

#ifdef CONFIG_M5253
#include <asm/immap_5253.h>
#include <asm/m5249.h>
#include <asm/m5253.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

#define CFG_SYS_INTR_BASE		(MMAP_INTC)
#define CFG_SYS_NUM_IRQS		(64)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(mbar_readLong(MCFSIM_IPR))
#define CFG_SYS_TMRINTR_NO		(27)
#define CFG_SYS_TMRINTR_MASK	(0x00000400)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(MCFSIM_ICR_AUTOVEC | MCFSIM_ICR_LEVEL3 | MCFSIM_ICR_PRI3)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 2000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif				/* CONFIG_M5253 */

#ifdef CONFIG_M5271
#include <asm/immap_5271.h>
#include <asm/m5271.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR3)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprl0)
#define CFG_SYS_TMRINTR_NO		(INT0_LO_DTMR3)
#define CFG_SYS_TMRINTR_MASK	(INTC_IPRL_INT22)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(0x1E) /* Interrupt level 3, priority 6 */
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)
#endif				/* CONFIG_M5271 */

#ifdef CONFIG_M5272
#include <asm/immap_5272.h>
#include <asm/m5272.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

#define CFG_SYS_INTR_BASE		(MMAP_INTC)
#define CFG_SYS_NUM_IRQS		(64)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_TMR0)
#define CFG_SYS_TMR_BASE		(MMAP_TMR3)
#define CFG_SYS_TMRPND_REG		(((volatile intctrl_t *)(CFG_SYS_INTR_BASE))->int_isr)
#define CFG_SYS_TMRINTR_NO		(INT_TMR3)
#define CFG_SYS_TMRINTR_MASK	(INT_ISR_INT24)
#define CFG_SYS_TMRINTR_PEND	(0)
#define CFG_SYS_TMRINTR_PRI		(INT_ICR1_TMR3PI | INT_ICR1_TMR3IPL(5))
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif				/* CONFIG_M5272 */

#ifdef CONFIG_M5275
#include <asm/immap_5275.h>
#include <asm/m5275.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(192)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR3)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprl0)
#define CFG_SYS_TMRINTR_NO		(INT0_LO_DTMR3)
#define CFG_SYS_TMRINTR_MASK	(INTC_IPRL_INT22)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(0x1E)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif				/* CONFIG_M5275 */

#ifdef CONFIG_M5282
#include <asm/immap_5282.h>
#include <asm/m5282.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x40))

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR3)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprl0)
#define CFG_SYS_TMRINTR_NO		(INT0_LO_DTMR3)
#define CFG_SYS_TMRINTR_MASK	(1 << INT0_LO_DTMR3)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(0x1E)		/* Level must include inorder to work */
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif				/* CONFIG_M5282 */

#ifdef CONFIG_M5307
#include <asm/immap_5307.h>
#include <asm/m5307.h>

#define CFG_SYS_UART_BASE            (MMAP_UART0 + \
					(CFG_SYS_UART_PORT * 0x40))
#define CFG_SYS_INTR_BASE            (MMAP_INTC)
#define CFG_SYS_NUM_IRQS             (64)

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE          (MMAP_DTMR0)
#define CFG_SYS_TMR_BASE             (MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(((volatile intctrl_t *) \
					(CFG_SYS_INTR_BASE))->ipr)
#define CFG_SYS_TMRINTR_NO           (31)
#define CFG_SYS_TMRINTR_MASK		(0x00000400)
#define CFG_SYS_TMRINTR_PEND		(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI          (MCFSIM_ICR_AUTOVEC | \
					MCFSIM_ICR_LEVEL7 | MCFSIM_ICR_PRI3)
#define CFG_SYS_TIMER_PRESCALER      (((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif
#endif                          /* CONFIG_M5307 */

#if defined(CONFIG_MCF5301x)
#include <asm/immap_5301x.h>
#include <asm/m5301x.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x4000))

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprh0)
#define CFG_SYS_TMRINTR_NO		(INT0_HI_DTMR1)
#define CFG_SYS_TMRINTR_MASK		(INTC_IPRH_INT33)
#define CFG_SYS_TMRINTR_PEND		(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(6)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)
#endif				/* CONFIG_M5301x */

#if defined(CONFIG_M5329) || defined(CONFIG_M5373)
#include <asm/immap_5329.h>
#include <asm/m5329.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x4000))

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprh0)
#define CFG_SYS_TMRINTR_NO		(INT0_HI_DTMR1)
#define CFG_SYS_TMRINTR_MASK	(INTC_IPRH_INT33)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(6)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)
#endif				/* CONFIG_M5329 && CONFIG_M5373 */

#if defined(CONFIG_M54418)
#include <asm/immap_5441x.h>
#include <asm/m5441x.h>

#if (CFG_SYS_UART_PORT < 4)
#define CFG_SYS_UART_BASE		(MMAP_UART0 + \
					(CFG_SYS_UART_PORT * 0x4000))
#else
#define CFG_SYS_UART_BASE		(MMAP_UART4 + \
					((CFG_SYS_UART_PORT - 4) * 0x4000))
#endif

#define MMAP_DSPI			MMAP_DSPI0

/* Timer */
#if CONFIG_IS_ENABLED(MCFTMR)
#define CFG_SYS_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_SYS_TMR_BASE		(MMAP_DTMR1)
#define CFG_SYS_TMRPND_REG	(((int0_t *)(CFG_SYS_INTR_BASE))->iprh0)
#define CFG_SYS_TMRINTR_NO		(INT0_HI_DTMR1)
#define CFG_SYS_TMRINTR_MASK		(INTC_IPRH_INT33)
#define CFG_SYS_TMRINTR_PEND		(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(6)
#define CFG_SYS_TIMER_PRESCALER	(((gd->bus_clk / 1000000) - 1) << 8)
#else
#define CFG_SYS_UDELAY_BASE		(MMAP_PIT0)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(192)

#endif				/* CONFIG_M54418 */

#ifdef CONFIG_M547x
#include <asm/immap_547x_8x.h>
#include <asm/m547x_8x.h>

#define CFG_SYS_UART_BASE		(MMAP_UART0 + (CFG_SYS_UART_PORT * 0x100))

#ifdef CONFIG_SLTTMR
#define CFG_SYS_UDELAY_BASE		(MMAP_SLT1)
#define CFG_SYS_TMR_BASE		(MMAP_SLT0)
#define CFG_SYS_TMRPND_REG		(((volatile int0_t *)(CFG_SYS_INTR_BASE))->iprh0)
#define CFG_SYS_TMRINTR_NO		(INT0_HI_SLT0)
#define CFG_SYS_TMRINTR_MASK	(INTC_IPRH_INT54)
#define CFG_SYS_TMRINTR_PEND	(CFG_SYS_TMRINTR_MASK)
#define CFG_SYS_TMRINTR_PRI		(0x1E)
#define CFG_SYS_TIMER_PRESCALER	(gd->bus_clk / 1000000)
#endif

#define CFG_SYS_INTR_BASE		(MMAP_INTC0)
#define CFG_SYS_NUM_IRQS		(128)

#ifdef CONFIG_PCI
#define CFG_SYS_PCI_BAR0		(0x40000000)
#define CFG_SYS_PCI_BAR1		(CFG_SYS_SDRAM_BASE)
#define CFG_SYS_PCI_TBATR0		(CFG_SYS_MBAR)
#define CFG_SYS_PCI_TBATR1		(CFG_SYS_SDRAM_BASE)
#endif
#endif				/* CONFIG_M547x */

#endif				/* __IMMAP_H */
