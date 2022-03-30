// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

#include "pinctrl-mtk-common.h"

/**
 * struct mtk_drive_desc - the structure that holds the information
 *			    of the driving current
 * @min:	the minimum current of this group
 * @max:	the maximum current of this group
 * @step:	the step current of this group
 * @scal:	the weight factor
 *
 * formula: output = ((input) / step - 1) * scal
 */
struct mtk_drive_desc {
	u8 min;
	u8 max;
	u8 step;
	u8 scal;
};

/* The groups of drive strength */
static const struct mtk_drive_desc mtk_drive[] = {
	[DRV_GRP0] = { 4, 16, 4, 1 },
	[DRV_GRP1] = { 4, 16, 4, 2 },
	[DRV_GRP2] = { 2, 8, 2, 1 },
	[DRV_GRP3] = { 2, 8, 2, 2 },
	[DRV_GRP4] = { 2, 16, 2, 1 },
};

static const char *mtk_pinctrl_dummy_name = "_dummy";

static void mtk_w32(struct udevice *dev, u32 reg, u32 val)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	__raw_writel(val, priv->base + reg);
}

static u32 mtk_r32(struct udevice *dev, u32 reg)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	return __raw_readl(priv->base + reg);
}

static inline int get_count_order(unsigned int count)
{
	int order;

	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

void mtk_rmw(struct udevice *dev, u32 reg, u32 mask, u32 set)
{
	u32 val;

	val = mtk_r32(dev, reg);
	val &= ~mask;
	val |= set;
	mtk_w32(dev, reg, val);
}

static int mtk_hw_pin_field_lookup(struct udevice *dev, int pin,
				   const struct mtk_pin_reg_calc *rc,
				   struct mtk_pin_field *pfd)
{
	const struct mtk_pin_field_calc *c, *e;
	u32 bits;

	c = rc->range;
	e = c + rc->nranges;

	while (c < e) {
		if (pin >= c->s_pin && pin <= c->e_pin)
			break;
		c++;
	}

	if (c >= e)
		return -EINVAL;

	/* Calculated bits as the overall offset the pin is located at,
	 * if c->fixed is held, that determines the all the pins in the
	 * range use the same field with the s_pin.
	 */
	bits = c->fixed ? c->s_bit : c->s_bit + (pin - c->s_pin) * (c->x_bits);

	/* Fill pfd from bits. For example 32-bit register applied is assumed
	 * when c->sz_reg is equal to 32.
	 */
	pfd->offset = c->s_addr + c->x_addrs * (bits / c->sz_reg);
	pfd->bitpos = bits % c->sz_reg;
	pfd->mask = (1 << c->x_bits) - 1;

	/* pfd->next is used for indicating that bit wrapping-around happens
	 * which requires the manipulation for bit 0 starting in the next
	 * register to form the complete field read/write.
	 */
	pfd->next = pfd->bitpos + c->x_bits > c->sz_reg ? c->x_addrs : 0;

	return 0;
}

static int mtk_hw_pin_field_get(struct udevice *dev, int pin,
				int field, struct mtk_pin_field *pfd)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);
	const struct mtk_pin_reg_calc *rc;

	if (field < 0 || field >= PINCTRL_PIN_REG_MAX)
		return -EINVAL;

	if (priv->soc->reg_cal && priv->soc->reg_cal[field].range)
		rc = &priv->soc->reg_cal[field];
	else
		return -EINVAL;

	return mtk_hw_pin_field_lookup(dev, pin, rc, pfd);
}

static void mtk_hw_bits_part(struct mtk_pin_field *pf, int *h, int *l)
{
	*l = 32 - pf->bitpos;
	*h = get_count_order(pf->mask) - *l;
}

static void mtk_hw_write_cross_field(struct udevice *dev,
				     struct mtk_pin_field *pf, int value)
{
	int nbits_l, nbits_h;

	mtk_hw_bits_part(pf, &nbits_h, &nbits_l);

	mtk_rmw(dev, pf->offset, pf->mask << pf->bitpos,
		(value & pf->mask) << pf->bitpos);

	mtk_rmw(dev, pf->offset + pf->next, BIT(nbits_h) - 1,
		(value & pf->mask) >> nbits_l);
}

static void mtk_hw_read_cross_field(struct udevice *dev,
				    struct mtk_pin_field *pf, int *value)
{
	int nbits_l, nbits_h, h, l;

	mtk_hw_bits_part(pf, &nbits_h, &nbits_l);

	l  = (mtk_r32(dev, pf->offset) >> pf->bitpos) & (BIT(nbits_l) - 1);
	h  = (mtk_r32(dev, pf->offset + pf->next)) & (BIT(nbits_h) - 1);

