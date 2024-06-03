// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Atmel Semiconductor <www.atmel.com>
 * Written-by: Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

#include "ehci.h"

#if !CONFIG_IS_ENABLED(DM_USB)

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	/* Enable UTMI PLL */
	if (at91_upll_clk_enable())
		return -1;

	/* Enable USB Host clock */
	at91_periph_clk_enable(ATMEL_ID_UHPHS);

	*hccr = (struct ehci_hccr *)ATMEL_BASE_EHCI;
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	/* Disable USB Host Clock */
	at91_periph_clk_disable(ATMEL_ID_UHPHS);

	/* Disable UTMI PLL */
	if (at91_upll_clk_disable())
		return -1;

	return 0;
}

#else

struct ehci_atmel_priv {
	struct ehci_ctrl ehci;
};

static int ehci_atmel_enable_clk(struct udevice *dev)
{
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_free(&clk);

	return 0;
}

static int ehci_atmel_probe(struct udevice *dev)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	fdt_addr_t hcd_base;
	int ret;

	ret = ehci_atmel_enable_clk(dev);
	if (ret) {
		debug("Failed to enable USB Host clock\n");
		return ret;
	}

	/*
	 * Get the base address for EHCI controller from the device node
	 */
	hcd_base = devfdt_get_addr(dev);
	if (hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the EHCI register base address\n");
		return -ENXIO;
	}

	hccr = (struct ehci_hccr *)hcd_base;
	hcor = (struct ehci_hcor *)
		((u32)hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	debug("echi-atmel: init hccr %x and hcor %x hc_length %d\n",
	      (u32)hccr, (u32)hcor,
	      (u32)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "atmel,at91sam9g45-ehci", },
	{ }
};

U_BOOT_DRIVER(ehci_atmel) = {
	.name		= "ehci_atmel",
	.id		= UCLASS_USB,
	.of_match	= ehci_usb_ids,
	.probe		= ehci_atmel_probe,
	.remove		= ehci_deregister,
	.ops		= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_atmel_priv),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};

#endif
