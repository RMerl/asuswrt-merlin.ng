/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dwc3-generic.h - Generic DWC3 Glue layer header
 *
 * Copyright (C) 2016 - 2018 Xilinx, Inc.
 * Copyright (C) 2023 Socionext Inc.
 */

#ifndef __DRIVERS_USB_DWC3_GENERIC_H
#define __DRIVERS_USB_DWC3_GENERIC_H

#include <clk.h>
#include <reset.h>
#include <dwc3-uboot.h>

struct dwc3_glue_data {
	struct clk_bulk		clks;
	struct reset_ctl_bulk	resets;
	fdt_addr_t regs;
	fdt_size_t size;
};

struct dwc3_glue_ops {
	int (*glue_get_ctrl_dev)(struct udevice *parent, ofnode *node);
	void (*glue_configure)(struct udevice *dev, int index,
			       enum usb_dr_mode mode);
};

int dwc3_glue_bind(struct udevice *parent);
int dwc3_glue_probe(struct udevice *dev);
int dwc3_glue_remove(struct udevice *dev);

#endif
