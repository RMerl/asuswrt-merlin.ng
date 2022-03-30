// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 */

#include <common.h>
#include <lcd.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/mipi_dsim.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <power/max77693_muic.h>
#include <power/max77693_fg.h>
#include <libtizen.h>
#include <errno.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <usb_mass_storage.h>

static unsigned int board_rev = -1;

static inline u32 get_model_rev(void);

static void check_hw_revision(void)
{
	int modelrev = 0;
	char str[12];
	int i;

	/*
	 * GPM1[1:0]: MODEL_REV[1:0]
	 * Don't set as pull-none for these N/C pin.
	 * TRM say that it may cause unexcepted state and leakage current.
	 * and pull-none is only for output function.
	 */
	for (i = 0; i < 2; i++) {
		int pin = i + EXYNOS4X12_GPIO_M10;

		sprintf(str, "model_rev%d", i);
		gpio_request(pin, str);
		gpio_cfg_pin(pin, S5P_GPIO_INPUT);
	}

	/* GPM1[5:2]: HW_REV[3:0] */
	for (i = 0; i < 4; i++) {
		int pin = i + EXYNOS4X12_GPIO_M12;

		sprintf(str, "hw_rev%d", i);
		gpio_request(pin, str);
		gpio_cfg_pin(pin, S5P_GPIO_INPUT);
		gpio_set_pull(pin, S5P_GPIO_PULL_NONE);
	}

	/* GPM1[1:0]: MODEL_REV[1:0] */
	for (i = 0; i < 2; i++)
		modelrev |= (gpio_get_value(EXYNOS4X12_GPIO_M10 + i) << i);

	/* board_rev[15:8] = model */
	board_rev = modelrev << 8;
}

u32 get_board_rev(void)
{
	return board_rev;
}

static inline u32 get_model_rev(void)
{
	return (board_rev >> 8) & 0xff;
}

static void board_external_gpio_init(void)
{
	/*
	 * some pins which in alive block are connected with external pull-up
	 * but it's default setting is pull-down.
	 * if that pin set as input then that floated
	 */

	gpio_set_pull(EXYNOS4X12_GPIO_X02, S5P_GPIO_PULL_NONE);	/* PS_ALS_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X04, S5P_GPIO_PULL_NONE);	/* TSP_nINT */
	gpio_set_pull(EXYNOS4X12_GPIO_X07, S5P_GPIO_PULL_NONE);	/* AP_PMIC_IRQ*/
	gpio_set_pull(EXYNOS4X12_GPIO_X15, S5P_GPIO_PULL_NONE);	/* IF_PMIC_IRQ*/
	gpio_set_pull(EXYNOS4X12_GPIO_X20, S5P_GPIO_PULL_NONE);	/* VOL_UP */
	gpio_set_pull(EXYNOS4X12_GPIO_X21, S5P_GPIO_PULL_NONE);	/* VOL_DOWN */
	gpio_set_pull(EXYNOS4X12_GPIO_X23, S5P_GPIO_PULL_NONE);	/* FUEL_ALERT */
	gpio_set_pull(EXYNOS4X12_GPIO_X24, S5P_GPIO_PULL_NONE);	/* ADC_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X27, S5P_GPIO_PULL_NONE);	/* nPOWER */
	gpio_set_pull(EXYNOS4X12_GPIO_X30, S5P_GPIO_PULL_NONE);	/* WPC_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X35, S5P_GPIO_PULL_NONE);	/* OK_KEY */
	gpio_set_pull(EXYNOS4X12_GPIO_X37, S5P_GPIO_PULL_NONE);	/* HDMI_HPD */
}

int exynos_early_init_f(void)
{
	board_external_gpio_init();

	return 0;
}

int exynos_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)samsung_get_base_power();

	check_hw_revision();
	printf("HW Revision:\t0x%04x\n", board_rev);

	/*
	 * First bootloader on the TRATS2 platform uses
	 * INFORM4 and INFORM5 registers for recovery
	 *
	 * To indicate correct boot chain - those two
	 * registers must be cleared out
	 */
	writel(0, &pwr->inform4);
	writel(0, &pwr->inform5);

	return 0;
}

