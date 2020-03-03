/*
 * Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <dt-bindings/pinctrl/qcom,pmic-gpio.h>

#include "../core.h"
#include "../pinctrl-utils.h"

#define PMIC_GPIO_ADDRESS_RANGE			0x100

/* type and subtype registers base address offsets */
#define PMIC_GPIO_REG_TYPE			0x4
#define PMIC_GPIO_REG_SUBTYPE			0x5

/* GPIO peripheral type and subtype out_values */
#define PMIC_GPIO_TYPE				0x10
#define PMIC_GPIO_SUBTYPE_GPIO_4CH		0x1
#define PMIC_GPIO_SUBTYPE_GPIOC_4CH		0x5
#define PMIC_GPIO_SUBTYPE_GPIO_8CH		0x9
#define PMIC_GPIO_SUBTYPE_GPIOC_8CH		0xd

#define PMIC_MPP_REG_RT_STS			0x10
#define PMIC_MPP_REG_RT_STS_VAL_MASK		0x1

/* control register base address offsets */
#define PMIC_GPIO_REG_MODE_CTL			0x40
#define PMIC_GPIO_REG_DIG_VIN_CTL		0x41
#define PMIC_GPIO_REG_DIG_PULL_CTL		0x42
#define PMIC_GPIO_REG_DIG_OUT_CTL		0x45
#define PMIC_GPIO_REG_EN_CTL			0x46

/* PMIC_GPIO_REG_MODE_CTL */
#define PMIC_GPIO_REG_MODE_VALUE_SHIFT		0x1
#define PMIC_GPIO_REG_MODE_FUNCTION_SHIFT	1
#define PMIC_GPIO_REG_MODE_FUNCTION_MASK	0x7
#define PMIC_GPIO_REG_MODE_DIR_SHIFT		4
#define PMIC_GPIO_REG_MODE_DIR_MASK		0x7

/* PMIC_GPIO_REG_DIG_VIN_CTL */
#define PMIC_GPIO_REG_VIN_SHIFT			0
#define PMIC_GPIO_REG_VIN_MASK			0x7

/* PMIC_GPIO_REG_DIG_PULL_CTL */
#define PMIC_GPIO_REG_PULL_SHIFT		0
#define PMIC_GPIO_REG_PULL_MASK			0x7

#define PMIC_GPIO_PULL_DOWN			4
#define PMIC_GPIO_PULL_DISABLE			5

/* PMIC_GPIO_REG_DIG_OUT_CTL */
#define PMIC_GPIO_REG_OUT_STRENGTH_SHIFT	0
#define PMIC_GPIO_REG_OUT_STRENGTH_MASK		0x3
#define PMIC_GPIO_REG_OUT_TYPE_SHIFT		4
#define PMIC_GPIO_REG_OUT_TYPE_MASK		0x3

/*
 * Output type - indicates pin should be configured as push-pull,
 * open drain or open source.
 */
#define PMIC_GPIO_OUT_BUF_CMOS			0
#define PMIC_GPIO_OUT_BUF_OPEN_DRAIN_NMOS	1
#define PMIC_GPIO_OUT_BUF_OPEN_DRAIN_PMOS	2

/* PMIC_GPIO_REG_EN_CTL */
#define PMIC_GPIO_REG_MASTER_EN_SHIFT		7

#define PMIC_GPIO_PHYSICAL_OFFSET		1

/* Qualcomm specific pin configurations */
#define PMIC_GPIO_CONF_PULL_UP			(PIN_CONFIG_END + 1)
#define PMIC_GPIO_CONF_STRENGTH			(PIN_CONFIG_END + 2)

/**
 * struct pmic_gpio_pad - keep current GPIO settings
 * @base: Address base in SPMI device.
 * @irq: IRQ number which this GPIO generate.
 * @is_enabled: Set to false when GPIO should be put in high Z state.
 * @out_value: Cached pin output value
 * @have_buffer: Set to true if GPIO output could be configured in push-pull,
 *	open-drain or open-source mode.
 * @output_enabled: Set to true if GPIO output logic is enabled.
 * @input_enabled: Set to true if GPIO input buffer logic is enabled.
 * @num_sources: Number of power-sources supported by this GPIO.
 * @power_source: Current power-source used.
 * @buffer_type: Push-pull, open-drain or open-source.
 * @pullup: Constant current which flow trough GPIO output buffer.
 * @strength: No, Low, Medium, High
 * @function: See pmic_gpio_functions[]
 */
