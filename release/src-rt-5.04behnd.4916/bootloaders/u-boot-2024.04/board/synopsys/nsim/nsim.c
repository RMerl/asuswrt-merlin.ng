// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Synopsys, Inc. All rights reserved.
 */

#include <init.h>
#include <dm/device.h>
#include <virtio_types.h>
#include <virtio.h>

int board_early_init_r(void)
{
	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

int checkboard(void)
{
	printf("Board: ARC virtual or prototyping platform\n");
	return 0;
};
