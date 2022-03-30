// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Armando Visconti, ST Micoelectronics, <armando.visconti@st.com>.
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ehci.h"
#include <asm/arch/hardware.h>
#include <asm/arch/spr_misc.h>

static void spear6xx_usbh_stop(void)
{
	struct misc_regs *const misc_p =
	    (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 periph1_rst = readl(misc_p->periph1_rst);

	periph1_rst |= PERIPH_USBH1 | PERIPH_USBH2;
	writel(periph1_rst, misc_p->periph1_rst);

	udelay(1000);
	periph1_rst &= ~(PERIPH_USBH1 | PERIPH_USBH2);
	writel(periph1_rst, misc_p->periph1_rst);
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 ehci = 0;

	switch (index) {
	case 0:
		ehci = CONFIG_SYS_UHC0_EHCI_BASE;
		break;
	case 1:
		ehci = CONFIG_SYS_UHC1_EHCI_BASE;
		break;
	default:
		printf("ERROR: wrong controller index!\n");
		break;
	};

	*hccr = (struct ehci_hccr *)(ehci + 0x100);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("SPEAr-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
#if defined(CONFIG_SPEAR600)
	spear6xx_usbh_stop();
#endif

	return 0;
}
