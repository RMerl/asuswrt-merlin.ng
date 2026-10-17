// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <linux/err.h>
#include <asm/arch-tegra/pmc.h>

static int tegra_sysreset_request(struct udevice *dev,
				  enum sysreset_t type)
{
	u32 value;

	switch (type) {
	case SYSRESET_WARM:
	case SYSRESET_COLD:
		/* resets everything but scratch 0 and reset status */
		value = tegra_pmc_readl(PMC_CNTRL);
		value |= PMC_CNTRL_MAIN_RST;
		tegra_pmc_writel(value, PMC_CNTRL);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops tegra_sysreset = {
	.request = tegra_sysreset_request,
};

U_BOOT_DRIVER(sysreset_tegra) = {
	.id	= UCLASS_SYSRESET,
	.name	= "sysreset_tegra",
	.ops	= &tegra_sysreset,
};

/* Link to Tegra PMC once there is a driver */
U_BOOT_DRVINFO(sysreset_tegra) = {
	.name = "sysreset_tegra"
};
