/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Actions Semi SoCs Clock Definitions
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 *
 */

#ifndef _OWL_CLK_H_
#define _OWL_CLK_H_

#include <clk-uclass.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

enum owl_soc {
	S700,
	S900,
};

struct owl_clk_priv {
	phys_addr_t base;
};

/* BUSCLK register definitions */
#define CMU_PDBGDIV_8		7
#define CMU_PDBGDIV_SHIFT	26
#define CMU_PDBGDIV_DIV		(CMU_PDBGDIV_8 << CMU_PDBGDIV_SHIFT)
#define CMU_PERDIV_8		7
#define CMU_PERDIV_SHIFT	20
#define CMU_PERDIV_DIV		(CMU_PERDIV_8 << CMU_PERDIV_SHIFT)
#define CMU_NOCDIV_2		1
#define CMU_NOCDIV_SHIFT	19
#define CMU_NOCDIV_DIV		(CMU_NOCDIV_2 << CMU_NOCDIV_SHIFT)
#define CMU_DMMCLK_SRC_APLL	2
#define CMU_DMMCLK_SRC_SHIFT	10
#define CMU_DMMCLK_SRC		(CMU_DMMCLK_SRC_APLL << CMU_DMMCLK_SRC_SHIFT)
#define CMU_APBCLK_DIV		BIT(8)
#define CMU_NOCCLK_SRC		BIT(7)
#define CMU_AHBCLK_DIV		BIT(4)
#define CMU_CORECLK_MASK	3
#define CMU_CORECLK_CPLL	BIT(1)
#define CMU_CORECLK_HOSC	BIT(0)

/* COREPLL register definitions */
#define CMU_COREPLL_EN		BIT(9)
#define CMU_COREPLL_HOSC_EN	BIT(8)
#define CMU_COREPLL_OUT		(1104 / 24)

/* DEVPLL register definitions */
#define CMU_DEVPLL_CLK		BIT(12)
#define CMU_DEVPLL_EN		BIT(8)
#define CMU_DEVPLL_OUT		(660 / 6)

/* UARTCLK register definitions */
#define CMU_UARTCLK_SRC_DEVPLL	BIT(16)

#define PLL_STABILITY_WAIT_US	50

#define CMU_DEVCLKEN1_UART5	BIT(21)
#define CMU_DEVCLKEN1_UART3	BIT(11)

#endif
