// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 */

#include <common.h>
#include <spl.h>
#include <linux/bitops.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "../sg-regs.h"

#define SDCTRL_EMMC_HW_RESET	0x59810280

void uniphier_ld11_clk_init(void)
{
	/* if booted from a device other than USB, without stand-by MPU */
	if ((readl(SG_PINMON0) & BIT(27)) &&
	    uniphier_boot_device_raw() != BOOT_DEVICE_USB) {
		writel(1, SG_ETPHYPSHUT);
		writel(1, SG_ETPHYCNT);

		udelay(1); /* wait for regulator level 1.1V -> 2.5V */

		writel(3, SG_ETPHYCNT);
		writel(3, SG_ETPHYPSHUT);
		writel(7, SG_ETPHYCNT);
	}

	/* TODO: use "mmc-pwrseq-emmc" */
	writel(1, SDCTRL_EMMC_HW_RESET);

#ifdef CONFIG_USB_EHCI_HCD
	{
		int ch;

		for (ch = 0; ch < 3; ch++) {
			void __iomem *phyctrl = (void __iomem *)SG_USBPHYCTRL;

			writel(0x82280600, phyctrl + 8 * ch);
			writel(0x00000106, phyctrl + 8 * ch + 4);
		}
	}
#endif
}
