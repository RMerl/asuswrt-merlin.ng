// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#include <clk.h>
#include <dm/device.h>

#include "clk-lib.h"

#define HZ_IN_MHZ	1000000
#define ceil(x, y)	({ ulong __x = (x), __y = (y); (__x + __y - 1) / __y; })

int soc_clk_ctl(const char *name, ulong *rate, enum clk_ctl_ops ctl)
{
	int ret;
	ulong mhz_rate, priv_rate;
	struct clk clk;

	/* Dummy fmeas device, just to be able to use standard clk_* api */
	struct udevice fmeas = {
		.name = "clk-fmeas",
		.node = ofnode_path("/clk-fmeas"),
	};

	ret = clk_get_by_name(&fmeas, name, &clk);
	if (ret) {
		pr_err("clock '%s' not found, err=%d\n", name, ret);
		return ret;
	}

	if (ctl & CLK_ON) {
		ret = clk_enable(&clk);
		if (ret && ret != -ENOSYS && ret != -ENOTSUPP)
			return ret;
	}

	if ((ctl & CLK_SET) && rate) {
		priv_rate = ctl & CLK_MHZ ? (*rate) * HZ_IN_MHZ : *rate;
		ret = clk_set_rate(&clk, priv_rate);
		if (ret)
			return ret;
	}

	if (ctl & CLK_OFF) {
		ret = clk_disable(&clk);
		if (ret) {
			pr_err("clock '%s' can't be disabled, err=%d\n", name, ret);
			return ret;
		}
	}

	priv_rate = clk_get_rate(&clk);

	clk_free(&clk);

	mhz_rate = ceil(priv_rate, HZ_IN_MHZ);

	if (ctl & CLK_MHZ)
		priv_rate = mhz_rate;

	if ((ctl & CLK_GET) && rate)
		*rate = priv_rate;

	if ((ctl & CLK_PRINT) && (ctl & CLK_MHZ))
		printf("HSDK: clock '%s' rate %lu MHz\n", name, priv_rate);
	else if (ctl & CLK_PRINT)
		printf("HSDK: clock '%s' rate %lu Hz\n", name, priv_rate);
	else
		debug("HSDK: clock '%s' rate %lu MHz\n", name, mhz_rate);

	return 0;
}