struct pmic_gpio_pad {
	u16		base;
	int		irq;
	bool		is_enabled;
	bool		out_value;
	bool		have_buffer;
	bool		output_enabled;
	bool		input_enabled;
	unsigned int	num_sources;
	unsigned int	power_source;
	unsigned int	buffer_type;
	unsigned int	pullup;
	unsigned int	strength;
	unsigned int	function;
};

struct pmic_gpio_state {
	struct device	*dev;
	struct regmap	*map;
	struct pinctrl_dev *ctrl;
	struct gpio_chip chip;
};

static const struct pinconf_generic_params pmic_gpio_bindings[] = {
	{"qcom,pull-up-strength",	PMIC_GPIO_CONF_PULL_UP,		0},
	{"qcom,drive-strength",		PMIC_GPIO_CONF_STRENGTH,	0},
};

#ifdef CONFIG_DEBUG_FS
static const struct pin_config_item pmic_conf_items[ARRAY_SIZE(pmic_gpio_bindings)] = {
	PCONFDUMP(PMIC_GPIO_CONF_PULL_UP,  "pull up strength", NULL, true),
	PCONFDUMP(PMIC_GPIO_CONF_STRENGTH, "drive-strength", NULL, true),
};
#endif

static const char *const pmic_gpio_groups[] = {
	"gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7", "gpio8",
	"gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14", "gpio15",
	"gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21", "gpio22",
	"gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28", "gpio29",
	"gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35", "gpio36",
};

static const char *const pmic_gpio_functions[] = {
	PMIC_GPIO_FUNC_NORMAL, PMIC_GPIO_FUNC_PAIRED,
	PMIC_GPIO_FUNC_FUNC1, PMIC_GPIO_FUNC_FUNC2,
	PMIC_GPIO_FUNC_DTEST1, PMIC_GPIO_FUNC_DTEST2,
	PMIC_GPIO_FUNC_DTEST3, PMIC_GPIO_FUNC_DTEST4,
};

static inline struct pmic_gpio_state *to_gpio_state(struct gpio_chip *chip)
{
	return container_of(chip, struct pmic_gpio_state, chip);
};

static int pmic_gpio_read(struct pmic_gpio_state *state,
			  struct pmic_gpio_pad *pad, unsigned int addr)
{
	unsigned int val;
	int ret;

	ret = regmap_read(state->map, pad->base + addr, &val);
	if (ret < 0)
		dev_err(state->dev, "read 0x%x failed\n", addr);
	else
		ret = val;

	return ret;
}

static int pmic_gpio_write(struct pmic_gpio_state *state,
			   struct pmic_gpio_pad *pad, unsigned int addr,
			   unsigned int val)
{
	int ret;

	ret = regmap_write(state->map, pad->base + addr, val);
	if (ret < 0)
		dev_err(state->dev, "write 0x%x failed\n", addr);

	return ret;
}

static int pmic_gpio_get_groups_count(struct pinctrl_dev *pctldev)
{
	/* Every PIN is a group */
	return pctldev->desc->npins;
}

static const char *pmic_gpio_get_group_name(struct pinctrl_dev *pctldev,
					    unsigned pin)
{
	return pctldev->desc->pins[pin].name;
}

static int pmic_gpio_get_group_pins(struct pinctrl_dev *pctldev, unsigned pin,
				    const unsigned **pins, unsigned *num_pins)
{
	*pins = &pctldev->desc->pins[pin].number;
	*num_pins = 1;
	return 0;
}

static const struct pinctrl_ops pmic_gpio_pinctrl_ops = {
	.get_groups_count	= pmic_gpio_get_groups_count,
	.get_group_name		= pmic_gpio_get_group_name,
	.get_group_pins		= pmic_gpio_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_group,
	.dt_free_map		= pinctrl_utils_dt_free_map,
};

static int pmic_gpio_get_functions_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(pmic_gpio_functions);
}

static const char *pmic_gpio_get_function_name(struct pinctrl_dev *pctldev,
					       unsigned function)
{
	return pmic_gpio_functions[function];
}

