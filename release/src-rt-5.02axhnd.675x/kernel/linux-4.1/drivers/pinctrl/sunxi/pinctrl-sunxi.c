/*
 * Allwinner A1X SoCs pinctrl driver.
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "../core.h"
#include "../../gpio/gpiolib.h"
#include "pinctrl-sunxi.h"

static struct irq_chip sunxi_pinctrl_edge_irq_chip;
static struct irq_chip sunxi_pinctrl_level_irq_chip;

static struct sunxi_pinctrl_group *
sunxi_pinctrl_find_group_by_name(struct sunxi_pinctrl *pctl, const char *group)
{
	int i;

	for (i = 0; i < pctl->ngroups; i++) {
		struct sunxi_pinctrl_group *grp = pctl->groups + i;

		if (!strcmp(grp->name, group))
			return grp;
	}

	return NULL;
}

static struct sunxi_pinctrl_function *
sunxi_pinctrl_find_function_by_name(struct sunxi_pinctrl *pctl,
				    const char *name)
{
	struct sunxi_pinctrl_function *func = pctl->functions;
	int i;

	for (i = 0; i < pctl->nfunctions; i++) {
		if (!func[i].name)
			break;

		if (!strcmp(func[i].name, name))
			return func + i;
	}

	return NULL;
}

static struct sunxi_desc_function *
sunxi_pinctrl_desc_find_function_by_name(struct sunxi_pinctrl *pctl,
					 const char *pin_name,
					 const char *func_name)
{
	int i;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;

		if (!strcmp(pin->pin.name, pin_name)) {
			struct sunxi_desc_function *func = pin->functions;

			while (func->name) {
				if (!strcmp(func->name, func_name))
					return func;

				func++;
			}
		}
	}

	return NULL;
}

static struct sunxi_desc_function *
sunxi_pinctrl_desc_find_function_by_pin(struct sunxi_pinctrl *pctl,
					const u16 pin_num,
					const char *func_name)
{
	int i;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;

		if (pin->pin.number == pin_num) {
			struct sunxi_desc_function *func = pin->functions;

			while (func->name) {
				if (!strcmp(func->name, func_name))
					return func;

				func++;
			}
		}
	}

	return NULL;
}

static int sunxi_pctrl_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->ngroups;
}

static const char *sunxi_pctrl_get_group_name(struct pinctrl_dev *pctldev,
					      unsigned group)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->groups[group].name;
}

static int sunxi_pctrl_get_group_pins(struct pinctrl_dev *pctldev,
				      unsigned group,
				      const unsigned **pins,
				      unsigned *num_pins)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*pins = (unsigned *)&pctl->groups[group].pin;
	*num_pins = 1;

	return 0;
}

static int sunxi_pctrl_dt_node_to_map(struct pinctrl_dev *pctldev,
				      struct device_node *node,
				      struct pinctrl_map **map,
				      unsigned *num_maps)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long *pinconfig;
	struct property *prop;
	const char *function;
	const char *group;
	int ret, nmaps, i = 0;
	u32 val;

	*map = NULL;
	*num_maps = 0;

	ret = of_property_read_string(node, "allwinner,function", &function);
	if (ret) {
		dev_err(pctl->dev,
			"missing allwinner,function property in node %s\n",
			node->name);
		return -EINVAL;
	}

	nmaps = of_property_count_strings(node, "allwinner,pins") * 2;
	if (nmaps < 0) {
		dev_err(pctl->dev,
			"missing allwinner,pins property in node %s\n",
			node->name);
		return -EINVAL;
	}

	*map = kmalloc(nmaps * sizeof(struct pinctrl_map), GFP_KERNEL);
	if (!*map)
		return -ENOMEM;

	of_property_for_each_string(node, "allwinner,pins", prop, group) {
		struct sunxi_pinctrl_group *grp =
			sunxi_pinctrl_find_group_by_name(pctl, group);
		int j = 0, configlen = 0;

		if (!grp) {
			dev_err(pctl->dev, "unknown pin %s", group);
			continue;
		}

		if (!sunxi_pinctrl_desc_find_function_by_name(pctl,
							      grp->name,
							      function)) {
			dev_err(pctl->dev, "unsupported function %s on pin %s",
				function, group);
			continue;
		}

		(*map)[i].type = PIN_MAP_TYPE_MUX_GROUP;
		(*map)[i].data.mux.group = group;
		(*map)[i].data.mux.function = function;

		i++;

		(*map)[i].type = PIN_MAP_TYPE_CONFIGS_GROUP;
		(*map)[i].data.configs.group_or_pin = group;

		if (of_find_property(node, "allwinner,drive", NULL))
			configlen++;
		if (of_find_property(node, "allwinner,pull", NULL))
			configlen++;

		pinconfig = kzalloc(configlen * sizeof(*pinconfig), GFP_KERNEL);
		if (!pinconfig) {
			kfree(*map);
			return -ENOMEM;
		}

		if (!of_property_read_u32(node, "allwinner,drive", &val)) {
			u16 strength = (val + 1) * 10;
			pinconfig[j++] =
				pinconf_to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
							 strength);
		}

		if (!of_property_read_u32(node, "allwinner,pull", &val)) {
			enum pin_config_param pull = PIN_CONFIG_END;
			if (val == 1)
				pull = PIN_CONFIG_BIAS_PULL_UP;
			else if (val == 2)
				pull = PIN_CONFIG_BIAS_PULL_DOWN;
			pinconfig[j++] = pinconf_to_config_packed(pull, 0);
		}

		(*map)[i].data.configs.configs = pinconfig;
		(*map)[i].data.configs.num_configs = configlen;

		i++;
	}

	*num_maps = nmaps;

	return 0;
}

static void sunxi_pctrl_dt_free_map(struct pinctrl_dev *pctldev,
				    struct pinctrl_map *map,
				    unsigned num_maps)
{
	int i;

	for (i = 0; i < num_maps; i++) {
		if (map[i].type == PIN_MAP_TYPE_CONFIGS_GROUP)
			kfree(map[i].data.configs.configs);
	}

	kfree(map);
}

static const struct pinctrl_ops sunxi_pctrl_ops = {
	.dt_node_to_map		= sunxi_pctrl_dt_node_to_map,
	.dt_free_map		= sunxi_pctrl_dt_free_map,
	.get_groups_count	= sunxi_pctrl_get_groups_count,
	.get_group_name		= sunxi_pctrl_get_group_name,
	.get_group_pins		= sunxi_pctrl_get_group_pins,
};

static int sunxi_pconf_group_get(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *config)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*config = pctl->groups[group].config;

	return 0;
}

static int sunxi_pconf_group_set(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *configs,
				 unsigned num_configs)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct sunxi_pinctrl_group *g = &pctl->groups[group];
	unsigned long flags;
	unsigned pin = g->pin - pctl->desc->pin_base;
	u32 val, mask;
	u16 strength;
	u8 dlevel;
	int i;

	spin_lock_irqsave(&pctl->lock, flags);

	for (i = 0; i < num_configs; i++) {
		switch (pinconf_to_config_param(configs[i])) {
		case PIN_CONFIG_DRIVE_STRENGTH:
			strength = pinconf_to_config_argument(configs[i]);
			if (strength > 40) {
				spin_unlock_irqrestore(&pctl->lock, flags);
				return -EINVAL;
			}
			/*
			 * We convert from mA to what the register expects:
			 *   0: 10mA
			 *   1: 20mA
			 *   2: 30mA
			 *   3: 40mA
			 */
			dlevel = strength / 10 - 1;
			val = readl(pctl->membase + sunxi_dlevel_reg(pin));
			mask = DLEVEL_PINS_MASK << sunxi_dlevel_offset(pin);
			writel((val & ~mask)
				| dlevel << sunxi_dlevel_offset(pin),
				pctl->membase + sunxi_dlevel_reg(pin));
			break;
		case PIN_CONFIG_BIAS_PULL_UP:
			val = readl(pctl->membase + sunxi_pull_reg(pin));
			mask = PULL_PINS_MASK << sunxi_pull_offset(pin);
			writel((val & ~mask) | 1 << sunxi_pull_offset(pin),
				pctl->membase + sunxi_pull_reg(pin));
			break;
		case PIN_CONFIG_BIAS_PULL_DOWN:
			val = readl(pctl->membase + sunxi_pull_reg(pin));
			mask = PULL_PINS_MASK << sunxi_pull_offset(pin);
			writel((val & ~mask) | 2 << sunxi_pull_offset(pin),
				pctl->membase + sunxi_pull_reg(pin));
			break;
		default:
			break;
		}
		/* cache the config value */
		g->config = configs[i];
	} /* for each config */

	spin_unlock_irqrestore(&pctl->lock, flags);

	return 0;
}

