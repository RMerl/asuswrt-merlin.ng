// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022  Stefan Bosch <stefan_b@posteo.net>
 */

#include <common.h>
#include <dm.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <linux/delay.h>

#include <dm/platform_data/serial_pl01x.h>
#include <serial.h>
#include "serial_pl01x_internal.h"

int s5p4418_pl011_serial_probe(struct udevice *dev)
{
	struct pl01x_serial_plat *plat = dev_get_plat(dev);
	struct clk *nx_clk;
	ulong rate_act;
	char uart_clk_name[10];
	int uart_num = -1;
	int rst_id, ret;

	if (!plat->skip_init) {
		uart_num = dev->seq_;
		rst_id = RESET_ID_UART0 + uart_num;

		if (uart_num < 0 || rst_id > RESET_ID_UART5) {
			/* invalid UART-number */
			debug("%s: sequence/uart number %d is invalid!\n", __func__, uart_num);
			return -ENODEV;
		}

		sprintf(uart_clk_name, "nx-uart.%d", uart_num);
		nx_clk = clk_get(uart_clk_name);
		if (!nx_clk) {
			debug("%s: clk_get('%s') failed!\n", __func__, uart_clk_name);
			return -ENODEV;
		}

		/* wait to make sure all pending characters have been sent */
		mdelay(100);
	}

	/*
	 * Note: Unless !plat->skip_init, the UART is disabled here, so printf()
	 * or debug() must not be used until pl01x_serial_setbrg() has been called
	 * (enables the UART). Otherwise u-boot is hanging!
	 */
	ret = pl01x_serial_probe(dev);
	if (ret)
		return ret;

	if (!plat->skip_init) {
		/* do reset UART */
		nx_rstcon_setrst(rst_id, RSTCON_ASSERT);
		udelay(10);
		nx_rstcon_setrst(rst_id, RSTCON_NEGATE);
		udelay(10);
		clk_disable(nx_clk);

		rate_act = clk_set_rate(nx_clk, plat->clock);
		clk_enable(nx_clk);

		plat->clock = rate_act;
	}

	return 0;
}

static const struct dm_serial_ops s5p4418_pl011_serial_ops = {
	.putc = pl01x_serial_putc,
	.pending = pl01x_serial_pending,
	.getc = pl01x_serial_getc,
	.setbrg = pl01x_serial_setbrg,
};

static const struct udevice_id s5p4418_pl011_serial_id[] = {
	{.compatible = "nexell,s5p4418-pl011", .data = TYPE_PL011},
	{}
};

U_BOOT_DRIVER(s5p4418_pl011_uart) = {
	.name	= "s5p4418_pl011",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(s5p4418_pl011_serial_id),
	.of_to_plat = of_match_ptr(pl01x_serial_of_to_plat),
	.plat_auto	= sizeof(struct pl01x_serial_plat),
	.probe = s5p4418_pl011_serial_probe,
	.ops	= &s5p4418_pl011_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(struct pl01x_priv),
};