static int pmic_gpio_get_function_groups(struct pinctrl_dev *pctldev,
					 unsigned function,
					 const char *const **groups,
					 unsigned *const num_qgroups)
{
	*groups = pmic_gpio_groups;
	*num_qgroups = pctldev->desc->npins;
	return 0;
}

static int pmic_gpio_set_mux(struct pinctrl_dev *pctldev, unsigned function,
				unsigned pin)
{
	struct pmic_gpio_state *state = pinctrl_dev_get_drvdata(pctldev);
	struct pmic_gpio_pad *pad;
	unsigned int val;
	int ret;

	pad = pctldev->desc->pins[pin].drv_data;

	pad->function = function;

	val = 0;
	if (pad->output_enabled) {
		if (pad->input_enabled)
			val = 2;
		else
			val = 1;
	}

	val = val << PMIC_GPIO_REG_MODE_DIR_SHIFT;
	val |= pad->function << PMIC_GPIO_REG_MODE_FUNCTION_SHIFT;
	val |= pad->out_value & PMIC_GPIO_REG_MODE_VALUE_SHIFT;

	ret = pmic_gpio_write(state, pad, PMIC_GPIO_REG_MODE_CTL, val);
	if (ret < 0)
		return ret;

	val = pad->is_enabled << PMIC_GPIO_REG_MASTER_EN_SHIFT;

	return pmic_gpio_write(state, pad, PMIC_GPIO_REG_EN_CTL, val);
}

static const struct pinmux_ops pmic_gpio_pinmux_ops = {
	.get_functions_count	= pmic_gpio_get_functions_count,
	.get_function_name	= pmic_gpio_get_function_name,
	.get_function_groups	= pmic_gpio_get_function_groups,
	.set_mux		= pmic_gpio_set_mux,
};

static int pmic_gpio_config_get(struct pinctrl_dev *pctldev,
				unsigned int pin, unsigned long *config)
{
	unsigned param = pinconf_to_config_param(*config);
	struct pmic_gpio_pad *pad;
	unsigned arg;

	pad = pctldev->desc->pins[pin].drv_data;

	switch (param) {
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		arg = pad->buffer_type == PMIC_GPIO_OUT_BUF_CMOS;
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		arg = pad->buffer_type == PMIC_GPIO_OUT_BUF_OPEN_DRAIN_NMOS;
		break;
	case PIN_CONFIG_DRIVE_OPEN_SOURCE:
		arg = pad->buffer_type == PMIC_GPIO_OUT_BUF_OPEN_DRAIN_PMOS;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		arg = pad->pullup == PMIC_GPIO_PULL_DOWN;
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		arg = pad->pullup = PMIC_GPIO_PULL_DISABLE;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		arg = pad->pullup == PMIC_GPIO_PULL_UP_30;
		break;
	case PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
		arg = !pad->is_enabled;
		break;
	case PIN_CONFIG_POWER_SOURCE:
		arg = pad->power_source;
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		arg = pad->input_enabled;
		break;
	case PIN_CONFIG_OUTPUT:
		arg = pad->out_value;
		break;
	case PMIC_GPIO_CONF_PULL_UP:
		arg = pad->pullup;
		break;
	case PMIC_GPIO_CONF_STRENGTH:
		arg = pad->strength;
		break;
	default:
		return -EINVAL;
	}

	*config = pinconf_to_config_packed(param, arg);
	return 0;
}

