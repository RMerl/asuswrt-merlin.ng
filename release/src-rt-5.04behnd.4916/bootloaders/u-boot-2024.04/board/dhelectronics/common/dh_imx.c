// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 * Copyright 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <common.h>
#include <net.h>
#include "dh_imx.h"

int dh_imx_get_mac_from_fuse(unsigned char *enetaddr)
{
	/*
	 * If IIM fuses contain valid MAC address, use it.
	 * The IIM MAC address fuses are NOT programmed by default.
	 */
	imx_get_mac_from_fuse(0, enetaddr);
	if (!is_valid_ethaddr(enetaddr))
		return -EINVAL;

	return 0;
}
