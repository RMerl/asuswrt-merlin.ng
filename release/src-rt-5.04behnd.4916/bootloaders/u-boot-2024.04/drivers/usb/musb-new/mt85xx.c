// SPDX-License-Identifier: GPL-2.0
/*
 * Mediatek "glue layer"
 *
 * Copyright (C) 2019-2021 by Mediatek
 * Based on the AllWinner SUNXI "glue layer" code.
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of the Inventra Controller Driver for Linux.
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/usb/musb.h>
#include <usb.h>
#include "linux-compat.h"
#include "musb_core.h"
#include "musb_uboot.h"

#define DBG_I(fmt, ...) \
	pr_info(fmt, ##__VA_ARGS__)

struct mtk_musb_config {
	struct musb_hdrc_config *config;
};

struct mtk_musb_glue {
	struct musb_host_data mdata;
	struct clk usbpllclk;
	struct clk usbmcuclk;
	struct clk usbclk;
	struct mtk_musb_config *cfg;
	struct device dev;
};

#define to_mtk_musb_glue(d)	container_of(d, struct mtk_musb_glue, dev)

/******************************************************************************
 * phy settings
 ******************************************************************************/
#define USB20_PHY_BASE			0x11110800
#define USBPHY_READ8(offset)	 \
	readb((void *)(USB20_PHY_BASE + (offset)))
#define USBPHY_WRITE8(offset, value)	\
	writeb(value, (void *)(USB20_PHY_BASE + (offset)))
#define USBPHY_SET8(offset, mask)	\
	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)	\
	USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~(mask)))

static void mt_usb_phy_poweron(void)
{
	/*
	 * switch to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);
	USBPHY_CLR8(0x21, 0x03);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_SET8(0x22, 0x04);
	USBPHY_CLR8(0x1a, 0x80);

	/* RG_USB20_DP_100K_EN = 1'b0 */
	/* RG_USB20_DP_100K_EN = 1'b0 */
	USBPHY_CLR8(0x22, 0x03);

	/*OTG enable*/
	USBPHY_SET8(0x20, 0x10);
	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);

	mdelay(800);

	/* force enter device mode */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2E);
	USBPHY_SET8(0x6d, 0x3E);
}

static void mt_usb_phy_savecurrent(void)
{
	/*
	 * switch to USB function.
	 * (system register, force ip into usb mode).
	 */
	USBPHY_CLR8(0x6b, 0x04);
	USBPHY_CLR8(0x6e, 0x01);
	USBPHY_CLR8(0x21, 0x03);

	/* release force suspendm */
	USBPHY_CLR8(0x6a, 0x04);
	USBPHY_SET8(0x68, 0x04);
	/* RG_DPPULLDOWN./RG_DMPULLDOWN. */
	USBPHY_SET8(0x68, 0xc0);
	/* RG_XCVRSEL[1:0] = 2'b01 */
	USBPHY_CLR8(0x68, 0x30);
	USBPHY_SET8(0x68, 0x10);
	/* RG_TERMSEL = 1'b1 */
	USBPHY_SET8(0x68, 0x04);
	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	/*
	 * force_dp_pulldown, force_dm_pulldown,
	 * force_xcversel, force_termsel.
	 */
	USBPHY_SET8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x10);

	mdelay(800);

	USBPHY_CLR8(0x6a, 0x04);
	/* rg_usb20_pll_stable = 1 */
	//USBPHY_SET8(0x63, 0x02);

	mdelay(1);

	/* force suspendm = 1 */
	//USBPHY_SET8(0x6a, 0x04);
}

static void mt_usb_phy_recover(void)
{
	/* clean PUPD_BIST_EN */
	/* PUPD_BIST_EN = 1'b0 */
	/* PMIC will use it to detect charger type */
	USBPHY_CLR8(0x1d, 0x10);

	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN = 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* force_uart_en = 1'b0 */
	USBPHY_CLR8(0x6a, 0x04);

	USBPHY_CLR8(0x21, 0x03);
	USBPHY_CLR8(0x68, 0xf4);

	/* RG_DATAIN[3:0] = 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	USBPHY_CLR8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSSCMP_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x10);

	//HQA adjustment
	USBPHY_CLR8(0x18, 0x08);
	USBPHY_SET8(0x18, 0x06);
	mdelay(800);

	/* force enter device mode */
	//USBPHY_CLR8(0x6c, 0x10);
	//USBPHY_SET8(0x6c, 0x2E);
	//USBPHY_SET8(0x6d, 0x3E);

	/* enable VRT internal R architecture */
	/* RG_USB20_INTR_EN = 1'b1 */
	USBPHY_SET8(0x00, 0x20);
}

/******************************************************************************
 * MUSB Glue code
 ******************************************************************************/

static irqreturn_t mtk_musb_interrupt(int irq, void *__hci)
{
	struct musb		*musb = __hci;
	irqreturn_t		retval = IRQ_NONE;

	/* read and flush interrupts */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
//	last_int_usb = musb->int_usb;
	if (musb->int_usb)
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	if (musb->int_tx)
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);
	if (musb->int_rx)
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval |= musb_interrupt(musb);

	return retval;
}

/* musb_core does not call enable / disable in a balanced manner <sigh> */
static bool enabled;

static int mtk_musb_enable(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);

	DBG_I("%s():\n", __func__);

	musb_ep_select(musb->mregs, 0);
	musb_writeb(musb->mregs, MUSB_FADDR, 0);

	if (enabled)
		return 0;

	mt_usb_phy_recover();

	enabled = true;

	return 0;
}