static int pmic_gpio_config_set(struct pinctrl_dev *pctldev, unsigned int pin,
				unsigned long *configs, unsigned nconfs)
{
	struct pmic_gpio_state *state = pinctrl_dev_get_drvdata(pctldev);
	struct pmic_gpio_pad *pad;
	unsigned param, arg;
	unsigned int val;
	int i, ret;

	pad = pctldev->desc->pins[pin].drv_data;

	for (i = 0; i < nconfs; i++) {
		param = pinconf_to_config_param(configs[i]);
		arg = pinconf_to_config_argument(configs[i]);

		switch (param) {
		case PIN_CONFIG_DRIVE_PUSH_PULL:
			pad->buffer_type = PMIC_GPIO_OUT_BUF_CMOS;
			break;
		case PIN_CONFIG_DRIVE_OPEN_DRAIN:
			if (!pad->have_buffer)
				return -EINVAL;
			pad->buffer_type = PMIC_GPIO_OUT_BUF_OPEN_DRAIN_NMOS;
			break;
		case PIN_CONFIG_DRIVE_OPEN_SOURCE:
			if (!pad->have_buffer)
				return -EINVAL;
			pad->buffer_type = PMIC_GPIO_OUT_BUF_OPEN_DRAIN_PMOS;
			break;
		case PIN_CONFIG_BIAS_DISABLE:
			pad->pullup = PMIC_GPIO_PULL_DISABLE;
			break;
		case PIN_CONFIG_BIAS_PULL_UP:
			pad->pullup = PMIC_GPIO_PULL_UP_30;
			break;
		case PIN_CONFIG_BIAS_PULL_DOWN:
			if (arg)
				pad->pullup = PMIC_GPIO_PULL_DOWN;
			else
				pad->pullup = PMIC_GPIO_PULL_DISABLE;
			break;
		case PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
			pad->is_enabled = false;
			break;
		case PIN_CONFIG_POWER_SOURCE:
			if (arg > pad->num_sources)
				return -EINVAL;
			pad->power_source = arg;
			break;
		case PIN_CONFIG_INPUT_ENABLE:
			pad->input_enabled = arg ? true : false;
			break;
		case PIN_CONFIG_OUTPUT:
			pad->output_enabled = true;
			pad->out_value = arg;
			break;
		case PMIC_GPIO_CONF_PULL_UP:
			if (arg > PMIC_GPIO_PULL_UP_1P5_30)
				return -EINVAL;
			pad->pullup = arg;
			break;
		case PMIC_GPIO_CONF_STRENGTH:
			if (arg > PMIC_GPIO_STRENGTH_LOW)
				return -EINVAL;
			pad->strength = arg;
			break;
		default:
			return -EINVAL;
		}
	}

	val = pad->power_source << PMIC_GPIO_REG_VIN_SHIFT;

	ret = pmic_gpio_write(state, pad, PMIC_GPIO_REG_DIG_VIN_CTL, val);
	if (ret < 0)
		return ret;

	val = pad->pullup << PMIC_GPIO_REG_PULL_SHIFT;

	ret = pmic_gpio_write(state, pad, PMIC_GPIO_REG_DIG_PULL_CTL, val);
	if (ret < 0)
		return ret;

	val = pad->buffer_type << PMIC_GPIO_REG_OUT_TYPE_SHIFT;
	val |= pad->strength << PMIC_GPIO_REG_OUT_STRENGTH_SHIFT;

	ret = pmic_gpio_write(state, pad, PMIC_GPIO_REG_DIG_OUT_CTL, val);
	if (ret < 0)
		return ret;

	val = 0;
	if (pad->output_enabled) {
		if (pad->input_enabled)
			val = 2;
		else
			val = 1;
	}

	val = val << PMIC_GPIO_REG_MODE_DIR_SHIFT;
	val |= pad->function << PMIC_GPIO_REG_MODE_FUNCTION_SHIFT;
	val |= pad->out_value & PMIC_GPIO_REG_MODE_VALUE_SHIFT;

	return pmic_gpio_write(state, pad, PMIC_GPIO_REG_MODE_CTL, val);
}

static void pmic_gpio_config_dbg_show(struct pinctrl_dev *pctldev,
				      struct seq_file *s, unsigned pin)
{
	struct pmic_gpio_state *state = pinctrl_dev_get_drvdata(pctldev);
	struct pmic_gpio_pad *pad;
	int ret, val;

	static const char *const biases[] = {
		"pull-up 30uA", "pull-up 1.5uA", "pull-up 31.5uA",
		"pull-up 1.5uA + 30uA boost", "pull-down 10uA", "no pull"
	};
	static const char *const buffer_types[] = {
		"push-pull", "open-drain", "open-source"
	};
	static const char *const strengths[] = {
		"no", "high", "medium", "low"
	};

	pad = pctldev->desc->pins[pin].drv_data;

	seq_printf(s, " gpio%-2d:", pin + PMIC_GPIO_PHYSICAL_OFFSET);

	val = pmic_gpio_read(state, pad, PMIC_GPIO_REG_EN_CTL);

