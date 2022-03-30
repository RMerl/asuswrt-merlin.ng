// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for B&R BRPPT1
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <power/tps65217.h>
#include "../common/bur_common.h"
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------------------*/
/* -- defines for GPIO -- */
#define	REPSWITCH	(0+20)	/* GPIO0_20 */

#if defined(CONFIG_SPL_BUILD)
/* TODO: check ram-timing ! */
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

#define OSC	(V_OSCK/1000000)
static const struct dpll_params dpll_ddr3 = { 400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	int rc;

	struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
	/*struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;*/
	struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;

	/*
	 * in TRM they write a reset value of 1 (=CLK_M_OSC) for the
	 * CLKSEL_TIMER6_CLK Register, in fact reset value is 0, so we need set
	 * the source of timer6 clk to CLK_M_OSC
	 */
	writel(0x01, &cmdpll->clktimer6clk);

	/* enable additional clocks of modules which are accessed later */
	u32 *const clk_domains[] = {
		&cmper->lcdcclkstctrl,
		0
	};

	u32 *const clk_modules_tsspecific[] = {
		&cmper->lcdclkctrl,
		&cmper->timer5clkctrl,
		&cmper->timer6clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_tsspecific, 1);

	/* setup I2C */
	enable_i2c_pin_mux();

	pmicsetup(0, 0);

	/* peripheral reset */
	rc = gpio_request(64 + 29, "GPMC_WAIT1");
	if (rc != 0)
		printf("cannot request GPMC_WAIT1 GPIO!\n");
	rc = gpio_direction_output(64 + 29, 1);
	if (rc != 0)
		printf("cannot set GPMC_WAIT1 GPIO!\n");

	rc = gpio_request(64 + 28, "GPMC_WAIT0");
	if (rc != 0)
		printf("cannot request GPMC_WAIT0 GPIO!\n");
	rc = gpio_direction_output(64 + 28, 1);
	if (rc != 0)
		printf("cannot set GPMC_WAIT0 GPIO!\n");

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

/* Basic board specific setup.  Pinmux has been handled already. */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#ifdef CONFIG_NAND
	gpmc_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
static char *bootmodeascii[16] = {
	"BOOT",		"reserved",	"reserved",	"reserved",
	"RUN",		"reserved",	"reserved",	"reserved",
	"reserved",	"reserved",	"reserved",	"reserved",
	"PME",		"reserved",	"reserved",	"DIAG",
};

int board_late_init(void)
{
	unsigned char bmode = 0;
	ulong bootcount = 0;
	int rc;

	bootcount = bootcount_load() & 0xF;

	rc = gpio_request(REPSWITCH, "REPSWITCH");

	if (rc != 0 || gpio_get_value(REPSWITCH) == 0 || bootcount == 12)
		bmode = 12;
	else if (bootcount > 0)
		bmode = 0;
	else
		bmode = 4;

	printf("Mode:  %s\n", bootmodeascii[bmode & 0x0F]);
	env_set_ulong("b_mode", bmode);

	/* get sure that bootcmd isn't affected by any bootcount value */
	env_set_ulong("bootlimit", 0);

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */
