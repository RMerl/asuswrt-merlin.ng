// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 Boot PMIC init
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

#include "mxs_init.h"

#ifdef CONFIG_SYS_MXS_VDD5V_ONLY
#define DCDC4P2_DROPOUT_CONFIG	POWER_DCDC4P2_DROPOUT_CTRL_100MV | \
				POWER_DCDC4P2_DROPOUT_CTRL_SRC_4P2
#else
#define DCDC4P2_DROPOUT_CONFIG	POWER_DCDC4P2_DROPOUT_CTRL_100MV | \
				POWER_DCDC4P2_DROPOUT_CTRL_SRC_SEL
#endif
/**
 * mxs_power_clock2xtal() - Switch CPU core clock source to 24MHz XTAL
 *
 * This function switches the CPU core clock from PLL to 24MHz XTAL
 * oscilator. This is necessary if the PLL is being reconfigured to
 * prevent crash of the CPU core.
 */
static void mxs_power_clock2xtal(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	debug("SPL: Switching CPU clock to 24MHz XTAL\n");

	/* Set XTAL as CPU reference clock */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
		&clkctrl_regs->hw_clkctrl_clkseq_set);
}

/**
 * mxs_power_clock2pll() - Switch CPU core clock source to PLL
 *
 * This function switches the CPU core clock from 24MHz XTAL oscilator
 * to PLL. This can only be called once the PLL has re-locked and once
 * the PLL is stable after reconfiguration.
 */
static void mxs_power_clock2pll(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	debug("SPL: Switching CPU core clock source to PLL\n");

	/*
	 * TODO: Are we really? It looks like we turn on PLL0, but we then
	 * set the CLKCTRL_CLKSEQ_BYPASS_CPU bit of the (which was already
	 * set by mxs_power_clock2xtal()). Clearing this bit here seems to
	 * introduce some instability (causing the CPU core to hang). Maybe
	 * we aren't giving PLL0 enough time to stabilise?
	 */
	setbits_le32(&clkctrl_regs->hw_clkctrl_pll0ctrl0,
			CLKCTRL_PLL0CTRL0_POWER);
	early_delay(100);

	/*
	 * TODO: Should the PLL0 FORCE_LOCK bit be set here followed be a
	 * wait on the PLL0 LOCK bit?
	 */
	setbits_le32(&clkctrl_regs->hw_clkctrl_clkseq,
			CLKCTRL_CLKSEQ_BYPASS_CPU);
}

/**
 * mxs_power_set_auto_restart() - Set the auto-restart bit
 *
 * This function ungates the RTC block and sets the AUTO_RESTART
 * bit to work around a design bug on MX28EVK Rev. A .
 */

static void mxs_power_set_auto_restart(void)
{
	struct mxs_rtc_regs *rtc_regs =
		(struct mxs_rtc_regs *)MXS_RTC_BASE;

	debug("SPL: Setting auto-restart bit\n");

	writel(RTC_CTRL_SFTRST, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_ctrl) & RTC_CTRL_SFTRST)
		;

	writel(RTC_CTRL_CLKGATE, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_ctrl) & RTC_CTRL_CLKGATE)
		;

	/* Do nothing if flag already set */
	if (readl(&rtc_regs->hw_rtc_persistent0) & RTC_PERSISTENT0_AUTO_RESTART)
		return;

	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_NEW_REGS_MASK)
		;

	setbits_le32(&rtc_regs->hw_rtc_persistent0,
			RTC_PERSISTENT0_AUTO_RESTART);
	writel(RTC_CTRL_FORCE_UPDATE, &rtc_regs->hw_rtc_ctrl_set);
	writel(RTC_CTRL_FORCE_UPDATE, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_NEW_REGS_MASK)
		;
	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_STALE_REGS_MASK)
		;
}

/**
 * mxs_power_set_linreg() - Set linear regulators 25mV below DC-DC converter
 *
 * This function configures the VDDIO, VDDA and VDDD linear regulators output
 * to be 25mV below the VDDIO, VDDA and VDDD output from the DC-DC switching
 * converter. This is the recommended setting for the case where we use both
 * linear regulators and DC-DC converter to power the VDDIO rail.
 */
static void mxs_power_set_linreg(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Set linear regulator 25mV below switching converter */
	debug("SPL: Setting VDDD 25mV below DC-DC converters\n");
	clrsetbits_le32(&power_regs->hw_power_vdddctrl,
			POWER_VDDDCTRL_LINREG_OFFSET_MASK,
			POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW);

	debug("SPL: Setting VDDA 25mV below DC-DC converters\n");
	clrsetbits_le32(&power_regs->hw_power_vddactrl,
			POWER_VDDACTRL_LINREG_OFFSET_MASK,
			POWER_VDDACTRL_LINREG_OFFSET_1STEPS_BELOW);

	debug("SPL: Setting VDDIO 25mV below DC-DC converters\n");
	clrsetbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_LINREG_OFFSET_MASK,
			POWER_VDDIOCTRL_LINREG_OFFSET_1STEPS_BELOW);
}

/**
 * mxs_get_batt_volt() - Measure battery input voltage
 *
 * This function retrieves the battery input voltage and returns it.
 */
