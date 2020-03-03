/*
 * DM81xx hwmod data.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 * Copyright (C) 2013 SKTB SKiT, http://www.skitlab.ru/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_data/gpio-omap.h>
#include <linux/platform_data/hsmmc-omap.h>
#include <linux/platform_data/spi-omap2-mcspi.h>
#include <plat/dmtimer.h>

#include "omap_hwmod_common_data.h"
#include "cm81xx.h"
#include "ti81xx.h"
#include "wd_timer.h"

/*
 * DM816X hardware modules integration data
 *
 * Note: This is incomplete and at present, not generated from h/w database.
 */

/*
 * The alwon .clkctrl_offs field is offset from the CM_ALWON, that's
 * TRM 18.7.17 CM_ALWON device register values minus 0x1400.
 */
#define DM816X_DM_ALWON_BASE		0x1400
#define DM816X_CM_ALWON_MCASP0_CLKCTRL	(0x1540 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MCASP1_CLKCTRL	(0x1544 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MCASP2_CLKCTRL	(0x1548 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MCBSP_CLKCTRL	(0x154c - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_UART_0_CLKCTRL	(0x1550 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_UART_1_CLKCTRL	(0x1554 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_UART_2_CLKCTRL	(0x1558 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_GPIO_0_CLKCTRL	(0x155c - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_GPIO_1_CLKCTRL	(0x1560 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_I2C_0_CLKCTRL	(0x1564 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_I2C_1_CLKCTRL	(0x1568 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_1_CLKCTRL	(0x1570 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_2_CLKCTRL	(0x1574 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_3_CLKCTRL	(0x1578 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_4_CLKCTRL	(0x157c - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_5_CLKCTRL	(0x1580 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_6_CLKCTRL	(0x1584 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TIMER_7_CLKCTRL	(0x1588 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_WDTIMER_CLKCTRL	(0x158c - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_SPI_CLKCTRL	(0x1590 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MAILBOX_CLKCTRL	(0x1594 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_SPINBOX_CLKCTRL	(0x1598 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MMUDATA_CLKCTRL	(0x159c - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MMUCFG_CLKCTRL	(0x15a8 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_SDIO_CLKCTRL	(0x15b0 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_OCMC_0_CLKCTRL	(0x15b4 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_OCMC_1_CLKCTRL	(0x15b8 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_CONTRL_CLKCTRL	(0x15c4 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_GPMC_CLKCTRL	(0x15d0 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_ETHERNET_0_CLKCTRL (0x15d4 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_ETHERNET_1_CLKCTRL (0x15d8 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_MPU_CLKCTRL	(0x15dc - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_L3_CLKCTRL	(0x15e4 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_L4HS_CLKCTRL	(0x15e8 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_L4LS_CLKCTRL	(0x15ec - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_RTC_CLKCTRL	(0x15f0 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TPCC_CLKCTRL	(0x15f4 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TPTC0_CLKCTRL	(0x15f8 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TPTC1_CLKCTRL	(0x15fc - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TPTC2_CLKCTRL	(0x1600 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_TPTC3_CLKCTRL	(0x1604 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_SR_0_CLKCTRL	(0x1608 - DM816X_DM_ALWON_BASE)
#define DM816X_CM_ALWON_SR_1_CLKCTRL	(0x160c - DM816X_DM_ALWON_BASE)

/*
 * The default .clkctrl_offs field is offset from CM_DEFAULT, that's
 * TRM 18.7.6 CM_DEFAULT device register values minus 0x500
 */
#define DM816X_CM_DEFAULT_OFFSET	0x500
#define DM816X_CM_DEFAULT_USB_CLKCTRL	(0x558 - DM816X_CM_DEFAULT_OFFSET)

/* L3 Interconnect entries clocked at 125, 250 and 500MHz */
static struct omap_hwmod dm816x_alwon_l3_slow_hwmod = {
	.name		= "alwon_l3_slow",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &l3_hwmod_class,
	.flags		= HWMOD_NO_IDLEST,
};

static struct omap_hwmod dm816x_default_l3_slow_hwmod = {
	.name		= "default_l3_slow",
	.clkdm_name	= "default_l3_slow_clkdm",
	.class		= &l3_hwmod_class,
	.flags		= HWMOD_NO_IDLEST,
};

