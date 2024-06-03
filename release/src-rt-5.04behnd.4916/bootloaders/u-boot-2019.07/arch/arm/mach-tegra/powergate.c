// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014-2019, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <errno.h>

#include <asm/io.h>
#include <asm/types.h>

#include <asm/arch/powergate.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/pmc.h>

#define PWRGATE_TOGGLE 0x30
#define  PWRGATE_TOGGLE_START (1 << 8)

#define REMOVE_CLAMPING 0x34

#define PWRGATE_STATUS 0x38

static int tegra_powergate_set(enum tegra_powergate id, bool state)
{
	u32 value, mask = state ? (1 << id) : 0, old_mask;
	unsigned long start, timeout = 25;

	value = tegra_pmc_readl(PWRGATE_STATUS);
	old_mask = value & (1 << id);

	if (mask == old_mask)
		return 0;

	tegra_pmc_writel(PWRGATE_TOGGLE_START | id, PWRGATE_TOGGLE);

	start = get_timer(0);

	while (get_timer(start) < timeout) {
		value = tegra_pmc_readl(PWRGATE_STATUS);
		if ((value & (1 << id)) == mask)
			return 0;
	}

	return -ETIMEDOUT;
}

int tegra_powergate_power_on(enum tegra_powergate id)
{
	return tegra_powergate_set(id, true);
}

int tegra_powergate_power_off(enum tegra_powergate id)
{
	return tegra_powergate_set(id, false);
}

static int tegra_powergate_remove_clamping(enum tegra_powergate id)
{
	unsigned long value;

	/*
	 * The REMOVE_CLAMPING register has the bits for the PCIE and VDEC
	 * partitions reversed. This was originally introduced on Tegra20 but
	 * has since been carried forward for backwards-compatibility.
	 */
	if (id == TEGRA_POWERGATE_VDEC)
		value = 1 << TEGRA_POWERGATE_PCIE;
	else if (id == TEGRA_POWERGATE_PCIE)
		value = 1 << TEGRA_POWERGATE_VDEC;
	else
		value = 1 << id;

	tegra_pmc_writel(value, REMOVE_CLAMPING);

	return 0;
}

int tegra_powergate_sequence_power_up(enum tegra_powergate id,
				      enum periph_id periph)
{
	int err;

	reset_set_enable(periph, 1);

	err = tegra_powergate_power_on(id);
	if (err < 0)
		return err;

	clock_enable(periph);

	udelay(10);

	err = tegra_powergate_remove_clamping(id);
	if (err < 0)
		return err;

	udelay(10);

	reset_set_enable(periph, 0);

	return 0;
}
