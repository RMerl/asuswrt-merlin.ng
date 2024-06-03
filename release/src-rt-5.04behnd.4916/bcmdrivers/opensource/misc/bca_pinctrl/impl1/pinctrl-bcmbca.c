/*
* <:copyright-BRCM:2019:DUAL/GPL:standard
* 
*    Copyright (c) 2019 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
 
*/


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <drivers/pinctrl/core.h>


#define MODULE_NAME "pinctrl-bcmbca"
#define BCMBCA_PINCTRL_PIN_NAME_MAX_LENGTH      32
#define BCMBCA_PINCTRL_GROUP_NAME_MAX_LENGTH    32
#define BCMBCA_PINCTRL_FUNCTION_NAME_MAX_LENGTH 32

#define FREE_KZALLOCATION(address) do {		\
		if (address) {						\
			devm_kfree(pc->dev, address);	\
			address = NULL;					\
		}									\
	} while(0)

struct pinconf_params {
	const char * const property;
	enum pin_config_param param;
	u32 default_value;
};

/* Array of pinconfs that we support */
static const struct pinconf_params dt_config_params[] = {
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 2 },
};

struct bcmbca_pinctrl_pin_name {
	char name[BCMBCA_PINCTRL_PIN_NAME_MAX_LENGTH];
};

struct bcmbca_pinctrl_group_name {
	char name[BCMBCA_PINCTRL_GROUP_NAME_MAX_LENGTH];
};

struct bcmbca_pinctrl_function_name {
	char name[BCMBCA_PINCTRL_FUNCTION_NAME_MAX_LENGTH];
};

struct bcmbca_pinctrl {
	struct device *dev;
	void __iomem *base;

	struct pinctrl_dev *pctl_dev;
	char **bcmbca_groups_names;
	unsigned nfuncs;
	unsigned ngroups; /* groups number is equal to pins number */

	struct pinctrl_desc *desc;
	struct bcmbca_pinctrl_group_name *groups_names;
	struct bcmbca_pinctrl_function_name *functions_names;
	struct pinctrl_pin_desc *pins_descriptors;
	unsigned int gpio_mux;
};


static int bcmbca_create_pinctrl_tables(struct bcmbca_pinctrl *pc);


static int bcmbca_pctl_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	return pc->ngroups;
}

static const char *bcmbca_pctl_get_group_name(struct pinctrl_dev *pctldev, unsigned selector)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	return pc->groups_names[selector].name;
}

static int bcmbca_pctl_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector, const unsigned **pins,
	unsigned *num_pins)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	*pins = &pc->pins_descriptors[selector].number;
	*num_pins = 1;

	return 0;
}

static void bcmbca_pctl_pin_dbg_show(struct pinctrl_dev *pctldev, struct seq_file *s, unsigned offset)
{
	seq_printf(s, " " MODULE_NAME);
}

static void bcmbca_pctl_dt_free_map(struct pinctrl_dev *pctldev, struct pinctrl_map *maps, unsigned num_maps)
{
	int i;

	for (i = 0; i < num_maps; i++)
		if (maps[i].type == PIN_MAP_TYPE_CONFIGS_PIN || maps[i].type == PIN_MAP_TYPE_CONFIGS_GROUP)
			kfree(maps[i].data.configs.configs);

	kfree(maps);
}

static int bcmbca_pctl_dt_node_to_map_func(struct bcmbca_pinctrl *pc, struct device_node *np, 
	u32 pin, u32 fnum, struct pinctrl_map **maps)
{
	struct pinctrl_map *map = *maps;

	if (fnum >= pc->nfuncs) {
		dev_err(pc->dev, "%s: invalid function %d (max=%d)\n", of_node_full_name(np), fnum, pc->nfuncs - 1);
		return -EINVAL;
	}

	if (pin >= pc->desc->npins) {
		dev_err(pc->dev, "%s: invalid pin %d (max=%d)\n", of_node_full_name(np), pin, pc->desc->npins - 1);
		return -EINVAL;
	}

	map->type = PIN_MAP_TYPE_MUX_GROUP;
	map->data.mux.group = pc->groups_names[pin].name;
	map->data.mux.function = pc->functions_names[fnum].name;
	(*maps)++;

	return 0;
}

