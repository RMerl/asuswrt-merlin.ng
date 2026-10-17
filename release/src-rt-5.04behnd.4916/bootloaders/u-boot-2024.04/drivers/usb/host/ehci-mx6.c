// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clk.h>
#include <log.h>
#include <usb.h>
#include <errno.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <usb/ehci-ci.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sys_proto.h>
#include <dm.h>
#include <asm/mach-types.h>
#include <power/regulator.h>
#include <linux/usb/otg.h>
#include <linux/usb/phy.h>

#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

#define USB_OTGREGS_OFFSET	0x000
#define USB_H1REGS_OFFSET	0x200
#define USB_H2REGS_OFFSET	0x400
#define USB_H3REGS_OFFSET	0x600
#define USB_OTHERREGS_OFFSET	0x800

#define USB_H1_CTRL_OFFSET	0x04

#define USBPHY_CTRL				0x00000030
#define USBPHY_CTRL_SET				0x00000034
#define USBPHY_CTRL_CLR				0x00000038
#define USBPHY_CTRL_TOG				0x0000003c

#define USBPHY_PWD				0x00000000
#define USBPHY_CTRL_SFTRST			0x80000000
#define USBPHY_CTRL_CLKGATE			0x40000000
#define USBPHY_CTRL_ENUTMILEVEL3		0x00008000
#define USBPHY_CTRL_ENUTMILEVEL2		0x00004000
#define USBPHY_CTRL_OTG_ID			0x08000000

#define ANADIG_USB2_CHRG_DETECT_EN_B		0x00100000
#define ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B	0x00080000

#define ANADIG_USB2_PLL_480_CTRL_BYPASS		0x00010000
#define ANADIG_USB2_PLL_480_CTRL_ENABLE		0x00002000
#define ANADIG_USB2_PLL_480_CTRL_POWER		0x00001000
#define ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS	0x00000040

#define USBNC_OFFSET		0x200
#define USBNC_PHY_STATUS_OFFSET	0x23C
#define USBNC_PHYSTATUS_ID_DIG	(1 << 4) /* otg_id status */
#define USBNC_PHYCFG2_ACAENB	(1 << 4) /* otg_id detection enable */
#define UCTRL_PWR_POL		(1 << 9) /* OTG Polarity of Power Pin */
#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP           (1 << 0) /* controller run/stop */
#define UCMD_RESET		(1 << 1) /* controller reset */

/* If this is not defined, assume MX6/MX7/MX8M SoC default */
#ifndef CFG_MXC_USB_PORTSC
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif

/* Base address for this IP block is 0x02184800 */
struct usbnc_regs {
	u32 ctrl[4]; /* otg/host1-3 */
	u32 uh2_hsic_ctrl;
	u32 uh3_hsic_ctrl;
	u32 otg_phy_ctrl_0;
	u32 uh1_phy_ctrl_0;
	u32 reserve1[4];
	u32 phy_cfg1;
	u32 phy_cfg2;
	u32 reserve2;
	u32 phy_status;
	u32 reserve3[4];
	u32 adp_cfg1;
	u32 adp_cfg2;
	u32 adp_status;
};

#if defined(CONFIG_MX6) && !defined(CONFIG_PHY)
static void usb_power_config_mx6(struct anatop_regs __iomem *anatop,
				 int anatop_bits_index)
{
	void __iomem *chrg_detect;
	void __iomem *pll_480_ctrl_clr;
	void __iomem *pll_480_ctrl_set;

	if (!is_mx6())
		return;

	switch (anatop_bits_index) {
	case 0:
		chrg_detect = &anatop->usb1_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb1_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb1_pll_480_ctrl_set;
		break;
	case 1:
		chrg_detect = &anatop->usb2_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb2_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb2_pll_480_ctrl_set;
		break;
	default:
		return;
	}
	/*
	 * Some phy and power's special controls
	 * 1. The external charger detector needs to be disabled
	 * or the signal at DP will be poor
	 * 2. The PLL's power and output to usb
	 * is totally controlled by IC, so the Software only needs
	 * to enable them at initializtion.
	 */
	writel(ANADIG_USB2_CHRG_DETECT_EN_B |
		     ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B,
		     chrg_detect);

	writel(ANADIG_USB2_PLL_480_CTRL_BYPASS,
		     pll_480_ctrl_clr);

	writel(ANADIG_USB2_PLL_480_CTRL_ENABLE |
		     ANADIG_USB2_PLL_480_CTRL_POWER |
		     ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS,
		     pll_480_ctrl_set);
}
#else
static void __maybe_unused
usb_power_config_mx6(void *anatop, int anatop_bits_index) { }
#endif

