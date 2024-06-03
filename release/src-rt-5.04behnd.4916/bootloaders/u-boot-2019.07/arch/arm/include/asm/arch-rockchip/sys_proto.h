/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co.,Ltd
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#ifdef CONFIG_ROCKCHIP_RK3288
#include <asm/armv7.h>

static void configure_l2ctlr(void)
{
	uint32_t l2ctlr;

	l2ctlr = read_l2ctlr();
	l2ctlr &= 0xfffc0000; /* clear bit0~bit17 */

	/*
	* Data RAM write latency: 2 cycles
	* Data RAM read latency: 2 cycles
	* Data RAM setup latency: 1 cycle
	* Tag RAM write latency: 1 cycle
	* Tag RAM read latency: 1 cycle
	* Tag RAM setup latency: 1 cycle
	*/
	l2ctlr |= (1 << 3 | 1 << 0);
	write_l2ctlr(l2ctlr);
}
#endif /* CONFIG_ROCKCHIP_RK3288 */

/* provided to defeat compiler optimisation in board_init_f() */
void gru_dummy_function(int i);

#endif /* _ASM_ARCH_SYS_PROTO_H */
