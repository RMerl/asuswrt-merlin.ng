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

U_BOOT_DRVINFO(demo0) = {
	.name = "demo_shape_drv",
	.plat = &red_square,
};

U_BOOT_DRVINFO(demo1) = {
	.name = "demo_simple_drv",
	.plat = &red_square,
};

U_BOOT_DRVINFO(demo2) = {
	.name = "demo_shape_drv",
	.plat = &green_triangle,
};

U_BOOT_DRVINFO(demo3) = {
	.name = "demo_simple_drv",
	.plat = &yellow_hexagon,
};

U_BOOT_DRVINFO(demo4) = {
	.name = "demo_shape_drv",
	.plat = &yellow_hexagon,
};