static int bcmbca_pctl_dt_node_to_map_pin_config(struct bcmbca_pinctrl *pc, struct device_node *np, 
	u32 pin, unsigned long * configs, u32 num_configs, struct pinctrl_map **maps)
{
	struct pinctrl_map *map = *maps;

	if (pin >= pc->desc->npins) {
		dev_err(pc->dev, "%s: invalid pin %d (max=%d)\n", of_node_full_name(np), pin, pc->desc->npins - 1);
		return -EINVAL;
	}

	/* Since we support single pin groups, we are using the group based accessors */
        map->type = PIN_MAP_TYPE_CONFIGS_GROUP;
        map->data.configs.group_or_pin = pc->groups_names[pin].name;
        map->data.configs.configs = configs;
        map->data.configs.num_configs = num_configs;
	(*maps)++;

	return 0;
}

int bcmbca_pctl_parse_dt_config(struct device_node *np,
				    u32 num_pins,
				    struct pinctrl_dev *pctldev,
				    unsigned long **configs,
				    unsigned int *nconfigs_per_pin)
{
	unsigned long *cfg;
	unsigned int max_cfg_per_pin, ncfg = 0;
	struct property * config_prop;
	u32 config_val, num_config_vals, pin;
	int ret=0;
	int i,j;
	const struct pinconf_params *par;

	if (!np)
		return -EINVAL;

	/* allocate a temporary array big enough to hold one of each option */
	max_cfg_per_pin = ARRAY_SIZE(dt_config_params);
	cfg = kcalloc(max_cfg_per_pin, sizeof(unsigned long), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;
		
	/* For each pin, find out which configs exist and store them in array.
	 * This is an extension of the algorithm found in pinconf-generic.c. The
	 * existing algorigthm only allows a single config setting per pin group,
	 * we want a bit more flexibility to allow specifiying a custom config for
	 * each pin in the pin group */
	for( i=0; i<num_pins; i++ ) {
			of_property_read_u32_index(np, "pins", i, &pin);

		for (j = 0; j < ARRAY_SIZE(dt_config_params); j++) {
			par = &dt_config_params[j];
			config_prop = of_find_property(np, par->property, NULL); 

			/* If config not found, move to next config */
			if(!config_prop)
				continue;

			/* Determine if we have single config value for all pins */
			num_config_vals = config_prop->length / sizeof(config_val);
			if( num_config_vals > 1) {
				/* config is defined per pin */
				ret = of_property_read_u32_index(np, par->property, i, &config_val);
			} else {
				/* Single config is defined for all pins */
				ret = of_property_read_u32(np, par->property, &config_val);

			}
			/* property not found */
			if (ret == -EINVAL)
				continue;

			/* use default value, when no value is specified */
			if (ret)
				config_val = par->default_value;

			/* Pack and store config */
			cfg[ncfg] = pinconf_to_config_packed(par->param, config_val);
			ncfg++;
		}

		if(ncfg) {
			/* 
			 * We mandata that each pin in the pin group has the same number of 
			 * pinconfigs i.e we cannot have a situation where pins =<x y> and
			 * pin x has 2 configs applying to it and pin y has only one config 
			 */
			if(!*nconfigs_per_pin)
				*nconfigs_per_pin = ncfg;
		} else {
			/* No configs for one pin implies that there are no configs for ANY pins */
			configs[i] = NULL;
			*nconfigs_per_pin = 0;
			ret = 0;
			goto out;
		}

		/*
		 * Now limit the number of configs to the real number of
		 * found properties.
		 */
		configs[i] = kmemdup(cfg, ncfg * sizeof(unsigned long), GFP_KERNEL);
		if (!configs[i]) {
			ret = -ENOMEM;
			goto out;
		}

		/* reset the config counter */
		ncfg = 0;
	}

out:
	kfree(cfg);
	return ret;
}

static int bcmbca_pctl_dt_node_to_map(struct pinctrl_dev *pctldev, struct device_node *np, struct pinctrl_map **map,
	unsigned *num_maps)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	struct property *pins, *funcs;
	int num_pins = 0;
	int num_funcs = 0;
	struct pinctrl_map *maps, *cur_map;
	int i, err;
	u32 pin, func;
	int num_maps_per_pin = 0;
	unsigned long **all_configs = NULL;
	unsigned num_configs_per_pin = 0;

	cur_map = maps = NULL;
	pins = of_find_property(np, "pins", NULL);
	if (!pins) {
		dev_err(pc->dev, "%s: missing pins property\n", of_node_full_name(np));
		return -EINVAL;
	}
	num_pins = pins->length / sizeof(pin);

	/* Parse any pinconfigs present */
	all_configs = kzalloc(num_pins * sizeof(unsigned long*), GFP_KERNEL);
	if (!all_configs)
		return -ENOMEM;
	err = bcmbca_pctl_parse_dt_config(np, num_pins, pctldev, all_configs,
					      &num_configs_per_pin);
	if (err < 0) {
		dev_err(pc->dev, "%s: could not parse pinconfigs property\n", of_node_full_name(np));
		goto out;
	}
	/* If we found pinconfigs, we have to allocate maps for them */
	if(num_configs_per_pin)
		num_maps_per_pin++;

	/* Look for pinmux setting 'function' */
	funcs = of_find_property(np, "function", NULL);
	if(funcs) {
		num_funcs = funcs->length / sizeof(func);
		num_maps_per_pin++;
	} else {
		/* IF no pinmux setting and no pinconf we error out */
		if (!num_configs_per_pin) {
			dev_err(pc->dev, "%s: function and pinconfig not specified\n", of_node_full_name(np));
			err = -EINVAL;
			goto out;
		}
	}

	/* If multiple pinmux settings present, they should align with number of pins in group */
	if (num_funcs > 1 && num_funcs != num_pins) {
		dev_err(pc->dev, "%s: function must have 1 or %d entries\n", of_node_full_name(np), num_pins);
		err = -EINVAL;
		goto out;
	}

	/* Number of maps is multiplied by each config type present */
	cur_map = maps = kzalloc(num_pins * num_maps_per_pin * sizeof(*maps), GFP_KERNEL);
	if (!maps)
	{
		err = -ENOMEM;
		goto out;
	}

	for (i = 0; i < num_pins; i++) {
		err = of_property_read_u32_index(np, "pins", i, &pin);
		if (err)
			goto out;
		if (pin >= pc->desc->npins) {
			dev_err(pc->dev, "%s: invalid pins value %d (max=%d)\n", of_node_full_name(np), pin, pc->desc->npins - 1);
			err = -EINVAL;
			goto out;
		}
	
		/* Handle pinmux */
		if (num_funcs) {
			err = of_property_read_u32_index(np, "function", 
				(num_funcs > 1) ? i : 0, &func);
			if (err)
				goto out;
			err = bcmbca_pctl_dt_node_to_map_func(pc, np, pin, func, &cur_map);
			if (err)
				goto out;
		}

		if(num_configs_per_pin) {
			err = bcmbca_pctl_dt_node_to_map_pin_config(pc, np, pin, 
				all_configs[i], num_configs_per_pin, &cur_map);
		}
	}

	*map = maps;
	*num_maps = num_pins * num_maps_per_pin;
	
	if(all_configs)
		kfree(all_configs);

	return 0;

out:
	if(all_configs)
		kfree(all_configs);
	if(maps)
		kfree(maps);
	return err;
}

