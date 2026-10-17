// SPDX-License-Identifier: GPL-2.0+
/*
 * Atmel PIO4 pinctrl driver
 *
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm/uclass-internal.h>
#include <mach/atmel_pio4.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Warning:
 * In order to not introduce confusion between Atmel PIO groups and pinctrl
 * framework groups, Atmel PIO groups will be called banks.
 */

struct atmel_pio4_plat {
	struct atmel_pio4_port *reg_base;
	unsigned int slew_rate_support;
};

/*
 * Table keeping track of the pinctrl driver's slew rate support and the
 * corresponding index into the struct udevice_id of the gpio_atmel_pio4 GPIO
 * driver. This has been done in order to align the DT of U-Boot with the DT of
 * Linux. In Linux, a phandle from a '-gpio' DT property is linked to the
 * pinctrl driver, unlike U-Boot which redirects this phandle to a corresponding
 * UCLASS_GPIO driver. Thus, in order to link the two, a hook to the bind method
 * of the pinctrl driver in U-Boot has been added. This bind method will attach
 * the GPIO driver to the pinctrl DT node using this table.
 * @slew_rate_support	 pinctrl driver's slew rate support
 * @gdidx		 index into the GPIO driver's struct udevide_id
 *			 (needed in order to properly bind with driver_data)
 */

struct atmel_pinctrl_data {
	unsigned int slew_rate_support;
	int gdidx;
};

static const struct pinconf_param conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-debounce", PIN_CONFIG_INPUT_DEBOUNCE, 0 },
	{ "atmel,drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0},
};

static u32 atmel_pinctrl_get_pinconf(struct udevice *config,
				     struct atmel_pio4_plat *plat)
{
	const struct pinconf_param *params;
	u32 param, arg, conf = 0;
	u32 i;
	u32 val;

	for (i = 0; i < ARRAY_SIZE(conf_params); i++) {
		params = &conf_params[i];
		if (!dev_read_prop(config, params->property, NULL))
			continue;

		param = params->param;
		arg = params->default_value;

		/* Keep slew rate enabled by default. */
		if (plat->slew_rate_support)
			conf |= ATMEL_PIO_SR;

		switch (param) {
		case PIN_CONFIG_BIAS_DISABLE:
			conf &= (~ATMEL_PIO_PUEN_MASK);
			conf &= (~ATMEL_PIO_PDEN_MASK);
			break;
		case PIN_CONFIG_BIAS_PULL_UP:
			conf |= ATMEL_PIO_PUEN_MASK;
			break;
		case PIN_CONFIG_BIAS_PULL_DOWN:
			conf |= ATMEL_PIO_PDEN_MASK;
			break;
		case PIN_CONFIG_DRIVE_OPEN_DRAIN:
			if (arg == 0)
				conf &= (~ATMEL_PIO_OPD_MASK);
			else
				conf |= ATMEL_PIO_OPD_MASK;
			break;
		case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
			if (arg == 0)
				conf |= ATMEL_PIO_SCHMITT_MASK;
			else
				conf &= (~ATMEL_PIO_SCHMITT_MASK);
			break;
		case PIN_CONFIG_INPUT_DEBOUNCE:
			if (arg == 0) {
				conf &= (~ATMEL_PIO_IFEN_MASK);
				conf &= (~ATMEL_PIO_IFSCEN_MASK);
			} else {
				conf |= ATMEL_PIO_IFEN_MASK;
				conf |= ATMEL_PIO_IFSCEN_MASK;
			}
			break;
		case PIN_CONFIG_DRIVE_STRENGTH:
			dev_read_u32(config, params->property, &val);
			conf &= (~ATMEL_PIO_DRVSTR_MASK);
			conf |= (val << ATMEL_PIO_DRVSTR_OFFSET)
				& ATMEL_PIO_DRVSTR_MASK;
			break;
		case PIN_CONFIG_SLEW_RATE:
			if (!plat->slew_rate_support)
				break;

			dev_read_u32(config, params->property, &val);
			/* And disable it if requested. */
			if (val == 0)
				conf &= ~ATMEL_PIO_SR;
			break;
		default:
			printf("%s: Unsupported configuration parameter: %u\n",
			       __func__, param);
			break;
		}
	}

	return conf;
}

static inline struct atmel_pio4_port *atmel_pio4_bank_base(struct udevice *dev,
							   u32 bank)
{
	struct atmel_pio4_plat *plat = dev_get_plat(dev);
	struct atmel_pio4_port *bank_base =
			(struct atmel_pio4_port *)((u32)plat->reg_base +
			ATMEL_PIO_BANK_OFFSET * bank);

	return bank_base;
}

#define MAX_PINMUX_ENTRIES	40

