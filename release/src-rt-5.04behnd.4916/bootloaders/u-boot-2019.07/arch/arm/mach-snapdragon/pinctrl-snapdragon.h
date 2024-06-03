/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm Pin control
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */
#ifndef _PINCTRL_SNAPDRAGON_H
#define _PINCTRL_SNAPDRAGON_H

#include <common.h>

struct msm_pinctrl_data {
	int pin_count;
	int functions_count;
	const char *(*get_function_name)(struct udevice *dev,
					 unsigned int selector);
	unsigned int (*get_function_mux)(unsigned int selector);
	const char *(*get_pin_name)(struct udevice *dev,
				    unsigned int selector);
};

struct pinctrl_function {
	const char *name;
	int val;
};

extern struct msm_pinctrl_data apq8016_data;
extern struct msm_pinctrl_data apq8096_data;

#endif
