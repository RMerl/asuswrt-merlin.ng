/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/prefetch.h>
#include <linux/usb/usb_phy_generic.h>

#include <asm/cacheflush.h>

#include "musb_core.h"
#include "musbhsdma.h"
#include "blackfin.h"

struct bfin_glue {
	struct device		*dev;
	struct platform_device	*musb;
	struct platform_device	*phy;
};
#define glue_to_musb(g)		platform_get_drvdata(g->musb)

static u32 bfin_fifo_offset(u8 epnum)
{
	return USB_OFFSET(USB_EP0_FIFO) + (epnum * 8);
}

static u8 bfin_readb(const void __iomem *addr, unsigned offset)
{
	return (u8)(bfin_read16(addr + offset));
}

static u16 bfin_readw(const void __iomem *addr, unsigned offset)
{
	return bfin_read16(addr + offset);
}

static u32 bfin_readl(const void __iomem *addr, unsigned offset)
{
	return (u32)(bfin_read16(addr + offset));
}

static void bfin_writeb(void __iomem *addr, unsigned offset, u8 data)
{
	bfin_write16(addr + offset, (u16)data);
}

static void bfin_writew(void __iomem *addr, unsigned offset, u16 data)
{
	bfin_write16(addr + offset, data);
}

static void bfin_writel(void __iomem *addr, unsigned offset, u32 data)
{
	bfin_write16(addr + offset, (u16)data);
}

/*
 * Load an endpoint's FIFO
 */
static void bfin_write_fifo(struct musb_hw_ep *hw_ep, u16 len, const u8 *src)
{
	struct musb *musb = hw_ep->musb;
	void __iomem *fifo = hw_ep->fifo;
	void __iomem *epio = hw_ep->regs;
	u8 epnum = hw_ep->epnum;

	prefetch((u8 *)src);

	musb_writew(epio, MUSB_TXCOUNT, len);

	dev_dbg(musb->controller, "TX ep%d fifo %p count %d buf %p, epio %p\n",
			hw_ep->epnum, fifo, len, src, epio);

	dump_fifo_data(src, len);

	if (!ANOMALY_05000380 && epnum != 0) {
		u16 dma_reg;

		flush_dcache_range((unsigned long)src,
			(unsigned long)(src + len));

		/* Setup DMA address register */
		dma_reg = (u32)src;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_ADDR_LOW), dma_reg);
		SSYNC();

		dma_reg = (u32)src >> 16;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_ADDR_HIGH), dma_reg);
		SSYNC();

		/* Setup DMA count register */
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_COUNT_LOW), len);
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_COUNT_HIGH), 0);
		SSYNC();

		/* Enable the DMA */
		dma_reg = (epnum << 4) | DMA_ENA | INT_ENA | DIRECTION;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_CTRL), dma_reg);
		SSYNC();

		/* Wait for complete */
		while (!(bfin_read_USB_DMA_INTERRUPT() & (1 << epnum)))
			cpu_relax();

		/* acknowledge dma interrupt */
		bfin_write_USB_DMA_INTERRUPT(1 << epnum);
		SSYNC();

		/* Reset DMA */
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_CTRL), 0);
		SSYNC();
	} else {
		SSYNC();

		if (unlikely((unsigned long)src & 0x01))
			outsw_8((unsigned long)fifo, src, (len + 1) >> 1);
		else
			outsw((unsigned long)fifo, src, (len + 1) >> 1);
	}
}
/*
 * Unload an endpoint's FIFO
 */
