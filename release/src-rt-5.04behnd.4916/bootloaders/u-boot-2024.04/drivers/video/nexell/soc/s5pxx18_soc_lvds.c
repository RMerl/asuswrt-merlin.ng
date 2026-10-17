// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_disptop.h"
#include "s5pxx18_soc_lvds.h"

#ifndef pow
static inline unsigned int pow(int a, int b)
{
	if (b == 0)
		return 1;
	else
		return a * pow(a, b - 1);
}
#endif

static struct nx_lvds_register_set *__g_pregister[NUMBER_OF_LVDS_MODULE];

int nx_lvds_initialize(void)
{
	static int binit;
	u32 i;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_LVDS_MODULE; i++)
			__g_pregister[i] = NULL;
		binit = 1;
	}

	return 1;
}

u32 nx_lvds_get_number_of_module(void)
{
	return NUMBER_OF_LVDS_MODULE;
}

u32 nx_lvds_get_size_of_register_set(void)
{
	return sizeof(struct nx_lvds_register_set);
}

void nx_lvds_set_base_address(u32 module_index, void *base_address)
{
	__g_pregister[module_index] =
	    (struct nx_lvds_register_set *)base_address;
}

void *nx_lvds_get_base_address(u32 module_index)
{
	return (void *)__g_pregister[module_index];
}

u32 nx_lvds_get_physical_address(u32 module_index)
{
	const u32 physical_addr[] = PHY_BASEADDR_LVDS_LIST;

	return physical_addr[module_index];
}

int nx_lvds_open_module(u32 module_index)
{
	return true;
}

int nx_lvds_close_module(u32 module_index)
{
	return true;
}

int nx_lvds_check_busy(u32 module_index)
{
	return false;
}

void nx_lvds_set_lvdsctrl0(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsctrl0);
}

void nx_lvds_set_lvdsctrl1(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsctrl1);
}

void nx_lvds_set_lvdsctrl2(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsctrl2);
}

void nx_lvds_set_lvdsctrl3(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsctrl3);
}

void nx_lvds_set_lvdsctrl4(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsctrl4);
}

void nx_lvds_set_lvdstmode0(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdstmode0);
}

void nx_lvds_set_lvdsloc0(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc0);
}

void nx_lvds_set_lvdsloc1(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc1);
}

void nx_lvds_set_lvdsloc2(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc2);
}

void nx_lvds_set_lvdsloc3(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc3);
}

void nx_lvds_set_lvdsloc4(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc4);
}

void nx_lvds_set_lvdsloc5(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc5);
}

void nx_lvds_set_lvdsloc6(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdsloc6);
}

void nx_lvds_set_lvdslocmask0(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdslocmask0);
}

void nx_lvds_set_lvdslocmask1(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdslocmask1);
}

void nx_lvds_set_lvdslocpol0(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdslocpol0);
}

void nx_lvds_set_lvdslocpol1(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	writel(regvalue, &pregister->lvdslocpol1);
}

void nx_lvds_set_lvdsdummy(u32 module_index, u32 regvalue)
{
	register struct nx_lvds_register_set *pregister;
	u32 oldvalue;

	pregister = __g_pregister[module_index];
	oldvalue = readl(&pregister->lvdsctrl1) & 0x00ffffff;
	writel(oldvalue | ((regvalue & 0xff) << 24), &pregister->lvdsctrl1);
}

u32 nx_lvds_get_lvdsdummy(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;
	u32 oldvalue;

	pregister = __g_pregister[module_index];
	oldvalue = readl(&pregister->lvdsctrl1);
	oldvalue = oldvalue >> 24;
	return oldvalue;
}

u32 nx_lvds_get_lvdsctrl0(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	return (u32)readl(&pregister->lvdsctrl0);
}

u32 nx_lvds_get_lvdsctrl1(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	return (u32)readl(&pregister->lvdsctrl1);
}

u32 nx_lvds_get_lvdsctrl2(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	return (u32)readl(&pregister->lvdsctrl2);
}

u32 nx_lvds_get_lvdsctrl3(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	return (u32)readl(&pregister->lvdsctrl3);
}

u32 nx_lvds_get_lvdsctrl4(u32 module_index)
{
	register struct nx_lvds_register_set *pregister;

	pregister = __g_pregister[module_index];
	return (u32)readl(&pregister->lvdsctrl4);
}