static int mxs_get_batt_volt(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t volt = readl(&power_regs->hw_power_battmonitor);
	volt &= POWER_BATTMONITOR_BATT_VAL_MASK;
	volt >>= POWER_BATTMONITOR_BATT_VAL_OFFSET;
	volt *= 8;

	debug("SPL: Battery Voltage = %dmV\n", volt);
	return volt;
}

/**
 * mxs_is_batt_ready() - Test if the battery provides enough voltage to boot
 *
 * This function checks if the battery input voltage is higher than 3.6V and
 * therefore allows the system to successfully boot using this power source.
 */
static int mxs_is_batt_ready(void)
{
	return (mxs_get_batt_volt() >= 3600);
}

/**
 * mxs_is_batt_good() - Test if battery is operational at all
 *
 * This function starts recharging the battery and tests if the input current
 * provided by the 5V input recharging the battery is also sufficient to power
 * the DC-DC converter.
 */
static int mxs_is_batt_good(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t volt = mxs_get_batt_volt();

	if ((volt >= 2400) && (volt <= 4300)) {
		debug("SPL: Battery is good\n");
		return 1;
	}

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x3 << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	clrsetbits_le32(&power_regs->hw_power_charge,
		POWER_CHARGE_STOP_ILIMIT_MASK | POWER_CHARGE_BATTCHRG_I_MASK,
		POWER_CHARGE_STOP_ILIMIT_10MA | 0x3);

	writel(POWER_CHARGE_PWD_BATTCHRG, &power_regs->hw_power_charge_clr);
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	early_delay(500000);

	volt = mxs_get_batt_volt();

	if (volt >= 3500) {
		debug("SPL: Battery Voltage too high\n");
		return 0;
	}

	if (volt >= 2400) {
		debug("SPL: Battery is good\n");
		return 1;
	}

	writel(POWER_CHARGE_STOP_ILIMIT_MASK | POWER_CHARGE_BATTCHRG_I_MASK,
		&power_regs->hw_power_charge_clr);
	writel(POWER_CHARGE_PWD_BATTCHRG, &power_regs->hw_power_charge_set);

	debug("SPL: Battery Voltage too low\n");
	return 0;
}

/**
 * mxs_power_setup_5v_detect() - Start the 5V input detection comparator
 *
 * This function enables the 5V detection comparator and sets the 5V valid
 * threshold to 4.4V . We use 4.4V threshold here to make sure that even
 * under high load, the voltage drop on the 5V input won't be so critical
 * to cause undervolt on the 4P2 linear regulator supplying the DC-DC
 * converter and thus making the system crash.
 */
static void mxs_power_setup_5v_detect(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Start 5V detection */
	debug("SPL: Starting 5V input detection comparator\n");
	clrsetbits_le32(&power_regs->hw_power_5vctrl,
			POWER_5VCTRL_VBUSVALID_TRSH_MASK,
			POWER_5VCTRL_VBUSVALID_TRSH_4V4 |
			POWER_5VCTRL_PWRUP_VBUS_CMPS);
}

/**
 * mxs_power_switch_dcdc_clocksource() - Switch PLL clock for DC-DC converters
 * @freqsel:	One of the POWER_MISC_FREQSEL_xxx defines to select the clock
 *
 * This function configures and then enables an alternative PLL clock source
 * for the DC-DC converters.
 */
void mxs_power_switch_dcdc_clocksource(uint32_t freqsel)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Select clocksource for DC-DC converters */
	clrsetbits_le32(&power_regs->hw_power_misc,
			POWER_MISC_FREQSEL_MASK,
			freqsel);
	setbits_le32(&power_regs->hw_power_misc,
			POWER_MISC_SEL_PLLCLK);
}

/**
 * mxs_power_setup_dcdc_clocksource() - Setup PLL clock source for DC-DC converters
 *
 * Normally, there is no need to switch DC-DC clocksource. This is the reason,
 * why this function is a stub and does nothing. However, boards can implement
 * this function when required and call mxs_power_switch_dcdc_clocksource() to
 * switch to an alternative clock source.
 */
__weak void mxs_power_setup_dcdc_clocksource(void)
{
	debug("SPL: Using default DC-DC clocksource\n");
}

/**
 * mxs_src_power_init() - Preconfigure the power block
 *
 * This function configures reasonable values for the DC-DC control loop
 * and battery monitor.
 */
static void mxs_src_power_init(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Pre-Configuring power block\n");

	/* Improve efficieny and reduce transient ripple */
	writel(POWER_LOOPCTRL_TOGGLE_DIF | POWER_LOOPCTRL_EN_CM_HYST |
		POWER_LOOPCTRL_EN_DF_HYST, &power_regs->hw_power_loopctrl_set);

	clrsetbits_le32(&power_regs->hw_power_dclimits,
			POWER_DCLIMITS_POSLIMIT_BUCK_MASK,
			0x30 << POWER_DCLIMITS_POSLIMIT_BUCK_OFFSET);

	setbits_le32(&power_regs->hw_power_battmonitor,
			POWER_BATTMONITOR_EN_BATADJ);

	/* Increase the RCSCALE level for quick DCDC response to dynamic load */
	clrsetbits_le32(&power_regs->hw_power_loopctrl,
			POWER_LOOPCTRL_EN_RCSCALE_MASK,
			POWER_LOOPCTRL_RCSCALE_THRESH |
			POWER_LOOPCTRL_EN_RCSCALE_8X);

	clrsetbits_le32(&power_regs->hw_power_minpwr,
			POWER_MINPWR_HALFFETS, POWER_MINPWR_DOUBLE_FETS);

	/* 5V to battery handoff ... FIXME */
	setbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
	early_delay(30);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
}

