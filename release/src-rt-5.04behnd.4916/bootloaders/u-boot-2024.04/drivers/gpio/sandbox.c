// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <acpi/acpi_device.h>
#include <asm/gpio.h>
#include <dm/acpi.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/of.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/gpio/sandbox-gpio.h>

struct gpio_state {
	const char *label;	/* label given by requester */
	ulong flags;		/* flags (GPIOD_...) */
};

/* Access routines for GPIO info */
static struct gpio_state *get_gpio_state(struct udevice *dev, uint offset)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct gpio_state *state = dev_get_priv(dev);

	if (offset >= uc_priv->gpio_count) {
		printf("sandbox_gpio: error: invalid gpio %u\n", offset);
		return NULL;
	}

	return &state[offset];
}

/* Access routines for GPIO flags */
static ulong *get_gpio_flags(struct udevice *dev, unsigned int offset)
{
	struct gpio_state *state = get_gpio_state(dev, offset);

	if (!state)
		return NULL;

	return &state->flags;

}

static int get_gpio_flag(struct udevice *dev, unsigned int offset, ulong flag)
{
	return (*get_gpio_flags(dev, offset) & flag) != 0;
}

static int set_gpio_flag(struct udevice *dev, unsigned int offset, ulong flag,
			 int value)
{
	struct gpio_state *state = get_gpio_state(dev, offset);

	if (value)
		state->flags |= flag;
	else
		state->flags &= ~flag;

	return 0;
}

/*
 * Back-channel sandbox-internal-only access to GPIO state
 */

int sandbox_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct gpio_state *state = get_gpio_state(dev, offset);
	bool val;

	if (get_gpio_flag(dev, offset, GPIOD_IS_OUT))
		debug("sandbox_gpio: get_value on output gpio %u\n", offset);

	if (state->flags & GPIOD_EXT_DRIVEN) {
		val = state->flags & GPIOD_EXT_HIGH;
	} else {
		if (state->flags & GPIOD_EXT_PULL_UP)
			val = true;
		else if (state->flags & GPIOD_EXT_PULL_DOWN)
			val = false;
		else
			val = state->flags & GPIOD_PULL_UP;
	}

	return val;
}

int sandbox_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	set_gpio_flag(dev, offset, GPIOD_EXT_DRIVEN | GPIOD_EXT_HIGH, value);

	return 0;
}

int sandbox_gpio_get_direction(struct udevice *dev, unsigned offset)
{
	return get_gpio_flag(dev, offset, GPIOD_IS_OUT);
}

int sandbox_gpio_set_direction(struct udevice *dev, unsigned offset, int output)
{
	set_gpio_flag(dev, offset, GPIOD_IS_OUT, output);
	set_gpio_flag(dev, offset, GPIOD_IS_IN, !output);

	return 0;
}

ulong sandbox_gpio_get_flags(struct udevice *dev, uint offset)
{
	ulong flags = *get_gpio_flags(dev, offset);

	return flags & ~GPIOD_SANDBOX_MASK;
}

int sandbox_gpio_set_flags(struct udevice *dev, uint offset, ulong flags)
{
	struct gpio_state *state = get_gpio_state(dev, offset);

	state->flags = flags;

	return 0;
}

/*
 * These functions implement the public interface within U-Boot
 */

/* set GPIO port 'offset' as an input */
static int sb_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_set_direction(dev, offset, 0);
}

/* set GPIO port 'offset' as an output, with polarity 'value' */
static int sb_gpio_direction_output(struct udevice *dev, unsigned offset,
				    int value)
{
	int ret;

	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	ret = sandbox_gpio_set_direction(dev, offset, 1);
	if (ret)
		return ret;
	ret = set_gpio_flag(dev, offset, GPIOD_IS_OUT_ACTIVE |
			    GPIOD_EXT_DRIVEN | GPIOD_EXT_HIGH, value);
	if (ret)
		return ret;

	return 0;
}

/* read GPIO IN value of port 'offset' */
static int sb_gpio_get_value(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_get_value(dev, offset);
}

/* write GPIO OUT value to port 'offset' */
static int sb_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	int ret;

	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (!sandbox_gpio_get_direction(dev, offset)) {
		printf("sandbox_gpio: error: set_value on input gpio %u\n",
		       offset);
		return -1;
	}

	ret = set_gpio_flag(dev, offset, GPIOD_IS_OUT_ACTIVE |
			    GPIOD_EXT_DRIVEN | GPIOD_EXT_HIGH, value);
	if (ret)
		return ret;

	return 0;
}

