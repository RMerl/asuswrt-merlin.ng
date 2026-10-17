// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ddr.h>
#include <asm/mach-imx/boot_mode.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <power/pmic.h>
#include <power/bd71837.h>

#include "lpddr4_timing.h"

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

int data_modul_imx_edm_sbc_board_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@4b", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* Unlock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x1);

	/* Increase VDD_SOC to typical value 0.85V before first DRAM access */
	pmic_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0x0f);

	/* Increase VDD_DRAM to 0.975V for 3GHz DDR */
	pmic_reg_write(dev, BD718XX_1ST_NODVS_BUCK_VOLT, 0x83);

	/* Lock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x11);

	return 0;
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (boot_dev_spl == MMC3_BOOT)
		return BOOT_DEVICE_MMC2;	/* eMMC */
	else
		return BOOT_DEVICE_MMC1;	/* SD */
}

void board_boot_order(u32 *spl_boot_list)
{
	int boot_device = spl_boot_device();

	spl_boot_list[0] = boot_device;		/* 1:SD 2:eMMC */

	if (boot_device == BOOT_DEVICE_MMC1)
		spl_boot_list[1] = BOOT_DEVICE_MMC2;	/* eMMC */
	else
		spl_boot_list[1] = BOOT_DEVICE_MMC1;	/* SD */

	spl_boot_list[2] = BOOT_DEVICE_UART;	/* YModem */
	spl_boot_list[3] = BOOT_DEVICE_NONE;
}

static struct dram_timing_info *dram_timing_info[8] = {
	&dmo_imx8mm_sbc_dram_timing_32_32,	/* 32 Gbit x32 */
	NULL,					/* 32 Gbit x16 */
	&dmo_imx8mm_sbc_dram_timing_16_32,	/* 16 Gbit x32 */
	NULL,					/* 16 Gbit x16 */
	NULL,					/* 8 Gbit x32 */
	NULL,					/* 8 Gbit x16 */
	NULL,					/* INVALID */
	NULL,					/* INVALID */
};

void board_init_f(ulong dummy)
{
	dmo_board_init_f(IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B, dram_timing_info);
}
