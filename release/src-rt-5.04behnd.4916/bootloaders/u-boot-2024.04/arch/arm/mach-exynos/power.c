// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/power.h>

static void exynos4_mipi_phy_control(unsigned int dev_index,
					unsigned int enable)
{
	struct exynos4_power *pmu =
	    (struct exynos4_power *)samsung_get_base_power();
	unsigned int addr, cfg = 0;

	if (dev_index == 0)
		addr = (unsigned int)&pmu->mipi_phy0_control;
	else
		addr = (unsigned int)&pmu->mipi_phy1_control;


	cfg = readl(addr);
	if (enable)
		cfg |= (EXYNOS_MIPI_PHY_MRESETN | EXYNOS_MIPI_PHY_ENABLE);
	else
		cfg &= ~(EXYNOS_MIPI_PHY_MRESETN | EXYNOS_MIPI_PHY_ENABLE);

	writel(cfg, addr);
}

void set_mipi_phy_ctrl(unsigned int dev_index, unsigned int enable)
{
	if (cpu_is_exynos4())
		exynos4_mipi_phy_control(dev_index, enable);
}

void exynos5_set_usbhost_phy_ctrl(unsigned int enable)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();

	if (enable) {
		/* Enabling USBHOST_PHY */
		setbits_le32(&power->usbhost_phy_control,
				POWER_USB_HOST_PHY_CTRL_EN);
	} else {
		/* Disabling USBHOST_PHY */
		clrbits_le32(&power->usbhost_phy_control,
				POWER_USB_HOST_PHY_CTRL_EN);
	}
}

void exynos4412_set_usbhost_phy_ctrl(unsigned int enable)
{
	struct exynos4412_power *power =
		(struct exynos4412_power *)samsung_get_base_power();

	if (enable) {
		/* Enabling USBHOST_PHY */
		setbits_le32(&power->usbhost_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
		setbits_le32(&power->hsic1_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
		setbits_le32(&power->hsic2_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
	} else {
		/* Disabling USBHOST_PHY */
		clrbits_le32(&power->usbhost_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
		clrbits_le32(&power->hsic1_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
		clrbits_le32(&power->hsic2_phy_control,
			     POWER_USB_HOST_PHY_CTRL_EN);
	}
}

void set_usbhost_phy_ctrl(unsigned int enable)
{
	if (cpu_is_exynos5())
		exynos5_set_usbhost_phy_ctrl(enable);
	else if (cpu_is_exynos4())
		if (proid_is_exynos4412())
			exynos4412_set_usbhost_phy_ctrl(enable);
}

static void exynos5_set_usbdrd_phy_ctrl(unsigned int enable)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();

	if (enable) {
		/* Enabling USBDRD_PHY */
		setbits_le32(&power->usbdrd_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
	} else {
		/* Disabling USBDRD_PHY */
		clrbits_le32(&power->usbdrd_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
	}
}

static void exynos5420_set_usbdev_phy_ctrl(unsigned int enable)
{
	struct exynos5420_power *power =
		(struct exynos5420_power *)samsung_get_base_power();

	if (enable) {
		/* Enabling USBDEV_PHY */
		setbits_le32(&power->usbdev_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
		setbits_le32(&power->usbdev1_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
	} else {
		/* Disabling USBDEV_PHY */
		clrbits_le32(&power->usbdev_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
		clrbits_le32(&power->usbdev1_phy_control,
				POWER_USB_DRD_PHY_CTRL_EN);
	}
}

void set_usbdrd_phy_ctrl(unsigned int enable)
{
	if (cpu_is_exynos5()) {
		if (proid_is_exynos542x())
			exynos5420_set_usbdev_phy_ctrl(enable);
		else
			exynos5_set_usbdrd_phy_ctrl(enable);
	}
}

static void exynos5_dp_phy_control(unsigned int enable)
{
	unsigned int cfg;
	struct exynos5_power *power =
	    (struct exynos5_power *)samsung_get_base_power();

	cfg = readl(&power->dptx_phy_control);
	if (enable)
		cfg |= EXYNOS_DP_PHY_ENABLE;
	else
		cfg &= ~EXYNOS_DP_PHY_ENABLE;

	writel(cfg, &power->dptx_phy_control);
}

void exynos_dp_phy_ctrl(unsigned int enable)
{
	if (cpu_is_exynos5())
		exynos5_dp_phy_control(enable);
}

static void exynos5_set_ps_hold_ctrl(void)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();

	/* Set PS-Hold high */
	setbits_le32(&power->ps_hold_control,
			EXYNOS_PS_HOLD_CONTROL_DATA_HIGH);
}

/*
 * Set ps_hold data driving value high
 * This enables the machine to stay powered on
 * after the initial power-on condition goes away
 * (e.g. power button).
 */
void set_ps_hold_ctrl(void)
{
	if (cpu_is_exynos5())
		exynos5_set_ps_hold_ctrl();
}


static void exynos5_set_xclkout(void)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();

	/* use xxti for xclk out */
	clrsetbits_le32(&power->pmu_debug, PMU_DEBUG_CLKOUT_SEL_MASK,
				PMU_DEBUG_XXTI);
}

void set_xclkout(void)
{
	if (cpu_is_exynos5())
		exynos5_set_xclkout();
}

/* Enables hardware tripping to power off the system when TMU fails */
void set_hw_thermal_trip(void)
{
	if (cpu_is_exynos5()) {
		struct exynos5_power *power =
			(struct exynos5_power *)samsung_get_base_power();

		/* PS_HOLD_CONTROL register ENABLE_HW_TRIP bit*/
		setbits_le32(&power->ps_hold_control, POWER_ENABLE_HW_TRIP);
	}
}

static uint32_t exynos5_get_reset_status(void)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();

	return power->inform1;
}

static uint32_t exynos4_get_reset_status(void)
{
	struct exynos4_power *power =
		(struct exynos4_power *)samsung_get_base_power();

	return power->inform1;
}

uint32_t get_reset_status(void)
{
	if (cpu_is_exynos5())
		return exynos5_get_reset_status();
	else
		return  exynos4_get_reset_status();
}

static void exynos5_power_exit_wakeup(void)
{
	struct exynos5_power *power =
		(struct exynos5_power *)samsung_get_base_power();
	typedef void (*resume_func)(void);

	((resume_func)power->inform0)();
}

static void exynos4_power_exit_wakeup(void)
{
	struct exynos4_power *power =
		(struct exynos4_power *)samsung_get_base_power();
	typedef void (*resume_func)(void);

	((resume_func)power->inform0)();
}

void power_exit_wakeup(void)
{
	if (cpu_is_exynos5())
		exynos5_power_exit_wakeup();
	else
		exynos4_power_exit_wakeup();
}

unsigned int get_boot_mode(void)
{
	unsigned int om_pin = samsung_get_base_power();

	return readl(om_pin) & OM_PIN_MASK;
}
