// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 */


#include <common.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <usb/ehci-ci.h>
#include <errno.h>

#include "ehci.h"

#define USBCTRL_OTGBASE_OFFSET	0x600

#define MX25_OTG_SIC_SHIFT	29
#define MX25_OTG_SIC_MASK	(0x3 << MX25_OTG_SIC_SHIFT)
#define MX25_OTG_PM_BIT		(1 << 24)
#define MX25_OTG_PP_BIT		(1 << 11)
#define MX25_OTG_OCPOL_BIT	(1 << 3)

#define MX25_H1_SIC_SHIFT	21
#define MX25_H1_SIC_MASK	(0x3 << MX25_H1_SIC_SHIFT)
#define MX25_H1_PP_BIT		(1 << 18)
#define MX25_H1_PM_BIT		(1 << 16)
#define MX25_H1_IPPUE_UP_BIT	(1 << 7)
#define MX25_H1_IPPUE_DOWN_BIT	(1 << 6)
#define MX25_H1_TLL_BIT		(1 << 5)
#define MX25_H1_USBTE_BIT	(1 << 4)
#define MX25_H1_OCPOL_BIT	(1 << 2)

#define MX31_OTG_SIC_SHIFT	29
#define MX31_OTG_SIC_MASK	(0x3 << MX31_OTG_SIC_SHIFT)
#define MX31_OTG_PM_BIT		(1 << 24)

#define MX31_H2_SIC_SHIFT	21
#define MX31_H2_SIC_MASK	(0x3 << MX31_H2_SIC_SHIFT)
#define MX31_H2_PM_BIT		(1 << 16)
#define MX31_H2_DT_BIT		(1 << 5)

#define MX31_H1_SIC_SHIFT	13
#define MX31_H1_SIC_MASK	(0x3 << MX31_H1_SIC_SHIFT)
#define MX31_H1_PM_BIT		(1 << 8)
#define MX31_H1_DT_BIT		(1 << 4)

#define MX35_OTG_SIC_SHIFT	29
#define MX35_OTG_SIC_MASK	(0x3 << MX35_OTG_SIC_SHIFT)
#define MX35_OTG_PM_BIT		(1 << 24)
#define MX35_OTG_PP_BIT		(1 << 11)
#define MX35_OTG_OCPOL_BIT	(1 << 3)

#define MX35_H1_SIC_SHIFT	21
#define MX35_H1_SIC_MASK	(0x3 << MX35_H1_SIC_SHIFT)
#define MX35_H1_PP_BIT		(1 << 18)
#define MX35_H1_PM_BIT		(1 << 16)
#define MX35_H1_IPPUE_UP_BIT	(1 << 7)
#define MX35_H1_IPPUE_DOWN_BIT	(1 << 6)
#define MX35_H1_TLL_BIT		(1 << 5)
#define MX35_H1_USBTE_BIT	(1 << 4)
#define MX35_H1_OCPOL_BIT	(1 << 2)