static void bfin_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst)
{
	struct musb *musb = hw_ep->musb;
	void __iomem *fifo = hw_ep->fifo;
	u8 epnum = hw_ep->epnum;

	if (ANOMALY_05000467 && epnum != 0) {
		u16 dma_reg;

		invalidate_dcache_range((unsigned long)dst,
			(unsigned long)(dst + len));

		/* Setup DMA address register */
		dma_reg = (u32)dst;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_ADDR_LOW), dma_reg);
		SSYNC();

		dma_reg = (u32)dst >> 16;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_ADDR_HIGH), dma_reg);
		SSYNC();

		/* Setup DMA count register */
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_COUNT_LOW), len);
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_COUNT_HIGH), 0);
		SSYNC();

		/* Enable the DMA */
		dma_reg = (epnum << 4) | DMA_ENA | INT_ENA;
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_CTRL), dma_reg);
		SSYNC();

		/* Wait for complete */
		while (!(bfin_read_USB_DMA_INTERRUPT() & (1 << epnum)))
			cpu_relax();

		/* acknowledge dma interrupt */
		bfin_write_USB_DMA_INTERRUPT(1 << epnum);
		SSYNC();

		/* Reset DMA */
		bfin_write16(USB_DMA_REG(epnum, USB_DMAx_CTRL), 0);
		SSYNC();
	} else {
		SSYNC();
		/* Read the last byte of packet with odd size from address fifo + 4
		 * to trigger 1 byte access to EP0 FIFO.
		 */
		if (len == 1)
			*dst = (u8)inw((unsigned long)fifo + 4);
		else {
			if (unlikely((unsigned long)dst & 0x01))
				insw_8((unsigned long)fifo, dst, len >> 1);
			else
				insw((unsigned long)fifo, dst, len >> 1);

			if (len & 0x01)
				*(dst + len - 1) = (u8)inw((unsigned long)fifo + 4);
		}
	}
	dev_dbg(musb->controller, "%cX ep%d fifo %p count %d buf %p\n",
			'R', hw_ep->epnum, fifo, len, dst);

	dump_fifo_data(dst, len);
}

static irqreturn_t blackfin_interrupt(int irq, void *__hci)
{
	unsigned long	flags;
	irqreturn_t	retval = IRQ_NONE;
	struct musb	*musb = __hci;

	spin_lock_irqsave(&musb->lock, flags);

	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);

	if (musb->int_usb || musb->int_tx || musb->int_rx) {
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);
		retval = musb_interrupt(musb);
	}

	/* Start sampling ID pin, when plug is removed from MUSB */
	if ((musb->xceiv->otg->state == OTG_STATE_B_IDLE
		|| musb->xceiv->otg->state == OTG_STATE_A_WAIT_BCON) ||
		(musb->int_usb & MUSB_INTR_DISCONNECT && is_host_active(musb))) {
		mod_timer(&musb_conn_timer, jiffies + TIMER_DELAY);
		musb->a_wait_bcon = TIMER_DELAY;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	return retval;
}