static const struct pinconf_ops sunxi_pconf_ops = {
	.pin_config_group_get	= sunxi_pconf_group_get,
	.pin_config_group_set	= sunxi_pconf_group_set,
};

static int sunxi_pmx_get_funcs_cnt(struct pinctrl_dev *pctldev)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->nfunctions;
}

static const char *sunxi_pmx_get_func_name(struct pinctrl_dev *pctldev,
					   unsigned function)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->functions[function].name;
}

static int sunxi_pmx_get_func_groups(struct pinctrl_dev *pctldev,
				     unsigned function,
				     const char * const **groups,
				     unsigned * const num_groups)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*groups = pctl->functions[function].groups;
	*num_groups = pctl->functions[function].ngroups;

	return 0;
}

static void sunxi_pmx_set(struct pinctrl_dev *pctldev,
				 unsigned pin,
				 u8 config)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long flags;
	u32 val, mask;

	spin_lock_irqsave(&pctl->lock, flags);

	pin -= pctl->desc->pin_base;
	val = readl(pctl->membase + sunxi_mux_reg(pin));
	mask = MUX_PINS_MASK << sunxi_mux_offset(pin);
	writel((val & ~mask) | config << sunxi_mux_offset(pin),
		pctl->membase + sunxi_mux_reg(pin));

	spin_unlock_irqrestore(&pctl->lock, flags);
}

