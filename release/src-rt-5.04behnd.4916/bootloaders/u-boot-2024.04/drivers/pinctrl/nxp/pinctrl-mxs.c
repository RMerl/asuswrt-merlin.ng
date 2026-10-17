// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include "pinctrl-mxs.h"

DECLARE_GLOBAL_DATA_PTR;

struct mxs_pinctrl_priv {
	void __iomem *base;
	const struct mxs_regs *regs;
};

static unsigned long mxs_dt_node_to_map(struct udevice *conf)
{
	unsigned long config = 0;
	int ret;
	u32 val;

	ret = dev_read_u32(conf, "fsl,drive-strength", &val);
	if (!ret)
		config = val | MA_PRESENT;

	ret = dev_read_u32(conf, "fsl,voltage", &val);
	if (!ret)
		config |= val << VOL_SHIFT | VOL_PRESENT;

	ret = dev_read_u32(conf, "fsl,pull-up", &val);
	if (!ret)
		config |= val << PULL_SHIFT | PULL_PRESENT;

	return config;
}

static int mxs_pinctrl_set_mux(struct udevice *dev, u32 val, int bank, int pin)
{
	struct mxs_pinctrl_priv *iomux = dev_get_priv(dev);
	int muxsel = MUXID_TO_MUXSEL(val), shift;
	void __iomem *reg;

	reg = iomux->base + iomux->regs->muxsel;
	reg += bank * 0x20 + pin / 16 * 0x10;
	shift = pin % 16 * 2;

	mxs_pinctrl_rmwl(muxsel, 0x3, shift, reg);
	debug(" mux %d,", muxsel);

	return 0;
}

static int mxs_pinctrl_set_state(struct udevice *dev, struct udevice *conf)
{
	struct mxs_pinctrl_priv *iomux = dev_get_priv(dev);
	u32 *pin_data, val, ma, vol, pull;
	int npins, size, i, ret;
	unsigned long config;

	debug("\n%s: set state: %s\n", __func__, conf->name);

	size = dev_read_size(conf, "fsl,pinmux-ids");
	if (size < 0)
		return size;

	if (!size || size % sizeof(int)) {
		dev_err(dev, "Invalid fsl,pinmux-ids property in %s\n",
			conf->name);
		return -EINVAL;
	}

	npins = size / sizeof(int);

	pin_data = devm_kzalloc(dev, size, 0);
	if (!pin_data)
		return -ENOMEM;

	ret = dev_read_u32_array(conf, "fsl,pinmux-ids", pin_data, npins);
	if (ret) {
		dev_err(dev, "Error reading pin data.\n");
		devm_kfree(dev, pin_data);
		return -EINVAL;
	}

	config = mxs_dt_node_to_map(conf);

	ma = CFG_TO_MA(config);
	vol = CFG_TO_VOL(config);
	pull = CFG_TO_PULL(config);

	for (i = 0; i < npins; i++) {
		int pinid, bank, pin, shift;
		void __iomem *reg;

		val = pin_data[i];

		pinid = MUXID_TO_PINID(val);
		bank = PINID_TO_BANK(pinid);
		pin = PINID_TO_PIN(pinid);

		debug("(val: 0x%x) pin %d,", val, pinid);
		/* Setup pinmux */
		mxs_pinctrl_set_mux(dev, val, bank, pin);

		debug(" ma: %d, vol: %d, pull: %d\n", ma, vol, pull);

		/* drive */
		reg = iomux->base + iomux->regs->drive;
		reg += bank * 0x40 + pin / 8 * 0x10;

		/* mA */
		if (config & MA_PRESENT) {
			shift = pin % 8 * 4;
			mxs_pinctrl_rmwl(ma, 0x3, shift, reg);
		}

		/* vol */
		if (config & VOL_PRESENT) {
			shift = pin % 8 * 4 + 2;
			if (vol)
				writel(1 << shift, reg + SET);
			else
				writel(1 << shift, reg + CLR);
		}

		/* pull */
		if (config & PULL_PRESENT) {
			reg = iomux->base + iomux->regs->pull;
			reg += bank * 0x10;
			shift = pin;
			if (pull)
				writel(1 << shift, reg + SET);
			else
				writel(1 << shift, reg + CLR);
		}
	}

	devm_kfree(dev, pin_data);
	return 0;
}

static struct pinctrl_ops mxs_pinctrl_ops = {
	.set_state = mxs_pinctrl_set_state,
};

static int mxs_pinctrl_probe(struct udevice *dev)
{
	struct mxs_pinctrl_priv *iomux = dev_get_priv(dev);

	iomux->base = dev_read_addr_ptr(dev);
	iomux->regs = (struct mxs_regs *)dev_get_driver_data(dev);

	return 0;
}

static const struct mxs_regs imx23_regs = {
	.muxsel = 0x100,
	.drive = 0x200,
	.pull = 0x400,
};

static const struct mxs_regs imx28_regs = {
	.muxsel = 0x100,
	.drive = 0x300,
	.pull = 0x600,
};

static const struct udevice_id mxs_pinctrl_match[] = {
	{ .compatible = "fsl,imx23-pinctrl", .data = (ulong)&imx23_regs },
	{ .compatible = "fsl,imx28-pinctrl", .data = (ulong)&imx28_regs },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(fsl_imx23_pinctrl) = {
	.name = "fsl_imx23_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(mxs_pinctrl_match),
	.probe = mxs_pinctrl_probe,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.priv_auto	= sizeof(struct mxs_pinctrl_priv),
	.ops = &mxs_pinctrl_ops,
};

DM_DRIVER_ALIAS(fsl_imx23_pinctrl, fsl_imx28_pinctrl)