static struct omap_hwmod dm816x_alwon_l3_med_hwmod = {
	.name		= "l3_med",
	.clkdm_name	= "alwon_l3_med_clkdm",
	.class		= &l3_hwmod_class,
	.flags		= HWMOD_NO_IDLEST,
};

static struct omap_hwmod dm816x_alwon_l3_fast_hwmod = {
	.name		= "l3_fast",
	.clkdm_name	= "alwon_l3_fast_clkdm",
	.class		= &l3_hwmod_class,
	.flags		= HWMOD_NO_IDLEST,
};

/*
 * L4 standard peripherals, see TRM table 1-12 for devices using this.
 * See TRM table 1-73 for devices using the 125MHz SYSCLK6 clock.
 */
static struct omap_hwmod dm816x_l4_ls_hwmod = {
	.name		= "l4_ls",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &l4_hwmod_class,
};

/*
 * L4 high-speed peripherals. For devices using this, please see the TRM
 * table 1-13. On dm816x, only EMAC, MDIO and SATA use this. See also TRM
 * table 1-73 for devices using 250MHz SYSCLK5 clock.
 */
static struct omap_hwmod dm816x_l4_hs_hwmod = {
	.name		= "l4_hs",
	.clkdm_name	= "alwon_l3_med_clkdm",
	.class		= &l4_hwmod_class,
};

/* L3 slow -> L4 ls peripheral interface running at 125MHz */
static struct omap_hwmod_ocp_if dm816x_alwon_l3_slow__l4_ls = {
	.master	= &dm816x_alwon_l3_slow_hwmod,
	.slave	= &dm816x_l4_ls_hwmod,
	.user	= OCP_USER_MPU,
};

/* L3 med -> L4 fast peripheral interface running at 250MHz */
static struct omap_hwmod_ocp_if dm816x_alwon_l3_slow__l4_hs = {
	.master	= &dm816x_alwon_l3_med_hwmod,
	.slave	= &dm816x_l4_hs_hwmod,
	.user	= OCP_USER_MPU,
};

/* MPU */
static struct omap_hwmod dm816x_mpu_hwmod = {
	.name		= "mpu",
	.clkdm_name	= "alwon_mpu_clkdm",
	.class		= &mpu_hwmod_class,
	.flags		= HWMOD_INIT_NO_IDLE,
	.main_clk	= "mpu_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_MPU_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
};

static struct omap_hwmod_ocp_if dm816x_mpu__alwon_l3_slow = {
	.master		= &dm816x_mpu_hwmod,
	.slave		= &dm816x_alwon_l3_slow_hwmod,
	.user		= OCP_USER_MPU,
};

/* L3 med peripheral interface running at 250MHz */
static struct omap_hwmod_ocp_if dm816x_mpu__alwon_l3_med = {
	.master	= &dm816x_mpu_hwmod,
	.slave	= &dm816x_alwon_l3_med_hwmod,
	.user	= OCP_USER_MPU,
};

