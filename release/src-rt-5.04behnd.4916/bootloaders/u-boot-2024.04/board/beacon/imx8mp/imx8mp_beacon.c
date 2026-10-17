// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2023 Logic PD, Inc dba Beacon EmbeddedWorks */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <asm/arch/sys_proto.h>

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
}

#if IS_ENABLED(CONFIG_NET)
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

int board_init(void)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(FEC_MXC))
		setup_fec();

	return ret;
}