static void musb_conn_timer_handler(unsigned long _musb)
{
	struct musb *musb = (void *)_musb;
	unsigned long flags;
	u16 val;
	static u8 toggle;

	spin_lock_irqsave(&musb->lock, flags);
	switch (musb->xceiv->otg->state) {
	case OTG_STATE_A_IDLE:
	case OTG_STATE_A_WAIT_BCON:
		/* Start a new session */
		val = musb_readw(musb->mregs, MUSB_DEVCTL);
		val &= ~MUSB_DEVCTL_SESSION;
		musb_writew(musb->mregs, MUSB_DEVCTL, val);
		val |= MUSB_DEVCTL_SESSION;
		musb_writew(musb->mregs, MUSB_DEVCTL, val);
		/* Check if musb is host or peripheral. */
		val = musb_readw(musb->mregs, MUSB_DEVCTL);

		if (!(val & MUSB_DEVCTL_BDEVICE)) {
			gpio_set_value(musb->config->gpio_vrsel, 1);
			musb->xceiv->otg->state = OTG_STATE_A_WAIT_BCON;
		} else {
			gpio_set_value(musb->config->gpio_vrsel, 0);
			/* Ignore VBUSERROR and SUSPEND IRQ */
			val = musb_readb(musb->mregs, MUSB_INTRUSBE);
			val &= ~MUSB_INTR_VBUSERROR;
			musb_writeb(musb->mregs, MUSB_INTRUSBE, val);

			val = MUSB_INTR_SUSPEND | MUSB_INTR_VBUSERROR;
			musb_writeb(musb->mregs, MUSB_INTRUSB, val);
			musb->xceiv->otg->state = OTG_STATE_B_IDLE;
		}
		mod_timer(&musb_conn_timer, jiffies + TIMER_DELAY);
		break;
	case OTG_STATE_B_IDLE:
		/*
		 * Start a new session.  It seems that MUSB needs taking
		 * some time to recognize the type of the plug inserted?
		 */
		val = musb_readw(musb->mregs, MUSB_DEVCTL);
		val |= MUSB_DEVCTL_SESSION;
		musb_writew(musb->mregs, MUSB_DEVCTL, val);
		val = musb_readw(musb->mregs, MUSB_DEVCTL);

		if (!(val & MUSB_DEVCTL_BDEVICE)) {
			gpio_set_value(musb->config->gpio_vrsel, 1);
			musb->xceiv->otg->state = OTG_STATE_A_WAIT_BCON;
		} else {
			gpio_set_value(musb->config->gpio_vrsel, 0);

			/* Ignore VBUSERROR and SUSPEND IRQ */
			val = musb_readb(musb->mregs, MUSB_INTRUSBE);
			val &= ~MUSB_INTR_VBUSERROR;
			musb_writeb(musb->mregs, MUSB_INTRUSBE, val);

			val = MUSB_INTR_SUSPEND | MUSB_INTR_VBUSERROR;
			musb_writeb(musb->mregs, MUSB_INTRUSB, val);

			/* Toggle the Soft Conn bit, so that we can response to
			 * the inserting of either A-plug or B-plug.
			 */
			if (toggle) {
				val = musb_readb(musb->mregs, MUSB_POWER);
				val &= ~MUSB_POWER_SOFTCONN;
				musb_writeb(musb->mregs, MUSB_POWER, val);
				toggle = 0;
			} else {
				val = musb_readb(musb->mregs, MUSB_POWER);
				val |= MUSB_POWER_SOFTCONN;
				musb_writeb(musb->mregs, MUSB_POWER, val);
				toggle = 1;
			}
			/* The delay time is set to 1/4 second by default,
			 * shortening it, if accelerating A-plug detection
			 * is needed in OTG mode.
			 */
			mod_timer(&musb_conn_timer, jiffies + TIMER_DELAY / 4);
		}
		break;
	default:
		dev_dbg(musb->controller, "%s state not handled\n",
			usb_otg_state_string(musb->xceiv->otg->state));
		break;
	}
	spin_unlock_irqrestore(&musb->lock, flags);

	dev_dbg(musb->controller, "state is %s\n",
		usb_otg_state_string(musb->xceiv->otg->state));
}

static void bfin_musb_enable(struct musb *musb)
{
	/* REVISIT is this really correct ? */
}

static void bfin_musb_disable(struct musb *musb)
{
}

static void bfin_musb_set_vbus(struct musb *musb, int is_on)
{
	int value = musb->config->gpio_vrsel_active;
	if (!is_on)
		value = !value;
	gpio_set_value(musb->config->gpio_vrsel, value);

	dev_dbg(musb->controller, "VBUS %s, devctl %02x "
		/* otg %3x conf %08x prcm %08x */ "\n",
		usb_otg_state_string(musb->xceiv->otg->state),
		musb_readb(musb->mregs, MUSB_DEVCTL));
}

static int bfin_musb_set_power(struct usb_phy *x, unsigned mA)
{
	return 0;
}

static int bfin_musb_vbus_status(struct musb *musb)
{
	return 0;
}

static int bfin_musb_set_mode(struct musb *musb, u8 musb_mode)
{
	return -EIO;
}

static int bfin_musb_adjust_channel_params(struct dma_channel *channel,
				u16 packet_sz, u8 *mode,
				dma_addr_t *dma_addr, u32 *len)
{
	struct musb_dma_channel *musb_channel = channel->private_data;

	/*
	 * Anomaly 05000450 might cause data corruption when using DMA
	 * MODE 1 transmits with short packet.  So to work around this,
	 * we truncate all MODE 1 transfers down to a multiple of the
	 * max packet size, and then do the last short packet transfer
	 * (if there is any) using MODE 0.
	 */
	if (ANOMALY_05000450) {
		if (musb_channel->transmit && *mode == 1)
			*len = *len - (*len % packet_sz);
	}

	return 0;
}

