// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 * RGPIO2P driver for the Freescale i.MX7ULP.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <malloc.h>

enum imx_rgpio2p_direction {
	IMX_RGPIO2P_DIRECTION_IN,
	IMX_RGPIO2P_DIRECTION_OUT,
};

#define GPIO_PER_BANK			32

struct imx_rgpio2p_data {
	struct gpio_regs *regs;
};

struct imx_rgpio2p_plat {
	int bank_index;
	struct gpio_regs *regs;
};

static int imx_rgpio2p_is_output(struct gpio_regs *regs, int offset)
{
	u32 val;

	val = readl(&regs->gpio_pddr);

	return val & (1 << offset) ? 1 : 0;
}

static int imx_rgpio2p_bank_get_direction(struct gpio_regs *regs, int offset)
{
	if ((readl(&regs->gpio_pddr) >> offset) & 0x01)
		return IMX_RGPIO2P_DIRECTION_OUT;

	return IMX_RGPIO2P_DIRECTION_IN;
}

static void imx_rgpio2p_bank_direction(struct gpio_regs *regs, int offset,
				    enum imx_rgpio2p_direction direction)
{
	u32 l;

	l = readl(&regs->gpio_pddr);

	switch (direction) {
	case IMX_RGPIO2P_DIRECTION_OUT:
		l |= 1 << offset;
		break;
	case IMX_RGPIO2P_DIRECTION_IN:
		l &= ~(1 << offset);
	}
	writel(l, &regs->gpio_pddr);
}

static void imx_rgpio2p_bank_set_value(struct gpio_regs *regs, int offset,
				    int value)
{
	if (value)
		writel((1 << offset), &regs->gpio_psor);
	else
		writel((1 << offset), &regs->gpio_pcor);
}

static int imx_rgpio2p_bank_get_value(struct gpio_regs *regs, int offset)
{
	if (imx_rgpio2p_bank_get_direction(regs, offset) ==
	    IMX_RGPIO2P_DIRECTION_IN)
		return (readl(&regs->gpio_pdir) >> offset) & 0x01;

	return (readl(&regs->gpio_pdor) >> offset) & 0x01;
}

static int  imx_rgpio2p_direction_input(struct udevice *dev, unsigned offset)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);

	/* Configure GPIO direction as input. */
	imx_rgpio2p_bank_direction(bank->regs, offset, IMX_RGPIO2P_DIRECTION_IN);

	return 0;
}

static int imx_rgpio2p_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);

	/* Configure GPIO output value. */
	imx_rgpio2p_bank_set_value(bank->regs, offset, value);

	/* Configure GPIO direction as output. */
	imx_rgpio2p_bank_direction(bank->regs, offset, IMX_RGPIO2P_DIRECTION_OUT);

	return 0;
}

static int imx_rgpio2p_get_value(struct udevice *dev, unsigned offset)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);

	return imx_rgpio2p_bank_get_value(bank->regs, offset);
}

static int imx_rgpio2p_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);

	imx_rgpio2p_bank_set_value(bank->regs, offset, value);

	return 0;
}

static int imx_rgpio2p_get_function(struct udevice *dev, unsigned offset)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);

	/* GPIOF_FUNC is not implemented yet */
	if (imx_rgpio2p_is_output(bank->regs, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops imx_rgpio2p_ops = {
	.direction_input	= imx_rgpio2p_direction_input,
	.direction_output	= imx_rgpio2p_direction_output,
	.get_value		= imx_rgpio2p_get_value,
	.set_value		= imx_rgpio2p_set_value,
	.get_function		= imx_rgpio2p_get_function,
};

static int imx_rgpio2p_probe(struct udevice *dev)
{
	struct imx_rgpio2p_data *bank = dev_get_priv(dev);
	struct imx_rgpio2p_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int banknum;
	char name[18], *str;

	banknum = plat->bank_index;
	sprintf(name, "GPIO%d_", banknum + 1);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = GPIO_PER_BANK;
	bank->regs = plat->regs;

	return 0;
}

static int imx_rgpio2p_bind(struct udevice *dev)
{
	struct imx_rgpio2p_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	/*
	 * If plat already exsits, directly return.
	 * Actually only when DT is not supported, plat
	 * is statically initialized in U_BOOT_DRVINFOS.Here
	 * will return.
	 */
	if (plat)
		return 0;

	addr = devfdt_get_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	/*
	 * TODO:
	 * When every board is converted to driver model and DT is supported,
	 * this can be done by auto-alloc feature, but not using calloc
	 * to alloc memory for plat.
	 *
	 * For example imx_rgpio2p_plat uses platform data rather than device
	 * tree.
	 *
	 * NOTE: DO NOT COPY this code if you are using device tree.
	 */
	plat = calloc(1, sizeof(*plat));
	if (!plat)
		return -ENOMEM;

	plat->regs = (struct gpio_regs *)addr;
	plat->bank_index = dev_seq(dev);
	dev_set_plat(dev, plat);

	return 0;
}


static const struct udevice_id imx_rgpio2p_ids[] = {
	{ .compatible = "fsl,imx7ulp-gpio" },
	{ }
};

U_BOOT_DRIVER(imx_rgpio2p) = {
	.name	= "imx_rgpio2p",
	.id	= UCLASS_GPIO,
	.ops	= &imx_rgpio2p_ops,
	.probe	= imx_rgpio2p_probe,
	.priv_auto	= sizeof(struct imx_rgpio2p_plat),
	.of_match = imx_rgpio2p_ids,
	.bind	= imx_rgpio2p_bind,
};

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct imx_rgpio2p_plat imx_plat[] = {
	{ 0, (struct gpio_regs *)RGPIO2P_GPIO1_BASE_ADDR },
	{ 1, (struct gpio_regs *)RGPIO2P_GPIO2_BASE_ADDR },
	{ 2, (struct gpio_regs *)RGPIO2P_GPIO3_BASE_ADDR },
	{ 3, (struct gpio_regs *)RGPIO2P_GPIO4_BASE_ADDR },
	{ 4, (struct gpio_regs *)RGPIO2P_GPIO5_BASE_ADDR },
	{ 5, (struct gpio_regs *)RGPIO2P_GPIO6_BASE_ADDR },
};

U_BOOT_DRVINFOS(imx_rgpio2ps) = {
	{ "imx_rgpio2p", &imx_plat[0] },
	{ "imx_rgpio2p", &imx_plat[1] },
	{ "imx_rgpio2p", &imx_plat[2] },
	{ "imx_rgpio2p", &imx_plat[3] },
	{ "imx_rgpio2p", &imx_plat[4] },
	{ "imx_rgpio2p", &imx_plat[5] },
};
#endif
