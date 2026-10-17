// SPDX-License-Identifier: GPL-2.0+
/*
 * This file configures the internal USB PHY in AM35X.
 *
 * Copyright (C) 2012 Ilya Yanok <ilya.yanok@gmail.com>
 *
 * Based on omap_phy_internal.c code from Linux by
 * Hema HK <hemahk@ti.com>
 */

#include <common.h>
#include <log.h>
#include <dm/device.h>
#include <asm/io.h>
#include <asm/arch/am35x_def.h>

void am35x_musb_reset(struct udevice *dev)
{
	/* Reset the musb interface */
	clrsetbits_le32(&am35x_scm_general_regs->ip_sw_reset,
			0, USBOTGSS_SW_RST);
	clrsetbits_le32(&am35x_scm_general_regs->ip_sw_reset,
			USBOTGSS_SW_RST, 0);
}

void am35x_musb_phy_power(struct udevice *dev, u8 on)
{
	unsigned long start = get_timer(0);

	if (on) {
		/*
		 * Start the on-chip PHY and its PLL.
		 */
		clrsetbits_le32(&am35x_scm_general_regs->devconf2,
				CONF2_RESET | CONF2_PHYPWRDN | CONF2_OTGPWRDN,
				CONF2_PHY_PLLON);

		debug("Waiting for PHY clock good...\n");
		while (!(readl(&am35x_scm_general_regs->devconf2)
				& CONF2_PHYCLKGD)) {

			if (get_timer(start) > CONFIG_SYS_HZ / 10) {
				printf("musb PHY clock good timed out\n");
				break;
			}
		}
	} else {
		/*
		 * Power down the on-chip PHY.
		 */
		clrsetbits_le32(&am35x_scm_general_regs->devconf2,
				CONF2_PHY_PLLON,
				CONF2_PHYPWRDN | CONF2_OTGPWRDN);
	}
}

void am35x_musb_clear_irq(struct udevice *dev)
{
	clrsetbits_le32(&am35x_scm_general_regs->lvl_intr_clr,
			0, USBOTGSS_INT_CLR);
	readl(&am35x_scm_general_regs->lvl_intr_clr);
}
