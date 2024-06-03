/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Helmut Raiger, HALE electronic GmbH, helmut.raiger@hale.at
 */

#ifndef _MX31_SYS_PROTO_H_
#define _MX31_SYS_PROTO_H_

#include <asm/mach-imx/sys_proto.h>

struct mxc_weimcs {
	u32 upper;
	u32 lower;
	u32 additional;
};

void mxc_setup_weimcs(int cs, const struct mxc_weimcs *weimcs);
int mxc_mmc_init(bd_t *bis);
#endif