/**
 * mxs_power_init_4p2_params() - Configure the parameters of the 4P2 regulator
 *
 * This function configures the necessary parameters for the 4P2 linear
 * regulator to supply the DC-DC converter from 5V input.
 */
static void mxs_power_init_4p2_params(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Configuring common 4P2 regulator params\n");

	/* Setup 4P2 parameters */
	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
		POWER_DCDC4P2_CMPTRIP_MASK | POWER_DCDC4P2_TRG_MASK,
		POWER_DCDC4P2_TRG_4V2 | (31 << POWER_DCDC4P2_CMPTRIP_OFFSET));

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_HEADROOM_ADJ_MASK,
		0x4 << POWER_5VCTRL_HEADROOM_ADJ_OFFSET);

	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
		POWER_DCDC4P2_DROPOUT_CTRL_MASK,
		DCDC4P2_DROPOUT_CONFIG);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x3f << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
}

/**
 * mxs_enable_4p2_dcdc_input() - Enable or disable the DCDC input from 4P2
 * @xfer:	Select if the input shall be enabled or disabled
 *
 * This function enables or disables the 4P2 input into the DC-DC converter.
 */
static void mxs_enable_4p2_dcdc_input(int xfer)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp, vbus_thresh, vbus_5vdetect, pwd_bo;
	uint32_t prev_5v_brnout, prev_5v_droop;

	debug("SPL: %s 4P2 DC-DC Input\n", xfer ? "Enabling" : "Disabling");

	if (xfer && (readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
		return;
	}

	prev_5v_brnout = readl(&power_regs->hw_power_5vctrl) &
				POWER_5VCTRL_PWDN_5VBRNOUT;
	prev_5v_droop = readl(&power_regs->hw_power_ctrl) &
				POWER_CTRL_ENIRQ_VDD5V_DROOP;

	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_PWDN_5VBRNOUT);
	writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
		&power_regs->hw_power_reset);

	clrbits_le32(&power_regs->hw_power_ctrl, POWER_CTRL_ENIRQ_VDD5V_DROOP);

	/*
	 * Recording orignal values that will be modified temporarlily
	 * to handle a chip bug. See chip errata for CQ ENGR00115837
	 */
	tmp = readl(&power_regs->hw_power_5vctrl);
	vbus_thresh = tmp & POWER_5VCTRL_VBUSVALID_TRSH_MASK;
	vbus_5vdetect = tmp & POWER_5VCTRL_VBUSVALID_5VDETECT;

	pwd_bo = readl(&power_regs->hw_power_minpwr) & POWER_MINPWR_PWD_BO;

	/*
	 * Disable mechanisms that get erroneously tripped by when setting
	 * the DCDC4P2 EN_DCDC
	 */
	clrbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_VBUSVALID_5VDETECT |
		POWER_5VCTRL_VBUSVALID_TRSH_MASK);

	writel(POWER_MINPWR_PWD_BO, &power_regs->hw_power_minpwr_set);

	if (xfer) {
		setbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_DCDC_XFER);
		early_delay(20);
		clrbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_DCDC_XFER);

		setbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_ENABLE_DCDC);
	} else {
		setbits_le32(&power_regs->hw_power_dcdc4p2,
				POWER_DCDC4P2_ENABLE_DCDC);
	}

	early_delay(25);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
			POWER_5VCTRL_VBUSVALID_TRSH_MASK, vbus_thresh);

	if (vbus_5vdetect)
		writel(vbus_5vdetect, &power_regs->hw_power_5vctrl_set);

	if (!pwd_bo)
		clrbits_le32(&power_regs->hw_power_minpwr, POWER_MINPWR_PWD_BO);

	while (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ)
		writel(POWER_CTRL_VBUS_VALID_IRQ,
			&power_regs->hw_power_ctrl_clr);

	if (prev_5v_brnout) {
		writel(POWER_5VCTRL_PWDN_5VBRNOUT,
			&power_regs->hw_power_5vctrl_set);
		writel(POWER_RESET_UNLOCK_KEY,
			&power_regs->hw_power_reset);
	} else {
		writel(POWER_5VCTRL_PWDN_5VBRNOUT,
			&power_regs->hw_power_5vctrl_clr);
		writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
			&power_regs->hw_power_reset);
	}

	while (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VDD5V_DROOP_IRQ)
		writel(POWER_CTRL_VDD5V_DROOP_IRQ,
			&power_regs->hw_power_ctrl_clr);

	if (prev_5v_droop)
		clrbits_le32(&power_regs->hw_power_ctrl,
				POWER_CTRL_ENIRQ_VDD5V_DROOP);
	else
		setbits_le32(&power_regs->hw_power_ctrl,
				POWER_CTRL_ENIRQ_VDD5V_DROOP);
}

/**
 * mxs_power_init_4p2_regulator() - Start the 4P2 regulator
 *
 * This function enables the 4P2 regulator and switches the DC-DC converter
 * to use the 4P2 input.
 */
