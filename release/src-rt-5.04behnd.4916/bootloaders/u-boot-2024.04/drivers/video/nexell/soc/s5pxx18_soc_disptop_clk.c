// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <linux/types.h>
#include <linux/io.h>

#include "s5pxx18_soc_disptop_clk.h"
#include "s5pxx18_soc_disptop.h"

static struct {
	struct nx_disptop_clkgen_register_set *__g_pregister;
} __g_module_variables[NUMBER_OF_DISPTOP_CLKGEN_MODULE] = {
	{ NULL,},
};

int nx_disp_top_clkgen_initialize(void)
{
	static int binit;
	u32 i;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_DISPTOP_CLKGEN_MODULE; i++)
			__g_module_variables[i].__g_pregister = NULL;
		binit = 1;
	}
	return 1;
}

u32 nx_disp_top_clkgen_get_number_of_module(void)
{
	return NUMBER_OF_DISPTOP_CLKGEN_MODULE;
}

u32 nx_disp_top_clkgen_get_physical_address(u32 module_index)
{
	static const u32 physical_addr[] =
		PHY_BASEADDR_DISPTOP_CLKGEN_LIST;

	return (u32)physical_addr[module_index];
}

u32 nx_disp_top_clkgen_get_size_of_register_set(void)
{
	return sizeof(struct nx_disptop_clkgen_register_set);
}

void nx_disp_top_clkgen_set_base_address(u32 module_index, void *base_address)
{
	__g_module_variables[module_index].__g_pregister =
	    (struct nx_disptop_clkgen_register_set *)base_address;
}

void *nx_disp_top_clkgen_get_base_address(u32 module_index)
{
	return (void *)__g_module_variables[module_index].__g_pregister;
}

void nx_disp_top_clkgen_set_clock_bclk_mode(u32 module_index,
					    enum nx_bclkmode mode)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 regvalue;
	u32 clkmode = 0;

	pregister = __g_module_variables[module_index].__g_pregister;
	switch (mode) {
	case nx_bclkmode_disable:
		clkmode = 0;
	case nx_bclkmode_dynamic:
		clkmode = 2;
		break;
	case nx_bclkmode_always:
		clkmode = 3;
		break;
	default:
		break;
	}

	regvalue = pregister->clkenb;
	regvalue &= ~3ul;
	regvalue |= (clkmode & 0x03);

	writel(regvalue, &pregister->clkenb);
}

enum nx_bclkmode nx_disp_top_clkgen_get_clock_bclk_mode(u32 module_index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	u32 mode = 0;

	pregister = __g_module_variables[module_index].__g_pregister;
	mode = (pregister->clkenb & 3ul);

	switch (mode) {
	case 0:
		return nx_bclkmode_disable;
	case 2:
		return nx_bclkmode_dynamic;
	case 3:
		return nx_bclkmode_always;
	default:
		break;
	}
	return nx_bclkmode_disable;
}

void nx_disp_top_clkgen_set_clock_pclk_mode(u32 module_index,
					    enum nx_pclkmode mode)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 regvalue;
	const u32 pclkmode_pos = 3;
	u32 clkmode = 0;

	pregister = __g_module_variables[module_index].__g_pregister;
	switch (mode) {
	case nx_pclkmode_dynamic:
		clkmode = 0;
		break;
	case nx_pclkmode_always:
		clkmode = 1;
		break;
	default:
		break;
	}

	regvalue = pregister->clkenb;
	regvalue &= ~(1ul << pclkmode_pos);
	regvalue |= (clkmode & 0x01) << pclkmode_pos;

	writel(regvalue, &pregister->clkenb);
}

enum nx_pclkmode nx_disp_top_clkgen_get_clock_pclk_mode(u32 module_index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 pclkmode_pos = 3;

	pregister = __g_module_variables[module_index].__g_pregister;

	if (pregister->clkenb & (1ul << pclkmode_pos))
		return nx_pclkmode_always;

	return nx_pclkmode_dynamic;
}

