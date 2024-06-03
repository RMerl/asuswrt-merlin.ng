/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef _MX35_SYS_PROTO_H_
#define _MX35_SYS_PROTO_H_

#include <asm/mach-imx/sys_proto.h>

void mx3_setup_sdram_bank(u32 start_address, u32 ddr2_config, u32 row,
			  u32 col, u32 dsize, u32 refresh);

#endif
