// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <config.h>
#include <common.h>
#include <errno.h>

#include <asm/arch/display.h>

#include "soc/s5pxx18_soc_disptop.h"

static int rgb_switch(int module, int input, struct dp_sync_info *sync,
		      struct dp_rgb_dev *dev)
{
	int mpu = dev->lcd_mpu_type;
	int rsc = 0, sel = 0;

	switch (module) {
	case 0:
		sel = mpu ? 1 : 0;
		break;
	case 1:
		sel = rsc ? 3 : 2;
		break;
	default:
		printf("Fail, %s nuknown module %d\n", __func__, module);
		return -1;
	}

	nx_disp_top_set_primary_mux(sel);
	return 0;
}

void nx_rgb_display(int module,
		    struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		    struct dp_plane_top *top, struct dp_plane_info *planes,
		    struct dp_rgb_dev *dev)
{
	struct dp_plane_info *plane = planes;
	int input = module == 0 ? DP_DEVICE_DP0 : DP_DEVICE_DP1;
	int count = top->plane_num;
	int i = 0;

	printf("RGB:   dp.%d\n", module);

	dp_control_init(module);
	dp_plane_init(module);

	/* set plane */
	dp_plane_screen_setup(module, top);

	for (i = 0; count > i; i++, plane++) {
		if (!plane->enable)
			continue;
		dp_plane_layer_setup(module, plane);
		dp_plane_layer_enable(module, plane, 1);
	}

	dp_plane_screen_enable(module, 1);

	rgb_switch(module, input, sync, dev);

	dp_control_setup(module, sync, ctrl);
	dp_control_enable(module, 1);
}
