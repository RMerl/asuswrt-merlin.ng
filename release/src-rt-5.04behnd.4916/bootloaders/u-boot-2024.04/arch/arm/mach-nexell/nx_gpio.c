// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <ybpark@nexell.co.kr>
 */

/*
 * FIXME : will be remove after support pinctrl
 */
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/nexell.h>
#include "asm/arch/nx_gpio.h"
#define NUMBER_OF_GPIO_MODULE 5
u32 __g_nx_gpio_valid_bit[NUMBER_OF_GPIO_MODULE] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

static struct {
	struct nx_gpio_register_set *pregister;
} __g_module_variables[NUMBER_OF_GPIO_MODULE] = {
	{ (struct nx_gpio_register_set *)PHY_BASEADDR_GPIOA },
	{ (struct nx_gpio_register_set *)PHY_BASEADDR_GPIOB },
	{ (struct nx_gpio_register_set *)PHY_BASEADDR_GPIOC },
	{ (struct nx_gpio_register_set *)PHY_BASEADDR_GPIOD },
	{ (struct nx_gpio_register_set *)PHY_BASEADDR_GPIOE },
};

enum { nx_gpio_max_bit = 32 };

void nx_gpio_set_bit(u32 *value, u32 bit, int enable)
{
	register u32 newvalue;

	newvalue = *value;
	newvalue &= ~(1ul << bit);
	newvalue |= (u32)enable << bit;
	writel(newvalue, value);
}

int nx_gpio_get_bit(u32 value, u32 bit)
{
	return (int)((value >> bit) & (1ul));
}

void nx_gpio_set_bit2(u32 *value, u32 bit, u32 bit_value)
{
	register u32 newvalue = *value;

	newvalue = (u32)(newvalue & ~(3ul << (bit * 2)));
	newvalue = (u32)(newvalue | (bit_value << (bit * 2)));

	writel(newvalue, value);
}

u32 nx_gpio_get_bit2(u32 value, u32 bit)
{
	return (u32)((u32)(value >> (bit * 2)) & 3ul);
}

int nx_gpio_initialize(void)
{
	static int binit;
	u32 i;

	binit = 0;

	if (binit == 0) {
		for (i = 0; i < NUMBER_OF_GPIO_MODULE; i++)
			__g_module_variables[i].pregister = NULL;
		binit = true;
	}
	for (i = 0; i < NUMBER_OF_GPIO_MODULE; i++) {
		__g_nx_gpio_valid_bit[i] = 0xFFFFFFFF;
	};
	return true;
}

u32 nx_gpio_get_number_of_module(void)
{
	return NUMBER_OF_GPIO_MODULE;
}

u32 nx_gpio_get_size_of_register_set(void)
{
	return sizeof(struct nx_gpio_register_set);
}

void nx_gpio_set_base_address(u32 module_index, void *base_address)
{
	__g_module_variables[module_index].pregister =
		(struct nx_gpio_register_set *)base_address;
}

void *nx_gpio_get_base_address(u32 module_index)
{
	return (void *)__g_module_variables[module_index].pregister;
}

int nx_gpio_open_module(u32 module_index)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(0xFFFFFFFF, &pregister->gpiox_slew_disable_default);
	writel(0xFFFFFFFF, &pregister->gpiox_drv1_disable_default);
	writel(0xFFFFFFFF, &pregister->gpiox_drv0_disable_default);
	writel(0xFFFFFFFF, &pregister->gpiox_pullsel_disable_default);
	writel(0xFFFFFFFF, &pregister->gpiox_pullenb_disable_default);
	return true;
}

int nx_gpio_close_module(u32 module_index) { return true; }

int nx_gpio_check_busy(u32 module_index) { return false; }

void nx_gpio_set_pad_function(u32 module_index, u32 bit_number,
			      u32 padfunc)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit2(&pregister->gpioxaltfn[bit_number / 16],
			 bit_number % 16, padfunc);
}

void nx_gpio_set_pad_function32(u32 module_index, u32 msbvalue, u32 lsbvalue)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(lsbvalue, &pregister->gpioxaltfn[0]);
	writel(msbvalue, &pregister->gpioxaltfn[1]);
}

int nx_gpio_get_pad_function(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return (int)nx_gpio_get_bit2
		(readl(&pregister->gpioxaltfn[bit_number / 16]),
		 bit_number % 16);
}

void nx_gpio_set_output_enable(u32 module_index, u32 bit_number,
			       int output_enb)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpioxoutenb, bit_number, output_enb);
}

int nx_gpio_get_detect_enable(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return nx_gpio_get_bit(readl(&pregister->gpioxdetenb), bit_number);
}

u32 nx_gpio_get_detect_enable32(u32 module_index)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return readl(&pregister->gpioxdetenb);
}