static const struct pinctrl_ops bcmbca_pctl_ops = {
	.get_groups_count = bcmbca_pctl_get_groups_count,
	.get_group_name = bcmbca_pctl_get_group_name,
	.get_group_pins = bcmbca_pctl_get_group_pins,
	.pin_dbg_show = bcmbca_pctl_pin_dbg_show,
	.dt_node_to_map = bcmbca_pctl_dt_node_to_map,
	.dt_free_map = bcmbca_pctl_dt_free_map,
};

static int bcmbca_pmx_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	return pc->nfuncs;
}

static const char *bcmbca_pmx_get_function_name(struct pinctrl_dev *pctldev, unsigned selector)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	return pc->functions_names[selector].name;
}

static int bcmbca_pmx_get_function_groups(struct pinctrl_dev *pctldev, unsigned selector, const char * const **groups,
	unsigned * const num_groups)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

	*groups = (const char * const *)pc->bcmbca_groups_names;
	*num_groups = pc->ngroups;

	return 0;
}

static int bcmbca_pmx_set(struct pinctrl_dev *pctldev, unsigned func_selector, unsigned group_selector)
{
#define LOAD_MUX_REG_CMD        0x21
#define PINMUX_DATA_SHIFT       12

	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	volatile u32 __iomem *test_port_block_data_msb = pc->base;
	volatile u32 __iomem *test_port_block_data_lsb = test_port_block_data_msb + 1;
	volatile u32 __iomem *test_port_cmd            = test_port_block_data_lsb + 1;
	u32 tp_blk_data_lsb;


	tp_blk_data_lsb = group_selector;
    tp_blk_data_lsb |= (func_selector << PINMUX_DATA_SHIFT);
    *test_port_block_data_msb = 0;
    *test_port_block_data_lsb = tp_blk_data_lsb;
    *test_port_cmd = LOAD_MUX_REG_CMD;

	return 0;
}