	if (val < 0 || !(val >> PMIC_GPIO_REG_MASTER_EN_SHIFT)) {
		seq_puts(s, " ---");
	} else {

		if (pad->input_enabled) {
			ret = pmic_gpio_read(state, pad, PMIC_MPP_REG_RT_STS);
			if (ret < 0)
				return;

			ret &= PMIC_MPP_REG_RT_STS_VAL_MASK;
			pad->out_value = ret;
		}

		seq_printf(s, " %-4s", pad->output_enabled ? "out" : "in");
		seq_printf(s, " %-7s", pmic_gpio_functions[pad->function]);
		seq_printf(s, " vin-%d", pad->power_source);
		seq_printf(s, " %-27s", biases[pad->pullup]);
		seq_printf(s, " %-10s", buffer_types[pad->buffer_type]);
		seq_printf(s, " %-4s", pad->out_value ? "high" : "low");
		seq_printf(s, " %-7s", strengths[pad->strength]);
	}
}

static const struct pinconf_ops pmic_gpio_pinconf_ops = {
	.is_generic			= true,
	.pin_config_group_get		= pmic_gpio_config_get,
	.pin_config_group_set		= pmic_gpio_config_set,
	.pin_config_group_dbg_show	= pmic_gpio_config_dbg_show,
};

static int pmic_gpio_direction_input(struct gpio_chip *chip, unsigned pin)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	unsigned long config;

	config = pinconf_to_config_packed(PIN_CONFIG_INPUT_ENABLE, 1);

	return pmic_gpio_config_set(state->ctrl, pin, &config, 1);
}

static int pmic_gpio_direction_output(struct gpio_chip *chip,
				      unsigned pin, int val)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	unsigned long config;

	config = pinconf_to_config_packed(PIN_CONFIG_OUTPUT, val);

	return pmic_gpio_config_set(state->ctrl, pin, &config, 1);
}

static int pmic_gpio_get(struct gpio_chip *chip, unsigned pin)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	struct pmic_gpio_pad *pad;
	int ret;

	pad = state->ctrl->desc->pins[pin].drv_data;

	if (!pad->is_enabled)
		return -EINVAL;

	if (pad->input_enabled) {
		ret = pmic_gpio_read(state, pad, PMIC_MPP_REG_RT_STS);
		if (ret < 0)
			return ret;

		pad->out_value = ret & PMIC_MPP_REG_RT_STS_VAL_MASK;
	}

	return pad->out_value;
}

static void pmic_gpio_set(struct gpio_chip *chip, unsigned pin, int value)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	unsigned long config;

	config = pinconf_to_config_packed(PIN_CONFIG_OUTPUT, value);

	pmic_gpio_config_set(state->ctrl, pin, &config, 1);
}

static int pmic_gpio_request(struct gpio_chip *chip, unsigned base)
{
	return pinctrl_request_gpio(chip->base + base);
}

static void pmic_gpio_free(struct gpio_chip *chip, unsigned base)
{
	pinctrl_free_gpio(chip->base + base);
}

static int pmic_gpio_of_xlate(struct gpio_chip *chip,
			      const struct of_phandle_args *gpio_desc,
			      u32 *flags)
{
	if (chip->of_gpio_n_cells < 2)
		return -EINVAL;

	if (flags)
		*flags = gpio_desc->args[1];

	return gpio_desc->args[0] - PMIC_GPIO_PHYSICAL_OFFSET;
}

static int pmic_gpio_to_irq(struct gpio_chip *chip, unsigned pin)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	struct pmic_gpio_pad *pad;

	pad = state->ctrl->desc->pins[pin].drv_data;

	return pad->irq;
}

static void pmic_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip)
{
	struct pmic_gpio_state *state = to_gpio_state(chip);
	unsigned i;

	for (i = 0; i < chip->ngpio; i++) {
		pmic_gpio_config_dbg_show(state->ctrl, s, i);
		seq_puts(s, "\n");
	}
}

static const struct gpio_chip pmic_gpio_gpio_template = {
	.direction_input	= pmic_gpio_direction_input,
	.direction_output	= pmic_gpio_direction_output,
	.get			= pmic_gpio_get,
	.set			= pmic_gpio_set,
	.request		= pmic_gpio_request,
	.free			= pmic_gpio_free,
	.of_xlate		= pmic_gpio_of_xlate,
	.to_irq			= pmic_gpio_to_irq,
	.dbg_show		= pmic_gpio_dbg_show,
};

