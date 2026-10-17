// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY	UCLASS_GPIO

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <p2sb.h>
#include <pch.h>
#include <pci.h>
#include <syscon.h>
#include <acpi/acpi_device.h>
#include <asm/cpu.h>
#include <asm/gpio.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_pinctrl_defs.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/gpio.h>
#include <dm/acpi.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/x86-gpio.h>

static int intel_gpio_get_value(struct udevice *dev, uint offset)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint mode, rx_tx;
	u32 reg;

	reg = intel_pinctrl_get_config_reg(pinctrl, offset);
	mode = (reg & PAD_CFG0_MODE_MASK) >> PAD_CFG0_MODE_SHIFT;
	if (!mode) {
		rx_tx = reg & (PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE);
		if (rx_tx == PAD_CFG0_TX_DISABLE)
			return reg & PAD_CFG0_RX_STATE ? 1 : 0;
		else if (rx_tx == PAD_CFG0_RX_DISABLE)
			return reg & PAD_CFG0_TX_STATE ? 1 : 0;
	}

	return 0;
}

static int intel_gpio_set_value(struct udevice *dev, unsigned int offset,
				int value)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint config_offset;

	config_offset = intel_pinctrl_get_config_reg_offset(pinctrl, offset);

	pcr_clrsetbits32(pinctrl, config_offset, PAD_CFG0_TX_STATE,
			 value ? PAD_CFG0_TX_STATE : 0);

	return 0;
}

static int intel_gpio_get_function(struct udevice *dev, uint offset)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint mode, rx_tx;
	u32 reg;

	reg = intel_pinctrl_get_config_reg(pinctrl, offset);
	mode = (reg & PAD_CFG0_MODE_MASK) >> PAD_CFG0_MODE_SHIFT;
	if (!mode) {
		rx_tx = reg & (PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE);
		if (rx_tx == PAD_CFG0_TX_DISABLE)
			return GPIOF_INPUT;
		else if (rx_tx == PAD_CFG0_RX_DISABLE)
			return GPIOF_OUTPUT;
	}

	return GPIOF_FUNC;
}

static int intel_gpio_xlate(struct udevice *orig_dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	struct udevice *pinctrl, *dev;
	int gpio, ret;

	/*
	 * GPIO numbers are global in the device tree so it doesn't matter
	 * which @orig_dev is used
	 */
	gpio = args->args[0];
	ret = intel_pinctrl_get_pad(gpio, &pinctrl, &desc->offset);
	if (ret)
		return log_msg_ret("bad", ret);
	device_find_first_child(pinctrl, &dev);
	if (!dev)
		return log_msg_ret("no child", -ENOENT);
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;
	desc->dev = dev;

	/*
	 * Handle the case where the wrong GPIO device was provided, since this
	 * will not have been probed by the GPIO uclass before calling here
	 * (see gpio_request_tail()).
	 */
	if (orig_dev != dev) {
		ret = device_probe(dev);
		if (ret)
			return log_msg_ret("probe", ret);
	}

	return 0;
}

static int intel_gpio_set_flags(struct udevice *dev, unsigned int offset,
				ulong flags)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	u32 bic0 = 0, bic1 = 0;
	u32 or0, or1;
	uint config_offset;

	config_offset = intel_pinctrl_get_config_reg_offset(pinctrl, offset);

	if (flags & GPIOD_IS_OUT) {
		bic0 |= PAD_CFG0_MODE_MASK | PAD_CFG0_RX_STATE |
			PAD_CFG0_TX_DISABLE;
		or0 |= PAD_CFG0_MODE_GPIO | PAD_CFG0_RX_DISABLE;
	} else if (flags & GPIOD_IS_IN) {
		bic0 |= PAD_CFG0_MODE_MASK | PAD_CFG0_TX_STATE |
			PAD_CFG0_RX_DISABLE;
		or0 |= PAD_CFG0_MODE_GPIO | PAD_CFG0_TX_DISABLE;
	}
	if (flags & GPIOD_PULL_UP) {
		bic1 |= PAD_CFG1_PULL_MASK;
		or1 |= PAD_CFG1_PULL_UP_20K;
	} else if (flags & GPIOD_PULL_DOWN) {
		bic1 |= PAD_CFG1_PULL_MASK;
		or1 |= PAD_CFG1_PULL_DN_20K;
	}

	pcr_clrsetbits32(pinctrl, PAD_CFG0_OFFSET(config_offset), bic0, or0);
	pcr_clrsetbits32(pinctrl, PAD_CFG1_OFFSET(config_offset), bic1, or1);
	log_debug("%s: flags=%lx, offset=%x, config_offset=%x, %x/%x %x/%x\n",
		  dev->name, flags, offset, config_offset, bic0, or0, bic1, or1);

	return 0;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int intel_gpio_get_acpi(const struct gpio_desc *desc,
			       struct acpi_gpio *gpio)
{
	struct udevice *pinctrl;
	int ret;

	if (!dm_gpio_is_valid(desc))
		return -ENOENT;
	pinctrl = dev_get_parent(desc->dev);

	memset(gpio, '\0', sizeof(*gpio));

	gpio->type = ACPI_GPIO_TYPE_IO;
	gpio->pull = ACPI_GPIO_PULL_DEFAULT;
	gpio->io_restrict = ACPI_GPIO_IO_RESTRICT_OUTPUT;
	gpio->polarity = ACPI_GPIO_ACTIVE_HIGH;
	gpio->pin_count = 1;
	gpio->pins[0] = intel_pinctrl_get_acpi_pin(pinctrl, desc->offset);
	gpio->pin0_addr = intel_pinctrl_get_config_reg_addr(pinctrl,
							    desc->offset);
	ret = acpi_get_path(pinctrl, gpio->resource, sizeof(gpio->resource));
	if (ret)
		return log_msg_ret("resource", ret);

	return 0;
}
#endif

static int intel_gpio_probe(struct udevice *dev)
{
	return 0;
}

static int intel_gpio_of_to_plat(struct udevice *dev)
{
	struct gpio_dev_priv *upriv = dev_get_uclass_priv(dev);
	struct intel_pinctrl_priv *pinctrl_priv = dev_get_priv(dev->parent);
	const struct pad_community *comm = pinctrl_priv->comm;

	upriv->gpio_count = comm->last_pad - comm->first_pad + 1;
	upriv->bank_name = dev->name;

	return 0;
}

static const struct dm_gpio_ops gpio_intel_ops = {
	.get_value		= intel_gpio_get_value,
	.set_value		= intel_gpio_set_value,
	.get_function		= intel_gpio_get_function,
	.xlate			= intel_gpio_xlate,
	.set_flags		= intel_gpio_set_flags,
#if CONFIG_IS_ENABLED(ACPIGEN)
	.get_acpi		= intel_gpio_get_acpi,
#endif
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id intel_intel_gpio_ids[] = {
	{ .compatible = "intel,gpio" },
	{ }
};
#endif

U_BOOT_DRIVER(intel_gpio) = {
	.name	= "intel_gpio",
	.id	= UCLASS_GPIO,
	.of_match = of_match_ptr(intel_intel_gpio_ids),
	.ops	= &gpio_intel_ops,
	.of_to_plat	= intel_gpio_of_to_plat,
	.probe	= intel_gpio_probe,
};
