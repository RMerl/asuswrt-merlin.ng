// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 - 2018 Xilinx, Michal Simek
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm.h>
#include <dt-bindings/gpio/gpio.h>

#define XILINX_GPIO_MAX_BANK	2

/* Gpio simple map */
struct gpio_regs {
	u32 gpiodata;
	u32 gpiodir;
};

struct xilinx_gpio_platdata {
	struct gpio_regs *regs;
	int bank_max[XILINX_GPIO_MAX_BANK];
	int bank_input[XILINX_GPIO_MAX_BANK];
	int bank_output[XILINX_GPIO_MAX_BANK];
	u32 dout_default[XILINX_GPIO_MAX_BANK];
};

struct xilinx_gpio_privdata {
	u32 output_val[XILINX_GPIO_MAX_BANK];
};

static int xilinx_gpio_get_bank_pin(unsigned offset, u32 *bank_num,
				    u32 *bank_pin_num, struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	u32 bank, max_pins;
	/* the first gpio is 0 not 1 */
	u32 pin_num = offset;

	for (bank = 0; bank < XILINX_GPIO_MAX_BANK; bank++) {
		max_pins = platdata->bank_max[bank];
		if (pin_num < max_pins) {
			debug("%s: found at bank 0x%x pin 0x%x\n", __func__,
			      bank, pin_num);
			*bank_num = bank;
			*bank_pin_num = pin_num;
			return 0;
		}
		pin_num -= max_pins;
	}

	return -EINVAL;
}

static int xilinx_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	struct xilinx_gpio_privdata *priv = dev_get_priv(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	val = priv->output_val[bank];

	debug("%s: regs: %lx, value: %x, gpio: %x, bank %x, pin %x, out %x\n",
	      __func__, (ulong)platdata->regs, value, offset, bank, pin, val);

	if (value)
		val = val | (1 << pin);
	else
		val = val & ~(1 << pin);

	writel(val, &platdata->regs->gpiodata + bank * 2);

	priv->output_val[bank] = val;

	return 0;
};

static int xilinx_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	struct xilinx_gpio_privdata *priv = dev_get_priv(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	debug("%s: regs: %lx, gpio: %x, bank %x, pin %x\n", __func__,
	      (ulong)platdata->regs, offset, bank, pin);

	if (platdata->bank_output[bank]) {
		debug("%s: Read saved output value\n", __func__);
		val = priv->output_val[bank];
	} else {
		debug("%s: Read input value from reg\n", __func__);
		val = readl(&platdata->regs->gpiodata + bank * 2);
	}

	val = !!(val & (1 << pin));

	return val;
};

static int xilinx_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* Check if all pins are inputs */
	if (platdata->bank_input[bank])
		return GPIOF_INPUT;

	/* Check if all pins are outputs */
	if (platdata->bank_output[bank])
		return GPIOF_OUTPUT;

	/* FIXME test on dual */
	val = readl(&platdata->regs->gpiodir + bank * 2);
	val = !(val & (1 << pin));

	/* input is 1 in reg but GPIOF_INPUT is 0 */
	/* output is 0 in reg but GPIOF_OUTPUT is 1 */

	return val;
}

static int xilinx_gpio_direction_output(struct udevice *dev, unsigned offset,
					int value)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* can't change it if all is input by default */
	if (platdata->bank_input[bank])
		return -EINVAL;

	xilinx_gpio_set_value(dev, offset, value);

	if (!platdata->bank_output[bank]) {
		val = readl(&platdata->regs->gpiodir + bank * 2);
		val = val & ~(1 << pin);
		writel(val, &platdata->regs->gpiodir + bank * 2);
	}

	return 0;
}

static int xilinx_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int val, ret;
	u32 bank, pin;

	ret = xilinx_gpio_get_bank_pin(offset, &bank, &pin, dev);
	if (ret)
		return ret;

	/* Already input */
	if (platdata->bank_input[bank])
		return 0;

	/* can't change it if all is output by default */
	if (platdata->bank_output[bank])
		return -EINVAL;

	val = readl(&platdata->regs->gpiodir + bank * 2);
	val = val | (1 << pin);
	writel(val, &platdata->regs->gpiodir + bank * 2);

	return 0;
}

