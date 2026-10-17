// SPDX-License-Identifier: GPL-2.0
/*
 * Based on vendor support provided by AVNET Embedded
 *
 * Copyright (C) 2021 AVNET Embedded, MSC Technologies GmbH
 * Copyright 2021 General Electric Company
 * Copyright 2021 Collabora Ltd.
 */

#include <common.h>
#include <errno.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}

int board_init(void)
{
	setup_fec();

	return 0;
}