/* UART common */
static struct omap_hwmod_class_sysconfig uart_sysc = {
	.rev_offs	= 0x50,
	.sysc_offs	= 0x54,
	.syss_offs	= 0x58,
	.sysc_flags	= SYSC_HAS_ENAWAKEUP | SYSC_HAS_SIDLEMODE |
				SYSC_HAS_SOFTRESET | SYSC_HAS_AUTOIDLE |
				SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART |
				MSTANDBY_SMART_WKUP,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class uart_class = {
	.name = "uart",
	.sysc = &uart_sysc,
};

static struct omap_hwmod dm816x_uart1_hwmod = {
	.name		= "uart1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_UART_0_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &uart_class,
	.flags		= DEBUG_TI81XXUART1_FLAGS,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__uart1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_uart1_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_uart2_hwmod = {
	.name		= "uart2",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_UART_1_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &uart_class,
	.flags		= DEBUG_TI81XXUART2_FLAGS,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__uart2 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_uart2_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_uart3_hwmod = {
	.name		= "uart3",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_UART_2_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &uart_class,
	.flags		= DEBUG_TI81XXUART3_FLAGS,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__uart3 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_uart3_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig wd_timer_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x10,
	.syss_offs	= 0x14,
	.sysc_flags	= SYSC_HAS_EMUFREE | SYSC_HAS_SOFTRESET |
				SYSS_HAS_RESET_STATUS,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class wd_timer_class = {
	.name		= "wd_timer",
	.sysc		= &wd_timer_sysc,
	.pre_shutdown	= &omap2_wd_timer_disable,
	.reset		= &omap2_wd_timer_reset,
};

static struct omap_hwmod dm816x_wd_timer_hwmod = {
	.name		= "wd_timer",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk18_ck",
	.flags		= HWMOD_NO_IDLEST,
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_WDTIMER_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &wd_timer_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__wd_timer1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_wd_timer_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

/* I2C common */
static struct omap_hwmod_class_sysconfig i2c_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x10,
	.syss_offs	= 0x90,
	.sysc_flags	= SYSC_HAS_SIDLEMODE |
				SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
				SYSC_HAS_AUTOIDLE,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class i2c_class = {
	.name = "i2c",
	.sysc = &i2c_sysc,
};

static struct omap_hwmod dm81xx_i2c1_hwmod = {
	.name		= "i2c1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_I2C_0_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &i2c_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__i2c1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm81xx_i2c1_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_i2c2_hwmod = {
	.name		= "i2c2",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_I2C_1_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &i2c_class,
};

static struct omap_hwmod_class_sysconfig dm81xx_elm_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0014,
	.sysc_flags	= SYSC_HAS_CLOCKACTIVITY | SYSC_HAS_SIDLEMODE |
				SYSC_HAS_SOFTRESET |
				SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__i2c2 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_i2c2_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class dm81xx_elm_hwmod_class = {
	.name = "elm",
	.sysc = &dm81xx_elm_sysc,
};

static struct omap_hwmod dm81xx_elm_hwmod = {
	.name		= "elm",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &dm81xx_elm_hwmod_class,
	.main_clk	= "sysclk6_ck",
};

static struct omap_hwmod_ocp_if dm81xx_l4_ls__elm = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm81xx_elm_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm81xx_gpio_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0114,
	.sysc_flags	= SYSC_HAS_AUTOIDLE | SYSC_HAS_ENAWAKEUP |
				SYSC_HAS_SIDLEMODE | SYSC_HAS_SOFTRESET |
				SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART |
				SIDLE_SMART_WKUP,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class dm81xx_gpio_hwmod_class = {
	.name	= "gpio",
	.sysc	= &dm81xx_gpio_sysc,
	.rev	= 2,
};

static struct omap_gpio_dev_attr gpio_dev_attr = {
	.bank_width	= 32,
	.dbck_flag	= true,
};

static struct omap_hwmod_opt_clk gpio1_opt_clks[] = {
	{ .role = "dbclk", .clk = "sysclk18_ck" },
};

static struct omap_hwmod dm81xx_gpio1_hwmod = {
	.name		= "gpio1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &dm81xx_gpio_hwmod_class,
	.main_clk	= "sysclk6_ck",
	.prcm = {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_GPIO_0_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.opt_clks	= gpio1_opt_clks,
	.opt_clks_cnt	= ARRAY_SIZE(gpio1_opt_clks),
	.dev_attr	= &gpio_dev_attr,
};

static struct omap_hwmod_ocp_if dm81xx_l4_ls__gpio1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm81xx_gpio1_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_opt_clk gpio2_opt_clks[] = {
	{ .role = "dbclk", .clk = "sysclk18_ck" },
};

static struct omap_hwmod dm81xx_gpio2_hwmod = {
	.name		= "gpio2",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &dm81xx_gpio_hwmod_class,
	.main_clk	= "sysclk6_ck",
	.prcm = {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_GPIO_1_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.opt_clks	= gpio2_opt_clks,
	.opt_clks_cnt	= ARRAY_SIZE(gpio2_opt_clks),
	.dev_attr	= &gpio_dev_attr,
};

static struct omap_hwmod_ocp_if dm81xx_l4_ls__gpio2 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm81xx_gpio2_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm81xx_gpmc_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x10,
	.syss_offs	= 0x14,
	.sysc_flags	= SYSC_HAS_SIDLEMODE | SYSC_HAS_SOFTRESET |
				SYSC_HAS_AUTOIDLE | SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class dm81xx_gpmc_hwmod_class = {
	.name	= "gpmc",
	.sysc	= &dm81xx_gpmc_sysc,
};

static struct omap_hwmod dm81xx_gpmc_hwmod = {
	.name		= "gpmc",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &dm81xx_gpmc_hwmod_class,
	.main_clk	= "sysclk6_ck",
	.prcm = {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_GPMC_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm81xx_alwon_l3_slow__gpmc = {
	.master		= &dm816x_alwon_l3_slow_hwmod,
	.slave		= &dm81xx_gpmc_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm81xx_usbhsotg_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x10,
	.sysc_flags	= SYSC_HAS_SIDLEMODE | SYSC_HAS_MIDLEMODE |
				SYSC_HAS_SOFTRESET,
	.idlemodes	= SIDLE_SMART | MSTANDBY_FORCE | MSTANDBY_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type2,
};

static struct omap_hwmod_class dm81xx_usbotg_class = {
	.name = "usbotg",
	.sysc = &dm81xx_usbhsotg_sysc,
};

static struct omap_hwmod dm81xx_usbss_hwmod = {
	.name		= "usb_otg_hs",
	.clkdm_name	= "default_l3_slow_clkdm",
	.main_clk	= "sysclk6_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_DEFAULT_USB_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &dm81xx_usbotg_class,
};

static struct omap_hwmod_ocp_if dm81xx_default_l3_slow__usbss = {
	.master		= &dm816x_default_l3_slow_hwmod,
	.slave		= &dm81xx_usbss_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm816x_timer_sysc = {
	.rev_offs	= 0x0000,
	.sysc_offs	= 0x0010,
	.syss_offs	= 0x0014,
	.sysc_flags	= SYSC_HAS_SIDLEMODE | SYSC_HAS_SOFTRESET,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART |
				SIDLE_SMART_WKUP,
	.sysc_fields	= &omap_hwmod_sysc_type2,
};

static struct omap_hwmod_class dm816x_timer_hwmod_class = {
	.name = "timer",
	.sysc = &dm816x_timer_sysc,
};

static struct omap_timer_capability_dev_attr capability_alwon_dev_attr = {
	.timer_capability	= OMAP_TIMER_ALWON,
};

static struct omap_hwmod dm816x_timer1_hwmod = {
	.name		= "timer1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer1_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_1_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer1_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer2_hwmod = {
	.name		= "timer2",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer2_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_2_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer2 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer2_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer3_hwmod = {
	.name		= "timer3",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer3_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_3_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer3 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer3_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer4_hwmod = {
	.name		= "timer4",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer4_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_4_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer4 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer4_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer5_hwmod = {
	.name		= "timer5",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer5_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_5_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer5 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer5_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer6_hwmod = {
	.name		= "timer6",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer6_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_6_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer6 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer6_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_timer7_hwmod = {
	.name		= "timer7",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "timer7_fck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_TIMER_7_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &capability_alwon_dev_attr,
	.class		= &dm816x_timer_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__timer7 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_timer7_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

/* EMAC Ethernet */
static struct omap_hwmod_class_sysconfig dm816x_emac_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x4,
	.sysc_flags	= SYSC_HAS_SOFTRESET,
	.sysc_fields	= &omap_hwmod_sysc_type2,
};

static struct omap_hwmod_class dm816x_emac_hwmod_class = {
	.name		= "emac",
	.sysc		= &dm816x_emac_sysc,
};

/*
 * On dm816x the MDIO is within EMAC0. As the MDIO driver is a separate
 * driver probed before EMAC0, we let MDIO do the clock idling.
 */
static struct omap_hwmod dm816x_emac0_hwmod = {
	.name		= "emac0",
	.clkdm_name	= "alwon_ethernet_clkdm",
	.class		= &dm816x_emac_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_hs__emac0 = {
	.master		= &dm816x_l4_hs_hwmod,
	.slave		= &dm816x_emac0_hwmod,
	.clk		= "sysclk5_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class dm816x_mdio_hwmod_class = {
	.name		= "davinci_mdio",
	.sysc		= &dm816x_emac_sysc,
};

struct omap_hwmod dm816x_emac0_mdio_hwmod = {
	.name		= "davinci_mdio",
	.class		= &dm816x_mdio_hwmod_class,
	.clkdm_name	= "alwon_ethernet_clkdm",
	.main_clk	= "sysclk24_ck",
	.flags		= HWMOD_NO_IDLEST,
	/*
	 * REVISIT: This should be moved to the emac0_hwmod
	 * once we have a better way to handle device slaves.
	 */
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_ETHERNET_0_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_emac0__mdio = {
	.master		= &dm816x_l4_hs_hwmod,
	.slave		= &dm816x_emac0_mdio_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod dm816x_emac1_hwmod = {
	.name		= "emac1",
	.clkdm_name	= "alwon_ethernet_clkdm",
	.main_clk	= "sysclk24_ck",
	.flags		= HWMOD_NO_IDLEST,
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_ETHERNET_1_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &dm816x_emac_hwmod_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_hs__emac1 = {
	.master		= &dm816x_l4_hs_hwmod,
	.slave		= &dm816x_emac1_hwmod,
	.clk		= "sysclk5_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm816x_mmc_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x110,
	.syss_offs	= 0x114,
	.sysc_flags	= SYSC_HAS_CLOCKACTIVITY | SYSC_HAS_SIDLEMODE |
				SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
				SYSC_HAS_AUTOIDLE | SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class dm816x_mmc_class = {
	.name = "mmc",
	.sysc = &dm816x_mmc_sysc,
};

static struct omap_hwmod_opt_clk dm816x_mmc1_opt_clks[] = {
	{ .role = "dbck", .clk = "sysclk18_ck", },
};

static struct omap_hsmmc_dev_attr mmc1_dev_attr = {
	.flags = OMAP_HSMMC_SUPPORTS_DUAL_VOLT,
};

static struct omap_hwmod dm816x_mmc1_hwmod = {
	.name		= "mmc1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.opt_clks	= dm816x_mmc1_opt_clks,
	.opt_clks_cnt	= ARRAY_SIZE(dm816x_mmc1_opt_clks),
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_SDIO_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.dev_attr	= &mmc1_dev_attr,
	.class		= &dm816x_mmc_class,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__mmc1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_mmc1_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
	.flags		= OMAP_FIREWALL_L4
};

static struct omap_hwmod_class_sysconfig dm816x_mcspi_sysc = {
	.rev_offs	= 0x0,
	.sysc_offs	= 0x110,
	.syss_offs	= 0x114,
	.sysc_flags	= SYSC_HAS_CLOCKACTIVITY | SYSC_HAS_SIDLEMODE |
				SYSC_HAS_ENAWAKEUP | SYSC_HAS_SOFTRESET |
				SYSC_HAS_AUTOIDLE | SYSS_HAS_RESET_STATUS,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class dm816x_mcspi_class = {
	.name = "mcspi",
	.sysc = &dm816x_mcspi_sysc,
	.rev = OMAP3_MCSPI_REV,
};

static struct omap2_mcspi_dev_attr dm816x_mcspi1_dev_attr = {
	.num_chipselect = 4,
};

static struct omap_hwmod dm816x_mcspi1_hwmod = {
	.name		= "mcspi1",
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk10_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_SPI_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
	.class		= &dm816x_mcspi_class,
	.dev_attr	= &dm816x_mcspi1_dev_attr,
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__mcspi1 = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_mcspi1_hwmod,
	.clk		= "sysclk6_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class_sysconfig dm816x_mailbox_sysc = {
	.rev_offs	= 0x000,
	.sysc_offs	= 0x010,
	.syss_offs	= 0x014,
	.sysc_flags	= SYSC_HAS_CLOCKACTIVITY | SYSC_HAS_SIDLEMODE |
				SYSC_HAS_SOFTRESET | SYSC_HAS_AUTOIDLE,
	.idlemodes	= SIDLE_FORCE | SIDLE_NO | SIDLE_SMART,
	.sysc_fields	= &omap_hwmod_sysc_type1,
};

static struct omap_hwmod_class dm816x_mailbox_hwmod_class = {
	.name = "mailbox",
	.sysc = &dm816x_mailbox_sysc,
};

static struct omap_hwmod dm816x_mailbox_hwmod = {
	.name		= "mailbox",
	.clkdm_name	= "alwon_l3s_clkdm",
	.class		= &dm816x_mailbox_hwmod_class,
	.main_clk	= "sysclk6_ck",
	.prcm		= {
		.omap4 = {
			.clkctrl_offs = DM816X_CM_ALWON_MAILBOX_CLKCTRL,
			.modulemode = MODULEMODE_SWCTRL,
		},
	},
};

static struct omap_hwmod_ocp_if dm816x_l4_ls__mailbox = {
	.master		= &dm816x_l4_ls_hwmod,
	.slave		= &dm816x_mailbox_hwmod,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_class dm816x_tpcc_hwmod_class = {
	.name		= "tpcc",
};

struct omap_hwmod dm816x_tpcc_hwmod = {
	.name		= "tpcc",
	.class		= &dm816x_tpcc_hwmod_class,
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk4_ck",
	.prcm		= {
		.omap4	= {
			.clkctrl_offs	= DM816X_CM_ALWON_TPCC_CLKCTRL,
			.modulemode	= MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_alwon_l3_fast__tpcc = {
	.master		= &dm816x_alwon_l3_fast_hwmod,
	.slave		= &dm816x_tpcc_hwmod,
	.clk		= "sysclk4_ck",
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_addr_space dm816x_tptc0_addr_space[] = {
	{
		.pa_start	= 0x49800000,
		.pa_end		= 0x49800000 + SZ_8K - 1,
		.flags		= ADDR_TYPE_RT,
	},
	{ },
};

static struct omap_hwmod_class dm816x_tptc0_hwmod_class = {
	.name		= "tptc0",
};

struct omap_hwmod dm816x_tptc0_hwmod = {
	.name		= "tptc0",
	.class		= &dm816x_tptc0_hwmod_class,
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk4_ck",
	.prcm		= {
		.omap4	= {
			.clkctrl_offs	= DM816X_CM_ALWON_TPTC0_CLKCTRL,
			.modulemode	= MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_alwon_l3_fast__tptc0 = {
	.master		= &dm816x_alwon_l3_fast_hwmod,
	.slave		= &dm816x_tptc0_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc0_addr_space,
	.user		= OCP_USER_MPU,
};

struct omap_hwmod_ocp_if dm816x_tptc0__alwon_l3_fast = {
	.master		= &dm816x_tptc0_hwmod,
	.slave		= &dm816x_alwon_l3_fast_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc0_addr_space,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_addr_space dm816x_tptc1_addr_space[] = {
	{
		.pa_start	= 0x49900000,
		.pa_end		= 0x49900000 + SZ_8K - 1,
		.flags		= ADDR_TYPE_RT,
	},
	{ },
};

static struct omap_hwmod_class dm816x_tptc1_hwmod_class = {
	.name		= "tptc1",
};

struct omap_hwmod dm816x_tptc1_hwmod = {
	.name		= "tptc1",
	.class		= &dm816x_tptc1_hwmod_class,
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk4_ck",
	.prcm		= {
		.omap4	= {
			.clkctrl_offs	= DM816X_CM_ALWON_TPTC1_CLKCTRL,
			.modulemode	= MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_alwon_l3_fast__tptc1 = {
	.master		= &dm816x_alwon_l3_fast_hwmod,
	.slave		= &dm816x_tptc1_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc1_addr_space,
	.user		= OCP_USER_MPU,
};

struct omap_hwmod_ocp_if dm816x_tptc1__alwon_l3_fast = {
	.master		= &dm816x_tptc1_hwmod,
	.slave		= &dm816x_alwon_l3_fast_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc1_addr_space,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_addr_space dm816x_tptc2_addr_space[] = {
	{
		.pa_start	= 0x49a00000,
		.pa_end		= 0x49a00000 + SZ_8K - 1,
		.flags		= ADDR_TYPE_RT,
	},
	{ },
};

static struct omap_hwmod_class dm816x_tptc2_hwmod_class = {
	.name		= "tptc2",
};

struct omap_hwmod dm816x_tptc2_hwmod = {
	.name		= "tptc2",
	.class		= &dm816x_tptc2_hwmod_class,
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk4_ck",
	.prcm		= {
		.omap4	= {
			.clkctrl_offs	= DM816X_CM_ALWON_TPTC2_CLKCTRL,
			.modulemode	= MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_alwon_l3_fast__tptc2 = {
	.master		= &dm816x_alwon_l3_fast_hwmod,
	.slave		= &dm816x_tptc2_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc2_addr_space,
	.user		= OCP_USER_MPU,
};

struct omap_hwmod_ocp_if dm816x_tptc2__alwon_l3_fast = {
	.master		= &dm816x_tptc2_hwmod,
	.slave		= &dm816x_alwon_l3_fast_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc2_addr_space,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_addr_space dm816x_tptc3_addr_space[] = {
	{
		.pa_start	= 0x49b00000,
		.pa_end		= 0x49b00000 + SZ_8K - 1,
		.flags		= ADDR_TYPE_RT,
	},
	{ },
};

static struct omap_hwmod_class dm816x_tptc3_hwmod_class = {
	.name		= "tptc3",
};

struct omap_hwmod dm816x_tptc3_hwmod = {
	.name		= "tptc3",
	.class		= &dm816x_tptc3_hwmod_class,
	.clkdm_name	= "alwon_l3s_clkdm",
	.main_clk	= "sysclk4_ck",
	.prcm		= {
		.omap4	= {
			.clkctrl_offs	= DM816X_CM_ALWON_TPTC3_CLKCTRL,
			.modulemode	= MODULEMODE_SWCTRL,
		},
	},
};

struct omap_hwmod_ocp_if dm816x_alwon_l3_fast__tptc3 = {
	.master		= &dm816x_alwon_l3_fast_hwmod,
	.slave		= &dm816x_tptc3_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc3_addr_space,
	.user		= OCP_USER_MPU,
};

struct omap_hwmod_ocp_if dm816x_tptc3__alwon_l3_fast = {
	.master		= &dm816x_tptc3_hwmod,
	.slave		= &dm816x_alwon_l3_fast_hwmod,
	.clk		= "sysclk4_ck",
	.addr		= dm816x_tptc3_addr_space,
	.user		= OCP_USER_MPU,
};

static struct omap_hwmod_ocp_if *dm816x_hwmod_ocp_ifs[] __initdata = {
	&dm816x_mpu__alwon_l3_slow,
	&dm816x_mpu__alwon_l3_med,
	&dm816x_alwon_l3_slow__l4_ls,
	&dm816x_alwon_l3_slow__l4_hs,
	&dm816x_l4_ls__uart1,
	&dm816x_l4_ls__uart2,
	&dm816x_l4_ls__uart3,
	&dm816x_l4_ls__wd_timer1,
	&dm816x_l4_ls__i2c1,
	&dm816x_l4_ls__i2c2,
	&dm81xx_l4_ls__gpio1,
	&dm81xx_l4_ls__gpio2,
	&dm81xx_l4_ls__elm,
	&dm816x_l4_ls__mmc1,
	&dm816x_l4_ls__timer1,
	&dm816x_l4_ls__timer2,
	&dm816x_l4_ls__timer3,
	&dm816x_l4_ls__timer4,
	&dm816x_l4_ls__timer5,
	&dm816x_l4_ls__timer6,
	&dm816x_l4_ls__timer7,
	&dm816x_l4_ls__mcspi1,
	&dm816x_l4_ls__mailbox,
	&dm816x_l4_hs__emac0,
	&dm816x_emac0__mdio,
	&dm816x_l4_hs__emac1,
	&dm816x_alwon_l3_fast__tpcc,
	&dm816x_alwon_l3_fast__tptc0,
	&dm816x_alwon_l3_fast__tptc1,
	&dm816x_alwon_l3_fast__tptc2,
	&dm816x_alwon_l3_fast__tptc3,
	&dm816x_tptc0__alwon_l3_fast,
	&dm816x_tptc1__alwon_l3_fast,
	&dm816x_tptc2__alwon_l3_fast,
	&dm816x_tptc3__alwon_l3_fast,
	&dm81xx_alwon_l3_slow__gpmc,
	&dm81xx_default_l3_slow__usbss,
	NULL,
};

int __init ti81xx_hwmod_init(void)
{
	omap_hwmod_init();
	return omap_hwmod_register_links(dm816x_hwmod_ocp_ifs);
}