	*value = (h << nbits_l) | l;
}

static int mtk_hw_set_value(struct udevice *dev, int pin, int field,
			    int value)
{
	struct mtk_pin_field pf;
	int err;

	err = mtk_hw_pin_field_get(dev, pin, field, &pf);
	if (err)
		return err;

	if (!pf.next)
		mtk_rmw(dev, pf.offset, pf.mask << pf.bitpos,
			(value & pf.mask) << pf.bitpos);
	else
		mtk_hw_write_cross_field(dev, &pf, value);

	return 0;
}

static int mtk_hw_get_value(struct udevice *dev, int pin, int field,
			    int *value)
{
	struct mtk_pin_field pf;
	int err;

	err = mtk_hw_pin_field_get(dev, pin, field, &pf);
	if (err)
		return err;

	if (!pf.next)
		*value = (mtk_r32(dev, pf.offset) >> pf.bitpos) & pf.mask;
	else
		mtk_hw_read_cross_field(dev, &pf, value);

	return 0;
}

static int mtk_get_groups_count(struct udevice *dev)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->soc->ngrps;
}

static const char *mtk_get_pin_name(struct udevice *dev,
				    unsigned int selector)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->soc->grps[selector].name)
		return mtk_pinctrl_dummy_name;

	return priv->soc->pins[selector].name;
}

static int mtk_get_pins_count(struct udevice *dev)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->soc->npins;
}

static const char *mtk_get_group_name(struct udevice *dev,
				      unsigned int selector)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->soc->grps[selector].name)
		return mtk_pinctrl_dummy_name;

	return priv->soc->grps[selector].name;
}

static int mtk_get_functions_count(struct udevice *dev)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->soc->nfuncs;
}

static const char *mtk_get_function_name(struct udevice *dev,
					 unsigned int selector)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->soc->funcs[selector].name)
		return mtk_pinctrl_dummy_name;

	return priv->soc->funcs[selector].name;
}

static int mtk_pinmux_group_set(struct udevice *dev,
				unsigned int group_selector,
				unsigned int func_selector)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);
	const struct mtk_group_desc *grp =
			&priv->soc->grps[group_selector];
	int i;

	for (i = 0; i < grp->num_pins; i++) {
		int *pin_modes = grp->data;

		mtk_hw_set_value(dev, grp->pins[i], PINCTRL_PIN_REG_MODE,
				 pin_modes[i]);
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PINCONF)
static const struct pinconf_param mtk_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1, },
	{ "output-low", PIN_CONFIG_OUTPUT, 0, },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
};

int mtk_pinconf_drive_set(struct udevice *dev, u32 pin, u32 arg)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);
	const struct mtk_pin_desc *desc = &priv->soc->pins[pin];
	const struct mtk_drive_desc *tb;
	int err = -ENOTSUPP;

	tb = &mtk_drive[desc->drv_n];
	/* 4mA when (e8, e4) = (0, 0)
	 * 8mA when (e8, e4) = (0, 1)
	 * 12mA when (e8, e4) = (1, 0)
	 * 16mA when (e8, e4) = (1, 1)
	 */
	if ((arg >= tb->min && arg <= tb->max) && !(arg % tb->step)) {
		arg = (arg / tb->step - 1) * tb->scal;

		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DRV, arg);
		if (err)
			return err;
	}

	return 0;
}

static int mtk_pinconf_set(struct udevice *dev, unsigned int pin,
			   unsigned int param, unsigned int arg)
{
	int err = 0;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
		arg = (param == PIN_CONFIG_BIAS_DISABLE) ? 0 :
			(param == PIN_CONFIG_BIAS_PULL_UP) ? 3 : 2;

		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_PULLSEL,
				       arg & 1);
		if (err)
			goto err;

		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_PULLEN,
				       !!(arg & 2));
		if (err)
			goto err;
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_SMT, 0);
		if (err)
			goto err;
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DIR, 1);
		if (err)
			goto err;
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_IES, 1);
		if (err)
			goto err;
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DIR, 0);
		if (err)
			goto err;
		break;
	case PIN_CONFIG_OUTPUT:
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DIR, 1);
		if (err)
			goto err;

		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DO, arg);
		if (err)
			goto err;
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		/* arg = 1: Input mode & SMT enable ;
		 * arg = 0: Output mode & SMT disable
		 */
		arg = arg ? 2 : 1;
		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_DIR,
				       arg & 1);
		if (err)
			goto err;

		err = mtk_hw_set_value(dev, pin, PINCTRL_PIN_REG_SMT,
				       !!(arg & 2));
		if (err)
			goto err;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		err = mtk_pinconf_drive_set(dev, pin, arg);
		if (err)
			goto err;
		break;

	default:
		err = -ENOTSUPP;
	}

