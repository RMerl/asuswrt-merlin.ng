// SPDX-License-Identifier: GPL-2.0
/*
 * Generic DWC3 Glue layer
 *
 * Copyright (C) 2016 - 2018 Xilinx, Inc.
 *
 * Based on dwc3-omap.c.
 */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dwc3-uboot.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <malloc.h>
#include <usb.h>
#include "core.h"
#include "gadget.h"
#include <reset.h>
#include <clk.h>
#include <usb/xhci.h>
#include <asm/gpio.h>

#include "dwc3-generic.h"

struct dwc3_generic_plat {
	fdt_addr_t base;
	u32 maximum_speed;
	enum usb_dr_mode dr_mode;
};

struct dwc3_generic_priv {
	void *base;
	struct dwc3 dwc3;
	struct phy_bulk phys;
	struct gpio_desc *ulpi_reset;
};

struct dwc3_generic_host_priv {
	struct xhci_ctrl xhci_ctrl;
	struct dwc3_generic_priv gen_priv;
};

static int dwc3_generic_probe(struct udevice *dev,
			      struct dwc3_generic_priv *priv)
{
	int rc;
	struct dwc3_generic_plat *plat = dev_get_plat(dev);
	struct dwc3 *dwc3 = &priv->dwc3;
	struct dwc3_glue_data *glue = dev_get_plat(dev->parent);
	int __maybe_unused index;
	ofnode __maybe_unused node;

	dwc3->dev = dev;
	dwc3->maximum_speed = plat->maximum_speed;
	dwc3->dr_mode = plat->dr_mode;
#if CONFIG_IS_ENABLED(OF_CONTROL)
	dwc3_of_parse(dwc3);

	/*
	 * There are currently four disparate placement possibilities of DWC3
	 * reference clock phandle in SoC DTs:
	 * - in top level glue node, with generic subnode without clock (ZynqMP)
	 * - in top level generic node, with no subnode (i.MX8MQ)
	 * - in generic subnode, with other clock in top level node (i.MX8MP)
	 * - in both top level node and generic subnode (Rockchip)
	 * Cover all the possibilities here by looking into both nodes, start
	 * with the top level node as that seems to be used in majority of DTs
	 * to reference the clock.
	 */
	node = dev_ofnode(dev->parent);
	index = ofnode_stringlist_search(node, "clock-names", "ref");
	if (index < 0)
		index = ofnode_stringlist_search(node, "clock-names", "ref_clk");
	if (index < 0) {
		node = dev_ofnode(dev);
		index = ofnode_stringlist_search(node, "clock-names", "ref");
		if (index < 0)
			index = ofnode_stringlist_search(node, "clock-names", "ref_clk");
	}
	if (index >= 0)
		dwc3->ref_clk = &glue->clks.clks[index];
#endif

	/*
	 * It must hold whole USB3.0 OTG controller in resetting to hold pipe
	 * power state in P2 before initializing TypeC PHY on RK3399 platform.
	 */
	if (device_is_compatible(dev->parent, "rockchip,rk3399-dwc3")) {
		reset_assert_bulk(&glue->resets);
		udelay(1);
	}

	rc = dwc3_setup_phy(dev, &priv->phys);
	if (rc && rc != -ENOTSUPP)
		return rc;

	if (CONFIG_IS_ENABLED(DM_GPIO) &&
	    device_is_compatible(dev->parent, "xlnx,zynqmp-dwc3")) {
		priv->ulpi_reset = devm_gpiod_get_optional(dev->parent, "reset",
							   GPIOD_IS_OUT | GPIOD_ACTIVE_LOW);
		/* property is optional, don't return error! */
		if (priv->ulpi_reset) {
			/* Toggle ulpi to reset the phy. */
			rc = dm_gpio_set_value(priv->ulpi_reset, 1);
			if (rc)
				return rc;

			mdelay(5);

			rc = dm_gpio_set_value(priv->ulpi_reset, 0);
			if (rc)
				return rc;

			mdelay(5);
		}
	}

	if (device_is_compatible(dev->parent, "rockchip,rk3399-dwc3"))
		reset_deassert_bulk(&glue->resets);

	priv->base = map_physmem(plat->base, DWC3_OTG_REGS_END, MAP_NOCACHE);
	dwc3->regs = priv->base + DWC3_GLOBALS_REGS_START;


