// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>

#include "pinctrl-rockchip.h"

static int rk3066_pinctrl_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, ret, mask, mux_type;
	u8 bit;
	u32 data;

	regmap = priv->regmap_base;

	/* get basic quadrupel of mux registers and the correct reg inside */
	mux_type = bank->iomux[iomux_num].type;
	reg = bank->iomux[iomux_num].offset;
	reg += rockchip_get_mux_data(mux_type, pin, &bit, &mask);

	data = (mask << (bit + 16));
	data |= (mux & mask) << bit;
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define RK3066_PULL_OFFSET		0x118
#define RK3066_PULL_PINS_PER_REG	16
#define RK3066_PULL_BANK_STRIDE		8

static void rk3066_pinctrl_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
						 int pin_num, struct regmap **regmap,
						 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	*reg = RK3066_PULL_OFFSET;
	*reg += bank->bank_num * RK3066_PULL_BANK_STRIDE;
	*reg += (pin_num / RK3066_PULL_PINS_PER_REG) * 4;

	*bit = pin_num % RK3066_PULL_PINS_PER_REG;
};

static int rk3066_pinctrl_set_pull(struct rockchip_pin_bank *bank,
				   int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit;
	u32 data;

	if (pull != PIN_CONFIG_BIAS_PULL_PIN_DEFAULT &&
	    pull != PIN_CONFIG_BIAS_DISABLE)
		return -EOPNOTSUPP;

	rk3066_pinctrl_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	data = BIT(bit + 16);
	if (pull == PIN_CONFIG_BIAS_DISABLE)
		data |= BIT(bit);
	ret = regmap_write(regmap, reg, data);

	return ret;
}

static struct rockchip_pin_bank rk3066_pin_banks[] = {
	PIN_BANK(0, 32, "gpio0"),
	PIN_BANK(1, 32, "gpio1"),
	PIN_BANK(2, 32, "gpio2"),
	PIN_BANK(3, 32, "gpio3"),
	PIN_BANK(4, 32, "gpio4"),
	PIN_BANK(6, 16, "gpio6"),
};

static struct rockchip_pin_ctrl rk3066_pin_ctrl = {
	.pin_banks		= rk3066_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3066_pin_banks),
	.grf_mux_offset		= 0xa8,
	.set_mux		= rk3066_pinctrl_set_mux,
	.set_pull		= rk3066_pinctrl_set_pull,
};

static const struct udevice_id rk3066_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3066a-pinctrl",
		.data = (ulong)&rk3066_pin_ctrl
	},
	{}
};

U_BOOT_DRIVER(rockchip_rk3066a_pinctrl) = {
	.name		= "rockchip_rk3066a_pinctrl",
	.id		= UCLASS_PINCTRL,
	.ops		= &rockchip_pinctrl_ops,
	.probe		= rockchip_pinctrl_probe,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.of_match	= rk3066_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
};
