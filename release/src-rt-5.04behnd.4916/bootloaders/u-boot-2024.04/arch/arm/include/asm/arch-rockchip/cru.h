/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * (C) Copyright 2019 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#ifndef _ROCKCHIP_CLOCK_H
#define _ROCKCHIP_CLOCK_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#if defined(CONFIG_ROCKCHIP_RK3288)
# include <asm/arch-rockchip/cru_rk3288.h>
#elif defined(CONFIG_ROCKCHIP_RK3399)
# include <asm/arch-rockchip/cru_rk3399.h>
#elif defined(CONFIG_ROCKCHIP_RK3568)
#include <asm/arch-rockchip/cru_rk3568.h>
#endif

/* CRU_GLB_RST_ST */
enum {
	GLB_POR_RST,
	FST_GLB_RST_ST		= BIT(0),
	SND_GLB_RST_ST		= BIT(1),
	FST_GLB_TSADC_RST_ST	= BIT(2),
	SND_GLB_TSADC_RST_ST	= BIT(3),
	FST_GLB_WDT_RST_ST	= BIT(4),
	SND_GLB_WDT_RST_ST	= BIT(5),
};

#define MHz		1000000

char *get_reset_cause(void);

#endif /* _ROCKCHIP_CLOCK_H */
