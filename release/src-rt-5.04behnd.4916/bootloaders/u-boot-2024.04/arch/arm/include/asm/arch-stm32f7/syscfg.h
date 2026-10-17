/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016
 * Michael Kurz, michi.kurz@gmail.com.
 */

#ifndef _STM32_SYSCFG_H
#define _STM32_SYSCFG_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

struct stm32_syscfg_regs {
	u32 memrmp;
	u32 pmc;
	u32 exticr1;
	u32 exticr2;
	u32 exticr3;
	u32 exticr4;
	u32 cmpcr;
};

/*
 * SYSCFG registers base
 */
#define STM32_SYSCFG		((struct stm32_syscfg_regs *)STM32_SYSCFG_BASE)

/* SYSCFG peripheral mode configuration register */
#define SYSCFG_PMC_MII_RMII_SEL	BIT(23)

#endif
