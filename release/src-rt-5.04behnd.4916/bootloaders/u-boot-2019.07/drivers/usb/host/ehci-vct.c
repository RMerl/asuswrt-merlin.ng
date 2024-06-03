// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#include <common.h>
#include <usb.h>

#include "ehci.h"

int vct_ehci_hcd_init(u32 *hccr, u32 *hcor);

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	int ret;
	u32 vct_hccr;
	u32 vct_hcor;

	/*
	 * Init VCT specific stuff
	 */
	ret = vct_ehci_hcd_init(&vct_hccr, &vct_hcor);
	if (ret)
		return ret;

	*hccr = (struct ehci_hccr *)vct_hccr;
	*hcor = (struct ehci_hcor *)vct_hcor;

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