static void bfin_musb_reg_init(struct musb *musb)
{
	if (ANOMALY_05000346) {
		bfin_write_USB_APHY_CALIB(ANOMALY_05000346_value);
		SSYNC();
	}

	if (ANOMALY_05000347) {
		bfin_write_USB_APHY_CNTRL(0x0);
		SSYNC();
	}

	/* Configure PLL oscillator register */
	bfin_write_USB_PLLOSC_CTRL(0x3080 |
			((480/musb->config->clkin) << 1));
	SSYNC();

	bfin_write_USB_SRP_CLKDIV((get_sclk()/1000) / 32 - 1);
	SSYNC();

	bfin_write_USB_EP_NI0_RXMAXP(64);
	SSYNC();

	bfin_write_USB_EP_NI0_TXMAXP(64);
	SSYNC();

	/* Route INTRUSB/INTR_RX/INTR_TX to USB_INT0*/
	bfin_write_USB_GLOBINTR(0x7);
	SSYNC();

	bfin_write_USB_GLOBAL_CTL(GLOBAL_ENA | EP1_TX_ENA | EP2_TX_ENA |
				EP3_TX_ENA | EP4_TX_ENA | EP5_TX_ENA |
				EP6_TX_ENA | EP7_TX_ENA | EP1_RX_ENA |
				EP2_RX_ENA | EP3_RX_ENA | EP4_RX_ENA |
				EP5_RX_ENA | EP6_RX_ENA | EP7_RX_ENA);
	SSYNC();
}

static int bfin_musb_init(struct musb *musb)
{

	/*
	 * Rev 1.0 BF549 EZ-KITs require PE7 to be high for both DEVICE
	 * and OTG HOST modes, while rev 1.1 and greater require PE7 to
	 * be low for DEVICE mode and high for HOST mode. We set it high
	 * here because we are in host mode
	 */

	if (gpio_request(musb->config->gpio_vrsel, "USB_VRSEL")) {
		printk(KERN_ERR "Failed ro request USB_VRSEL GPIO_%d\n",
			musb->config->gpio_vrsel);
		return -ENODEV;
	}
	gpio_direction_output(musb->config->gpio_vrsel, 0);

	musb->xceiv = usb_get_phy(USB_PHY_TYPE_USB2);
	if (IS_ERR_OR_NULL(musb->xceiv)) {
		gpio_free(musb->config->gpio_vrsel);
		return -EPROBE_DEFER;
	}

	bfin_musb_reg_init(musb);

	setup_timer(&musb_conn_timer, musb_conn_timer_handler,
			(unsigned long) musb);

	musb->xceiv->set_power = bfin_musb_set_power;

	musb->isr = blackfin_interrupt;
	musb->double_buffer_not_ok = true;

	return 0;
}

static int bfin_musb_exit(struct musb *musb)
{
	gpio_free(musb->config->gpio_vrsel);
	usb_put_phy(musb->xceiv);

	return 0;
}

static const struct musb_platform_ops bfin_ops = {
	.init		= bfin_musb_init,
	.exit		= bfin_musb_exit,

	.readb		= bfin_readb,
	.writeb		= bfin_writeb,
	.readw		= bfin_readw,
	.writew		= bfin_writew,
	.readl		= bfin_readl,
	.writel		= bfin_writel,
	.fifo_mode	= 2,
	.read_fifo	= bfin_read_fifo,
	.write_fifo	= bfin_write_fifo,
	.enable		= bfin_musb_enable,
	.disable	= bfin_musb_disable,

	.set_mode	= bfin_musb_set_mode,

	.vbus_status	= bfin_musb_vbus_status,
	.set_vbus	= bfin_musb_set_vbus,

	.adjust_channel_params = bfin_musb_adjust_channel_params,
};

