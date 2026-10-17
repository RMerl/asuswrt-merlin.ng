// SPDX-License-Identifier: GPL-2.0
/*
 * ZynqMP GPIO modepin driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dm.h>
#include <asm/arch/hardware.h>
#include <zynqmp_firmware.h>

#define OUTEN(pin)		(BIT(0) << (pin))
#define INVAL(pin)		(BIT(4) << (pin))
#define OUTVAL(pin)		(BIT(8) << (pin))

#define ZYNQMP_CRL_APB_BOOTPIN_CTRL_MASK	0xF0F
#define ZYNQMP_CRL_APB_BOOT_PIN_CTRL		(ZYNQMP_CRL_APB_BASEADDR + \
						(0x250U))

static int get_gpio_modepin(u32 *ret_payload)
{
	return xilinx_pm_request(PM_MMIO_READ, ZYNQMP_CRL_APB_BOOT_PIN_CTRL,
				 0, 0, 0, ret_payload);
}

static int set_gpio_modepin(int val)
{
	return xilinx_pm_request(PM_MMIO_WRITE, ZYNQMP_CRL_APB_BOOT_PIN_CTRL,
				 ZYNQMP_CRL_APB_BOOTPIN_CTRL_MASK,
				 val, 0, NULL);
}

static int modepin_gpio_direction_input(struct udevice *dev,
					unsigned int offset)
{
	return 0;
}

static int modepin_gpio_set_value(struct udevice *dev, unsigned int offset,
				  int value)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 out_val = 0;
	int ret;

	ret = get_gpio_modepin(ret_payload);
	if (ret)
		return ret;

	if (value)
		out_val = OUTVAL(offset) | ret_payload[1];
	else
		out_val = ~OUTVAL(offset) & ret_payload[1];

	return set_gpio_modepin(out_val);
}

static int modepin_gpio_direction_output(struct udevice *dev,
					 unsigned int offset, int value)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 out_en = 0;
	int ret;

	ret = get_gpio_modepin(ret_payload);
	if (ret)
		return ret;

	if (value)
		out_en = OUTEN(offset) | ret_payload[1];
	else
		out_en = ~OUTEN(offset) & ret_payload[1];

	ret = set_gpio_modepin(out_en);
	if (ret)
		return ret;

	return modepin_gpio_set_value(dev, offset, value);
}

static int modepin_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			      struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];

	return 0;
}

static int modepin_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	ret = get_gpio_modepin(ret_payload);
	if (ret)
		return ret;

	return (INVAL(offset) & ret_payload[1]) ? 1 : 0;
}

static int modepin_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	ret = get_gpio_modepin(ret_payload);
	if (ret)
		return ret;

	return (OUTEN(offset) & ret_payload[1]) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops modepin_gpio_ops = {
	.direction_input = modepin_gpio_direction_input,
	.direction_output = modepin_gpio_direction_output,
	.get_value = modepin_gpio_get_value,
	.set_value = modepin_gpio_set_value,
	.get_function = modepin_gpio_get_function,
	.xlate = modepin_gpio_xlate,
};

static int modepin_gpio_probe(struct udevice *dev)
{
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

	uc_priv->gpio_count = 4;

	return 0;
}

static const struct udevice_id modepin_gpio_ids[] = {
	{ .compatible = "xlnx,zynqmp-gpio-modepin",},
	{ }
};

U_BOOT_DRIVER(modepin_gpio) = {
	.name = "modepin_gpio",
	.id = UCLASS_GPIO,
	.ops = &modepin_gpio_ops,
	.of_match = modepin_gpio_ids,
	.probe = modepin_gpio_probe,
};