static int xilinx_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			     struct ofnode_phandle_args *args)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);

	desc->offset = args->args[0];

	debug("%s: argc: %x, [0]: %x, [1]: %x, [2]: %x\n", __func__,
	      args->args_count, args->args[0], args->args[1], args->args[2]);

	/*
	 * The second cell is channel offset:
	 *  0 is first channel, 8 is second channel
	 *
	 * U-Boot driver just combine channels together that's why simply
	 * add amount of pins in second channel if present.
	 */
	if (args->args[1]) {
		if (!platdata->bank_max[1]) {
			printf("%s: %s has no second channel\n",
			       __func__, dev->name);
			return -EINVAL;
		}

		desc->offset += platdata->bank_max[0];
	}

	/* The third cell is optional */
	if (args->args_count > 2)
		desc->flags = (args->args[2] &
			       GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0);

	debug("%s: offset %x, flags %lx\n",
	      __func__, desc->offset, desc->flags);
	return 0;
}

static const struct dm_gpio_ops xilinx_gpio_ops = {
	.direction_input = xilinx_gpio_direction_input,
	.direction_output = xilinx_gpio_direction_output,
	.get_value = xilinx_gpio_get_value,
	.set_value = xilinx_gpio_set_value,
	.get_function = xilinx_gpio_get_function,
	.xlate = xilinx_gpio_xlate,
};

static int xilinx_gpio_probe(struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	struct xilinx_gpio_privdata *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *label_ptr;

	label_ptr = dev_read_prop(dev, "label", NULL);
	if (label_ptr) {
		uc_priv->bank_name = strdup(label_ptr);
		if (!uc_priv->bank_name)
			return -ENOMEM;
	} else {
		uc_priv->bank_name = dev->name;
	}

	uc_priv->gpio_count = platdata->bank_max[0] + platdata->bank_max[1];

	priv->output_val[0] = platdata->dout_default[0];

	if (platdata->bank_max[1])
		priv->output_val[1] = platdata->dout_default[1];

	return 0;
}

static int xilinx_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct xilinx_gpio_platdata *platdata = dev_get_platdata(dev);
	int is_dual;

	platdata->regs = (struct gpio_regs *)dev_read_addr(dev);

	platdata->bank_max[0] = dev_read_u32_default(dev,
						     "xlnx,gpio-width", 0);
	platdata->bank_input[0] = dev_read_u32_default(dev,
						       "xlnx,all-inputs", 0);
	platdata->bank_output[0] = dev_read_u32_default(dev,
							"xlnx,all-outputs", 0);
	platdata->dout_default[0] = dev_read_u32_default(dev,
							 "xlnx,dout-default",
							 0);

	is_dual = dev_read_u32_default(dev, "xlnx,is-dual", 0);
	if (is_dual) {
		platdata->bank_max[1] = dev_read_u32_default(dev,
						"xlnx,gpio2-width", 0);
		platdata->bank_input[1] = dev_read_u32_default(dev,
						"xlnx,all-inputs-2", 0);
		platdata->bank_output[1] = dev_read_u32_default(dev,
						"xlnx,all-outputs-2", 0);
		platdata->dout_default[1] = dev_read_u32_default(dev,
						"xlnx,dout-default-2", 0);
	}

	return 0;
}

static const struct udevice_id xilinx_gpio_ids[] = {
	{ .compatible = "xlnx,xps-gpio-1.00.a",},
	{ }
};

U_BOOT_DRIVER(xilinx_gpio) = {
	.name = "xlnx_gpio",
	.id = UCLASS_GPIO,
	.ops = &xilinx_gpio_ops,
	.of_match = xilinx_gpio_ids,
	.ofdata_to_platdata = xilinx_gpio_ofdata_to_platdata,
	.probe = xilinx_gpio_probe,
	.platdata_auto_alloc_size = sizeof(struct xilinx_gpio_platdata),
	.priv_auto_alloc_size = sizeof(struct xilinx_gpio_privdata),
};
