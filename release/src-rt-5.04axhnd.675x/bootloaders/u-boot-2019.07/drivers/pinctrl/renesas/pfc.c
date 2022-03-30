// SPDX-License-Identifier: GPL-2.0
/*
 * Pin Control driver for SuperH Pin Function Controller.
 *
 * Authors: Magnus Damm, Paul Mundt, Laurent Pinchart
 *
 * Copyright (C) 2008 Magnus Damm
 * Copyright (C) 2009 - 2012 Paul Mundt
 * Copyright (C) 2017 Marek Vasut
 */

#define DRV_NAME "sh-pfc"

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "sh_pfc.h"

enum sh_pfc_model {
	SH_PFC_R8A7790 = 0,
	SH_PFC_R8A7791,
	SH_PFC_R8A7792,
	SH_PFC_R8A7793,
	SH_PFC_R8A7794,
	SH_PFC_R8A7795,
	SH_PFC_R8A7796,
	SH_PFC_R8A77965,
	SH_PFC_R8A77970,
	SH_PFC_R8A77990,
	SH_PFC_R8A77995,
};

struct sh_pfc_pin_config {
	u32 type;
};

struct sh_pfc_pinctrl {
	struct sh_pfc *pfc;

	struct sh_pfc_pin_config *configs;

	const char *func_prop_name;
	const char *groups_prop_name;
	const char *pins_prop_name;
};

struct sh_pfc_pin_range {
	u16 start;
	u16 end;
};

struct sh_pfc_pinctrl_priv {
	struct sh_pfc			pfc;
	struct sh_pfc_pinctrl		pmx;
};

int sh_pfc_get_pin_index(struct sh_pfc *pfc, unsigned int pin)
{
	unsigned int offset;
	unsigned int i;

	for (i = 0, offset = 0; i < pfc->nr_ranges; ++i) {
		const struct sh_pfc_pin_range *range = &pfc->ranges[i];

		if (pin <= range->end)
			return pin >= range->start
			     ? offset + pin - range->start : -1;

		offset += range->end - range->start + 1;
	}

	return -EINVAL;
}

static int sh_pfc_enum_in_range(u16 enum_id, const struct pinmux_range *r)
{
	if (enum_id < r->begin)
		return 0;

	if (enum_id > r->end)
		return 0;

	return 1;
}

u32 sh_pfc_read_raw_reg(void __iomem *mapped_reg, unsigned int reg_width)
{
	switch (reg_width) {
	case 8:
		return readb(mapped_reg);
	case 16:
		return readw(mapped_reg);
	case 32:
		return readl(mapped_reg);
	}

	BUG();
	return 0;
}

void sh_pfc_write_raw_reg(void __iomem *mapped_reg, unsigned int reg_width,
			  u32 data)
{
	switch (reg_width) {
	case 8:
		writeb(data, mapped_reg);
		return;
	case 16:
		writew(data, mapped_reg);
		return;
	case 32:
		writel(data, mapped_reg);
		return;
	}

	BUG();
}

u32 sh_pfc_read(struct sh_pfc *pfc, u32 reg)
{
	return sh_pfc_read_raw_reg((void __iomem *)(uintptr_t)reg, 32);
}

void sh_pfc_write(struct sh_pfc *pfc, u32 reg, u32 data)
{
	void __iomem *unlock_reg =
		(void __iomem *)(uintptr_t)pfc->info->unlock_reg;

	if (pfc->info->unlock_reg)
		sh_pfc_write_raw_reg(unlock_reg, 32, ~data);

	sh_pfc_write_raw_reg((void __iomem *)(uintptr_t)reg, 32, data);
}

