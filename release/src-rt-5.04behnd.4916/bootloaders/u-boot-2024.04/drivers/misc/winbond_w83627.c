// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pnp_def.h>

#define WINBOND_ENTRY_KEY	0x87
#define WINBOND_EXIT_KEY	0xaa

/* Enable configuration: pass entry key '0x87' into index port dev twice */
static void pnp_enter_conf_state(u16 dev)
{
	u16 port = dev >> 8;

	outb(WINBOND_ENTRY_KEY, port);
	outb(WINBOND_ENTRY_KEY, port);
}

/* Disable configuration: pass exit key '0xAA' into index port dev */
static void pnp_exit_conf_state(u16 dev)
{
	u16 port = dev >> 8;

	outb(WINBOND_EXIT_KEY, port);
}

/* Bring up early serial debugging output before the RAM is initialized */
void winbond_enable_serial(uint dev, uint iobase, uint irq)
{
	pnp_enter_conf_state(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, iobase);
	pnp_set_irq(dev, PNP_IDX_IRQ0, irq);
	pnp_set_enable(dev, 1);
	pnp_exit_conf_state(dev);
}
