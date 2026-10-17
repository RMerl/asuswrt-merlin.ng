// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2021 BSH Hausgeraete GmbH
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <common.h>
#include <env.h>
#include <linux/sizes.h>

static void setup_gpmi_nand(void)
{
	setup_gpmi_io_clk((MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			   MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			   MXC_CCM_CS2CDR_ENFC_CLK_SEL(3)));
};

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_gpmi_nand();

	return 0;
}

int board_late_init(void)
{
	if (is_boot_from_usb()) {
		env_set("bootcmd", "run bootcmd_mfg");
		env_set("bootdelay", "0");
	}

	return 0;
}
