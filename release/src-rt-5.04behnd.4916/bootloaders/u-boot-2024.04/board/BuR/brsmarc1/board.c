// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for B&R BRSMARC1 Board
 *
 * Copyright (C) 2017 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 */
#include <common.h>
#include <errno.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/emif.h>
#include <power/tps65217.h>
#include "../common/bur_common.h"
#include "../common/br_resetc.h"

/* -------------------------------------------------------------------------*/
/* -- defines for used GPIO Hardware -- */
#define PER_RESET		(2 * 32 + 0)

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)
static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

static const struct ctrl_ioregs ddr3_ioregs = {
	.cm0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
};

#define OSC	(V_OSCK / 1000000)
const struct dpll_params dpll_ddr3 = { 400, OSC - 1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
	struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;

	int rc;
	/*
	 * enable additional clocks of modules which are accessed later from
	 * VxWorks OS
	 */
	u32 *const clk_domains[] = { 0 };
	u32 *const clk_modules_specific[] = {
		&cmwkup->wkup_adctscctrl,
		&cmper->spi1clkctrl,
		&cmper->dcan0clkctrl,
		&cmper->dcan1clkctrl,
		&cmper->timer4clkctrl,
		&cmper->timer5clkctrl,
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_specific, 1);

	/* setup I2C */
	enable_i2c_pin_mux();

	/* peripheral reset */
	rc = gpio_request(PER_RESET, "PER_RESET");
	if (rc != 0)
		printf("cannot request PER_RESET GPIO!\n");

	rc = gpio_direction_output(PER_RESET, 0);
	if (rc != 0)
		printf("cannot set PER_RESET GPIO!\n");

	/* setup pmic */
	pmicsetup(0, 0);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr3;
}

void sdram_init(void)
{
	config_ddr(400, &ddr3_ioregs,
		   &ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &ddr3_emif_reg_data, 0);
}
#endif /* CONFIG_SPL_BUILD */
#if !defined(CONFIG_SPL_BUILD)

/* decision if backlight is switched on or not on powerup */
int board_backlightstate(void)
{
	u8 bklmask, rstcause;
	int rc = 0;

	rc |= br_resetc_regget(RSTCTRL_SCRATCHREG1, &bklmask);
	rc |= br_resetc_regget(RSTCTRL_ERSTCAUSE, &rstcause);

	if (rc != 0) {
		printf("%s: read rstctrl failed!\n", __func__);
		return 1;
	}

	if ((rstcause & bklmask) != 0)
		return 0;

	return 1;
}

/* Basic board specific setup. run quite after relocation */
int board_init(void)
{
	if (power_tps65217_init(0))
		printf("WARN: cannot setup PMIC 0x24 @ bus #0, not found!.\n");

	return 0;
}

#if defined(CONFIG_BOARD_LATE_INIT)

int board_late_init(void)
{
	br_resetc_bmode();

	return 0;
}

#endif /* CONFIG_BOARD_LATE_INIT */
#endif /* !CONFIG_SPL_BUILD */
