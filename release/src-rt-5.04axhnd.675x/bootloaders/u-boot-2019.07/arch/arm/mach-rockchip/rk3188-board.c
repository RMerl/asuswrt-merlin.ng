// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/pmu_rk3288.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <dm/pinctrl.h>

__weak int rk_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct rk3188_grf *grf;

	setup_boot_mode();
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(grf)) {
		pr_err("grf syscon returned %ld\n", PTR_ERR(grf));
	} else {
		/* enable noc remap to mimic legacy loaders */
		rk_clrsetreg(&grf->soc_con0,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT,
			NOC_REMAP_MASK << NOC_REMAP_SHIFT);
	}

	return rk_board_late_init();
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM)
	struct udevice *pinctrl;
	int ret;

	/*
	 * We need to implement sdcard iomux here for the further
	 * initialization, otherwise, it'll hit sdcard command sending
	 * timeout exception.
	 */
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_SDCARD);
	if (ret) {
		debug("%s: Failed to set up SD card\n", __func__);
		goto err;
	}

	return 0;
err:
	printf("board_init: Error %d\n", ret);

	/* No way to report error here */
	hang();

	return -1;
#else
	return 0;
#endif
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
