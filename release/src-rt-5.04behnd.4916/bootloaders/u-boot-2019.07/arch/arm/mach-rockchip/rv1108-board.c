// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data rv1108_otg_data = {
	.rx_fifo_sz	= 512,
	.np_tx_fifo_sz	= 16,
	.tx_fifo_sz	= 128,
};

int board_usb_init(int index, enum usb_init_type init)
{
	const void *blob = gd->fdt_blob;
	bool matched = false;
	int node, phy_node;
	u32 grf_phy_offset;
	const char *mode;

	/* find the usb_otg node */
	node = fdt_node_offset_by_compatible(blob, -1, "rockchip,rk3066-usb");
	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
						     "rockchip,rk3066-usb");
	}

	if (!matched) {
		debug("usb_otg device not found\n");
		return -ENODEV;
	}

	rv1108_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	node = fdtdec_lookup_phandle(blob, node, "phys");
	if (node <= 0) {
		debug("phys node not found\n");
		return -ENODEV;
	}

	phy_node = fdt_parent_offset(blob, node);
	if (phy_node <= 0) {
		debug("usb phy node not found\n");
		return -ENODEV;
	}

	rv1108_otg_data.phy_of_node = phy_node;
	grf_phy_offset = fdtdec_get_addr(blob, node, "reg");

	/* find the grf node */
	node = fdt_node_offset_by_compatible(blob, -1,
					     "rockchip,rv1108-grf");
	if (node <= 0) {
		debug("grf node not found\n");
		return -ENODEV;
	}

	rv1108_otg_data.regs_phy = grf_phy_offset + fdtdec_get_addr(blob, node,
								    "reg");

	return dwc2_udc_probe(&rv1108_otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif
