// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/io.h>
#include <linux/libfdt.h>

#include "../gadget/dwc2_udc_otg_priv.h"

DECLARE_GLOBAL_DATA_PTR;

#define BIT_WRITEABLE_SHIFT	16

struct usb2phy_reg {
	unsigned int offset;
	unsigned int bitend;
	unsigned int bitstart;
	unsigned int disable;
	unsigned int enable;
};

/**
 * struct rockchip_usb2_phy_cfg: usb-phy port configuration
 * @port_reset: usb otg per-port reset register
 * @soft_con: software control usb otg register
 * @suspend: phy suspend register
 */
struct rockchip_usb2_phy_cfg {
	struct usb2phy_reg port_reset;
	struct usb2phy_reg soft_con;
	struct usb2phy_reg suspend;
};

struct rockchip_usb2_phy_dt_id {
	char		compatible[128];
	const void	*data;
};

static const struct rockchip_usb2_phy_cfg rk3288_pdata = {
	.port_reset     = {0x00, 12, 12, 0, 1},
	.soft_con       = {0x08, 2, 2, 0, 1},
	.suspend	= {0x0c, 5, 0, 0x01, 0x2A},
};

static struct rockchip_usb2_phy_dt_id rockchip_usb2_phy_dt_ids[] = {
	{ .compatible = "rockchip,rk3288-usb-phy", .data = &rk3288_pdata },
	{}
};

static void property_enable(struct dwc2_plat_otg_data *pdata,
				  const struct usb2phy_reg *reg, bool en)
{
	unsigned int val, mask, tmp;

	tmp = en ? reg->enable : reg->disable;
	mask = GENMASK(reg->bitend, reg->bitstart);
	val = (tmp << reg->bitstart) | (mask << BIT_WRITEABLE_SHIFT);

	writel(val, pdata->regs_phy + reg->offset);
}


void otg_phy_init(struct dwc2_udc *dev)
{
	struct dwc2_plat_otg_data *pdata = dev->pdata;
	struct rockchip_usb2_phy_cfg *phy_cfg = NULL;
	struct rockchip_usb2_phy_dt_id *of_id;
	int i;

	for (i = 0; i < ARRAY_SIZE(rockchip_usb2_phy_dt_ids); i++) {
		of_id = &rockchip_usb2_phy_dt_ids[i];
		if (fdt_node_check_compatible(gd->fdt_blob, pdata->phy_of_node,
					      of_id->compatible) == 0) {
			phy_cfg = (struct rockchip_usb2_phy_cfg *)of_id->data;
			break;
		}
	}
	if (!phy_cfg) {
		debug("Can't find device platform data\n");

		hang();
		return;
	}
	pdata->priv = phy_cfg;
	/* disable software control */
	property_enable(pdata, &phy_cfg->soft_con, false);

	/* reset otg port */
	property_enable(pdata, &phy_cfg->port_reset, true);
	mdelay(1);
	property_enable(pdata, &phy_cfg->port_reset, false);
	udelay(1);
}

void otg_phy_off(struct dwc2_udc *dev)
{
	struct dwc2_plat_otg_data *pdata = dev->pdata;
	struct rockchip_usb2_phy_cfg *phy_cfg = pdata->priv;

	/* enable software control */
	property_enable(pdata, &phy_cfg->soft_con, true);
	/* enter suspend */
	property_enable(pdata, &phy_cfg->suspend, true);
}
