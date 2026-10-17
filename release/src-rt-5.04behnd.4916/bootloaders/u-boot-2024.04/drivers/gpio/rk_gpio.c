// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/gpio.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <dt-bindings/pinctrl/rockchip.h>

#define SWPORT_DR		0x0000
#define SWPORT_DDR		0x0004
#define EXT_PORT		0x0050
#define SWPORT_DR_L		0x0000
#define SWPORT_DR_H		0x0004
#define SWPORT_DDR_L		0x0008
#define SWPORT_DDR_H		0x000C
#define EXT_PORT_V2		0x0070
#define VER_ID_V2		0x0078

enum {
	ROCKCHIP_GPIOS_PER_BANK		= 32,
};

struct rockchip_gpio_priv {
	void __iomem *regs;
	struct udevice *pinctrl;
	int bank;
	char name[2];
	u32 version;
};

static int rockchip_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	u32 mask = BIT(offset), data;

	if (priv->version)
		data = readl(priv->regs + EXT_PORT_V2);
	else
		data = readl(priv->regs + EXT_PORT);

	return (data & mask) ? 1 : 0;
}

static int rockchip_gpio_set_value(struct udevice *dev, unsigned offset,
				   int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	u32 mask = BIT(offset), data = value ? mask : 0;

	if (priv->version && offset >= 16)
		rk_clrsetreg(priv->regs + SWPORT_DR_H, mask >> 16, data >> 16);
	else if (priv->version)
		rk_clrsetreg(priv->regs + SWPORT_DR_L, mask, data);
	else
		clrsetbits_le32(priv->regs + SWPORT_DR, mask, data);

	return 0;
}

static int rockchip_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	u32 mask = BIT(offset);

	if (priv->version && offset >= 16)
		rk_clrreg(priv->regs + SWPORT_DDR_H, mask >> 16);
	else if (priv->version)
		rk_clrreg(priv->regs + SWPORT_DDR_L, mask);
	else
		clrbits_le32(priv->regs + SWPORT_DDR, mask);

	return 0;
}

static int rockchip_gpio_direction_output(struct udevice *dev, unsigned offset,
					  int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	u32 mask = BIT(offset);

	rockchip_gpio_set_value(dev, offset, value);

	if (priv->version && offset >= 16)
		rk_setreg(priv->regs + SWPORT_DDR_H, mask >> 16);
	else if (priv->version)
		rk_setreg(priv->regs + SWPORT_DDR_L, mask);
	else
		setbits_le32(priv->regs + SWPORT_DDR, mask);

	return 0;
}

static int rockchip_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	u32 mask = BIT(offset), data;
	int ret;

	if (CONFIG_IS_ENABLED(PINCTRL)) {
		ret = pinctrl_get_gpio_mux(priv->pinctrl, priv->bank, offset);
		if (ret < 0)
			return ret;
		else if (ret != RK_FUNC_GPIO)
			return GPIOF_FUNC;
	}

	if (priv->version && offset >= 16)
		data = readl(priv->regs + SWPORT_DDR_H) << 16;
	else if (priv->version)
		data = readl(priv->regs + SWPORT_DDR_L);
	else
		data = readl(priv->regs + SWPORT_DDR);

	return (data & mask) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

/* Simple SPL interface to GPIOs */
#ifdef CONFIG_SPL_BUILD

enum {
	PULL_NONE_1V8 = 0,
	PULL_DOWN_1V8 = 1,
	PULL_UP_1V8 = 3,
};

int spl_gpio_set_pull(void *vregs, uint gpio, int pull)
{
	u32 *regs = vregs;
	uint val;

	regs += gpio >> GPIO_BANK_SHIFT;
	gpio &= GPIO_OFFSET_MASK;
	switch (pull) {
	case GPIO_PULL_UP:
		val = PULL_UP_1V8;
		break;
	case GPIO_PULL_DOWN:
		val = PULL_DOWN_1V8;
		break;
	case GPIO_PULL_NORMAL:
	default:
		val = PULL_NONE_1V8;
		break;
	}
	clrsetbits_le32(regs, 3 << (gpio * 2), val << (gpio * 2));

	return 0;
}

int spl_gpio_output(void *vregs, uint gpio, int value)
{
	struct rockchip_gpio_regs * const regs = vregs;

	clrsetbits_le32(&regs->swport_dr, 1 << gpio, value << gpio);

	/* Set direction */
	clrsetbits_le32(&regs->swport_ddr, 1 << gpio, 1 << gpio);

	return 0;
}
#endif /* CONFIG_SPL_BUILD */

static int rockchip_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	char *end;
	int ret;

	priv->regs = dev_read_addr_ptr(dev);

	if (CONFIG_IS_ENABLED(PINCTRL)) {
		ret = uclass_first_device_err(UCLASS_PINCTRL, &priv->pinctrl);
		if (ret)
			return ret;
	}

	/*
	 * If "gpio-ranges" is present in the devicetree use it to parse
	 * the GPIO bank ID, otherwise use the legacy method.
	 */
	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev),
					     "gpio-ranges", NULL, 3,
					     0, &args);
	if (!ret || ret != -ENOENT) {
		uc_priv->gpio_count = args.args[2];
		priv->bank = args.args[1] / ROCKCHIP_GPIOS_PER_BANK;
	} else {
		uc_priv->gpio_count = ROCKCHIP_GPIOS_PER_BANK;
		end = strrchr(dev->name, '@');
		priv->bank = trailing_strtoln(dev->name, end);
	}

	priv->name[0] = 'A' + priv->bank;
	uc_priv->bank_name = priv->name;

	priv->version = readl(priv->regs + VER_ID_V2);

	return 0;
}

static const struct dm_gpio_ops gpio_rockchip_ops = {
	.direction_input	= rockchip_gpio_direction_input,
	.direction_output	= rockchip_gpio_direction_output,
	.get_value		= rockchip_gpio_get_value,
	.set_value		= rockchip_gpio_set_value,
	.get_function		= rockchip_gpio_get_function,
};

static const struct udevice_id rockchip_gpio_ids[] = {
	{ .compatible = "rockchip,gpio-bank" },
	{ }
};

U_BOOT_DRIVER(rockchip_gpio_bank) = {
	.name	= "rockchip_gpio_bank",
	.id	= UCLASS_GPIO,
	.of_match = rockchip_gpio_ids,
	.ops	= &gpio_rockchip_ops,
	.priv_auto	= sizeof(struct rockchip_gpio_priv),
	.probe	= rockchip_gpio_probe,
};
