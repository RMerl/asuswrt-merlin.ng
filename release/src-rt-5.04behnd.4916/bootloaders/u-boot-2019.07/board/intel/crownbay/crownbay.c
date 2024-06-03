// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>
#include <smsc_lpc47m.h>

int board_early_init_f(void)
{
	lpc47m_enable_serial(PNP_DEV(LPC47M_IO_PORT, LPC47M_SP1),
			     UART0_BASE, UART0_IRQ);
	lpc47m_enable_kbc(PNP_DEV(LPC47M_IO_PORT, LPC47M_KBC),
			  KBD_IRQ, MSE_IRQ);

	return 0;
}
