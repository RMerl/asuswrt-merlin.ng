// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <park@nexell.co.kr>
 */

/*
 *FIXME : Not support device tree & reset control driver.
 *        will remove after support device tree & reset control driver.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/nexell.h>
#include <asm/arch/reset.h>

struct	nx_rstcon_registerset {
	u32	regrst[(NUMBER_OF_RESET_MODULE_PIN + 31) >> 5];
};

static struct nx_rstcon_registerset *nx_rstcon =
			(struct nx_rstcon_registerset *)PHY_BASEADDR_RSTCON;

void nx_rstcon_setrst(u32 rstindex, enum rstcon status)
{
	u32 regnum, bitpos, curstat;

	regnum		= rstindex >> 5;
	curstat		= (u32)readl(&nx_rstcon->regrst[regnum]);
	bitpos		= rstindex & 0x1f;
	curstat		&= ~(1UL << bitpos);
	curstat		|= (status & 0x01) << bitpos;
	writel(curstat, &nx_rstcon->regrst[regnum]);
}