err:

	return err;
}

static int mtk_pinconf_group_set(struct udevice *dev,
				 unsigned int group_selector,
				 unsigned int param, unsigned int arg)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);
	const struct mtk_group_desc *grp =
			&priv->soc->grps[group_selector];
	int i, ret;

	for (i = 0; i < grp->num_pins; i++) {
		ret = mtk_pinconf_set(dev, grp->pins[i], param, arg);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

const struct pinctrl_ops mtk_pinctrl_ops = {
	.get_pins_count = mtk_get_pins_count,
	.get_pin_name = mtk_get_pin_name,
	.get_groups_count = mtk_get_groups_count,
	.get_group_name = mtk_get_group_name,
	.get_functions_count = mtk_get_functions_count,
	.get_function_name = mtk_get_function_name,
	.pinmux_group_set = mtk_pinmux_group_set,
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params = ARRAY_SIZE(mtk_conf_params),
	.pinconf_params = mtk_conf_params,
	.pinconf_set = mtk_pinconf_set,
	.pinconf_group_set = mtk_pinconf_group_set,
#endif
	.set_state = pinctrl_generic_set_state,
};

static int mtk_gpio_get(struct udevice *dev, unsigned int off)
{
	int val, err;

	err = mtk_hw_get_value(dev->parent, off, PINCTRL_PIN_REG_DI, &val);
	if (err)
		return err;

	return !!val;
}

static int mtk_gpio_set(struct udevice *dev, unsigned int off, int val)
{
	return mtk_hw_set_value(dev->parent, off, PINCTRL_PIN_REG_DO, !!val);
}

static int mtk_gpio_get_direction(struct udevice *dev, unsigned int off)
{
	int val, err;

	err = mtk_hw_get_value(dev->parent, off, PINCTRL_PIN_REG_DIR, &val);
	if (err)
		return err;

	return val ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int mtk_gpio_direction_input(struct udevice *dev, unsigned int off)
{
	return mtk_hw_set_value(dev->parent, off, PINCTRL_PIN_REG_DIR, 0);
}

static int mtk_gpio_direction_output(struct udevice *dev,
				     unsigned int off, int val)
{
	mtk_gpio_set(dev, off, val);

	/* And set the requested value */
	return mtk_hw_set_value(dev->parent, off, PINCTRL_PIN_REG_DIR, 1);
}

static int mtk_gpio_request(struct udevice *dev, unsigned int off,
			    const char *label)
{
	return mtk_hw_set_value(dev->parent, off, PINCTRL_PIN_REG_MODE, 0);
}

static int mtk_gpio_probe(struct udevice *dev)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev->parent);
	struct gpio_dev_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	uc_priv->bank_name = priv->soc->name;
	uc_priv->gpio_count = priv->soc->npins;

	return 0;
}

static const struct dm_gpio_ops mtk_gpio_ops = {
	.request = mtk_gpio_request,
	.set_value = mtk_gpio_set,
	.get_value = mtk_gpio_get,
	.get_function = mtk_gpio_get_direction,
	.direction_input = mtk_gpio_direction_input,
	.direction_output = mtk_gpio_direction_output,
};

static struct driver mtk_gpio_driver = {
	.name = "mediatek_gpio",
	.id	= UCLASS_GPIO,
	.probe = mtk_gpio_probe,
	.ops = &mtk_gpio_ops,
};

static int mtk_gpiochip_register(struct udevice *parent)
{
	struct uclass_driver *drv;
	struct udevice *dev;
	int ret;
	ofnode node;

	drv = lists_uclass_lookup(UCLASS_GPIO);
	if (!drv)
		return -ENOENT;

	dev_for_each_subnode(node, parent)
		if (ofnode_read_bool(node, "gpio-controller")) {
			ret = 0;
			break;
		}

	if (ret)
		return ret;

	ret = device_bind_with_driver_data(parent, &mtk_gpio_driver,
					   "mediatek_gpio", 0, node,
					   &dev);
	if (ret)
		return ret;

	return 0;
}

int mtk_pinctrl_common_probe(struct udevice *dev,
			     struct mtk_pinctrl_soc *soc)
{
	struct mtk_pinctrl_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (priv->base == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->soc = soc;

	ret = mtk_gpiochip_register(dev);
	if (ret)
		return ret;

	return 0;
}
