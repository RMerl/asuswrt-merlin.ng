// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Pepperl+Fuchs
 * Simon Goldschmidt <simon.k.r.goldschmidt@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/reset_manager.h>
#include <linux/bitops.h>

struct socfpga_sysreset_data {
	void __iomem *rstmgr_base;
};

static int socfpga_sysreset_request(struct udevice *dev,
				    enum sysreset_t type)
{
	struct socfpga_sysreset_data *data = dev_get_priv(dev);

	switch (type) {
	case SYSRESET_WARM:
		writel(BIT(RSTMGR_CTRL_SWWARMRSTREQ_LSB),
		       data->rstmgr_base + RSTMGR_CTRL);
		break;
	case SYSRESET_COLD:
		writel(BIT(RSTMGR_CTRL_SWCOLDRSTREQ_LSB),
		       data->rstmgr_base + RSTMGR_CTRL);
		break;
	default:
		return -EPROTONOSUPPORT;
	}
	return -EINPROGRESS;
}

static int socfpga_sysreset_probe(struct udevice *dev)
{
	struct socfpga_sysreset_data *data = dev_get_priv(dev);

	data->rstmgr_base = dev_read_addr_ptr(dev_get_parent(dev));
	return 0;
}

static struct sysreset_ops socfpga_sysreset = {
	.request = socfpga_sysreset_request,
};

U_BOOT_DRIVER(sysreset_socfpga) = {
	.id	= UCLASS_SYSRESET,
	.name	= "socfpga_sysreset",
	.priv_auto	= sizeof(struct socfpga_sysreset_data),
	.ops	= &socfpga_sysreset,
	.probe	= socfpga_sysreset_probe,
};