static int bcmbca_gpio_request(struct pinctrl_dev *pctldev, struct pinctrl_gpio_range *range, unsigned offset)
{
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);

    return bcmbca_pmx_set(pctldev, pc->gpio_mux, offset);
}

static const struct pinmux_ops bcmbca_pmx_ops = {
	.get_functions_count = bcmbca_pmx_get_functions_count,
	.get_function_name = bcmbca_pmx_get_function_name,
	.get_function_groups = bcmbca_pmx_get_function_groups,
	.set_mux = bcmbca_pmx_set,
    .gpio_request_enable = bcmbca_gpio_request,
};

static int bcmbca_pin_sel_config_get(struct pinctrl_dev *pctldev,
					   unsigned pin,
					   u32 *sel_config)
{
#define READ_SEL_REG_CMD	0x23
#define SEL_DATA_R_SHIFT	16 
#define SEL_MASK		0x3F	
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	volatile u32 __iomem *test_port_block_data_msb = pc->base;
	volatile u32 __iomem *test_port_block_data_lsb = test_port_block_data_msb + 1;
	volatile u32 __iomem *test_port_cmd            = test_port_block_data_lsb + 1;
	volatile u32 __iomem *test_port_read_back      = test_port_cmd + 1;
	u32 tp_blk_data_lsb;

	/* Get sel settings for pin */
	tp_blk_data_lsb = pin;
	*test_port_block_data_msb = 0;
	*test_port_block_data_lsb = tp_blk_data_lsb;
	*test_port_cmd = READ_SEL_REG_CMD;
	msleep(10);

	/* mask off and return sel values */
	*sel_config = (*test_port_read_back >> SEL_DATA_R_SHIFT) & SEL_MASK;

	return 0;
}

static int bcmbca_pin_sel_config_set(struct pinctrl_dev *pctldev,
					   unsigned pin,
					   u32 sel_config)
{
#define LOAD_SEL_REG_CMD	0x22
#define SEL_DATA_W_SHIFT	12
	struct bcmbca_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	volatile u32 __iomem *test_port_block_data_msb = pc->base;
	volatile u32 __iomem *test_port_block_data_lsb = test_port_block_data_msb + 1;
	volatile u32 __iomem *test_port_cmd            = test_port_block_data_lsb + 1;
	u32 tp_blk_data_lsb;

	/* Get sel settings for pin */
	tp_blk_data_lsb = pin;
	*test_port_block_data_msb = 0;
	*test_port_block_data_lsb = (sel_config << SEL_DATA_W_SHIFT) |tp_blk_data_lsb;
	*test_port_cmd = LOAD_SEL_REG_CMD;
	msleep(10);

	return 0;
}

static int bcmbca_group_config_get(struct pinctrl_dev *pctldev,
					   unsigned group,
					   unsigned long *config)
{
	return -ENOTSUPP;
}

