/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */

#ifndef __DM_DEMO_H
#define __DM_DEMO_H

#include <dm.h>

/**
 * struct dm_demo_pdata - configuration data for demo instance
 *
 * @colour: Color of the demo
 * @sides: Numbers of sides
 * @default_char: Default ASCII character to output (65 = 'A')
 */
struct dm_demo_pdata {
	const char *colour;
	int sides;
	int default_char;
};

struct demo_ops {
	int (*hello)(struct udevice *dev, int ch);
	int (*status)(struct udevice *dev, int *status);
	int (*set_light)(struct udevice *dev, int light);
	int (*get_light)(struct udevice *dev);
};

int demo_hello(struct udevice *dev, int ch);
int demo_status(struct udevice *dev, int *status);
int demo_set_light(struct udevice *dev, int light);
int demo_get_light(struct udevice *dev);
int demo_list(void);

int demo_parse_dt(struct udevice *dev);

#endif