void nx_disp_top_clkgen_set_clock_source(u32 module_index, u32 index,
					 u32 clk_src)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 read_value;

	const u32 clksrcsel_pos = 2;
	const u32 clksrcsel_mask = 0x07 << clksrcsel_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->CLKGEN[index << 1];
	read_value &= ~clksrcsel_mask;
	read_value |= clk_src << clksrcsel_pos;

	writel(read_value, &pregister->CLKGEN[index << 1]);
}

u32 nx_disp_top_clkgen_get_clock_source(u32 module_index, u32 index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 clksrcsel_pos = 2;
	const u32 clksrcsel_mask = 0x07 << clksrcsel_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	return (pregister->CLKGEN[index << 1] &
		clksrcsel_mask) >> clksrcsel_pos;
}

void nx_disp_top_clkgen_set_clock_divisor(u32 module_index, u32 index,
					  u32 divisor)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 clkdiv_pos = 5;
	const u32 clkdiv_mask = 0xff << clkdiv_pos;
	register u32 read_value;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->CLKGEN[index << 1];
	read_value &= ~clkdiv_mask;
	read_value |= (divisor - 1) << clkdiv_pos;
	writel(read_value, &pregister->CLKGEN[index << 1]);
}

u32 nx_disp_top_clkgen_get_clock_divisor(u32 module_index, u32 index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 clkdiv_pos = 5;
	const u32 clkdiv_mask = 0xff << clkdiv_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	return ((pregister->CLKGEN[index << 1] &
		 clkdiv_mask) >> clkdiv_pos) + 1;
}

void nx_disp_top_clkgen_set_clock_divisor_enable(u32 module_index, int enable)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 read_value;
	const u32 clkgenenb_pos = 2;
	const u32 clkgenenb_mask = 1ul << clkgenenb_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->clkenb;
	read_value &= ~clkgenenb_mask;
	read_value |= (u32)enable << clkgenenb_pos;

	writel(read_value, &pregister->clkenb);
}

int nx_disp_top_clkgen_get_clock_divisor_enable(u32 module_index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 clkgenenb_pos = 2;
	const u32 clkgenenb_mask = 1ul << clkgenenb_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	return (int)((pregister->clkenb &
		      clkgenenb_mask) >> clkgenenb_pos);
}

void nx_disp_top_clkgen_set_clock_out_inv(u32 module_index, u32 index,
					  int out_clk_inv)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 read_value;
	const u32 outclkinv_pos = 1;
	const u32 outclkinv_mask = 1ul << outclkinv_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->CLKGEN[index << 1];
	read_value &= ~outclkinv_mask;
	read_value |= out_clk_inv << outclkinv_pos;

	writel(read_value, &pregister->CLKGEN[index << 1]);
}

int nx_disp_top_clkgen_get_clock_out_inv(u32 module_index, u32 index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 outclkinv_pos = 1;
	const u32 outclkinv_mask = 1ul << outclkinv_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	return (int)((pregister->CLKGEN[index << 1] &
		      outclkinv_mask) >> outclkinv_pos);
}

int nx_disp_top_clkgen_set_input_inv(u32 module_index,
				     u32 index, int in_clk_inv)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 read_value;
	const u32 inclkinv_pos = 4 + index;
	const u32 inclkinv_mask = 1ul << inclkinv_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->clkenb;
	read_value &= ~inclkinv_mask;
	read_value |= in_clk_inv << inclkinv_pos;

	writel(read_value, &pregister->clkenb);
	return true;
}

int nx_disp_top_clkgen_get_input_inv(u32 module_index, u32 index)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	const u32 inclkinv_pos = 4 + index;
	const u32 inclkinv_mask = 1ul << inclkinv_pos;

	pregister = __g_module_variables[module_index].__g_pregister;

	return (int)((pregister->clkenb &
		      inclkinv_mask) >> inclkinv_pos);
}

void nx_disp_top_clkgen_set_clock_out_select(u32 module_index, u32 index,
					     int bbypass)
{
	register struct nx_disptop_clkgen_register_set *pregister;
	register u32 read_value;

	pregister = __g_module_variables[module_index].__g_pregister;

	read_value = pregister->CLKGEN[index << 1];
	read_value = read_value & (~0x01);
	read_value = read_value | bbypass;

	writel(read_value, &pregister->CLKGEN[index << 1]);
}
