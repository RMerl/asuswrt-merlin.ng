// SPDX-License-Identifier: GPL-2.0+
/*
 * SAMSUNG EXYNOS5 USB HOST XHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
 *	Vikas Sajjan <vikas.sajjan@samsung.com>
 */

/*
 * This file is a conglomeration for DWC3-init sequence and further
 * exynos5 specific PHY-init sequence.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <usb.h>
#include <watchdog.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <asm/arch/xhci-exynos.h>
#include <asm/gpio.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>

#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

struct exynos_xhci_platdata {
	fdt_addr_t hcd_base;
	fdt_addr_t phy_base;
	struct gpio_desc vbus_gpio;
};

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */
struct exynos_xhci {
	struct usb_platdata usb_plat;
	struct xhci_ctrl ctrl;
	struct exynos_usb3_phy *usb3_phy;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
};

static int xhci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct exynos_xhci_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	unsigned int node;
	int depth;

	/*
	 * Get the base address for XHCI controller from the device node
	 */
	plat->hcd_base = devfdt_get_addr(dev);
	if (plat->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the XHCI register base address\n");
		return -ENXIO;
	}

	depth = 0;
	node = fdtdec_next_compatible_subnode(blob, dev_of_offset(dev),
				COMPAT_SAMSUNG_EXYNOS5_USB3_PHY, &depth);
	if (node <= 0) {
		debug("XHCI: Can't get device node for usb3-phy controller\n");
		return -ENODEV;
	}

	/*
	 * Get the base address for usbphy from the device node
	 */
	plat->phy_base = fdtdec_get_addr(blob, node, "reg");
	if (plat->phy_base == FDT_ADDR_T_NONE) {
		debug("Can't get the usbphy register address\n");
		return -ENXIO;
	}

	/* Vbus gpio */
	gpio_request_by_name(dev, "samsung,vbus-gpio", 0,
			     &plat->vbus_gpio, GPIOD_IS_OUT);

	return 0;
}

static void exynos5_usb3_phy_init(struct exynos_usb3_phy *phy)
{
	u32 reg;

	/* enabling usb_drd phy */
	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_EN);

	/* Reset USB 3.0 PHY */
	writel(0x0, &phy->phy_reg0);

	clrbits_le32(&phy->phy_param0,
			/* Select PHY CLK source */
			PHYPARAM0_REF_USE_PAD |
			/* Set Loss-of-Signal Detector sensitivity */
			PHYPARAM0_REF_LOSLEVEL_MASK);
	setbits_le32(&phy->phy_param0, PHYPARAM0_REF_LOSLEVEL);

	writel(0x0, &phy->phy_resume);

	/*
	 * Setting the Frame length Adj value[6:1] to default 0x20
	 * See xHCI 1.0 spec, 5.2.4
	 */
	setbits_le32(&phy->link_system,
			LINKSYSTEM_XHCI_VERSION_CONTROL |
			LINKSYSTEM_FLADJ(0x20));

	/* Set Tx De-Emphasis level */
	clrbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH_MASK);
	setbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH);

	setbits_le32(&phy->phy_batchg, PHYBATCHG_UTMI_CLKSEL);

	/* PHYTEST POWERDOWN Control */
	clrbits_le32(&phy->phy_test,
			PHYTEST_POWERDOWN_SSP |
			PHYTEST_POWERDOWN_HSP);

	/* UTMI Power Control */
	writel(PHYUTMI_OTGDISABLE, &phy->phy_utmi);

		/* Use core clock from main PLL */
	reg = PHYCLKRST_REFCLKSEL_EXT_REFCLK |
		/* Default 24Mhz crystal clock */
		PHYCLKRST_FSEL(FSEL_CLKSEL_24M) |
		PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF |
		PHYCLKRST_SSC_REFCLKSEL(0x88) |
		/* Force PortReset of PHY */
		PHYCLKRST_PORTRESET |
		/* Digital power supply in normal operating mode */
		PHYCLKRST_RETENABLEN |
		/* Enable ref clock for SS function */
		PHYCLKRST_REF_SSP_EN |
		/* Enable spread spectrum */
		PHYCLKRST_SSC_EN |
		/* Power down HS Bias and PLL blocks in suspend mode */
		PHYCLKRST_COMMONONN;

	writel(reg, &phy->phy_clk_rst);

	/* giving time to Phy clock to settle before resetting */
	udelay(10);

	reg &= ~PHYCLKRST_PORTRESET;
	writel(reg, &phy->phy_clk_rst);
}

static void exynos5_usb3_phy_exit(struct exynos_usb3_phy *phy)
{
	setbits_le32(&phy->phy_utmi,
			PHYUTMI_OTGDISABLE |
			PHYUTMI_FORCESUSPEND |
			PHYUTMI_FORCESLEEP);

	clrbits_le32(&phy->phy_clk_rst,
			PHYCLKRST_REF_SSP_EN |
			PHYCLKRST_SSC_EN |
			PHYCLKRST_COMMONONN);

	/* PHYTEST POWERDOWN Control to remove leakage current */
	setbits_le32(&phy->phy_test,
			PHYTEST_POWERDOWN_SSP |
			PHYTEST_POWERDOWN_HSP);

	/* disabling usb_drd phy */
	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_DISABLE);
}

static int exynos_xhci_core_init(struct exynos_xhci *exynos)
{
	int ret;

	exynos5_usb3_phy_init(exynos->usb3_phy);

	ret = dwc3_core_init(exynos->dwc3_reg);
	if (ret) {
		debug("failed to initialize core\n");
		return -EINVAL;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(exynos->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return 0;
}

static void exynos_xhci_core_exit(struct exynos_xhci *exynos)
{
	exynos5_usb3_phy_exit(exynos->usb3_phy);
}

static int xhci_usb_probe(struct udevice *dev)
{
	struct exynos_xhci_platdata *plat = dev_get_platdata(dev);
	struct exynos_xhci *ctx = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	ctx->hcd = (struct xhci_hccr *)plat->hcd_base;
	ctx->usb3_phy = (struct exynos_usb3_phy *)plat->phy_base;
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);
	hcor = (struct xhci_hcor *)((uint32_t)ctx->hcd +
			HC_LENGTH(xhci_readl(&ctx->hcd->cr_capbase)));

	/* setup the Vbus gpio here */
	if (dm_gpio_is_valid(&plat->vbus_gpio))
		dm_gpio_set_value(&plat->vbus_gpio, 1);

	ret = exynos_xhci_core_init(ctx);
	if (ret) {
		puts("XHCI: failed to initialize controller\n");
		return -EINVAL;
	}

	return xhci_register(dev, ctx->hcd, hcor);
}

static int xhci_usb_remove(struct udevice *dev)
{
	struct exynos_xhci *ctx = dev_get_priv(dev);
	int ret;

	ret = xhci_deregister(dev);
	if (ret)
		return ret;
	exynos_xhci_core_exit(ctx);

	return 0;
}

static const struct udevice_id xhci_usb_ids[] = {
	{ .compatible = "samsung,exynos5250-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name	= "xhci_exynos",
	.id	= UCLASS_USB,
	.of_match = xhci_usb_ids,
	.ofdata_to_platdata = xhci_usb_ofdata_to_platdata,
	.probe = xhci_usb_probe,
	.remove = xhci_usb_remove,
	.ops	= &xhci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct exynos_xhci_platdata),
	.priv_auto_alloc_size = sizeof(struct exynos_xhci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
