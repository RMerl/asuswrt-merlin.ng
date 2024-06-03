/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 * Peng Fan <peng.fan@nxp.com>
 */

struct imx8_clks {
	ulong id;
	const char *name;
};

#if CONFIG_IS_ENABLED(CMD_CLK)
extern struct imx8_clks imx8_clk_names[];
extern int num_clks;
#endif

ulong imx8_clk_get_rate(struct clk *clk);
ulong imx8_clk_set_rate(struct clk *clk, unsigned long rate);
int __imx8_clk_enable(struct clk *clk, bool enable);
