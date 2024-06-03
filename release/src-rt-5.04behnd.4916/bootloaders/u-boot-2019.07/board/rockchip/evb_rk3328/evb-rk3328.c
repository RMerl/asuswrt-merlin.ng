// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <dwc3-uboot.h>
#include <power/regulator.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	int ret;

	ret = regulators_enable_boot_on(false);
	if (ret)
		debug("%s: Cannot enable boot on regulator\n", __func__);

	return ret;
}

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)
#include <usb.h>
#include <usb/dwc2_udc.h>

static struct dwc2_plat_otg_data rk3328_otg_data = {
	.rx_fifo_sz	= 512,
	.np_tx_fifo_sz	= 16,
	.tx_fifo_sz	= 128,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node;
	const char *mode;
	bool matched = false;
	const void *blob = gd->fdt_blob;

	/* find the usb_otg node */
	node = fdt_node_offset_by_compatible(blob, -1,
					"rockchip,rk3328-usb");

	while (node > 0) {
		mode = fdt_getprop(blob, node, "dr_mode", NULL);
		if (mode && strcmp(mode, "otg") == 0) {
			matched = true;
			break;
		}

		node = fdt_node_offset_by_compatible(blob, node,
					"rockchip,rk3328-usb");
	}
	if (!matched) {
		debug("Not found usb_otg device\n");
		return -ENODEV;
	}

	rk3328_otg_data.regs_otg = fdtdec_get_addr(blob, node, "reg");

	return dwc2_udc_probe(&rk3328_otg_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	return 0;
}
#endif