static int sunxi_pmx_set_mux(struct pinctrl_dev *pctldev,
			     unsigned function,
			     unsigned group)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct sunxi_pinctrl_group *g = pctl->groups + group;
	struct sunxi_pinctrl_function *func = pctl->functions + function;
	struct sunxi_desc_function *desc =
		sunxi_pinctrl_desc_find_function_by_name(pctl,
							 g->name,
							 func->name);

	if (!desc)
		return -EINVAL;

	sunxi_pmx_set(pctldev, g->pin, desc->muxval);

	return 0;
}

static int
sunxi_pmx_gpio_set_direction(struct pinctrl_dev *pctldev,
			struct pinctrl_gpio_range *range,
			unsigned offset,
			bool input)
{
	struct sunxi_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct sunxi_desc_function *desc;
	const char *func;

	if (input)
		func = "gpio_in";
	else
		func = "gpio_out";

	desc = sunxi_pinctrl_desc_find_function_by_pin(pctl, offset, func);
	if (!desc)
		return -EINVAL;

	sunxi_pmx_set(pctldev, offset, desc->muxval);

	return 0;
}

static const struct pinmux_ops sunxi_pmx_ops = {
	.get_functions_count	= sunxi_pmx_get_funcs_cnt,
	.get_function_name	= sunxi_pmx_get_func_name,
	.get_function_groups	= sunxi_pmx_get_func_groups,
	.set_mux		= sunxi_pmx_set_mux,
	.gpio_set_direction	= sunxi_pmx_gpio_set_direction,
};

static int sunxi_pinctrl_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	return pinctrl_request_gpio(chip->base + offset);
}

static void sunxi_pinctrl_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	pinctrl_free_gpio(chip->base + offset);
}

static int sunxi_pinctrl_gpio_direction_input(struct gpio_chip *chip,
					unsigned offset)
{
	return pinctrl_gpio_direction_input(chip->base + offset);
}

static int sunxi_pinctrl_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct sunxi_pinctrl *pctl = dev_get_drvdata(chip->dev);
	u32 reg = sunxi_data_reg(offset);
	u8 index = sunxi_data_offset(offset);
	u32 set_mux = pctl->desc->irq_read_needs_mux &&
			test_bit(FLAG_USED_AS_IRQ, &chip->desc[offset].flags);
	u32 val;

	if (set_mux)
		sunxi_pmx_set(pctl->pctl_dev, offset, SUN4I_FUNC_INPUT);

	val = (readl(pctl->membase + reg) >> index) & DATA_PINS_MASK;

	if (set_mux)
		sunxi_pmx_set(pctl->pctl_dev, offset, SUN4I_FUNC_IRQ);

	return val;
}