static void mtk_musb_disable(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);
	int ret;

	DBG_I("%s():\n", __func__);

	if (!enabled)
		return;

	mt_usb_phy_savecurrent();

	enabled = false;
}

static int mtk_musb_init(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);
	int ret;

	DBG_I("%s():\n", __func__);

	ret = clk_enable(&glue->usbpllclk);
	if (ret) {
		dev_err(musb->controller, "failed to enable usbpll clock\n");
		return ret;
	}
	ret = clk_enable(&glue->usbmcuclk);
	if (ret) {
		dev_err(musb->controller, "failed to enable usbmcu clock\n");
		return ret;
	}
	ret = clk_enable(&glue->usbclk);
	if (ret) {
		dev_err(musb->controller, "failed to enable usb clock\n");
		return ret;
	}

	musb->isr = mtk_musb_interrupt;

	return 0;
}

static int mtk_musb_exit(struct musb *musb)
{
	struct mtk_musb_glue *glue = to_mtk_musb_glue(musb->controller);

	clk_disable(&glue->usbclk);
	clk_disable(&glue->usbmcuclk);
	clk_disable(&glue->usbpllclk);

	return 0;
}

static const struct musb_platform_ops mtk_musb_ops = {
	.init		= mtk_musb_init,
	.exit		= mtk_musb_exit,
	.enable		= mtk_musb_enable,
	.disable	= mtk_musb_disable,
};

/* MTK OTG supports up to 7 endpoints */
#define MTK_MUSB_MAX_EP_NUM		8
#define MTK_MUSB_RAM_BITS		16

static struct musb_fifo_cfg mtk_musb_mode_cfg[] = {
	MUSB_EP_FIFO_SINGLE(1, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(1, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(2, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(2, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(3, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(3, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(4, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(4, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(5, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(5, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(6, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(6, FIFO_RX, 512),
	MUSB_EP_FIFO_SINGLE(7, FIFO_TX, 512),
	MUSB_EP_FIFO_SINGLE(7, FIFO_RX, 512),
};

static struct musb_hdrc_config musb_config = {
	.fifo_cfg       = mtk_musb_mode_cfg,
	.fifo_cfg_size  = ARRAY_SIZE(mtk_musb_mode_cfg),
	.multipoint	= true,
	.dyn_fifo	= true,
	.num_eps	= MTK_MUSB_MAX_EP_NUM,
	.ram_bits	= MTK_MUSB_RAM_BITS,
};

static int musb_usb_probe(struct udevice *dev)
{
	struct mtk_musb_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;
	struct musb_hdrc_platform_data pdata;
	void *base = dev_read_addr_ptr(dev);
	int ret;

	DBG_I("%s():\n", __func__);

#ifdef CONFIG_USB_MUSB_HOST
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
#endif

	if (!base)
		return -EINVAL;

	glue->cfg = (struct mtk_musb_config *)dev_get_driver_data(dev);
	if (!glue->cfg)
		return -EINVAL;

	ret = clk_get_by_name(dev, "usbpll", &glue->usbpllclk);
	if (ret) {
		dev_err(dev, "failed to get usbpll clock\n");
		return ret;
	}
	ret = clk_get_by_name(dev, "usbmcu", &glue->usbmcuclk);
	if (ret) {
		dev_err(dev, "failed to get usbmcu clock\n");
		return ret;
	}
	ret = clk_get_by_name(dev, "usb", &glue->usbclk);
	if (ret) {
		dev_err(dev, "failed to get usb clock\n");
		return ret;
	}

	memset(&pdata, 0, sizeof(pdata));
	pdata.power = (u8)400;
	pdata.platform_ops = &mtk_musb_ops;
	pdata.config = glue->cfg->config;

#ifdef CONFIG_USB_MUSB_HOST
	priv->desc_before_addr = true;

	pdata.mode = MUSB_HOST;
	host->host = musb_init_controller(&pdata, &glue->dev, base);
	if (!host->host)
		return -EIO;

	ret = musb_lowlevel_init(host);
	if (!ret)
		printf("MTK MUSB OTG (Host)\n");
#else
	pdata.mode = MUSB_PERIPHERAL;
	host->host = musb_register(&pdata, &glue->dev, base);
	if (!host->host)
		return -EIO;

	printf("MTK MUSB OTG (Peripheral)\n");
#endif

	mt_usb_phy_poweron();

	return ret;
}

static int musb_usb_remove(struct udevice *dev)
{
	struct mtk_musb_glue *glue = dev_get_priv(dev);
	struct musb_host_data *host = &glue->mdata;

	musb_stop(host->host);
	free(host->host);
	host->host = NULL;

	return 0;
}

static const struct mtk_musb_config mt8518_cfg = {
	.config = &musb_config,
};

static const struct udevice_id mtk_musb_ids[] = {
	{ .compatible = "mediatek,mt8518-musb",
	  .data = (ulong)&mt8518_cfg },
	{ }
};

U_BOOT_DRIVER(mtk_musb) = {
	.name		= "mtk_musb",
#ifdef CONFIG_USB_MUSB_HOST
	.id		= UCLASS_USB,
#else
	.id		= UCLASS_USB_GADGET_GENERIC,
#endif
	.of_match	= mtk_musb_ids,
	.probe		= musb_usb_probe,
	.remove		= musb_usb_remove,
#ifdef CONFIG_USB_MUSB_HOST
	.ops		= &musb_usb_ops,
#endif
	.plat_auto	= sizeof(struct usb_plat),
	.priv_auto	= sizeof(struct mtk_musb_glue),
};
