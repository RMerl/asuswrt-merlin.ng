// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone EVM : Power off
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/mon.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/hardware.h>

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	mon_power_off(0);

	psc_disable_module(KS2_LPSC_TETRIS);
	psc_disable_domain(KS2_TETRIS_PWR_DOMAIN);

	asm volatile ("isb\n"
		      "dsb\n"
		      "wfi\n");

	return 0;
}