static void mxs_power_init_4p2_regulator(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp, tmp2;

	debug("SPL: Enabling 4P2 regulator\n");

	setbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_ENABLE_4P2);

	writel(POWER_CHARGE_ENABLE_LOAD, &power_regs->hw_power_charge_set);

	writel(POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		&power_regs->hw_power_5vctrl_clr);
	clrbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_TRG_MASK);

	/* Power up the 4p2 rail and logic/control */
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	/*
	 * Start charging up the 4p2 capacitor. We ramp of this charge
	 * gradually to avoid large inrush current from the 5V cable which can
	 * cause transients/problems
	 */
	debug("SPL: Charging 4P2 capacitor\n");
	mxs_enable_4p2_dcdc_input(0);

	if (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ) {
		/*
		 * If we arrived here, we were unable to recover from mx23 chip
		 * errata 5837. 4P2 is disabled and sufficient battery power is
		 * not present. Exiting to not enable DCDC power during 5V
		 * connected state.
		 */
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC);
		writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
			&power_regs->hw_power_5vctrl_set);

		debug("SPL: Unable to recover from mx23 errata 5837\n");
		hang();
	}

	/*
	 * Here we set the 4p2 brownout level to something very close to 4.2V.
	 * We then check the brownout status. If the brownout status is false,
	 * the voltage is already close to the target voltage of 4.2V so we
	 * can go ahead and set the 4P2 current limit to our max target limit.
	 * If the brownout status is true, we need to ramp us the current limit
	 * so that we don't cause large inrush current issues. We step up the
	 * current limit until the brownout status is false or until we've
	 * reached our maximum defined 4p2 current limit.
	 */
	debug("SPL: Setting 4P2 brownout level\n");
	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_BO_MASK,
			22 << POWER_DCDC4P2_BO_OFFSET);	/* 4.15V */

	if (!(readl(&power_regs->hw_power_sts) & POWER_STS_DCDC_4P2_BO)) {
		setbits_le32(&power_regs->hw_power_5vctrl,
			0x3f << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
	} else {
		tmp = (readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK) >>
			POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET;
		while (tmp < 0x3f) {
			if (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DCDC_4P2_BO)) {
				tmp = readl(&power_regs->hw_power_5vctrl);
				tmp |= POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK;
				early_delay(100);
				writel(tmp, &power_regs->hw_power_5vctrl);
				break;
			} else {
				tmp++;
				tmp2 = readl(&power_regs->hw_power_5vctrl);
				tmp2 &= ~POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK;
				tmp2 |= tmp <<
					POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET;
				writel(tmp2, &power_regs->hw_power_5vctrl);
				early_delay(100);
			}
		}
	}

	clrbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_BO_MASK);
	writel(POWER_CTRL_DCDC4P2_BO_IRQ, &power_regs->hw_power_ctrl_clr);
}

/**
 * mxs_power_init_dcdc_4p2_source() - Switch DC-DC converter to 4P2 source
 *
 * This function configures the DC-DC converter to be supplied from the 4P2
 * linear regulator.
 */
static void mxs_power_init_dcdc_4p2_source(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Switching DC-DC converters to 4P2\n");

	if (!(readl(&power_regs->hw_power_dcdc4p2) &
		POWER_DCDC4P2_ENABLE_DCDC)) {
		debug("SPL: Already switched - aborting\n");
		hang();
	}

	mxs_enable_4p2_dcdc_input(1);

	if (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ) {
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC);
		writel(POWER_5VCTRL_ENABLE_DCDC,
			&power_regs->hw_power_5vctrl_clr);
		writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
			&power_regs->hw_power_5vctrl_set);
	}
}

/**
 * mxs_power_enable_4p2() - Power up the 4P2 regulator
 *
 * This function drives the process of powering up the 4P2 linear regulator
 * and switching the DC-DC converter input over to the 4P2 linear regulator.
 */
static void mxs_power_enable_4p2(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t vdddctrl, vddactrl, vddioctrl;
	uint32_t tmp;

	debug("SPL: Powering up 4P2 regulator\n");

	vdddctrl = readl(&power_regs->hw_power_vdddctrl);
	vddactrl = readl(&power_regs->hw_power_vddactrl);
	vddioctrl = readl(&power_regs->hw_power_vddioctrl);

	setbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG |
		POWER_VDDDCTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddactrl,
		POWER_VDDACTRL_DISABLE_FET | POWER_VDDACTRL_ENABLE_LINREG |
		POWER_VDDACTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddioctrl,
		POWER_VDDIOCTRL_DISABLE_FET | POWER_VDDIOCTRL_PWDN_BRNOUT);

	mxs_power_init_4p2_params();
	mxs_power_init_4p2_regulator();

	/* Shutdown battery (none present) */
	if (!mxs_is_batt_ready()) {
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
				POWER_DCDC4P2_BO_MASK);
		writel(POWER_CTRL_DCDC4P2_BO_IRQ,
				&power_regs->hw_power_ctrl_clr);
		writel(POWER_CTRL_ENIRQ_DCDC4P2_BO,
				&power_regs->hw_power_ctrl_clr);
	}

	mxs_power_init_dcdc_4p2_source();

	writel(vdddctrl, &power_regs->hw_power_vdddctrl);
	early_delay(20);
	writel(vddactrl, &power_regs->hw_power_vddactrl);
	early_delay(20);
	writel(vddioctrl, &power_regs->hw_power_vddioctrl);

	/*
	 * Check if FET is enabled on either powerout and if so,
	 * disable load.
	 */
	tmp = 0;
	tmp |= !(readl(&power_regs->hw_power_vdddctrl) &
			POWER_VDDDCTRL_DISABLE_FET);
	tmp |= !(readl(&power_regs->hw_power_vddactrl) &
			POWER_VDDACTRL_DISABLE_FET);
	tmp |= !(readl(&power_regs->hw_power_vddioctrl) &
			POWER_VDDIOCTRL_DISABLE_FET);
	if (tmp)
		writel(POWER_CHARGE_ENABLE_LOAD,
			&power_regs->hw_power_charge_clr);

	debug("SPL: 4P2 regulator powered-up\n");
}

