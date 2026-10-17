// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <dt-structs.h>
#include <ns16550.h>
#include <serial.h>
#include <asm/arch-rockchip/clock.h>
#include <dm/device-internal.h>

struct rockchip_uart_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_uart dtplat;
#endif
	struct ns16550_plat plat;
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct dtd_rockchip_uart *dtplat, s_dtplat;
#endif

static int rockchip_serial_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_uart_plat *plat = dev_get_plat(dev);

	/* Create some new platform data for the standard driver */
	plat->plat.base = plat->dtplat.reg[0];
	plat->plat.reg_shift = plat->dtplat.reg_shift;
	plat->plat.clock = plat->dtplat.clock_frequency;
	plat->plat.fcr = UART_FCR_DEFVAL;
	dev_set_plat(dev, &plat->plat);

	return ns16550_serial_probe(dev);
#else
	return -ENODEV;
#endif
}

U_BOOT_DRIVER(rockchip_uart) = {
	.name		= "rockchip_uart",
	.id		= UCLASS_SERIAL,
	.priv_auto	= sizeof(struct ns16550),
	.plat_auto	= sizeof(struct rockchip_uart_plat),
	.probe		= rockchip_serial_probe,
	.ops		= &ns16550_serial_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
DM_DRIVER_ALIAS(rockchip_uart, rockchip_rk3066_uart)
DM_DRIVER_ALIAS(rockchip_uart, rockchip_rk3188_uart)
DM_DRIVER_ALIAS(rockchip_uart, rockchip_rk3288_uart)
DM_DRIVER_ALIAS(rockchip_uart, rockchip_rk3328_uart)
DM_DRIVER_ALIAS(rockchip_uart, rockchip_rk3368_uart)
