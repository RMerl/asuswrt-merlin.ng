// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip PIC32 MUSB "glue layer"
 *
 * Copyright (C) 2015, Microchip Technology Inc.
 *  Cristian Birsan <cristian.birsan@microchip.com>
 *  Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 * Based on the dsps "glue layer" code.
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/usb/musb.h>
#include "linux-compat.h"
#include "musb_core.h"
#include "musb_uboot.h"

DECLARE_GLOBAL_DATA_PTR;

#define PIC32_TX_EP_MASK	0x0f		/* EP0 + 7 Tx EPs */
#define PIC32_RX_EP_MASK	0x0e		/* 7 Rx EPs */

#define MUSB_SOFTRST		0x7f
#define  MUSB_SOFTRST_NRST	BIT(0)
#define  MUSB_SOFTRST_NRSTX	BIT(1)

#define USBCRCON		0
#define  USBCRCON_USBWKUPEN	BIT(0)  /* Enable Wakeup Interrupt */
#define  USBCRCON_USBRIE	BIT(1)  /* Enable Remote resume Interrupt */
#define  USBCRCON_USBIE		BIT(2)  /* Enable USB General interrupt */
#define  USBCRCON_SENDMONEN	BIT(3)  /* Enable Session End VBUS monitoring */
#define  USBCRCON_BSVALMONEN	BIT(4)  /* Enable B-Device VBUS monitoring */
#define  USBCRCON_ASVALMONEN	BIT(5)  /* Enable A-Device VBUS monitoring */
#define  USBCRCON_VBUSMONEN	BIT(6)  /* Enable VBUS monitoring */
#define  USBCRCON_PHYIDEN	BIT(7)  /* PHY ID monitoring enable */
#define  USBCRCON_USBIDVAL	BIT(8)  /* USB ID value */
#define  USBCRCON_USBIDOVEN	BIT(9)  /* USB ID override enable */
#define  USBCRCON_USBWK		BIT(24) /* USB Wakeup Status */
#define  USBCRCON_USBRF		BIT(25) /* USB Resume Status */
#define  USBCRCON_USBIF		BIT(26) /* USB General Interrupt Status */

/* PIC32 controller data */
struct pic32_musb_data {
	struct musb_host_data mdata;
	struct device dev;
	void __iomem *musb_glue;
};

#define to_pic32_musb_data(d)	\
	container_of(d, struct pic32_musb_data, dev)

static void pic32_musb_disable(struct musb *musb)
{
	/* no way to shut the controller */
}

static int pic32_musb_enable(struct musb *musb)
{
	/* soft reset by NRSTx */
	musb_writeb(musb->mregs, MUSB_SOFTRST, MUSB_SOFTRST_NRSTX);
	/* set mode */
	musb_platform_set_mode(musb, musb->board_mode);

	return 0;
}

static irqreturn_t pic32_interrupt(int irq, void *hci)
{
	struct musb  *musb = hci;
	irqreturn_t ret = IRQ_NONE;
	u32 epintr, usbintr;

	/* ack usb core interrupts */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	if (musb->int_usb)
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);

	/* ack endpoint interrupts */
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX) & PIC32_RX_EP_MASK;
	if (musb->int_rx)
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);

	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX) & PIC32_TX_EP_MASK;
	if (musb->int_tx)
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);

	/* drop spurious RX and TX if device is disconnected */
	if (musb->int_usb & MUSB_INTR_DISCONNECT) {
		musb->int_tx = 0;
		musb->int_rx = 0;
	}

	if (musb->int_tx || musb->int_rx || musb->int_usb)
		ret = musb_interrupt(musb);

	return ret;
}

static int pic32_musb_set_mode(struct musb *musb, u8 mode)
{
	struct device *dev = musb->controller;
	struct pic32_musb_data *pdata = to_pic32_musb_data(dev);

	switch (mode) {
	case MUSB_HOST:
		clrsetbits_le32(pdata->musb_glue + USBCRCON,
				USBCRCON_USBIDVAL, USBCRCON_USBIDOVEN);
		break;
	case MUSB_PERIPHERAL:
		setbits_le32(pdata->musb_glue + USBCRCON,
			     USBCRCON_USBIDVAL | USBCRCON_USBIDOVEN);
		break;
	case MUSB_OTG:
		dev_err(dev, "support for OTG is unimplemented\n");
		break;
	default:
		dev_err(dev, "unsupported mode %d\n", mode);
		return -EINVAL;
	}

	return 0;
}

