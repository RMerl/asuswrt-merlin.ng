/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm Pin control
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */
#ifndef _PINCTRL_QCOM_H
#define _PINCTRL_QCOM_H

#include <asm/types.h>
#include <mach/gpio.h>

struct udevice;

struct msm_pinctrl_data {
	struct msm_pin_data pin_data;
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

extern struct pinctrl_ops msm_pinctrl_ops;

int msm_pinctrl_bind(struct udevice *dev);

#endif