/**
 * mxs_boot_valid_5v() - Boot from 5V supply
 *
 * This function configures the power block to boot from valid 5V input.
 * This is called only if the 5V is reliable and can properly supply the
 * CPU. This function proceeds to configure the 4P2 converter to be supplied
 * from the 5V input.
 */
static void mxs_boot_valid_5v(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Booting from 5V supply\n");

	/*
	 * Use VBUSVALID level instead of VDD5V_GT_VDDIO level to trigger a 5V
	 * disconnect event. FIXME
	 */
	writel(POWER_5VCTRL_VBUSVALID_5VDETECT,
		&power_regs->hw_power_5vctrl_set);

	/* Configure polarity to check for 5V disconnection. */
	writel(POWER_CTRL_POLARITY_VBUSVALID |
		POWER_CTRL_POLARITY_VDD5V_GT_VDDIO,
		&power_regs->hw_power_ctrl_clr);

	writel(POWER_CTRL_VBUS_VALID_IRQ | POWER_CTRL_VDD5V_GT_VDDIO_IRQ,
		&power_regs->hw_power_ctrl_clr);

	mxs_power_enable_4p2();
}

/**
 * mxs_powerdown() - Shut down the system
 *
 * This function powers down the CPU completely.
 */
static void mxs_powerdown(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("Powering Down\n");

	writel(POWER_RESET_UNLOCK_KEY, &power_regs->hw_power_reset);
	writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
		&power_regs->hw_power_reset);
}

/**
 * mxs_batt_boot() - Configure the power block to boot from battery input
 *
 * This function configures the power block to boot from the battery voltage
 * supply.
 */
static void mxs_batt_boot(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Configuring power block to boot from battery\n");

	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_PWDN_5VBRNOUT);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_ENABLE_DCDC);

	clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC | POWER_DCDC4P2_ENABLE_4P2);
	writel(POWER_CHARGE_ENABLE_LOAD, &power_regs->hw_power_charge_clr);

	/* 5V to battery handoff. */
	setbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
	early_delay(30);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);

	writel(POWER_CTRL_ENIRQ_DCDC4P2_BO, &power_regs->hw_power_ctrl_clr);

	clrsetbits_le32(&power_regs->hw_power_minpwr,
			POWER_MINPWR_HALFFETS, POWER_MINPWR_DOUBLE_FETS);

	mxs_power_set_linreg();

	clrbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG);

	clrbits_le32(&power_regs->hw_power_vddactrl,
		POWER_VDDACTRL_DISABLE_FET | POWER_VDDACTRL_ENABLE_LINREG);

	clrbits_le32(&power_regs->hw_power_vddioctrl,
		POWER_VDDIOCTRL_DISABLE_FET);

	setbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_PWD_CHARGE_4P2_MASK);

	setbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_ENABLE_DCDC);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x8 << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);

	mxs_power_enable_4p2();
}

/**
 * mxs_handle_5v_conflict() - Test if the 5V input is reliable
 *
 * This function tests if the 5V input can reliably supply the system. If it
 * can, then proceed to configuring the system to boot from 5V source, otherwise
 * try booting from battery supply. If we can not boot from battery supply
 * either, shut down the system.
 */
static void mxs_handle_5v_conflict(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	debug("SPL: Resolving 5V conflict\n");

	setbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_BO_OFFSET_MASK);

	for (;;) {
		tmp = readl(&power_regs->hw_power_sts);

		if (tmp & POWER_STS_VDDIO_BO) {
			/*
			 * VDDIO has a brownout, then the VDD5V_GT_VDDIO becomes
			 * unreliable
			 */
			debug("SPL: VDDIO has a brownout\n");
			mxs_powerdown();
			break;
		}

		if (tmp & POWER_STS_VDD5V_GT_VDDIO) {
			debug("SPL: POWER_STS_VDD5V_GT_VDDIO is set\n");
			mxs_boot_valid_5v();
			break;
		} else {
			debug("SPL: POWER_STS_VDD5V_GT_VDDIO is not set\n");
			mxs_powerdown();
			break;
		}

		/*
		 * TODO: I can't see this being reached. We'll either
		 * powerdown or boot from a stable 5V supply.
		 */
		if (tmp & POWER_STS_PSWITCH_MASK) {
			debug("SPL: POWER_STS_PSWITCH_MASK is set\n");
			mxs_batt_boot();
			break;
		}
	}
}

