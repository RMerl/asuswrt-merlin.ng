// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Alexander Graf <agraf@suse.de>
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <dm/platform_data/serial_pl01x.h>
#include <serial.h>
#include "serial_pl01x_internal.h"

/*
 * Check if this serial device is muxed
 *
 * The serial device will only work properly if it has been muxed to the serial
 * pins by firmware. Check whether that happened here.
 *
 * Return: true if serial device is muxed, false if not
 */
static bool bcm283x_is_serial_muxed(void)
{
	int serial_gpio = 15;
	struct udevice *dev;

	if (uclass_first_device_err(UCLASS_PINCTRL, &dev))
		return false;

	if (pinctrl_get_gpio_mux(dev, 0, serial_gpio) != BCM2835_GPIO_ALT0)
		return false;

	return true;
}

static int bcm283x_pl011_serial_probe(struct udevice *dev)
{
	struct pl01x_serial_plat *plat = dev_get_plat(dev);
	int ret;

	/* Don't spawn the device if it's not muxed */
	if (!bcm283x_is_serial_muxed())
		return -ENODEV;

	/*
	 * Read the ofdata here rather than in an of_to_plat() method
	 * since we need the soc simple-bus to be probed so that the 'ranges'
	 * property is used.
	 */
	ret = pl01x_serial_of_to_plat(dev);
	if (ret)
		return ret;

	/*
	 * TODO: Reinitialization doesn't always work for now, just skip
	 *       init always - we know we're already initialized
	 */
	plat->skip_init = true;

	return pl01x_serial_probe(dev);
}

static int bcm283x_pl011_serial_setbrg(struct udevice *dev, int baudrate)
{
	int r;

	r = pl01x_serial_setbrg(dev, baudrate);

	/*
	 * We may have been muxed to a bogus line before. Drain the RX
	 * queue so we start at a clean slate.
	 */
	while (pl01x_serial_getc(dev) != -EAGAIN) ;

	return r;
}

static const struct dm_serial_ops bcm283x_pl011_serial_ops = {
	.putc = pl01x_serial_putc,
	.pending = pl01x_serial_pending,
	.getc = pl01x_serial_getc,
	.setbrg = bcm283x_pl011_serial_setbrg,
};

static const struct udevice_id bcm283x_pl011_serial_id[] = {
	{.compatible = "brcm,bcm2835-pl011", .data = TYPE_PL011},
	{}
};

U_BOOT_DRIVER(bcm283x_pl011_uart) = {
	.name	= "bcm283x_pl011",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(bcm283x_pl011_serial_id),
	.probe	= bcm283x_pl011_serial_probe,
	.plat_auto	= sizeof(struct pl01x_serial_plat),
	.ops	= &bcm283x_pl011_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL) || IS_ENABLED(CONFIG_OF_BOARD)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
	.priv_auto	= sizeof(struct pl01x_priv),
};
