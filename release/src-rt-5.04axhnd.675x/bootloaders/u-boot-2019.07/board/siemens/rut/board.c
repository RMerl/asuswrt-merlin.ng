// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for TI AM335X based rut board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/board.c
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <errno.h>
#include <spi.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <video.h>
#include <watchdog.h>
#include "board.h"
#include "../common/factoryset.h"
#include "../../../drivers/video/da8xx-fb.h"

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(void)
{
	return 0;
}

#ifdef CONFIG_SPL_BUILD
static void board_init_ddr(void)
{
struct emif_regs rut_ddr3_emif_reg_data = {
	.sdram_config = 0x61C04AB2,
	.sdram_tim1 = 0x0888A39B,
	.sdram_tim2 = 0x26337FDA,
	.sdram_tim3 = 0x501F830F,
	.emif_ddr_phy_ctlr_1 = 0x6,
	.zq_config = 0x50074BE4,
	.ref_ctrl = 0x93B,
};

struct ddr_data rut_ddr3_data = {
	.datardsratio0 = 0x3b,
	.datawdsratio0 = 0x85,
	.datafwsratio0 = 0x100,
	.datawrsratio0 = 0xc1,
};

struct cmd_control rut_ddr3_cmd_ctrl_data = {
	.cmd0csratio = 0x40,
	.cmd0iclkout = 1,
	.cmd1csratio = 0x40,
	.cmd1iclkout = 1,
	.cmd2csratio = 0x40,
	.cmd2iclkout = 1,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= RUT_IOCTRL_VAL,
	.cm1ioctl		= RUT_IOCTRL_VAL,
	.cm2ioctl		= RUT_IOCTRL_VAL,
	.dt0ioctl		= RUT_IOCTRL_VAL,
	.dt1ioctl		= RUT_IOCTRL_VAL,
};

	config_ddr(DDR_PLL_FREQ, &ioregs, &rut_ddr3_data,
		   &rut_ddr3_cmd_ctrl_data, &rut_ddr3_emif_reg_data, 0);
}

static int request_and_pulse_reset(int gpio, const char *name)
{
	int ret;
	const int delay_us = 2000; /* 2ms */

	ret = gpio_request(gpio, name);
	if (ret < 0) {
		printf("%s: Unable to request %s\n", __func__, name);
		goto err;
	}

	ret = gpio_direction_output(gpio, 0);
	if (ret < 0) {
		printf("%s: Unable to set %s  as output\n", __func__, name);
		goto err_free_gpio;
	}

	udelay(delay_us);

	gpio_set_value(gpio, 1);

	return 0;

err_free_gpio:
	gpio_free(gpio);
err:
	return ret;
}

#define GPIO_TO_PIN(bank, gpio)		(32 * (bank) + (gpio))
#define ETH_PHY_RESET_GPIO		GPIO_TO_PIN(2, 18)
#define MAXTOUCH_RESET_GPIO		GPIO_TO_PIN(3, 18)
#define DISPLAY_RESET_GPIO		GPIO_TO_PIN(3, 19)

