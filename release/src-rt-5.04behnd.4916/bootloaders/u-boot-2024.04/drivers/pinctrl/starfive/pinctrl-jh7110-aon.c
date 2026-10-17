// SPDX-License-Identifier: GPL-2.0
/*
 * Pinctrl / GPIO driver for StarFive JH7110 SoC
 *
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 *   Author: Lee Kuan Lim <kuanlim.lee@starfivetech.com>
 *   Author: Jianlong Huang <jianlong.huang@starfivetech.com>
 */

#include <dm/read.h>
#include <dm/device_compat.h>
#include <linux/io.h>

#include <dt-bindings/pinctrl/pinctrl-starfive-jh7110.h>
#include "pinctrl-starfive.h"

#define JH7110_AON_NGPIO		4
#define JH7110_AON_GC_BASE		64

/* registers */
#define JH7110_AON_DOEN		0x0
#define JH7110_AON_DOUT		0x4
#define JH7110_AON_GPI			0x8
#define JH7110_AON_GPIOIN		0x2c

#define JH7110_AON_GPIOEN		0xc
#define JH7110_AON_GPIOIS		0x10
#define JH7110_AON_GPIOIC		0x14
#define JH7110_AON_GPIOIBE		0x18
#define JH7110_AON_GPIOIEV		0x1c
#define JH7110_AON_GPIOIE		0x20
#define JH7110_AON_GPIORIS		0x28
#define JH7110_AON_GPIOMIS		0x28

#define AON_GPO_PDA_0_5_CFG		0x30

static int jh7110_aon_set_one_pin_mux(struct udevice *dev, unsigned int pin,
				      unsigned int din, u32 dout,
				      u32 doen, u32 func)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);

	if (pin < priv->info->ngpios && func == 0)
		starfive_set_gpiomux(dev, pin, din, dout, doen);

	return 0;
}

static int jh7110_aon_get_padcfg_base(struct udevice *dev,
				      unsigned int pin)
{
	if (pin < PAD_GMAC0_MDC)
		return AON_GPO_PDA_0_5_CFG;

	return -1;
}

static void jh7110_aon_init_hw(struct udevice *dev)
{
	struct starfive_pinctrl_priv *priv = dev_get_priv(dev);

	/* mask all GPIO interrupts */
	writel(0, priv->base + JH7110_AON_GPIOIE);
	/* clear edge interrupt flags */
	writel(0, priv->base + JH7110_AON_GPIOIC);
	writel(0x0f, priv->base + JH7110_AON_GPIOIC);
	/* enable GPIO interrupts */
	writel(1, priv->base + JH7110_AON_GPIOEN);
}

const struct starfive_pinctrl_soc_info jh7110_aon_pinctrl_info = {
	/* pin conf */
	.set_one_pinmux = jh7110_aon_set_one_pin_mux,
	.get_padcfg_base  = jh7110_aon_get_padcfg_base,

	/* gpio dout/doen/din/gpioinput register */
	.dout_reg_base = JH7110_AON_DOUT,
	.dout_mask = GENMASK(3, 0),
	.doen_reg_base = JH7110_AON_DOEN,
	.doen_mask = GENMASK(2, 0),
	.gpi_reg_base = JH7110_AON_GPI,
	.gpi_mask = GENMASK(3, 0),
	.gpioin_reg_base = JH7110_AON_GPIOIN,

	/* gpio */
	.gpio_bank_name = "RGPIO",
	.ngpios = JH7110_AON_NGPIO,
	.gpio_init_hw = jh7110_aon_init_hw,
};

static int jh7110_aon_pinctrl_probe(struct udevice *dev)
{
	struct starfive_pinctrl_soc_info *info =
		(struct starfive_pinctrl_soc_info *)dev_get_driver_data(dev);

	return starfive_pinctrl_probe(dev, info);
}

static const struct udevice_id jh7110_aon_pinctrl_ids[] = {
	/* JH7110 aon pinctrl */
	{ .compatible = "starfive,jh7110-aon-pinctrl",
	  .data = (ulong)&jh7110_aon_pinctrl_info, },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(jh7110_aon_pinctrl) = {
	.name		= "jh7110-aon-pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= jh7110_aon_pinctrl_ids,
	.priv_auto	= sizeof(struct starfive_pinctrl_priv),
	.ops		= &starfive_pinctrl_ops,
	.probe		= jh7110_aon_pinctrl_probe,
};
