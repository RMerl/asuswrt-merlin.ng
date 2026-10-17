// SPDX-License-Identifier: GPL-2.0
/*
 * Special driver to handle of-platdata
 *
 * Copyright 2019 Google LLC
 *
 * Some code from coreboot lpss.c
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/lpss.h>

enum {
	LPSS_RESET_CTL_REG	= 0x204,

	/*
	 * Bit 1:0 controls LPSS controller reset.
	 *
	 * 00 ->LPSS Host Controller is in reset (Reset Asserted)
	 * 01/10 ->Reserved
	 * 11 ->LPSS Host Controller is NOT at reset (Reset Released)
	 */
	LPSS_CNT_RST_RELEASE	= 3,

	/* Power management control and status register */
	PME_CTRL_STATUS		= 0x84,

	/* Bit 1:0 Powerstate, controls D0 and D3 state */
	POWER_STATE_MASK	= 3,
};

/* Take controller out of reset */
void lpss_reset_release(void *regs)
{
	writel(LPSS_CNT_RST_RELEASE, regs + LPSS_RESET_CTL_REG);
}

void lpss_set_power_state(struct udevice *dev, enum lpss_pwr_state state)
{
	dm_pci_clrset_config8(dev, PME_CTRL_STATUS, POWER_STATE_MASK, state);
}