int exynos_power_init(void)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	int chrg;
	struct power_battery *pb;
	struct pmic *p_chrg, *p_muic, *p_fg, *p_bat;

	pmic_init_max77693(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_muic_init(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_fg_init(I2C_9);		/* I2C adapter 9 - bus name soft0 */
	power_bat_init(0);

	p_chrg = pmic_get("MAX77693_PMIC");
	if (!p_chrg) {
		puts("MAX77693_PMIC: Not found\n");
		return -ENODEV;
	}

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic) {
		puts("MAX77693_MUIC: Not found\n");
		return -ENODEV;
	}

	p_fg = pmic_get("MAX77693_FG");
	if (!p_fg) {
		puts("MAX17042_FG: Not found\n");
		return -ENODEV;
	}

	if (p_chrg->chrg->chrg_bat_present(p_chrg) == 0)
		puts("No battery detected\n");

	p_bat = pmic_get("BAT_TRATS2");
	if (!p_bat) {
		puts("BAT_TRATS2: Not found\n");
		return -ENODEV;
	}

	p_fg->parent =  p_bat;
	p_chrg->parent = p_bat;
	p_muic->parent = p_bat;

	p_bat->pbat->battery_init(p_bat, p_fg, p_chrg, p_muic);

	pb = p_bat->pbat;
	chrg = p_muic->chrg->chrg_type(p_muic);
	debug("CHARGER TYPE: %d\n", chrg);

	if (!p_chrg->chrg->chrg_bat_present(p_chrg)) {
		puts("No battery detected\n");
		return 0;
	}

	p_fg->fg->fg_battery_check(p_fg, p_bat);

	if (pb->bat->state == CHARGE && chrg == CHARGER_USB)
		puts("CHARGE Battery !\n");
#endif
	return 0;
}

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	int ret = 0;
	unsigned int val;
	struct pmic *p, *p_pmic, *p_muic;

	p_pmic = pmic_get("MAX77686_PMIC");
	if (!p_pmic)
		return -ENODEV;

	if (pmic_probe(p_pmic))
		return -1;

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic)
		return -ENODEV;

	if (pmic_probe(p_muic))
		return -1;

	if (on) {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_ON);
		if (ret)
			return -1;

		p = pmic_get("MAX77693_PMIC");
		if (!p)
			return -ENODEV;

		if (pmic_probe(p))
			return -1;

		/* SAFEOUT */
		ret = pmic_reg_read(p, MAX77693_SAFEOUT, &val);
		if (ret)
			return -1;

		val |= MAX77693_ENSAFEOUT1;
		ret = pmic_reg_write(p, MAX77693_SAFEOUT, val);
		if (ret)
			return -1;

		/* PATH: USB */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_DN1DP2);

	} else {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_LPM);
		if (ret)
			return -1;

		/* PATH: UART */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_UT1UR2);
	}

	if (ret)
		return -1;
#endif
	return 0;
}

struct dwc2_plat_otg_data s5pc210_otg_data = {
	.phy_control	= s5pc210_phy_control,
	.regs_phy	= EXYNOS4X12_USBPHY_BASE,
	.regs_otg	= EXYNOS4X12_USBOTG_BASE,
	.usb_phy_ctrl	= EXYNOS4X12_USBPHY_CONTROL,
	.usb_flags	= PHY0_SLEEP,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return dwc2_udc_probe(&s5pc210_otg_data);
}

int g_dnl_board_usb_cable_connected(void)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	struct pmic *muic = pmic_get("MAX77693_MUIC");
	if (!muic)
		return 0;

	return !!muic->chrg->chrg_type(muic);
#else
	return false;
#endif
}
#endif

/*
 * LCD
 */

#ifdef CONFIG_LCD
int mipi_power(void)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	struct pmic *p = pmic_get("MAX77686_PMIC");

	/* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_ON);
	/* LDO10 VMIPI_1.8V_AP */
	max77686_set_ldo_mode(p, 10, OPMODE_ON);
#endif

	return 0;
}

void exynos_lcd_power_on(void)
{
#ifndef CONFIG_DM_I2C /* TODO(maintainer): Convert to driver model */
	struct pmic *p = pmic_get("MAX77686_PMIC");

	/* LCD_2.2V_EN: GPC0[1] */
	gpio_request(EXYNOS4X12_GPIO_C01, "lcd_2v2_en");
	gpio_set_pull(EXYNOS4X12_GPIO_C01, S5P_GPIO_PULL_UP);
	gpio_direction_output(EXYNOS4X12_GPIO_C01, 1);

	/* LDO25 VCC_3.1V_LCD */
	pmic_probe(p);
	max77686_set_ldo_voltage(p, 25, 3100000);
	max77686_set_ldo_mode(p, 25, OPMODE_LPM);
#endif
}

void exynos_reset_lcd(void)
{
	/* reset lcd */
	gpio_request(EXYNOS4X12_GPIO_F21, "lcd_reset");
	gpio_direction_output(EXYNOS4X12_GPIO_F21, 0);
	udelay(10);
	gpio_set_value(EXYNOS4X12_GPIO_F21, 1);
}

void exynos_lcd_misc_init(vidinfo_t *vid)
{
#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif
#ifdef CONFIG_S6E8AX0
	s6e8ax0_init();
#endif
}
#endif /* LCD */
