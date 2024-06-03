// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Soren Brinkmann <soren.brinkmann@xilinx.com>
 * Copyright (C) 2013 Xilinx, Inc. All rights reserved.
 */
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

static const char * const clk_names[clk_max] = {
	"armpll", "ddrpll", "iopll",
	"cpu_6or4x", "cpu_3or2x", "cpu_2x", "cpu_1x",
	"ddr2x", "ddr3x", "dci",
	"lqspi", "smc", "pcap", "gem0", "gem1",
	"fclk0", "fclk1", "fclk2", "fclk3", "can0", "can1",
	"sdio0", "sdio1", "uart0", "uart1", "spi0", "spi1", "dma",
	"usb0_aper", "usb1_aper", "gem0_aper", "gem1_aper",
	"sdio0_aper", "sdio1_aper", "spi0_aper", "spi1_aper",
	"can0_aper", "can1_aper", "i2c0_aper", "i2c1_aper",
	"uart0_aper", "uart1_aper", "gpio_aper", "lqspi_aper",
	"smc_aper", "swdt", "dbg_trc", "dbg_apb"
};

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
		DM_GET_DRIVER(zynq_clk), &dev);
	if (ret)
		return ret;

	for (i = 0; i < 2; i++) {
		clk.id = i ? ddr3x_clk : cpu_6or4x_clk;
		ret = clk_request(dev, &clk);
		if (ret < 0)
			return ret;

		rate = clk_get_rate(&clk) / 1000000;
		if (i)
			gd->bd->bi_ddr_freq = rate;
		else
			gd->bd->bi_arm_freq = rate;

		clk_free(&clk);
	}
	gd->bd->bi_dsp_freq = 0;

	return 0;
}

/**
 * soc_clk_dump() - Print clock frequencies
 * Returns zero on success
 *
 * Implementation for the clk dump command.
 */
int soc_clk_dump(void)
{
	struct udevice *dev;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
		DM_GET_DRIVER(zynq_clk), &dev);
	if (ret)
		return ret;

	printf("clk\t\tfrequency\n");
	for (i = 0; i < clk_max; i++) {
		const char *name = clk_names[i];
		if (name) {
			struct clk clk;
			unsigned long rate;

			clk.id = i;
			ret = clk_request(dev, &clk);
			if (ret < 0)
				return ret;

			rate = clk_get_rate(&clk);

			clk_free(&clk);

			if ((rate == (unsigned long)-ENOSYS) ||
			    (rate == (unsigned long)-ENXIO))
				printf("%10s%20s\n", name, "unknown");
			else
				printf("%10s%20lu\n", name, rate);
		}
	}

	return 0;
}
