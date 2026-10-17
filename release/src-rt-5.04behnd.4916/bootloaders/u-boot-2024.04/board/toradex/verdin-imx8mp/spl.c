// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2022 Toradex
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/ddr.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	/*
	 * Try configuring for dual rank memory falling back to single rank
	 */
	if (!ddr_init(&dram_timing)) {
		puts("DDR configured as dual rank\n");
		return;
	}

	lpddr4_single_rank_training_patch();
	if (!ddr_init(&dram_timing)) {
		puts("DDR configured as single rank\n");
		return;
	}
	puts("DDR configuration failed\n");
}

void spl_board_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);

	puts("Normal Boot\n");
}

#define I2C_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX8MP_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = MX8MP_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

#if CONFIG_IS_ENABLED(POWER_LEGACY)
#define I2C_PMIC	0
int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_pca9450_init(I2C_PMIC, 0x25);
	if (ret)
		printf("power init failed\n");
	p = pmic_get("PCA9450");
	pmic_probe(p);

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(p, PCA9450_BUCK123_DVS, 0x29);

	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* set DVS0 to 0.85v for special case */
		pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x1c);
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD freq for SoC */
	/* To avoid timing risk from SoC to ARM, increase VDD_ARM to OD voltage 0.95v */
	pmic_reg_write(p, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* set LDO4 and CONFIG2 to enable the I2C level translator */
	pmic_reg_write(p, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(p, PCA9450_CONFIG2, 0x1);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_SPL_LOAD_FIT)
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

/* Do not use BSS area in this phase */
void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(1);

	board_early_init_f();

	ret = spl_early_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* Adjust PMIC voltage to 1.0V for 800 MHz */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	/* PMIC initialization */
	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}