static void sh_pfc_config_reg_helper(struct sh_pfc *pfc,
				     const struct pinmux_cfg_reg *crp,
				     unsigned int in_pos,
				     void __iomem **mapped_regp, u32 *maskp,
				     unsigned int *posp)
{
	unsigned int k;

	*mapped_regp = (void __iomem *)(uintptr_t)crp->reg;

	if (crp->field_width) {
		*maskp = (1 << crp->field_width) - 1;
		*posp = crp->reg_width - ((in_pos + 1) * crp->field_width);
	} else {
		*maskp = (1 << crp->var_field_width[in_pos]) - 1;
		*posp = crp->reg_width;
		for (k = 0; k <= in_pos; k++)
			*posp -= crp->var_field_width[k];
	}
}

static void sh_pfc_write_config_reg(struct sh_pfc *pfc,
				    const struct pinmux_cfg_reg *crp,
				    unsigned int field, u32 value)
{
	void __iomem *mapped_reg;
	void __iomem *unlock_reg =
		(void __iomem *)(uintptr_t)pfc->info->unlock_reg;
	unsigned int pos;
	u32 mask, data;

	sh_pfc_config_reg_helper(pfc, crp, field, &mapped_reg, &mask, &pos);

	dev_dbg(pfc->dev, "write_reg addr = %x, value = 0x%x, field = %u, "
		"r_width = %u, f_width = %u\n",
		crp->reg, value, field, crp->reg_width, crp->field_width);

	mask = ~(mask << pos);
	value = value << pos;

	data = sh_pfc_read_raw_reg(mapped_reg, crp->reg_width);
	data &= mask;
	data |= value;

	if (pfc->info->unlock_reg)
		sh_pfc_write_raw_reg(unlock_reg, 32, ~data);

	sh_pfc_write_raw_reg(mapped_reg, crp->reg_width, data);
}

static int sh_pfc_get_config_reg(struct sh_pfc *pfc, u16 enum_id,
				 const struct pinmux_cfg_reg **crp,
				 unsigned int *fieldp, u32 *valuep)
{
	unsigned int k = 0;

	while (1) {
		const struct pinmux_cfg_reg *config_reg =
			pfc->info->cfg_regs + k;
		unsigned int r_width = config_reg->reg_width;
		unsigned int f_width = config_reg->field_width;
		unsigned int curr_width;
		unsigned int bit_pos;
		unsigned int pos = 0;
		unsigned int m = 0;

		if (!r_width)
			break;

		for (bit_pos = 0; bit_pos < r_width; bit_pos += curr_width) {
			u32 ncomb;
			u32 n;

			if (f_width)
				curr_width = f_width;
			else
				curr_width = config_reg->var_field_width[m];

			ncomb = 1 << curr_width;
			for (n = 0; n < ncomb; n++) {
				if (config_reg->enum_ids[pos + n] == enum_id) {
					*crp = config_reg;
					*fieldp = m;
					*valuep = n;
					return 0;
				}
			}
			pos += ncomb;
			m++;
		}
		k++;
	}

	return -EINVAL;
}

static int sh_pfc_mark_to_enum(struct sh_pfc *pfc, u16 mark, int pos,
			      u16 *enum_idp)
{
	const u16 *data = pfc->info->pinmux_data;
	unsigned int k;

	if (pos) {
		*enum_idp = data[pos + 1];
		return pos + 1;
	}

	for (k = 0; k < pfc->info->pinmux_data_size; k++) {
		if (data[k] == mark) {
			*enum_idp = data[k + 1];
			return k + 1;
		}
	}

	dev_err(pfc->dev, "cannot locate data/mark enum_id for mark %d\n",
		mark);
	return -EINVAL;
}