/**
 * mxs_5v_boot() - Configure the power block to boot from 5V input
 *
 * This function handles configuration of the power block when supplied by
 * a 5V input.
 */
static void mxs_5v_boot(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Configuring power block to boot from 5V input\n");

	/*
	 * NOTE: In original IMX-Bootlets, this also checks for VBUSVALID,
	 * but their implementation always returns 1 so we omit it here.
	 */
	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		debug("SPL: 5V VDD good\n");
		mxs_boot_valid_5v();
		return;
	}

	early_delay(1000);
	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		debug("SPL: 5V VDD good (after delay)\n");
		mxs_boot_valid_5v();
		return;
	}

	debug("SPL: 5V VDD not good\n");
	mxs_handle_5v_conflict();
}

/**
 * mxs_init_batt_bo() - Configure battery brownout threshold
 *
 * This function configures the battery input brownout threshold. The value
 * at which the battery brownout happens is configured to 3.0V in the code.
 */
static void mxs_init_batt_bo(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Initialising battery brown-out level to 3.0V\n");

	/* Brownout at 3V */
	clrsetbits_le32(&power_regs->hw_power_battmonitor,
		POWER_BATTMONITOR_BRWNOUT_LVL_MASK,
		15 << POWER_BATTMONITOR_BRWNOUT_LVL_OFFSET);

	writel(POWER_CTRL_BATT_BO_IRQ, &power_regs->hw_power_ctrl_clr);
	writel(POWER_CTRL_ENIRQ_BATT_BO, &power_regs->hw_power_ctrl_clr);
}

/**
 * mxs_switch_vddd_to_dcdc_source() - Switch VDDD rail to DC-DC converter
 *
 * This function turns off the VDDD linear regulator and therefore makes
 * the VDDD rail be supplied only by the DC-DC converter.
 */
static void mxs_switch_vddd_to_dcdc_source(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Switching VDDD to DC-DC converters\n");

	clrsetbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_LINREG_OFFSET_MASK,
		POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW);

	clrbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG |
		POWER_VDDDCTRL_DISABLE_STEPPING);
}

/**
 * mxs_power_configure_power_source() - Configure power block source
 *
 * This function is the core of the power configuration logic. The function
 * selects the power block input source and configures the whole power block
 * accordingly. After the configuration is complete and the system is stable
 * again, the function switches the CPU clock source back to PLL. Finally,
 * the function switches the voltage rails to DC-DC converter.
 */
static void mxs_power_configure_power_source(void)
{
	int batt_ready, batt_good;
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	struct mxs_lradc_regs *lradc_regs =
		(struct mxs_lradc_regs *)MXS_LRADC_BASE;

	debug("SPL: Configuring power source\n");

	mxs_power_setup_dcdc_clocksource();
	mxs_src_power_init();

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		batt_ready = mxs_is_batt_ready();
		if (batt_ready) {
			/* 5V source detected, good battery detected. */
			mxs_batt_boot();
		} else {
			batt_good = mxs_is_batt_good();
			if (!batt_good) {
				/* 5V source detected, bad battery detected. */
				writel(LRADC_CONVERSION_AUTOMATIC,
					&lradc_regs->hw_lradc_conversion_clr);
				clrbits_le32(&power_regs->hw_power_battmonitor,
					POWER_BATTMONITOR_BATT_VAL_MASK);
			}
			mxs_5v_boot();
		}
	} else {
		/* 5V not detected, booting from battery. */
		mxs_batt_boot();
	}

	/*
	 * TODO: Do not switch CPU clock to PLL if we are VDD5V is sourced
	 * from USB VBUS
	 */
	mxs_power_clock2pll();

	mxs_init_batt_bo();

	mxs_switch_vddd_to_dcdc_source();

#ifdef CONFIG_MX23
	/* Fire up the VDDMEM LinReg now that we're all set. */
	debug("SPL: Enabling mx23 VDDMEM linear regulator\n");
	writel(POWER_VDDMEMCTRL_ENABLE_LINREG | POWER_VDDMEMCTRL_ENABLE_ILIMIT,
		&power_regs->hw_power_vddmemctrl);
#endif
}

/**
 * mxs_enable_output_rail_protection() - Enable power rail protection
 *
 * This function enables overload protection on the power rails. This is
 * triggered if the power rails' voltage drops rapidly due to overload and
 * in such case, the supply to the powerrail is cut-off, protecting the
 * CPU from damage. Note that under such condition, the system will likely
 * crash or misbehave.
 */
static void mxs_enable_output_rail_protection(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Enabling output rail protection\n");

	writel(POWER_CTRL_VDDD_BO_IRQ | POWER_CTRL_VDDA_BO_IRQ |
		POWER_CTRL_VDDIO_BO_IRQ, &power_regs->hw_power_ctrl_clr);

	setbits_le32(&power_regs->hw_power_vdddctrl,
			POWER_VDDDCTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddactrl,
			POWER_VDDACTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_PWDN_BRNOUT);
}

/**
 * mxs_get_vddio_power_source_off() - Get VDDIO rail power source
 *
 * This function tests if the VDDIO rail is supplied by linear regulator
 * or by the DC-DC converter. Returns 1 if powered by linear regulator,
 * returns 0 if powered by the DC-DC converter.
 */
