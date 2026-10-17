// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	struct rk3399_grf_regs *grf =
	    syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	/**
	 * Some SSD's to work on rock960 would require explicit
	 * domain voltage change, so BT565 is in 1.8v domain
	 */
	rk_setreg(&grf->io_vsel, BIT(0));

	return 0;
}
#endif
