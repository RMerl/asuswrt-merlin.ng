/*
 * Nexus SPI SHIM registration
 *
 * Copyright (C) 2017-2018, Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/module.h>

struct brcmstb_spi_controller {
	const char *compat;
	unsigned int max_cs;
};

static const struct brcmstb_spi_controller spi_ctls[] = {
	{
		.compat = "brcm,spi-brcmstb-mspi",
		.max_cs = 4,
	},
	{
		.compat = "brcm,bcm2835-spi",
		/* Maximum number of native CS */
		.max_cs = 2,
	},
};

static struct of_changeset brcmstb_spi_of_changeset;

static int __init brcmstb_spi_add_prop(struct of_changeset *ocs,
				       struct device_node *np,
				       const char *name, const void *value,
				       int length)
{
	struct property *prop;
	int ret = -ENOMEM;

	prop = kzalloc(sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return ret;

	prop->name = kstrdup(name, GFP_KERNEL);
	if (!prop->name)
		goto out_err;

	prop->value = kmemdup(value, length, GFP_KERNEL);
	if (!prop->value)
		goto out_err;

	of_property_set_flag(prop, OF_DYNAMIC);

	prop->length = length;

	ret = of_changeset_add_property(ocs, np, prop);
	if (!ret)
		return 0;

out_err:
	kfree(prop->value);
	kfree(prop->name);
	kfree(prop);
	return ret;
}

static struct device_node *brcmstb_spi_add_node(struct device_node *parent,
						const char *full_name)
{
	struct device_node *node;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return NULL;

	node->full_name = kstrdup(full_name, GFP_KERNEL);
	if (!node->full_name) {
		kfree(node);
		return NULL;
	}

	of_node_set_flag(node, OF_DYNAMIC);
	of_node_set_flag(node, OF_DETACHED);
#if defined(CONFIG_OF_KOBJ)
	kobject_init(&node->kobj, parent->kobj.ktype);
#endif
	fwnode_init(&node->fwnode, &of_fwnode_ops);

	return node;
}

static int __init brcmstb_register_spi_one(struct device_node *dn,
					   unsigned int max_cs)
{
	u32 addr, dt_enabled_cs = 0;
	struct device_node *child;
	unsigned int cs = 0;
	__be32 value;
	char buf[10];
	int ret;

	/* Scan for DT enabled SPI devices */
	for_each_available_child_of_node(dn, child) {
		ret = of_property_read_u32(child, "reg", &addr);
		if (ret)
			continue;

		dt_enabled_cs |= BIT(addr);
	}

	of_changeset_init(&brcmstb_spi_of_changeset);

	/* Populate SPI controller node with non DT enabled SPI devices */
	for (cs = 0; cs < max_cs; cs++) {
		/* Skip over existing DT enabled CS */
		if (BIT(cs) & dt_enabled_cs)
			continue;

		snprintf(buf, sizeof(buf), "spidev@%d", cs);
		child = brcmstb_spi_add_node(dn, buf);
		if (!child) {
			pr_err("%s: failed to duplicate node\n", __func__);
			continue;
		}

		child->parent = dn;
		ret = of_changeset_attach_node(&brcmstb_spi_of_changeset, child);
		if (ret) {
			pr_err("%s: failed to attach node: %d\n", __func__, ret);
			continue;
		}

		ret = brcmstb_spi_add_prop(&brcmstb_spi_of_changeset, child,
					   "compatible", "nexus_spi_shim",
					   strlen("nexus_spi_shim") + 1);
		if (ret) {
			pr_err("%s: failed to add compatible property: %d\n",
				__func__, ret);
			continue;
		}

		value = cpu_to_be32(cs);
		ret = brcmstb_spi_add_prop(&brcmstb_spi_of_changeset, child,
					   "reg", &value, sizeof(value));
		if (ret) {
			pr_err("%s: failed to add reg property: %d\n", __func__, ret);
			continue;
		}

		value = cpu_to_be32(13500000);
		ret = brcmstb_spi_add_prop(&brcmstb_spi_of_changeset, child,
					   "spi-max-frequency",
					   &value, sizeof(value));
		if (ret) {
			pr_err("%s: failed to add max-speed property: %d\n", __func__, ret);
			continue;
		}
	}

	ret = of_changeset_apply(&brcmstb_spi_of_changeset);
	if (ret)
		pr_err("%s: failed to apply OF changeset: %d\n",
			__func__, ret);
	return ret;
}

static int __init brcmstb_register_spi_devices(void)
{
	struct device_node *dn = NULL;
	unsigned int i;
	int ret = 0;
	int num_cs;

	for (i = 0; i < ARRAY_SIZE(spi_ctls); i++) {
		for_each_compatible_node(dn, NULL, spi_ctls[i].compat) {
			if (!of_device_is_available(dn))
				continue;

			/*
			 * for bcm2835 spi controller we register based on
			 * number of cs-gpios in the device node
			 */
			num_cs = of_gpio_named_count(dn, "cs-gpios");
			if (num_cs == -ENOENT)
				/* fallback to max_cs when no entry found */
				num_cs = spi_ctls[i].max_cs;
			else if (num_cs == -EINVAL)
				/* malformed cs-gpios property */
				continue;

			ret = brcmstb_register_spi_one(dn, num_cs);
			if (ret) {
				of_node_put(dn);
				return ret;
			}
		}
	}

	return ret;
}
#if IS_BUILTIN(CONFIG_BRCMSTB_NEXUS_SPI)
arch_initcall(brcmstb_register_spi_devices);
#else
module_init(brcmstb_register_spi_devices);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom SPI Nexus shim");
#endif
