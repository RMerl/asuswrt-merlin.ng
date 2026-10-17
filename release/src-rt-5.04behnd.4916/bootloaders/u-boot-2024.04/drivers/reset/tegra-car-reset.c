// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>

static int tegra_car_reset_request(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	/* PERIPH_ID_COUNT varies per SoC */
	if (reset_ctl->id >= PERIPH_ID_COUNT)
		return -EINVAL;

	return 0;
}

static int tegra_car_reset_assert(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	reset_set_enable(reset_ctl->id, 1);

	return 0;
}

static int tegra_car_reset_deassert(struct reset_ctl *reset_ctl)
{
	debug("%s(reset_ctl=%p) (dev=%p, id=%lu)\n", __func__, reset_ctl,
	      reset_ctl->dev, reset_ctl->id);

	reset_set_enable(reset_ctl->id, 0);

	return 0;
}

struct reset_ops tegra_car_reset_ops = {
	.request = tegra_car_reset_request,
	.rst_assert = tegra_car_reset_assert,
	.rst_deassert = tegra_car_reset_deassert,
};

U_BOOT_DRIVER(tegra_car_reset) = {
	.name = "tegra_car_reset",
	.id = UCLASS_RESET,
	.ops = &tegra_car_reset_ops,
};