#define REQUEST_AND_PULSE_RESET(N) \
		request_and_pulse_reset(N, #N);

static void spl_siemens_board_init(void)
{
	REQUEST_AND_PULSE_RESET(ETH_PHY_RESET_GPIO);
	REQUEST_AND_PULSE_RESET(MAXTOUCH_RESET_GPIO);
	REQUEST_AND_PULSE_RESET(DISPLAY_RESET_GPIO);
}
#endif /* if def CONFIG_SPL_BUILD */

#if defined(CONFIG_DRIVER_TI_CPSW)
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
	int n = 0;
	int rv;

#ifndef CONFIG_SPL_BUILD
	factoryset_env_set();
#endif

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel((RMII_MODE_ENABLE | RMII_CHIPCKL_ENABLE), &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
	return n;
}
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#if defined(CONFIG_HW_WATCHDOG)
static bool hw_watchdog_init_done;
static int  hw_watchdog_trigger_level;

void hw_watchdog_reset(void)
{
	if (!hw_watchdog_init_done)
		return;

	hw_watchdog_trigger_level = hw_watchdog_trigger_level ? 0 : 1;
	gpio_set_value(WATCHDOG_TRIGGER_GPIO, hw_watchdog_trigger_level);
}

void hw_watchdog_init(void)
{
	gpio_request(WATCHDOG_TRIGGER_GPIO, "watchdog_trigger");
	gpio_direction_output(WATCHDOG_TRIGGER_GPIO, hw_watchdog_trigger_level);

	hw_watchdog_reset();

	hw_watchdog_init_done = 1;
}
#endif /* defined(CONFIG_HW_WATCHDOG) */

#if defined(CONFIG_VIDEO) && !defined(CONFIG_SPL_BUILD)
static struct da8xx_panel lcd_panels[] = {
	/* FORMIKE, 4.3", 480x800, KWH043MC17-F01 */
	[0] = {
		.name   = "KWH043MC17-F01",
		.width  = 480,
		.height = 800,
		.hfp = 50,              /* no spec, "don't care" values */
		.hbp = 50,
		.hsw = 50,
		.vfp = 50,
		.vbp = 50,
		.vsw = 50,
		.pxl_clk = 35910000,    /* tCYCD=20ns, max 50MHz, 60fps */
		.invert_pxl_clk = 1,
	},
	/* FORMIKE, 4.3", 480x800, KWH043ST20-F01 */
	[1] = {
		.name   = "KWH043ST20-F01",
		.width  = 480,
		.height = 800,
		.hfp = 50,              /* no spec, "don't care" values */
		.hbp = 50,
		.hsw = 50,
		.vfp = 50,
		.vbp = 50,
		.vsw = 50,
		.pxl_clk = 35910000,    /* tCYCD=20ns, max 50MHz, 60fps */
		.invert_pxl_clk = 1,
	},
	/* Multi-Inno, 4.3", 480x800, MI0430VT-1 */
	[2] = {
		.name   = "MI0430VT-1",
		.width  = 480,
		.height = 800,
		.hfp = 50,              /* no spec, "don't care" values */
		.hbp = 50,
		.hsw = 50,
		.vfp = 50,
		.vbp = 50,
		.vsw = 50,
		.pxl_clk = 35910000,    /* tCYCD=20ns, max 50MHz, 60fps */
		.invert_pxl_clk = 1,
	},
};

static const struct display_panel disp_panels[] = {
	[0] = {
		WVGA,
		16,	/* RGB 888 */
		16,
		COLOR_ACTIVE,
	},
	[1] = {
		WVGA,
		16,	/* RGB 888 */
		16,
		COLOR_ACTIVE,
	},
	[2] = {
		WVGA,
		24,	/* RGB 888 */
		16,
		COLOR_ACTIVE,
	},
};

static const struct lcd_ctrl_config lcd_cfgs[] = {
	[0] = {
		&disp_panels[0],
		.ac_bias		= 255,
		.ac_bias_intrpt		= 0,
		.dma_burst_sz		= 16,
		.bpp			= 16,
		.fdd			= 0x80,
		.tft_alt_mode		= 0,
		.stn_565_mode		= 0,
		.mono_8bit_mode		= 0,
		.invert_line_clock	= 1,
		.invert_frm_clock	= 1,
		.sync_edge		= 0,
		.sync_ctrl		= 1,
		.raster_order		= 0,
	},
	[1] = {
		&disp_panels[1],
		.ac_bias		= 255,
		.ac_bias_intrpt		= 0,
		.dma_burst_sz		= 16,
		.bpp			= 16,
		.fdd			= 0x80,
		.tft_alt_mode		= 0,
		.stn_565_mode		= 0,
		.mono_8bit_mode		= 0,
		.invert_line_clock	= 1,
		.invert_frm_clock	= 1,
		.sync_edge		= 0,
		.sync_ctrl		= 1,
		.raster_order		= 0,
	},
	[2] = {
		&disp_panels[2],
		.ac_bias		= 255,
		.ac_bias_intrpt		= 0,
		.dma_burst_sz		= 16,
		.bpp			= 24,
		.fdd			= 0x80,
		.tft_alt_mode		= 0,
		.stn_565_mode		= 0,
		.mono_8bit_mode		= 0,
		.invert_line_clock	= 1,
		.invert_frm_clock	= 1,
		.sync_edge		= 0,
		.sync_ctrl		= 1,
		.raster_order		= 0,
	},

};

/* no console on this board */
int board_cfb_skip(void)
{
	return 1;
}

#define PLL_GET_M(v) ((v >> 8) & 0x7ff)
#define PLL_GET_N(v) (v & 0x7f)

static struct dpll_regs dpll_lcd_regs = {
	.cm_clkmode_dpll = CM_WKUP + 0x98,
	.cm_idlest_dpll = CM_WKUP + 0x48,
	.cm_clksel_dpll = CM_WKUP + 0x54,
};

static int get_clk(struct dpll_regs *dpll_regs)
{
	unsigned int val;
	unsigned int m, n;
	int f = 0;

	val = readl(dpll_regs->cm_clksel_dpll);
	m = PLL_GET_M(val);
	n = PLL_GET_N(val);
	f = (m * V_OSCK) / n;

	return f;
};

int clk_get(int clk)
{
	return get_clk(&dpll_lcd_regs);
};

static int conf_disp_pll(int m, int n)
{
	struct cm_perpll *cmper = (struct cm_perpll *)CM_PER;
	struct dpll_params dpll_lcd = {m, n, -1, -1, -1, -1, -1};
#if defined(DISPL_PLL_SPREAD_SPECTRUM)
	struct cm_wkuppll *cmwkup = (struct cm_wkuppll *)CM_WKUP;
#endif

	u32 *const clk_domains[] = {
		&cmper->lcdclkctrl,
		0
	};
	u32 *const clk_modules_explicit_en[] = {
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		&cmper->spi1clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);

	do_setup_dpll(&dpll_lcd_regs, &dpll_lcd);

#if defined(DISPL_PLL_SPREAD_SPECTRUM)
	writel(0x64, &cmwkup->resv6[3]); /* 0x50 */
	writel(0x800, &cmwkup->resv6[2]); /* 0x4c */
	writel(readl(&cmwkup->clkmoddplldisp) | CM_CLKMODE_DPLL_SSC_EN_MASK,
	       &cmwkup->clkmoddplldisp); /* 0x98 */
#endif
	return 0;
}

static int set_gpio(int gpio, int state)
{
	gpio_request(gpio, "temp");
	gpio_direction_output(gpio, state);
	gpio_set_value(gpio, state);
	gpio_free(gpio);
	return 0;
}

static int enable_lcd(void)
{
	unsigned char buf[1];

	set_gpio(BOARD_LCD_RESET, 0);
	mdelay(1);
	set_gpio(BOARD_LCD_RESET, 1);
	mdelay(1);

	/* spi lcd init */
	kwh043st20_f01_spi_startup(1, 0, 5000000, SPI_MODE_0);

	/* backlight on */
	buf[0] = 0xf;
	i2c_write(0x24, 0x7, 1, buf, 1);
	buf[0] = 0x3f;
	i2c_write(0x24, 0x8, 1, buf, 1);
	return 0;
}

int arch_early_init_r(void)
{
	enable_lcd();
	return 0;
}

static int board_video_init(void)
{
	int i;
	int anzdisp = ARRAY_SIZE(lcd_panels);
	int display = 1;

	for (i = 0; i < anzdisp; i++) {
		if (strncmp((const char *)factory_dat.disp_name,
			    lcd_panels[i].name,
		    strlen((const char *)factory_dat.disp_name)) == 0) {
			printf("DISPLAY: %s\n", factory_dat.disp_name);
			break;
		}
	}
	if (i == anzdisp) {
		i = 1;
		printf("%s: %s not found, using default %s\n", __func__,
		       factory_dat.disp_name, lcd_panels[i].name);
	}
	conf_disp_pll(24, 1);
	da8xx_video_init(&lcd_panels[display], &lcd_cfgs[display],
			 lcd_cfgs[display].bpp);

	return 0;
}
#endif /* ifdef CONFIG_VIDEO */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;
	char tmp[2 * MAX_STRING_LENGTH + 2];

	omap_nand_switch_ecc(1, 8);

	if (factory_dat.asn[0] != 0)
		sprintf(tmp, "%s_%s", factory_dat.asn,
			factory_dat.comp_version);
	else
		strcpy(tmp, "QMX7.E38_4.0");

	ret = env_set("boardid", tmp);
	if (ret)
		printf("error setting board id\n");

	return 0;
}
#endif

#include "../common/board.c"
