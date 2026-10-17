// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 *
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dt-bindings/clock/agilex-clock.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong cm_get_rate_dm(u32 id)
{
	struct udevice *dev;
	struct clk clk;
	ulong rate;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(socfpga_agilex_clk),
					  &dev);
	if (ret)
		return 0;

	clk.id = id;
	ret = clk_request(dev, &clk);
	if (ret < 0)
		return 0;

	rate = clk_get_rate(&clk);

	if ((rate == (unsigned long)-ENOSYS) ||
	    (rate == (unsigned long)-ENXIO) ||
	    (rate == (unsigned long)-EIO)) {
		debug("%s id %u: clk_get_rate err: %ld\n",
		      __func__, id, rate);
		return 0;
	}

	return rate;
}

static u32 cm_get_rate_dm_khz(u32 id)
{
	return cm_get_rate_dm(id) / 1000;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	return cm_get_rate_dm(AGILEX_MPU_CLK);
}

unsigned int cm_get_l4_sys_free_clk_hz(void)
{
	return cm_get_rate_dm(AGILEX_L4_SYS_FREE_CLK);
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU       %10d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_MPU_CLK));
	printf("L4 Main	    %8d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_L4_MAIN_CLK));
	printf("L4 sys free %8d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_L4_SYS_FREE_CLK));
	printf("L4 MP       %8d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_L4_MP_CLK));
	printf("L4 SP       %8d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_L4_SP_CLK));
	printf("SDMMC       %8d kHz\n",
	       cm_get_rate_dm_khz(AGILEX_SDMMC_CLK));
}
