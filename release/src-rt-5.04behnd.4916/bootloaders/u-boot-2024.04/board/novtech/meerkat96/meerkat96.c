// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Linaro Ltd.
 * Copyright (C) 2016 NXP Semiconductors
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <common.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | \
			PAD_CTL_PUS_PU100KOHM | PAD_CTL_HYS)

static iomux_v3_cfg_t const meerkat96_pads[] = {
	/* UART6 as debug serial */
	MX7D_PAD_SD1_CD_B__UART6_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_SD1_WP__UART6_DCE_TX   | MUX_PAD_CTRL(UART_PAD_CTRL),
	/* WDOG1 for reset */
	MX7D_PAD_GPIO1_IO00__WDOG1_WDOG_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(meerkat96_pads,
					 ARRAY_SIZE(meerkat96_pads));

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	char *mode;

	if (IS_ENABLED(CONFIG_ARMV7_BOOT_SEC_DEFAULT))
		mode = "secure";
	else
		mode = "non-secure";

	printf("Board: i.MX7D Meerkat96 in %s mode\n", mode);

	return 0;
}

int board_late_init(void)
{
	set_wdog_reset((struct wdog_regs *)WDOG1_BASE_ADDR);

	return 0;
}
