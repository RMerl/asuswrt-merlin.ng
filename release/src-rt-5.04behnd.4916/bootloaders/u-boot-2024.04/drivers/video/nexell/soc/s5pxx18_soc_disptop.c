// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_disptop.h"

static struct {
	struct nx_disp_top_register_set *pregister;
} __g_module_variables = { NULL, };

int nx_disp_top_initialize(void)
{
	static int binit;
	u32 i;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_DISPTOP_MODULE; i++)
			__g_module_variables.pregister = NULL;
		binit = 1;
	}
	return 1;
}

u32 nx_disp_top_get_number_of_module(void)
{
	return NUMBER_OF_DISPTOP_MODULE;
}

u32 nx_disp_top_get_physical_address(void)
{
	static const u32 physical_addr[] = PHY_BASEADDR_DISPTOP_LIST;

	return (u32)(physical_addr[0] + PHY_BASEADDR_DISPLAYTOP_MODULE_OFFSET);
}

u32 nx_disp_top_get_size_of_register_set(void)
{
	return sizeof(struct nx_disp_top_register_set);
}

void nx_disp_top_set_base_address(void *base_address)
{
	__g_module_variables.pregister =
	    (struct nx_disp_top_register_set *)base_address;
}

void *nx_disp_top_get_base_address(void)
{
	return (void *)__g_module_variables.pregister;
}

void nx_disp_top_set_resconvmux(int benb, u32 sel)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = (benb << 31) | (sel << 0);
	writel((u32)regvalue, &pregister->resconv_mux_ctrl);
}

void nx_disp_top_set_hdmimux(int benb, u32 sel)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = (benb << 31) | (sel << 0);
	writel((u32)regvalue, &pregister->interconv_mux_ctrl);
}

void nx_disp_top_set_mipimux(int benb, u32 sel)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = (benb << 31) | (sel << 0);
	writel((u32)regvalue, &pregister->mipi_mux_ctrl);
}

void nx_disp_top_set_lvdsmux(int benb, u32 sel)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = (benb << 31) | (sel << 0);
	writel((u32)regvalue, &pregister->lvds_mux_ctrl);
}

void nx_disp_top_set_primary_mux(u32 sel)
{
	register struct nx_disp_top_register_set *pregister;

	pregister = __g_module_variables.pregister;
	writel((u32)sel, &pregister->tftmpu_mux);
}

void nx_disp_top_hdmi_set_vsync_start(u32 sel)
{
	register struct nx_disp_top_register_set *pregister;

	pregister = __g_module_variables.pregister;
	writel((u32)sel, &pregister->hdmisyncctrl0);
}

void nx_disp_top_hdmi_set_vsync_hsstart_end(u32 start, u32 end)
{
	register struct nx_disp_top_register_set *pregister;

	pregister = __g_module_variables.pregister;
	writel((u32)(end << 16) | (start << 0), &pregister->hdmisyncctrl3);
}

void nx_disp_top_hdmi_set_hactive_start(u32 sel)
{
	register struct nx_disp_top_register_set *pregister;

	pregister = __g_module_variables.pregister;
	writel((u32)sel, &pregister->hdmisyncctrl1);
}

void nx_disp_top_hdmi_set_hactive_end(u32 sel)
{
	register struct nx_disp_top_register_set *pregister;

	pregister = __g_module_variables.pregister;
	writel((u32)sel, &pregister->hdmisyncctrl2);
}

void nx_disp_top_set_hdmifield(u32 enable, u32 init_val, u32 vsynctoggle,
			       u32 hsynctoggle, u32 vsyncclr, u32 hsyncclr,
			       u32 field_use, u32 muxsel)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = ((enable & 0x01) << 0) | ((init_val & 0x01) << 1) |
		   ((vsynctoggle & 0x3fff) << 2) |
		   ((hsynctoggle & 0x3fff) << 17);
	writel(regvalue, &pregister->hdmifieldctrl);
	regvalue = ((field_use & 0x01) << 31) | ((muxsel & 0x01) << 30) |
		   ((hsyncclr) << 15) | ((vsyncclr) << 0);
	writel(regvalue, &pregister->greg0);
}

void nx_disp_top_set_padclock(u32 mux_index, u32 padclk_cfg)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = readl(&pregister->greg1);
	if (padmux_secondary_mlc == mux_index) {
		regvalue = regvalue & (~(0x7 << 3));
		regvalue = regvalue | (padclk_cfg << 3);
	} else if (padmux_resolution_conv == mux_index) {
		regvalue = regvalue & (~(0x7 << 6));
		regvalue = regvalue | (padclk_cfg << 6);
	} else {
		regvalue = regvalue & (~(0x7 << 0));
		regvalue = regvalue | (padclk_cfg << 0);
	}
	writel(regvalue, &pregister->greg1);
}

void nx_disp_top_set_lcdif_enb(int enb)
{
	register struct nx_disp_top_register_set *pregister;
	u32 regvalue;

	pregister = __g_module_variables.pregister;
	regvalue = readl(&pregister->greg1);
	regvalue = regvalue & (~(0x1 << 9));
	regvalue = regvalue | ((enb & 0x1) << 9);
	writel(regvalue, &pregister->greg1);
}
