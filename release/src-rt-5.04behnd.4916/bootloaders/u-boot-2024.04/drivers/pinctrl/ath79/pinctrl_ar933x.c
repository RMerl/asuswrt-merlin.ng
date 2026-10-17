// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <mach/ar71xx_regs.h>

DECLARE_GLOBAL_DATA_PTR;

enum periph_id {
	PERIPH_ID_UART0,
	PERIPH_ID_SPI0,
	PERIPH_ID_NONE = -1,
};

struct ar933x_pinctrl_priv {
	void __iomem *regs;
};

static void pinctrl_ar933x_spi_config(struct ar933x_pinctrl_priv *priv, int cs)
{
	switch (cs) {
	case 0:
		clrsetbits_be32(priv->regs + AR71XX_GPIO_REG_OE,
				AR933X_GPIO(4), AR933X_GPIO(3) |
				AR933X_GPIO(5) | AR933X_GPIO(2));
		setbits_be32(priv->regs + AR71XX_GPIO_REG_FUNC,
			     AR933X_GPIO_FUNC_SPI_EN |
			     AR933X_GPIO_FUNC_RES_TRUE);
		break;
	}
}

static void pinctrl_ar933x_uart_config(struct ar933x_pinctrl_priv *priv, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART0:
		clrsetbits_be32(priv->regs + AR71XX_GPIO_REG_OE,
				AR933X_GPIO(9), AR933X_GPIO(10));
		setbits_be32(priv->regs + AR71XX_GPIO_REG_FUNC,
			     AR933X_GPIO_FUNC_UART_EN |
			     AR933X_GPIO_FUNC_RES_TRUE);
		break;
	}
}

static int ar933x_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct ar933x_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_SPI0:
		pinctrl_ar933x_spi_config(priv, flags);
		break;
	case PERIPH_ID_UART0:
		pinctrl_ar933x_uart_config(priv, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ar933x_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[2];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(periph),
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[0]) {
	case 128:
		return PERIPH_ID_UART0;
	case 129:
		return PERIPH_ID_SPI0;
	}
	return -ENOENT;
}

static int ar933x_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = ar933x_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return ar933x_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops ar933x_pinctrl_ops = {
	.set_state_simple	= ar933x_pinctrl_set_state_simple,
	.request	= ar933x_pinctrl_request,
	.get_periph_id	= ar933x_pinctrl_get_periph_id,
};

static int ar933x_pinctrl_probe(struct udevice *dev)
{
	struct ar933x_pinctrl_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr,
				 AR71XX_GPIO_SIZE,
				 MAP_NOCACHE);
	return 0;
}

static const struct udevice_id ar933x_pinctrl_ids[] = {
	{ .compatible = "qca,ar933x-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_ar933x) = {
	.name		= "pinctrl_ar933x",
	.id		= UCLASS_PINCTRL,
	.of_match	= ar933x_pinctrl_ids,
	.priv_auto	= sizeof(struct ar933x_pinctrl_priv),
	.ops		= &ar933x_pinctrl_ops,
	.probe		= ar933x_pinctrl_probe,
};