static int mxs_get_vddio_power_source_off(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		tmp = readl(&power_regs->hw_power_vddioctrl);
		if (tmp & POWER_VDDIOCTRL_DISABLE_FET) {
			if ((tmp & POWER_VDDIOCTRL_LINREG_OFFSET_MASK) ==
				POWER_VDDIOCTRL_LINREG_OFFSET_0STEPS) {
				return 1;
			}
		}

		if (!(readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
			if ((tmp & POWER_VDDIOCTRL_LINREG_OFFSET_MASK) ==
				POWER_VDDIOCTRL_LINREG_OFFSET_0STEPS) {
				return 1;
			}
		}
	}

	return 0;

}

/**
 * mxs_get_vddd_power_source_off() - Get VDDD rail power source
 *
 * This function tests if the VDDD rail is supplied by linear regulator
 * or by the DC-DC converter. Returns 1 if powered by linear regulator,
 * returns 0 if powered by the DC-DC converter.
 */
static int mxs_get_vddd_power_source_off(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	tmp = readl(&power_regs->hw_power_vdddctrl);
	if (tmp & POWER_VDDDCTRL_DISABLE_FET) {
		if ((tmp & POWER_VDDDCTRL_LINREG_OFFSET_MASK) ==
			POWER_VDDDCTRL_LINREG_OFFSET_0STEPS) {
			return 1;
		}
	}

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		if (!(readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
			return 1;
		}
	}

	if (!(tmp & POWER_VDDDCTRL_ENABLE_LINREG)) {
		if ((tmp & POWER_VDDDCTRL_LINREG_OFFSET_MASK) ==
			POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW) {
			return 1;
		}
	}

	return 0;
}

struct mxs_vddx_cfg {
	uint32_t		*reg;
	uint8_t			step_mV;
	uint16_t		lowest_mV;
	int			(*powered_by_linreg)(void);
	uint32_t		trg_mask;
	uint32_t		bo_irq;
	uint32_t		bo_enirq;
	uint32_t		bo_offset_mask;
	uint32_t		bo_offset_offset;
};

static const struct mxs_vddx_cfg mxs_vddio_cfg = {
	.reg			= &(((struct mxs_power_regs *)MXS_POWER_BASE)->
					hw_power_vddioctrl),
#if defined(CONFIG_MX23)
	.step_mV		= 25,
#else
	.step_mV		= 50,
#endif
	.lowest_mV		= 2800,
	.powered_by_linreg	= mxs_get_vddio_power_source_off,
	.trg_mask		= POWER_VDDIOCTRL_TRG_MASK,
	.bo_irq			= POWER_CTRL_VDDIO_BO_IRQ,
	.bo_enirq		= POWER_CTRL_ENIRQ_VDDIO_BO,
	.bo_offset_mask		= POWER_VDDIOCTRL_BO_OFFSET_MASK,
	.bo_offset_offset	= POWER_VDDIOCTRL_BO_OFFSET_OFFSET,
};

static const struct mxs_vddx_cfg mxs_vddd_cfg = {
	.reg			= &(((struct mxs_power_regs *)MXS_POWER_BASE)->
					hw_power_vdddctrl),
	.step_mV		= 25,
	.lowest_mV		= 800,
	.powered_by_linreg	= mxs_get_vddd_power_source_off,
	.trg_mask		= POWER_VDDDCTRL_TRG_MASK,
	.bo_irq			= POWER_CTRL_VDDD_BO_IRQ,
	.bo_enirq		= POWER_CTRL_ENIRQ_VDDD_BO,
	.bo_offset_mask		= POWER_VDDDCTRL_BO_OFFSET_MASK,
	.bo_offset_offset	= POWER_VDDDCTRL_BO_OFFSET_OFFSET,
};

#ifdef CONFIG_MX23
static const struct mxs_vddx_cfg mxs_vddmem_cfg = {
	.reg			= &(((struct mxs_power_regs *)MXS_POWER_BASE)->
					hw_power_vddmemctrl),
	.step_mV		= 50,
	.lowest_mV		= 1700,
	.powered_by_linreg	= NULL,
	.trg_mask		= POWER_VDDMEMCTRL_TRG_MASK,
	.bo_irq			= 0,
	.bo_enirq		= 0,
	.bo_offset_mask		= 0,
	.bo_offset_offset	= 0,
};
#endif

/**
 * mxs_power_set_vddx() - Configure voltage on DC-DC converter rail
 * @cfg:		Configuration data of the DC-DC converter rail
 * @new_target:		New target voltage of the DC-DC converter rail
 * @new_brownout:	New brownout trigger voltage
 *
 * This function configures the output voltage on the DC-DC converter rail.
 * The rail is selected by the @cfg argument. The new voltage target is
 * selected by the @new_target and the voltage is specified in mV. The
 * new brownout value is selected by the @new_brownout argument and the
 * value is also in mV.
 */
