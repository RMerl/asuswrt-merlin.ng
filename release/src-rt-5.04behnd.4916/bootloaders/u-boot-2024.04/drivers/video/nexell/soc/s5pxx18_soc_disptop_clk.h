/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#ifndef _S5PXX18_SOC_DISPTOP_CLK_H_
#define _S5PXX18_SOC_DISPTOP_CLK_H_

#include "s5pxx18_soc_disptype.h"

#define	PHY_BASEADDR_DISPTOP_CLKGEN_LIST	\
		{ PHY_BASEADDR_DISPTOP_CLKGEN0_MODULE, \
		  PHY_BASEADDR_DISPTOP_CLKGEN1_MODULE, \
		  PHY_BASEADDR_DISPTOP_CLKGEN2_MODULE, \
		  PHY_BASEADDR_DISPTOP_CLKGEN3_MODULE, \
		  PHY_BASEADDR_DISPTOP_CLKGEN4_MODULE, \
		}

struct nx_disptop_clkgen_register_set {
	u32 clkenb;
	u32 CLKGEN[4];
};

int nx_disp_top_clkgen_initialize(void);
u32 nx_disp_top_clkgen_get_number_of_module(void);
u32 nx_disp_top_clkgen_get_physical_address(u32 module_index);
u32 nx_disp_top_clkgen_get_size_of_register_set(void);
void nx_disp_top_clkgen_set_base_address(u32 module_index,
					 void *base_address);
void *nx_disp_top_clkgen_get_base_address(u32 module_index);
void nx_disp_top_clkgen_set_clock_pclk_mode(u32 module_index,
					    enum nx_pclkmode mode);
enum nx_pclkmode nx_disp_top_clkgen_get_clock_pclk_mode(u32 module_index);
void nx_disp_top_clkgen_set_clock_source(u32 module_index, u32 index,
					 u32 clk_src);
u32 nx_disp_top_clkgen_get_clock_source(u32 module_index, u32 index);
void nx_disp_top_clkgen_set_clock_divisor(u32 module_index, u32 index,
					  u32 divisor);
u32 nx_disp_top_clkgen_get_clock_divisor(u32 module_index, u32 index);
void nx_disp_top_clkgen_set_clock_divisor_enable(u32 module_index,
						 int enable);
int nx_disp_top_clkgen_get_clock_divisor_enable(u32 module_index);
void nx_disp_top_clkgen_set_clock_bclk_mode(u32 module_index,
					    enum nx_bclkmode mode);
enum nx_bclkmode nx_disp_top_clkgen_get_clock_bclk_mode(u32 module_index);

void nx_disp_top_clkgen_set_clock_out_inv(u32 module_index, u32 index,
					  int out_clk_inv);
int nx_disp_top_clkgen_get_clock_out_inv(u32 module_index, u32 index);
int nx_disp_top_clkgen_set_input_inv(u32 module_index, u32 index,
				     int out_clk_inv);
int nx_disp_top_clkgen_get_input_inv(u32 module_index, u32 index);

void nx_disp_top_clkgen_set_clock_out_select(u32 module_index, u32 index,
					     int bbypass);

#endif
