// SPDX-License-Identifier: GPL-2.0+
/*
 * Cadence Tensilica xtfpga system reset driver.
 *
 * (C) Copyright 2016 Cadence Design Systems Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>

static int xtfpga_reset_request(struct udevice *dev, enum sysreset_t type)
{
	switch (type) {
	case SYSRESET_COLD:
		writel(CONFIG_SYS_FPGAREG_RESET_CODE,
		       CONFIG_SYS_FPGAREG_RESET);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops xtfpga_sysreset_ops = {
	.request	= xtfpga_reset_request,
};

U_BOOT_DRIVER(xtfpga_sysreset) = {
	.name	= "xtfpga_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &xtfpga_sysreset_ops,
};
