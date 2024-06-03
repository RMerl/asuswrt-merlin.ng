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

#if defined(CONFIG_ROCKCHIP_RK3188)
struct rockchip_uart_platdata {
	struct dtd_rockchip_rk3188_uart dtplat;
	struct ns16550_platdata plat;
};
struct dtd_rockchip_rk3188_uart *dtplat, s_dtplat;
#elif defined(CONFIG_ROCKCHIP_RK3288)
struct rockchip_uart_platdata {
	struct dtd_rockchip_rk3288_uart dtplat;
	struct ns16550_platdata plat;
};
struct dtd_rockchip_rk3288_uart *dtplat, s_dtplat;
#endif

static int rockchip_serial_probe(struct udevice *dev)
{
	struct rockchip_uart_platdata *plat = dev_get_platdata(dev);

	/* Create some new platform data for the standard driver */
	plat->plat.base = plat->dtplat.reg[0];
	plat->plat.reg_shift = plat->dtplat.reg_shift;
	plat->plat.clock = plat->dtplat.clock_frequency;
	plat->plat.fcr = UART_FCR_DEFVAL;
	dev->platdata = &plat->plat;

	return ns16550_serial_probe(dev);
}

U_BOOT_DRIVER(rockchip_rk3188_uart) = {
	.name	= "rockchip_rk3188_uart",
	.id	= UCLASS_SERIAL,
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.platdata_auto_alloc_size = sizeof(struct rockchip_uart_platdata),
	.probe	= rockchip_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(rockchip_rk3288_uart) = {
	.name	= "rockchip_rk3288_uart",
	.id	= UCLASS_SERIAL,
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.platdata_auto_alloc_size = sizeof(struct rockchip_uart_platdata),
	.probe	= rockchip_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