static int pmic_gpio_populate(struct pmic_gpio_state *state,
			      struct pmic_gpio_pad *pad)
{
	int type, subtype, val, dir;

	type = pmic_gpio_read(state, pad, PMIC_GPIO_REG_TYPE);
	if (type < 0)
		return type;

	if (type != PMIC_GPIO_TYPE) {
		dev_err(state->dev, "incorrect block type 0x%x at 0x%x\n",
			type, pad->base);
		return -ENODEV;
	}

	subtype = pmic_gpio_read(state, pad, PMIC_GPIO_REG_SUBTYPE);
	if (subtype < 0)
		return subtype;

	switch (subtype) {
	case PMIC_GPIO_SUBTYPE_GPIO_4CH:
		pad->have_buffer = true;
	case PMIC_GPIO_SUBTYPE_GPIOC_4CH:
		pad->num_sources = 4;
		break;
	case PMIC_GPIO_SUBTYPE_GPIO_8CH:
		pad->have_buffer = true;
	case PMIC_GPIO_SUBTYPE_GPIOC_8CH:
		pad->num_sources = 8;
		break;
	default:
		dev_err(state->dev, "unknown GPIO type 0x%x\n", subtype);
		return -ENODEV;
	}

	val = pmic_gpio_read(state, pad, PMIC_GPIO_REG_MODE_CTL);
	if (val < 0)
		return val;

	pad->out_value = val & PMIC_GPIO_REG_MODE_VALUE_SHIFT;

	dir = val >> PMIC_GPIO_REG_MODE_DIR_SHIFT;
	dir &= PMIC_GPIO_REG_MODE_DIR_MASK;
	switch (dir) {
	case 0:
		pad->input_enabled = true;
		pad->output_enabled = false;
		break;
	case 1:
		pad->input_enabled = false;
		pad->output_enabled = true;
		break;
	case 2:
		pad->input_enabled = true;
		pad->output_enabled = true;
		break;
	default:
		dev_err(state->dev, "unknown GPIO direction\n");
		return -ENODEV;
	}

	pad->function = val >> PMIC_GPIO_REG_MODE_FUNCTION_SHIFT;
	pad->function &= PMIC_GPIO_REG_MODE_FUNCTION_MASK;

	val = pmic_gpio_read(state, pad, PMIC_GPIO_REG_DIG_VIN_CTL);
	if (val < 0)
		return val;

	pad->power_source = val >> PMIC_GPIO_REG_VIN_SHIFT;
	pad->power_source &= PMIC_GPIO_REG_VIN_MASK;

	val = pmic_gpio_read(state, pad, PMIC_GPIO_REG_DIG_PULL_CTL);
	if (val < 0)
		return val;

	pad->pullup = val >> PMIC_GPIO_REG_PULL_SHIFT;
	pad->pullup &= PMIC_GPIO_REG_PULL_MASK;

	val = pmic_gpio_read(state, pad, PMIC_GPIO_REG_DIG_OUT_CTL);
	if (val < 0)
		return val;

	pad->strength = val >> PMIC_GPIO_REG_OUT_STRENGTH_SHIFT;
	pad->strength &= PMIC_GPIO_REG_OUT_STRENGTH_MASK;

	pad->buffer_type = val >> PMIC_GPIO_REG_OUT_TYPE_SHIFT;
	pad->buffer_type &= PMIC_GPIO_REG_OUT_TYPE_MASK;

	/* Pin could be disabled with PIN_CONFIG_BIAS_HIGH_IMPEDANCE */
	pad->is_enabled = true;
	return 0;
}

