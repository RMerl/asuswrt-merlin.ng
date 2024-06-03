/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef _ASM_ARCH_DDR_RK3188_H
#define _ASM_ARCH_DDR_RK3188_H

#include <asm/arch-rockchip/ddr_rk3288.h>

/*
 * RK3188 Memory scheduler register map.
 */
struct rk3188_msch {
	u32 coreid;
	u32 revisionid;
	u32 ddrconf;
	u32 ddrtiming;
	u32 ddrmode;
	u32 readlatency;
};
check_member(rk3188_msch, readlatency, 0x0014);

#endif
