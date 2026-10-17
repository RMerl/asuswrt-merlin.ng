// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
 */

#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <power/pca9450.h>
#include <power/pmic.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_KTN_N801X,
	BOARD_TYPE_KTN_N802X,
	BOARD_TYPE_MAX
};

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const i2c1_pads[] = {
	IMX8MM_PAD_I2C1_SCL_I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL) | MUX_MODE_SION,
	IMX8MM_PAD_I2C1_SDA_I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL) | MUX_MODE_SION
};

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	case SPI_NOR_BOOT:
		return BOOT_DEVICE_SPI;
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}

bool check_ram_available(long size)
{
	long sz = get_ram_size((long *)PHYS_SDRAM, size);

	if (sz == size)
		return true;

	return false;
}

static void spl_dram_init(void)
{
	u32 size = 0;

	/*
	 * Try the default DDR settings in lpddr4_timing.c to
	 * comply with the Micron 4GB DDR.
	 */
	if (!ddr_init(&dram_timing) && check_ram_available(SZ_4G)) {
		size = 4;
	} else {
		/*
		 * Overwrite some values to comply with the Micron 1GB/2GB DDRs.
		 */
		dram_timing.ddrc_cfg[2].val = 0xa1080020;
		dram_timing.ddrc_cfg[37].val = 0x1f;

		dram_timing.fsp_msg[0].fsp_cfg[8].val = 0x110;
		dram_timing.fsp_msg[0].fsp_cfg[20].val = 0x1;
		dram_timing.fsp_msg[1].fsp_cfg[9].val = 0x110;
		dram_timing.fsp_msg[1].fsp_cfg[21].val = 0x1;
		dram_timing.fsp_msg[2].fsp_cfg[10].val = 0x110;
		dram_timing.fsp_msg[2].fsp_cfg[22].val = 0x1;

		if (!ddr_init(&dram_timing)) {
			if (check_ram_available(SZ_2G))
				size = 2;
			else if (check_ram_available(SZ_1G))
				size = 1;
		}
	}

	if (size == 0) {
		printf("Failed to initialize DDR RAM!\n");
		size = 1;
	}

	gd->ram_size = size;
	writel(size, MCU_BOOTROM_BASE_ADDR);
}

int do_board_detect(void)
{
	struct udevice *udev;

	/*
	 * Check for the RTC on the OSM module.
	 */
	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));

	if (i2c_get_chip_for_busnum(0, 0x52, 0, &udev) == 0) {
		gd->board_type = BOARD_TYPE_KTN_N802X;
		printf("Kontron OSM-S i.MX8MM (N802X) module, %u GB RAM detected\n",
		       (unsigned int)gd->ram_size);
	} else {
		gd->board_type = BOARD_TYPE_KTN_N801X;
		printf("Kontron SL i.MX8MM (N801X) module, %u GB RAM detected\n",
		       (unsigned int)gd->ram_size);
	}

	/*
	 * Check the I2C PMIC to detect the deprecated SoM with DA9063.
	 */
	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));

	if (i2c_get_chip_for_busnum(0, 0x58, 0, &udev) == 0) {
		printf("### ATTENTION: DEPRECATED SOM REVISION (N8010 Rev0) DETECTED! ###\n");
		printf("###  THIS HW IS NOT SUPPORTED AND BOOTING WILL PROBABLY FAIL  ###\n");
		printf("###             PLEASE UPGRADE TO LATEST MODULE               ###\n");
	}

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_KTN_N801X && is_imx8mm() &&
	    (!strcmp(name, "imx8mm-kontron-n801x-s") ||
	     !strcmp(name, "imx8mm-kontron-bl")))
		return 0;

	if (gd->board_type == BOARD_TYPE_KTN_N802X && is_imx8mm() &&
	    (!strcmp(name, "imx8mm-kontron-n802x-s") ||
	     !strcmp(name, "imx8mm-kontron-bl-osm-s")))
		return 0;

	return -1;
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	arch_misc_init();

	puts("Normal Boot\n");

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0)
		printf("Failed to find clock node. Check device tree\n");
}

static int power_init_board(void)
{
	struct udevice *dev;
	int ret  = pmic_get("pmic@25", &dev);

	if (ret == -ENODEV)
		puts("No pmic found\n");

	if (ret)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output, clear PRESET_EN */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* increase VDD_DRAM to 0.95V for 1.5GHz DDR */
	pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x1c);

	/* set VDD_SNVS_0V8 from default 0.85V to 0.8V */
	pmic_reg_write(dev, PCA9450_LDO2CTRL, 0xC0);

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(2);

	timer_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* PMIC initialization */
	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	/* Detect the board type */
	do_board_detect();

	board_init_r(NULL, 0);
}
