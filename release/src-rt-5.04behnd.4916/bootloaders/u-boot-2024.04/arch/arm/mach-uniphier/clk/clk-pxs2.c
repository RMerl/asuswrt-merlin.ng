// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/bitops.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

void uniphier_pxs2_clk_init(void)
{
#ifdef CONFIG_USB_DWC3_UNIPHIER
	u32 tmp;

	/* deassert reset */
	tmp = readl(sc_base + SC_RSTCTRL);
	tmp |= SC_RSTCTRL_NRST_USB3B0 | SC_RSTCTRL_NRST_GIO;
	writel(tmp, sc_base + SC_RSTCTRL);
	readl(sc_base + SC_RSTCTRL); /* dummy read */

	tmp = readl(sc_base + SC_RSTCTRL2);
	tmp |= SC_RSTCTRL2_NRST_USB3B1;
	writel(tmp, sc_base + SC_RSTCTRL2);
	readl(sc_base + SC_RSTCTRL2); /* dummy read */

	tmp = readl(sc_base + SC_RSTCTRL6);
	tmp |= 0x37;
	writel(tmp, sc_base + SC_RSTCTRL6);

	/* provide clocks */
	tmp = readl(sc_base + SC_CLKCTRL);
	tmp |= BIT(20) | BIT(19) | SC_CLKCTRL_CEN_USB31 | SC_CLKCTRL_CEN_USB30 |
		SC_CLKCTRL_CEN_GIO;
	writel(tmp, sc_base + SC_CLKCTRL);
	readl(sc_base + SC_CLKCTRL); /* dummy read */
#endif
}
