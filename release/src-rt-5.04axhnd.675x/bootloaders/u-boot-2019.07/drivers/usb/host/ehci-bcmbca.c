// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <dm/ofnode.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include "ehci.h"
#include <dm/device-internal.h>

#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_usb.h"
#endif

#define MDIO_USB2   0

/* USB Host contorl regs */
typedef struct usb_ctrl{
	uint32_t setup;
#define USBH_IPP                (1<<5)
#define USBH_IOC                (1<<4)
#define USBH_STRAP_IPP_SEL      (1<<25)
#define USB2_OC_DISABLE_PORT0   (1<<28)
#define USB2_OC_DISABLE_PORT1   (1<<29)
#define USB3_OC_DISABLE_PORT0   (1<<30)
#define USB3_OC_DISABLE_PORT1   (1<<31)
	uint32_t pll_ctl;
	uint32_t fladj_value;
	uint32_t bridge_ctl;
#define USB_BRCTL_OHCI_MEM_REQ_DIS (1<<16)
	uint32_t spare1;
	uint32_t mdio;
	uint32_t mdio2;
	uint32_t test_port_control;
	uint32_t usb_simctl;
#define USBH_OHCI_MEM_REQ_DIS   (1<<1)
	uint32_t usb_testctl;
	uint32_t usb_testmon;
	uint32_t utmi_ctl_1;
	uint32_t utmi_ctl_2;
	uint32_t usb_pm;
#define XHC_SOFT_RESETB         (1<<22)
#define USB_PWRDWN              (1<<31)
	uint32_t usb_pm_status;
	uint32_t spare3;
	uint32_t pll_ldo_ctl;
	uint32_t pll_ldo_pllbias;
	uint32_t pll_afe_bg_cntl;
	uint32_t afe_usbio_tst;
	uint32_t pll_ndiv_frac;
	uint32_t tp_diag;
	uint32_t ahb_capture_fifo;
	uint32_t spare4;
	uint32_t usb30_ctl1;
#define PHY3_PLL_SEQ_START      (1<<4)
	uint32_t usb30_ctl2;
	uint32_t usb30_ctl3;
	uint32_t usb30_ctl4;
	uint32_t usb30_pctl;
	uint32_t usb30_ctl5;
	uint32_t spare5;
	uint32_t spare6;
	uint32_t spare7;
	uint32_t unsused1[3];
	uint32_t usb_device_ctl1;
	uint32_t usb_device_ctl2;
	uint32_t unsused2[22];
	uint32_t usb20_id;
	uint32_t usb30_id;
	uint32_t bdc_coreid;
	uint32_t usb_revid;
} usb_ctrl;

struct bcmbca_usb_ctrl {
	volatile struct usb_ctrl *regp;
};

struct bcmbca_ehci {
	struct ehci_ctrl ctrl;
};

static void usb_mdio_write(volatile uint32_t *mdio, uint32_t reg, uint32_t val, int mode)
{
	uint32_t data;
	data = (reg << 16) | val | mode;
	*mdio = data;
	data |= (1 << 25);
	*mdio = data;
	mdelay(1);
	data &= ~(1 << 25);
	*mdio = data;
}

static void usb2_eye_fix(volatile struct usb_ctrl *regp)
{
	/* Updating USB 2.0 PHY registers */
	usb_mdio_write((void *)&regp->mdio, 0x1f, 0x80a0, MDIO_USB2);
	usb_mdio_write((void *)&regp->mdio, 0x0a, 0xc6a0, MDIO_USB2);
}

#if defined (CONFIG_BCM63138)
static void bcm63138B0_manual_usb_ldo_start(volatile struct usb_ctrl *regp)
{
	regp->pll_ctl &= ~(1 << 30); /*pll_resetb=0*/
	regp->utmi_ctl_1 = 0; 
	regp->pll_ldo_ctl = 4; /*ldo_ctl=core_rdy */
	regp->pll_ctl |= ( 1 << 31); /*pll_iddq=1*/
	mdelay(10);
	regp->pll_ctl &= ~( 1 << 31); /*pll_iddq=0*/
	regp->pll_ldo_ctl |= 1; /*ldo_ctl.AFE_LDO_PWRDWNB=1*/
	regp->pll_ldo_ctl |= 2; /*ldo_ctl.AFE_BG_PWRDWNB=1*/
	mdelay(1);
	regp->utmi_ctl_1 = 0x00020002;/* utmi_resetb &ref_clk_sel=0; */ 
	regp->pll_ctl |= ( 1 << 30); /*pll_resetb=1*/
	mdelay(10);
}    
#endif