#if defined(CONFIG_MX7) && !defined(CONFIG_PHY)
static void usb_power_config_mx7(struct usbnc_regs *usbnc)
{
	void __iomem *phy_cfg2 = (void __iomem *)(&usbnc->phy_cfg2);

	if (!is_mx7())
		return;

	/*
	 * Clear the ACAENB to enable usb_otg_id detection,
	 * otherwise it is the ACA detection enabled.
	 */
	clrbits_le32(phy_cfg2, USBNC_PHYCFG2_ACAENB);
}
#else
static void __maybe_unused
usb_power_config_mx7(void *usbnc) { }
#endif

#if defined(CONFIG_MX7ULP) && !defined(CONFIG_PHY)
static void usb_power_config_mx7ulp(struct usbphy_regs __iomem *usbphy)
{
	if (!is_mx7ulp())
		return;

	writel(ANADIG_USB2_CHRG_DETECT_EN_B |
	       ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B,
	       &usbphy->usb1_chrg_detect);

	scg_enable_usb_pll(true);
}
#else
static void __maybe_unused
usb_power_config_mx7ulp(void *usbphy) { }
#endif

#if defined(CONFIG_MX6) || defined(CONFIG_MX7ULP) || defined(CONFIG_IMXRT)
static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
#if defined(USB_PHY1_BASE_ADDR)
	USB_PHY1_BASE_ADDR,
#endif
};

#if !defined(CONFIG_PHY)
static void usb_internal_phy_clock_gate(void __iomem *phy_reg, int on)
{
	phy_reg += on ? USBPHY_CTRL_CLR : USBPHY_CTRL_SET;
	writel(USBPHY_CTRL_CLKGATE, phy_reg);
}

/* Return 0 : host node, <>0 : device mode */
static int usb_phy_enable(struct usb_ehci *ehci, void __iomem *phy_reg)
{
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;
	int ret;

	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	clrbits_le32(usb_cmd, UCMD_RUN_STOP);
	ret = wait_for_bit_le32(usb_cmd, UCMD_RUN_STOP, false, 10000, false);
	if (ret)
		return ret;

	setbits_le32(usb_cmd, UCMD_RESET);
	ret = wait_for_bit_le32(usb_cmd, UCMD_RESET, false, 10000, false);
	if (ret)
		return ret;

	/* Reset USBPHY module */
	setbits_le32(phy_ctrl, USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	clrbits_le32(phy_ctrl, USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Power up the PHY */
	writel(0, phy_reg + USBPHY_PWD);
	/* enable FS/LS device */
	setbits_le32(phy_ctrl, USBPHY_CTRL_ENUTMILEVEL2 |
			USBPHY_CTRL_ENUTMILEVEL3);

	return 0;
}
#endif

int usb_phy_mode(int port)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	u32 val;

	phy_reg = (void __iomem *)phy_bases[port];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);

	val = readl(phy_ctrl);

	if (val & USBPHY_CTRL_OTG_ID)
		return USB_INIT_DEVICE;
	else
		return USB_INIT_HOST;
}

#elif defined(CONFIG_MX7)
int usb_phy_mode(int port)
{
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * port) + USBNC_OFFSET);
	void __iomem *status = (void __iomem *)(&usbnc->phy_status);
	u32 val;

	val = readl(status);

	if (val & USBNC_PHYSTATUS_ID_DIG)
		return USB_INIT_DEVICE;
	else
		return USB_INIT_HOST;
}
#endif

#if !defined(CONFIG_PHY)
/* Should be done in the MXS PHY driver */
static void usb_oc_config(struct usbnc_regs *usbnc, int index)
{
	void __iomem *ctrl = (void __iomem *)(&usbnc->ctrl[index]);

	setbits_le32(ctrl, UCTRL_OVER_CUR_POL);

	setbits_le32(ctrl, UCTRL_OVER_CUR_DIS);

	/* Set power polarity to high active */
#ifdef CONFIG_MXC_USB_OTG_HACTIVE
	setbits_le32(ctrl, UCTRL_PWR_POL);
#else
	clrbits_le32(ctrl, UCTRL_PWR_POL);
#endif
}
#endif

#if !CONFIG_IS_ENABLED(DM_USB)
/**
 * board_usb_phy_mode - override usb phy mode
 * @port:	usb host/otg port
 *
 * Target board specific, override usb_phy_mode.
 * When usb-otg is used as usb host port, iomux pad usb_otg_id can be
 * left disconnected in this case usb_phy_mode will not be able to identify
 * the phy mode that usb port is used.
 * Machine file overrides board_usb_phy_mode.
 *
 * Return: USB_INIT_DEVICE or USB_INIT_HOST
 */