static int bcmbca_group_config_set(struct pinctrl_dev *pctldev,
					   unsigned group,
					   unsigned long *configs,
					   unsigned num_configs)
{
#define SEL_DRV_STR_MASK	0x7
	u32 drive_strength, sel_config;
	int i;

 	for(i=0; i<num_configs; i++) {
		switch(pinconf_to_config_param(configs[i])) {
			case PIN_CONFIG_DRIVE_STRENGTH:
				/* retrieve and map drive strength */
				drive_strength = pinconf_to_config_argument(configs[i]);
				drive_strength = drive_strength/2 - 1;

				/* retrieve existing pin settings */
				bcmbca_pin_sel_config_get(pctldev, group, &sel_config);

				/* commit new drive strength setting */
				sel_config &= ~SEL_DRV_STR_MASK;
				sel_config |= drive_strength;
				
				/* set pin config */
				bcmbca_pin_sel_config_set(pctldev, group, sel_config);
			break;

			default:
			break;
		}
	}	

	return 0;
}

static const struct pinconf_ops bcmbca_conf_ops = {
	.pin_config_group_get = bcmbca_group_config_get,
	.pin_config_group_set = bcmbca_group_config_set,
};

static struct pinctrl_desc bcmbca_pinctrl_desc = {
	.name = MODULE_NAME,
	.pctlops = &bcmbca_pctl_ops,
	.pmxops = &bcmbca_pmx_ops,
	.confops = &bcmbca_conf_ops,
	.owner = THIS_MODULE,
};


#define DEVTREE_GET_U32(name, address)					\
	do {												\
		err = of_property_read_u32(np, name, address);	\
		if (err) {										\
			dev_err(pc->dev, "Can't find %s\n", name);	\
			goto out;									\
		}												\
	} while (0);


static const struct of_device_id bcmbca_pinctrl_match[];


static int bcmbca_pinctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcmbca_pinctrl *pc;
	struct resource iomem;
	int err;

	pc = devm_kzalloc(dev, sizeof(*pc), GFP_KERNEL);
	if (!pc) {
		dev_err(dev, "Memory allocation failure\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, pc);
	pc->dev = dev;
	pc->bcmbca_groups_names = NULL;
	pc->desc = &bcmbca_pinctrl_desc;
	DEVTREE_GET_U32("pinctrl-npins", &pc->desc->npins);
	DEVTREE_GET_U32("pinctrl-nfuncs", &pc->nfuncs);
	DEVTREE_GET_U32("gpio-mux", &pc->gpio_mux);
	
	/* 
	 * In our implementation we only support single pin groups i.e Number of groups
	 * and number of pins are equal. This means one group per pin and thus we have
	 * to use 'group' based accessors and map group<->pin internally. 
	 */
	pc->ngroups = pc->desc->npins; 

	err = bcmbca_create_pinctrl_tables(pc);
	if (0 > err) {
		goto out;
	}

	err = of_address_to_resource(np, 0, &iomem);
	if (err) {
		dev_err(dev, "Failure in getting IO memory\n");
		goto out;
	}

	pc->base = devm_ioremap_resource(dev, &iomem);
	if (IS_ERR(pc->base)) {
		dev_err(dev, "Failure in IO remap\n");
		err = PTR_ERR(pc->base);
		goto out;
	}

	pc->pctl_dev = pinctrl_register(&bcmbca_pinctrl_desc, dev, pc);
	if (!pc->pctl_dev) {
		err = -EINVAL;
		goto out;
	}

	return 0;

out:
	FREE_KZALLOCATION(pc);
	return err;
}

static int bcmbca_create_pinctrl_tables(struct bcmbca_pinctrl *pc)
{
	int i;

	pc->groups_names = devm_kzalloc(pc->dev, pc->ngroups * sizeof(struct bcmbca_pinctrl_group_name), GFP_KERNEL);
	if (!pc->groups_names) {
		dev_err(pc->dev, "Memory allocation for bcmbca pinctrl groups names failure\n");
		goto out;
	}

	pc->functions_names = devm_kzalloc(pc->dev, pc->nfuncs * sizeof(struct bcmbca_pinctrl_function_name), GFP_KERNEL);
	if (!pc->functions_names) {
		dev_err(pc->dev, "Memory allocation for bcmbca pinctrl functions names failure\n");
		goto out;
	}

	pc->pins_descriptors = devm_kzalloc(pc->dev, pc->desc->npins * sizeof(struct pinctrl_pin_desc), GFP_KERNEL);
	if (!pc->pins_descriptors) {
		dev_err(pc->dev, "Memory allocation for bcmbca pinctrl pins descriptors failure\n");
		goto out;
	}

	pc->bcmbca_groups_names = devm_kzalloc(pc->dev, pc->ngroups * sizeof(char *), GFP_KERNEL);
	if (!pc->bcmbca_groups_names) {
		dev_err(pc->dev, "Memory allocation for bcmbca_groups_names array failure\n");
		goto out;
	}

	for (i = 0; i < pc->desc->npins; i++) {
		pc->pins_descriptors[i].number = i;

		/* 
		 * We dont assign pin names, therefore none of the 'pin' based accessors will function as they
		 * need to lookup and match pin names. We use a strictly 'group' based access
		 */
		pc->pins_descriptors[i].name = NULL;
		pc->pins_descriptors[i].drv_data = NULL;

		/* We map our local pin numbers to a group name */
		sprintf(pc->groups_names[i].name, "BCMBCA_PINCTRL_GROUP_%03d", i);
		pc->bcmbca_groups_names[i] = pc->groups_names[i].name;
	}

	pc->desc->pins = pc->pins_descriptors;

	for (i = 0; i < pc->nfuncs; i++) {
		sprintf(pc->functions_names[i].name, "BCMBCA_PINCTRL_FUNCTION_%02d", i);
	}

	return 0;

out:
	FREE_KZALLOCATION(pc->bcmbca_groups_names);
	FREE_KZALLOCATION(pc->pins_descriptors);
	FREE_KZALLOCATION(pc->functions_names);
	FREE_KZALLOCATION(pc->groups_names);

	return -ENOMEM;
}

static int bcmbca_pinctrl_remove(struct platform_device *pdev)
{
	struct bcmbca_pinctrl *pc = platform_get_drvdata(pdev);

	FREE_KZALLOCATION(pc->bcmbca_groups_names);
	FREE_KZALLOCATION(pc->pins_descriptors);
	FREE_KZALLOCATION(pc->functions_names);
	FREE_KZALLOCATION(pc->groups_names);
	pinctrl_unregister(pc->pctl_dev);

	return 0;
}

static const struct of_device_id bcmbca_pinctrl_match[] = {
	{ .compatible = "brcm,bcmbca-pinctrl" },
	{}
};
MODULE_DEVICE_TABLE(of, bcmbca_pinctrl_match);

static struct platform_driver bcmbca_pinctrl_driver = {
	.probe = bcmbca_pinctrl_probe,
	.remove = bcmbca_pinctrl_remove,
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = bcmbca_pinctrl_match,
	},
};

