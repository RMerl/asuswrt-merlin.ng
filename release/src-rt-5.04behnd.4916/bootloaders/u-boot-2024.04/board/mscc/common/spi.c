// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Coprporation
 */

#include <common.h>
#include <asm/io.h>
#include <spi.h>
#include <linux/bitops.h>

void external_cs_manage(struct udevice *dev, bool enable)
{
	u32 cs = spi_chip_select(dev);
	/* IF_SI0_OWNER, select the owner of the SI interface
	 * Encoding: 0: SI Slave
	 *	     1: SI Boot Master
	 *	     2: SI Master Controller
	 */
	if (!enable) {
		writel(ICPU_SW_MODE_SW_PIN_CTRL_MODE |
		       ICPU_SW_MODE_SW_SPI_CS(BIT(cs)),
		       BASE_CFG + ICPU_SW_MODE);
		clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
				ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
				ICPU_GENERAL_CTRL_IF_SI_OWNER(2));
	} else {
		writel(0, BASE_CFG + ICPU_SW_MODE);
		clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
				ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
				ICPU_GENERAL_CTRL_IF_SI_OWNER(1));
	}
}