int sh_pfc_config_mux(struct sh_pfc *pfc, unsigned mark, int pinmux_type)
{
	const struct pinmux_range *range;
	int pos = 0;

	switch (pinmux_type) {
	case PINMUX_TYPE_GPIO:
	case PINMUX_TYPE_FUNCTION:
		range = NULL;
		break;

	case PINMUX_TYPE_OUTPUT:
		range = &pfc->info->output;
		break;

	case PINMUX_TYPE_INPUT:
		range = &pfc->info->input;
		break;

	default:
		return -EINVAL;
	}

	/* Iterate over all the configuration fields we need to update. */
	while (1) {
		const struct pinmux_cfg_reg *cr;
		unsigned int field;
		u16 enum_id;
		u32 value;
		int in_range;
		int ret;

		pos = sh_pfc_mark_to_enum(pfc, mark, pos, &enum_id);
		if (pos < 0)
			return pos;

		if (!enum_id)
			break;

		/* Check if the configuration field selects a function. If it
		 * doesn't, skip the field if it's not applicable to the
		 * requested pinmux type.
		 */
		in_range = sh_pfc_enum_in_range(enum_id, &pfc->info->function);
		if (!in_range) {
			if (pinmux_type == PINMUX_TYPE_FUNCTION) {
				/* Functions are allowed to modify all
				 * fields.
				 */
				in_range = 1;
			} else if (pinmux_type != PINMUX_TYPE_GPIO) {
				/* Input/output types can only modify fields
				 * that correspond to their respective ranges.
				 */
				in_range = sh_pfc_enum_in_range(enum_id, range);

				/*
				 * special case pass through for fixed
				 * input-only or output-only pins without
				 * function enum register association.
				 */
				if (in_range && enum_id == range->force)
					continue;
			}
			/* GPIOs are only allowed to modify function fields. */
		}

		if (!in_range)
			continue;

		ret = sh_pfc_get_config_reg(pfc, enum_id, &cr, &field, &value);
		if (ret < 0)
			return ret;

		sh_pfc_write_config_reg(pfc, cr, field, value);
	}

	return 0;
}

const struct pinmux_bias_reg *
sh_pfc_pin_to_bias_reg(const struct sh_pfc *pfc, unsigned int pin,
		       unsigned int *bit)
{
	unsigned int i, j;

	for (i = 0; pfc->info->bias_regs[i].puen; i++) {
		for (j = 0; j < ARRAY_SIZE(pfc->info->bias_regs[i].pins); j++) {
			if (pfc->info->bias_regs[i].pins[j] == pin) {
				*bit = j;
				return &pfc->info->bias_regs[i];
			}
		}
	}

	WARN_ONCE(1, "Pin %u is not in bias info list\n", pin);

	return NULL;
}

static int sh_pfc_init_ranges(struct sh_pfc *pfc)
{
	struct sh_pfc_pin_range *range;
	unsigned int nr_ranges;
	unsigned int i;

	if (pfc->info->pins[0].pin == (u16)-1) {
		/* Pin number -1 denotes that the SoC doesn't report pin numbers
		 * in its pin arrays yet. Consider the pin numbers range as
		 * continuous and allocate a single range.
		 */
		pfc->nr_ranges = 1;
		pfc->ranges = kzalloc(sizeof(*pfc->ranges), GFP_KERNEL);
		if (pfc->ranges == NULL)
			return -ENOMEM;

		pfc->ranges->start = 0;
		pfc->ranges->end = pfc->info->nr_pins - 1;
		pfc->nr_gpio_pins = pfc->info->nr_pins;

		return 0;
	}

	/* Count, allocate and fill the ranges. The PFC SoC data pins array must
	 * be sorted by pin numbers, and pins without a GPIO port must come
	 * last.
	 */
	for (i = 1, nr_ranges = 1; i < pfc->info->nr_pins; ++i) {
		if (pfc->info->pins[i-1].pin != pfc->info->pins[i].pin - 1)
			nr_ranges++;
	}

	pfc->nr_ranges = nr_ranges;
	pfc->ranges = kzalloc(sizeof(*pfc->ranges) * nr_ranges, GFP_KERNEL);
	if (pfc->ranges == NULL)
		return -ENOMEM;

	range = pfc->ranges;
	range->start = pfc->info->pins[0].pin;

	for (i = 1; i < pfc->info->nr_pins; ++i) {
		if (pfc->info->pins[i-1].pin == pfc->info->pins[i].pin - 1)
			continue;

		range->end = pfc->info->pins[i-1].pin;
		if (!(pfc->info->pins[i-1].configs & SH_PFC_PIN_CFG_NO_GPIO))
			pfc->nr_gpio_pins = range->end + 1;

		range++;
		range->start = pfc->info->pins[i].pin;
	}

	range->end = pfc->info->pins[i-1].pin;
	if (!(pfc->info->pins[i-1].configs & SH_PFC_PIN_CFG_NO_GPIO))
		pfc->nr_gpio_pins = range->end + 1;

	return 0;
}

