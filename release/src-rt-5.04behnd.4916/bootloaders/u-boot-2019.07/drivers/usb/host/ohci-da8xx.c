// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Sughosh Ganu <urwithsughosh@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <generic-phy.h>
#include <reset.h>
#include "ohci.h"
#include <asm/arch/da8xx-usb.h>

struct da8xx_ohci {
	ohci_t ohci;
	struct clk *clocks;	/* clock list */
	struct phy phy;
	int clock_count;	/* number of clock in clock list */
};

static int usb_phy_on(void)
{
	unsigned long timeout;

	clrsetbits_le32(&davinci_syscfg_regs->cfgchip2,
			(CFGCHIP2_RESET | CFGCHIP2_PHYPWRDN |
			CFGCHIP2_OTGPWRDN | CFGCHIP2_OTGMODE |
			CFGCHIP2_REFFREQ | CFGCHIP2_USB1PHYCLKMUX),
			(CFGCHIP2_SESENDEN | CFGCHIP2_VBDTCTEN |
			CFGCHIP2_PHY_PLLON | CFGCHIP2_REFFREQ_24MHZ |
			CFGCHIP2_USB2PHYCLKMUX | CFGCHIP2_USB1SUSPENDM));

	/* wait until the usb phy pll locks */
	timeout = get_timer(0);
	while (get_timer(timeout) < 10) {
		if (readl(&davinci_syscfg_regs->cfgchip2) & CFGCHIP2_PHYCLKGD)
			return 1;
	}

	/* USB phy was not turned on */
	return 0;
}

static void usb_phy_off(void)
{
	/* Power down the on-chip PHY. */
	clrsetbits_le32(&davinci_syscfg_regs->cfgchip2,
			CFGCHIP2_PHY_PLLON | CFGCHIP2_USB1SUSPENDM,
			CFGCHIP2_PHYPWRDN | CFGCHIP2_OTGPWRDN |
			CFGCHIP2_RESET);
}

int usb_cpu_init(void)
{
	/* enable psc for usb2.0 */
	lpsc_on(DAVINCI_LPSC_USB20);

	/* enable psc for usb1.0 */
	lpsc_on(DAVINCI_LPSC_USB11);

	/* start the on-chip usb phy and its pll */
	if (usb_phy_on())
		return 0;

	return 1;
}

int usb_cpu_stop(void)
{
	usb_phy_off();

	/* turn off the usb clock and assert the module reset */
	lpsc_disable(DAVINCI_LPSC_USB11);
	lpsc_disable(DAVINCI_LPSC_USB20);

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}

#if CONFIG_IS_ENABLED(DM_USB)
static int ohci_da8xx_probe(struct udevice *dev)
{
	struct ohci_regs *regs = (struct ohci_regs *)devfdt_get_addr(dev);
	struct da8xx_ohci *priv = dev_get_priv(dev);
	int i, err, ret, clock_nb;

	err = 0;
	priv->clock_count = 0;
	clock_nb = dev_count_phandle_with_args(dev, "clocks", "#clock-cells");

	if (clock_nb < 0)
		return clock_nb;

	if (clock_nb > 0) {
		priv->clocks = devm_kcalloc(dev, clock_nb, sizeof(struct clk),
					    GFP_KERNEL);
		if (!priv->clocks)
			return -ENOMEM;

		for (i = 0; i < clock_nb; i++) {
			err = clk_get_by_index(dev, i, &priv->clocks[i]);
			if (err < 0)
				break;

			err = clk_enable(&priv->clocks[i]);
			if (err) {
				dev_err(dev, "failed to enable clock %d\n", i);
				clk_free(&priv->clocks[i]);
				goto clk_err;
			}
			priv->clock_count++;
		}
	}

	err = usb_cpu_init();

	if (err)
		goto clk_err;

	err = ohci_register(dev, regs);
	if (err)
		goto phy_err;

	return 0;

phy_err:
	ret = usb_cpu_stop();
	if (ret)
		dev_err(dev, "failed to shutdown usb phy\n");

clk_err:
	ret = clk_release_all(priv->clocks, priv->clock_count);
	if (ret)
		dev_err(dev, "failed to disable all clocks\n");

	return err;
}

static int ohci_da8xx_remove(struct udevice *dev)
{
	struct da8xx_ohci *priv = dev_get_priv(dev);
	int ret;

	ret = ohci_deregister(dev);
	if (ret)
		return ret;

	ret = usb_cpu_stop();
	if (ret)
		return ret;

	return clk_release_all(priv->clocks, priv->clock_count);
}

static const struct udevice_id da8xx_ohci_ids[] = {
	{ .compatible = "ti,da830-ohci" },
	{ }
};

U_BOOT_DRIVER(ohci_generic) = {
	.name	= "ohci-da8xx",
	.id	= UCLASS_USB,
	.of_match = da8xx_ohci_ids,
	.probe = ohci_da8xx_probe,
	.remove = ohci_da8xx_remove,
	.ops	= &ohci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct da8xx_ohci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA | DM_FLAG_OS_PREPARE,
};
#endif