static int mxc_set_usbcontrol(int port, unsigned int flags)
{
	unsigned int v;

	v = readl(IMX_USB_BASE + USBCTRL_OTGBASE_OFFSET);
#if defined(CONFIG_MX25)
	switch (port) {
	case 0:	/* OTG port */
		v &= ~(MX25_OTG_SIC_MASK | MX25_OTG_PM_BIT | MX25_OTG_PP_BIT |
				MX25_OTG_OCPOL_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX25_OTG_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX25_OTG_PM_BIT;

		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MX25_OTG_PP_BIT;

		if (!(flags & MXC_EHCI_OC_PIN_ACTIVE_LOW))
			v |= MX25_OTG_OCPOL_BIT;

		break;
	case 1: /* H1 port */
		v &= ~(MX25_H1_SIC_MASK | MX25_H1_PM_BIT | MX25_H1_PP_BIT |
				MX25_H1_OCPOL_BIT | MX25_H1_TLL_BIT |
				MX25_H1_USBTE_BIT | MX25_H1_IPPUE_DOWN_BIT |
				MX25_H1_IPPUE_UP_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX25_H1_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX25_H1_PM_BIT;

		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MX25_H1_PP_BIT;

		if (!(flags & MXC_EHCI_OC_PIN_ACTIVE_LOW))
			v |= MX25_H1_OCPOL_BIT;

		if (!(flags & MXC_EHCI_TTL_ENABLED))
			v |= MX25_H1_TLL_BIT;

		if (flags & MXC_EHCI_INTERNAL_PHY)
			v |= MX25_H1_USBTE_BIT;

		if (flags & MXC_EHCI_IPPUE_DOWN)
			v |= MX25_H1_IPPUE_DOWN_BIT;

		if (flags & MXC_EHCI_IPPUE_UP)
			v |= MX25_H1_IPPUE_UP_BIT;

		break;
	default:
		return -EINVAL;
	}
#elif defined(CONFIG_MX31)
	switch (port) {
	case 0:	/* OTG port */
		v &= ~(MX31_OTG_SIC_MASK | MX31_OTG_PM_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX31_OTG_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX31_OTG_PM_BIT;

		break;
	case 1: /* H1 port */
		v &= ~(MX31_H1_SIC_MASK | MX31_H1_PM_BIT | MX31_H1_DT_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX31_H1_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX31_H1_PM_BIT;

		if (!(flags & MXC_EHCI_TTL_ENABLED))
			v |= MX31_H1_DT_BIT;

		break;
	case 2:	/* H2 port */
		v &= ~(MX31_H2_SIC_MASK | MX31_H2_PM_BIT | MX31_H2_DT_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX31_H2_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX31_H2_PM_BIT;

		if (!(flags & MXC_EHCI_TTL_ENABLED))
			v |= MX31_H2_DT_BIT;

		break;
	default:
		return -EINVAL;
	}
#elif defined(CONFIG_MX35)
	switch (port) {
	case 0:	/* OTG port */
		v &= ~(MX35_OTG_SIC_MASK | MX35_OTG_PM_BIT | MX35_OTG_PP_BIT |
				MX35_OTG_OCPOL_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX35_OTG_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX35_OTG_PM_BIT;

		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MX35_OTG_PP_BIT;

		if (!(flags & MXC_EHCI_OC_PIN_ACTIVE_LOW))
			v |= MX35_OTG_OCPOL_BIT;

		break;
	case 1: /* H1 port */
		v &= ~(MX35_H1_SIC_MASK | MX35_H1_PM_BIT | MX35_H1_PP_BIT |
				MX35_H1_OCPOL_BIT | MX35_H1_TLL_BIT |
				MX35_H1_USBTE_BIT | MX35_H1_IPPUE_DOWN_BIT |
				MX35_H1_IPPUE_UP_BIT);
		v |= (flags & MXC_EHCI_INTERFACE_MASK) << MX35_H1_SIC_SHIFT;

		if (!(flags & MXC_EHCI_POWER_PINS_ENABLED))
			v |= MX35_H1_PM_BIT;

		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MX35_H1_PP_BIT;

		if (!(flags & MXC_EHCI_OC_PIN_ACTIVE_LOW))
			v |= MX35_H1_OCPOL_BIT;

		if (!(flags & MXC_EHCI_TTL_ENABLED))
			v |= MX35_H1_TLL_BIT;

		if (flags & MXC_EHCI_INTERNAL_PHY)
			v |= MX35_H1_USBTE_BIT;

		if (flags & MXC_EHCI_IPPUE_DOWN)
			v |= MX35_H1_IPPUE_DOWN_BIT;

		if (flags & MXC_EHCI_IPPUE_UP)
			v |= MX35_H1_IPPUE_UP_BIT;

		break;
	default:
		return -EINVAL;
	}
#else
#error MXC EHCI USB driver not supported on this platform
#endif
	writel(v, IMX_USB_BASE + USBCTRL_OTGBASE_OFFSET);

	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
#ifdef CONFIG_MX31
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;

	__raw_readl(&sc_regs->ccmr);
	__raw_writel(__raw_readl(&sc_regs->ccmr) | (1 << 9), &sc_regs->ccmr) ;
#endif

	udelay(80);

	ehci = (struct usb_ehci *)(IMX_USB_BASE +
			IMX_USB_PORT_OFFSET * CONFIG_MXC_USB_PORT);
	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	setbits_le32(&ehci->usbmode, CM_HOST);
	__raw_writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	mxc_set_usbcontrol(CONFIG_MXC_USB_PORT, CONFIG_MXC_USB_FLAGS);
#ifdef CONFIG_MX35
	/* Workaround for ENGcm11601 */
	__raw_writel(0, &ehci->sbuscfg);
#endif

	udelay(10000);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