static void mxs_power_set_vddx(const struct mxs_vddx_cfg *cfg,
				uint32_t new_target, uint32_t new_brownout)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t cur_target, diff, bo_int = 0;
	uint32_t powered_by_linreg = 0;
	int adjust_up, tmp;

	new_brownout = DIV_ROUND_CLOSEST(new_target - new_brownout,
					 cfg->step_mV);

	cur_target = readl(cfg->reg);
	cur_target &= cfg->trg_mask;
	cur_target *= cfg->step_mV;
	cur_target += cfg->lowest_mV;

	adjust_up = new_target > cur_target;
	if (cfg->powered_by_linreg)
		powered_by_linreg = cfg->powered_by_linreg();

	if (adjust_up && cfg->bo_irq) {
		if (powered_by_linreg) {
			bo_int = readl(cfg->reg);
			clrbits_le32(cfg->reg, cfg->bo_enirq);
		}
		setbits_le32(cfg->reg, cfg->bo_offset_mask);
	}

	do {
		if (abs(new_target - cur_target) > 100) {
			if (adjust_up)
				diff = cur_target + 100;
			else
				diff = cur_target - 100;
		} else {
			diff = new_target;
		}

		diff -= cfg->lowest_mV;
		diff /= cfg->step_mV;

		clrsetbits_le32(cfg->reg, cfg->trg_mask, diff);

		if (powered_by_linreg ||
			(readl(&power_regs->hw_power_sts) &
				POWER_STS_VDD5V_GT_VDDIO))
			early_delay(500);
		else {
			for (;;) {
				tmp = readl(&power_regs->hw_power_sts);
				if (tmp & POWER_STS_DC_OK)
					break;
			}
		}

		cur_target = readl(cfg->reg);
		cur_target &= cfg->trg_mask;
		cur_target *= cfg->step_mV;
		cur_target += cfg->lowest_mV;
	} while (new_target > cur_target);

	if (cfg->bo_irq) {
		if (adjust_up && powered_by_linreg) {
			writel(cfg->bo_irq, &power_regs->hw_power_ctrl_clr);
			if (bo_int & cfg->bo_enirq)
				setbits_le32(cfg->reg, cfg->bo_enirq);
		}

		clrsetbits_le32(cfg->reg, cfg->bo_offset_mask,
				new_brownout << cfg->bo_offset_offset);
	}
}

/**
 * mxs_setup_batt_detect() - Start the battery voltage measurement logic
 *
 * This function starts and configures the LRADC block. This allows the
 * power initialization code to measure battery voltage and based on this
 * knowledge, decide whether to boot at all, boot from battery or boot
 * from 5V input.
 */
static void mxs_setup_batt_detect(void)
{
	debug("SPL: Starting battery voltage measurement logic\n");

	mxs_lradc_init();
	mxs_lradc_enable_batt_measurement();
	early_delay(10);
}

/**
 * mxs_ungate_power() - Ungate the POWER block
 *
 * This function ungates clock to the power block. In case the power block
 * was still gated at this point, it will not be possible to configure the
 * block and therefore the power initialization would fail. This function
 * is only needed on i.MX233, on i.MX28 the power block is always ungated.
 */
static void mxs_ungate_power(void)
{
#ifdef CONFIG_MX23
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	writel(POWER_CTRL_CLKGATE, &power_regs->hw_power_ctrl_clr);
#endif
}

/**
 * mxs_power_init() - The power block init main function
 *
 * This function calls all the power block initialization functions in
 * proper sequence to start the power block.
 */
void mxs_power_init(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Initialising Power Block\n");

	mxs_ungate_power();

	mxs_power_clock2xtal();
	mxs_power_set_auto_restart();
	mxs_power_set_linreg();
	mxs_power_setup_5v_detect();

	mxs_setup_batt_detect();

	mxs_power_configure_power_source();
	mxs_enable_output_rail_protection();

	debug("SPL: Setting VDDIO to 3V3 (brownout @ 3v15)\n");
	mxs_power_set_vddx(&mxs_vddio_cfg, 3300, 3150);

	debug("SPL: Setting VDDD to 1V5 (brownout @ 1v315)\n");
	mxs_power_set_vddx(&mxs_vddd_cfg, 1500, 1315);
#ifdef CONFIG_MX23
	debug("SPL: Setting mx23 VDDMEM to 2V5 (brownout @ 1v7)\n");
	mxs_power_set_vddx(&mxs_vddmem_cfg, 2500, 1700);
#endif
	writel(POWER_CTRL_VDDD_BO_IRQ | POWER_CTRL_VDDA_BO_IRQ |
		POWER_CTRL_VDDIO_BO_IRQ | POWER_CTRL_VDD5V_DROOP_IRQ |
		POWER_CTRL_VBUS_VALID_IRQ | POWER_CTRL_BATT_BO_IRQ |
		POWER_CTRL_DCDC4P2_BO_IRQ, &power_regs->hw_power_ctrl_clr);

	writel(POWER_5VCTRL_PWDN_5VBRNOUT, &power_regs->hw_power_5vctrl_set);

	early_delay(1000);
}

#ifdef	CONFIG_SPL_MXS_PSWITCH_WAIT
/**
 * mxs_power_wait_pswitch() - Wait for power switch to be pressed
 *
 * This function waits until the power-switch was pressed to start booting
 * the board.
 */
void mxs_power_wait_pswitch(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	debug("SPL: Waiting for power switch input\n");
	while (!(readl(&power_regs->hw_power_sts) & POWER_STS_PSWITCH_MASK))
		;
}
#endif
