// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <spi.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/adc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/watchdog.h>
#include <ld9040.h>
#include <power/pmic.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <asm/arch/cpu.h>
#include <power/max8998_pmic.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <usb_mass_storage.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int board_rev;
static int init_pmic_lcd(void);

u32 get_board_rev(void)
{
	return board_rev;
}

int exynos_power_init(void)
{
	return init_pmic_lcd();
}

static int get_hwrev(void)
{
	return board_rev & 0xFF;
}

static unsigned short get_adc_value(int channel)
{
	struct s5p_adc *adc = (struct s5p_adc *)samsung_get_base_adc();
	unsigned short ret = 0;
	unsigned int reg;
	unsigned int loop = 0;

	writel(channel & 0xF, &adc->adcmux);
	writel((1 << 14) | (49 << 6), &adc->adccon);
	writel(1000 & 0xffff, &adc->adcdly);
	writel(readl(&adc->adccon) | (1 << 16), &adc->adccon); /* 12 bit */
	udelay(10);
	writel(readl(&adc->adccon) | (1 << 0), &adc->adccon); /* Enable */
	udelay(10);

	do {
		udelay(1);
		reg = readl(&adc->adccon);
	} while (!(reg & (1 << 15)) && (loop++ < 1000));

	ret = readl(&adc->adcdat0) & 0xFFF;

	return ret;
}

static int adc_power_control(int on)
{
	struct udevice *dev;
	int ret;
	u8 reg;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		puts("Failed to get MAX8998!\n");
		return ret;
	}

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
	if (on)
		reg |= MAX8998_LDO4;
	else
		reg &= ~MAX8998_LDO4;

	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);
	if (ret) {
		puts("MAX8998 LDO setting error\n");
		return -EINVAL;
	}

	return 0;
}

static unsigned int get_hw_revision(void)
{
	int hwrev, mode0, mode1;

	adc_power_control(1);

	mode0 = get_adc_value(1);		/* HWREV_MODE0 */
	mode1 = get_adc_value(2);		/* HWREV_MODE1 */

	/*
	 * XXX Always set the default hwrev as the latest board
	 * ADC = (voltage) / 3.3 * 4096
	 */
	hwrev = 3;

#define IS_RANGE(x, min, max)	((x) > (min) && (x) < (max))
	if (IS_RANGE(mode0, 80, 200) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x0;		/* 0.01V	0.01V */
	if (IS_RANGE(mode0, 750, 1000) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x1;		/* 610mV	0.01V */
	if (IS_RANGE(mode0, 1300, 1700) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x2;		/* 1.16V	0.01V */
	if (IS_RANGE(mode0, 2000, 2400) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x3;		/* 1.79V	0.01V */
#undef IS_RANGE

	debug("mode0: %d, mode1: %d, hwrev 0x%x\n", mode0, mode1, hwrev);

	adc_power_control(0);

	return hwrev;
}

static void check_hw_revision(void)
{
	int hwrev;

	hwrev = get_hw_revision();

	board_rev |= hwrev;
}

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	struct udevice *dev;
	int ret;
	u8 reg;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		puts("Failed to get MAX8998!\n");
		return ret;
	}

	if (on) {
		reg = pmic_reg_read(dev, MAX8998_REG_BUCK_ACTIVE_DISCHARGE3);
		reg |= MAX8998_SAFEOUT1;
		ret |= pmic_reg_write(dev,
			MAX8998_REG_BUCK_ACTIVE_DISCHARGE3, reg);

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg |= MAX8998_LDO3;
		ret |= pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg |= MAX8998_LDO8;
		ret |= pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);

	} else {
		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
		reg &= ~MAX8998_LDO8;
		ret |= pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);

		reg = pmic_reg_read(dev, MAX8998_REG_ONOFF1);
		reg &= ~MAX8998_LDO3;
		ret |= pmic_reg_write(dev, MAX8998_REG_ONOFF1, reg);

		reg = pmic_reg_read(dev, MAX8998_REG_BUCK_ACTIVE_DISCHARGE3);
		reg &= ~MAX8998_SAFEOUT1;
		ret |= pmic_reg_write(dev,
			MAX8998_REG_BUCK_ACTIVE_DISCHARGE3, reg);
	}

	if (ret) {
		puts("MAX8998 LDO setting error!\n");
		return -EINVAL;
	}

	return 0;
}

struct dwc2_plat_otg_data s5pc210_otg_data = {
	.phy_control = s5pc210_phy_control,
	.regs_phy = EXYNOS4_USBPHY_BASE,
	.regs_otg = EXYNOS4_USBOTG_BASE,
	.usb_phy_ctrl = EXYNOS4_USBPHY_CONTROL,
	.usb_flags = PHY0_SLEEP,
};
#endif

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5pc210_otg_data);
}

int exynos_early_init_f(void)
{
	wdt_stop();

	return 0;
}

