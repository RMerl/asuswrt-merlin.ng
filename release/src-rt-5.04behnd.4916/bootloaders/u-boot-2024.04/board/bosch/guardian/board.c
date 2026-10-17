// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for Bosch Guardian
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 * Copyright (C) 2018 Robert Bosch Power Tools GmbH
 */

#include <common.h>
#include <dm.h>
#include <env_internal.h>
#include <errno.h>
#include <i2c.h>
#include <led.h>
#include <panel.h>
#include <linux/delay.h>
#include <asm/global_data.h>
#include <power/tps65217.h>
#include <spl.h>
#include <watchdog.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mem-guardian.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <jffs2/load_kernel.h>
#include <mtd.h>
#include <nand.h>
#include <video.h>
#include <video_console.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41K128M16JT125K_RD_DQS,
	.datawdsratio0 = MT41K128M16JT125K_WR_DQS,
	.datafwsratio0 = MT41K128M16JT125K_PHY_FIFO_WE,
	.datawrsratio0 = MT41K128M16JT125K_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41K128M16JT125K_RATIO,
	.cmd0iclkout = MT41K128M16JT125K_INVERT_CLKOUT,

	.cmd1csratio = MT41K128M16JT125K_RATIO,
	.cmd1iclkout = MT41K128M16JT125K_INVERT_CLKOUT,

	.cmd2csratio = MT41K128M16JT125K_RATIO,
	.cmd2iclkout = MT41K128M16JT125K_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41K128M16JT125K_EMIF_SDCFG,
	.ref_ctrl = MT41K128M16JT125K_EMIF_SDREF,
	.sdram_tim1 = MT41K128M16JT125K_EMIF_TIM1,
	.sdram_tim2 = MT41K128M16JT125K_EMIF_TIM2,
	.sdram_tim3 = MT41K128M16JT125K_EMIF_TIM3,
	.zq_config = MT41K128M16JT125K_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K128M16JT125K_EMIF_READ_LATENCY,
};

#define OSC	(V_OSCK / 1000000)
const struct dpll_params dpll_ddr = {
		400, OSC - 1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	int mpu_vdd;
	int usb_cur_lim;

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	/* Initialize for Power Management */
	if (power_tps65217_init(0))
		return;

	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
	} else {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
	}

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			       TPS65217_POWER_PATH,
			       usb_cur_lim,
			       TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/*
	 * Set LDO3 to 1.8V and LDO4 to 3.3V
	 */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	enable_i2c0_pin_mux();

	return &dpll_ddr;
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT41K128M16JT125K_IOCTRL_VALUE,
	.cm1ioctl		= MT41K128M16JT125K_IOCTRL_VALUE,
	.cm2ioctl		= MT41K128M16JT125K_IOCTRL_VALUE,
	.dt0ioctl		= MT41K128M16JT125K_IOCTRL_VALUE,
	.dt1ioctl		= MT41K128M16JT125K_IOCTRL_VALUE,
};

void sdram_init(void)
{
	config_ddr(400, &ioregs,
		   &ddr3_data,
		   &ddr3_cmd_ctrl_data,
		   &ddr3_emif_reg_data, 0);
}
#endif

int board_init(void)
{
	save_omap_boot_params();

#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_MTD_RAW_NAND
	gpmc_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
static void set_bootmode_env(void)
{
	char *boot_mode_gpio = "gpio@44e07000_14";
	int   ret;

	struct gpio_desc boot_mode_desc;

	ret = dm_gpio_lookup_name(boot_mode_gpio, &boot_mode_desc);
	if (ret) {
		printf("%s is not found\n", boot_mode_gpio);
		goto err;
	}

	ret = dm_gpio_request(&boot_mode_desc, "setup_bootmode_env");
	if (ret && ret != -EBUSY) {
		printf("requesting gpio: %s failed\n", boot_mode_gpio);
		goto err;
	}

	dm_gpio_set_dir_flags(&boot_mode_desc, GPIOD_IS_IN);
	udelay(10);

	ret = dm_gpio_get_value(&boot_mode_desc);
	if (ret == 0) {
		env_set("swi_status", "1");
	} else if (ret == 1) {
		env_set("swi_status", "0");
	} else {
		printf("swi status gpio error\n");
		goto err;
	}

	return;

err:
	env_set("swi_status", "err");
}

void lcdbacklight_en(void)
{
	unsigned long brightness = env_get_ulong("backlight_brightness", 10, 50);

	if (brightness > 99 || brightness == 0)
		brightness = 99;

	/*
	 * Brightness range:
	 * WLEDCTRL2 DUTY[6:0]
	 *
	 * 000 0000b = 1%
	 * 000 0001b = 2%
	 * ...
	 * 110 0010b = 99%
	 * 110 0011b = 100%
	 *
	 */

	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_WLEDCTRL2,
			   brightness, 0xFF);
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_WLEDCTRL1,
			   brightness != 0 ? 0x0A : 0x02, 0xFF);
}

int board_late_init(void)
{
	int ret;
	struct udevice *cdev;

	set_bootmode_env();

	ret = uclass_get_device(UCLASS_PANEL, 0, &cdev);
	if (ret) {
		debug("video panel not found: %d\n", ret);
		return ret;
	}

	/* Initialize to enable backlight */
	if (power_tps65217_init(0))
		return 0;

	lcdbacklight_en();

	return 0;
}
#endif /* CONFIG_BOARD_LATE_INIT */
