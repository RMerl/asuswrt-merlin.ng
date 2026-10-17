// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for B&R BRXRE1 Board
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 */
#include <common.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <dm.h>
#include <power/tps65217.h>
#include "../common/bur_common.h"
#include "../common/br_resetc.h"

/* -------------------------------------------------------------------------*/
/* -- defines for used GPIO Hardware -- */
#define ESC_KEY					(0 + 19)
#define LCD_PWR					(0 + 5)

#define	RSTCTRL_FORCE_PWR_NEN			0x04
#define	RSTCTRL_CAN_STB				0x40

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
	int rc;

	struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
	struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
	/*
	 * enable additional clocks of modules which are accessed later from
	 * VxWorks OS
	 */
	u32 *const clk_domains[] = { 0 };

	u32 *const clk_modules_xre1specific[] = {
		&cmwkup->wkup_adctscctrl,
		&cmper->spi1clkctrl,
		&cmper->dcan0clkctrl,
		&cmper->dcan1clkctrl,
		&cmper->epwmss0clkctrl,
		&cmper->epwmss1clkctrl,
		&cmper->epwmss2clkctrl,
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_xre1specific, 1);
	/* power-OFF LCD-Display */
	if (gpio_request(LCD_PWR, "LCD_PWR") != 0)
		printf("cannot request gpio for LCD_PWR!\n");
	else if (gpio_direction_output(LCD_PWR, 0) != 0)
		printf("cannot set direction output on LCD_PWR!\n");

	/* setup I2C */
	enable_i2c_pin_mux();

	/* power-ON 3V3 via Resetcontroller */
	rc = br_resetc_regset(RSTCTRL_CTRLREG,
			      RSTCTRL_FORCE_PWR_NEN | RSTCTRL_CAN_STB);
	if (rc != 0)
		printf("ERROR: cannot write to resetc (turn on PWR_nEN)!\n");

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
/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	/* request common used gpios */
	gpio_request(ESC_KEY, "boot-key");

	if (power_tps65217_init(0))
		printf("WARN: cannot setup PMIC 0x24 @ bus #0, not found!.\n");

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT

int board_boot_key(void)
{
	return gpio_get_value(ESC_KEY);
}

int board_late_init(void)
{
	char othbootargs[128];

	br_resetc_bmode();

	/* setup othbootargs for bootvx-command (vxWorks bootline) */
	snprintf(othbootargs, sizeof(othbootargs),
		 "u=vxWorksFTP pw=vxWorks o=0x%08x;0x%08x;0x%08x;0x%08x",
		 (u32)gd->relocaddr,
		 (u32)env_get_ulong("vx_memtop", 16, gd->relocaddr),
		 (u32)env_get_ulong("vx_romfsbase", 16, 0),
		 (u32)env_get_ulong("vx_romfssize", 16, 0));
	env_set("othbootargs", othbootargs);
	/*
	 * reset VBAR registers to its reset location, VxWorks 6.9.3.2 does
	 * expect that vectors are there, original u-boot moves them to _start
	 */
	__asm__("ldr r0,=0x20000");
	__asm__("mcr p15, 0, r0, c12, c0, 0"); /* Set VBAR */

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */
