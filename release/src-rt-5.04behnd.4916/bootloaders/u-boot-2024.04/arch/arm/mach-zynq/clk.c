// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Soren Brinkmann <soren.brinkmann@xilinx.com>
 * Copyright (C) 2013 Xilinx, Inc. All rights reserved.
 */
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <init.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * set_cpu_clk_info() - Setup clock information
 *
 * This function is called from common code after relocation and sets up the
 * clock information.
 */
int set_cpu_clk_info(void)
{
	struct clk clk;
	struct udevice *dev;
	ulong rate;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
		DM_DRIVER_GET(zynq_clk), &dev);
	if (ret)
		return ret;

	for (i = 0; i < 2; i++) {
		clk.id = i ? ddr3x_clk : cpu_6or4x_clk;
		ret = clk_request(dev, &clk);
		if (ret < 0)
			return ret;

		rate = clk_get_rate(&clk) / 1000000;
		if (i) {
			gd->bd->bi_ddr_freq = rate;
		} else {
			gd->bd->bi_arm_freq = rate;
			gd->cpu_clk = clk_get_rate(&clk);
		}
	}
	gd->bd->bi_dsp_freq = 0;

	return 0;
}