static int sb_gpio_get_function(struct udevice *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOD_IS_OUT))
		return GPIOF_OUTPUT;
	if (get_gpio_flag(dev, offset, GPIOD_IS_IN))
		return GPIOF_INPUT;
	if (get_gpio_flag(dev, offset, GPIOD_IS_AF))
		return GPIOF_FUNC;

	return GPIOF_INPUT; /*GPIO is not configurated */
}

static int sb_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			 struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];
	if (args->args_count < 2)
		return 0;
	/* treat generic binding with gpio uclass */
	gpio_xlate_offs_flags(dev, desc, args);

	/* sandbox test specific, not defined in gpio.h */
	if (args->args[1] & GPIO_IN)
		desc->flags |= GPIOD_IS_IN;

	if (args->args[1] & GPIO_OUT)
		desc->flags |= GPIOD_IS_OUT;

	if (args->args[1] & GPIO_OUT_ACTIVE)
		desc->flags |= GPIOD_IS_OUT_ACTIVE;

	if (args->args[1] & GPIO_AF)
		desc->flags |= GPIOD_IS_AF;

	return 0;
}

static int sb_gpio_set_flags(struct udevice *dev, unsigned int offset,
			     ulong flags)
{
	debug("%s: offset:%u, flags = %lx\n", __func__, offset, flags);
	struct gpio_state *state = get_gpio_state(dev, offset);

	if (flags & GPIOD_IS_OUT) {
		flags |= GPIOD_EXT_DRIVEN;
		if (flags & GPIOD_IS_OUT_ACTIVE)
			flags |= GPIOD_EXT_HIGH;
		else
			flags &= ~GPIOD_EXT_HIGH;
	} else {
		flags |= state->flags & GPIOD_SANDBOX_MASK;
	}
	state->flags = flags;

	return 0;
}

static int sb_gpio_get_flags(struct udevice *dev, uint offset, ulong *flagsp)
{
	debug("%s: offset:%u\n", __func__, offset);
	*flagsp = *get_gpio_flags(dev, offset) & ~GPIOD_SANDBOX_MASK;

	return 0;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int sb_gpio_get_acpi(const struct gpio_desc *desc,
			    struct acpi_gpio *gpio)
{
	int ret;

	/* Note that gpio_get_acpi() zeroes *gpio before calling here */
	gpio->pin_count = 1;
	gpio->pins[0] = desc->offset;
	ret = acpi_device_scope(desc->dev, gpio->resource,
				sizeof(gpio->resource));
	if (ret)
		return log_ret(ret);

	/* All of these values are just used for testing */
	if (desc->flags & GPIOD_ACTIVE_LOW) {
		gpio->pin0_addr = 0x80012 + desc->offset;
		gpio->type = ACPI_GPIO_TYPE_INTERRUPT;
		gpio->pull = ACPI_GPIO_PULL_DOWN;
		gpio->interrupt_debounce_timeout = 4321;

		/* We use the GpioInt part */
		gpio->irq.pin = desc->offset;
		gpio->irq.polarity = ACPI_IRQ_ACTIVE_BOTH;
		gpio->irq.shared = ACPI_IRQ_SHARED;
		gpio->irq.wake = ACPI_IRQ_WAKE;

		/* The GpioIo part is only used for testing */
		gpio->polarity = ACPI_GPIO_ACTIVE_LOW;
	} else {
		gpio->pin0_addr = 0xc00dc + desc->offset;
		gpio->type = ACPI_GPIO_TYPE_IO;
		gpio->pull = ACPI_GPIO_PULL_UP;
		gpio->interrupt_debounce_timeout = 0;

		/* The GpioInt part is not used */

		/* We use the GpioIo part */
		gpio->output_drive_strength = 1234;
		gpio->io_shared = true;
		gpio->io_restrict = ACPI_GPIO_IO_RESTRICT_INPUT;
		gpio->polarity = 0;
	}

	return 0;
}

static int sb_gpio_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "GPIO");
}

struct acpi_ops gpio_sandbox_acpi_ops = {
	.get_name	= sb_gpio_get_name,
};
#endif /* ACPIGEN */