	rc =  dwc3_init(dwc3);
	if (rc) {
		unmap_physmem(priv->base, MAP_NOCACHE);
		return rc;
	}

	return 0;
}

static int dwc3_generic_remove(struct udevice *dev,
			       struct dwc3_generic_priv *priv)
{
	struct dwc3 *dwc3 = &priv->dwc3;

	if (CONFIG_IS_ENABLED(DM_GPIO) &&
	    device_is_compatible(dev->parent, "xlnx,zynqmp-dwc3") &&
	    priv->ulpi_reset) {
		struct gpio_desc *ulpi_reset = priv->ulpi_reset;

		dm_gpio_free(ulpi_reset->dev, ulpi_reset);
	}

	dwc3_remove(dwc3);
	dwc3_shutdown_phy(dev, &priv->phys);
	unmap_physmem(dwc3->regs, MAP_NOCACHE);

	return 0;
}

static int dwc3_generic_of_to_plat(struct udevice *dev)
{
	struct dwc3_generic_plat *plat = dev_get_plat(dev);
	ofnode node = dev_ofnode(dev);

	if (!strncmp(dev->name, "port", 4) || !strncmp(dev->name, "hub", 3)) {
		/* This is a leaf so check the parent */
		plat->base = dev_read_addr(dev->parent);
	} else {
		plat->base = dev_read_addr(dev);
	}

	plat->maximum_speed = usb_get_maximum_speed(node);
	if (plat->maximum_speed == USB_SPEED_UNKNOWN) {
		pr_info("No USB maximum speed specified. Using super speed\n");
		plat->maximum_speed = USB_SPEED_SUPER;
	}

	plat->dr_mode = usb_get_dr_mode(node);
	if (plat->dr_mode == USB_DR_MODE_UNKNOWN) {
		/* might be a leaf so check the parent for mode */
		node = dev_ofnode(dev->parent);
		plat->dr_mode = usb_get_dr_mode(node);
		if (plat->dr_mode == USB_DR_MODE_UNKNOWN) {
			pr_err("Invalid usb mode setup\n");
			return -ENODEV;
		}
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB_GADGET)
int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct dwc3_generic_priv *priv = dev_get_priv(dev);
	struct dwc3 *dwc3 = &priv->dwc3;

	dwc3_gadget_uboot_handle_interrupt(dwc3);

	return 0;
}

static int dwc3_generic_peripheral_probe(struct udevice *dev)
{
	struct dwc3_generic_priv *priv = dev_get_priv(dev);

	return dwc3_generic_probe(dev, priv);
}

static int dwc3_generic_peripheral_remove(struct udevice *dev)
{
	struct dwc3_generic_priv *priv = dev_get_priv(dev);

	return dwc3_generic_remove(dev, priv);
}

U_BOOT_DRIVER(dwc3_generic_peripheral) = {
	.name	= "dwc3-generic-peripheral",
	.id	= UCLASS_USB_GADGET_GENERIC,
	.of_to_plat = dwc3_generic_of_to_plat,
	.probe = dwc3_generic_peripheral_probe,
	.remove = dwc3_generic_peripheral_remove,
	.priv_auto	= sizeof(struct dwc3_generic_priv),
	.plat_auto	= sizeof(struct dwc3_generic_plat),
};
#endif

#if CONFIG_IS_ENABLED(USB_HOST)
static int dwc3_generic_host_probe(struct udevice *dev)
{
	struct xhci_hcor *hcor;
	struct xhci_hccr *hccr;
	struct dwc3_generic_host_priv *priv = dev_get_priv(dev);
	int rc;

	rc = dwc3_generic_probe(dev, &priv->gen_priv);
	if (rc)
		return rc;

	hccr = (struct xhci_hccr *)priv->gen_priv.base;
	hcor = (struct xhci_hcor *)(priv->gen_priv.base +
			HC_LENGTH(xhci_readl(&(hccr)->cr_capbase)));

	return xhci_register(dev, hccr, hcor);
}

static int dwc3_generic_host_remove(struct udevice *dev)
{
	struct dwc3_generic_host_priv *priv = dev_get_priv(dev);
	int rc;

	rc = xhci_deregister(dev);
	if (rc)
		return rc;

	return dwc3_generic_remove(dev, &priv->gen_priv);
}

