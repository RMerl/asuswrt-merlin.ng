// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Toradex
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <cpu_func.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <hang.h>
#include <i2c.h>
#include <power/pca9450.h>
#include <power/pmic.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define I2C_PMIC_BUS_ID        1

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case MMC1_BOOT: /* eMMC */
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT: /* SD card */
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	arch_misc_init();
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

__weak void board_early_init(void)
{
	init_uart_clk(0);
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_SPL_DM_PMIC_PCA9450)) {
		ret = pmic_get("pmic@25", &dev);
		if (ret == -ENODEV) {
			puts("No pmic found\n");
			return ret;
		}

		if (ret != 0)
			return ret;

		/* BUCKxOUT_DVS0/1 control BUCK123 output, clear PRESET_EN */
		pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

		/* increase VDD_DRAM to 0.975v for 1.5Ghz DDR */
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x1c);

		pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

		return 0;
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();

	board_early_init();

	timer_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
