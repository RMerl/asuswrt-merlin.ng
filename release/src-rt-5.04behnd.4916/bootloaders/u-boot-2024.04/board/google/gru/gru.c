// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/misc.h>

#define GRF_IO_VSEL_BT656_SHIFT 0
#define GRF_IO_VSEL_AUDIO_SHIFT 1
#define PMUGRF_CON0_VSEL_SHIFT 8
#define PMUGRF_CON0_VOL_SHIFT 9

#ifdef CONFIG_SPL_BUILD
/* provided to defeat compiler optimisation in board_init_f() */
void gru_dummy_function(int i)
{
}

int board_early_init_f(void)
{
# if defined(CONFIG_TARGET_CHROMEBOOK_BOB) || defined(CONFIG_TARGET_CHROMEBOOK_KEVIN)
	int sum, i;

	/*
	 * Add a delay and ensure that the compiler does not optimise this out.
	 * This is needed since the power rails tail a while to turn on, and
	 * we get garbage serial output otherwise.
	 */
	sum = 0;
	for (i = 0; i < 150000; i++)
		sum += i;
	gru_dummy_function(sum);
#endif /* CONFIG_TARGET_CHROMEBOOK_BOB */

	return 0;
}
#endif

#ifndef CONFIG_SPL_BUILD
int board_early_init_r(void)
{
	struct udevice *clk;
	int ret;

	/*
	 * This init is done in SPL, but when chain-loading U-Boot SPL will
	 * have been skipped. Allow the clock driver to check if it needs
	 * setting up.
	 */
	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(clk_rk3399), &clk);
	if (ret) {
		debug("%s: CLK init failed: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
#endif

static void setup_iodomain(void)
{
	struct rk3399_grf_regs *grf =
	   syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	struct rk3399_pmugrf_regs *pmugrf =
	   syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);

	/* BT656 and audio is in 1.8v domain */
	rk_setreg(&grf->io_vsel, (1 << GRF_IO_VSEL_BT656_SHIFT |
				  1 << GRF_IO_VSEL_AUDIO_SHIFT));

	/*
	 * Set GPIO1 1.8v/3.0v source select to PMU1830_VOL
	 * and explicitly configure that PMU1830_VOL to be 1.8V
	 */
	rk_setreg(&pmugrf->soc_con0, (1 << PMUGRF_CON0_VSEL_SHIFT |
				      1 << PMUGRF_CON0_VOL_SHIFT));
}

int misc_init_r(void)
{
	const u32 cpuid_offset = 0x7;
	const u32 cpuid_length = 0x10;
	u8 cpuid[cpuid_length];
	int ret;

	setup_iodomain();

	ret = rockchip_cpuid_from_efuse(cpuid_offset, cpuid_length, cpuid);
	if (ret)
		return ret;

	ret = rockchip_cpuid_set(cpuid, cpuid_length);
	if (ret)
		return ret;

	ret = rockchip_setup_macaddr();

	return ret;
}