static void sunxi_pinctrl_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct sunxi_pinctrl *pctl = dev_get_drvdata(chip->dev);
	u32 reg = sunxi_data_reg(offset);
	u8 index = sunxi_data_offset(offset);
	unsigned long flags;
	u32 regval;

	spin_lock_irqsave(&pctl->lock, flags);

	regval = readl(pctl->membase + reg);

	if (value)
		regval |= BIT(index);
	else
		regval &= ~(BIT(index));

	writel(regval, pctl->membase + reg);

	spin_unlock_irqrestore(&pctl->lock, flags);
}

static int sunxi_pinctrl_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	sunxi_pinctrl_gpio_set(chip, offset, value);
	return pinctrl_gpio_direction_output(chip->base + offset);
}

static int sunxi_pinctrl_gpio_of_xlate(struct gpio_chip *gc,
				const struct of_phandle_args *gpiospec,
				u32 *flags)
{
	int pin, base;

	base = PINS_PER_BANK * gpiospec->args[0];
	pin = base + gpiospec->args[1];

	if (pin > gc->ngpio)
		return -EINVAL;

	if (flags)
		*flags = gpiospec->args[2];

	return pin;
}

static int sunxi_pinctrl_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct sunxi_pinctrl *pctl = dev_get_drvdata(chip->dev);
	struct sunxi_desc_function *desc;
	unsigned pinnum = pctl->desc->pin_base + offset;
	unsigned irqnum;

	if (offset >= chip->ngpio)
		return -ENXIO;

	desc = sunxi_pinctrl_desc_find_function_by_pin(pctl, pinnum, "irq");
	if (!desc)
		return -EINVAL;

	irqnum = desc->irqbank * IRQ_PER_BANK + desc->irqnum;

	dev_dbg(chip->dev, "%s: request IRQ for GPIO %d, return %d\n",
		chip->label, offset + chip->base, irqnum);

	return irq_find_mapping(pctl->domain, irqnum);
}

static int sunxi_pinctrl_irq_request_resources(struct irq_data *d)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);
	struct sunxi_desc_function *func;
	int ret;

	func = sunxi_pinctrl_desc_find_function_by_pin(pctl,
					pctl->irq_array[d->hwirq], "irq");
	if (!func)
		return -EINVAL;

	ret = gpiochip_lock_as_irq(pctl->chip,
			pctl->irq_array[d->hwirq] - pctl->desc->pin_base);
	if (ret) {
		dev_err(pctl->dev, "unable to lock HW IRQ %lu for IRQ\n",
			irqd_to_hwirq(d));
		return ret;
	}

	/* Change muxing to INT mode */
	sunxi_pmx_set(pctl->pctl_dev, pctl->irq_array[d->hwirq], func->muxval);

	return 0;
}

static void sunxi_pinctrl_irq_release_resources(struct irq_data *d)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);

	gpiochip_unlock_as_irq(pctl->chip,
			      pctl->irq_array[d->hwirq] - pctl->desc->pin_base);
}

static int sunxi_pinctrl_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);
	struct irq_desc *desc = container_of(d, struct irq_desc, irq_data);
	u32 reg = sunxi_irq_cfg_reg(d->hwirq);
	u8 index = sunxi_irq_cfg_offset(d->hwirq);
	unsigned long flags;
	u32 regval;
	u8 mode;

	switch (type) {
	case IRQ_TYPE_EDGE_RISING:
		mode = IRQ_EDGE_RISING;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		mode = IRQ_EDGE_FALLING;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		mode = IRQ_EDGE_BOTH;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		mode = IRQ_LEVEL_HIGH;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		mode = IRQ_LEVEL_LOW;
		break;
	default:
		return -EINVAL;
	}

	if (type & IRQ_TYPE_LEVEL_MASK) {
		d->chip = &sunxi_pinctrl_level_irq_chip;
		desc->handle_irq = handle_fasteoi_irq;
	} else {
		d->chip = &sunxi_pinctrl_edge_irq_chip;
		desc->handle_irq = handle_edge_irq;
	}

	spin_lock_irqsave(&pctl->lock, flags);

	regval = readl(pctl->membase + reg);
	regval &= ~(IRQ_CFG_IRQ_MASK << index);
	writel(regval | (mode << index), pctl->membase + reg);

	spin_unlock_irqrestore(&pctl->lock, flags);

	return 0;
}

static void sunxi_pinctrl_irq_ack(struct irq_data *d)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);
	u32 status_reg = sunxi_irq_status_reg(d->hwirq);
	u8 status_idx = sunxi_irq_status_offset(d->hwirq);

	/* Clear the IRQ */
	writel(1 << status_idx, pctl->membase + status_reg);
}

