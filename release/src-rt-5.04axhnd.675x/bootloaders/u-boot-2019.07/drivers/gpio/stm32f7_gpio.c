// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <asm/arch/gpio.h>
#include <asm/arch/stm32.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/io.h>

#define MODE_BITS(gpio_pin)		(gpio_pin * 2)
#define MODE_BITS_MASK			3
#define BSRR_BIT(gpio_pin, value)	BIT(gpio_pin + (value ? 0 : 16))

#ifndef CONFIG_SPL_BUILD
/*
 * convert gpio offset to gpio index taking into account gpio holes
 * into gpio bank
 */
int stm32_offset_to_index(struct udevice *dev, unsigned int offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	int idx = 0;
	int i;

	for (i = 0; i < STM32_GPIOS_PER_BANK; i++) {
		if (priv->gpio_range & BIT(i)) {
			if (idx == offset)
				return idx;
			idx++;
		}
	}
	/* shouldn't happen */
	return -EINVAL;
}

static int stm32_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int bits_index;
	int mask;
	int idx;

	idx = stm32_offset_to_index(dev, offset);
	if (idx < 0)
		return idx;

	bits_index = MODE_BITS(idx);
	mask = MODE_BITS_MASK << bits_index;

	clrsetbits_le32(&regs->moder, mask, STM32_GPIO_MODE_IN << bits_index);

	return 0;
}

static int stm32_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int bits_index;
	int mask;
	int idx;

	idx = stm32_offset_to_index(dev, offset);
	if (idx < 0)
		return idx;

	bits_index = MODE_BITS(idx);
	mask = MODE_BITS_MASK << bits_index;

	clrsetbits_le32(&regs->moder, mask, STM32_GPIO_MODE_OUT << bits_index);

	writel(BSRR_BIT(idx, value), &regs->bsrr);

	return 0;
}

static int stm32_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int idx;

	idx = stm32_offset_to_index(dev, offset);
	if (idx < 0)
		return idx;

	return readl(&regs->idr) & BIT(idx) ? 1 : 0;
}

static int stm32_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int idx;

	idx = stm32_offset_to_index(dev, offset);
	if (idx < 0)
		return idx;

	writel(BSRR_BIT(idx, value), &regs->bsrr);

	return 0;
}

static int stm32_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct stm32_gpio_regs *regs = priv->regs;
	int bits_index;
	int mask;
	int idx;
	u32 mode;

	idx = stm32_offset_to_index(dev, offset);
	if (idx < 0)
		return idx;

	bits_index = MODE_BITS(idx);
	mask = MODE_BITS_MASK << bits_index;

	mode = (readl(&regs->moder) & mask) >> bits_index;
	if (mode == STM32_GPIO_MODE_OUT)
		return GPIOF_OUTPUT;
	if (mode == STM32_GPIO_MODE_IN)
		return GPIOF_INPUT;
	if (mode == STM32_GPIO_MODE_AN)
		return GPIOF_UNUSED;

	return GPIOF_FUNC;
}

static const struct dm_gpio_ops gpio_stm32_ops = {
	.direction_input	= stm32_gpio_direction_input,
	.direction_output	= stm32_gpio_direction_output,
	.get_value		= stm32_gpio_get_value,
	.set_value		= stm32_gpio_set_value,
	.get_function		= stm32_gpio_get_function,
};
#endif

static int gpio_stm32_probe(struct udevice *dev)
{
	struct stm32_gpio_priv *priv = dev_get_priv(dev);
	struct clk clk;
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = (struct stm32_gpio_regs *)addr;

#ifndef CONFIG_SPL_BUILD
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	const char *name;
	int i;

	name = dev_read_string(dev, "st,bank-name");
	if (!name)
		return -EINVAL;
	uc_priv->bank_name = name;

	i = 0;
	ret = dev_read_phandle_with_args(dev, "gpio-ranges",
					 NULL, 3, i, &args);

	if (ret == -ENOENT) {
		uc_priv->gpio_count = STM32_GPIOS_PER_BANK;
		priv->gpio_range = GENMASK(STM32_GPIOS_PER_BANK - 1, 0);
	}

	while (ret != -ENOENT) {
		priv->gpio_range |= GENMASK(args.args[2] + args.args[0] - 1,
				    args.args[0]);

		uc_priv->gpio_count += args.args[2];

		ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3,
						 ++i, &args);
	}

	dev_dbg(dev, "addr = 0x%p bank_name = %s gpio_count = %d gpio_range = 0x%x\n",
		(u32 *)priv->regs, uc_priv->bank_name, uc_priv->gpio_count,
		priv->gpio_range);
#endif
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);

	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
	debug("clock enabled for device %s\n", dev->name);

	return 0;
}

static const struct udevice_id stm32_gpio_ids[] = {
	{ .compatible = "st,stm32-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_stm32) = {
	.name	= "gpio_stm32",
	.id	= UCLASS_GPIO,
	.of_match = stm32_gpio_ids,
	.probe	= gpio_stm32_probe,
#ifndef CONFIG_SPL_BUILD
	.ops	= &gpio_stm32_ops,
#endif
	.flags	= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto_alloc_size	= sizeof(struct stm32_gpio_priv),
};
