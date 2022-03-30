// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for TI AM335X based pxm2 board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <environment.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include "../../../drivers/video/da8xx-fb.h"
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <watchdog.h>
#include "board.h"
#include "../common/factoryset.h"
#include "pmic.h"
#include <nand.h>
#include <bmp_layout.h>

#ifdef CONFIG_SPL_BUILD
static void board_init_ddr(void)
{
struct emif_regs pxm2_ddr3_emif_reg_data = {
	.sdram_config = 0x41805332,
	.sdram_tim1 = 0x666b3c9,
	.sdram_tim2 = 0x243631ca,
	.sdram_tim3 = 0x33f,
	.emif_ddr_phy_ctlr_1 = 0x100005,
	.zq_config = 0,
	.ref_ctrl = 0x81a,
};

struct ddr_data pxm2_ddr3_data = {
	.datardsratio0 = 0x81204812,
	.datawdsratio0 = 0,
	.datafwsratio0 = 0x8020080,
	.datawrsratio0 = 0x4010040,
};

struct cmd_control pxm2_ddr3_cmd_ctrl_data = {
	.cmd0csratio = 0x80,
	.cmd0iclkout = 0,
	.cmd1csratio = 0x80,
	.cmd1iclkout = 0,
	.cmd2csratio = 0x80,
	.cmd2iclkout = 0,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= DDR_IOCTRL_VAL,
	.cm1ioctl		= DDR_IOCTRL_VAL,
	.cm2ioctl		= DDR_IOCTRL_VAL,
	.dt0ioctl		= DDR_IOCTRL_VAL,
	.dt1ioctl		= DDR_IOCTRL_VAL,
};

	config_ddr(DDR_PLL_FREQ, &ioregs, &pxm2_ddr3_data,
		   &pxm2_ddr3_cmd_ctrl_data, &pxm2_ddr3_emif_reg_data, 0);
}

/*
 * voltage switching for MPU frequency switching.
 * @module = mpu - 0, core - 1
 * @vddx_op_vol_sel = vdd voltage to set
 */

#define MPU	0
#define CORE	1

int voltage_update(unsigned int module, unsigned char vddx_op_vol_sel)
{
	uchar buf[4];
	unsigned int reg_offset;

	if (module == MPU)
		reg_offset = PMIC_VDD1_OP_REG;
	else
		reg_offset = PMIC_VDD2_OP_REG;

	/* Select VDDx OP   */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_CMD_MASK;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	/* Configure VDDx OP  Voltage */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_SEL_MASK;
	buf[0] |= vddx_op_vol_sel;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	if ((buf[0] & PMIC_OP_REG_SEL_MASK) != vddx_op_vol_sel)
		return 1;

	return 0;
}

#define OSC     (V_OSCK/1000000)

const struct dpll_params dpll_mpu_pxm2 = {
		720, OSC-1, 1, -1, -1, -1, -1};

void spl_siemens_board_init(void)
{
	uchar buf[4];
	/*
	 * pxm2 PMIC code.  All boards currently want an MPU voltage
	 * of 1.2625V and CORE voltage of 1.1375V to operate at
	 * 720MHz.
	 */
	if (i2c_probe(PMIC_CTRL_I2C_ADDR))
		return;

	/* VDD1/2 voltage selection register access by control i/f */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, PMIC_DEVCTRL_REG, 1, buf, 1))
		return;

	buf[0] |= PMIC_DEVCTRL_REG_SR_CTL_I2C_SEL_CTL_I2C;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, PMIC_DEVCTRL_REG, 1, buf, 1))
		return;

	/* Frequency switching for OPP 120 */
	if (voltage_update(MPU, PMIC_OP_REG_SEL_1_2_6) ||
	    voltage_update(CORE, PMIC_OP_REG_SEL_1_1_3)) {
		printf("voltage update failed\n");
	}
}
#endif /* if def CONFIG_SPL_BUILD */

int read_eeprom(void)
{
	/* nothing ToDo here for this board */

	return 0;
}

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 4,
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
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
	int n = 0;
#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
#ifdef CONFIG_FACTORYSET
	int rv;
	if (!is_valid_ethaddr(factory_dat.mac))
		printf("Error: no valid mac address\n");
	else
		eth_env_set_enetaddr("ethaddr", factory_dat.mac);
#endif /* #ifdef CONFIG_FACTORYSET */

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel(RGMII_MODE_ENABLE  | RGMII_INT_DELAY, &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif
	return n;
}
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */

#if defined(CONFIG_VIDEO) && !defined(CONFIG_SPL_BUILD)
static struct da8xx_panel lcd_panels[] = {
	/* AUO G156XW01 V1 */
	[0] = {
		.name = "AUO_G156XW01_V1",
		.width = 1376,
		.height = 768,
		.hfp = 14,
		.hbp = 64,
		.hsw = 56,
		.vfp = 1,
		.vbp = 28,
		.vsw = 3,
		.pxl_clk = 60000000,
		.invert_pxl_clk = 0,
	},
	/* AUO B101EVN06 V0 */
	[1] = {
		.name = "AUO_B101EVN06_V0",
		.width = 1280,
		.height = 800,
		.hfp = 52,
		.hbp = 84,
		.hsw = 36,
		.vfp = 3,
		.vbp = 14,
		.vsw = 6,
		.pxl_clk = 60000000,
		.invert_pxl_clk = 0,
	},
	/*
	 * Settings from factoryset
	 * stored in EEPROM
	 */
	[2] = {
		.name = "factoryset",
		.width = 0,
		.height = 0,
		.hfp = 0,
		.hbp = 0,
		.hsw = 0,
		.vfp = 0,
		.vbp = 0,
		.vsw = 0,
		.pxl_clk = 60000000,
		.invert_pxl_clk = 0,
	},
};

