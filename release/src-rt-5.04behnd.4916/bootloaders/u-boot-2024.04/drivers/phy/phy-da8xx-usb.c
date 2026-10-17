// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on the DA8xx "glue layer" code.
 * Copyright (c) 2008-2019, MontaVista Software, Inc. <source@mvista.com>
 *
 * DT support added by: Adam Ford <aford173@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/arch/hardware.h>
#include <asm/arch/da8xx-usb.h>
#include <asm/io.h>
#include <generic-phy.h>

static int da8xx_usb_phy_power_on(struct phy *phy)
{
	unsigned long timeout;

	clrsetbits_le32(&davinci_syscfg_regs->cfgchip2,
			CFGCHIP2_RESET | CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN |
			CFGCHIP2_OTGMODE | CFGCHIP2_REFFREQ,
			CFGCHIP2_SESENDEN | CFGCHIP2_VBDTCTEN |
			CFGCHIP2_PHY_PLLON | CFGCHIP2_REFFREQ_24MHZ);

	/* wait until the usb phy pll locks */
	timeout = get_timer(0);
	while (get_timer(timeout) < 10) {
		if (readl(&davinci_syscfg_regs->cfgchip2) & CFGCHIP2_PHYCLKGD)
			return 0;
	}

	debug("Phy was not turned on\n");

	return -ENODEV;
}

static int da8xx_usb_phy_power_off(struct phy *phy)
{
	clrsetbits_le32(&davinci_syscfg_regs->cfgchip2,
			CFGCHIP2_PHY_PLLON,
			CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN);

	return 0;
}

static const struct udevice_id da8xx_phy_ids[] = {
	{ .compatible = "ti,da830-usb-phy" },
	{ }
};

static struct phy_ops da8xx_phy_ops = {
	.power_on = da8xx_usb_phy_power_on,
	.power_off = da8xx_usb_phy_power_off,
};

U_BOOT_DRIVER(da8xx_phy) = {
	.name	= "da8xx-usb-phy",
	.id	= UCLASS_PHY,
	.of_match = da8xx_phy_ids,
	.ops = &da8xx_phy_ops,
};