static void sunxi_pinctrl_irq_mask(struct irq_data *d)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);
	u32 reg = sunxi_irq_ctrl_reg(d->hwirq);
	u8 idx = sunxi_irq_ctrl_offset(d->hwirq);
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&pctl->lock, flags);

	/* Mask the IRQ */
	val = readl(pctl->membase + reg);
	writel(val & ~(1 << idx), pctl->membase + reg);

	spin_unlock_irqrestore(&pctl->lock, flags);
}

static void sunxi_pinctrl_irq_unmask(struct irq_data *d)
{
	struct sunxi_pinctrl *pctl = irq_data_get_irq_chip_data(d);
	u32 reg = sunxi_irq_ctrl_reg(d->hwirq);
	u8 idx = sunxi_irq_ctrl_offset(d->hwirq);
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&pctl->lock, flags);

	/* Unmask the IRQ */
	val = readl(pctl->membase + reg);
	writel(val | (1 << idx), pctl->membase + reg);

	spin_unlock_irqrestore(&pctl->lock, flags);
}

static void sunxi_pinctrl_irq_ack_unmask(struct irq_data *d)
{
	sunxi_pinctrl_irq_ack(d);
	sunxi_pinctrl_irq_unmask(d);
}

static struct irq_chip sunxi_pinctrl_edge_irq_chip = {
	.irq_ack	= sunxi_pinctrl_irq_ack,
	.irq_mask	= sunxi_pinctrl_irq_mask,
	.irq_unmask	= sunxi_pinctrl_irq_unmask,
	.irq_request_resources = sunxi_pinctrl_irq_request_resources,
	.irq_release_resources = sunxi_pinctrl_irq_release_resources,
	.irq_set_type	= sunxi_pinctrl_irq_set_type,
	.flags		= IRQCHIP_SKIP_SET_WAKE,
};

static struct irq_chip sunxi_pinctrl_level_irq_chip = {
	.irq_eoi	= sunxi_pinctrl_irq_ack,
	.irq_mask	= sunxi_pinctrl_irq_mask,
	.irq_unmask	= sunxi_pinctrl_irq_unmask,
	/* Define irq_enable / disable to avoid spurious irqs for drivers
	 * using these to suppress irqs while they clear the irq source */
	.irq_enable	= sunxi_pinctrl_irq_ack_unmask,
	.irq_disable	= sunxi_pinctrl_irq_mask,
	.irq_request_resources = sunxi_pinctrl_irq_request_resources,
	.irq_release_resources = sunxi_pinctrl_irq_release_resources,
	.irq_set_type	= sunxi_pinctrl_irq_set_type,
	.flags		= IRQCHIP_SKIP_SET_WAKE | IRQCHIP_EOI_THREADED |
			  IRQCHIP_EOI_IF_HANDLED,
};

static void sunxi_pinctrl_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct irq_chip *chip = irq_get_chip(irq);
	struct sunxi_pinctrl *pctl = irq_get_handler_data(irq);
	unsigned long bank, reg, val;

	for (bank = 0; bank < pctl->desc->irq_banks; bank++)
		if (irq == pctl->irq[bank])
			break;

	if (bank == pctl->desc->irq_banks)
		return;

	reg = sunxi_irq_status_reg_from_bank(bank);
	val = readl(pctl->membase + reg);

	if (val) {
		int irqoffset;

		chained_irq_enter(chip, desc);
		for_each_set_bit(irqoffset, &val, IRQ_PER_BANK) {
			int pin_irq = irq_find_mapping(pctl->domain,
						       bank * IRQ_PER_BANK + irqoffset);
			generic_handle_irq(pin_irq);
		}
		chained_irq_exit(chip, desc);
	}
}

static int sunxi_pinctrl_add_function(struct sunxi_pinctrl *pctl,
					const char *name)
{
	struct sunxi_pinctrl_function *func = pctl->functions;

	while (func->name) {
		/* function already there */
		if (strcmp(func->name, name) == 0) {
			func->ngroups++;
			return -EEXIST;
		}
		func++;
	}

	func->name = name;
	func->ngroups = 1;

	pctl->nfunctions++;

	return 0;
}