int __weak board_usb_phy_mode(int port)
{
	return usb_phy_mode(port);
}

/**
 * board_ehci_hcd_init - set usb vbus voltage
 * @port:      usb otg port
 *
 * Target board specific, setup iomux pad to setup supply vbus voltage
 * for usb otg port. Machine board file overrides board_ehci_hcd_init
 *
 * Return: 0 Success
 */
int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

/**
 * board_ehci_power - enables/disables usb vbus voltage
 * @port:      usb otg port
 * @on:        on/off vbus voltage
 *
 * Enables/disables supply vbus voltage for usb otg port.
 * Machine board file overrides board_ehci_power
 *
 * Return: 0 Success
 */
int __weak board_ehci_power(int port, int on)
{
	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	enum usb_init_type type;
#if defined(CONFIG_MX6) || defined(CONFIG_IMXRT)
	u32 controller_spacing = 0x200;
	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			USB_OTHERREGS_OFFSET);
#elif defined(CONFIG_MX7)
	u32 controller_spacing = 0x10000;
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * index) + USBNC_OFFSET);
#elif defined(CONFIG_MX7ULP)
	u32 controller_spacing = 0x10000;
	struct usbphy_regs __iomem *usbphy =
		(struct usbphy_regs __iomem *)USB_PHY0_BASE_ADDR;
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			(0x10000 * index) + USBNC_OFFSET);
#endif
	struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR +
		(controller_spacing * index));
	int ret;

	if (index > 3)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_IMX_MODULE_FUSE)) {
		if (usb_fused((ulong)ehci)) {
			printf("SoC fuse indicates USB@0x%lx is unavailable.\n",
			       (ulong)ehci);
			return	-ENODEV;
		}
	}

	enable_usboh3_clk(1);
	mdelay(1);

	/* Do board specific initialization */
	ret = board_ehci_hcd_init(index);
	if (ret) {
		enable_usboh3_clk(0);
		return ret;
	}

#if defined(CONFIG_MX6) || defined(CONFIG_IMXRT)
	usb_power_config_mx6(anatop, index);
#elif defined (CONFIG_MX7)
	usb_power_config_mx7(usbnc);
#elif defined (CONFIG_MX7ULP)
	usb_power_config_mx7ulp(usbphy);
#endif

	usb_oc_config(usbnc, index);

#if defined(CONFIG_MX6) || defined(CONFIG_MX7ULP) || defined(CONFIG_IMXRT)
	if (index < ARRAY_SIZE(phy_bases)) {
		usb_internal_phy_clock_gate((void __iomem *)phy_bases[index], 1);
		usb_phy_enable(ehci, (void __iomem *)phy_bases[index]);
	}