static int __init bcmbca_pinctrl_drv_reg(void)
{
	return platform_driver_register(&bcmbca_pinctrl_driver);
}

postcore_initcall(bcmbca_pinctrl_drv_reg);

MODULE_DESCRIPTION("BCMBCA Pin control driver");
MODULE_LICENSE("GPL");

int bcmbca_pinctrl_get_pins_by_state(struct pinctrl_state *state, int *pins, int *num_pins)
{
    struct pinctrl_setting *setting;
	struct pinctrl_dev *pctldev;
	const struct pinctrl_ops *pctlops;
	const unsigned *_pins;
	unsigned _num_pins = 0;
	int ret = 0;
    int i,j = 0;

	list_for_each_entry(setting, &state->settings, node) 
    {
        pctldev = setting->pctldev;
        pctlops = pctldev->desc->pctlops;
        _pins = NULL;
        _num_pins = 0;
        if (pctlops->get_group_pins)
            ret = pctlops->get_group_pins(pctldev, setting->data.mux.group, &_pins, &_num_pins);
        
        if (ret) 
        {
            const char *gname;

            /* errors only affect debug data, so just warn */
            gname = pctlops->get_group_name(pctldev, setting->data.mux.group);
            dev_warn(pctldev->dev, "could not get pins for group %s\n", gname);
            _num_pins = 0;
        }

        if (j == *num_pins && _num_pins != 0)
        {
            dev_err(pctldev->dev, "Number of requested Pins are bigger than allocated\n");
            return -ENOMEM;
        }

        for (i = 0; i<_num_pins && j < *num_pins; i++, j++)
        {
            pins[j] = _pins[i];
        }
    }

    *num_pins = j;
    return 0;
}
EXPORT_SYMBOL(bcmbca_pinctrl_get_pins_by_state);
