// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_hdmi.h"

static u32 *hdmi_base_addr;

u32 nx_hdmi_get_reg(u32 module_index, u32 offset)
{
	u32 *reg_addr;
	u32 regvalue;

	reg_addr = hdmi_base_addr + (offset / sizeof(u32));
	regvalue = readl((u32 *)reg_addr);

	return regvalue;
}

void nx_hdmi_set_reg(u32 module_index, u32 offset, u32 regvalue)
{
	s64 offset_new = (s64)((int32_t)offset);
	u32 *reg_addr;

	reg_addr = hdmi_base_addr + (offset_new / sizeof(u32));
	writel(regvalue, (u32 *)reg_addr);
}

void nx_hdmi_set_base_address(u32 module_index, void *base_address)
{
	hdmi_base_addr = (u32 *)base_address;
}

void *nx_hdmi_get_base_address(u32 module_index)
{
	return (u32 *)hdmi_base_addr;
}

u32 nx_hdmi_get_physical_address(u32 module_index)
{
	const u32 physical_addr[] = PHY_BASEADDR_HDMI_LIST;

	return physical_addr[module_index];
}