#endif

	type = board_usb_phy_mode(index);

	if (hccr && hcor) {
		*hccr = (struct ehci_hccr *)((uintptr_t)&ehci->caplength);
		*hcor = (struct ehci_hcor *)((uintptr_t)*hccr +
				HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	}

	if ((type == init) || (type == USB_INIT_DEVICE))
		board_ehci_power(index, (type == USB_INIT_DEVICE) ? 0 : 1);
	if (type != init)
		return -ENODEV;
	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel(CFG_MXC_USB_PORTSC, &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}
#else
struct ehci_mx6_priv_data {
	struct ehci_ctrl ctrl;
	struct usb_ehci *ehci;
	struct udevice *vbus_supply;
	struct clk clk;
	struct phy phy;
	enum usb_init_type init_type;
	enum usb_phy_interface phy_type;
#if !defined(CONFIG_PHY)
	int portnr;
	void __iomem *phy_addr;
	void __iomem *misc_addr;
	void __iomem *anatop_addr;
#endif
};

static u32 mx6_portsc(enum usb_phy_interface phy_type)
{
	switch (phy_type) {
	case USBPHY_INTERFACE_MODE_UTMI:
		return PORT_PTS_UTMI;
	case USBPHY_INTERFACE_MODE_UTMIW:
		return PORT_PTS_UTMI | PORT_PTS_PTW;
	case USBPHY_INTERFACE_MODE_ULPI:
		return PORT_PTS_ULPI;
	case USBPHY_INTERFACE_MODE_SERIAL:
		return PORT_PTS_SERIAL;
	case USBPHY_INTERFACE_MODE_HSIC:
		return PORT_PTS_HSIC;
	default:
		return CFG_MXC_USB_PORTSC;
	}
}

static int mx6_init_after_reset(struct ehci_ctrl *dev)
{
	struct ehci_mx6_priv_data *priv = dev->priv;
	enum usb_init_type type = priv->init_type;
	struct usb_ehci *ehci = priv->ehci;

#if !defined(CONFIG_PHY)
	usb_power_config_mx6(priv->anatop_addr, priv->portnr);
	usb_power_config_mx7(priv->misc_addr);
	usb_power_config_mx7ulp(priv->phy_addr);

	usb_oc_config(priv->misc_addr, priv->portnr);

#if defined(CONFIG_MX6) || defined(CONFIG_MX7ULP)
	usb_internal_phy_clock_gate(priv->phy_addr, 1);
	usb_phy_enable(ehci, priv->phy_addr);
#endif
#endif

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply) {
		int ret;
		ret = regulator_set_enable(priv->vbus_supply,
					   (type == USB_INIT_DEVICE) ?
					   false : true);
		if (ret && ret != -ENOSYS) {
			printf("Error enabling VBUS supply (ret=%i)\n", ret);
			return ret;
		}
	}
#endif

	if (type == USB_INIT_DEVICE)
		return 0;

	setbits_le32(&ehci->usbmode, CM_HOST);
	writel(mx6_portsc(priv->phy_type), &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

static const struct ehci_ops mx6_ehci_ops = {
	.init_after_reset = mx6_init_after_reset
};

static int ehci_usb_phy_mode(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	void *__iomem addr = dev_read_addr_ptr(dev);
	void *__iomem phy_ctrl, *__iomem phy_status;
	const void *blob = gd->fdt_blob;
	int offset = dev_of_offset(dev), phy_off;
	u32 val;

	/*
	 * About fsl,usbphy, Refer to
	 * Documentation/devicetree/bindings/usb/ci-hdrc-usb2.txt.
	 */
	if (is_mx6() || is_mx7ulp() || is_imxrt()) {
		phy_off = fdtdec_lookup_phandle(blob,
						offset,
						"fsl,usbphy");
		if (phy_off < 0)
			return -EINVAL;

		addr = (void __iomem *)fdtdec_get_addr(blob, phy_off,
						       "reg");
		if ((fdt_addr_t)addr == FDT_ADDR_T_NONE)
			return -EINVAL;

		phy_ctrl = (void __iomem *)(addr + USBPHY_CTRL);
		val = readl(phy_ctrl);

		if (val & USBPHY_CTRL_OTG_ID)
			plat->init_type = USB_INIT_DEVICE;
		else
			plat->init_type = USB_INIT_HOST;
	} else if (is_mx7() || is_imx8mm() || is_imx8mn() || is_imx93()) {
		phy_status = (void __iomem *)(addr +
					      USBNC_PHY_STATUS_OFFSET);
		val = readl(phy_status);

		if (val & USBNC_PHYSTATUS_ID_DIG)
			plat->init_type = USB_INIT_DEVICE;
		else
			plat->init_type = USB_INIT_HOST;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int ehci_usb_of_to_plat(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	enum usb_dr_mode dr_mode;

	dr_mode = usb_get_dr_mode(dev_ofnode(dev));

	switch (dr_mode) {
	case USB_DR_MODE_HOST:
		plat->init_type = USB_INIT_HOST;
		break;
	case USB_DR_MODE_PERIPHERAL:
		plat->init_type = USB_INIT_DEVICE;
		break;
	default:
		plat->init_type = USB_INIT_UNKNOWN;
	};

	return 0;
}

static int mx6_parse_dt_addrs(struct udevice *dev)
{
#if !defined(CONFIG_PHY)
	struct ehci_mx6_priv_data *priv = dev_get_priv(dev);
	int phy_off, misc_off;
	const void *blob = gd->fdt_blob;
	int offset = dev_of_offset(dev);
	void *__iomem addr;

	phy_off = fdtdec_lookup_phandle(blob, offset, "fsl,usbphy");
	if (phy_off < 0) {
		phy_off = fdtdec_lookup_phandle(blob, offset, "phys");
		if (phy_off < 0)
			return -EINVAL;
	}

	misc_off = fdtdec_lookup_phandle(blob, offset, "fsl,usbmisc");
	if (misc_off < 0)
		return -EINVAL;

	addr = (void __iomem *)fdtdec_get_addr(blob, phy_off, "reg");
	if ((fdt_addr_t)addr == FDT_ADDR_T_NONE)
		addr = NULL;

	priv->phy_addr = addr;

	addr = (void __iomem *)fdtdec_get_addr(blob, misc_off, "reg");
	if ((fdt_addr_t)addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->misc_addr = addr;

#if defined(CONFIG_MX6)
	int anatop_off, ret, devnump;

	ret = fdtdec_get_alias_seq(blob, dev->uclass->uc_drv->name,
				   phy_off, &devnump);
	if (ret < 0)
		return ret;
	priv->portnr = devnump;

	/* Resolve ANATOP offset through USB PHY node */
	anatop_off = fdtdec_lookup_phandle(blob, phy_off, "fsl,anatop");
	if (anatop_off < 0)
		return -EINVAL;

	addr = (void __iomem *)fdtdec_get_addr(blob, anatop_off, "reg");
	if ((fdt_addr_t)addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->anatop_addr = addr;
#endif
#endif
	return 0;
}

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	struct usb_ehci *ehci = dev_read_addr_ptr(dev);
	struct ehci_mx6_priv_data *priv = dev_get_priv(dev);
	enum usb_init_type type = plat->init_type;
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	if (IS_ENABLED(CONFIG_IMX_MODULE_FUSE)) {
		if (usb_fused((ulong)ehci)) {
			printf("SoC fuse indicates USB@0x%lx is unavailable.\n",
			       (ulong)ehci);
			return -ENODEV;
		}
	}

	ret = mx6_parse_dt_addrs(dev);
	if (ret)
		return ret;

	priv->ehci = ehci;
	priv->init_type = type;
	priv->phy_type = usb_get_phy_mode(dev_ofnode(dev));

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;
#else
	/* Compatibility with DM_USB and !CLK */
	enable_usboh3_clk(1);
	mdelay(1);
#endif

	/*
	 * If the device tree didn't specify host or device,
	 * the default is USB_INIT_UNKNOWN, so we need to check
	 * the register. For imx8mm and imx8mn, the clocks need to be
	 * running first, so we defer the check until they are.
	 */
	if (priv->init_type == USB_INIT_UNKNOWN) {
		ret = ehci_usb_phy_mode(dev);
		if (ret)
			goto err_clk;
		else
			priv->init_type = plat->init_type;
	}

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	ret = device_get_supply_regulator(dev, "vbus-supply",
					  &priv->vbus_supply);
	if (ret)
		debug("%s: No vbus supply\n", dev->name);
#endif

#if !defined(CONFIG_PHY)
	usb_power_config_mx6(priv->anatop_addr, priv->portnr);
	usb_power_config_mx7(priv->misc_addr);
	usb_power_config_mx7ulp(priv->phy_addr);

	usb_oc_config(priv->misc_addr, priv->portnr);

#if defined(CONFIG_MX6) || defined(CONFIG_MX7ULP) || defined(CONFIG_IMXRT)
	usb_internal_phy_clock_gate(priv->phy_addr, 1);
	usb_phy_enable(ehci, priv->phy_addr);
#endif
#else
	ret = generic_setup_phy(dev, &priv->phy, 0);
	if (ret)
		goto err_regulator;
#endif

	if (priv->init_type == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel(mx6_portsc(priv->phy_type), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	mdelay(10);

	hccr = (struct ehci_hccr *)((uintptr_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));

	ret = ehci_register(dev, hccr, hcor, &mx6_ehci_ops, 0, priv->init_type);
	if (ret)
		goto err_phy;

	return ret;

err_phy:
#if defined(CONFIG_PHY)
	generic_shutdown_phy(&priv->phy);
err_regulator:
#endif
err_clk:
#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
#else
	/* Compatibility with DM_USB and !CLK */
	enable_usboh3_clk(0);
#endif
	return ret;
}

int ehci_usb_remove(struct udevice *dev)
{
	struct ehci_mx6_priv_data *priv __maybe_unused = dev_get_priv(dev);

	ehci_deregister(dev);

#if defined(CONFIG_PHY)
	generic_shutdown_phy(&priv->phy);
#endif

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	if (priv->vbus_supply)
		regulator_set_enable(priv->vbus_supply, false);
#endif

#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
#endif

	return 0;
}

static const struct udevice_id mx6_usb_ids[] = {
	{ .compatible = "fsl,imx27-usb" },
	{ .compatible = "fsl,imx7d-usb" },
	{ .compatible = "fsl,imxrt-usb" },
	{ }
};

U_BOOT_DRIVER(usb_mx6) = {
	.name	= "ehci_mx6",
	.id	= UCLASS_USB,
	.of_match = mx6_usb_ids,
	.of_to_plat = ehci_usb_of_to_plat,
	.probe	= ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct ehci_mx6_priv_data),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
