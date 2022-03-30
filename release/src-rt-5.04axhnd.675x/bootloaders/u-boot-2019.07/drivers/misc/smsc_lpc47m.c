// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pnp_def.h>

static void pnp_enter_conf_state(u16 dev)
{
	u16 port = dev >> 8;

	outb(0x55, port);
}

static void pnp_exit_conf_state(u16 dev)
{
	u16 port = dev >> 8;

	outb(0xaa, port);
}

void lpc47m_enable_serial(uint dev, uint iobase, uint irq)
{
	pnp_enter_conf_state(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, iobase);
	pnp_set_irq(dev, PNP_IDX_IRQ0, irq);
	pnp_set_enable(dev, 1);
	pnp_exit_conf_state(dev);
}

void lpc47m_enable_kbc(uint dev, uint irq0, uint irq1)
{
	pnp_enter_conf_state(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_irq(dev, PNP_IDX_IRQ0, irq0);
	pnp_set_irq(dev, PNP_IDX_IRQ1, irq1);
	pnp_set_enable(dev, 1);
	pnp_exit_conf_state(dev);
}
