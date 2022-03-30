// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Stephen Warren <swarren@wwwdotorg.org>
 *
 * Derived from pl01x code:
 *
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */

/* Simple U-Boot driver for the BCM283x mini UART */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <watchdog.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <serial.h>
#include <dm/platform_data/serial_bcm283x_mu.h>
#include <dm/pinctrl.h>
#include <linux/compiler.h>

struct bcm283x_mu_regs {
	u32 io;
	u32 iir;
	u32 ier;
	u32 lcr;
	u32 mcr;
	u32 lsr;
	u32 msr;
	u32 scratch;
	u32 cntl;
	u32 stat;
	u32 baud;
};

#define BCM283X_MU_LCR_DATA_SIZE_8	3

#define BCM283X_MU_LSR_TX_IDLE		BIT(6)
/* This actually means not full, but is named not empty in the docs */
#define BCM283X_MU_LSR_TX_EMPTY		BIT(5)
#define BCM283X_MU_LSR_RX_READY		BIT(0)

struct bcm283x_mu_priv {
	struct bcm283x_mu_regs *regs;
};

static int bcm283x_mu_serial_getc(struct udevice *dev);

static int bcm283x_mu_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	u32 divider;

	if (plat->skip_init)
		goto out;

	divider = plat->clock / (baudrate * 8);

	writel(BCM283X_MU_LCR_DATA_SIZE_8, &regs->lcr);
	writel(divider - 1, &regs->baud);

out:
	/* Flush the RX queue - all data in there is bogus */
	while (bcm283x_mu_serial_getc(dev) != -EAGAIN) ;

	return 0;
}

static int bcm283x_mu_serial_probe(struct udevice *dev)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);

	priv->regs = (struct bcm283x_mu_regs *)plat->base;

	return 0;
}

static int bcm283x_mu_serial_getc(struct udevice *dev)
{
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	u32 data;

	/* Wait until there is data in the FIFO */
	if (!(readl(&regs->lsr) & BCM283X_MU_LSR_RX_READY))
		return -EAGAIN;

	data = readl(&regs->io);

	return (int)data;
}

static int bcm283x_mu_serial_putc(struct udevice *dev, const char data)
{
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;

	/* Wait until there is space in the FIFO */
	if (!(readl(&regs->lsr) & BCM283X_MU_LSR_TX_EMPTY))
		return -EAGAIN;

	/* Send the character */
	writel(data, &regs->io);

	return 0;
}

static int bcm283x_mu_serial_pending(struct udevice *dev, bool input)
{
	struct bcm283x_mu_priv *priv = dev_get_priv(dev);
	struct bcm283x_mu_regs *regs = priv->regs;
	unsigned int lsr;

	lsr = readl(&regs->lsr);

	if (input) {
		WATCHDOG_RESET();
		return (lsr & BCM283X_MU_LSR_RX_READY) ? 1 : 0;
	} else {
		return (lsr & BCM283X_MU_LSR_TX_IDLE) ? 0 : 1;
	}
}

static const struct dm_serial_ops bcm283x_mu_serial_ops = {
	.putc = bcm283x_mu_serial_putc,
	.pending = bcm283x_mu_serial_pending,
	.getc = bcm283x_mu_serial_getc,
	.setbrg = bcm283x_mu_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id bcm283x_mu_serial_id[] = {
	{.compatible = "brcm,bcm2835-aux-uart"},
	{}
};

/*
 * Check if this serial device is muxed
 *
 * The serial device will only work properly if it has been muxed to the serial
 * pins by firmware. Check whether that happened here.
 *
 * @return true if serial device is muxed, false if not
 */
static bool bcm283x_is_serial_muxed(void)
{
	int serial_gpio = 15;
	struct udevice *dev;

	if (uclass_first_device(UCLASS_PINCTRL, &dev) || !dev)
		return false;

	if (pinctrl_get_gpio_mux(dev, 0, serial_gpio) != BCM2835_GPIO_ALT5)
		return false;

	return true;
}

static int bcm283x_mu_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct bcm283x_mu_serial_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	/* Don't spawn the device if it's not muxed */
	if (!bcm283x_is_serial_muxed())
		return -ENODEV;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	plat->clock = dev_read_u32_default(dev, "clock", 1);

	/*
	 * TODO: Reinitialization doesn't always work for now, just skip
	 *       init always - we know we're already initialized
	 */
	plat->skip_init = true;

	return 0;
}
#endif

U_BOOT_DRIVER(serial_bcm283x_mu) = {
	.name = "serial_bcm283x_mu",
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(bcm283x_mu_serial_id),
	.ofdata_to_platdata = of_match_ptr(bcm283x_mu_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct bcm283x_mu_serial_platdata),
	.probe = bcm283x_mu_serial_probe,
	.ops = &bcm283x_mu_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
	.priv_auto_alloc_size = sizeof(struct bcm283x_mu_priv),
};