static int init_pmic_lcd(void)
{
	struct udevice *dev;
	unsigned char val;
	int ret = 0;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		puts("Failed to get MAX8998 for init_pmic_lcd()!\n");
		return ret;
	}

	/* LDO7 1.8V */
	val = 0x02; /* (1800 - 1600) / 100; */
	ret |= pmic_reg_write(dev,  MAX8998_REG_LDO7, val);

	/* LDO17 3.0V */
	val = 0xe; /* (3000 - 1600) / 100; */
	ret |= pmic_reg_write(dev,  MAX8998_REG_LDO17, val);

	/* Disable unneeded regulators */
	/*
	 * ONOFF1
	 * Buck1 ON, Buck2 OFF, Buck3 ON, Buck4 ON
	 * LDO2 ON, LDO3 OFF, LDO4 OFF, LDO5 ON
	 */
	val = 0xB9;
	ret |= pmic_reg_write(dev,  MAX8998_REG_ONOFF1, val);

	/* ONOFF2
	 * LDO6 OFF, LDO7 ON, LDO8 OFF, LDO9 ON,
	 * LDO10 OFF, LDO11 OFF, LDO12 OFF, LDO13 OFF
	 */
	val = 0x50;
	ret |= pmic_reg_write(dev,  MAX8998_REG_ONOFF2, val);

	/* ONOFF3
	 * LDO14 OFF, LDO15 OFF, LGO16 OFF, LDO17 OFF
	 * EPWRHOLD OFF, EBATTMON OFF, ELBCNFG2 OFF, ELBCNFG1 OFF
	 */
	val = 0x00;
	ret |= pmic_reg_write(dev,  MAX8998_REG_ONOFF3, val);

	if (ret) {
		puts("LCD pmic initialisation error!\n");
		return -EINVAL;
	}

	return 0;
}

void exynos_cfg_lcd_gpio(void)
{
	unsigned int i, f3_end = 4;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(EXYNOS4_GPIO_F00 + i, S5P_GPIO_FUNC(2));
		gpio_cfg_pin(EXYNOS4_GPIO_F10 + i, S5P_GPIO_FUNC(2));
		gpio_cfg_pin(EXYNOS4_GPIO_F20 + i, S5P_GPIO_FUNC(2));
		/* pull-up/down disable */
		gpio_set_pull(EXYNOS4_GPIO_F00 + i, S5P_GPIO_PULL_NONE);
		gpio_set_pull(EXYNOS4_GPIO_F10 + i, S5P_GPIO_PULL_NONE);
		gpio_set_pull(EXYNOS4_GPIO_F20 + i, S5P_GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		gpio_set_drv(EXYNOS4_GPIO_F00 + i, S5P_GPIO_DRV_4X);
		gpio_set_rate(EXYNOS4_GPIO_F00 + i, S5P_GPIO_DRV_SLOW);
		gpio_set_drv(EXYNOS4_GPIO_F10 + i, S5P_GPIO_DRV_4X);
		gpio_set_rate(EXYNOS4_GPIO_F10 + i, S5P_GPIO_DRV_SLOW);
		gpio_set_drv(EXYNOS4_GPIO_F20 + i, S5P_GPIO_DRV_4X);
		gpio_set_rate(EXYNOS4_GPIO_F00 + i, S5P_GPIO_DRV_SLOW);
	}

	for (i = EXYNOS4_GPIO_F30; i < (EXYNOS4_GPIO_F30 + f3_end); i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(i, S5P_GPIO_FUNC(2));
		/* pull-up/down disable */
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
		gpio_set_rate(i, S5P_GPIO_DRV_SLOW);
	}

	/* gpio pad configuration for LCD reset. */
	gpio_request(EXYNOS4_GPIO_Y45, "lcd_reset");
	gpio_cfg_pin(EXYNOS4_GPIO_Y45, S5P_GPIO_OUTPUT);
}

int mipi_power(void)
{
	return 0;
}

void exynos_reset_lcd(void)
{
	gpio_set_value(EXYNOS4_GPIO_Y45, 1);
	udelay(10000);
	gpio_set_value(EXYNOS4_GPIO_Y45, 0);
	udelay(10000);
	gpio_set_value(EXYNOS4_GPIO_Y45, 1);
	udelay(100);
}

void exynos_lcd_power_on(void)
{
	struct udevice *dev;
	int ret;
	u8 reg;

	ret = pmic_get("max8998-pmic", &dev);
	if (ret) {
		puts("Failed to get MAX8998!\n");
		return;
	}

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF3);
	reg |= MAX8998_LDO17;
	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF3, reg);
	if (ret) {
		puts("MAX8998 LDO setting error\n");
		return;
	}

	reg = pmic_reg_read(dev, MAX8998_REG_ONOFF2);
	reg |= MAX8998_LDO7;
	ret = pmic_reg_write(dev, MAX8998_REG_ONOFF2, reg);
	if (ret) {
		puts("MAX8998 LDO setting error\n");
		return;
	}
}

void exynos_cfg_ldo(void)
{
	ld9040_cfg_ldo();
}

void exynos_enable_ldo(unsigned int onoff)
{
	ld9040_enable_ldo(onoff);
}

int exynos_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_UNIVERSAL_C210;

	switch (get_hwrev()) {
	case 0:
		/*
		 * Set the low to enable LDO_EN
		 * But when you use the test board for eMMC booting
		 * you should set it HIGH since it removes the inverter
		 */
		/* MASSMEMORY_EN: XMDMDATA_6: GPE3[6] */
		gpio_request(EXYNOS4_GPIO_E36, "ldo_en");
		gpio_direction_output(EXYNOS4_GPIO_E36, 0);
		break;
	default:
		/*
		 * Default reset state is High and there's no inverter
		 * But set it as HIGH to ensure
		 */
		/* MASSMEMORY_EN: XMDMADDR_3: GPE1[3] */
		gpio_request(EXYNOS4_GPIO_E13, "massmemory_en");
		gpio_direction_output(EXYNOS4_GPIO_E13, 1);
		break;
	}

	check_hw_revision();
	printf("HW Revision:\t0x%x\n", board_rev);

	return 0;
}

#ifdef CONFIG_LCD
void exynos_lcd_misc_init(vidinfo_t *vid)
{
#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif

	/* for LD9040. */
	vid->pclk_name = 1;	/* MPLL */
	vid->sclk_div = 1;

	env_set("lcdinfo", "lcd=ld9040");
}
#endif