static int sh_pfc_pinctrl_get_pins_count(struct udevice *dev)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->nr_pins;
}

static const char *sh_pfc_pinctrl_get_pin_name(struct udevice *dev,
						  unsigned selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->pins[selector].name;
}

static int sh_pfc_pinctrl_get_groups_count(struct udevice *dev)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->nr_groups;
}

static const char *sh_pfc_pinctrl_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->groups[selector].name;
}

static int sh_pfc_pinctrl_get_functions_count(struct udevice *dev)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->nr_functions;
}

static const char *sh_pfc_pinctrl_get_function_name(struct udevice *dev,
						  unsigned selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pfc.info->functions[selector].name;
}

static int sh_pfc_gpio_request_enable(struct udevice *dev,
				      unsigned pin_selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	struct sh_pfc_pin_config *cfg;
	const struct sh_pfc_pin *pin = NULL;
	int i, ret, idx;

	for (i = 1; i < pfc->info->nr_pins; i++) {
		if (priv->pfc.info->pins[i].pin != pin_selector)
			continue;

		pin = &priv->pfc.info->pins[i];
		break;
	}

	if (!pin)
		return -EINVAL;

	idx = sh_pfc_get_pin_index(pfc, pin->pin);
	cfg = &pmx->configs[idx];

	if (cfg->type != PINMUX_TYPE_NONE)
		return -EBUSY;

	ret = sh_pfc_config_mux(pfc, pin->enum_id, PINMUX_TYPE_GPIO);
	if (ret)
		return ret;

	cfg->type = PINMUX_TYPE_GPIO;

	return 0;
}

static int sh_pfc_gpio_disable_free(struct udevice *dev,
				    unsigned pin_selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	struct sh_pfc_pin_config *cfg;
	const struct sh_pfc_pin *pin = NULL;
	int i, idx;

	for (i = 1; i < pfc->info->nr_pins; i++) {
		if (priv->pfc.info->pins[i].pin != pin_selector)
			continue;

		pin = &priv->pfc.info->pins[i];
		break;
	}

	if (!pin)
		return -EINVAL;

	idx = sh_pfc_get_pin_index(pfc, pin->pin);
	cfg = &pmx->configs[idx];

	cfg->type = PINMUX_TYPE_NONE;

	return 0;
}

static int sh_pfc_pinctrl_pin_set(struct udevice *dev, unsigned pin_selector,
				  unsigned func_selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	const struct sh_pfc_pin *pin = &priv->pfc.info->pins[pin_selector];
	int idx = sh_pfc_get_pin_index(pfc, pin->pin);
	struct sh_pfc_pin_config *cfg = &pmx->configs[idx];

	if (cfg->type != PINMUX_TYPE_NONE)
		return -EBUSY;

	return sh_pfc_config_mux(pfc, pin->enum_id, PINMUX_TYPE_FUNCTION);
}

static int sh_pfc_pinctrl_group_set(struct udevice *dev, unsigned group_selector,
				     unsigned func_selector)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	const struct sh_pfc_pin_group *grp = &priv->pfc.info->groups[group_selector];
	unsigned int i;
	int ret = 0;

	for (i = 0; i < grp->nr_pins; ++i) {
		int idx = sh_pfc_get_pin_index(pfc, grp->pins[i]);
		struct sh_pfc_pin_config *cfg = &pmx->configs[idx];

		if (cfg->type != PINMUX_TYPE_NONE) {
			ret = -EBUSY;
			goto done;
		}
	}

	for (i = 0; i < grp->nr_pins; ++i) {
		ret = sh_pfc_config_mux(pfc, grp->mux[i], PINMUX_TYPE_FUNCTION);
		if (ret < 0)
			break;
	}

