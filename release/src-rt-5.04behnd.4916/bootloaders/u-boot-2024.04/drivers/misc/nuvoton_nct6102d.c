// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <nuvoton_nct6102d.h>
#include <asm/io.h>
#include <asm/pnp_def.h>

static void superio_outb(int reg, int val)
{
	outb(reg, NCT_EFER);
	outb(val, NCT_EFDR);
}

static inline int superio_inb(int reg)
{
	outb(reg, NCT_EFER);
	return inb(NCT_EFDR);
}

static int superio_enter(void)
{
	outb(NCT_ENTRY_KEY, NCT_EFER); /* Enter extended function mode */
	outb(NCT_ENTRY_KEY, NCT_EFER); /* Again according to manual */

	return 0;
}

static void superio_select(int ld)
{
	superio_outb(NCT_LD_SELECT_REG, ld);
}

static void superio_exit(void)
{
	outb(NCT_EXIT_KEY, NCT_EFER); /* Leave extended function mode */
}

/*
 * The Nuvoton NCT6102D starts per default after reset with both,
 * the internal watchdog and the internal legacy UART enabled. This
 * code provides a function to disable the watchdog.
 */
int nct6102d_wdt_disable(void)
{
	superio_enter();
	/* Select logical device for WDT */
	superio_select(NCT6102D_LD_WDT);
	superio_outb(NCT6102D_WDT_TIMEOUT, 0x00);
	superio_exit();

	return 0;
}
