// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/clock.h>
#include <dt-bindings/clock/imx8qxp-clock.h>
#include <dt-bindings/soc/imx_rsrc.h>
#include <misc.h>

#include "clk-imx8.h"

__weak ulong imx8_clk_get_rate(struct clk *clk)
{
	return 0;
}

__weak ulong imx8_clk_set_rate(struct clk *clk, unsigned long rate)
{
	return 0;
}

__weak int __imx8_clk_enable(struct clk *clk, bool enable)
{
	return -ENOTSUPP;
}

static int imx8_clk_disable(struct clk *clk)
{
	return __imx8_clk_enable(clk, 0);
}

static int imx8_clk_enable(struct clk *clk)
{
	return __imx8_clk_enable(clk, 1);
}

#if CONFIG_IS_ENABLED(CMD_CLK)
int soc_clk_dump(void)
{
	struct udevice *dev;
	struct clk clk;
	unsigned long rate;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_GET_DRIVER(imx8_clk), &dev);
	if (ret)
		return ret;

	printf("Clk\t\tHz\n");

	for (i = 0; i < num_clks; i++) {
		clk.id = imx8_clk_names[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0) {
			debug("%s clk_request() failed: %d\n", __func__, ret);
			continue;
		}

		ret = clk_get_rate(&clk);
		rate = ret;

		clk_free(&clk);

		if (ret == -ENOTSUPP) {
			printf("clk ID %lu not supported yet\n",
			       imx8_clk_names[i].id);
			continue;
		}
		if (ret < 0) {
			printf("%s %lu: get_rate err: %d\n",
			       __func__, imx8_clk_names[i].id, ret);
			continue;
		}

		printf("%s(%3lu):\t%lu\n",
		       imx8_clk_names[i].name, imx8_clk_names[i].id, rate);
	}

	return 0;
}
#endif

static struct clk_ops imx8_clk_ops = {
	.set_rate = imx8_clk_set_rate,
	.get_rate = imx8_clk_get_rate,
	.enable = imx8_clk_enable,
	.disable = imx8_clk_disable,
};

static int imx8_clk_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id imx8_clk_ids[] = {
	{ .compatible = "fsl,imx8qxp-clk" },
	{ .compatible = "fsl,imx8qm-clk" },
	{ },
};

U_BOOT_DRIVER(imx8_clk) = {
	.name = "clk_imx8",
	.id = UCLASS_CLK,
	.of_match = imx8_clk_ids,
	.ops = &imx8_clk_ops,
	.probe = imx8_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
