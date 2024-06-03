// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <dm.h>
#include <dwc3-uboot.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>
#include <mmc.h>
#include <netdev.h>
#include <samsung-usb-phy-uboot.h>
#include <spi.h>
#include <usb.h>
#include <video_bridge.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/regulator.h>
#include <power/s2mps11.h>
#include <power/s5m8767.h>
#include <samsung/exynos5-dt-types.h>
#include <samsung/misc.h>
#include <tmu.h>

DECLARE_GLOBAL_DATA_PTR;

int exynos_init(void)
{
	return 0;
}

static int exynos_set_regulator(const char *name, uint uv)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname(name, &dev);
	if (ret) {
		debug("%s: Cannot find regulator %s\n", __func__, name);
		return ret;
	}
	ret = regulator_set_value(dev, uv);
	if (ret) {
		debug("%s: Cannot set regulator %s\n", __func__, name);
		return ret;
	}

	return 0;
}

int exynos_power_init(void)
{
	struct udevice *dev;
	int ret;

#ifdef CONFIG_PMIC_S2MPS11
	ret = pmic_get("s2mps11_pmic", &dev);
#else
	ret = pmic_get("max77686", &dev);
	if (!ret) {
		/* TODO(sjg@chromium.org): Move into the clock/pmic API */
		ret = pmic_clrsetbits(dev, MAX77686_REG_PMIC_32KHZ, 0,
				MAX77686_32KHCP_EN);
		if (ret)
			return ret;
		ret = pmic_clrsetbits(dev, MAX77686_REG_PMIC_BBAT, 0,
				MAX77686_BBCHOSTEN | MAX77686_BBCVS_3_5V);
		if (ret)
			return ret;
	} else {
		ret = pmic_get("s5m8767-pmic", &dev);
		/* TODO(sjg@chromium.org): Use driver model to access clock */
#ifdef CONFIG_PMIC_S5M8767
		if (!ret)
			s5m8767_enable_32khz_cp(dev);
#endif
	}
#endif	/* CONFIG_PMIC_S2MPS11 */
	if (ret == -ENODEV)
		return 0;

	ret = regulators_enable_boot_on(false);
	if (ret)
		return ret;

	ret = exynos_set_regulator("vdd_mif", 1100000);
	if (ret)
		return ret;

	ret = exynos_set_regulator("vdd_arm", 1300000);
	if (ret)
		return ret;
	ret = exynos_set_regulator("vdd_int", 1012500);
	if (ret)
		return ret;
	ret = exynos_set_regulator("vdd_g3d", 1200000);
	if (ret)
		return ret;

	return 0;
}

int board_get_revision(void)
{
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = 0x12400000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	struct exynos_usb3_phy *phy = (struct exynos_usb3_phy *)
		samsung_get_base_usb3_phy();

	if (!phy) {
		pr_err("usb3 phy not supported\n");
		return -ENODEV;
	}

	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_EN);
	exynos5_usb3_phy_init(phy);

	return dwc3_uboot_init(&dwc3_device_data);
}
#endif
#ifdef CONFIG_SET_DFU_ALT_INFO
char *get_dfu_alt_system(char *interface, char *devstr)
{
	char *info = "Not supported!";

	if (board_is_odroidxu4() || board_is_odroidhc1() || board_is_odroidhc2())
		return info;

	return env_get("dfu_alt_system");
}

char *get_dfu_alt_boot(char *interface, char *devstr)
{
	char *info = "Not supported!";
	struct mmc *mmc;
	char *alt_boot;
	int dev_num;

	if (board_is_odroidxu4() || board_is_odroidhc1() || board_is_odroidhc2())
		return info;

	dev_num = simple_strtoul(devstr, NULL, 10);

	mmc = find_mmc_device(dev_num);
	if (!mmc)
		return NULL;

	if (mmc_init(mmc))
		return NULL;

	if (IS_SD(mmc))
		alt_boot = CONFIG_DFU_ALT_BOOT_SD;
	else
		alt_boot = CONFIG_DFU_ALT_BOOT_EMMC;

	return alt_boot;
}
#endif