done:
	return ret;
}
#if CONFIG_IS_ENABLED(PINCONF)
static const struct pinconf_param sh_pfc_pinconf_params[] = {
	{ "bias-disable",	PIN_CONFIG_BIAS_DISABLE,	0 },
	{ "bias-pull-up",	PIN_CONFIG_BIAS_PULL_UP,	1 },
	{ "bias-pull-down",	PIN_CONFIG_BIAS_PULL_DOWN,	1 },
	{ "drive-strength",	PIN_CONFIG_DRIVE_STRENGTH,	0 },
	{ "power-source",	PIN_CONFIG_POWER_SOURCE,	3300 },
};

static void __iomem *
sh_pfc_pinconf_find_drive_strength_reg(struct sh_pfc *pfc, unsigned int pin,
				       unsigned int *offset, unsigned int *size)
{
	const struct pinmux_drive_reg_field *field;
	const struct pinmux_drive_reg *reg;
	unsigned int i;

	for (reg = pfc->info->drive_regs; reg->reg; ++reg) {
		for (i = 0; i < ARRAY_SIZE(reg->fields); ++i) {
			field = &reg->fields[i];

			if (field->size && field->pin == pin) {
				*offset = field->offset;
				*size = field->size;

				return (void __iomem *)(uintptr_t)reg->reg;
			}
		}
	}

	return NULL;
}

static int sh_pfc_pinconf_set_drive_strength(struct sh_pfc *pfc,
					     unsigned int pin, u16 strength)
{
	unsigned int offset;
	unsigned int size;
	unsigned int step;
	void __iomem *reg;
	void __iomem *unlock_reg =
		(void __iomem *)(uintptr_t)pfc->info->unlock_reg;
	u32 val;

	reg = sh_pfc_pinconf_find_drive_strength_reg(pfc, pin, &offset, &size);
	if (!reg)
		return -EINVAL;

	step = size == 2 ? 6 : 3;

	if (strength < step || strength > 24)
		return -EINVAL;

	/* Convert the value from mA based on a full drive strength value of
	 * 24mA. We can make the full value configurable later if needed.
	 */
	strength = strength / step - 1;

	val = sh_pfc_read_raw_reg(reg, 32);
	val &= ~GENMASK(offset + 4 - 1, offset);
	val |= strength << offset;

	if (unlock_reg)
		sh_pfc_write_raw_reg(unlock_reg, 32, ~val);

	sh_pfc_write_raw_reg(reg, 32, val);

	return 0;
}

/* Check whether the requested parameter is supported for a pin. */
static bool sh_pfc_pinconf_validate(struct sh_pfc *pfc, unsigned int _pin,
				    unsigned int param)
{
	int idx = sh_pfc_get_pin_index(pfc, _pin);
	const struct sh_pfc_pin *pin = &pfc->info->pins[idx];

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		return pin->configs &
			(SH_PFC_PIN_CFG_PULL_UP | SH_PFC_PIN_CFG_PULL_DOWN);

	case PIN_CONFIG_BIAS_PULL_UP:
		return pin->configs & SH_PFC_PIN_CFG_PULL_UP;

	case PIN_CONFIG_BIAS_PULL_DOWN:
		return pin->configs & SH_PFC_PIN_CFG_PULL_DOWN;

	case PIN_CONFIG_DRIVE_STRENGTH:
		return pin->configs & SH_PFC_PIN_CFG_DRIVE_STRENGTH;

	case PIN_CONFIG_POWER_SOURCE:
		return pin->configs & SH_PFC_PIN_CFG_IO_VOLTAGE;

	default:
		return false;
	}
}

