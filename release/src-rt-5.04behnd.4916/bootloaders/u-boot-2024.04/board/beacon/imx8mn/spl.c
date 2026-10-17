// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Compass Electronics Group, LLC
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mn_pins.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/sections.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/bd71837.h>
#include <spl.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	debug("Normal Boot\n");

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0)
		puts("Failed to find clock node. Check device tree\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

#define PWM1_PAD_CTRL (PAD_CTL_FSEL2 | PAD_CTL_DSE6)

static iomux_v3_cfg_t const pwm_pads[] = {
	IMX8MN_PAD_GPIO1_IO01__PWM1_OUT | MUX_PAD_CTRL(PWM1_PAD_CTRL),
};

static int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@4b", &dev);
	if (ret == -ENODEV) {
		puts("No pmic\n");
		return 0;
	}

	if (ret != 0)
		return ret;

	/* decrease RESET key long push time from the default 10s to 10ms */
	pmic_reg_write(dev, BD718XX_PWRONCONFIG1, 0x0);

	/* unlock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x1);

	/* increase VDD_SOC to typical value 0.85v before first DRAM access */
	pmic_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0x0f);

	/* increase VDD_DRAM to 0.975v for 3Ghz DDR */
	pmic_reg_write(dev, BD718XX_1ST_NODVS_BUCK_VOLT, 0x83);

	/* lock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x11);

	return 0;
}

int board_early_init_f(void)
{
	/* Claiming pwm pins prevents LCD flicker during startup*/
	imx_iomux_v3_setup_multiple_pads(pwm_pads, ARRAY_SIZE(pwm_pads));

	init_uart_clk(1);

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* LPDDR4 at 1.6GHz requires a voltage adjustment on the PMIC */
	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
