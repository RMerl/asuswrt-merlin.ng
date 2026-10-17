// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <asm/global_data.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
}

int board_init(void)
{
	setup_fec();

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "DDR4 EVK");
	env_set("board_rev", "iMX8MN");
#endif
	return 0;
}