static int atmel_process_config_dev(struct udevice *dev, struct udevice *config)
{
	struct atmel_pio4_plat *plat = dev_get_plat(dev);
	int node = dev_of_offset(config);
	struct atmel_pio4_port *bank_base;
	u32 offset, func, bank, line;
	u32 cells[MAX_PINMUX_ENTRIES];
	u32 i, conf;
	int count;

	conf = atmel_pinctrl_get_pinconf(config, plat);

	/*
	 * The only case where this function returns a negative error value
	 * is when there is no "pinmux" property attached to this node
	 */
	count = fdtdec_get_int_array_count(gd->fdt_blob, node, "pinmux",
					   cells, ARRAY_SIZE(cells));
	if (count < 0)
		return count;

	if (count > MAX_PINMUX_ENTRIES)
		return -EINVAL;

	for (i = 0 ; i < count; i++) {
		offset = ATMEL_GET_PIN_NO(cells[i]);
		func = ATMEL_GET_PIN_FUNC(cells[i]);

		bank = ATMEL_PIO_BANK(offset);
		line = ATMEL_PIO_LINE(offset);

		bank_base = atmel_pio4_bank_base(dev, bank);

		writel(BIT(line), &bank_base->mskr);
		conf &= (~ATMEL_PIO_CFGR_FUNC_MASK);
		conf |= (func & ATMEL_PIO_CFGR_FUNC_MASK);
		writel(conf, &bank_base->cfgr);
	}

	return 0;
}

static int atmel_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	int node = dev_of_offset(config);
	struct udevice *subconfig;
	int subnode, subnode_count = 0, ret;

	/*
	 * If this function returns a negative error code then that means
	 * that either the "pinmux" property of the node is missing, which is
	 * the case for pinctrl nodes that do not have all the pins with the
	 * same configuration and are split in multiple subnodes, or something
	 * else went wrong and we have to stop. For the latter case, it would
	 * mean that the node failed even though it has no subnodes.
	 */
	ret = atmel_process_config_dev(dev, config);
	if (!ret)
		return ret;

	/*
	 * If we reach here, it means that the subnode pinctrl's DT has multiple
	 * subnodes. If it does not, then something else went wrong in the
	 * previous call to atmel_process_config_dev.
	 */
	fdt_for_each_subnode(subnode, gd->fdt_blob, node) {
		/* Get subnode as an udevice */
		ret = uclass_find_device_by_of_offset(UCLASS_PINCONFIG, subnode,
						      &subconfig);
		if (ret)
			return ret;

		/*
		 * If this time the function returns an error code on a subnode
		 * then something is totally wrong so abort.
		 */
		ret = atmel_process_config_dev(dev, subconfig);
		if (ret)
			return ret;

		subnode_count++;
	}

	/*
	 * If we somehow got here and we do not have any subnodes, abort.
	 */
	if (!subnode_count)
		return -EINVAL;

	return 0;
}

const struct pinctrl_ops atmel_pinctrl_ops  = {
	.set_state = atmel_pinctrl_set_state,
};

static int atmel_pinctrl_probe(struct udevice *dev)
{
	struct atmel_pio4_plat *plat = dev_get_plat(dev);
	struct atmel_pinctrl_data *priv = (struct atmel_pinctrl_data *)dev_get_driver_data(dev);
	fdt_addr_t addr_base;

	addr_base = dev_read_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg_base = (struct atmel_pio4_port *)addr_base;
	plat->slew_rate_support = priv->slew_rate_support;

	return 0;
}

static int atmel_pinctrl_bind(struct udevice *dev)
{
	struct udevice *g;
	struct driver *drv;
	ofnode node = dev_ofnode(dev);
	struct atmel_pinctrl_data *priv = (struct atmel_pinctrl_data *)dev_get_driver_data(dev);

	if (!IS_ENABLED(CONFIG_ATMEL_PIO4))
		return 0;

	/* Obtain a handle to the GPIO driver */
	drv = lists_driver_lookup_name("gpio_atmel_pio4");
	if (!drv)
		return -ENOENT;

	/*
	 * Bind the GPIO driver to the pinctrl DT node, together
	 * with its corresponding driver_data.
	 */
	return device_bind_with_driver_data(dev, drv, drv->name,
					    drv->of_match[priv->gdidx].data,
					    node, &g);
}

static const struct atmel_pinctrl_data atmel_sama5d2_pinctrl_data = {
	.gdidx = 0,
};

static const struct atmel_pinctrl_data microchip_sama7g5_pinctrl_data = {
	.slew_rate_support = 1,
	.gdidx = 1,
};

static const struct udevice_id atmel_pinctrl_match[] = {
	{ .compatible = "atmel,sama5d2-pinctrl",
	  .data = (ulong)&atmel_sama5d2_pinctrl_data, },
	{ .compatible = "microchip,sama7g5-pinctrl",
	  .data = (ulong)&microchip_sama7g5_pinctrl_data, },
	{}
};

U_BOOT_DRIVER(atmel_pinctrl) = {
	.name = "pinctrl_atmel_pio4",
	.id = UCLASS_PINCTRL,
	.of_match = atmel_pinctrl_match,
	.bind = atmel_pinctrl_bind,
	.probe = atmel_pinctrl_probe,
	.plat_auto	= sizeof(struct atmel_pio4_plat),
	.ops = &atmel_pinctrl_ops,
};