static const struct display_panel disp_panel = {
	WVGA,
	32,
	16,
	COLOR_ACTIVE,
};

static const struct lcd_ctrl_config lcd_cfg = {
	&disp_panel,
	.ac_bias		= 255,
	.ac_bias_intrpt		= 0,
	.dma_burst_sz		= 16,
	.bpp			= 32,
	.fdd			= 0x80,
	.tft_alt_mode		= 0,
	.stn_565_mode		= 0,
	.mono_8bit_mode		= 0,
	.invert_line_clock	= 1,
	.invert_frm_clock	= 1,
	.sync_edge		= 0,
	.sync_ctrl		= 1,
	.raster_order		= 0,
};

static int set_gpio(int gpio, int state)
{
	gpio_request(gpio, "temp");
	gpio_direction_output(gpio, state);
	gpio_set_value(gpio, state);
	gpio_free(gpio);
	return 0;
}

static int enable_backlight(void)
{
	set_gpio(BOARD_LCD_POWER, 1);
	set_gpio(BOARD_BACK_LIGHT, 1);
	set_gpio(BOARD_TOUCH_POWER, 1);
	return 0;
}

static int enable_pwm(void)
{
	struct pwmss_regs *pwmss = (struct pwmss_regs *)PWMSS0_BASE;
	struct pwmss_ecap_regs *ecap;
	int ticks = PWM_TICKS;
	int duty = PWM_DUTY;

	ecap = (struct pwmss_ecap_regs *)AM33XX_ECAP0_BASE;
	/* enable clock */
	setbits_le32(&pwmss->clkconfig, ECAP_CLK_EN);
	/* TimeStam Counter register */
	writel(0xdb9, &ecap->tsctr);
	/* config period */
	writel(ticks - 1, &ecap->cap3);
	writel(ticks - 1, &ecap->cap1);
	setbits_le16(&ecap->ecctl2,
		     (ECTRL2_MDSL_ECAP | ECTRL2_SYNCOSEL_MASK | 0xd0));
	/* config duty */
	writel(duty, &ecap->cap2);
	writel(duty, &ecap->cap4);
	/* start */
	setbits_le16(&ecap->ecctl2, ECTRL2_CTRSTP_FREERUN);
	return 0;
}

static struct dpll_regs dpll_lcd_regs = {
	.cm_clkmode_dpll = CM_WKUP + 0x98,
	.cm_idlest_dpll = CM_WKUP + 0x48,
	.cm_clksel_dpll = CM_WKUP + 0x54,
};

/* no console on this board */
int board_cfb_skip(void)
{
	return 1;
}

#define PLL_GET_M(v) ((v >> 8) & 0x7ff)
#define PLL_GET_N(v) (v & 0x7f)

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
	struct cm_dpll *cmdpll = (struct cm_dpll *)CM_DPLL;
	struct dpll_params dpll_lcd = {m, n, -1, -1, -1, -1, -1};

	u32 *const clk_domains[] = {
		&cmper->lcdclkctrl,
		0
	};
	u32 *const clk_modules_explicit_en[] = {
		&cmper->lcdclkctrl,
		&cmper->lcdcclkstctrl,
		&cmper->epwmss0clkctrl,
		0
	};
	do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);
	writel(0x0, &cmdpll->clklcdcpixelclk);

	do_setup_dpll(&dpll_lcd_regs, &dpll_lcd);

	return 0;
}

static int board_video_init(void)
{
	conf_disp_pll(24, 1);
	if (factory_dat.pxm50)
		da8xx_video_init(&lcd_panels[0], &lcd_cfg, lcd_cfg.bpp);
	else
		da8xx_video_init(&lcd_panels[1], &lcd_cfg, lcd_cfg.bpp);

	enable_pwm();
	enable_backlight();

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;

	omap_nand_switch_ecc(1, 8);

#ifdef CONFIG_FACTORYSET
	if (factory_dat.asn[0] != 0) {
		char tmp[2 * MAX_STRING_LENGTH + 2];

		if (strncmp((const char *)factory_dat.asn, "PXM50", 5) == 0)
			factory_dat.pxm50 = 1;
		else
			factory_dat.pxm50 = 0;
		sprintf(tmp, "%s_%s", factory_dat.asn,
			factory_dat.comp_version);
		ret = env_set("boardid", tmp);
		if (ret)
			printf("error setting board id\n");
	} else {
		factory_dat.pxm50 = 1;
		ret = env_set("boardid", "PXM50_1.0");
		if (ret)
			printf("error setting board id\n");
	}
	debug("PXM50: %d\n", factory_dat.pxm50);
#endif

	return 0;
}
#endif

#include "../common/board.c"
