// SPDX-License-Identifier: GPL-2.0+
/*
 * Power and Sleep Controller (PSC) functions.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

/*
 * The PSC manages three inputs to a "module" which may be a peripheral or
 * CPU.  Those inputs are the module's:  clock; reset signal; and sometimes
 * its power domain.  For our purposes, we only care whether clock and power
 * are active, and the module is out of reset.
 *
 * DaVinci chips may include two separate power domains: "Always On" and "DSP".
 * Chips without a DSP generally have only one domain.
 *
 * The "Always On" power domain is always on when the chip is on, and is
 * powered by the VDD pins (on DM644X). The majority of DaVinci modules
 * lie within the "Always On" power domain.
 *
 * A separate domain called the "DSP" domain houses the C64x+ and other video
 * hardware such as VICP. In some chips, the "DSP" domain is not always on.
 * The "DSP" power domain is powered by the CVDDDSP pins (on DM644X).
 */

/* Works on Always On power domain only (no PD argument) */
static void lpsc_transition(unsigned int id, unsigned int state)
{
	dv_reg_p mdstat, mdctl, ptstat, ptcmd;
	struct davinci_psc_regs *psc_regs;

	if (id < DAVINCI_LPSC_PSC1_BASE) {
		if (id >= PSC_PSC0_MODULE_ID_CNT)
			return;
		psc_regs = davinci_psc0_regs;
		mdstat = &psc_regs->psc0.mdstat[id];
		mdctl = &psc_regs->psc0.mdctl[id];
	} else {
		id -= DAVINCI_LPSC_PSC1_BASE;
		if (id >= PSC_PSC1_MODULE_ID_CNT)
			return;
		psc_regs = davinci_psc1_regs;
		mdstat = &psc_regs->psc1.mdstat[id];
		mdctl = &psc_regs->psc1.mdctl[id];
	}
	ptstat = &psc_regs->ptstat;
	ptcmd = &psc_regs->ptcmd;

	while (readl(ptstat) & 0x01)
		continue;

	if ((readl(mdstat) & PSC_MDSTAT_STATE) == state)
		return; /* Already in that state */

	writel((readl(mdctl) & ~PSC_MDCTL_NEXT) | state, mdctl);
	writel(0x01, ptcmd);

	while (readl(ptstat) & 0x01)
		continue;
	while ((readl(mdstat) & PSC_MDSTAT_STATE) != state)
		continue;
}

void lpsc_on(unsigned int id)
{
	lpsc_transition(id, 0x03);
}

void lpsc_syncreset(unsigned int id)
{
	lpsc_transition(id, 0x01);
}

void lpsc_disable(unsigned int id)
{
	lpsc_transition(id, 0x0);
}