static int pmic_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pinctrl_pin_desc *pindesc;
	struct pinctrl_desc *pctrldesc;
	struct pmic_gpio_pad *pad, *pads;
	struct pmic_gpio_state *state;
	int ret, npins, i;
	u32 res[2];

	ret = of_property_read_u32_array(dev->of_node, "reg", res, 2);
	if (ret < 0) {
		dev_err(dev, "missing base address and/or range");
		return ret;
	}

	npins = res[1] / PMIC_GPIO_ADDRESS_RANGE;

	if (!npins)
		return -EINVAL;

	BUG_ON(npins > ARRAY_SIZE(pmic_gpio_groups));

	state = devm_kzalloc(dev, sizeof(*state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	platform_set_drvdata(pdev, state);

	state->dev = &pdev->dev;
	state->map = dev_get_regmap(dev->parent, NULL);

	pindesc = devm_kcalloc(dev, npins, sizeof(*pindesc), GFP_KERNEL);
	if (!pindesc)
		return -ENOMEM;

	pads = devm_kcalloc(dev, npins, sizeof(*pads), GFP_KERNEL);
	if (!pads)
		return -ENOMEM;

	pctrldesc = devm_kzalloc(dev, sizeof(*pctrldesc), GFP_KERNEL);
	if (!pctrldesc)
		return -ENOMEM;

	pctrldesc->pctlops = &pmic_gpio_pinctrl_ops;
	pctrldesc->pmxops = &pmic_gpio_pinmux_ops;
	pctrldesc->confops = &pmic_gpio_pinconf_ops;
	pctrldesc->owner = THIS_MODULE;
	pctrldesc->name = dev_name(dev);
	pctrldesc->pins = pindesc;
	pctrldesc->npins = npins;
	pctrldesc->num_custom_params = ARRAY_SIZE(pmic_gpio_bindings);
	pctrldesc->custom_params = pmic_gpio_bindings;
#ifdef CONFIG_DEBUG_FS
	pctrldesc->custom_conf_items = pmic_conf_items;
#endif

	for (i = 0; i < npins; i++, pindesc++) {
		pad = &pads[i];
		pindesc->drv_data = pad;
		pindesc->number = i;
		pindesc->name = pmic_gpio_groups[i];

		pad->irq = platform_get_irq(pdev, i);
		if (pad->irq < 0)
			return pad->irq;

		pad->base = res[0] + i * PMIC_GPIO_ADDRESS_RANGE;

		ret = pmic_gpio_populate(state, pad);
		if (ret < 0)
			return ret;
	}

	state->chip = pmic_gpio_gpio_template;
	state->chip.dev = dev;
	state->chip.base = -1;
	state->chip.ngpio = npins;
	state->chip.label = dev_name(dev);
	state->chip.of_gpio_n_cells = 2;
	state->chip.can_sleep = false;

	state->ctrl = pinctrl_register(pctrldesc, dev, state);
	if (!state->ctrl)
		return -ENODEV;

	ret = gpiochip_add(&state->chip);
	if (ret) {
		dev_err(state->dev, "can't add gpio chip\n");
		goto err_chip;
	}

	ret = gpiochip_add_pin_range(&state->chip, dev_name(dev), 0, 0, npins);
	if (ret) {
		dev_err(dev, "failed to add pin range\n");
		goto err_range;
	}

	return 0;

err_range:
	gpiochip_remove(&state->chip);
err_chip:
	pinctrl_unregister(state->ctrl);
	return ret;
}

static int pmic_gpio_remove(struct platform_device *pdev)
{
	struct pmic_gpio_state *state = platform_get_drvdata(pdev);

	gpiochip_remove(&state->chip);
	pinctrl_unregister(state->ctrl);
	return 0;
}

static const struct of_device_id pmic_gpio_of_match[] = {
	{ .compatible = "qcom,pm8916-gpio" },	/* 4 GPIO's */
	{ .compatible = "qcom,pm8941-gpio" },	/* 36 GPIO's */
	{ .compatible = "qcom,pma8084-gpio" },	/* 22 GPIO's */
	{ },
};

MODULE_DEVICE_TABLE(of, pmic_gpio_of_match);

static struct platform_driver pmic_gpio_driver = {
	.driver = {
		   .name = "qcom-spmi-gpio",
		   .of_match_table = pmic_gpio_of_match,
	},
	.probe	= pmic_gpio_probe,
	.remove = pmic_gpio_remove,
};

module_platform_driver(pmic_gpio_driver);

MODULE_AUTHOR("Ivan T. Ivanov <iivanov@mm-sol.com>");
MODULE_DESCRIPTION("Qualcomm SPMI PMIC GPIO pin control driver");
MODULE_ALIAS("platform:qcom-spmi-gpio");
MODULE_LICENSE("GPL v2");