static int sh_pfc_pinconf_set(struct sh_pfc_pinctrl *pmx, unsigned _pin,
			      unsigned int param, unsigned int arg)
{
	struct sh_pfc *pfc = pmx->pfc;
	void __iomem *pocctrl;
	void __iomem *unlock_reg =
		(void __iomem *)(uintptr_t)pfc->info->unlock_reg;
	u32 addr, val;
	int bit, ret;

	if (!sh_pfc_pinconf_validate(pfc, _pin, param))
		return -ENOTSUPP;

	switch (param) {
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_DISABLE:
		if (!pfc->info->ops || !pfc->info->ops->set_bias)
			return -ENOTSUPP;

		pfc->info->ops->set_bias(pfc, _pin, param);

		break;

	case PIN_CONFIG_DRIVE_STRENGTH:
		ret = sh_pfc_pinconf_set_drive_strength(pfc, _pin, arg);
		if (ret < 0)
			return ret;

		break;

	case PIN_CONFIG_POWER_SOURCE:
		if (!pfc->info->ops || !pfc->info->ops->pin_to_pocctrl)
			return -ENOTSUPP;

		bit = pfc->info->ops->pin_to_pocctrl(pfc, _pin, &addr);
		if (bit < 0) {
			printf("invalid pin %#x", _pin);
			return bit;
		}

		if (arg != 1800 && arg != 3300)
			return -EINVAL;

		pocctrl = (void __iomem *)(uintptr_t)addr;

		val = sh_pfc_read_raw_reg(pocctrl, 32);
		if (arg == 3300)
			val |= BIT(bit);
		else
			val &= ~BIT(bit);

		if (unlock_reg)
			sh_pfc_write_raw_reg(unlock_reg, 32, ~val);

		sh_pfc_write_raw_reg(pocctrl, 32, val);

		break;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int sh_pfc_pinconf_pin_set(struct udevice *dev,
				  unsigned int pin_selector,
				  unsigned int param, unsigned int arg)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	const struct sh_pfc_pin *pin = &pfc->info->pins[pin_selector];

	sh_pfc_pinconf_set(pmx, pin->pin, param, arg);

	return 0;
}

static int sh_pfc_pinconf_group_set(struct udevice *dev,
				      unsigned int group_selector,
				      unsigned int param, unsigned int arg)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	struct sh_pfc_pinctrl *pmx = &priv->pmx;
	struct sh_pfc *pfc = &priv->pfc;
	const struct sh_pfc_pin_group *grp = &pfc->info->groups[group_selector];
	unsigned int i;

	for (i = 0; i < grp->nr_pins; i++)
		sh_pfc_pinconf_set(pmx, grp->pins[i], param, arg);

	return 0;
}
#endif

static struct pinctrl_ops sh_pfc_pinctrl_ops = {
	.get_pins_count		= sh_pfc_pinctrl_get_pins_count,
	.get_pin_name		= sh_pfc_pinctrl_get_pin_name,
	.get_groups_count	= sh_pfc_pinctrl_get_groups_count,
	.get_group_name		= sh_pfc_pinctrl_get_group_name,
	.get_functions_count	= sh_pfc_pinctrl_get_functions_count,
	.get_function_name	= sh_pfc_pinctrl_get_function_name,

#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params	= ARRAY_SIZE(sh_pfc_pinconf_params),
	.pinconf_params		= sh_pfc_pinconf_params,
	.pinconf_set		= sh_pfc_pinconf_pin_set,
	.pinconf_group_set	= sh_pfc_pinconf_group_set,
#endif
	.pinmux_set		= sh_pfc_pinctrl_pin_set,
	.pinmux_group_set	= sh_pfc_pinctrl_group_set,
	.set_state		= pinctrl_generic_set_state,

	.gpio_request_enable	= sh_pfc_gpio_request_enable,
	.gpio_disable_free	= sh_pfc_gpio_disable_free,
};

