/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <ybpark@nexell.co.kr>
 */

#include <linux/types.h>
#include <asm/io.h>

#ifndef __nx_gpio_h__
#define __nx_gpio_h__

struct nx_gpio_register_set {
	u32 gpioxout;
	u32 gpioxoutenb;
	u32 gpioxdetmode[2];
	u32 gpioxintenb;
	u32 gpioxdet;
	u32 gpioxpad;
	u32 gpioxpuenb;
	u32 gpioxaltfn[2];
	u32 gpioxdetmodeex;
	u32 __reserved[4];
	u32 gpioxdetenb;
	u32 gpiox_slew;
	u32 gpiox_slew_disable_default;
	u32 gpiox_drv1;
	u32 gpiox_drv1_disable_default;
	u32 gpiox_drv0;
	u32 gpiox_drv0_disable_default;
	u32 gpiox_pullsel;
	u32 gpiox_pullsel_disable_default;
	u32 gpiox_pullenb;
	u32 gpiox_pullenb_disable_default;
	u32 gpiox_input_mux_select0;
	u32 gpiox_input_mux_select1;
	u8 __reserved1[0x1000 - 0x70];
};

enum {
	nx_gpio_padfunc_0 = 0ul,
	nx_gpio_padfunc_1 = 1ul,
	nx_gpio_padfunc_2 = 2ul,
	nx_gpio_padfunc_3 = 3ul
};

enum {
	nx_gpio_drvstrength_0 = 0ul,
	nx_gpio_drvstrength_1 = 1ul,
	nx_gpio_drvstrength_2 = 2ul,
	nx_gpio_drvstrength_3 = 3ul
};

enum {
	nx_gpio_pull_down = 0ul,
	nx_gpio_pull_up = 1ul,
	nx_gpio_pull_off = 2ul
};

int nx_gpio_initialize(void);
u32 nx_gpio_get_number_of_module(void);
u32 nx_gpio_get_size_of_register_set(void);
void nx_gpio_set_base_address(u32 module_index, void *base_address);
void *nx_gpio_get_base_address(u32 module_index);
int nx_gpio_open_module(u32 module_index);
int nx_gpio_close_module(u32 module_index);
int nx_gpio_check_busy(u32 module_index);
void nx_gpio_set_detect_enable(u32 module_index, u32 bit_number,
			       int detect_enb);
void nx_gpio_set_pad_function(u32 module_index, u32 bit_number, u32 padfunc);
void nx_gpio_set_pad_function32(u32 module_index, u32 msbvalue, u32 lsbvalue);
int nx_gpio_get_pad_function(u32 module_index, u32 bit_number);
void nx_gpio_set_output_enable(u32 module_index, u32 bit_number,
			       int output_enb);
int nx_gpio_get_detect_enable(u32 module_index, u32 bit_number);
u32 nx_gpio_get_detect_enable32(u32 module_index);
void nx_gpio_set_detect_enable(u32 module_index, u32 bit_number,
			       int detect_enb);
void nx_gpio_set_detect_enable32(u32 module_index, u32 enable_flag);
int nx_gpio_get_output_enable(u32 module_index, u32 bit_number);
void nx_gpio_set_output_enable32(u32 module_index, int output_enb);
u32 nx_gpio_get_output_enable32(u32 module_index);
void nx_gpio_set_output_value(u32 module_index, u32 bit_number, int value);
int nx_gpio_get_output_value(u32 module_index, u32 bit_number);
void nx_gpio_set_output_value32(u32 module_index, u32 value);
u32 nx_gpio_get_output_value32(u32 module_index);
int nx_gpio_get_input_value(u32 module_index, u32 bit_number);
void nx_gpio_set_pull_select(u32 module_index, u32 bit_number, int enable);
void nx_gpio_set_pull_select32(u32 module_index, u32 value);
int nx_gpio_get_pull_select(u32 module_index, u32 bit_number);
u32 nx_gpio_get_pull_select32(u32 module_index);
void nx_gpio_set_pull_mode(u32 module_index, u32 bit_number, u32 mode);
void nx_gpio_set_fast_slew(u32 module_index, u32 bit_number, int enable);
void nx_gpio_set_drive_strength_disable_default(u32 module_index,
						u32 bit_number, int enable);
void nx_gpio_set_drive_strength_disable_default(u32 module_index,
						u32 bit_number, int enable);
void nx_gpio_set_drive_strength(u32 module_index, u32 bit_number,
				u32 drvstrength);
void nx_gpio_set_drive_strength_disable_default(u32 module_index,
						u32 bit_number, int enable);
u32 nx_gpio_get_drive_strength(u32 module_index, u32 bit_number);
#endif