U_BOOT_DRIVER(dwc3_generic_host) = {
	.name	= "dwc3-generic-host",
	.id	= UCLASS_USB,
	.of_to_plat = dwc3_generic_of_to_plat,
	.probe = dwc3_generic_host_probe,
	.remove = dwc3_generic_host_remove,
	.priv_auto	= sizeof(struct dwc3_generic_host_priv),
	.plat_auto	= sizeof(struct dwc3_generic_plat),
	.ops = &xhci_usb_ops,
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif

void dwc3_imx8mp_glue_configure(struct udevice *dev, int index,
				enum usb_dr_mode mode)
{
/* USB glue registers */
#define USB_CTRL0		0x00
#define USB_CTRL1		0x04

#define USB_CTRL0_PORTPWR_EN	BIT(12) /* 1 - PPC enabled (default) */
#define USB_CTRL0_USB3_FIXED	BIT(22) /* 1 - USB3 permanent attached */
#define USB_CTRL0_USB2_FIXED	BIT(23) /* 1 - USB2 permanent attached */

#define USB_CTRL1_OC_POLARITY	BIT(16) /* 0 - HIGH / 1 - LOW */
#define USB_CTRL1_PWR_POLARITY	BIT(17) /* 0 - HIGH / 1 - LOW */
	fdt_addr_t regs = dev_read_addr_index(dev, 1);
	void *base = map_physmem(regs, 0x8, MAP_NOCACHE);
	u32 value;

	value = readl(base + USB_CTRL0);

	if (dev_read_bool(dev, "fsl,permanently-attached"))
		value |= (USB_CTRL0_USB2_FIXED | USB_CTRL0_USB3_FIXED);
	else
		value &= ~(USB_CTRL0_USB2_FIXED | USB_CTRL0_USB3_FIXED);

	if (dev_read_bool(dev, "fsl,disable-port-power-control"))
		value &= ~(USB_CTRL0_PORTPWR_EN);
	else
		value |= USB_CTRL0_PORTPWR_EN;

	writel(value, base + USB_CTRL0);

	value = readl(base + USB_CTRL1);
	if (dev_read_bool(dev, "fsl,over-current-active-low"))
		value |= USB_CTRL1_OC_POLARITY;
	else
		value &= ~USB_CTRL1_OC_POLARITY;

	if (dev_read_bool(dev, "fsl,power-active-low"))
		value |= USB_CTRL1_PWR_POLARITY;
	else
		value &= ~USB_CTRL1_PWR_POLARITY;

	writel(value, base + USB_CTRL1);

	unmap_physmem(base, MAP_NOCACHE);
}

struct dwc3_glue_ops imx8mp_ops = {
	.glue_configure = dwc3_imx8mp_glue_configure,
};

void dwc3_ti_glue_configure(struct udevice *dev, int index,
			    enum usb_dr_mode mode)
{
#define USBOTGSS_UTMI_OTG_STATUS		0x0084
#define USBOTGSS_UTMI_OTG_OFFSET		0x0480

/* UTMI_OTG_STATUS REGISTER */
#define USBOTGSS_UTMI_OTG_STATUS_SW_MODE	BIT(31)
#define USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT	BIT(9)
#define USBOTGSS_UTMI_OTG_STATUS_TXBITSTUFFENABLE BIT(8)
#define USBOTGSS_UTMI_OTG_STATUS_IDDIG		BIT(4)
#define USBOTGSS_UTMI_OTG_STATUS_SESSEND	BIT(3)
#define USBOTGSS_UTMI_OTG_STATUS_SESSVALID	BIT(2)
#define USBOTGSS_UTMI_OTG_STATUS_VBUSVALID	BIT(1)
enum dwc3_omap_utmi_mode {
	DWC3_OMAP_UTMI_MODE_UNKNOWN = 0,
	DWC3_OMAP_UTMI_MODE_HW,
	DWC3_OMAP_UTMI_MODE_SW,
};

	u32 use_id_pin;
	u32 host_mode;
	u32 reg;
	u32 utmi_mode;
	u32 utmi_status_offset = USBOTGSS_UTMI_OTG_STATUS;

	struct dwc3_glue_data *glue = dev_get_plat(dev);
	void *base = map_physmem(glue->regs, 0x10000, MAP_NOCACHE);

	if (device_is_compatible(dev, "ti,am437x-dwc3"))
		utmi_status_offset += USBOTGSS_UTMI_OTG_OFFSET;

	utmi_mode = dev_read_u32_default(dev, "utmi-mode",
					 DWC3_OMAP_UTMI_MODE_UNKNOWN);
	if (utmi_mode != DWC3_OMAP_UTMI_MODE_HW) {
		debug("%s: OTG is not supported. defaulting to PERIPHERAL\n",
		      dev->name);
		mode = USB_DR_MODE_PERIPHERAL;
	}

	switch (mode)  {
	case USB_DR_MODE_PERIPHERAL:
		use_id_pin = 0;
		host_mode = 0;
		break;
	case USB_DR_MODE_HOST:
		use_id_pin = 0;
		host_mode = 1;
		break;
	case USB_DR_MODE_OTG:
	default:
		use_id_pin = 1;
		host_mode = 0;
		break;
	}

	reg = readl(base + utmi_status_offset);

	reg &= ~(USBOTGSS_UTMI_OTG_STATUS_SW_MODE);
	if (!use_id_pin)
		reg |= USBOTGSS_UTMI_OTG_STATUS_SW_MODE;

	writel(reg, base + utmi_status_offset);

	reg &= ~(USBOTGSS_UTMI_OTG_STATUS_SESSEND |
		USBOTGSS_UTMI_OTG_STATUS_VBUSVALID |
		USBOTGSS_UTMI_OTG_STATUS_IDDIG);

	reg |= USBOTGSS_UTMI_OTG_STATUS_SESSVALID |
		USBOTGSS_UTMI_OTG_STATUS_POWERPRESENT;

	if (!host_mode)
		reg |= USBOTGSS_UTMI_OTG_STATUS_IDDIG |
			USBOTGSS_UTMI_OTG_STATUS_VBUSVALID;

	writel(reg, base + utmi_status_offset);

	unmap_physmem(base, MAP_NOCACHE);
}

struct dwc3_glue_ops ti_ops = {
	.glue_configure = dwc3_ti_glue_configure,
};

static int dwc3_rk_glue_get_ctrl_dev(struct udevice *dev, ofnode *node)
{
	*node = dev_ofnode(dev);
	if (!ofnode_valid(*node))
		return -EINVAL;

	return 0;
}

struct dwc3_glue_ops rk_ops = {
	.glue_get_ctrl_dev = dwc3_rk_glue_get_ctrl_dev,
};

static int dwc3_glue_bind_common(struct udevice *parent, ofnode node)
{
	const char *name = ofnode_get_name(node);
	const char *driver;
	enum usb_dr_mode dr_mode;
	struct udevice *dev;
	int ret;

	debug("%s: subnode name: %s\n", __func__, name);

	/* if the parent node doesn't have a mode check the leaf */
	dr_mode = usb_get_dr_mode(dev_ofnode(parent));
	if (!dr_mode)
		dr_mode = usb_get_dr_mode(node);

	if (CONFIG_IS_ENABLED(DM_USB_GADGET) &&
	    (dr_mode == USB_DR_MODE_PERIPHERAL || dr_mode == USB_DR_MODE_OTG)) {
		debug("%s: dr_mode: OTG or Peripheral\n", __func__);
		driver = "dwc3-generic-peripheral";
	} else if (CONFIG_IS_ENABLED(USB_HOST) && dr_mode == USB_DR_MODE_HOST) {
		debug("%s: dr_mode: HOST\n", __func__);
		driver = "dwc3-generic-host";
	} else {
		debug("%s: unsupported dr_mode %d\n", __func__, dr_mode);
		return -ENODEV;
	}

	ret = device_bind_driver_to_node(parent, driver, name,
					 node, &dev);
	if (ret) {
		debug("%s: not able to bind usb device mode\n",
		      __func__);
		return ret;
	}

	return 0;
}

int dwc3_glue_bind(struct udevice *parent)
{
	struct dwc3_glue_ops *ops = (struct dwc3_glue_ops *)dev_get_driver_data(parent);
	ofnode node;
	int ret;

	if (ops && ops->glue_get_ctrl_dev) {
		ret = ops->glue_get_ctrl_dev(parent, &node);
		if (ret)
			return ret;

		return dwc3_glue_bind_common(parent, node);
	}

	ofnode_for_each_subnode(node, dev_ofnode(parent)) {
		ret = dwc3_glue_bind_common(parent, node);
		if (ret == -ENXIO)
			continue;
		if (ret)
			return ret;
	}

	return 0;
}

static int dwc3_glue_reset_init(struct udevice *dev,
				struct dwc3_glue_data *glue)
{
	int ret;

	ret = reset_get_bulk(dev, &glue->resets);
	if (ret == -ENOTSUPP || ret == -ENOENT)
		return 0;
	else if (ret)
		return ret;

	ret = reset_deassert_bulk(&glue->resets);
	if (ret) {
		reset_release_bulk(&glue->resets);
		return ret;
	}

	return 0;
}

static int dwc3_glue_clk_init(struct udevice *dev,
			      struct dwc3_glue_data *glue)
{
	int ret;

	ret = clk_get_bulk(dev, &glue->clks);
	if (ret == -ENOSYS || ret == -ENOENT)
		return 0;
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable_bulk(&glue->clks);
	if (ret) {
		clk_release_bulk(&glue->clks);
		return ret;
	}
#endif

	return 0;
}

int dwc3_glue_probe(struct udevice *dev)
{
	struct dwc3_glue_ops *ops = (struct dwc3_glue_ops *)dev_get_driver_data(dev);
	struct dwc3_glue_data *glue = dev_get_plat(dev);
	struct udevice *child = NULL;
	int index = 0;
	int ret;
	struct phy phy;

	ret = generic_phy_get_by_name(dev, "usb3-phy", &phy);
	if (!ret) {
		ret = generic_phy_init(&phy);
		if (ret)
			return ret;
	} else if (ret != -ENOENT && ret != -ENODATA) {
		debug("could not get phy (err %d)\n", ret);
		return ret;
	}

	glue->regs = dev_read_addr_size_index(dev, 0, &glue->size);

	ret = dwc3_glue_clk_init(dev, glue);
	if (ret)
		return ret;

	ret = dwc3_glue_reset_init(dev, glue);
	if (ret)
		return ret;

	if (generic_phy_valid(&phy)) {
		ret = generic_phy_power_on(&phy);
		if (ret)
			return ret;
	}

	device_find_first_child(dev, &child);
	if (!child)
		return 0;

	if (glue->clks.count == 0) {
		ret = dwc3_glue_clk_init(child, glue);
		if (ret)
			return ret;
	}

	if (glue->resets.count == 0) {
		ret = dwc3_glue_reset_init(child, glue);
		if (ret)
			return ret;
	}

	while (child) {
		enum usb_dr_mode dr_mode;

		dr_mode = usb_get_dr_mode(dev_ofnode(child));
		device_find_next_child(&child);
		if (ops && ops->glue_configure)
			ops->glue_configure(dev, index, dr_mode);
		index++;
	}

	return 0;
}

int dwc3_glue_remove(struct udevice *dev)
{
	struct dwc3_glue_data *glue = dev_get_plat(dev);

	reset_release_bulk(&glue->resets);

	clk_release_bulk(&glue->clks);

	return 0;
}

static const struct udevice_id dwc3_glue_ids[] = {
	{ .compatible = "xlnx,zynqmp-dwc3" },
	{ .compatible = "xlnx,versal-dwc3" },
	{ .compatible = "ti,keystone-dwc3"},
	{ .compatible = "ti,dwc3", .data = (ulong)&ti_ops },
	{ .compatible = "ti,am437x-dwc3", .data = (ulong)&ti_ops },
	{ .compatible = "ti,am654-dwc3" },
	{ .compatible = "rockchip,rk3328-dwc3", .data = (ulong)&rk_ops },
	{ .compatible = "rockchip,rk3399-dwc3" },
	{ .compatible = "rockchip,rk3568-dwc3", .data = (ulong)&rk_ops },
	{ .compatible = "rockchip,rk3588-dwc3", .data = (ulong)&rk_ops },
	{ .compatible = "qcom,dwc3" },
	{ .compatible = "fsl,imx8mp-dwc3", .data = (ulong)&imx8mp_ops },
	{ .compatible = "fsl,imx8mq-dwc3" },
	{ .compatible = "intel,tangier-dwc3" },
	{ }
};

U_BOOT_DRIVER(dwc3_generic_wrapper) = {
	.name	= "dwc3-generic-wrapper",
	.id	= UCLASS_NOP,
	.of_match = dwc3_glue_ids,
	.bind = dwc3_glue_bind,
	.probe = dwc3_glue_probe,
	.remove = dwc3_glue_remove,
	.plat_auto	= sizeof(struct dwc3_glue_data),

};