static int sh_pfc_map_pins(struct sh_pfc *pfc, struct sh_pfc_pinctrl *pmx)
{
	unsigned int i;

	/* Allocate and initialize the pins and configs arrays. */
	pmx->configs = kzalloc(sizeof(*pmx->configs) * pfc->info->nr_pins,
				    GFP_KERNEL);
	if (unlikely(!pmx->configs))
		return -ENOMEM;

	for (i = 0; i < pfc->info->nr_pins; ++i) {
		struct sh_pfc_pin_config *cfg = &pmx->configs[i];
		cfg->type = PINMUX_TYPE_NONE;
	}

	return 0;
}


static int sh_pfc_pinctrl_probe(struct udevice *dev)
{
	struct sh_pfc_pinctrl_priv *priv = dev_get_priv(dev);
	enum sh_pfc_model model = dev_get_driver_data(dev);
	fdt_addr_t base;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->pfc.regs = devm_ioremap(dev, base, SZ_2K);
	if (!priv->pfc.regs)
		return -ENOMEM;

#ifdef CONFIG_PINCTRL_PFC_R8A7790
	if (model == SH_PFC_R8A7790)
		priv->pfc.info = &r8a7790_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7791
	if (model == SH_PFC_R8A7791)
		priv->pfc.info = &r8a7791_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7792
	if (model == SH_PFC_R8A7792)
		priv->pfc.info = &r8a7792_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7793
	if (model == SH_PFC_R8A7793)
		priv->pfc.info = &r8a7793_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7794
	if (model == SH_PFC_R8A7794)
		priv->pfc.info = &r8a7794_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7795
	if (model == SH_PFC_R8A7795)
		priv->pfc.info = &r8a7795_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7796
	if (model == SH_PFC_R8A7796)
		priv->pfc.info = &r8a7796_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77965
	if (model == SH_PFC_R8A77965)
		priv->pfc.info = &r8a77965_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77970
	if (model == SH_PFC_R8A77970)
		priv->pfc.info = &r8a77970_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77990
	if (model == SH_PFC_R8A77990)
		priv->pfc.info = &r8a77990_pinmux_info;
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77995
	if (model == SH_PFC_R8A77995)
		priv->pfc.info = &r8a77995_pinmux_info;
#endif

	priv->pmx.pfc = &priv->pfc;
	sh_pfc_init_ranges(&priv->pfc);
	sh_pfc_map_pins(&priv->pfc, &priv->pmx);

	return 0;
}

static const struct udevice_id sh_pfc_pinctrl_ids[] = {
#ifdef CONFIG_PINCTRL_PFC_R8A7790
	{
		.compatible = "renesas,pfc-r8a7790",
		.data = SH_PFC_R8A7790,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7791
	{
		.compatible = "renesas,pfc-r8a7791",
		.data = SH_PFC_R8A7791,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7792
	{
		.compatible = "renesas,pfc-r8a7792",
		.data = SH_PFC_R8A7792,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7793
	{
		.compatible = "renesas,pfc-r8a7793",
		.data = SH_PFC_R8A7793,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7794
	{
		.compatible = "renesas,pfc-r8a7794",
		.data = SH_PFC_R8A7794,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7795
	{
		.compatible = "renesas,pfc-r8a7795",
		.data = SH_PFC_R8A7795,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A7796
	{
		.compatible = "renesas,pfc-r8a7796",
		.data = SH_PFC_R8A7796,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77965
	{
		.compatible = "renesas,pfc-r8a77965",
		.data = SH_PFC_R8A77965,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77970
	{
		.compatible = "renesas,pfc-r8a77970",
		.data = SH_PFC_R8A77970,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77990
	{
		.compatible = "renesas,pfc-r8a77990",
		.data = SH_PFC_R8A77990,
	},
#endif
#ifdef CONFIG_PINCTRL_PFC_R8A77995
	{
		.compatible = "renesas,pfc-r8a77995",
		.data = SH_PFC_R8A77995,
	},
#endif
	{ },
};

U_BOOT_DRIVER(pinctrl_sh_pfc) = {
	.name		= "sh_pfc_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= sh_pfc_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct sh_pfc_pinctrl_priv),
	.ops		= &sh_pfc_pinctrl_ops,
	.probe		= sh_pfc_pinctrl_probe,
};
