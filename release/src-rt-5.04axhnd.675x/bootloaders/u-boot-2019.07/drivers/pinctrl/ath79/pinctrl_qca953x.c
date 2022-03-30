// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <mach/ar71xx_regs.h>

DECLARE_GLOBAL_DATA_PTR;

enum periph_id {
	PERIPH_ID_UART0,
	PERIPH_ID_SPI0,
	PERIPH_ID_NONE = -1,
};

struct qca953x_pinctrl_priv {
	void __iomem *regs;
};

static void pinctrl_qca953x_spi_config(struct qca953x_pinctrl_priv *priv, int cs)
{
	switch (cs) {
	case 0:
		clrsetbits_be32(priv->regs + AR71XX_GPIO_REG_OE,
				QCA953X_GPIO(5) | QCA953X_GPIO(6) |
				QCA953X_GPIO(7), QCA953X_GPIO(8));

		clrsetbits_be32(priv->regs + QCA953X_GPIO_REG_OUT_FUNC1,
				QCA953X_GPIO_MUX_MASK(8) |
				QCA953X_GPIO_MUX_MASK(16) |
				QCA953X_GPIO_MUX_MASK(24),
				(QCA953X_GPIO_OUT_MUX_SPI_CS0 << 8) |
				(QCA953X_GPIO_OUT_MUX_SPI_CLK << 16) |
				(QCA953X_GPIO_OUT_MUX_SPI_MOSI << 24));

		clrsetbits_be32(priv->regs + QCA953X_GPIO_REG_IN_ENABLE0,
				QCA953X_GPIO_MUX_MASK(0),
				QCA953X_GPIO_IN_MUX_SPI_DATA_IN);

		setbits_be32(priv->regs + AR71XX_GPIO_REG_OUT,
			     QCA953X_GPIO(8));
		break;
	}
}

static void pinctrl_qca953x_uart_config(struct qca953x_pinctrl_priv *priv, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART0:
		clrsetbits_be32(priv->regs + AR71XX_GPIO_REG_OE,
				QCA953X_GPIO(9), QCA953X_GPIO(10));

		clrsetbits_be32(priv->regs + QCA953X_GPIO_REG_OUT_FUNC2,
				QCA953X_GPIO_MUX_MASK(16),
				QCA953X_GPIO_OUT_MUX_UART0_SOUT << 16);

		clrsetbits_be32(priv->regs + QCA953X_GPIO_REG_IN_ENABLE0,
				QCA953X_GPIO_MUX_MASK(8),
				QCA953X_GPIO_IN_MUX_UART0_SIN << 8);

		setbits_be32(priv->regs + AR71XX_GPIO_REG_OUT,
			     QCA953X_GPIO(10));
		break;
	}
}

static int qca953x_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct qca953x_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%x, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_SPI0:
		pinctrl_qca953x_spi_config(priv, flags);
		break;
	case PERIPH_ID_UART0:
		pinctrl_qca953x_uart_config(priv, func);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int qca953x_pinctrl_get_periph_id(struct udevice *dev,
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

static int qca953x_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = qca953x_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;
	return qca953x_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops qca953x_pinctrl_ops = {
	.set_state_simple	= qca953x_pinctrl_set_state_simple,
	.request	= qca953x_pinctrl_request,
	.get_periph_id	= qca953x_pinctrl_get_periph_id,
};

static int qca953x_pinctrl_probe(struct udevice *dev)
{
	struct qca953x_pinctrl_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr,
				 AR71XX_GPIO_SIZE,
				 MAP_NOCACHE);
	return 0;
}

static const struct udevice_id qca953x_pinctrl_ids[] = {
	{ .compatible = "qca,qca953x-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_qca953x) = {
	.name		= "pinctrl_qca953x",
	.id		= UCLASS_PINCTRL,
	.of_match	= qca953x_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct qca953x_pinctrl_priv),
	.ops		= &qca953x_pinctrl_ops,
	.probe		= qca953x_pinctrl_probe,
};
