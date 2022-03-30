/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 */
#ifndef __SYS_PROTO_IMX7_
#define __SYS_PROTO_IMX7_

#include <asm/mach-imx/sys_proto.h>

void set_wdog_reset(struct wdog_regs *wdog);
enum boot_device get_boot_device(void);

#endif /* __SYS_PROTO_IMX7_ */