static int sunxi_pinctrl_build_state(struct platform_device *pdev)
{
	struct sunxi_pinctrl *pctl = platform_get_drvdata(pdev);
	int i;

	pctl->ngroups = pctl->desc->npins;

	/* Allocate groups */
	pctl->groups = devm_kzalloc(&pdev->dev,
				    pctl->ngroups * sizeof(*pctl->groups),
				    GFP_KERNEL);
	if (!pctl->groups)
		return -ENOMEM;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;
		struct sunxi_pinctrl_group *group = pctl->groups + i;

		group->name = pin->pin.name;
		group->pin = pin->pin.number;
	}

	/*
	 * We suppose that we won't have any more functions than pins,
	 * we'll reallocate that later anyway
	 */
	pctl->functions = devm_kzalloc(&pdev->dev,
				pctl->desc->npins * sizeof(*pctl->functions),
				GFP_KERNEL);
	if (!pctl->functions)
		return -ENOMEM;

	/* Count functions and their associated groups */
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;
		struct sunxi_desc_function *func = pin->functions;

		while (func->name) {
			/* Create interrupt mapping while we're at it */
			if (!strcmp(func->name, "irq")) {
				int irqnum = func->irqnum + func->irqbank * IRQ_PER_BANK;
				pctl->irq_array[irqnum] = pin->pin.number;
			}

			sunxi_pinctrl_add_function(pctl, func->name);
			func++;
		}
	}

	pctl->functions = krealloc(pctl->functions,
				pctl->nfunctions * sizeof(*pctl->functions),
				GFP_KERNEL);

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;
		struct sunxi_desc_function *func = pin->functions;

		while (func->name) {
			struct sunxi_pinctrl_function *func_item;
			const char **func_grp;

			func_item = sunxi_pinctrl_find_function_by_name(pctl,
									func->name);
			if (!func_item)
				return -EINVAL;

			if (!func_item->groups) {
				func_item->groups =
					devm_kzalloc(&pdev->dev,
						     func_item->ngroups * sizeof(*func_item->groups),
						     GFP_KERNEL);
				if (!func_item->groups)
					return -ENOMEM;
			}

			func_grp = func_item->groups;
			while (*func_grp)
				func_grp++;

			*func_grp = pin->pin.name;
			func++;
		}
	}

	return 0;
}