static int ehci_usb_probe (struct udevice *dev)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;
	struct resource res;
	struct udevice *ctrl_dev;

	uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(ctrl_bcmbca_drv), &ctrl_dev);

	ret = dev_read_resource_byname (dev, "usb-ehci", &res);
	if (ret)
	{
		dev_err(dev, "can't get usb-ehci register for usb (ret=%d)\n", ret);
		return ret;
	}

	hccr = devm_ioremap (dev, res.start, resource_size(&res));
	hcor = (struct ehci_hcor *)((uint64_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));
	debug("dev %p usb %d hccr %p hcor %p\n", dev, dev->seq, hccr, hcor);

	ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);

	return 0;
}

static int ehci_usb_remove(struct udevice *dev)
{
	int ret;
	struct udevice *next_devp = dev;
	struct udevice *ctrl_dev;

	debug ("remove bcmbca ehci dev %p seq %d\n", dev, dev->seq);
	ret = ehci_deregister(dev);

	if (ret)
	{
		printf("failed to deregister bcmbca ehci seq %d\n", dev->seq);
		return ret;
	}
	ret = uclass_find_next_device (&next_devp);
	if (!ret && !next_devp) {
		uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(ctrl_bcmbca_drv), &ctrl_dev);
		if (ctrl_dev)
			device_remove(ctrl_dev, DM_REMOVE_NORMAL);
	}

	return 0;
}


static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "brcm,bcmbca-ehci" },
	{ }
};

U_BOOT_DRIVER(ehci_bcmbca) = {
	.name	= "ehci-bcmbca",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct bcmbca_ehci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

/* definition of the dummy driver for usb ctrl */
static int bcmbca_usb_ctrl_probe (struct udevice *dev)
{
	int ret;
	uint32_t val;
	struct resource res;
	volatile struct usb_ctrl *regp;
	struct bcmbca_usb_ctrl *priv = dev_get_priv (dev);

	debug("bcmbca_usb_ctrl_probe called....\n");

	//usb-ctrl drive probe function is expected to be called once

#if defined(CONFIG_BCMBCA_PMC)
	if (pmc_usb_power_up(PMC_USB_HOST_ALL)) 
	{
		printf("Failed to Power Up USB Host\n");
		return -1;
	}
	mdelay(1);
#endif
	priv->regp = NULL;
	ret = dev_read_resource_byname (dev, "usb-ctrl", &res);
	if (ret)
	{
		dev_err(dev, "can't get usb-ctrl register (ret=%d)\n", ret);
		return ret;
	}
	regp = devm_ioremap (dev, res.start, resource_size(&res));
	priv->regp = regp;
	debug("usb-ctrl reg %p\n", priv->regp);

	/* adjust over current & port power polarity */
	regp->setup |= (USBH_IOC);
	if (dev_read_bool(dev, "pwrflt-bias-pull-up"))
		regp->setup &= ~(USBH_IOC);

	/*overide strap for IPP*/
	val = regp->setup;
	val &= ~(USBH_STRAP_IPP_SEL);
	val |= (USBH_IPP);
	regp->setup = val;

	if (dev_read_bool(dev, "pwron-bias-pull-up"))
		regp->setup &= ~(USBH_IPP);

#if defined (CONFIG_BCM63138)
	mdelay(300);
	bcm63138B0_manual_usb_ldo_start(regp);
#endif
	/*enable USB PHYs*/
	mdelay(1);
	regp->usb_pm &= ~(USB_PWRDWN);
	mdelay(300);
	regp->usb_pm &= ~XHC_SOFT_RESETB;
	/*adjust the default AFE settings for better eye diagrams */
	usb2_eye_fix(regp);

	/*initialize EHCI & OHCI settings*/
	/* no swap for data & desciptors */    
	regp->bridge_ctl &= ~(0xf); /*clear lower 4 bits */

	/* reset host controllers for possible fake overcurrent indications */ 
	val = regp->usb_pm;
	regp->usb_pm = 0;
	regp->usb_pm = val;
	mdelay(1);

	return 0;
}

static int bcmbca_usb_ctrl_remove(struct udevice *dev)
{
	debug("bcmbca_usb_ctrl_remove called\n");
#if defined(CONFIG_BCMBCA_PMC)
	if (pmc_usb_power_down(PMC_USB_HOST_ALL))
	{
		printf("Failed to Power Down USB Host\n");
		return -1;
	}
#endif

	return 0;
}

static const struct udevice_id bcmbca_usb_ctrl_ids[] = {
	{ .compatible = "brcm,bcmbca-usb-ctrl" },
	{ }
};

U_BOOT_DRIVER(ctrl_bcmbca_drv) = {
	.name = "bcmbca-usbctrl",
	.id = UCLASS_NOP,
	.of_match = bcmbca_usb_ctrl_ids,
	.probe = bcmbca_usb_ctrl_probe,
	.remove = bcmbca_usb_ctrl_remove,
	.priv_auto_alloc_size = sizeof(struct bcmbca_usb_ctrl),
};
