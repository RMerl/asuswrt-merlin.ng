// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3328.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/err.h>

int rockchip_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct sysreset_reg *offset = dev_get_priv(dev);
	unsigned long cru_base = (unsigned long)rockchip_get_cru();

	if (IS_ERR_VALUE(cru_base))
		return (int)cru_base;

	switch (type) {
	case SYSRESET_WARM:
		writel(0xeca8, cru_base + offset->glb_srst_snd_value);
		break;
	case SYSRESET_COLD:
		writel(0xfdb9, cru_base + offset->glb_srst_fst_value);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	return -EINPROGRESS;
}

static struct sysreset_ops rockchip_sysreset = {
	.request	= rockchip_sysreset_request,
};

U_BOOT_DRIVER(sysreset_rockchip) = {
	.name	= "rockchip_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &rockchip_sysreset,
};