static int pic32_musb_init(struct musb *musb)
{
	struct pic32_musb_data *pdata = to_pic32_musb_data(musb->controller);
	u32 ctrl, hwvers;
	u8 power;

	/* Returns zero if not clocked */
	hwvers = musb_read_hwvers(musb->mregs);
	if (!hwvers)
		return -ENODEV;

	/* Reset the musb */
	power = musb_readb(musb->mregs, MUSB_POWER);
	power = power | MUSB_POWER_RESET;
	musb_writeb(musb->mregs, MUSB_POWER, power);
	mdelay(100);

	/* Start the on-chip PHY and its PLL. */
	power = power & ~MUSB_POWER_RESET;
	musb_writeb(musb->mregs, MUSB_POWER, power);

	musb->isr = pic32_interrupt;

	ctrl =  USBCRCON_USBIF | USBCRCON_USBRF |
		USBCRCON_USBWK | USBCRCON_USBIDOVEN |
		USBCRCON_PHYIDEN | USBCRCON_USBIE |
		USBCRCON_USBRIE | USBCRCON_USBWKUPEN |
		USBCRCON_VBUSMONEN;
	writel(ctrl, pdata->musb_glue + USBCRCON);

	return 0;
}

/* PIC32 supports only 32bit read operation */
void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst)
{
	void __iomem *fifo = hw_ep->fifo;
	u32 val, rem = len % 4;

	/* USB stack ensures dst is always 32bit aligned. */
	readsl(fifo, dst, len / 4);
	if (rem) {
		dst += len & ~0x03;
		val = musb_readl(fifo, 0);
		memcpy(dst, &val, rem);
	}
}

const struct musb_platform_ops pic32_musb_ops = {
	.init		= pic32_musb_init,
	.set_mode	= pic32_musb_set_mode,
	.disable	= pic32_musb_disable,
	.enable		= pic32_musb_enable,
};

/* PIC32 default FIFO config - fits in 8KB */
static struct musb_fifo_cfg pic32_musb_fifo_config[] = {
	{ .hw_ep_num = 1, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 1, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 2, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 2, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 3, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 3, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 4, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 4, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 5, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 5, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 6, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 6, .style = FIFO_RX, .maxpacket = 512, },
	{ .hw_ep_num = 7, .style = FIFO_TX, .maxpacket = 512, },
	{ .hw_ep_num = 7, .style = FIFO_RX, .maxpacket = 512, },
};

static struct musb_hdrc_config pic32_musb_config = {
	.fifo_cfg	= pic32_musb_fifo_config,
	.fifo_cfg_size	= ARRAY_SIZE(pic32_musb_fifo_config),
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 8,
	.ram_bits       = 11,
};

/* PIC32 has one MUSB controller which can be host or gadget */
static struct musb_hdrc_platform_data pic32_musb_plat = {
	.mode           = MUSB_HOST,
	.config         = &pic32_musb_config,
	.power          = 250,		/* 500mA */
	.platform_ops	= &pic32_musb_ops,
};

static int musb_usb_probe(struct udevice *dev)
{
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	struct pic32_musb_data *pdata = dev_get_priv(dev);
	struct musb_host_data *mdata = &pdata->mdata;
	struct fdt_resource mc, glue;
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	void __iomem *mregs;
	int ret;

	priv->desc_before_addr = true;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "mc", &mc);
	if (ret < 0) {
		printf("pic32-musb: resource \"mc\" not found\n");
		return ret;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "control", &glue);
	if (ret < 0) {
		printf("pic32-musb: resource \"control\" not found\n");
		return ret;
	}

	mregs = ioremap(mc.start, fdt_resource_size(&mc));
	pdata->musb_glue = ioremap(glue.start, fdt_resource_size(&glue));

	/* init controller */
#ifdef CONFIG_USB_MUSB_HOST
	mdata->host = musb_init_controller(&pic32_musb_plat,
					   &pdata->dev, mregs);
	if (!mdata->host)
		return -EIO;

	ret = musb_lowlevel_init(mdata);
#else
	pic32_musb_plat.mode = MUSB_PERIPHERAL;
	mdata->host = musb_register(&pic32_musb_plat, &pdata->dev, mregs);
	if (!mdata->host)
		return -EIO;
#endif
	if ((ret == 0) && mdata->host)
		printf("PIC32 MUSB OTG\n");

	return ret;
}

static int musb_usb_remove(struct udevice *dev)
{
	struct pic32_musb_data *pdata = dev_get_priv(dev);

	musb_stop(pdata->mdata.host);

	return 0;
}

static const struct udevice_id pic32_musb_ids[] = {
	{ .compatible = "microchip,pic32mzda-usb" },
	{ }
};

U_BOOT_DRIVER(usb_musb) = {
	.name		= "pic32-musb",
	.id		= UCLASS_USB,
	.of_match	= pic32_musb_ids,
	.probe		= musb_usb_probe,
	.remove		= musb_usb_remove,
#ifdef CONFIG_USB_MUSB_HOST
	.ops		= &musb_usb_ops,
#endif
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct pic32_musb_data),
};
