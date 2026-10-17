// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Collabora Ltd.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <env.h>

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	if (is_usb_boot()) {
		env_set("bootcmd", "run bootcmd_mfg");
		env_set("bootdelay", "0");
	}

	return 0;
}
