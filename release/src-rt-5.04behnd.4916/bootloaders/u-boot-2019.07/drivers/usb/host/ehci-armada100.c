// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * This driver is based on Kirkwood echi driver
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ehci.h"
#include <asm/arch/cpu.h>
#include <asm/arch/armada100.h>
#include <asm/arch/utmi-armada100.h>

/*
 * EHCI host controller init
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	if (utmi_init() < 0)
		return -1;

	*hccr = (struct ehci_hccr *)(ARMD1_USB_HOST_BASE + 0x100);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
			+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("armada100-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

/*
 * EHCI host controller stop
 */
int ehci_hcd_stop(int index)
{
	return 0;
}