int sunxi_pinctrl_init(struct platform_device *pdev,
		       const struct sunxi_pinctrl_desc *desc)
{
	struct device_node *node = pdev->dev.of_node;
	struct pinctrl_desc *pctrl_desc;
	struct pinctrl_pin_desc *pins;
	struct sunxi_pinctrl *pctl;
	struct resource *res;
	int i, ret, last_pin;
	struct clk *clk;

	pctl = devm_kzalloc(&pdev->dev, sizeof(*pctl), GFP_KERNEL);
	if (!pctl)
		return -ENOMEM;
	platform_set_drvdata(pdev, pctl);

	spin_lock_init(&pctl->lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pctl->membase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pctl->membase))
		return PTR_ERR(pctl->membase);

	pctl->dev = &pdev->dev;
	pctl->desc = desc;

	pctl->irq_array = devm_kcalloc(&pdev->dev,
				       IRQ_PER_BANK * pctl->desc->irq_banks,
				       sizeof(*pctl->irq_array),
				       GFP_KERNEL);
	if (!pctl->irq_array)
		return -ENOMEM;

	ret = sunxi_pinctrl_build_state(pdev);
	if (ret) {
		dev_err(&pdev->dev, "dt probe failed: %d\n", ret);
		return ret;
	}

	pins = devm_kzalloc(&pdev->dev,
			    pctl->desc->npins * sizeof(*pins),
			    GFP_KERNEL);
	if (!pins)
		return -ENOMEM;

	for (i = 0; i < pctl->desc->npins; i++)
		pins[i] = pctl->desc->pins[i].pin;

	pctrl_desc = devm_kzalloc(&pdev->dev,
				  sizeof(*pctrl_desc),
				  GFP_KERNEL);
	if (!pctrl_desc)
		return -ENOMEM;

	pctrl_desc->name = dev_name(&pdev->dev);
	pctrl_desc->owner = THIS_MODULE;
	pctrl_desc->pins = pins;
	pctrl_desc->npins = pctl->desc->npins;
	pctrl_desc->confops = &sunxi_pconf_ops;
	pctrl_desc->pctlops = &sunxi_pctrl_ops;
	pctrl_desc->pmxops =  &sunxi_pmx_ops;

	pctl->pctl_dev = pinctrl_register(pctrl_desc,
					  &pdev->dev, pctl);
	if (!pctl->pctl_dev) {
		dev_err(&pdev->dev, "couldn't register pinctrl driver\n");
		return -EINVAL;
	}

	pctl->chip = devm_kzalloc(&pdev->dev, sizeof(*pctl->chip), GFP_KERNEL);
	if (!pctl->chip) {
		ret = -ENOMEM;
		goto pinctrl_error;
	}

	last_pin = pctl->desc->pins[pctl->desc->npins - 1].pin.number;
	pctl->chip->owner = THIS_MODULE;
	pctl->chip->request = sunxi_pinctrl_gpio_request,
	pctl->chip->free = sunxi_pinctrl_gpio_free,
	pctl->chip->direction_input = sunxi_pinctrl_gpio_direction_input,
	pctl->chip->direction_output = sunxi_pinctrl_gpio_direction_output,
	pctl->chip->get = sunxi_pinctrl_gpio_get,
	pctl->chip->set = sunxi_pinctrl_gpio_set,
	pctl->chip->of_xlate = sunxi_pinctrl_gpio_of_xlate,
	pctl->chip->to_irq = sunxi_pinctrl_gpio_to_irq,
	pctl->chip->of_gpio_n_cells = 3,
	pctl->chip->can_sleep = false,
	pctl->chip->ngpio = round_up(last_pin, PINS_PER_BANK) -
			    pctl->desc->pin_base;
	pctl->chip->label = dev_name(&pdev->dev);
	pctl->chip->dev = &pdev->dev;
	pctl->chip->base = pctl->desc->pin_base;

	ret = gpiochip_add(pctl->chip);
	if (ret)
		goto pinctrl_error;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct sunxi_desc_pin *pin = pctl->desc->pins + i;

		ret = gpiochip_add_pin_range(pctl->chip, dev_name(&pdev->dev),
					     pin->pin.number - pctl->desc->pin_base,
					     pin->pin.number, 1);
		if (ret)
			goto gpiochip_error;
	}

	clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		ret = PTR_ERR(clk);
		goto gpiochip_error;
	}

	ret = clk_prepare_enable(clk);
	if (ret)
		goto gpiochip_error;

	pctl->irq = devm_kcalloc(&pdev->dev,
				 pctl->desc->irq_banks,
				 sizeof(*pctl->irq),
				 GFP_KERNEL);
	if (!pctl->irq) {
		ret = -ENOMEM;
		goto clk_error;
	}

	for (i = 0; i < pctl->desc->irq_banks; i++) {
		pctl->irq[i] = platform_get_irq(pdev, i);
		if (pctl->irq[i] < 0) {
			ret = pctl->irq[i];
			goto clk_error;
		}
	}

	pctl->domain = irq_domain_add_linear(node,
					     pctl->desc->irq_banks * IRQ_PER_BANK,
					     &irq_domain_simple_ops,
					     NULL);
	if (!pctl->domain) {
		dev_err(&pdev->dev, "Couldn't register IRQ domain\n");
		ret = -ENOMEM;
		goto clk_error;
	}

	for (i = 0; i < (pctl->desc->irq_banks * IRQ_PER_BANK); i++) {
		int irqno = irq_create_mapping(pctl->domain, i);

		irq_set_chip_and_handler(irqno, &sunxi_pinctrl_edge_irq_chip,
					 handle_edge_irq);
		irq_set_chip_data(irqno, pctl);
	};

	for (i = 0; i < pctl->desc->irq_banks; i++) {
		/* Mask and clear all IRQs before registering a handler */
		writel(0, pctl->membase + sunxi_irq_ctrl_reg_from_bank(i));
		writel(0xffffffff,
			pctl->membase + sunxi_irq_status_reg_from_bank(i));

		irq_set_chained_handler(pctl->irq[i],
					sunxi_pinctrl_irq_handler);
		irq_set_handler_data(pctl->irq[i], pctl);
	}

	dev_info(&pdev->dev, "initialized sunXi PIO driver\n");

	return 0;

clk_error:
	clk_disable_unprepare(clk);
gpiochip_error:
	gpiochip_remove(pctl->chip);
pinctrl_error:
	pinctrl_unregister(pctl->pctl_dev);
	return ret;
}
