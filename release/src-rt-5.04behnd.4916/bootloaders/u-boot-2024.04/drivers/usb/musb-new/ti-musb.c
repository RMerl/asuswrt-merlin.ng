// SPDX-License-Identifier: GPL-2.0+
/*
 * MISC driver for TI MUSB Glue.
 *
 * (C) Copyright 2016
 *     Texas Instruments Incorporated, <www.ti.com>
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/printk.h>
#include <linux/usb/otg.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

#include <asm/io.h>
#include <asm/omap_musb.h>
#include "musb_uboot.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(DM_USB)
/* USB 2.0 PHY Control */
#define CM_PHY_PWRDN			(1 << 0)
#define CM_PHY_OTG_PWRDN		(1 << 1)
#define OTGVDET_EN			(1 << 19)
#define OTGSESSENDEN			(1 << 20)

#define AM335X_USB0_CTRL	0x0
#define AM335X_USB1_CTRL	0x8

static void ti_musb_set_phy_power(struct udevice *dev, u8 on)
{
	struct ti_musb_plat *plat = dev_get_plat(dev);

	if (!plat->ctrl_mod_base)
		return;

	if (on) {
		clrsetbits_le32(plat->ctrl_mod_base,
				CM_PHY_PWRDN | CM_PHY_OTG_PWRDN,
				OTGVDET_EN | OTGSESSENDEN);
	} else {
		clrsetbits_le32(plat->ctrl_mod_base, 0,
				CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	}
}

#if CONFIG_IS_ENABLED(OF_CONTROL)

static int ti_musb_get_usb_index(int node)
{
	const void *fdt = gd->fdt_blob;
	int i = 0;
	char path[64];
	const char *alias_path;
	char alias[16];

	fdt_get_path(fdt, node, path, sizeof(path));

	do {
		snprintf(alias, sizeof(alias), "usb%d", i);
		alias_path = fdt_get_alias(fdt, alias);
		if (alias_path == NULL) {
			debug("USB index not found\n");
			return -ENOENT;
		}

		if (!strcmp(path, alias_path))
			return i;

		i++;
	} while (alias_path);

	return -ENOENT;
}

static int ti_musb_of_to_plat(struct udevice *dev)
{
	struct ti_musb_plat *plat = dev_get_plat(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int phys;
	int ctrl_mod;
	int usb_index;
	struct musb_hdrc_config *musb_config;

	plat->base = devfdt_get_addr_index_ptr(dev, 1);

	phys = fdtdec_lookup_phandle(fdt, node, "phys");
	ctrl_mod = fdtdec_lookup_phandle(fdt, phys, "ti,ctrl_mod");
	plat->ctrl_mod_base = (void *)fdtdec_get_addr(fdt, ctrl_mod, "reg");
	usb_index = ti_musb_get_usb_index(node);
	switch (usb_index) {
	case 1:
		plat->ctrl_mod_base += AM335X_USB1_CTRL;
		break;
	case 0:
		plat->ctrl_mod_base += AM335X_USB0_CTRL;
		break;
	default:
		break;
	}

	musb_config = malloc(sizeof(struct musb_hdrc_config));
	memset(musb_config, 0, sizeof(struct musb_hdrc_config));

	musb_config->multipoint = fdtdec_get_int(fdt, node,
						 "mentor,multipoint", -1);
	if (musb_config->multipoint < 0) {
		pr_err("MUSB multipoint DT entry missing\n");
		return -ENOENT;
	}

	musb_config->dyn_fifo = 1;

	musb_config->num_eps = fdtdec_get_int(fdt, node, "mentor,num-eps",
					      -1);
	if (musb_config->num_eps < 0) {
		pr_err("MUSB num-eps DT entry missing\n");
		return -ENOENT;
	}

	musb_config->ram_bits = fdtdec_get_int(fdt, node, "mentor,ram-bits",
					       -1);
	if (musb_config->ram_bits < 0) {
		pr_err("MUSB ram-bits DT entry missing\n");
		return -ENOENT;
	}

	plat->plat.config = musb_config;

	plat->plat.power = fdtdec_get_int(fdt, node, "mentor,power", -1);
	if (plat->plat.power < 0) {
		pr_err("MUSB mentor,power DT entry missing\n");
		return -ENOENT;
	}

	plat->plat.platform_ops = &musb_dsps_ops;

	return 0;
}
#endif

static int ti_musb_host_probe(struct udevice *dev)
{
	struct musb_host_data *host = dev_get_priv(dev);
	struct ti_musb_plat *plat = dev_get_plat(dev);
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	priv->desc_before_addr = true;

	host->host = musb_init_controller(&plat->plat,
					  NULL,
					  plat->base);
	if (!host->host)
		return -EIO;

	ti_musb_set_phy_power(dev, 1);
	ret = musb_lowlevel_init(host);

	return ret;
}

static int ti_musb_host_remove(struct udevice *dev)
{
	struct musb_host_data *host = dev_get_priv(dev);

	musb_stop(host->host);
	ti_musb_set_phy_power(dev, 0);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int ti_musb_host_of_to_plat(struct udevice *dev)
{
	struct ti_musb_plat *plat = dev_get_plat(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = ti_musb_of_to_plat(dev);
	if (ret) {
		pr_err("plat dt parse error\n");
		return ret;
	}

	plat->plat.mode = MUSB_HOST;

	return 0;
}
#endif

U_BOOT_DRIVER(ti_musb_host) = {
	.name	= "ti-musb-host",
	.id	= UCLASS_USB,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_to_plat = ti_musb_host_of_to_plat,
#endif
	.probe = ti_musb_host_probe,
	.remove = ti_musb_host_remove,
	.ops	= &musb_usb_ops,
	.plat_auto	= sizeof(struct ti_musb_plat),
	.priv_auto	= sizeof(struct musb_host_data),
};

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
struct ti_musb_peripheral {
	struct musb *periph;
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int ti_musb_peripheral_of_to_plat(struct udevice *dev)
{
	struct ti_musb_plat *plat = dev_get_plat(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = ti_musb_of_to_plat(dev);
	if (ret) {
		pr_err("plat dt parse error\n");
		return ret;
	}
	plat->plat.mode = MUSB_PERIPHERAL;

	return 0;
}
#endif

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct ti_musb_peripheral *priv = dev_get_priv(dev);

	priv->periph->isr(0, priv->periph);

	return 0;
}

static int ti_musb_peripheral_probe(struct udevice *dev)
{
	struct ti_musb_peripheral *priv = dev_get_priv(dev);
	struct ti_musb_plat *plat = dev_get_plat(dev);
	int ret;

	priv->periph = musb_init_controller(&plat->plat,
					    NULL,
					    plat->base);
	if (!priv->periph)
		return -EIO;

	ti_musb_set_phy_power(dev, 1);
	musb_gadget_setup(priv->periph);
	return usb_add_gadget_udc((struct device *)dev, &priv->periph->g);
}

static int ti_musb_peripheral_remove(struct udevice *dev)
{
	struct ti_musb_peripheral *priv = dev_get_priv(dev);

	usb_del_gadget_udc(&priv->periph->g);
	ti_musb_set_phy_power(dev, 0);

	return 0;
}

U_BOOT_DRIVER(ti_musb_peripheral) = {
	.name	= "ti-musb-peripheral",
	.id	= UCLASS_USB_GADGET_GENERIC,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_to_plat = ti_musb_peripheral_of_to_plat,
#endif
	.probe = ti_musb_peripheral_probe,
	.remove = ti_musb_peripheral_remove,
	.ops	= &musb_usb_ops,
	.plat_auto	= sizeof(struct ti_musb_plat),
	.priv_auto	= sizeof(struct ti_musb_peripheral),
	.flags = DM_FLAG_PRE_RELOC,
};
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int ti_musb_wrapper_bind(struct udevice *parent)
{
	ofnode node;
	int ret;

	ofnode_for_each_subnode(node, dev_ofnode(parent)) {
		struct udevice *dev;
		const char *name = ofnode_get_name(node);
		enum usb_dr_mode dr_mode;
		struct driver *drv;

		if (strncmp(name, "usb@", 4))
			continue;

		dr_mode = usb_get_dr_mode(node);
		switch (dr_mode) {
		case USB_DR_MODE_PERIPHERAL:
			/* Bind MUSB device */
			ret = device_bind_driver_to_node(parent,
							 "ti-musb-peripheral",
							 name,
							 node,
							 &dev);
			if (ret)
				pr_err("musb - not able to bind usb peripheral node\n");
			break;
		case USB_DR_MODE_HOST:
			/* Bind MUSB host */
			ret = device_bind_driver_to_node(parent,
							 "ti-musb-host",
							 name,
							 node,
							 &dev);
			if (ret)
				pr_err("musb - not able to bind usb host node\n");
			break;
		default:
			break;
		};
	}
	return 0;
}

static const struct udevice_id ti_musb_ids[] = {
	{ .compatible = "ti,am33xx-usb" },
	{ }
};

U_BOOT_DRIVER(ti_musb_wrapper) = {
	.name	= "ti-musb-wrapper",
	.id	= UCLASS_MISC,
	.of_match = ti_musb_ids,
	.bind = ti_musb_wrapper_bind,
};
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

#endif /* CONFIG_IS_ENABLED(DM_USB) */