static u64 bfin_dmamask = DMA_BIT_MASK(32);

static int bfin_probe(struct platform_device *pdev)
{
	struct resource musb_resources[2];
	struct musb_hdrc_platform_data	*pdata = dev_get_platdata(&pdev->dev);
	struct platform_device		*musb;
	struct bfin_glue		*glue;

	int				ret = -ENOMEM;

	glue = devm_kzalloc(&pdev->dev, sizeof(*glue), GFP_KERNEL);
	if (!glue)
		goto err0;

	musb = platform_device_alloc("musb-hdrc", PLATFORM_DEVID_AUTO);
	if (!musb)
		goto err0;

	musb->dev.parent		= &pdev->dev;
	musb->dev.dma_mask		= &bfin_dmamask;
	musb->dev.coherent_dma_mask	= bfin_dmamask;

	glue->dev			= &pdev->dev;
	glue->musb			= musb;

	pdata->platform_ops		= &bfin_ops;

	glue->phy = usb_phy_generic_register();
	if (IS_ERR(glue->phy))
		goto err1;
	platform_set_drvdata(pdev, glue);

	memset(musb_resources, 0x00, sizeof(*musb_resources) *
			ARRAY_SIZE(musb_resources));

	musb_resources[0].name = pdev->resource[0].name;
	musb_resources[0].start = pdev->resource[0].start;
	musb_resources[0].end = pdev->resource[0].end;
	musb_resources[0].flags = pdev->resource[0].flags;

	musb_resources[1].name = pdev->resource[1].name;
	musb_resources[1].start = pdev->resource[1].start;
	musb_resources[1].end = pdev->resource[1].end;
	musb_resources[1].flags = pdev->resource[1].flags;

	ret = platform_device_add_resources(musb, musb_resources,
			ARRAY_SIZE(musb_resources));
	if (ret) {
		dev_err(&pdev->dev, "failed to add resources\n");
		goto err2;
	}

	ret = platform_device_add_data(musb, pdata, sizeof(*pdata));
	if (ret) {
		dev_err(&pdev->dev, "failed to add platform_data\n");
		goto err2;
	}

	ret = platform_device_add(musb);
	if (ret) {
		dev_err(&pdev->dev, "failed to register musb device\n");
		goto err2;
	}

	return 0;

err2:
	usb_phy_generic_unregister(glue->phy);

err1:
	platform_device_put(musb);

err0:
	return ret;
}

static int bfin_remove(struct platform_device *pdev)
{
	struct bfin_glue		*glue = platform_get_drvdata(pdev);

	platform_device_unregister(glue->musb);
	usb_phy_generic_unregister(glue->phy);

	return 0;
}

#ifdef CONFIG_PM
static int bfin_suspend(struct device *dev)
{
	struct bfin_glue	*glue = dev_get_drvdata(dev);
	struct musb		*musb = glue_to_musb(glue);

	if (is_host_active(musb))
		/*
		 * During hibernate gpio_vrsel will change from high to low
		 * low which will generate wakeup event resume the system
		 * immediately.  Set it to 0 before hibernate to avoid this
		 * wakeup event.
		 */
		gpio_set_value(musb->config->gpio_vrsel, 0);

	return 0;
}

static int bfin_resume(struct device *dev)
{
	struct bfin_glue	*glue = dev_get_drvdata(dev);
	struct musb		*musb = glue_to_musb(glue);

	bfin_musb_reg_init(musb);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(bfin_pm_ops, bfin_suspend, bfin_resume);

static struct platform_driver bfin_driver = {
	.probe		= bfin_probe,
	.remove		= bfin_remove,
	.driver		= {
		.name	= "musb-blackfin",
		.pm	= &bfin_pm_ops,
	},
};

MODULE_DESCRIPTION("Blackfin MUSB Glue Layer");
MODULE_AUTHOR("Bryan Wy <cooloney@kernel.org>");
MODULE_LICENSE("GPL v2");
module_platform_driver(bfin_driver);
