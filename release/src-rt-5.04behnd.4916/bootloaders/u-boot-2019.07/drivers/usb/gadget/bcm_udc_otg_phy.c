// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Broadcom Corporation.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>

#include "dwc2_udc_otg_priv.h"
#include "bcm_udc_otg.h"

void otg_phy_init(struct dwc2_udc *dev)
{
	/* turn on the USB OTG clocks */
	clk_usb_otg_enable((void *)HSOTG_BASE_ADDR);

	/* set Phy to driving mode */
	wfld_clear(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		   HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);

	udelay(100);

	/* clear Soft Disconnect */
	wfld_clear(HSOTG_BASE_ADDR + HSOTG_DCTL_OFFSET,
		   HSOTG_DCTL_SFTDISCON_MASK);

	/* invoke Reset (active low) */
	wfld_clear(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		   HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);

	/* Reset needs to be asserted for 2ms */
	udelay(2000);

	/* release Reset */
	wfld_set(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		 HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK,
		 HSOTG_CTRL_PHY_P1CTL_SOFT_RESET_MASK);
}

void otg_phy_off(struct dwc2_udc *dev)
{
	/* Soft Disconnect */
	wfld_set(HSOTG_BASE_ADDR + HSOTG_DCTL_OFFSET,
		 HSOTG_DCTL_SFTDISCON_MASK,
		 HSOTG_DCTL_SFTDISCON_MASK);

	/* set Phy to non-driving (reset) mode */
	wfld_set(HSOTG_CTRL_BASE_ADDR + HSOTG_CTRL_PHY_P1CTL_OFFSET,
		 HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK,
		 HSOTG_CTRL_PHY_P1CTL_NON_DRIVING_MASK);
}
