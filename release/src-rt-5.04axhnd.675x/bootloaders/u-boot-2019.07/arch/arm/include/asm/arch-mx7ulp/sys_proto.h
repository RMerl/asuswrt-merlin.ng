/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef _SYS_PROTO_MX7ULP_H_
#define _SYS_PROTO_MX7ULP_H_

#include <asm/mach-imx/sys_proto.h>

#define BT0CFG_LPBOOT_MASK 0x1
#define BT0CFG_DUALBOOT_MASK 0x2

enum bt_mode {
	LOW_POWER_BOOT,		/* LP_BT = 1 */
	DUAL_BOOT,		/* LP_BT = 0, DUAL_BT = 1 */
	SINGLE_BOOT		/* LP_BT = 0, DUAL_BT = 0 */
};

#endif