static const struct dm_gpio_ops gpio_sandbox_ops = {
	.direction_input	= sb_gpio_direction_input,
	.direction_output	= sb_gpio_direction_output,
	.get_value		= sb_gpio_get_value,
	.set_value		= sb_gpio_set_value,
	.get_function		= sb_gpio_get_function,
	.xlate			= sb_gpio_xlate,
	.set_flags		= sb_gpio_set_flags,
	.get_flags		= sb_gpio_get_flags,
#if CONFIG_IS_ENABLED(ACPIGEN)
	.get_acpi		= sb_gpio_get_acpi,
#endif
};

static int sandbox_gpio_of_to_plat(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

		uc_priv->gpio_count =
			dev_read_u32_default(dev, "sandbox,gpio-count", 0);
		uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	}

	return 0;
}

static int gpio_sandbox_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!dev_has_ofnode(dev))
		/* Tell the uclass how many GPIOs we have */
		uc_priv->gpio_count = CONFIG_SANDBOX_GPIO_COUNT;

	dev_set_priv(dev,
		     calloc(sizeof(struct gpio_state), uc_priv->gpio_count));

	return 0;
}

static int gpio_sandbox_remove(struct udevice *dev)
{
	free(dev_get_priv(dev));

	return 0;
}

static const struct udevice_id sandbox_gpio_ids[] = {
	{ .compatible = "sandbox,gpio" },
	{ }
};

U_BOOT_DRIVER(sandbox_gpio) = {
	.name	= "sandbox_gpio",
	.id	= UCLASS_GPIO,
	.of_match = sandbox_gpio_ids,
	.of_to_plat = sandbox_gpio_of_to_plat,
	.probe	= gpio_sandbox_probe,
	.remove	= gpio_sandbox_remove,
	.ops	= &gpio_sandbox_ops,
	ACPI_OPS_PTR(&gpio_sandbox_acpi_ops)
};

DM_DRIVER_ALIAS(sandbox_gpio, sandbox_gpio_alias)

#if CONFIG_IS_ENABLED(PINCTRL)

/* pincontrol: used only to check GPIO pin configuration (pinmux command) */

struct sb_pinctrl_priv {
	int pinctrl_ngpios;
	struct list_head gpio_dev;
};

struct sb_gpio_bank {
	struct udevice *gpio_dev;
	struct list_head list;
};

static int sb_populate_gpio_dev_list(struct udevice *dev)
{
	struct sb_pinctrl_priv *priv = dev_get_priv(dev);
	struct udevice *gpio_dev;
	struct udevice *child;
	struct sb_gpio_bank *gpio_bank;
	int ret;

	/*
	 * parse pin-controller sub-nodes (ie gpio bank nodes) and fill
	 * a list with all gpio device reference which belongs to the
	 * current pin-controller. This list is used to find pin_name and
	 * pin muxing
	 */
	list_for_each_entry(child, &dev->child_head, sibling_node) {
		ret = uclass_get_device_by_name(UCLASS_GPIO, child->name,
						&gpio_dev);
		if (ret < 0)
			continue;

		gpio_bank = malloc(sizeof(*gpio_bank));
		if (!gpio_bank) {
			dev_err(dev, "Not enough memory\n");
			return -ENOMEM;
		}

		gpio_bank->gpio_dev = gpio_dev;
		list_add_tail(&gpio_bank->list, &priv->gpio_dev);
	}

	return 0;
}

static int sb_pinctrl_get_pins_count(struct udevice *dev)
{
	struct sb_pinctrl_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv;
	struct sb_gpio_bank *gpio_bank;

	/*
	 * if get_pins_count has already been executed once on this
	 * pin-controller, no need to run it again
	 */
	if (priv->pinctrl_ngpios)
		return priv->pinctrl_ngpios;

	if (list_empty(&priv->gpio_dev))
		sb_populate_gpio_dev_list(dev);
	/*
	 * walk through all banks to retrieve the pin-controller
	 * pins number
	 */
	list_for_each_entry(gpio_bank, &priv->gpio_dev, list) {
		uc_priv = dev_get_uclass_priv(gpio_bank->gpio_dev);

		priv->pinctrl_ngpios += uc_priv->gpio_count;
	}

	return priv->pinctrl_ngpios;
}

