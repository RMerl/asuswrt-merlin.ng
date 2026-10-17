// SPDX-License-Identifier: GPL-2.0+
/*
 * SAM9X60's USB Clock support.
 *
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sergiu Moga <sergiu.moga@microchip.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_SAM9X60_USB		"at91-sam9x60-usb-clk"

struct sam9x60_usb {
	const struct clk_usbck_layout		*layout;
	void					__iomem *base;
	struct clk				clk;
	const u32				*clk_mux_table;
	const u32				*mux_table;
	const char * const			*parent_names;
	u32					num_parents;
	u8					id;
};

#define to_sam9x60_usb(_clk)	container_of(_clk, struct sam9x60_usb, clk)
#define USB_MAX_DIV		15

static int sam9x60_usb_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct sam9x60_usb *usb = to_sam9x60_usb(clk);
	int index;
	u32 val;

	index = at91_clk_mux_val_to_index(usb->clk_mux_table, usb->num_parents,
					  parent->id);
	if (index < 0)
		return index;

	index = at91_clk_mux_index_to_val(usb->mux_table, usb->num_parents,
					  index);
	if (index < 0)
		return index;

	pmc_read(usb->base, usb->layout->offset, &val);
	val &= ~usb->layout->usbs_mask;
	val |= index << (ffs(usb->layout->usbs_mask - 1));
	pmc_write(usb->base, usb->layout->offset, val);

	return 0;
}

static ulong sam9x60_usb_clk_get_rate(struct clk *clk)
{
	struct sam9x60_usb *usb = to_sam9x60_usb(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val, usbdiv;

	if (!parent_rate)
		return 0;

	pmc_read(usb->base, usb->layout->offset, &val);
	usbdiv = (val & usb->layout->usbdiv_mask) >>
		(ffs(usb->layout->usbdiv_mask) - 1);
	return parent_rate / (usbdiv + 1);
}

static ulong sam9x60_usb_clk_set_rate(struct clk *clk, ulong rate)
{
	struct sam9x60_usb *usb = to_sam9x60_usb(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 usbdiv, val;

	if (!parent_rate)
		return 0;

	usbdiv = DIV_ROUND_CLOSEST(parent_rate, rate);
	if (usbdiv > USB_MAX_DIV + 1 || !usbdiv)
		return 0;

	pmc_read(usb->base, usb->layout->offset, &val);
	val &= usb->layout->usbdiv_mask;
	val |= (usbdiv - 1) << (ffs(usb->layout->usbdiv_mask) - 1);
	pmc_write(usb->base, usb->layout->offset, val);

	return parent_rate / usbdiv;
}

static const struct clk_ops sam9x60_usb_ops = {
	.set_parent = sam9x60_usb_clk_set_parent,
	.set_rate = sam9x60_usb_clk_set_rate,
	.get_rate = sam9x60_usb_clk_get_rate,
};

struct clk *
sam9x60_clk_register_usb(void __iomem *base,  const char *name,
			 const char * const *parent_names, u8 num_parents,
			 const struct clk_usbck_layout *usbck_layout,
			 const u32 *clk_mux_table, const u32 *mux_table, u8 id)
{
	struct sam9x60_usb *usb;
	struct clk *clk;
	int ret, index;
	u32 val;

	if (!base || !name || !parent_names || !num_parents ||
	    !clk_mux_table || !mux_table)
		return ERR_PTR(-EINVAL);

	usb = kzalloc(sizeof(*usb), GFP_KERNEL);
	if (!usb)
		return ERR_PTR(-ENOMEM);

	usb->id = id;
	usb->base = base;
	usb->layout = usbck_layout;
	usb->parent_names = parent_names;
	usb->num_parents = num_parents;
	usb->clk_mux_table = clk_mux_table;
	usb->mux_table = mux_table;

	clk = &usb->clk;
	clk->flags = CLK_SET_RATE_GATE | CLK_SET_PARENT_GATE |
		     CLK_SET_RATE_PARENT;

	pmc_read(usb->base, usb->layout->offset, &val);

	val = (val & usb->layout->usbs_mask) >>
		(ffs(usb->layout->usbs_mask) - 1);

	index = at91_clk_mux_val_to_index(usb->mux_table, usb->num_parents,
					  val);

	if (index < 0) {
		kfree(usb);
		return ERR_PTR(index);
	}

	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SAM9X60_USB, name,
			   parent_names[index]);
	if (ret) {
		kfree(usb);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_sam9x60_usb_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X60_USB,
	.id = UCLASS_CLK,
	.ops = &sam9x60_usb_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
