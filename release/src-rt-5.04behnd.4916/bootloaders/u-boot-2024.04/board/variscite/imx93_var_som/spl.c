// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 * Copyright 2023 Variscite Ltd.
 */

#include <command.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx93_pins.h>
#include <asm/arch/mu.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch-mx7ulp/gpio.h>
#include <asm/sections.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/mach-imx/syscounter.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <linux/delay.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccm_regs.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <asm/arch/trdc.h>

#include "../common/imx9_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

static struct var_eeprom eeprom = {0};

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	struct var_eeprom *ep = VAR_EEPROM_DATA;
	int ret;

	puts("Normal Boot\n");

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	/* Copy EEPROM contents to DRAM */
	memcpy(ep, &eeprom, sizeof(*ep));
}

void spl_dram_init(void)
{
	/* EEPROM initialization */
	var_eeprom_read_header(&eeprom);

	ddr_init(&dram_timing);
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_SPL_DM_PMIC_PCA9450)) {
		ret = pmic_get("pmic@25", &dev);
		if (ret == -ENODEV) {
			puts("No pca9450@25\n");
			return 0;
		}
		if (ret != 0)
			return ret;

		/* BUCKxOUT_DVS0/1 control BUCK123 output */
		pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

		/* enable DVS control through PMIC_STBY_REQ */
		pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x18);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x18);

		/* set standby voltage to 0.65V */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

		/* I2C_LT_EN*/
		pmic_reg_write(dev, 0xa, 0x3);

		/* set WDOG_B_CFG to cold reset */
		pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu(NULL, NULL);
	if (ret) {
		printf("Fail to init ELE API\n");
	} else {
		printf("SOC: 0x%x\n", gd->arch.soc_rev);
		printf("LC: 0x%x\n", gd->arch.lifecycle);
	}
	power_init_board();

	set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (!ret)
		printf("M33 prepare ok\n");

	board_init_r(NULL, 0);
}
