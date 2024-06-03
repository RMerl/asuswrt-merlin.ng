// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <dm-demo.h>

static const struct dm_demo_pdata red_square = {
	.colour = "red",
	.sides = 4.
};
static const struct dm_demo_pdata green_triangle = {
	.colour = "green",
	.sides = 3.
};
static const struct dm_demo_pdata yellow_hexagon = {
	.colour = "yellow",
	.sides = 6.
};

U_BOOT_DEVICE(demo0) = {
	.name = "demo_shape_drv",
	.platdata = &red_square,
};

U_BOOT_DEVICE(demo1) = {
	.name = "demo_simple_drv",
	.platdata = &red_square,
};

U_BOOT_DEVICE(demo2) = {
	.name = "demo_shape_drv",
	.platdata = &green_triangle,
};

U_BOOT_DEVICE(demo3) = {
	.name = "demo_simple_drv",
	.platdata = &yellow_hexagon,
};

U_BOOT_DEVICE(demo4) = {
	.name = "demo_shape_drv",
	.platdata = &yellow_hexagon,
};