void nx_gpio_set_detect_enable(u32 module_index, u32 bit_number,
			       int detect_enb)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpioxdetenb, bit_number, detect_enb);
}

void nx_gpio_set_detect_enable32(u32 module_index, u32 enable_flag)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(enable_flag, &pregister->gpioxdetenb);
}

int nx_gpio_get_output_enable(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return nx_gpio_get_bit(readl(&pregister->gpioxoutenb), bit_number);
}

void nx_gpio_set_output_enable32(u32 module_index, int output_enb)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	if (output_enb)
		writel(0xFFFFFFFF, &pregister->gpioxoutenb);
	else
		writel(0x0, &pregister->gpioxoutenb);
}

u32 nx_gpio_get_output_enable32(u32 module_index)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return readl(&pregister->gpioxoutenb);
}

void nx_gpio_set_output_value(u32 module_index, u32 bit_number, int value)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpioxout, bit_number, value);
}

int nx_gpio_get_output_value(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return nx_gpio_get_bit(readl(&pregister->gpioxout), bit_number);
}

void nx_gpio_set_output_value32(u32 module_index, u32 value)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	writel(value, &pregister->gpioxout);
}

u32 nx_gpio_get_output_value32(u32 module_index)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return readl(&pregister->gpioxout);
}

int nx_gpio_get_input_value(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	return nx_gpio_get_bit(readl(&pregister->gpioxpad), bit_number);
}

void nx_gpio_set_pull_select(u32 module_index, u32 bit_number, int enable)
{
	nx_gpio_set_bit(&__g_module_variables[module_index]
			.pregister->gpiox_pullsel_disable_default,
			bit_number, true);
	nx_gpio_set_bit
		(&__g_module_variables[module_index].pregister->gpiox_pullsel,
		 bit_number, enable);
}

void nx_gpio_set_pull_select32(u32 module_index, u32 value)
{
	writel(value,
	       &__g_module_variables[module_index].pregister->gpiox_pullsel);
}

int nx_gpio_get_pull_select(u32 module_index, u32 bit_number)
{
	return nx_gpio_get_bit
		(__g_module_variables[module_index].pregister->gpiox_pullsel,
		 bit_number);
}

u32 nx_gpio_get_pull_select32(u32 module_index)
{
	return __g_module_variables[module_index].pregister->gpiox_pullsel;
}

void nx_gpio_set_pull_mode(u32 module_index, u32 bit_number, u32 mode)
{
	nx_gpio_set_bit(&__g_module_variables[module_index]
			.pregister->gpiox_pullsel_disable_default,
			bit_number, true);
	nx_gpio_set_bit(&__g_module_variables[module_index]
			.pregister->gpiox_pullenb_disable_default,
			bit_number, true);
	if (mode == nx_gpio_pull_off) {
		nx_gpio_set_bit
		 (&__g_module_variables[module_index].pregister->gpiox_pullenb,
		 bit_number, false);
		nx_gpio_set_bit
		 (&__g_module_variables[module_index].pregister->gpiox_pullsel,
		 bit_number, false);
	} else {
		nx_gpio_set_bit
		 (&__g_module_variables[module_index].pregister->gpiox_pullsel,
		 bit_number, (mode & 1 ? true : false));
		nx_gpio_set_bit
		 (&__g_module_variables[module_index].pregister->gpiox_pullenb,
		 bit_number, true);
	}
}

void nx_gpio_set_fast_slew(u32 module_index, u32 bit_number,
			   int enable)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpiox_slew, bit_number,
			(int)(!enable));
}

void nx_gpio_set_drive_strength(u32 module_index, u32 bit_number,
				u32 drvstrength)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpiox_drv1, bit_number,
			(int)(((u32)drvstrength >> 0) & 0x1));
	nx_gpio_set_bit(&pregister->gpiox_drv0, bit_number,
			(int)(((u32)drvstrength >> 1) & 0x1));
}

void nx_gpio_set_drive_strength_disable_default(u32 module_index,
						u32 bit_number, int enable)
{
	register struct nx_gpio_register_set *pregister;

	pregister = __g_module_variables[module_index].pregister;
	nx_gpio_set_bit(&pregister->gpiox_drv1_disable_default, bit_number,
			(int)(enable));
	nx_gpio_set_bit(&pregister->gpiox_drv0_disable_default, bit_number,
			(int)(enable));
}

u32 nx_gpio_get_drive_strength(u32 module_index, u32 bit_number)
{
	register struct nx_gpio_register_set *pregister;
	register u32 retvalue;

	pregister = __g_module_variables[module_index].pregister;
	retvalue =
		nx_gpio_get_bit(readl(&pregister->gpiox_drv0), bit_number) << 1;
	retvalue |=
		nx_gpio_get_bit(readl(&pregister->gpiox_drv1), bit_number) << 0;
	return retvalue;
}