static struct udevice *sb_pinctrl_get_gpio_dev(struct udevice *dev,
					       unsigned int selector,
					       unsigned int *idx)
{
	struct sb_pinctrl_priv *priv = dev_get_priv(dev);
	struct sb_gpio_bank *gpio_bank;
	struct gpio_dev_priv *uc_priv;
	int pin_count = 0;

	if (list_empty(&priv->gpio_dev))
		sb_populate_gpio_dev_list(dev);

	/* look up for the bank which owns the requested pin */
	list_for_each_entry(gpio_bank, &priv->gpio_dev, list) {
		uc_priv = dev_get_uclass_priv(gpio_bank->gpio_dev);

		if (selector < (pin_count + uc_priv->gpio_count)) {
			/*
			 * we found the bank, convert pin selector to
			 * gpio bank index
			 */
			*idx = selector - pin_count;

			return gpio_bank->gpio_dev;
		}
		pin_count += uc_priv->gpio_count;
	}

	return NULL;
}

static const char *sb_pinctrl_get_pin_name(struct udevice *dev,
					   unsigned int selector)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *gpio_dev;
	unsigned int gpio_idx;
	static char pin_name[PINNAME_SIZE];

	/* look up for the bank which owns the requested pin */
	gpio_dev = sb_pinctrl_get_gpio_dev(dev, selector, &gpio_idx);
	if (!gpio_dev) {
		snprintf(pin_name, PINNAME_SIZE, "Error");
	} else {
		uc_priv = dev_get_uclass_priv(gpio_dev);

		snprintf(pin_name, PINNAME_SIZE, "%s%d",
			 uc_priv->bank_name,
			 gpio_idx);
	}

	return pin_name;
}

static char *get_flags_string(ulong flags)
{
	if (flags & GPIOD_OPEN_DRAIN)
		return "drive-open-drain";
	if (flags & GPIOD_OPEN_SOURCE)
		return "drive-open-source";
	if (flags & GPIOD_PULL_UP)
		return "bias-pull-up";
	if (flags & GPIOD_PULL_DOWN)
		return "bias-pull-down";
	return ".";
}

static int sb_pinctrl_get_pin_muxing(struct udevice *dev,
				     unsigned int selector,
				     char *buf, int size)
{
	struct udevice *gpio_dev;
	unsigned int gpio_idx;
	ulong flags;
	int function;

	/* look up for the bank which owns the requested pin */
	gpio_dev = sb_pinctrl_get_gpio_dev(dev, selector, &gpio_idx);
	if (!gpio_dev) {
		snprintf(buf, size, "Error");
	} else {
		function = sb_gpio_get_function(gpio_dev, gpio_idx);
		flags = *get_gpio_flags(gpio_dev, gpio_idx);

		snprintf(buf, size, "gpio %s %s",
			 function == GPIOF_OUTPUT ? "output" : "input",
			 get_flags_string(flags));
	}

	return 0;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int sb_pinctrl_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "PINC");
}
#endif

static int sandbox_pinctrl_probe(struct udevice *dev)
{
	struct sb_pinctrl_priv *priv = dev_get_priv(dev);

	INIT_LIST_HEAD(&priv->gpio_dev);

	return 0;
}

static struct pinctrl_ops sandbox_pinctrl_gpio_ops = {
	.get_pin_name		= sb_pinctrl_get_pin_name,
	.get_pins_count		= sb_pinctrl_get_pins_count,
	.get_pin_muxing		= sb_pinctrl_get_pin_muxing,
};

#if CONFIG_IS_ENABLED(ACPIGEN)
struct acpi_ops pinctrl_sandbox_acpi_ops = {
	.get_name	= sb_pinctrl_get_name,
};
#endif

static const struct udevice_id sandbox_pinctrl_gpio_match[] = {
	{ .compatible = "sandbox,pinctrl-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_pinctrl_gpio) = {
	.name = "sandbox_pinctrl_gpio",
	.id = UCLASS_PINCTRL,
	.of_match = sandbox_pinctrl_gpio_match,
	.ops = &sandbox_pinctrl_gpio_ops,
	.bind = dm_scan_fdt_dev,
	.probe = sandbox_pinctrl_probe,
	.priv_auto	= sizeof(struct sb_pinctrl_priv),
	ACPI_OPS_PTR(&pinctrl_sandbox_acpi_ops)
};

#endif /* PINCTRL */
